#!/usr/bin/env python3
"""
Generate Platonic solid demonstration data.

Generates educational demonstrations for:
1. Euler's formula (V - E + F = 2)
2. Symmetry groups
3. Dual relationships
4. Golden ratio
5. Sphere packing

All data is deterministic (seeded) for reproducibility.
"""

import json
import math
from pathlib import Path

SEED = 42

OUTPUT_DIR = Path("../data/platonic_demo")
OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

def save_json(data, filename):
    with open(filename, 'w') as f:
        json.dump(data, f, indent=2)

def generate_euler_demo():
    """Generate Euler's formula demonstration."""
    solids = [
        {"name": "Tetrahedron", "V": 4, "E": 6, "F": 4, "euler": 2},
        {"name": "Cube", "V": 8, "E": 12, "F": 6, "euler": 2},
        {"name": "Octahedron", "V": 6, "E": 12, "F": 8, "euler": 2},
        {"name": "Dodecahedron", "V": 20, "E": 30, "F": 12, "euler": 2},
        {"name": "Icosahedron", "V": 12, "E": 30, "F": 20, "euler": 2}
    ]
    
    metadata = {
        "name": "Euler's Formula Demonstration",
        "type": "Educational",
        "formula": "V - E + F = 2",
        "solids": solids,
        "description": "Demonstrates Euler's formula for all Platonic solids"
    }
    
    return metadata

def generate_symmetry_demo():
    """Generate symmetry groups demonstration."""
    symmetries = [
        {"solid": "Tetrahedron", "group": "Td", "order": 24, "rotations": 12, "reflections": 12},
        {"solid": "Cube", "group": "Oh", "order": 48, "rotations": 24, "reflections": 24},
        {"solid": "Octahedron", "group": "Oh", "order": 48, "rotations": 24, "reflections": 24},
        {"solid": "Dodecahedron", "group": "Ih", "order": 120, "rotations": 60, "reflections": 60},
        {"solid": "Icosahedron", "group": "Ih", "order": 120, "rotations": 60, "reflections": 60}
    ]
    
    metadata = {
        "name": "Symmetry Groups Demonstration",
        "type": "Educational",
        "symmetries": symmetries,
        "description": "Symmetry groups of Platonic solids"
    }
    
    return metadata

def generate_duals_demo():
    """Generate dual relationships demonstration."""
    duals = [
        {"solid": "Tetrahedron", "dual": "Tetrahedron", "self_dual": True},
        {"solid": "Cube", "dual": "Octahedron", "self_dual": False},
        {"solid": "Octahedron", "dual": "Cube", "self_dual": False},
        {"solid": "Dodecahedron", "dual": "Icosahedron", "self_dual": False},
        {"solid": "Icosahedron", "dual": "Dodecahedron", "self_dual": False}
    ]
    
    metadata = {
        "name": "Dual Relationships Demonstration",
        "type": "Educational",
        "duals": duals,
        "description": "Dual relationships between Platonic solids"
    }
    
    return metadata

def generate_golden_ratio_demo():
    """Generate golden ratio demonstration."""
    phi = (1 + math.sqrt(5)) / 2
    
    golden_ratio_solids = [
        {
            "solid": "Dodecahedron",
            "uses_phi": True,
            "coordinates": "Vertices involve φ",
            "edge_ratio": phi
        },
        {
            "solid": "Icosahedron",
            "uses_phi": True,
            "coordinates": "Vertices involve φ",
            "edge_ratio": phi
        }
    ]
    
    metadata = {
        "name": "Golden Ratio Demonstration",
        "type": "Educational",
        "phi": phi,
        "solids": golden_ratio_solids,
        "description": "Golden ratio in Platonic solids"
    }
    
    return metadata

def generate_sphere_packing_demo():
    """Generate sphere packing demonstration."""
    packing = [
        {"solid": "Tetrahedron", "packing_efficiency": 0.367, "kissing_number": 4},
        {"solid": "Cube", "packing_efficiency": 0.524, "kissing_number": 6},
        {"solid": "Octahedron", "packing_efficiency": 0.605, "kissing_number": 8},
        {"solid": "Dodecahedron", "packing_efficiency": 0.740, "kissing_number": 12},
        {"solid": "Icosahedron", "packing_efficiency": 0.605, "kissing_number": 12}
    ]
    
    metadata = {
        "name": "Sphere Packing Demonstration",
        "type": "Educational",
        "packing": packing,
        "description": "Sphere packing efficiency for Platonic solids"
    }
    
    return metadata

def main():
    print("Generating Platonic demonstration data...")
    print(f"Output directory: {OUTPUT_DIR}")
    print()
    
    generators = [
        ("euler_formula", generate_euler_demo),
        ("symmetry_groups", generate_symmetry_demo),
        ("dual_relationships", generate_duals_demo),
        ("golden_ratio", generate_golden_ratio_demo),
        ("sphere_packing", generate_sphere_packing_demo)
    ]
    
    for name, generator in generators:
        metadata = generator()
        json_file = OUTPUT_DIR / f"{name}.json"
        save_json(metadata, json_file)
        print(f"  ✓ Generated {name}")
    
    total_files = len(list(OUTPUT_DIR.glob('*')))
    print(f"\n✅ Generated {total_files} Platonic demonstration files")

if __name__ == "__main__":
    main()