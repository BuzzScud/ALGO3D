#!/usr/bin/env python3
"""
Network Operations Examples
Demonstrates Bitcoin network operations and blockchain interaction
"""

import sys
import os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))

from recovery.network import BitcoinClient, BitcoinNetwork
import time

def example_1_connect_to_node():
    """Example 1: Connect to Bitcoin node"""
    print("=" * 60)
    print("Example 1: Connect to Bitcoin Node")
    print("=" * 60)
    
    # Connection parameters
    rpc_url = "http://localhost:8332"  # Mainnet
    rpc_user = "bitcoinrpc"
    rpc_password = "your_password_here"
    
    print(f"Connecting to: {rpc_url}")
    print(f"Username: {rpc_user}")
    print()
    
    try:
        # Create client
        client = BitcoinClient(rpc_url, rpc_user, rpc_password)
        print("✅ Connected successfully!")
        print()
        
        # Note: Actual connection is tested when making first RPC call
        print("Note: Connection will be tested when making the first RPC call.")
        
    except Exception as e:
        print(f"❌ Connection failed: {e}")
        print()
        print("Make sure you have:")
        print("  1. Bitcoin Core running")
        print("  2. RPC server enabled in bitcoin.conf")
        print("  3. Correct RPC credentials")
    
    print()

def example_2_get_block_info():
    """Example 2: Get block information"""
    print("=" * 60)
    print("Example 2: Get Block Information")
    print("=" * 60)
    
    # Genesis block hash
    genesis_hash = "000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f"
    
    print(f"Querying block: {genesis_hash}")
    print()
    
    try:
        client = BitcoinClient("http://localhost:8332", "bitcoinrpc", "password")
        
        # Get block info
        block = client.get_block(genesis_hash)
        
        print("Block Information:")
        print(f"  Hash: {block['hash']}")
        print(f"  Height: {block['height']}")
        print(f"  Timestamp: {time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(block['time']))}")
        print(f"  Transactions: {block['tx_count']}")
        print()
        
    except Exception as e:
        print(f"❌ Failed to get block: {e}")
        print()
        print("This example requires a running Bitcoin node.")
    
    print()

def example_3_get_transaction_info():
    """Example 3: Get transaction information"""
    print("=" * 60)
    print("Example 3: Get Transaction Information")
    print("=" * 60)
    
    # First Bitcoin transaction (Satoshi to Hal Finney)
    tx_hash = "f4184fc596403b9d638783cf57adfe4c75c605f6356fbc91338530e9831e9e16"
    
    print(f"Querying transaction: {tx_hash}")
    print()
    
    try:
        client = BitcoinClient("http://localhost:8332", "bitcoinrpc", "password")
        
        # Get transaction info
        tx = client.get_transaction(tx_hash)
        
        print("Transaction Information:")
        print(f"  TXID: {tx['txid']}")
        print(f"  Confirmations: {tx['confirmations']}")
        print(f"  Size: {tx['size']} bytes")
        print()
        
    except Exception as e:
        print(f"❌ Failed to get transaction: {e}")
        print()
        print("This example requires a running Bitcoin node with txindex enabled.")
    
    print()

def example_4_check_address_balance():
    """Example 4: Check address balance"""
    print("=" * 60)
    print("Example 4: Check Address Balance")
    print("=" * 60)
    
    # Satoshi's address (Genesis block coinbase)
    address = "1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa"
    
    print(f"Checking balance for: {address}")
    print()
    
    try:
        client = BitcoinClient("http://localhost:8332", "bitcoinrpc", "password")
        
        # Get balance
        balance = client.get_balance(address)
        
        print(f"Balance: {balance:.8f} BTC")
        print()
        
        # Convert to satoshis
        satoshis = int(balance * 100000000)
        print(f"Balance: {satoshis:,} satoshis")
        print()
        
    except Exception as e:
        print(f"❌ Failed to get balance: {e}")
        print()
        print("This example requires a running Bitcoin node.")
    
    print()

def example_5_send_transaction():
    """Example 5: Send raw transaction"""
    print("=" * 60)
    print("Example 5: Send Raw Transaction")
    print("=" * 60)
    
    print("⚠️  WARNING: This example is for demonstration only!")
    print("    Do not send real transactions without proper testing.")
    print()
    
    # Example raw transaction (hex encoded)
    # This is a dummy transaction for demonstration
    raw_tx = "0100000001..." # Truncated for safety
    
    print("Raw transaction (truncated):")
    print(f"  {raw_tx[:50]}...")
    print()
    
    try:
        client = BitcoinClient("http://localhost:8332", "bitcoinrpc", "password")
        
        # Send transaction (commented out for safety)
        # txid = client.send_transaction(raw_tx)
        # print(f"✅ Transaction sent!")
        # print(f"   TXID: {txid}")
        
        print("Note: Transaction sending is disabled in this example for safety.")
        print("      Uncomment the code to actually send transactions.")
        
    except Exception as e:
        print(f"❌ Failed to send transaction: {e}")
    
    print()

def example_6_testnet_operations():
    """Example 6: Testnet operations"""
    print("=" * 60)
    print("Example 6: Testnet Operations")
    print("=" * 60)
    
    print("Bitcoin testnet is a separate blockchain for testing.")
    print("Testnet coins have no value and can be obtained from faucets.")
    print()
    
    # Testnet connection
    testnet_url = "http://localhost:18332"  # Testnet default port
    
    print(f"Testnet RPC URL: {testnet_url}")
    print()
    
    try:
        client = BitcoinClient(testnet_url, "bitcoinrpc", "password")
        
        print("✅ Connected to testnet node")
        print()
        print("Testnet is ideal for:")
        print("  - Testing applications")
        print("  - Learning Bitcoin development")
        print("  - Experimenting without risk")
        print("  - Integration testing")
        
    except Exception as e:
        print(f"❌ Connection failed: {e}")
        print()
        print("To use testnet:")
        print("  1. Run Bitcoin Core with -testnet flag")
        print("  2. Configure RPC in bitcoin.conf")
        print("  3. Get testnet coins from a faucet")
    
    print()

def example_7_regtest_operations():
    """Example 7: Regtest operations"""
    print("=" * 60)
    print("Example 7: Regtest Operations")
    print("=" * 60)
    
    print("Regtest (Regression Test) is a private blockchain for development.")
    print("You have complete control and can mine blocks instantly.")
    print()
    
    # Regtest connection
    regtest_url = "http://localhost:18443"  # Regtest default port
    
    print(f"Regtest RPC URL: {regtest_url}")
    print()
    
    try:
        client = BitcoinClient(regtest_url, "bitcoinrpc", "password")
        
        print("✅ Connected to regtest node")
        print()
        print("Regtest is ideal for:")
        print("  - Unit testing")
        print("  - Development")
        print("  - Automated testing")
        print("  - Instant block generation")
        print("  - Complete control over blockchain")
        
    except Exception as e:
        print(f"❌ Connection failed: {e}")
        print()
        print("To use regtest:")
        print("  1. Run Bitcoin Core with -regtest flag")
        print("  2. Configure RPC in bitcoin.conf")
        print("  3. Generate blocks with: bitcoin-cli -regtest generate 101")
    
    print()

def example_8_multi_network_comparison():
    """Example 8: Multi-network comparison"""
    print("=" * 60)
    print("Example 8: Multi-Network Comparison")
    print("=" * 60)
    
    networks = [
        ("Mainnet", "http://localhost:8332", "Real Bitcoin blockchain"),
        ("Testnet", "http://localhost:18332", "Public test blockchain"),
        ("Regtest", "http://localhost:18443", "Private development blockchain"),
    ]
    
    print("Bitcoin Network Comparison:")
    print()
    
    for name, url, description in networks:
        print(f"{name}:")
        print(f"  URL: {url}")
        print(f"  Description: {description}")
        print()
    
    print("Choosing the right network:")
    print("  - Mainnet: Production applications with real value")
    print("  - Testnet: Public testing and integration")
    print("  - Regtest: Development and unit testing")
    print()

def main():
    """Run all examples"""
    print("\n" + "=" * 60)
    print("Network Operations Examples")
    print("=" * 60 + "\n")
    
    print("⚠️  IMPORTANT NOTES:")
    print("   - These examples require a running Bitcoin node")
    print("   - Configure RPC credentials in bitcoin.conf")
    print("   - Use testnet or regtest for testing")
    print("   - Never expose RPC credentials")
    print()
    
    try:
        example_1_connect_to_node()
        example_2_get_block_info()
        example_3_get_transaction_info()
        example_4_check_address_balance()
        example_5_send_transaction()
        example_6_testnet_operations()
        example_7_regtest_operations()
        example_8_multi_network_comparison()
        
        print("=" * 60)
        print("All examples completed!")
        print("=" * 60)
        print()
        print("Next steps:")
        print("  1. Set up a Bitcoin node (mainnet, testnet, or regtest)")
        print("  2. Configure RPC credentials")
        print("  3. Run these examples with real connections")
        print("  4. Explore the Bitcoin RPC API documentation")
        print("=" * 60)
        
    except Exception as e:
        print(f"\n❌ Error: {e}")
        import traceback
        traceback.print_exc()
        return 1
    
    return 0

if __name__ == "__main__":
    sys.exit(main())