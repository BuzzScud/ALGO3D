#!/usr/bin/env python3
"""
Signal Processing Examples
Demonstrates audio, radio, and time series signal recovery
"""

import sys
import os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))

from recovery.signal import (
    recover_audio,
    recover_radio_signal,
    recover_time_series,
    denoise_signal,
    resample_signal,
    calculate_snr
)
import numpy as np
import matplotlib
matplotlib.use('Agg')  # Non-interactive backend
import matplotlib.pyplot as plt

def example_1_audio_recovery():
    """Example 1: Audio signal recovery"""
    print("=" * 60)
    print("Example 1: Audio Signal Recovery")
    print("=" * 60)
    
    # Generate clean sine wave (440 Hz A note)
    sample_rate = 44100
    duration = 1.0
    frequency = 440.0
    
    t = np.linspace(0, duration, int(sample_rate * duration), dtype=np.float32)
    clean_signal = np.sin(2 * np.pi * frequency * t).astype(np.float32)
    
    # Add noise to simulate corruption
    noise = np.random.normal(0, 0.1, clean_signal.shape).astype(np.float32)
    corrupted_signal = clean_signal + noise
    
    print(f"Sample rate: {sample_rate} Hz")
    print(f"Duration: {duration} s")
    print(f"Frequency: {frequency} Hz")
    print(f"Samples: {len(clean_signal)}")
    print()
    
    # Calculate SNR before recovery
    snr_before = calculate_snr(clean_signal, corrupted_signal)
    print(f"SNR before recovery: {snr_before:.2f} dB")
    
    # Recover audio
    print("Recovering audio signal...")
    recovered_signal = recover_audio(corrupted_signal, sample_rate, channels=1)
    
    # Calculate SNR after recovery
    snr_after = calculate_snr(clean_signal, recovered_signal)
    print(f"SNR after recovery:  {snr_after:.2f} dB")
    print(f"Improvement:         {snr_after - snr_before:.2f} dB")
    print()
    
    # Save plot
    plt.figure(figsize=(12, 8))
    
    plt.subplot(3, 1, 1)
    plt.plot(t[:1000], clean_signal[:1000], 'g-', label='Clean')
    plt.title('Clean Signal')
    plt.ylabel('Amplitude')
    plt.legend()
    plt.grid(True)
    
    plt.subplot(3, 1, 2)
    plt.plot(t[:1000], corrupted_signal[:1000], 'r-', label='Corrupted')
    plt.title('Corrupted Signal')
    plt.ylabel('Amplitude')
    plt.legend()
    plt.grid(True)
    
    plt.subplot(3, 1, 3)
    plt.plot(t[:1000], recovered_signal[:1000], 'b-', label='Recovered')
    plt.title('Recovered Signal')
    plt.xlabel('Time (s)')
    plt.ylabel('Amplitude')
    plt.legend()
    plt.grid(True)
    
    plt.tight_layout()
    plt.savefig('audio_recovery.png', dpi=150)
    print("Saved plot to: audio_recovery.png")
    print()

def example_2_radio_signal_recovery():
    """Example 2: Radio signal (IQ data) recovery"""
    print("=" * 60)
    print("Example 2: Radio Signal Recovery")
    print("=" * 60)
    
    # Generate clean IQ signal (complex baseband)
    sample_rate = 2000000  # 2 MHz
    duration = 0.001  # 1 ms
    center_freq = 100e6  # 100 MHz
    
    t = np.linspace(0, duration, int(sample_rate * duration), dtype=np.float32)
    
    # Generate QPSK-like signal
    symbols = np.random.choice([1+1j, 1-1j, -1+1j, -1-1j], len(t))
    clean_signal = symbols.astype(np.complex64)
    
    # Convert to interleaved I/Q format
    clean_iq = np.empty(len(clean_signal) * 2, dtype=np.float32)
    clean_iq[0::2] = clean_signal.real
    clean_iq[1::2] = clean_signal.imag
    
    # Add noise
    noise = np.random.normal(0, 0.2, clean_iq.shape).astype(np.float32)
    corrupted_iq = clean_iq + noise
    
    print(f"Sample rate: {sample_rate / 1e6:.1f} MHz")
    print(f"Center frequency: {center_freq / 1e6:.1f} MHz")
    print(f"Duration: {duration * 1000:.1f} ms")
    print(f"IQ samples: {len(clean_iq) // 2}")
    print()
    
    # Calculate SNR before recovery
    snr_before = calculate_snr(clean_iq, corrupted_iq)
    print(f"SNR before recovery: {snr_before:.2f} dB")
    
    # Recover radio signal
    print("Recovering radio signal...")
    recovered_iq = recover_radio_signal(corrupted_iq, sample_rate, center_freq)
    
    # Calculate SNR after recovery
    snr_after = calculate_snr(clean_iq, recovered_iq)
    print(f"SNR after recovery:  {snr_after:.2f} dB")
    print(f"Improvement:         {snr_after - snr_before:.2f} dB")
    print()
    
    # Plot constellation diagram
    plt.figure(figsize=(12, 4))
    
    plt.subplot(1, 3, 1)
    plt.scatter(clean_iq[0::2][:1000], clean_iq[1::2][:1000], alpha=0.5, s=1)
    plt.title('Clean Signal Constellation')
    plt.xlabel('I')
    plt.ylabel('Q')
    plt.grid(True)
    plt.axis('equal')
    
    plt.subplot(1, 3, 2)
    plt.scatter(corrupted_iq[0::2][:1000], corrupted_iq[1::2][:1000], alpha=0.5, s=1)
    plt.title('Corrupted Signal Constellation')
    plt.xlabel('I')
    plt.ylabel('Q')
    plt.grid(True)
    plt.axis('equal')
    
    plt.subplot(1, 3, 3)
    plt.scatter(recovered_iq[0::2][:1000], recovered_iq[1::2][:1000], alpha=0.5, s=1)
    plt.title('Recovered Signal Constellation')
    plt.xlabel('I')
    plt.ylabel('Q')
    plt.grid(True)
    plt.axis('equal')
    
    plt.tight_layout()
    plt.savefig('radio_recovery.png', dpi=150)
    print("Saved plot to: radio_recovery.png")
    print()

def example_3_time_series_recovery():
    """Example 3: Time series data recovery"""
    print("=" * 60)
    print("Example 3: Time Series Recovery")
    print("=" * 60)
    
    # Generate clean time series (e.g., stock prices, sensor data)
    sample_interval = 1.0  # 1 second between samples
    num_samples = 1000
    
    t = np.arange(num_samples) * sample_interval
    
    # Generate synthetic time series with trend and seasonality
    trend = 0.01 * t
    seasonal = 5 * np.sin(2 * np.pi * t / 100)
    noise_clean = np.random.normal(0, 0.5, num_samples)
    clean_series = trend + seasonal + noise_clean
    
    # Add corruption (missing data, outliers)
    corrupted_series = clean_series.copy()
    
    # Add missing data (NaN)
    missing_indices = np.random.choice(num_samples, size=50, replace=False)
    corrupted_series[missing_indices] = np.nan
    
    # Add outliers
    outlier_indices = np.random.choice(num_samples, size=20, replace=False)
    corrupted_series[outlier_indices] += np.random.normal(0, 10, 20)
    
    print(f"Sample interval: {sample_interval} s")
    print(f"Number of samples: {num_samples}")
    print(f"Missing data points: {len(missing_indices)}")
    print(f"Outliers: {len(outlier_indices)}")
    print()
    
    # Replace NaN with interpolated values for recovery
    mask = ~np.isnan(corrupted_series)
    corrupted_series_filled = np.interp(
        np.arange(num_samples),
        np.arange(num_samples)[mask],
        corrupted_series[mask]
    )
    
    # Calculate error before recovery
    error_before = np.sqrt(np.mean((corrupted_series_filled - clean_series) ** 2))
    print(f"RMSE before recovery: {error_before:.4f}")
    
    # Recover time series
    print("Recovering time series...")
    recovered_series = recover_time_series(corrupted_series_filled, sample_interval)
    
    # Calculate error after recovery
    error_after = np.sqrt(np.mean((recovered_series - clean_series) ** 2))
    print(f"RMSE after recovery:  {error_after:.4f}")
    print(f"Improvement:          {error_before - error_after:.4f}")
    print()
    
    # Plot
    plt.figure(figsize=(12, 8))
    
    plt.subplot(3, 1, 1)
    plt.plot(t, clean_series, 'g-', label='Clean', linewidth=1)
    plt.title('Clean Time Series')
    plt.ylabel('Value')
    plt.legend()
    plt.grid(True)
    
    plt.subplot(3, 1, 2)
    plt.plot(t, corrupted_series, 'r.', label='Corrupted', markersize=2)
    plt.title('Corrupted Time Series (with missing data and outliers)')
    plt.ylabel('Value')
    plt.legend()
    plt.grid(True)
    
    plt.subplot(3, 1, 3)
    plt.plot(t, recovered_series, 'b-', label='Recovered', linewidth=1)
    plt.title('Recovered Time Series')
    plt.xlabel('Time (s)')
    plt.ylabel('Value')
    plt.legend()
    plt.grid(True)
    
    plt.tight_layout()
    plt.savefig('timeseries_recovery.png', dpi=150)
    print("Saved plot to: timeseries_recovery.png")
    print()

def example_4_signal_denoising():
    """Example 4: Signal denoising"""
    print("=" * 60)
    print("Example 4: Signal Denoising")
    print("=" * 60)
    
    # Generate clean signal
    sample_rate = 44100
    duration = 0.1
    t = np.linspace(0, duration, int(sample_rate * duration), dtype=np.float32)
    
    # Multi-frequency signal
    clean_signal = (
        np.sin(2 * np.pi * 440 * t) +
        0.5 * np.sin(2 * np.pi * 880 * t) +
        0.25 * np.sin(2 * np.pi * 1320 * t)
    ).astype(np.float32)
    
    # Add different levels of noise
    noise_levels = [0.1, 0.3, 0.5]
    thresholds = [0.05, 0.15, 0.25]
    
    print(f"Sample rate: {sample_rate} Hz")
    print(f"Duration: {duration} s")
    print()
    
    for noise_level, threshold in zip(noise_levels, thresholds):
        noise = np.random.normal(0, noise_level, clean_signal.shape).astype(np.float32)
        noisy_signal = clean_signal + noise
        
        # Calculate SNR before denoising
        snr_before = calculate_snr(clean_signal, noisy_signal)
        
        # Denoise
        denoised_signal = denoise_signal(noisy_signal, threshold)
        
        # Calculate SNR after denoising
        snr_after = calculate_snr(clean_signal, denoised_signal)
        
        print(f"Noise level: {noise_level:.2f}")
        print(f"  Threshold: {threshold:.2f}")
        print(f"  SNR before: {snr_before:.2f} dB")
        print(f"  SNR after:  {snr_after:.2f} dB")
        print(f"  Improvement: {snr_after - snr_before:.2f} dB")
        print()

def example_5_signal_resampling():
    """Example 5: Signal resampling"""
    print("=" * 60)
    print("Example 5: Signal Resampling")
    print("=" * 60)
    
    # Generate signal at 44.1 kHz
    original_rate = 44100
    duration = 0.01
    frequency = 440.0
    
    t = np.linspace(0, duration, int(original_rate * duration), dtype=np.float32)
    signal = np.sin(2 * np.pi * frequency * t).astype(np.float32)
    
    print(f"Original sample rate: {original_rate} Hz")
    print(f"Original samples: {len(signal)}")
    print()
    
    # Resample to different rates
    target_rates = [22050, 48000, 96000]
    
    for target_rate in target_rates:
        resampled = resample_signal(signal, original_rate, target_rate)
        
        expected_samples = int(len(signal) * target_rate / original_rate)
        
        print(f"Target rate: {target_rate} Hz")
        print(f"  Expected samples: {expected_samples}")
        print(f"  Actual samples: {len(resampled)}")
        print(f"  Ratio: {len(resampled) / len(signal):.4f}")
        print()

def main():
    """Run all examples"""
    print("\n" + "=" * 60)
    print("Signal Processing Examples")
    print("=" * 60 + "\n")
    
    try:
        example_1_audio_recovery()
        example_2_radio_signal_recovery()
        example_3_time_series_recovery()
        example_4_signal_denoising()
        example_5_signal_resampling()
        
        print("=" * 60)
        print("All examples completed successfully!")
        print("=" * 60)
        print()
        print("Generated plots:")
        print("  - audio_recovery.png")
        print("  - radio_recovery.png")
        print("  - timeseries_recovery.png")
        print("=" * 60)
        
    except Exception as e:
        print(f"\n❌ Error: {e}")
        import traceback
        traceback.print_exc()
        return 1
    
    return 0

if __name__ == "__main__":
    sys.exit(main())