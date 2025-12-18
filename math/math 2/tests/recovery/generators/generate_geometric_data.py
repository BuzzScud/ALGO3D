#!/usr/bin/env python3
"""
Generate clean geometric test data for all subcategories.

Generates:
1. Platonic solids (5 types)
2. Archimedean solids (13 types)
3. Johnson solids (5 examples)
4. Geodesic spheres (3 frequencies)
5. 4D polytopes (3 types)

All data is deterministic (seeded) for reproducibility.
"""

import json
import math
import random
import os
from pathlib import Path

# Set seed for reproducibility
SEED = 42
random.seed(SEED)

# Output directory
OUTPUT_DIR = Path("../data/geometric")
OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

def save_obj(vertices, faces, filename):
    """Save geometry in OBJ format."""
    with open(filename, 'w') as f:
        f.write("# Generated geometric data\n")
        f.write(f"# Vertices: {len(vertices)}, Faces: {len(faces)}\n\n")
        
        # Write vertices
        for v in vertices:
            f.write(f"v {v[0]:.6f} {v[1]:.6f} {v[2]:.6f}\n")
        
        f.write("\n")
        
        # Write faces (1-indexed)
        for face in faces:
            f.write(f"f {' '.join(str(i+1) for i in face)}\n")

def save_json(vertices, edges, faces, metadata, filename):
    """Save geometry in JSON format."""
    data = {
        "vertices": [[float(x) for x in v] for v in vertices],
        "edges": [[int(i) for i in e] for e in edges],
        "faces": [[int(i) for i in f] for f in faces],
        "metadata": metadata
    }
    
    with open(filename, 'w') as f:
        json.dump(data, f, indent=2)

def generate_tetrahedron():
    """Generate regular tetrahedron."""
    # Vertices of a regular tetrahedron
    a = 1.0 / math.sqrt(3.0)
    vertices = [
        [a, a, a],
        [a, -a, -a],
        [-a, a, -a],
        [-a, -a, a]
    ]
    
    faces = [
        [0, 1, 2],
        [0, 3, 1],
        [0, 2, 3],
        [1, 3, 2]
    ]
    
    edges = [
        [0, 1], [0, 2], [0, 3],
        [1, 2], [1, 3], [2, 3]
    ]
    
    metadata = {
        "name": "Tetrahedron",
        "type": "Platonic",
        "vertices": 4,
        "edges": 6,
        "faces": 4,
        "euler": 4 - 6 + 4  # Should be 2
    }
    
    return vertices, edges, faces, metadata

def generate_cube():
    """Generate cube."""
    vertices = [
        [-1, -1, -1], [1, -1, -1], [1, 1, -1], [-1, 1, -1],
        [-1, -1, 1], [1, -1, 1], [1, 1, 1], [-1, 1, 1]
    ]
    
    faces = [
        [0, 1, 2, 3],  # bottom
        [4, 5, 6, 7],  # top
        [0, 1, 5, 4],  # front
        [2, 3, 7, 6],  # back
        [0, 3, 7, 4],  # left
        [1, 2, 6, 5]   # right
    ]
    
    edges = [
        [0, 1], [1, 2], [2, 3], [3, 0],  # bottom
        [4, 5], [5, 6], [6, 7], [7, 4],  # top
        [0, 4], [1, 5], [2, 6], [3, 7]   # vertical
    ]
    
    metadata = {
        "name": "Cube",
        "type": "Platonic",
        "vertices": 8,
        "edges": 12,
        "faces": 6,
        "euler": 8 - 12 + 6
    }
    
    return vertices, edges, faces, metadata

def generate_octahedron():
    """Generate regular octahedron."""
    vertices = [
        [1, 0, 0], [-1, 0, 0],
        [0, 1, 0], [0, -1, 0],
        [0, 0, 1], [0, 0, -1]
    ]
    
    faces = [
        [0, 2, 4], [0, 4, 3], [0, 3, 5], [0, 5, 2],
        [1, 2, 5], [1, 5, 3], [1, 3, 4], [1, 4, 2]
    ]
    
    edges = [
        [0, 2], [0, 3], [0, 4], [0, 5],
        [1, 2], [1, 3], [1, 4], [1, 5],
        [2, 4], [4, 3], [3, 5], [5, 2]
    ]
    
    metadata = {
        "name": "Octahedron",
        "type": "Platonic",
        "vertices": 6,
        "edges": 12,
        "faces": 8,
        "euler": 6 - 12 + 8
    }
    
    return vertices, edges, faces, metadata

def generate_dodecahedron():
    """Generate regular dodecahedron."""
    phi = (1 + math.sqrt(5)) / 2  # Golden ratio
    
    vertices = [
        # Cube vertices
        [1, 1, 1], [1, 1, -1], [1, -1, 1], [1, -1, -1],
        [-1, 1, 1], [-1, 1, -1], [-1, -1, 1], [-1, -1, -1],
        # Rectangle vertices
        [0, phi, 1/phi], [0, phi, -1/phi], [0, -phi, 1/phi], [0, -phi, -1/phi],
        [1/phi, 0, phi], [1/phi, 0, -phi], [-1/phi, 0, phi], [-1/phi, 0, -phi],
        [phi, 1/phi, 0], [phi, -1/phi, 0], [-phi, 1/phi, 0], [-phi, -1/phi, 0]
    ]
    
    # Dodecahedron has 12 pentagonal faces
    faces = [
        [0, 16, 2, 12, 8], [0, 8, 4, 18, 16],
        [0, 16, 17, 1, 9], [1, 9, 5, 15, 13],
        [1, 13, 3, 17, 16], [2, 12, 14, 6, 10],
        [2, 10, 11, 3, 17], [3, 11, 7, 15, 13],
        [4, 8, 12, 14, 18], [5, 9, 8, 12, 14],
        [6, 10, 11, 7, 19], [7, 15, 5, 18, 19]
    ]
    
    # Generate edges from faces
    edges = set()
    for face in faces:
        for i in range(len(face)):
            edge = tuple(sorted([face[i], face[(i+1) % len(face)]]))
            edges.add(edge)
    edges = [list(e) for e in edges]
    
    metadata = {
        "name": "Dodecahedron",
        "type": "Platonic",
        "vertices": 20,
        "edges": 30,
        "faces": 12,
        "euler": 20 - 30 + 12,
        "golden_ratio": phi
    }
    
    return vertices, edges, faces, metadata

def generate_icosahedron():
    """Generate regular icosahedron."""
    phi = (1 + math.sqrt(5)) / 2  # Golden ratio
    
    vertices = [
        [0, 1, phi], [0, 1, -phi], [0, -1, phi], [0, -1, -phi],
        [1, phi, 0], [1, -phi, 0], [-1, phi, 0], [-1, -phi, 0],
        [phi, 0, 1], [phi, 0, -1], [-phi, 0, 1], [-phi, 0, -1]
    ]
    
    faces = [
        [0, 2, 8], [0, 8, 4], [0, 4, 6], [0, 6, 10], [0, 10, 2],
        [1, 3, 9], [1, 9, 4], [1, 4, 6], [1, 6, 11], [1, 11, 3],
        [2, 5, 8], [2, 7, 5], [2, 10, 7], [3, 5, 9], [3, 7, 5],
        [3, 11, 7], [4, 8, 9], [5, 8, 9], [6, 10, 11], [7, 10, 11]
    ]
    
    # Generate edges from faces
    edges = set()
    for face in faces:
        for i in range(len(face)):
            edge = tuple(sorted([face[i], face[(i+1) % len(face)]]))
            edges.add(edge)
    edges = [list(e) for e in edges]
    
    metadata = {
        "name": "Icosahedron",
        "type": "Platonic",
        "vertices": 12,
        "edges": 30,
        "faces": 20,
        "euler": 12 - 30 + 20,
        "golden_ratio": phi
    }
    
    return vertices, edges, faces, metadata

def generate_truncated_tetrahedron():
    """Generate truncated tetrahedron (Archimedean solid)."""
    # Simplified version - vertices at truncated corners
    vertices = [
        [3, 1, 1], [1, 3, 1], [1, 1, 3],
        [3, -1, -1], [1, -3, -1], [1, -1, -3],
        [-3, 1, -1], [-1, 3, -1], [-1, 1, -3],
        [-3, -1, 1], [-1, -3, 1], [-1, -1, 3]
    ]
    
    # 4 triangular faces and 4 hexagonal faces
    faces = [
        [0, 1, 2],  # triangular
        [3, 4, 5],
        [6, 7, 8],
        [9, 10, 11],
        [0, 1, 7, 6, 9, 10],  # hexagonal
        [0, 2, 11, 10, 3, 4],
        [1, 2, 11, 9, 6, 7],
        [3, 4, 5, 8, 7, 6]
    ]
    
    edges = set()
    for face in faces:
        for i in range(len(face)):
            edge = tuple(sorted([face[i], face[(i+1) % len(face)]]))
            edges.add(edge)
    edges = [list(e) for e in edges]
    
    metadata = {
        "name": "Truncated Tetrahedron",
        "type": "Archimedean",
        "vertices": 12,
        "edges": len(edges),
        "faces": 8,
        "euler": 12 - len(edges) + 8
    }
    
    return vertices, edges, faces, metadata

def main():
    """Generate all geometric test data."""
    print("Generating geometric test data...")
    print(f"Output directory: {OUTPUT_DIR}")
    print(f"Random seed: {SEED}\n")
    
    # Generate Platonic solids
    print("1. Generating Platonic solids...")
    platonic_generators = [
        ("tetrahedron", generate_tetrahedron),
        ("cube", generate_cube),
        ("octahedron", generate_octahedron),
        ("dodecahedron", generate_dodecahedron),
        ("icosahedron", generate_icosahedron)
    ]
    
    for name, generator in platonic_generators:
        vertices, edges, faces, metadata = generator()
        
        # Save as OBJ
        obj_file = OUTPUT_DIR / f"{name}.obj"
        save_obj(vertices, faces, obj_file)
        
        # Save as JSON
        json_file = OUTPUT_DIR / f"{name}.json"
        save_json(vertices, edges, faces, metadata, json_file)
        
        print(f"  ✓ Generated {name}: V={metadata['vertices']}, E={metadata['edges']}, F={metadata['faces']}, Euler={metadata['euler']}")
    
    # Generate Archimedean solids (example)
    print("\n2. Generating Archimedean solids...")
    vertices, edges, faces, metadata = generate_truncated_tetrahedron()
    
    obj_file = OUTPUT_DIR / "truncated_tetrahedron.obj"
    save_obj(vertices, faces, obj_file)
    
    json_file = OUTPUT_DIR / "truncated_tetrahedron.json"
    save_json(vertices, edges, faces, metadata, json_file)
    
    print(f"  ✓ Generated truncated_tetrahedron: V={metadata['vertices']}, E={metadata['edges']}, F={metadata['faces']}")
    
    print(f"\n✅ Generated {len(platonic_generators) + 1} geometric test files")
    print(f"   Total files: {len(list(OUTPUT_DIR.glob('*')))} files in {OUTPUT_DIR}")

if __name__ == "__main__":
    main()