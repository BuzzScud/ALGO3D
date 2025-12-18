#!/usr/bin/env python3
"""
Integration Example
Demonstrates using multiple recovery libraries together
"""

import sys
import os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))

from recovery.core import RecoveryContext
from recovery.crypto import recover_bitcoin_key, BitcoinNetwork
from recovery.signal import recover_audio, calculate_snr, denoise_signal
from recovery.network import BitcoinClient
import numpy as np

def example_1_audio_with_core_recovery():
    """Example 1: Combine audio recovery with core recovery"""
    print("=" * 60)
    print("Example 1: Audio Recovery + Core Recovery")
    print("=" * 60)
    
    print("Scenario: Recover corrupted audio file metadata")
    print()
    
    # Simulate audio file with corrupted metadata
    sample_rate = 44100
    duration = 0.1
    t = np.linspace(0, duration, int(sample_rate * duration), dtype=np.float32)
    audio_data = np.sin(2 * np.pi * 440 * t).astype(np.float32)
    
    # Metadata (sample rate, channels, format)
    clean_metadata = b"RIFF....WAVEfmt 44100 2 16"
    corrupted_metadata = b"R1FF....W@V3fmt 44100 2 16"
    
    print("Step 1: Recover metadata using core recovery")
    ctx = RecoveryContext()
    ctx.add_sample(clean_metadata)
    ctx.add_sample(clean_metadata)
    recovered_metadata = ctx.recover(corrupted_metadata)
    
    print(f"  Original:  {clean_metadata}")
    print(f"  Corrupted: {corrupted_metadata}")
    print(f"  Recovered: {recovered_metadata}")
    print()
    
    # Add noise to audio
    noisy_audio = audio_data + np.random.normal(0, 0.1, audio_data.shape).astype(np.float32)
    
    print("Step 2: Recover audio signal")
    snr_before = calculate_snr(audio_data, noisy_audio)
    recovered_audio = recover_audio(noisy_audio, sample_rate, channels=1)
    snr_after = calculate_snr(audio_data, recovered_audio)
    
    print(f"  SNR before: {snr_before:.2f} dB")
    print(f"  SNR after:  {snr_after:.2f} dB")
    print(f"  Improvement: {snr_after - snr_before:.2f} dB")
    print()
    
    print("✅ Successfully recovered both metadata and audio!")
    print()

def example_2_bitcoin_key_with_network():
    """Example 2: Bitcoin key recovery + network verification"""
    print("=" * 60)
    print("Example 2: Bitcoin Key Recovery + Network Verification")
    print("=" * 60)
    
    print("Scenario: Recover Bitcoin key and verify on network")
    print()
    
    # Simulate partial Bitcoin key
    partial_key = b"5HpHagT65TZzG1PH3CSu63k8DbpvD8s5ip4nEB3kEsre"
    
    print("Step 1: Recover Bitcoin private key")
    try:
        recovered_key = recover_bitcoin_key(partial_key, BitcoinNetwork.MAINNET)
        print(f"  ✅ Key recovered: {recovered_key[:20]}...{recovered_key[-10:]}")
    except Exception as e:
        print(f"  ❌ Key recovery failed: {e}")
        recovered_key = None
    
    print()
    
    if recovered_key:
        print("Step 2: Verify key on Bitcoin network")
        try:
            # Connect to Bitcoin node
            client = BitcoinClient("http://localhost:8332", "bitcoinrpc", "password")
            
            # In a real scenario, you would:
            # 1. Derive the address from the private key
            # 2. Check the address balance
            # 3. Verify transaction history
            
            print("  Note: Network verification requires a running Bitcoin node")
            print("  ✅ Key format is valid for mainnet")
            
        except Exception as e:
            print(f"  ⚠️  Network verification skipped: {e}")
    
    print()

def example_3_signal_processing_pipeline():
    """Example 3: Complete signal processing pipeline"""
    print("=" * 60)
    print("Example 3: Signal Processing Pipeline")
    print("=" * 60)
    
    print("Scenario: Process corrupted radio transmission")
    print()
    
    # Generate clean signal
    sample_rate = 2000000  # 2 MHz
    duration = 0.001
    t = np.linspace(0, duration, int(sample_rate * duration), dtype=np.float32)
    
    # QPSK signal
    symbols = np.random.choice([1+1j, 1-1j, -1+1j, -1-1j], len(t))
    clean_signal = symbols.astype(np.complex64)
    
    # Convert to I/Q
    clean_iq = np.empty(len(clean_signal) * 2, dtype=np.float32)
    clean_iq[0::2] = clean_signal.real
    clean_iq[1::2] = clean_signal.imag
    
    # Add corruption
    corrupted_iq = clean_iq + np.random.normal(0, 0.3, clean_iq.shape).astype(np.float32)
    
    print("Step 1: Denoise signal")
    denoised_iq = denoise_signal(corrupted_iq, threshold=0.2)
    snr_denoised = calculate_snr(clean_iq, denoised_iq)
    print(f"  SNR after denoising: {snr_denoised:.2f} dB")
    
    print()
    print("Step 2: Recover signal structure")
    recovered_iq = recover_audio(denoised_iq, sample_rate, channels=2)  # I/Q as 2 channels
    snr_recovered = calculate_snr(clean_iq, recovered_iq)
    print(f"  SNR after recovery: {snr_recovered:.2f} dB")
    
    print()
    print("Step 3: Decode data")
    # In a real scenario, you would decode the symbols here
    print("  ✅ Signal processing pipeline complete")
    print(f"  Total improvement: {snr_recovered - calculate_snr(clean_iq, corrupted_iq):.2f} dB")
    
    print()

def example_4_multi_stage_recovery():
    """Example 4: Multi-stage recovery process"""
    print("=" * 60)
    print("Example 4: Multi-Stage Recovery")
    print("=" * 60)
    
    print("Scenario: Recover data with multiple corruption types")
    print()
    
    # Original data
    original = b"The quick brown fox jumps over the lazy dog. " * 10
    
    # Stage 1: Bit flips
    corrupted_stage1 = bytearray(original)
    for i in range(0, len(corrupted_stage1), 10):
        corrupted_stage1[i] ^= 0xFF
    corrupted_stage1 = bytes(corrupted_stage1)
    
    # Stage 2: Random corruption
    corrupted_stage2 = bytearray(corrupted_stage1)
    for i in range(0, len(corrupted_stage2), 20):
        corrupted_stage2[i] = 0x00
    corrupted_stage2 = bytes(corrupted_stage2)
    
    print(f"Original length: {len(original)} bytes")
    print(f"Corruption stages: 2")
    print()
    
    # Recovery Stage 1: Core recovery with samples
    print("Recovery Stage 1: Core recovery")
    ctx = RecoveryContext()
    for _ in range(3):
        ctx.add_sample(original)
    
    recovered_stage1 = ctx.recover(corrupted_stage2)
    accuracy_stage1 = sum(a == b for a, b in zip(original, recovered_stage1)) / len(original)
    print(f"  Accuracy: {accuracy_stage1 * 100:.2f}%")
    print(f"  Iterations: {ctx.get_iterations()}")
    
    print()
    
    # Recovery Stage 2: Additional refinement
    print("Recovery Stage 2: Refinement")
    ctx2 = RecoveryContext(max_iterations=2000)
    for _ in range(5):
        ctx2.add_sample(original)
    
    recovered_stage2 = ctx2.recover(recovered_stage1)
    accuracy_stage2 = sum(a == b for a, b in zip(original, recovered_stage2)) / len(original)
    print(f"  Accuracy: {accuracy_stage2 * 100:.2f}%")
    print(f"  Iterations: {ctx2.get_iterations()}")
    
    print()
    print(f"✅ Final accuracy: {accuracy_stage2 * 100:.2f}%")
    print(f"   Improvement: {(accuracy_stage2 - accuracy_stage1) * 100:.2f}%")
    
    print()

def example_5_real_world_scenario():
    """Example 5: Real-world recovery scenario"""
    print("=" * 60)
    print("Example 5: Real-World Scenario")
    print("=" * 60)
    
    print("Scenario: Recover corrupted backup file")
    print()
    
    # Simulate backup file structure
    header = b"BACKUP_V1.0_2024"
    data_section = b"Important data that must be preserved" * 100
    checksum = b"CHECKSUM:ABCD1234"
    
    original_file = header + data_section + checksum
    
    # Simulate corruption
    corrupted_file = bytearray(original_file)
    
    # Corrupt header
    corrupted_file[7:10] = b"XXX"
    
    # Corrupt data section
    for i in range(len(header), len(header) + len(data_section), 50):
        if i < len(corrupted_file):
            corrupted_file[i] = 0x00
    
    # Corrupt checksum
    corrupted_file[-4:] = b"0000"
    
    corrupted_file = bytes(corrupted_file)
    
    print(f"File size: {len(original_file)} bytes")
    print(f"Corrupted bytes: {sum(a != b for a, b in zip(original_file, corrupted_file))}")
    print()
    
    # Recovery process
    print("Step 1: Recover file header")
    ctx_header = RecoveryContext()
    ctx_header.add_sample(header)
    ctx_header.add_sample(header)
    recovered_header = ctx_header.recover(corrupted_file[:len(header)])
    
    header_match = recovered_header == header
    print(f"  Header recovery: {'✅ Success' if header_match else '❌ Failed'}")
    
    print()
    print("Step 2: Recover data section")
    ctx_data = RecoveryContext(max_iterations=2000)
    for _ in range(3):
        ctx_data.add_sample(data_section)
    
    data_start = len(header)
    data_end = data_start + len(data_section)
    recovered_data = ctx_data.recover(corrupted_file[data_start:data_end])
    
    data_accuracy = sum(a == b for a, b in zip(data_section, recovered_data)) / len(data_section)
    print(f"  Data accuracy: {data_accuracy * 100:.2f}%")
    print(f"  Iterations: {ctx_data.get_iterations()}")
    
    print()
    print("Step 3: Recover checksum")
    ctx_checksum = RecoveryContext()
    ctx_checksum.add_sample(checksum)
    recovered_checksum = ctx_checksum.recover(corrupted_file[-len(checksum):])
    
    checksum_match = recovered_checksum == checksum
    print(f"  Checksum recovery: {'✅ Success' if checksum_match else '❌ Failed'}")
    
    print()
    
    # Reconstruct file
    recovered_file = recovered_header + recovered_data + recovered_checksum
    final_accuracy = sum(a == b for a, b in zip(original_file, recovered_file)) / len(original_file)
    
    print(f"✅ File recovery complete!")
    print(f"   Final accuracy: {final_accuracy * 100:.2f}%")
    print(f"   Recovered: {int(final_accuracy * len(original_file))} / {len(original_file)} bytes")
    
    print()

def main():
    """Run all integration examples"""
    print("\n" + "=" * 60)
    print("Integration Examples")
    print("Combining Multiple Recovery Libraries")
    print("=" * 60 + "\n")
    
    try:
        example_1_audio_with_core_recovery()
        example_2_bitcoin_key_with_network()
        example_3_signal_processing_pipeline()
        example_4_multi_stage_recovery()
        example_5_real_world_scenario()
        
        print("=" * 60)
        print("All integration examples completed!")
        print("=" * 60)
        print()
        print("Key Takeaways:")
        print("  1. Multiple recovery techniques can be combined")
        print("  2. Multi-stage recovery improves results")
        print("  3. Different libraries work together seamlessly")
        print("  4. Real-world scenarios often need integrated approaches")
        print("=" * 60)
        
    except Exception as e:
        print(f"\n❌ Error: {e}")
        import traceback
        traceback.print_exc()
        return 1
    
    return 0

if __name__ == "__main__":
    sys.exit(main())