#!/usr/bin/env python3
"""
Autoregressive WaveNet inference at 3 kHz, 8-bit mu-law (matching train.py / convert_data.py).
"""

from __future__ import annotations

import argparse

import numpy as np
import soundfile as sf
import torch
from scipy.signal import resample
from tqdm import tqdm

from wavenet import WaveNet

SAMPLE_RATE = 3000
NUM_CLASSES = 256
NUM_BITS = 8
START_TOKEN = 128  # mu-law index for silence (input amplitude 0)


def mu_law_encode(x: np.ndarray, num_bits: int) -> np.ndarray:
    """Same formulation as convert_data.py."""
    if np.any(x > 1.0) or np.any(x < -1.0):
        raise ValueError("Input signal must be in the range [-1, 1]")
    mu = 2**num_bits - 1
    y = np.sign(x) * np.log1p(mu * np.abs(x)) / np.log1p(mu)
    return np.round((y + 1) / 2 * mu).astype(np.uint8)


def mu_law_decode(indices: np.ndarray, num_bits: int) -> np.ndarray:
    """Inverse of mu_law_encode; indices are integers in [0, 2**num_bits - 1]."""
    mu = 2**num_bits - 1
    q = indices.astype(np.float32)
    y = q / mu * 2.0 - 1.0
    x = np.sign(y) * (np.expm1(np.abs(y) * np.log1p(mu))) / mu
    return np.clip(x, -1.0, 1.0)


def load_prompt_tokens(
    path: str,
    prompt_duration_sec: float | None,
    sample_rate: int,
) -> np.ndarray:
    """Load WAV, convert to mono 3 kHz float [-1, 1], optionally truncate, mu-law encode."""
    x, fs = sf.read(path, always_2d=False)
    if x.ndim > 1:
        x = x.mean(axis=-1)
    if fs != sample_rate:
        n_out = int(round(x.shape[0] * sample_rate / fs))
        x = resample(x, n_out)
    peak = float(np.max(np.abs(x))) if x.size else 1.0
    if peak < 1e-8:
        peak = 1.0
    x = np.clip(x / peak, -1.0, 1.0)
    if prompt_duration_sec is not None:
        n_keep = int(round(prompt_duration_sec * sample_rate))
        x = x[:n_keep]
    return mu_law_encode(x, NUM_BITS)


def build_model(device: torch.device, dtype: torch.dtype) -> WaveNet:
    """Hyperparameters must match train.py."""
    return WaveNet(
        num_embeddings=NUM_CLASSES,
        embedding_channels=32,
        hidden_channels=32,
        layers_per_stack=10,
        num_stacks=3,
        kernel_size=3,
    ).to(device=device, dtype=dtype)


def sample_from_logits(logits: torch.Tensor, temperature: float) -> int:
    """logits shape (NUM_CLASSES,) or (1, NUM_CLASSES)."""
    logits = logits.reshape(-1).float()
    if temperature < 1e-8:
        return int(torch.argmax(logits).item())
    p = torch.softmax(logits / temperature, dim=-1)
    return int(torch.multinomial(p, num_samples=1).item())


@torch.no_grad()
def generate(
    model: WaveNet,
    device: torch.device,
    dtype: torch.dtype,
    num_samples: int,
    temperature: float,
    prompt_tokens: np.ndarray | None,
) -> np.ndarray:
    """
    Batch size 1 autoregressive sampling. Ring-buffer WaveNet state is advanced one timestep at a time.
    Returns mu-law indices as int64 array of shape (num_samples,).
    """
    model.eval()
    model.reset_state(batch_size=1, dtype=dtype, device=device)

    generated: list[int] = []

    def forward_one(token: int) -> torch.Tensor:
        inp = torch.tensor([[token]], device=device, dtype=torch.long)
        out = model(inp)
        return out[0, :, -1]

    last_logits: torch.Tensor | None = None

    if prompt_tokens is not None and len(prompt_tokens) > 0:
        for t in tqdm(prompt_tokens.tolist(), desc="Priming prompt", leave=False):
            last_logits = forward_one(int(t))
    else:
        last_logits = forward_one(START_TOKEN)

    with tqdm(total=num_samples, desc="Generating") as bar:
        while len(generated) < num_samples:
            tok = sample_from_logits(last_logits, temperature)
            generated.append(tok)
            last_logits = forward_one(tok)
            bar.update(1)

    return np.asarray(generated, dtype=np.int64)


def main() -> None:
    p = argparse.ArgumentParser(description="WaveNet unconditional / prompted inference (3 kHz mu-law)")
    p.add_argument("checkpoint", type=str, help="Path to model .pt checkpoint (state dict)")
    p.add_argument(
        "--temperature",
        type=float,
        default=1.0,
        help="Softmax temperature for sampling (default: 1.0)",
    )
    p.add_argument(
        "--seconds",
        type=float,
        default=1.0,
        help="How many seconds of audio to generate at %d Hz (default: 1.0)" % SAMPLE_RATE,
    )
    p.add_argument("--prompt", type=str, default=None, help="Optional WAV file to condition on")
    p.add_argument(
        "--prompt-duration",
        type=float,
        default=None,
        metavar="SEC",
        help="Use only the first SEC seconds of the prompt file (requires --prompt)",
    )
    p.add_argument("--output", "-o", type=str, default="output.wav", help="Output WAV path")
    p.add_argument(
        "--device",
        type=str,
        choices=("cpu", "cuda"),
        default="cpu",
        help="Computation device (default: cpu). Use cuda for GPU inference.",
    )
    args = p.parse_args()

    if args.prompt_duration is not None and not args.prompt:
        p.error("--prompt-duration requires --prompt")

    if args.device == "cuda" and not torch.cuda.is_available():
        p.error("CUDA requested (--device cuda) but torch.cuda.is_available() is False")

    num_samples = max(1, int(round(args.seconds * SAMPLE_RATE)))

    prompt_tokens = None
    if args.prompt:
        prompt_tokens = load_prompt_tokens(args.prompt, args.prompt_duration, SAMPLE_RATE)
        if prompt_tokens.size == 0:
            prompt_tokens = None

    device = torch.device(args.device)
    dtype = torch.float32

    model = build_model(device, dtype)
    state = torch.load(args.checkpoint, map_location=device)
    model.load_state_dict(state)

    print(f"Loaded model from {args.checkpoint}")
    model.store_to_binary()
    print("Saved model weights to wavenet_weights.bin")

    indices = generate(
        model=model,
        device=device,
        dtype=dtype,
        num_samples=num_samples,
        temperature=args.temperature,
        prompt_tokens=prompt_tokens,
    )

    parts: list[np.ndarray] = []
    if prompt_tokens is not None and prompt_tokens.size > 0:
        parts.append(mu_law_decode(prompt_tokens.astype(np.int64), NUM_BITS))
    parts.append(mu_law_decode(indices, NUM_BITS))
    audio = np.concatenate(parts).astype(np.float32)

    sf.write(args.output, audio, SAMPLE_RATE, subtype="PCM_16")
    gen_sec = len(indices) / SAMPLE_RATE
    print(
        f"Wrote {args.output} ({len(audio)} samples, {len(audio) / SAMPLE_RATE:.3f} s total; "
        f"{gen_sec:.3f} s newly generated)"
    )


if __name__ == "__main__":
    main()
