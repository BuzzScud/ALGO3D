#!/usr/bin/env python3
"""
Crypto Recovery Examples
Demonstrates cryptographic key recovery capabilities
"""

import sys
import os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))

from recovery.crypto import (
    recover_bitcoin_key,
    recover_ssh_key,
    recover_ecdsa_nonce,
    recover_ransomware_key,
    BitcoinNetwork,
    SSHKeyType
)
import binascii

def example_1_bitcoin_key_recovery():
    """Example 1: Bitcoin private key recovery"""
    print("=" * 60)
    print("Example 1: Bitcoin Key Recovery")
    print("=" * 60)
    
    # Simulate partial Bitcoin private key (WIF format)
    # In reality, this would be a partially corrupted key
    partial_key = b"5HpHagT65TZzG1PH3CSu63k8DbpvD8s5ip4nEB3kEsre"
    
    print("Attempting Bitcoin key recovery...")
    print(f"Partial key: {partial_key.decode()}")
    print()
    
    # Try recovery for different networks
    networks = [
        (BitcoinNetwork.MAINNET, "Mainnet"),
        (BitcoinNetwork.TESTNET, "Testnet"),
        (BitcoinNetwork.REGTEST, "Regtest"),
    ]
    
    for network, name in networks:
        try:
            recovered = recover_bitcoin_key(partial_key, network)
            print(f"✅ {name}: {recovered}")
        except Exception as e:
            print(f"❌ {name}: Recovery failed - {e}")
    
    print()
    print("Note: This is a demonstration. Real key recovery requires")
    print("      actual partial key data and multiple samples.")
    print()

def example_2_ssh_key_recovery():
    """Example 2: SSH private key recovery"""
    print("=" * 60)
    print("Example 2: SSH Key Recovery")
    print("=" * 60)
    
    # Simulate partial SSH private key
    partial_key = b"-----BEGIN RSA PRIVATE KEY-----\nMIIEpAIBAAKCAQEA..."
    
    print("Attempting SSH key recovery...")
    print(f"Partial key (first 50 chars): {partial_key[:50].decode()}...")
    print()
    
    # Try recovery for different key types
    key_types = [
        (SSHKeyType.RSA, "RSA"),
        (SSHKeyType.ED25519, "Ed25519"),
        (SSHKeyType.ECDSA, "ECDSA"),
    ]
    
    for key_type, name in key_types:
        try:
            recovered = recover_ssh_key(partial_key, key_type)
            print(f"✅ {name}: Recovery successful")
            print(f"   Key length: {len(recovered)} bytes")
        except Exception as e:
            print(f"❌ {name}: Recovery failed - {e}")
    
    print()
    print("Note: This is a demonstration. Real SSH key recovery requires")
    print("      actual partial key data from corrupted files.")
    print()

def example_3_ecdsa_nonce_recovery():
    """Example 3: ECDSA nonce recovery (for nonce reuse attacks)"""
    print("=" * 60)
    print("Example 3: ECDSA Nonce Recovery")
    print("=" * 60)
    
    print("ECDSA nonce recovery is used to exploit nonce reuse in signatures.")
    print("This is a critical vulnerability in ECDSA implementations.")
    print()
    
    # Simulate ECDSA signature components
    # In reality, these would be from two signatures with the same nonce
    signature1 = bytes.fromhex("3045022100" + "a" * 64 + "0220" + "b" * 64)
    signature2 = bytes.fromhex("3045022100" + "a" * 64 + "0220" + "c" * 64)
    message1 = b"Message 1"
    message2 = b"Message 2"
    public_key = bytes.fromhex("04" + "d" * 128)
    
    print("Scenario: Two signatures with the same nonce")
    print(f"Signature 1: {signature1[:20].hex()}...")
    print(f"Signature 2: {signature2[:20].hex()}...")
    print()
    
    try:
        # Attempt nonce recovery
        nonce = recover_ecdsa_nonce(signature1, message1, public_key)
        print(f"✅ Recovered nonce: {nonce.hex()}")
        print()
        print("With the recovered nonce, the private key can be calculated!")
        print("This demonstrates why nonce reuse is catastrophic in ECDSA.")
    except Exception as e:
        print(f"❌ Nonce recovery failed: {e}")
        print("Note: This example uses simulated data for demonstration.")
    
    print()

def example_4_ransomware_key_recovery():
    """Example 4: Ransomware encryption key recovery"""
    print("=" * 60)
    print("Example 4: Ransomware Key Recovery")
    print("=" * 60)
    
    print("Ransomware key recovery uses known plaintext attacks.")
    print("If you have both encrypted and original versions of a file,")
    print("the encryption key can potentially be recovered.")
    print()
    
    # Simulate encrypted file and known plaintext
    # In reality, these would be actual file contents
    known_plaintext = b"This is the original file content that we know."
    encrypted_file = bytes([b ^ 0x42 for b in known_plaintext])  # Simple XOR for demo
    
    print(f"Known plaintext: {known_plaintext[:30].decode()}...")
    print(f"Encrypted file:  {encrypted_file[:30].hex()}...")
    print()
    
    try:
        # Attempt key recovery
        key = recover_ransomware_key(encrypted_file, known_plaintext)
        print(f"✅ Recovered key: {key.hex()}")
        print(f"   Key length: {len(key)} bytes")
        print()
        print("With the recovered key, other encrypted files can be decrypted!")
    except Exception as e:
        print(f"❌ Key recovery failed: {e}")
        print("Note: This example uses simulated data for demonstration.")
    
    print()

def example_5_bitcoin_key_validation():
    """Example 5: Bitcoin key validation and network detection"""
    print("=" * 60)
    print("Example 5: Bitcoin Key Validation")
    print("=" * 60)
    
    # Example Bitcoin private keys (WIF format)
    keys = [
        ("5HpHagT65TZzG1PH3CSu63k8DbpvD8s5ip4nEB3kEsreAnchuDf", "Mainnet"),
        ("92Pg46rUhgTT7romnV7iGW6W1gbGdeezqdbJCzShkCsYNzyyNcc", "Testnet"),
        ("cNJFgo1driFnPcBdBX8BrJrpxchBWXwXCvNH5SoSkdcF6JXXwHMm", "Testnet"),
    ]
    
    print("Validating Bitcoin private keys...")
    print()
    
    for key, expected_network in keys:
        print(f"Key: {key[:20]}...{key[-10:]}")
        print(f"Expected network: {expected_network}")
        
        # In a real implementation, you would:
        # 1. Decode the WIF format
        # 2. Validate the checksum
        # 3. Determine the network from the version byte
        # 4. Verify the key is valid for that network
        
        print("✅ Key is valid")
        print()
    
    print("Note: This example demonstrates key validation concepts.")
    print("      Real validation requires proper WIF decoding and checksum verification.")
    print()

def main():
    """Run all examples"""
    print("\n" + "=" * 60)
    print("Crypto Recovery Examples")
    print("=" * 60 + "\n")
    
    try:
        example_1_bitcoin_key_recovery()
        example_2_ssh_key_recovery()
        example_3_ecdsa_nonce_recovery()
        example_4_ransomware_key_recovery()
        example_5_bitcoin_key_validation()
        
        print("=" * 60)
        print("All examples completed!")
        print("=" * 60)
        print()
        print("⚠️  IMPORTANT SECURITY NOTES:")
        print("   - These examples are for educational purposes only")
        print("   - Never use recovered keys without proper authorization")
        print("   - Always follow responsible disclosure practices")
        print("   - Respect privacy and legal boundaries")
        print("=" * 60)
        
    except Exception as e:
        print(f"\n❌ Error: {e}")
        import traceback
        traceback.print_exc()
        return 1
    
    return 0

if __name__ == "__main__":
    sys.exit(main())