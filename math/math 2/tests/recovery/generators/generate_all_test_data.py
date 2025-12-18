#!/usr/bin/env python3
"""
Master script to generate all test data for all 8 categories.

Generates clean test data for:
1. Geometric data (Platonic, Archimedean, etc.)
2. Signal data (Audio, Time Series, etc.)
3. Image data (Natural, Medical, Synthetic)
4. Network data (Social, Computer, Biological)
5. Cryptographic data (Hash Chains, Merkle Trees)
6. Scientific data (Molecular, Crystal, Quantum)
7. ML data (Weights, Embeddings, Checkpoints)
8. Platonic demonstrations (Educational)

All data is deterministic (seeded) for reproducibility.
"""

import subprocess
import sys
from pathlib import Path

def run_generator(script_name):
    """Run a generator script."""
    script_path = Path(__file__).parent / script_name
    
    if not script_path.exists():
        print(f"  ⚠ Skipping {script_name} (not found)")
        return False
    
    try:
        result = subprocess.run(
            [sys.executable, str(script_path)],
            capture_output=True,
            text=True,
            check=True
        )
        print(result.stdout)
        return True
    except subprocess.CalledProcessError as e:
        print(f"  ✗ Error running {script_name}:")
        print(e.stderr)
        return False

def main():
    """Generate all test data."""
    print("="*80)
    print("GENERATING ALL TEST DATA FOR 8 CATEGORIES")
    print("="*80)
    print()
    
    generators = [
        ("generate_geometric_data.py", "Geometric Data"),
        ("generate_signal_data.py", "Signal Data"),
        ("generate_image_data.py", "Image Data"),
        ("generate_network_data.py", "Network Data"),
        ("generate_crypto_data.py", "Cryptographic Data"),
        ("generate_scientific_data.py", "Scientific Data"),
        ("generate_ml_data.py", "ML Data"),
        ("generate_platonic_demo_data.py", "Platonic Demonstrations")
    ]
    
    success_count = 0
    total_count = len(generators)
    
    for script, category in generators:
        print(f"\n{'='*80}")
        print(f"Category: {category}")
        print(f"Script: {script}")
        print(f"{'='*80}\n")
        
        if run_generator(script):
            success_count += 1
        else:
            print(f"  ⚠ {category} generation incomplete or skipped")
    
    print(f"\n{'='*80}")
    print(f"SUMMARY: {success_count}/{total_count} categories generated successfully")
    print(f"{'='*80}\n")
    
    # Count total files
    data_dir = Path(__file__).parent.parent / "data"
    if data_dir.exists():
        total_files = sum(1 for _ in data_dir.rglob('*') if _.is_file())
        print(f"Total test files generated: {total_files}")
        
        # Show breakdown by category
        print("\nBreakdown by category:")
        for subdir in sorted(data_dir.iterdir()):
            if subdir.is_dir():
                count = sum(1 for _ in subdir.rglob('*') if _.is_file())
                print(f"  {subdir.name}: {count} files")
    
    return success_count == total_count

if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 1)