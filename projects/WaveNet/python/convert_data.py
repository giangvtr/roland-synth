import os
import tempfile
import wave

import numpy as np
from scipy.signal import resample
import soundfile as sf
from tqdm import tqdm


def mu_law_encode(x, num_bits):
    if np.any(x > 1.0) or np.any(x < -1.0):
        raise ValueError("Input signal must be in the range [-1, 1]")
    mu = 2 ** num_bits - 1
    # Apply mu-law companding
    y = np.sign(x) * np.log1p(mu * np.abs(x)) / np.log1p(mu)
    # Quantize to the specified number of bits
    y_quantized = np.round((y + 1) / 2 * mu).astype(np.uint8)
    return y_quantized


input_dir = '/home/epd/git/data/maestro-2017-downsampled'
output_dir = '/home/epd/git/data/maestro-2017-3kHz-8bit-mulaw'


num_bits = 8

files_in = [f for f in os.listdir(input_dir) if f.endswith('.wav')]

# create temp directory to store resampled files
max_val = 0
with tempfile.TemporaryDirectory() as temp_dir:
    for filename in tqdm(files_in, desc="Resampling audio files"):
        input_path = os.path.join(input_dir, filename)
        temp_path = os.path.join(temp_dir, filename)
        x, fs = sf.read(input_path)

        # Resample to 3000 Hz
        num_samples = int(round(x.shape[0]) * 3000 / fs)
        y = resample(x, num_samples)

        # read max value
        max_val = max(np.max(np.abs(y)), max_val)

        # Save the resampled audio to the temporary directory
        sf.write(temp_path, y, 3000, subtype='FLOAT')
    # do a second pass to normalize and quantize to 8-bit mu-law
    for filename in tqdm(files_in, desc="Quantizing audio files"):
        temp_path = os.path.join(temp_dir, filename)
        output_path = os.path.join(output_dir, filename)
        y, fs = sf.read(temp_path)

        # Normalize to the range [-1, 1]
        y_normalized = np.clip(y / max_val, -1.0, 1.0)

        # Quantize to 8-bit mu-law
        y_quantized = mu_law_encode(y_normalized, num_bits)

        # Save the quantized audio to the output directory
        with wave.open(output_path, 'wb') as wf:
            wf.setnchannels(1)
            wf.setsampwidth(1)
            wf.setframerate(3000)
            wf.writeframes(y_quantized.tobytes())
