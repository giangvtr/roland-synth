import argparse
import math
import sys
import numpy as np
from scipy.io import wavfile
from scipy.signal import resample_poly

def normalize_audio(audio_data):
    """Converts audio data to floating point [-1.0, 1.0] if it isn't already."""
    if np.issubdtype(audio_data.dtype, np.floating):
        return audio_data.astype(np.float32)
    elif np.issubdtype(audio_data.dtype, np.integer):
        # Normalize based on the max value of the integer type
        max_val = np.iinfo(audio_data.dtype).max
        return audio_data.astype(np.float32) / max_val
    else:
        raise ValueError("Unsupported audio data type.")

def to_minimum_phase(ir):
    """
    Converts a given impulse response to minimum phase using the homomorphic 
    (complex cepstrum) method. This packs the energy towards the beginning of the IR.
    """
    # Pad to a large power of 2 to avoid time-aliasing during cepstral operations
    n_fft = 2 ** int(np.ceil(np.log2(len(ir) * 4)))
    
    # 1. Take the FFT
    H = np.fft.fft(ir, n_fft)
    
    # 2. Compute the log magnitude (clip at a small value to avoid log(0))
    log_mag = np.log(np.maximum(np.abs(H), 1e-15))
    
    # 3. Compute the real cepstrum via IFFT
    cepstrum = np.real(np.fft.ifft(log_mag))
    
    # 4. Construct the cepstral window (fold the negative time onto the positive time)
    w = np.zeros(n_fft)
    w[0] = 1.0
    if n_fft % 2 == 0:
        w[n_fft // 2] = 1.0
        w[1:n_fft // 2] = 2.0
    else:
        w[1:(n_fft + 1) // 2] = 2.0
        
    # 5. Apply window and go back to the frequency domain
    min_phase_spectrum = np.exp(np.fft.fft(cepstrum * w))
    
    # 6. Go back to the time domain to get the minimum phase IR
    ir_min_phase = np.real(np.fft.ifft(min_phase_spectrum))
    
    # Return to the original length (min phase IRs decay faster, so this is safe)
    return ir_min_phase[:len(ir)]

def resample_ir(ir, orig_sr, target_sr):
    """Resamples the impulse response using a polyphase filter."""
    if orig_sr == target_sr:
        return ir
    
    # Calculate up/down factors based on the greatest common divisor
    gcd = math.gcd(orig_sr, target_sr)
    up = target_sr // gcd
    down = orig_sr // gcd
    
    print(f"Resampling from {orig_sr}Hz to {target_sr}Hz...")
    return resample_poly(ir, up, down)

def apply_cosine_fadeout(ir, fade_percent=10.0):
    """Applies a cosine fade-out to the tail of the IR to ensure it ends exactly at zero."""
    length = len(ir)
    fade_len = int(length * (fade_percent / 100.0))
    
    if fade_len <= 0:
        fade_len = 1  # At least fade the very last sample if 0% is passed
        
    envelope = np.ones(length, dtype=np.float32)
    
    # Cosine curve from 1.0 down to 0.0
    fade_curve = np.cos(np.linspace(0, np.pi/2, fade_len))
    fade_curve[-1] = 0.0 # Guarantee exactly 0.0 on the last sample
    
    envelope[-fade_len:] = fade_curve
    return ir * envelope

def export_to_cpp(ir, output_filepath, array_name="ir_data"):
    """Writes the float array to a standard C++ header/source format."""
    with open(output_filepath, 'w') as f:
        f.write("#include <cstddef>\n\n")
        f.write(f"// Generated Minimum Phase Impulse Response\n")
        f.write(f"// Length: {len(ir)} samples\n")
        f.write(f"// Sample Rate: 48000 Hz\n\n")
        
        f.write(f"const float {array_name}[] = {{\n    ")
        
        for i, val in enumerate(ir):
            # Format float nicely, ensuring 'f' suffix for C++ floats
            f.write(f"{val:.9f}f")
            if i < len(ir) - 1:
                f.write(", ")
            # Line break every 8 values for readability
            if (i + 1) % 8 == 0:
                f.write("\n    ")
                
        f.write("\n};\n\n")
        f.write(f"const size_t {array_name}_length = {len(ir)};\n")
    print(f"Successfully wrote {len(ir)} samples to {output_filepath}")

def process_ir(input_wav, output_cpp, target_length, array_name="ir_data", fade_percent=10.0):
    # 1. Load the wav file
    try:
        sample_rate, data = wavfile.read(input_wav)
    except Exception as e:
        print(f"Error reading {input_wav}: {e}")
        sys.exit(1)
        
    # Convert to mono if it's stereo
    if data.ndim > 1:
        print("Stereo file detected. Converting to mono by averaging channels...")
        data = np.mean(data, axis=1)
        
    # Normalize to float
    ir = normalize_audio(data)
    
    # 2. Convert to minimum phase
    print("Converting to minimum phase...")
    ir_min_phase = to_minimum_phase(ir)
    
    # 3. Resample to 48kHz
    ir_resampled = resample_ir(ir_min_phase, sample_rate, 48000)
    
    # 4. Truncate to specified length
    print(f"Truncating to {target_length} samples...")
    if len(ir_resampled) > target_length:
        ir_final = ir_resampled[:target_length]
    else:
        # Pad with zeros if the IR is shorter than the requested length
        print(f"Warning: IR is shorter than {target_length} after resampling. Padding with zeros.")
        ir_final = np.zeros(target_length, dtype=np.float32)
        ir_final[:len(ir_resampled)] = ir_resampled
        
    # 5. Apply cosine fade-out
    print(f"Applying cosine fade-out over the tail ({fade_percent}% of the IR)...")
    ir_final = apply_cosine_fadeout(ir_final, fade_percent)
        
    # 6. Output to .cpp
    export_to_cpp(ir_final, output_cpp, array_name)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Process an IR WAV file into a minimum phase C++ array.")
    parser.add_argument("-i", "--input", required=True, help="Path to the input .wav file")
    parser.add_argument("-o", "--output", required=True, help="Path to the output .cpp file")
    parser.add_argument("-l", "--length", required=True, type=int, help="Target length in samples")
    parser.add_argument("-n", "--name", default="ir_data", help="Name of the C++ array (default: ir_data)")
    parser.add_argument("-f", "--fade", default=10.0, type=float, help="Fade-out percentage at the tail (default: 10.0)")
    
    args = parser.parse_args()
    
    process_ir(args.input, args.output, args.length, args.name, args.fade)