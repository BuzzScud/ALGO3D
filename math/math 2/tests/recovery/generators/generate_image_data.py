#!/usr/bin/env python3
"""
Generate clean image test data (metadata only - actual images would be too large).

Generates metadata for:
1. Natural images (photos)
2. Medical images (MRI, CT scans)
3. Synthetic patterns (fractals, procedural)
4. Multi-resolution images (pyramids)
5. Video frames (frame sequences)

All data is deterministic (seeded) for reproducibility.
"""

import json
import random
import numpy as np
from pathlib import Path

SEED = 42
random.seed(SEED)
np.random.seed(SEED)

OUTPUT_DIR = Path("../data/image")
OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

def save_json(data, filename):
    with open(filename, 'w') as f:
        json.dump(data, f, indent=2)

def generate_natural_image():
    """Generate metadata for a natural image."""
    width, height = 1920, 1080
    
    # Generate small thumbnail data
    thumbnail = np.random.randint(0, 256, (32, 32, 3)).tolist()
    
    metadata = {
        "name": "Natural Image",
        "type": "Natural",
        "width": width,
        "height": height,
        "channels": 3,
        "format": "RGB",
        "thumbnail": thumbnail,
        "description": "Sample natural photograph"
    }
    
    return metadata

def generate_medical_image():
    """Generate metadata for a medical image."""
    width, height = 512, 512
    
    # Generate small thumbnail data (grayscale)
    thumbnail = np.random.randint(0, 256, (32, 32)).tolist()
    
    metadata = {
        "name": "Medical Image",
        "type": "Medical",
        "width": width,
        "height": height,
        "channels": 1,
        "format": "Grayscale",
        "modality": "MRI",
        "thumbnail": thumbnail,
        "description": "Sample MRI scan"
    }
    
    return metadata

def generate_synthetic_pattern():
    """Generate metadata for a synthetic pattern."""
    width, height = 512, 512
    
    # Generate checkerboard pattern thumbnail
    thumbnail = [[(i+j) % 2 * 255 for j in range(32)] for i in range(32)]
    
    metadata = {
        "name": "Synthetic Pattern",
        "type": "Synthetic",
        "width": width,
        "height": height,
        "channels": 1,
        "format": "Grayscale",
        "pattern": "checkerboard",
        "thumbnail": thumbnail,
        "description": "Procedurally generated checkerboard"
    }
    
    return metadata

def main():
    print("Generating image test data...")
    print(f"Output directory: {OUTPUT_DIR}")
    print(f"Random seed: {SEED}\n")
    
    generators = [
        ("natural_image", generate_natural_image),
        ("medical_image", generate_medical_image),
        ("synthetic_pattern", generate_synthetic_pattern)
    ]
    
    for name, generator in generators:
        metadata = generator()
        json_file = OUTPUT_DIR / f"{name}.json"
        save_json(metadata, json_file)
        print(f"  ✓ Generated {name}: {metadata['width']}x{metadata['height']}, {metadata['channels']} channels")
    
    total_files = len(list(OUTPUT_DIR.glob('*')))
    print(f"\n✅ Generated {total_files} image test files")

if __name__ == "__main__":
    main()