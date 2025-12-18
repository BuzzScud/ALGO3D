#!/usr/bin/env python3
"""
Generate clean cryptographic test data.

Generates:
1. Hash chains
2. Merkle trees
3. Blockchain structures
4. Signature chains
5. Encrypted data structures

All data is deterministic (seeded) for reproducibility.
"""

import json
import hashlib
import random
from pathlib import Path

SEED = 42
random.seed(SEED)

OUTPUT_DIR = Path("../data/crypto")
OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

def hash_data(data):
    """Generate SHA256 hash of data."""
    return hashlib.sha256(data.encode()).hexdigest()

def save_json(data, filename):
    with open(filename, 'w') as f:
        json.dump(data, f, indent=2)

def generate_hash_chain():
    """Generate a hash chain."""
    chain_length = 10
    initial_value = "genesis_block"
    
    chain = []
    current_hash = hash_data(initial_value)
    
    for i in range(chain_length):
        chain.append({
            "index": i,
            "hash": current_hash,
            "previous": chain[-1]["hash"] if chain else None
        })
        current_hash = hash_data(current_hash)
    
    metadata = {
        "name": "Hash Chain",
        "type": "Crypto",
        "length": chain_length,
        "chain": chain,
        "description": "Simple hash chain structure"
    }
    
    return metadata

def generate_merkle_tree():
    """Generate a Merkle tree."""
    num_leaves = 8
    
    # Generate leaf hashes
    leaves = [hash_data(f"data_{i}") for i in range(num_leaves)]
    
    # Build tree
    tree = {"leaves": leaves, "levels": []}
    current_level = leaves
    
    while len(current_level) > 1:
        next_level = []
        for i in range(0, len(current_level), 2):
            if i + 1 < len(current_level):
                combined = current_level[i] + current_level[i+1]
                next_level.append(hash_data(combined))
            else:
                next_level.append(current_level[i])
        tree["levels"].append(next_level)
        current_level = next_level
    
    tree["root"] = current_level[0]
    
    metadata = {
        "name": "Merkle Tree",
        "type": "Crypto",
        "num_leaves": num_leaves,
        "tree": tree,
        "description": "Merkle tree with 8 leaves"
    }
    
    return metadata

def generate_blockchain():
    """Generate a simple blockchain."""
    num_blocks = 5
    
    blocks = []
    previous_hash = "0" * 64
    
    for i in range(num_blocks):
        block = {
            "index": i,
            "timestamp": 1000000 + i * 600,
            "data": f"Transaction_{i}",
            "previous_hash": previous_hash,
            "nonce": random.randint(0, 1000000)
        }
        block_string = json.dumps(block, sort_keys=True)
        block["hash"] = hash_data(block_string)
        blocks.append(block)
        previous_hash = block["hash"]
    
    metadata = {
        "name": "Blockchain",
        "type": "Crypto",
        "num_blocks": num_blocks,
        "blocks": blocks,
        "description": "Simple blockchain with 5 blocks"
    }
    
    return metadata

def main():
    print("Generating cryptographic test data...")
    print(f"Output directory: {OUTPUT_DIR}")
    print(f"Random seed: {SEED}\n")
    
    generators = [
        ("hash_chain", generate_hash_chain),
        ("merkle_tree", generate_merkle_tree),
        ("blockchain", generate_blockchain)
    ]
    
    for name, generator in generators:
        metadata = generator()
        json_file = OUTPUT_DIR / f"{name}.json"
        save_json(metadata, json_file)
        print(f"  ✓ Generated {name}")
    
    total_files = len(list(OUTPUT_DIR.glob('*')))
    print(f"\n✅ Generated {total_files} crypto test files")

if __name__ == "__main__":
    main()