#!/usr/bin/env python3
"""
Phase 3: Organize Tests and Prototypes
Maps existing tests to appropriate chapter directories
"""

import os
import shutil
from pathlib import Path

def create_chapter_readme(chapter_path: str, chapter_num: int, chapter_name: str, description: str = ""):
    """Create README for a chapter"""
    readme_content = f"""# Chapter {chapter_num}: {chapter_name}

## Overview

{description if description else f"This chapter covers {chapter_name.lower()}."}

## Contents

- [Main Content](content.md)

## Related

- **Prototypes:** See `prototypes/chapter_{chapter_num:02d}_*/`
- **Tests:** See `tests/unit/chapter_{chapter_num:02d}/`

## Navigation

- [Previous Chapter](../chapter_{chapter_num-1:02d}_*/) (if applicable)
- [Next Chapter](../chapter_{chapter_num+1:02d}_*/) (if applicable)
- [Back to Part](../)
- [Back to Thesis](../../)
"""
    
    readme_path = os.path.join(chapter_path, "README.md")
    with open(readme_path, 'w') as f:
        f.write(readme_content)
    print(f"✓ Created README: {readme_path}")

def map_tests_to_chapters():
    """Map existing tests to chapter directories"""
    test_mappings = {
        'test_clock_lattice.c': 'chapter_05',
        'test_pure_crystalline.c': 'chapter_05',
        'test_plimpton_integration.c': 'chapter_02',
        'test_cymatic_integration.c': 'chapter_02',
        'test_mathematical_integration.c': 'chapter_04',
        'test_blind_recovery.c': 'chapter_12',
        'test_blind_recovery_simple.c': 'chapter_12',
        'test_cllm_attention_gradients.c': 'chapter_19',
        'test_training_functions.c': 'chapter_19',
        'benchmark_plimpton_integration.c': 'chapter_02',
    }
    
    tests_dir = Path('tests')
    
    for test_file, chapter in test_mappings.items():
        src = tests_dir / test_file
        if src.exists():
            dest_dir = tests_dir / 'unit' / chapter
            dest_dir.mkdir(parents=True, exist_ok=True)
            dest = dest_dir / test_file
            
            # Copy (not move) to preserve originals
            shutil.copy2(src, dest)
            print(f"✓ Copied {test_file} → tests/unit/{chapter}/")

def create_prototype_readmes():
    """Create README files for prototype directories"""
    prototypes = {
        'chapter_02_babylonian': 'Babylonian Mathematics Implementations',
        'chapter_05_clock_lattice': 'Clock Lattice and Duality Prototypes',
        'chapter_06_abacus': 'Crystalline Abacus Implementations',
        'chapter_12_blind_recovery': 'Blind Recovery Algorithms',
        'chapter_13_ntt': 'NTT and 88D Architecture',
        'chapter_15_platonic': 'Platonic Solid Generators',
        'chapter_17_hashing': 'Geometric Hashing Algorithms',
        'chapter_19_cllm': 'CLLM Architecture Implementations',
    }
    
    for proto_dir, description in prototypes.items():
        readme_path = f"prototypes/{proto_dir}/README.md"
        content = f"""# {description}

## Overview

This directory contains prototype implementations for the corresponding thesis chapter.

## Structure

- `src/` - Source code
- `include/` - Header files  
- `tests/` - Unit tests
- `Makefile` - Build configuration

## Building

```bash
make
```

## Testing

```bash
make test
```

## Related

- **Thesis Chapter:** See `thesis/part_*/chapter_{proto_dir}/`
- **Tests:** See `tests/unit/{proto_dir}/`
"""
        
        os.makedirs(os.path.dirname(readme_path), exist_ok=True)
        with open(readme_path, 'w') as f:
            f.write(content)
        print(f"✓ Created {readme_path}")

def main():
    print("=" * 60)
    print("PHASE 3: ORGANIZE TESTS AND PROTOTYPES")
    print("=" * 60)
    print()
    
    # Create chapter READMEs
    print("Creating chapter README files...")
    
    chapters = [
        ("part_01_theoretical_foundations/chapter_01_introduction", 1, "Introduction"),
        ("part_01_theoretical_foundations/chapter_02_babylonian_mathematics", 2, "Babylonian Mathematics"),
        ("part_01_theoretical_foundations/chapter_03_ancient_proverb", 3, "Ancient Proverb"),
        ("part_01_theoretical_foundations/chapter_04_geometric_arithmetic", 4, "Geometric Arithmetic"),
        ("part_01_theoretical_foundations/chapter_05_clock_lattice", 5, "Clock Lattice"),
        ("part_01_theoretical_foundations/chapter_06_crystalline_abacus", 6, "Crystalline Abacus"),
        ("part_02_core_principles/chapter_07_triangulation", 7, "Triangulation"),
        ("part_02_core_principles/chapter_08_self_similarity", 8, "Self-Similarity"),
        ("part_02_core_principles/chapter_09_prime_generation", 9, "Prime Generation"),
        ("part_02_core_principles/chapter_10_clock_triangle", 10, "Clock Triangle"),
        ("part_02_core_principles/chapter_11_babylonian_operations", 11, "Babylonian Operations"),
        ("part_03_advanced_concepts/chapter_12_blind_recovery", 12, "Blind Recovery"),
        ("part_03_advanced_concepts/chapter_13_ntt_algorithms", 13, "NTT Algorithms"),
        ("part_03_advanced_concepts/chapter_14_kissing_spheres", 14, "Kissing Spheres"),
        ("part_03_advanced_concepts/chapter_15_platonic_solids", 15, "Platonic Solids"),
        ("part_03_advanced_concepts/chapter_16_geometric_recovery", 16, "Geometric Recovery"),
        ("part_04_applications/chapter_17_novel_hashing", 17, "Novel Hashing"),
        ("part_04_applications/chapter_18_blockchain", 18, "Blockchain"),
        ("part_04_applications/chapter_19_ai_architecture", 19, "AI Architecture"),
        ("part_04_applications/chapter_20_cryptography", 20, "Cryptography"),
    ]
    
    for chapter_path, num, name in chapters:
        full_path = f"thesis/{chapter_path}"
        if os.path.exists(full_path):
            create_chapter_readme(full_path, num, name)
    
    print()
    
    # Map tests to chapters
    print("Organizing tests by chapter...")
    map_tests_to_chapters()
    print()
    
    # Create prototype READMEs
    print("Creating prototype README files...")
    create_prototype_readmes()
    print()
    
    print("=" * 60)
    print("PHASE 3 COMPLETE")
    print("=" * 60)
    print()
    print("Summary:")
    print("✓ Chapter README files created")
    print("✓ Tests organized by chapter")
    print("✓ Prototype READMEs created")
    print()
    print("Next: Phase 4 - Cross-referencing and validation")

if __name__ == "__main__":
    main()