#!/usr/bin/env python3
"""
Signal Corruption Tool

Applies various types of corruption to signal data:
- Dropout (missing samples)
- Additive noise (Gaussian, impulse)
- Amplitude distortion
- Phase distortion
- Frequency distortion
"""

import numpy as np
import json
import sys
from pathlib import Path
import copy

def corrupt_signal_dropout(signal, corruption_level):
    """Corrupt signal by randomly dropping samples"""
    corrupted = signal.copy()
    num_to_corrupt = int(len(signal) * corruption_level)
    
    indices = np.random.choice(len(signal), num_to_corrupt, replace=False)
    corrupted[indices] = 0  # Set to zero (dropout)
    
    return corrupted, sorted(indices.tolist())

def corrupt_signal_noise(signal, corruption_level, noise_type='gaussian'):
    """Add noise to signal"""
    corrupted = signal.copy()
    
    if noise_type == 'gaussian':
        # Gaussian noise proportional to signal amplitude
        noise_amplitude = corruption_level * np.std(signal)
        noise = np.random.normal(0, noise_amplitude, len(signal))
        corrupted += noise
    elif noise_type == 'impulse':
        # Impulse noise (salt and pepper)
        num_impulses = int(len(signal) * corruption_level)
        indices = np.random.choice(len(signal), num_impulses, replace=False)
        corrupted[indices] = np.random.choice([-1, 1], num_impulses) * np.max(np.abs(signal))
    
    return corrupted, []

def corrupt_signal_amplitude(signal, corruption_level):
    """Distort signal amplitude"""
    corrupted = signal.copy()
    
    # Random amplitude scaling for portions of the signal
    num_segments = int(10 * corruption_level)
    segment_length = len(signal) // num_segments if num_segments > 0 else len(signal)
    
    for i in range(num_segments):
        start = i * segment_length
        end = min((i + 1) * segment_length, len(signal))
        scale = 1.0 + np.random.uniform(-corruption_level, corruption_level)
        corrupted[start:end] *= scale
    
    return corrupted, []

def load_signal_json(filename):
    """Load signal from JSON file"""
    with open(filename, 'r') as f:
        data = json.load(f)
    return data

def save_corrupted_signal(filename, original_data, corrupted_signal, corruption_info):
    """Save corrupted signal to JSON"""
    data = copy.deepcopy(original_data)
    data['signal'] = corrupted_signal.tolist()
    data['corruption'] = corruption_info
    
    with open(filename, 'w') as f:
        json.dump(data, f, indent=2)

def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <input_json> [output_dir]")
        print(f"Example: {sys.argv[0]} signal.json corrupted/")
        return 1
    
    input_file = sys.argv[1]
    output_dir = sys.argv[2] if len(sys.argv) > 2 else "algorithms/test_data/signal/corrupted"
    
    # Create output directory
    Path(output_dir).mkdir(parents=True, exist_ok=True)
    
    # Load original signal
    original_data = load_signal_json(input_file)
    signal = np.array(original_data['signal'])
    
    print("Signal Corruption Tool")
    print("======================\n")
    print(f"Input: {input_file}")
    print(f"Signal length: {len(signal)} samples")
    print(f"Output directory: {output_dir}\n")
    
    # Corruption levels
    corruption_levels = [0.05, 0.10, 0.15, 0.20, 0.25]
    
    # Seed for reproducibility
    np.random.seed(42)
    
    base_name = Path(input_file).stem
    
    for level in corruption_levels:
        # Dropout corruption
        corrupted, indices = corrupt_signal_dropout(signal, level)
        
        corruption_info = {
            "type": "dropout",
            "level": level,
            "num_corrupted": len(indices),
            "corrupted_indices": indices[:100]  # Store first 100 indices
        }
        
        output_file = Path(output_dir) / f"{base_name}_dropout_{int(level*100)}.json"
        save_corrupted_signal(output_file, original_data, corrupted, corruption_info)
        
        print(f"Generated: {output_file.name} (dropout {int(level*100)}%, {len(indices)} samples)")
    
    print(f"\n✓ Generated {len(corruption_levels)} corrupted versions")
    
    return 0

if __name__ == "__main__":
    sys.exit(main())