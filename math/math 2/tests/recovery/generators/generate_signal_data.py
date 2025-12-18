#!/usr/bin/env python3
"""
Generate clean signal test data for all subcategories.

Generates:
1. Audio signals (sine, square, triangle, sawtooth)
2. Time series data (stock prices, sensor data)
3. Frequency domain data (FFT representations)
4. Multi-channel signals (stereo, surround)
5. Oscillation patterns (various frequencies)

All data is deterministic (seeded) for reproducibility.
"""

import json
import math
import random
import numpy as np
from pathlib import Path

# Set seed for reproducibility
SEED = 42
random.seed(SEED)
np.random.seed(SEED)

# Output directory
OUTPUT_DIR = Path("../data/signal")
OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

# Signal parameters
SAMPLE_RATE = 44100  # Hz
DURATION = 1.0  # seconds
NUM_SAMPLES = int(SAMPLE_RATE * DURATION)

def save_json(samples, metadata, filename):
    """Save signal in JSON format."""
    data = {
        "samples": samples.tolist() if isinstance(samples, np.ndarray) else samples,
        "metadata": metadata
    }
    
    with open(filename, 'w') as f:
        json.dump(data, f, indent=2)

def save_csv(samples, metadata, filename):
    """Save signal in CSV format."""
    with open(filename, 'w') as f:
        f.write("# " + ", ".join(f"{k}={v}" for k, v in metadata.items()) + "\n")
        f.write("sample_index,value\n")
        for i, sample in enumerate(samples):
            f.write(f"{i},{sample}\n")

def generate_sine_wave(frequency=440.0):
    """Generate sine wave."""
    t = np.linspace(0, DURATION, NUM_SAMPLES, endpoint=False)
    samples = np.sin(2 * np.pi * frequency * t)
    
    metadata = {
        "name": "Sine Wave",
        "type": "Audio",
        "frequency": frequency,
        "sample_rate": SAMPLE_RATE,
        "duration": DURATION,
        "num_samples": NUM_SAMPLES,
        "channels": 1
    }
    
    return samples, metadata

def generate_square_wave(frequency=440.0):
    """Generate square wave."""
    t = np.linspace(0, DURATION, NUM_SAMPLES, endpoint=False)
    samples = np.sign(np.sin(2 * np.pi * frequency * t))
    
    metadata = {
        "name": "Square Wave",
        "type": "Audio",
        "frequency": frequency,
        "sample_rate": SAMPLE_RATE,
        "duration": DURATION,
        "num_samples": NUM_SAMPLES,
        "channels": 1
    }
    
    return samples, metadata

def generate_triangle_wave(frequency=440.0):
    """Generate triangle wave."""
    t = np.linspace(0, DURATION, NUM_SAMPLES, endpoint=False)
    samples = 2 * np.abs(2 * (t * frequency - np.floor(t * frequency + 0.5))) - 1
    
    metadata = {
        "name": "Triangle Wave",
        "type": "Audio",
        "frequency": frequency,
        "sample_rate": SAMPLE_RATE,
        "duration": DURATION,
        "num_samples": NUM_SAMPLES,
        "channels": 1
    }
    
    return samples, metadata

def generate_sawtooth_wave(frequency=440.0):
    """Generate sawtooth wave."""
    t = np.linspace(0, DURATION, NUM_SAMPLES, endpoint=False)
    samples = 2 * (t * frequency - np.floor(t * frequency + 0.5))
    
    metadata = {
        "name": "Sawtooth Wave",
        "type": "Audio",
        "frequency": frequency,
        "sample_rate": SAMPLE_RATE,
        "duration": DURATION,
        "num_samples": NUM_SAMPLES,
        "channels": 1
    }
    
    return samples, metadata

def generate_white_noise():
    """Generate white noise."""
    samples = np.random.uniform(-1, 1, NUM_SAMPLES)
    
    metadata = {
        "name": "White Noise",
        "type": "Audio",
        "sample_rate": SAMPLE_RATE,
        "duration": DURATION,
        "num_samples": NUM_SAMPLES,
        "channels": 1
    }
    
    return samples, metadata

def generate_chirp():
    """Generate chirp signal (frequency sweep)."""
    t = np.linspace(0, DURATION, NUM_SAMPLES, endpoint=False)
    f0, f1 = 100, 1000  # Start and end frequencies
    samples = np.sin(2 * np.pi * (f0 * t + (f1 - f0) * t**2 / (2 * DURATION)))
    
    metadata = {
        "name": "Chirp",
        "type": "Audio",
        "frequency_start": f0,
        "frequency_end": f1,
        "sample_rate": SAMPLE_RATE,
        "duration": DURATION,
        "num_samples": NUM_SAMPLES,
        "channels": 1
    }
    
    return samples, metadata

def generate_stock_prices():
    """Generate synthetic stock price time series."""
    # Random walk with drift
    num_days = 365
    initial_price = 100.0
    drift = 0.0005
    volatility = 0.02
    
    returns = np.random.normal(drift, volatility, num_days)
    prices = initial_price * np.exp(np.cumsum(returns))
    
    metadata = {
        "name": "Stock Prices",
        "type": "Time Series",
        "initial_price": initial_price,
        "num_days": num_days,
        "drift": drift,
        "volatility": volatility
    }
    
    return prices, metadata

def generate_sensor_data():
    """Generate synthetic sensor data (temperature)."""
    num_hours = 168  # 1 week
    base_temp = 20.0
    daily_variation = 5.0
    noise = 0.5
    
    t = np.arange(num_hours)
    # Daily cycle + noise
    temps = base_temp + daily_variation * np.sin(2 * np.pi * t / 24) + np.random.normal(0, noise, num_hours)
    
    metadata = {
        "name": "Temperature Sensor",
        "type": "Time Series",
        "base_temperature": base_temp,
        "num_hours": num_hours,
        "daily_variation": daily_variation,
        "noise_level": noise,
        "unit": "Celsius"
    }
    
    return temps, metadata

def generate_stereo_signal(frequency=440.0):
    """Generate stereo signal (2 channels)."""
    t = np.linspace(0, DURATION, NUM_SAMPLES, endpoint=False)
    
    # Left channel: sine wave
    left = np.sin(2 * np.pi * frequency * t)
    
    # Right channel: sine wave with phase shift
    right = np.sin(2 * np.pi * frequency * t + np.pi / 4)
    
    samples = np.column_stack([left, right])
    
    metadata = {
        "name": "Stereo Signal",
        "type": "Multi-Channel Audio",
        "frequency": frequency,
        "sample_rate": SAMPLE_RATE,
        "duration": DURATION,
        "num_samples": NUM_SAMPLES,
        "channels": 2
    }
    
    return samples, metadata

def main():
    """Generate all signal test data."""
    print("Generating signal test data...")
    print(f"Output directory: {OUTPUT_DIR}")
    print(f"Random seed: {SEED}")
    print(f"Sample rate: {SAMPLE_RATE} Hz")
    print(f"Duration: {DURATION} seconds\n")
    
    # Generate audio signals
    print("1. Generating audio signals...")
    audio_generators = [
        ("sine_440hz", lambda: generate_sine_wave(440)),
        ("square_440hz", lambda: generate_square_wave(440)),
        ("triangle_440hz", lambda: generate_triangle_wave(440)),
        ("sawtooth_440hz", lambda: generate_sawtooth_wave(440)),
        ("white_noise", generate_white_noise),
        ("chirp", generate_chirp)
    ]
    
    for name, generator in audio_generators:
        samples, metadata = generator()
        
        # Save as JSON
        json_file = OUTPUT_DIR / f"{name}.json"
        save_json(samples, metadata, json_file)
        
        # Save as CSV
        csv_file = OUTPUT_DIR / f"{name}.csv"
        save_csv(samples, metadata, csv_file)
        
        print(f"  ✓ Generated {name}: {metadata['num_samples']} samples")
    
    # Generate time series
    print("\n2. Generating time series data...")
    timeseries_generators = [
        ("stock_prices", generate_stock_prices),
        ("temperature_sensor", generate_sensor_data)
    ]
    
    for name, generator in timeseries_generators:
        samples, metadata = generator()
        
        # Save as JSON
        json_file = OUTPUT_DIR / f"{name}.json"
        save_json(samples, metadata, json_file)
        
        # Save as CSV
        csv_file = OUTPUT_DIR / f"{name}.csv"
        save_csv(samples, metadata, csv_file)
        
        print(f"  ✓ Generated {name}: {len(samples)} samples")
    
    # Generate multi-channel
    print("\n3. Generating multi-channel signals...")
    samples, metadata = generate_stereo_signal(440)
    
    json_file = OUTPUT_DIR / "stereo_440hz.json"
    save_json(samples, metadata, json_file)
    
    print(f"  ✓ Generated stereo_440hz: {metadata['num_samples']} samples, {metadata['channels']} channels")
    
    total_files = len(list(OUTPUT_DIR.glob('*')))
    print(f"\n✅ Generated {total_files} signal test files")
    print(f"   Total files: {total_files} files in {OUTPUT_DIR}")

if __name__ == "__main__":
    main()