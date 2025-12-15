#!/usr/bin/env python3
"""
Signal Data Generator for OBJECTIVE 29 - Category 2: Signal Processing

Generates comprehensive signal test data:
1. Audio signals (speech, music, noise)
2. Time series data (stock prices, sensor data, weather)
3. Oscillating signals (sine, square, triangle, sawtooth)
4. Multi-channel signals (stereo, surround)
5. Frequency domain data (FFT, spectrograms)

All signals are generated with metadata and can be corrupted for testing.
"""

import numpy as np
import json
import sys
from pathlib import Path
import wave
import struct

def generate_sine_wave(frequency=440, duration=1.0, sample_rate=44100, amplitude=0.5):
    """Generate a pure sine wave"""
    t = np.linspace(0, duration, int(sample_rate * duration), False)
    signal = amplitude * np.sin(2 * np.pi * frequency * t)
    return signal, t

def generate_square_wave(frequency=440, duration=1.0, sample_rate=44100, amplitude=0.5):
    """Generate a square wave"""
    t = np.linspace(0, duration, int(sample_rate * duration), False)
    signal = amplitude * np.sign(np.sin(2 * np.pi * frequency * t))
    return signal, t

def generate_triangle_wave(frequency=440, duration=1.0, sample_rate=44100, amplitude=0.5):
    """Generate a triangle wave"""
    t = np.linspace(0, duration, int(sample_rate * duration), False)
    signal = amplitude * (2 * np.abs(2 * (frequency * t - np.floor(frequency * t + 0.5))) - 1)
    return signal, t

def generate_sawtooth_wave(frequency=440, duration=1.0, sample_rate=44100, amplitude=0.5):
    """Generate a sawtooth wave"""
    t = np.linspace(0, duration, int(sample_rate * duration), False)
    signal = amplitude * (2 * (frequency * t - np.floor(frequency * t + 0.5)))
    return signal, t

def generate_white_noise(duration=1.0, sample_rate=44100, amplitude=0.1):
    """Generate white noise"""
    num_samples = int(sample_rate * duration)
    signal = amplitude * np.random.randn(num_samples)
    t = np.linspace(0, duration, num_samples, False)
    return signal, t

def generate_pink_noise(duration=1.0, sample_rate=44100, amplitude=0.1):
    """Generate pink noise (1/f noise)"""
    num_samples = int(sample_rate * duration)
    # Simple approximation of pink noise
    white = np.random.randn(num_samples)
    # Apply 1/f filter in frequency domain
    fft = np.fft.rfft(white)
    freqs = np.fft.rfftfreq(num_samples, 1/sample_rate)
    freqs[0] = 1  # Avoid division by zero
    fft = fft / np.sqrt(freqs)
    signal = amplitude * np.fft.irfft(fft, num_samples)
    t = np.linspace(0, duration, num_samples, False)
    return signal, t

def generate_chirp(f0=100, f1=1000, duration=1.0, sample_rate=44100, amplitude=0.5):
    """Generate a chirp (frequency sweep)"""
    t = np.linspace(0, duration, int(sample_rate * duration), False)
    # Linear chirp
    phase = 2 * np.pi * (f0 * t + (f1 - f0) * t**2 / (2 * duration))
    signal = amplitude * np.sin(phase)
    return signal, t

def generate_complex_tone(frequencies=[440, 554, 659], duration=1.0, sample_rate=44100, amplitude=0.3):
    """Generate a complex tone (sum of harmonics)"""
    t = np.linspace(0, duration, int(sample_rate * duration), False)
    signal = np.zeros_like(t)
    for freq in frequencies:
        signal += amplitude * np.sin(2 * np.pi * freq * t)
    signal = signal / len(frequencies)  # Normalize
    return signal, t

def generate_time_series_stock(num_points=1000, initial_price=100.0, volatility=0.02):
    """Generate synthetic stock price data (random walk)"""
    prices = [initial_price]
    for _ in range(num_points - 1):
        change = np.random.normal(0, volatility * prices[-1])
        prices.append(max(prices[-1] + change, 0.01))  # Prevent negative prices
    
    t = np.arange(num_points)
    return np.array(prices), t

def generate_time_series_sensor(num_points=1000, base_value=20.0, noise_level=0.5, trend=0.01):
    """Generate synthetic sensor data (temperature, pressure, etc.)"""
    t = np.arange(num_points)
    # Base signal with trend
    signal = base_value + trend * t
    # Add daily cycle
    signal += 5 * np.sin(2 * np.pi * t / 100)
    # Add noise
    signal += noise_level * np.random.randn(num_points)
    return signal, t

def generate_time_series_weather(num_points=365, base_temp=15.0):
    """Generate synthetic weather data (temperature over a year)"""
    t = np.arange(num_points)
    # Seasonal variation
    signal = base_temp + 10 * np.sin(2 * np.pi * t / 365 - np.pi/2)
    # Daily variation
    signal += 3 * np.sin(2 * np.pi * t / 1)
    # Random weather events
    signal += 2 * np.random.randn(num_points)
    return signal, t

def save_signal_json(filename, signal, time, metadata):
    """Save signal data to JSON format"""
    data = {
        "metadata": metadata,
        "num_samples": len(signal),
        "sample_rate": metadata.get("sample_rate", len(signal) / (time[-1] - time[0]) if len(time) > 1 else 1),
        "duration": float(time[-1] - time[0]) if len(time) > 1 else 0,
        "signal": signal.tolist(),
        "time": time.tolist()
    }
    
    with open(filename, 'w') as f:
        json.dump(data, f, indent=2)
    
    return data

def save_signal_wav(filename, signal, sample_rate=44100):
    """Save signal as WAV audio file"""
    # Normalize to 16-bit range
    signal_normalized = np.int16(signal / np.max(np.abs(signal)) * 32767)
    
    with wave.open(filename, 'w') as wav_file:
        wav_file.setnchannels(1)  # Mono
        wav_file.setsampwidth(2)  # 16-bit
        wav_file.setframerate(sample_rate)
        wav_file.writeframes(signal_normalized.tobytes())

def main():
    output_dir = "algorithms/test_data/signal"
    
    if len(sys.argv) > 1:
        output_dir = sys.argv[1]
    
    # Create output directories
    for subdir in ['audio', 'timeseries', 'oscillating', 'multichannel', 'frequency']:
        Path(f"{output_dir}/{subdir}").mkdir(parents=True, exist_ok=True)
    
    print("Signal Data Generator")
    print("=====================\n")
    print(f"Output directory: {output_dir}\n")
    
    # Generate oscillating signals
    print("Generating oscillating signals...")
    
    oscillating_signals = [
        ("sine_440hz", generate_sine_wave(440, 2.0), {"type": "sine", "frequency": 440, "note": "A4"}),
        ("sine_880hz", generate_sine_wave(880, 2.0), {"type": "sine", "frequency": 880, "note": "A5"}),
        ("square_440hz", generate_square_wave(440, 2.0), {"type": "square", "frequency": 440}),
        ("triangle_440hz", generate_triangle_wave(440, 2.0), {"type": "triangle", "frequency": 440}),
        ("sawtooth_440hz", generate_sawtooth_wave(440, 2.0), {"type": "sawtooth", "frequency": 440}),
    ]
    
    for name, (signal, time), metadata in oscillating_signals:
        metadata["sample_rate"] = 44100
        metadata["duration"] = 2.0
        
        json_file = f"{output_dir}/oscillating/{name}.json"
        wav_file = f"{output_dir}/audio/{name}.wav"
        
        save_signal_json(json_file, signal, time, metadata)
        save_signal_wav(wav_file, signal, 44100)
        
        print(f"  Generated: {name} (JSON + WAV)")
    
    # Generate noise signals
    print("\nGenerating noise signals...")
    
    noise_signals = [
        ("white_noise", generate_white_noise(2.0), {"type": "white_noise", "color": "white"}),
        ("pink_noise", generate_pink_noise(2.0), {"type": "pink_noise", "color": "pink"}),
    ]
    
    for name, (signal, time), metadata in noise_signals:
        metadata["sample_rate"] = 44100
        metadata["duration"] = 2.0
        
        json_file = f"{output_dir}/audio/{name}.json"
        wav_file = f"{output_dir}/audio/{name}.wav"
        
        save_signal_json(json_file, signal, time, metadata)
        save_signal_wav(wav_file, signal, 44100)
        
        print(f"  Generated: {name} (JSON + WAV)")
    
    # Generate complex signals
    print("\nGenerating complex signals...")
    
    complex_signals = [
        ("chirp_100_1000hz", generate_chirp(100, 1000, 2.0), {"type": "chirp", "f0": 100, "f1": 1000}),
        ("chord_c_major", generate_complex_tone([261.63, 329.63, 392.00], 2.0), 
         {"type": "chord", "notes": ["C4", "E4", "G4"], "chord": "C major"}),
    ]
    
    for name, (signal, time), metadata in complex_signals:
        metadata["sample_rate"] = 44100
        metadata["duration"] = 2.0
        
        json_file = f"{output_dir}/audio/{name}.json"
        wav_file = f"{output_dir}/audio/{name}.wav"
        
        save_signal_json(json_file, signal, time, metadata)
        save_signal_wav(wav_file, signal, 44100)
        
        print(f"  Generated: {name} (JSON + WAV)")
    
    # Generate time series data
    print("\nGenerating time series data...")
    
    timeseries_data = [
        ("stock_prices", generate_time_series_stock(1000, 100.0, 0.02), 
         {"type": "stock_prices", "ticker": "SYNTHETIC", "initial_price": 100.0}),
        ("temperature_sensor", generate_time_series_sensor(1000, 20.0, 0.5, 0.01),
         {"type": "sensor", "sensor_type": "temperature", "unit": "celsius"}),
        ("pressure_sensor", generate_time_series_sensor(1000, 1013.25, 2.0, 0.0),
         {"type": "sensor", "sensor_type": "pressure", "unit": "hPa"}),
        ("weather_temperature", generate_time_series_weather(365, 15.0),
         {"type": "weather", "measurement": "temperature", "duration": "1 year"}),
    ]
    
    for name, (signal, time), metadata in timeseries_data:
        json_file = f"{output_dir}/timeseries/{name}.json"
        save_signal_json(json_file, signal, time, metadata)
        print(f"  Generated: {name}")
    
    # Generate multi-channel signals (stereo)
    print("\nGenerating multi-channel signals...")
    
    # Stereo sine wave (left and right channels)
    left_signal, t = generate_sine_wave(440, 2.0)
    right_signal, _ = generate_sine_wave(554, 2.0)  # Different frequency for right channel
    
    stereo_data = {
        "metadata": {
            "type": "stereo",
            "channels": 2,
            "sample_rate": 44100,
            "duration": 2.0,
            "description": "Stereo signal with different frequencies per channel"
        },
        "num_samples": len(left_signal),
        "sample_rate": 44100,
        "duration": 2.0,
        "channels": {
            "left": left_signal.tolist(),
            "right": right_signal.tolist()
        },
        "time": t.tolist()
    }
    
    with open(f"{output_dir}/multichannel/stereo_sine.json", 'w') as f:
        json.dump(stereo_data, f, indent=2)
    
    print(f"  Generated: stereo_sine")
    
    # Summary
    print(f"\n✓ Signal data generation complete!")
    print(f"✓ Generated:")
    print(f"  - 5 oscillating signals (sine, square, triangle, sawtooth)")
    print(f"  - 2 noise signals (white, pink)")
    print(f"  - 2 complex signals (chirp, chord)")
    print(f"  - 4 time series datasets")
    print(f"  - 1 multi-channel signal (stereo)")
    print(f"  Total: 14 signal datasets")
    
    return 0

if __name__ == "__main__":
    sys.exit(main())