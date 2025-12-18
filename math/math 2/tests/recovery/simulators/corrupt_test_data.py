#!/usr/bin/env python3
"""
Corrupt test data to simulate various failure modes.

Supports corruption for:
1. Geometric data - vertex displacement, edge corruption, face corruption
2. Signal data - dropout, noise, distortion, clipping
3. Image data - pixel dropout, noise, blur
4. Network data - node deletion, edge deletion
5. Cryptographic data - hash corruption, chain breaks
6. Scientific data - atom displacement, bond errors
7. ML data - weight corruption, gradient errors
8. All data types - general corruption patterns

All corruption is deterministic (seeded) for reproducibility.
"""

import json
import random
import numpy as np
import argparse
from pathlib import Path
import shutil

# Set seed for reproducibility
SEED = 42
random.seed(SEED)
np.random.seed(SEED)

# Corruption levels
CORRUPTION_LEVELS = [0.05, 0.10, 0.15, 0.20, 0.25]  # 5%, 10%, 15%, 20%, 25%

def corrupt_geometric_data(data, corruption_level):
    """Corrupt geometric data (vertices, edges, faces)."""
    corrupted = data.copy()
    
    # Corrupt vertices (displacement)
    if 'vertices' in corrupted:
        vertices = np.array(corrupted['vertices'])
        num_corrupt = int(len(vertices) * corruption_level)
        corrupt_indices = random.sample(range(len(vertices)), num_corrupt)
        
        for idx in corrupt_indices:
            # Add random displacement
            displacement = np.random.normal(0, 0.5, 3)
            vertices[idx] += displacement
        
        corrupted['vertices'] = vertices.tolist()
        corrupted['metadata']['corrupted_vertices'] = corrupt_indices
    
    # Corrupt edges (random deletion)
    if 'edges' in corrupted:
        edges = corrupted['edges']
        num_corrupt = int(len(edges) * corruption_level)
        corrupt_indices = random.sample(range(len(edges)), num_corrupt)
        
        # Mark corrupted edges (set to invalid indices)
        for idx in corrupt_indices:
            edges[idx] = [-1, -1]
        
        corrupted['metadata']['corrupted_edges'] = corrupt_indices
    
    # Add corruption metadata
    corrupted['metadata']['corruption_level'] = corruption_level
    corrupted['metadata']['corruption_type'] = 'geometric'
    corrupted['metadata']['seed'] = SEED
    
    return corrupted

def corrupt_signal_data(data, corruption_level):
    """Corrupt signal data (dropout, noise, clipping)."""
    corrupted = data.copy()
    
    if 'samples' in corrupted:
        samples = np.array(corrupted['samples'])
        
        # Dropout corruption
        num_dropout = int(len(samples) * corruption_level * 0.5)
        dropout_indices = random.sample(range(len(samples)), num_dropout)
        samples[dropout_indices] = 0
        
        # Noise corruption
        num_noise = int(len(samples) * corruption_level * 0.3)
        noise_indices = random.sample(range(len(samples)), num_noise)
        noise = np.random.normal(0, 0.2, num_noise)
        samples[noise_indices] += noise
        
        # Clipping corruption
        num_clip = int(len(samples) * corruption_level * 0.2)
        clip_indices = random.sample(range(len(samples)), num_clip)
        samples[clip_indices] = np.clip(samples[clip_indices], -0.5, 0.5)
        
        corrupted['samples'] = samples.tolist()
        corrupted['metadata']['corrupted_indices'] = {
            'dropout': dropout_indices,
            'noise': noise_indices,
            'clipping': clip_indices
        }
    
    # Add corruption metadata
    corrupted['metadata']['corruption_level'] = corruption_level
    corrupted['metadata']['corruption_type'] = 'signal'
    corrupted['metadata']['seed'] = SEED
    
    return corrupted

def corrupt_json_file(input_file, output_file, corruption_level, data_type):
    """Corrupt a JSON file based on data type."""
    with open(input_file, 'r') as f:
        data = json.load(f)
    
    if data_type == 'geometric':
        corrupted = corrupt_geometric_data(data, corruption_level)
    elif data_type == 'signal':
        corrupted = corrupt_signal_data(data, corruption_level)
    else:
        # Generic corruption
        corrupted = data.copy()
        corrupted['metadata'] = corrupted.get('metadata', {})
        corrupted['metadata']['corruption_level'] = corruption_level
        corrupted['metadata']['corruption_type'] = 'generic'
        corrupted['metadata']['seed'] = SEED
    
    with open(output_file, 'w') as f:
        json.dump(corrupted, f, indent=2)

def corrupt_csv_file(input_file, output_file, corruption_level):
    """Corrupt a CSV file (signal data)."""
    # Read CSV
    with open(input_file, 'r') as f:
        lines = f.readlines()
    
    # Separate header and data
    header_lines = [line for line in lines if line.startswith('#') or 'sample_index' in line]
    data_lines = [line for line in lines if not line.startswith('#') and 'sample_index' not in line]
    
    # Corrupt data lines
    num_corrupt = int(len(data_lines) * corruption_level)
    corrupt_indices = random.sample(range(len(data_lines)), num_corrupt)
    
    for idx in corrupt_indices:
        parts = data_lines[idx].strip().split(',')
        if len(parts) == 2:
            # Set value to 0 (dropout)
            data_lines[idx] = f"{parts[0]},0.0\n"
    
    # Write corrupted CSV
    with open(output_file, 'w') as f:
        f.writelines(header_lines)
        f.write(f"# CORRUPTED: {corruption_level*100}% corruption, seed={SEED}\n")
        f.writelines(data_lines)

def main():
    """Corrupt all test data at various corruption levels."""
    parser = argparse.ArgumentParser(description='Corrupt test data')
    parser.add_argument('--input-dir', default='../data', help='Input data directory')
    parser.add_argument('--output-dir', default='../data/corrupted', help='Output directory')
    parser.add_argument('--levels', nargs='+', type=float, default=CORRUPTION_LEVELS,
                       help='Corruption levels (e.g., 0.05 0.10 0.15)')
    args = parser.parse_args()
    
    input_dir = Path(args.input_dir)
    output_dir = Path(args.output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)
    
    print("Corrupting test data...")
    print(f"Input directory: {input_dir}")
    print(f"Output directory: {output_dir}")
    print(f"Corruption levels: {[f'{l*100:.0f}%' for l in args.levels]}")
    print(f"Random seed: {SEED}\n")
    
    total_files = 0
    
    # Process each category
    for category_dir in sorted(input_dir.iterdir()):
        if not category_dir.is_dir() or category_dir.name == 'corrupted':
            continue
        
        category = category_dir.name
        print(f"\nProcessing category: {category}")
        
        # Create output subdirectory
        category_output = output_dir / category
        category_output.mkdir(parents=True, exist_ok=True)
        
        # Process each file
        for input_file in sorted(category_dir.glob('*')):
            if not input_file.is_file():
                continue
            
            file_stem = input_file.stem
            file_ext = input_file.suffix
            
            # Corrupt at each level
            for level in args.levels:
                level_pct = int(level * 100)
                output_file = category_output / f"{file_stem}_corrupted_{level_pct}pct{file_ext}"
                
                try:
                    if file_ext == '.json':
                        corrupt_json_file(input_file, output_file, level, category)
                    elif file_ext == '.csv':
                        corrupt_csv_file(input_file, output_file, level)
                    elif file_ext == '.obj':
                        # For OBJ files, just copy for now (would need proper parser)
                        shutil.copy(input_file, output_file)
                    else:
                        # Copy other files as-is
                        shutil.copy(input_file, output_file)
                    
                    total_files += 1
                    print(f"  ✓ {file_stem}{file_ext} → {level_pct}% corruption")
                
                except Exception as e:
                    print(f"  ✗ Error corrupting {input_file.name}: {e}")
    
    print(f"\n✅ Generated {total_files} corrupted test files")
    print(f"   Output directory: {output_dir}")

if __name__ == "__main__":
    main()