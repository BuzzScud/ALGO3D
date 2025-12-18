#!/usr/bin/env python3
"""
Generate clean scientific test data.

Generates:
1. Molecular structures (simple molecules)
2. Crystal lattices
3. Quantum states
4. Simulation data
5. Experimental data

All data is deterministic (seeded) for reproducibility.
"""

import json
import random
import math
from pathlib import Path

SEED = 42
random.seed(SEED)

OUTPUT_DIR = Path("../data/scientific")
OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

def save_json(data, filename):
    with open(filename, 'w') as f:
        json.dump(data, f, indent=2)

def generate_molecule():
    """Generate a simple molecule (water H2O)."""
    atoms = [
        {"id": 0, "element": "O", "position": [0.0, 0.0, 0.0]},
        {"id": 1, "element": "H", "position": [0.96, 0.0, 0.0]},
        {"id": 2, "element": "H", "position": [-0.24, 0.93, 0.0]}
    ]
    
    bonds = [
        {"atom1": 0, "atom2": 1, "type": "single", "length": 0.96},
        {"atom1": 0, "atom2": 2, "type": "single", "length": 0.96}
    ]
    
    metadata = {
        "name": "Water Molecule",
        "type": "Molecular",
        "formula": "H2O",
        "num_atoms": 3,
        "num_bonds": 2,
        "atoms": atoms,
        "bonds": bonds,
        "description": "Simple water molecule"
    }
    
    return metadata

def generate_crystal():
    """Generate a simple crystal lattice (cubic)."""
    lattice_constant = 5.0
    num_cells = 3
    
    atoms = []
    atom_id = 0
    
    for i in range(num_cells):
        for j in range(num_cells):
            for k in range(num_cells):
                atoms.append({
                    "id": atom_id,
                    "element": "Na" if (i+j+k) % 2 == 0 else "Cl",
                    "position": [i * lattice_constant, j * lattice_constant, k * lattice_constant]
                })
                atom_id += 1
    
    metadata = {
        "name": "Crystal Lattice",
        "type": "Crystal",
        "structure": "cubic",
        "lattice_constant": lattice_constant,
        "num_atoms": len(atoms),
        "atoms": atoms,
        "description": "Simple cubic crystal lattice (NaCl-like)"
    }
    
    return metadata

def generate_quantum_state():
    """Generate a quantum state representation."""
    num_qubits = 3
    
    # Generate random quantum state amplitudes (normalized)
    amplitudes = []
    for i in range(2**num_qubits):
        real = random.uniform(-1, 1)
        imag = random.uniform(-1, 1)
        amplitudes.append({"real": real, "imag": imag})
    
    # Normalize
    norm = math.sqrt(sum(a["real"]**2 + a["imag"]**2 for a in amplitudes))
    for a in amplitudes:
        a["real"] /= norm
        a["imag"] /= norm
    
    metadata = {
        "name": "Quantum State",
        "type": "Quantum",
        "num_qubits": num_qubits,
        "dimension": 2**num_qubits,
        "amplitudes": amplitudes,
        "description": f"Quantum state of {num_qubits} qubits"
    }
    
    return metadata

def main():
    print("Generating scientific test data...")
    print(f"Output directory: {OUTPUT_DIR}")
    print(f"Random seed: {SEED}\n")
    
    generators = [
        ("water_molecule", generate_molecule),
        ("crystal_lattice", generate_crystal),
        ("quantum_state", generate_quantum_state)
    ]
    
    for name, generator in generators:
        metadata = generator()
        json_file = OUTPUT_DIR / f"{name}.json"
        save_json(metadata, json_file)
        print(f"  ✓ Generated {name}")
    
    total_files = len(list(OUTPUT_DIR.glob('*')))
    print(f"\n✅ Generated {total_files} scientific test files")

if __name__ == "__main__":
    main()