#!/usr/bin/env python3
"""
Corrupted Data Generator

Takes a Platonic solid JSON file and generates corrupted versions
with specified corruption levels (5%, 10%, 15%, 20%, 25%)

Corruption types:
- Vertex displacement (random noise added to coordinates)
"""

import json
import sys
import random
import os
from pathlib import Path

def corrupt_vertices(data, corruption_level):
    """Corrupt vertices by adding random noise"""
    num_vertices = data['num_vertices']
    num_to_corrupt = int(num_vertices * corruption_level)
    
    # Randomly select vertices to corrupt
    indices = list(range(num_vertices))
    random.shuffle(indices)
    corrupted_indices = sorted(indices[:num_to_corrupt])
    
    # Add noise to selected vertices
    noise_scale = 0.5  # 50% of coordinate range
    for idx in corrupted_indices:
        data['vertices'][idx]['x'] += random.uniform(-noise_scale, noise_scale)
        data['vertices'][idx]['y'] += random.uniform(-noise_scale, noise_scale)
        data['vertices'][idx]['z'] += random.uniform(-noise_scale, noise_scale)
    
    return corrupted_indices

def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <input_json> [output_dir]")
        print(f"Example: {sys.argv[0]} tetrahedron.json corrupted/")
        return 1
    
    input_file = sys.argv[1]
    output_dir = sys.argv[2] if len(sys.argv) > 2 else "algorithms/test_data/geometric/corrupted"
    
    # Create output directory if it doesn't exist
    Path(output_dir).mkdir(parents=True, exist_ok=True)
    
    # Read input data
    with open(input_file, 'r') as f:
        original_data = json.load(f)
    
    print("Corrupted Data Generator")
    print("========================\n")
    print(f"Input: {input_file} ({original_data['name']})")
    print(f"Vertices: {original_data['num_vertices']}, "
          f"Edges: {original_data['num_edges']}, "
          f"Faces: {original_data['num_faces']}\n")
    
    # Generate corrupted versions at different levels
    corruption_levels = [0.05, 0.10, 0.15, 0.20, 0.25]
    
    # Seed random number generator for reproducibility
    random.seed(42)
    
    for level in corruption_levels:
        # Make a deep copy of the original data
        import copy
        corrupted_data = copy.deepcopy(original_data)
        
        # Corrupt the data
        corrupted_indices = corrupt_vertices(corrupted_data, level)
        
        # Add corruption metadata
        corrupted_data['corruption_level'] = level
        corrupted_data['num_corrupted'] = len(corrupted_indices)
        corrupted_data['corrupted_indices'] = corrupted_indices
        
        # Generate output filename
        base_name = Path(input_file).stem
        output_file = Path(output_dir) / f"{base_name}_corrupted_{int(level*100)}.json"
        
        # Write corrupted data
        with open(output_file, 'w') as f:
            json.dump(corrupted_data, f, indent=2)
        
        print(f"Generated: {output_file} ({int(level*100)}% corruption, "
              f"{len(corrupted_indices)} vertices affected)")
    
    print("\n✓ All corruption levels generated successfully!")
    return 0

if __name__ == "__main__":
    sys.exit(main())