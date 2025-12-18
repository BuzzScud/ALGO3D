#!/usr/bin/env python3
"""
Generate clean ML test data.

Generates:
1. Neural network weights
2. Embeddings
3. Model checkpoints
4. Training datasets
5. Model architectures

All data is deterministic (seeded) for reproducibility.
"""

import json
import random
import numpy as np
from pathlib import Path

SEED = 42
random.seed(SEED)
np.random.seed(SEED)

OUTPUT_DIR = Path("../data/ml")
OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

def save_json(data, filename):
    with open(filename, 'w') as f:
        json.dump(data, f, indent=2)

def generate_nn_weights():
    """Generate neural network weights."""
    layers = [
        {"name": "layer1", "shape": [10, 20], "weights": np.random.randn(10, 20).tolist()},
        {"name": "layer2", "shape": [20, 10], "weights": np.random.randn(20, 10).tolist()},
        {"name": "layer3", "shape": [10, 5], "weights": np.random.randn(10, 5).tolist()}
    ]
    
    metadata = {
        "name": "Neural Network Weights",
        "type": "ML",
        "architecture": "feedforward",
        "num_layers": 3,
        "layers": layers,
        "description": "Simple feedforward network weights"
    }
    
    return metadata

def generate_embeddings():
    """Generate word embeddings."""
    vocab_size = 100
    embedding_dim = 50
    
    # Generate random embeddings
    embeddings = np.random.randn(vocab_size, embedding_dim).tolist()
    
    # Sample vocabulary
    vocab = [f"word_{i}" for i in range(vocab_size)]
    
    metadata = {
        "name": "Word Embeddings",
        "type": "ML",
        "vocab_size": vocab_size,
        "embedding_dim": embedding_dim,
        "vocab": vocab[:10],  # Only store first 10 for brevity
        "embeddings": embeddings[:10],  # Only store first 10 for brevity
        "description": "Word embeddings for NLP"
    }
    
    return metadata

def generate_checkpoint():
    """Generate model checkpoint."""
    checkpoint = {
        "epoch": 10,
        "step": 1000,
        "loss": 0.234,
        "accuracy": 0.876,
        "optimizer_state": {
            "learning_rate": 0.001,
            "momentum": 0.9
        },
        "model_state": {
            "layer1_weights": np.random.randn(10, 20).tolist(),
            "layer1_bias": np.random.randn(20).tolist()
        }
    }
    
    metadata = {
        "name": "Model Checkpoint",
        "type": "ML",
        "checkpoint": checkpoint,
        "description": "Training checkpoint at epoch 10"
    }
    
    return metadata

def main():
    print("Generating ML test data...")
    print(f"Output directory: {OUTPUT_DIR}")
    print(f"Random seed: {SEED}\n")
    
    generators = [
        ("nn_weights", generate_nn_weights),
        ("embeddings", generate_embeddings),
        ("checkpoint", generate_checkpoint)
    ]
    
    for name, generator in generators:
        metadata = generator()
        json_file = OUTPUT_DIR / f"{name}.json"
        save_json(metadata, json_file)
        print(f"  ✓ Generated {name}")
    
    total_files = len(list(OUTPUT_DIR.glob('*')))
    print(f"\n✅ Generated {total_files} ML test files")

if __name__ == "__main__":
    main()