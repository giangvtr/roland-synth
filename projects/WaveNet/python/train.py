from datetime import datetime
import os
import wave

import numpy as np
import torch
from torch.utils.tensorboard import SummaryWriter

from wavenet import WaveNet


data_dir = '/home/epd/git/data/maestro-2017-3kHz-8bit-mulaw'
files_in = [f for f in os.listdir(data_dir) if f.endswith('.wav')]

x = []
for f in files_in:
    path = os.path.join(data_dir, f)
    with wave.open(path, "rb") as wf:
        n_frames = wf.getnframes()
        n_channels = wf.getnchannels()
        if n_channels != 1:
            raise ValueError(f"Expected mono audio, but got {n_channels} channels in file {f}.")
        raw_bytes = wf.readframes(n_frames)
    xi = np.frombuffer(raw_bytes, dtype=np.uint8)
    x.append(xi)

# stack all audio in to a single long array
x = np.concatenate(x, axis=0)


num_classes = 256
embedding_channels = 32
hidden_channels = 32
layers_per_stack = 10
num_stacks = 3


model = WaveNet(
    num_embeddings=num_classes,
    embedding_channels=embedding_channels,
    hidden_channels=hidden_channels,
    layers_per_stack=layers_per_stack,
    num_stacks=num_stacks,
    kernel_size=3,
)

dtype = torch.float32
device = 'cuda'

x = torch.from_numpy(x).long().to(device)
model = model.to(dtype=dtype, device=device)

criterion = torch.nn.CrossEntropyLoss()
optimizer = torch.optim.Adam(model.parameters(), lr=3e-4)

segment_length = 2**14
batch_size = 6
num_segments = x.shape[0] // segment_length
num_samples = num_segments * segment_length
x = x[:num_samples]
offset = num_samples // batch_size

timenow = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
writer = SummaryWriter(log_dir=f'runs/wavenet_experiment/{timenow}')

idx_segment = 0
iteration = 0
model.reset_state(batch_size=batch_size, dtype=dtype, device=device)
while True:
    X = []
    for i in range(batch_size):
        start = (i * offset + idx_segment * segment_length) % num_samples
        idx = (start + torch.arange(segment_length)) % num_samples
        X.append(x[idx])
    X = torch.stack(X, dim=0)
    X_hat = model(X)
    loss = criterion(X_hat[:, :, :-1], X[:, 1:])

    loss.backward()
    optimizer.step()
    optimizer.zero_grad(set_to_none=True)
    model.detach_state()

    print(f"Iteration {iteration}, Loss: {loss.item():.4f}")

    writer.add_scalar('Loss/train', loss.item(), iteration)

    # save checkpoint every 10000 iterations
    if iteration % 10000 == 0:
        checkpoint_path = f'checkpoints/{timenow}/wavenet_checkpoint_{iteration}.pt'
        os.makedirs(os.path.dirname(checkpoint_path), exist_ok=True)
        torch.save(model.state_dict(), checkpoint_path)
        print(f"Saved checkpoint: {checkpoint_path}")

    idx_segment = (idx_segment + 1) % num_segments
    iteration += 1
