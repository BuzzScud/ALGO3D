#!/usr/bin/env python3
"""Update algorithms/Makefile to include all geometric recovery files"""

import os

# All geometric recovery source files
geometric_recovery_files = [
    # Original 19 files
    "anchor_tracking.c", "ecdsa_sample_loader.c", "ecdsa_test_generator.c",
    "g_triangulation.c", "geometric_anchors.c", "geometric_recovery_complete.c",
    "geometric_utils.c", "harmonic_folding.c", "integrated_recovery.c",
    "iterative_recovery.c", "iterative_recovery_v2.c", "multi_torus_tracker.c",
    "oscillation_decomposition.c", "oscillation_vector.c", "plateau_detection.c",
    "prime_float_math.c", "q_validation.c", "quadrant_polarity.c", "shared_geometry.c",
    # NEW: Core Recovery (4)
    "micro_model.c", "torus_analysis.c", "oscillation_detection.c", "multi_scalar_analysis.c",
    # NEW: Platonic Model (6)
    "platonic_model_core.c", "platonic_model_oscillations.c", "platonic_model_persistence.c",
    "platonic_model_recovery.c", "platonic_model_scaling.c", "platonic_solids.c",
    # NEW: Search & Recovery (5)
    "search_recovery.c", "search_recovery_v2.c", "search_recovery_v3.c",
    "search_recovery_v4.c", "search_recovery_v5.c",
    # NEW: Advanced Recovery (6)
    "recursive_recovery.c", "recursive_search.c", "spherical_recovery.c",
    "k_recovery_enhanced.c", "clock_recovery.c", "prime_rainbow_recovery.c",
    # NEW: Support Systems (6)
    "tetration_attractors.c", "tetration_real.c", "kissing_spheres.c",
    "crystal_abacus.c", "anchor_grid_24.c", "ecdlp_integration.c",
]

makefile_path = "algorithms/Makefile"

# Read Makefile
with open(makefile_path, 'r') as f:
    lines = f.readlines()

# Find geometric_recovery section
start_idx = None
for i, line in enumerate(lines):
    if 'src/geometric_recovery/' in line:
        start_idx = i
        break

if start_idx is None:
    print("Error: Could not find geometric_recovery section")
    exit(1)

# Find end of section
end_idx = start_idx
for i in range(start_idx, len(lines)):
    if 'src/geometric_recovery/' not in lines[i]:
        end_idx = i
        break

# Generate new lines
new_lines = []
for i, filename in enumerate(sorted(geometric_recovery_files)):
    # Skip if in platonic_model directory
    if os.path.exists(f"algorithms/src/platonic_model/{filename}"):
        print(f"Skipping {filename} (in platonic_model/)")
        continue
    
    line = f"          src/geometric_recovery/{filename}"
    if i < len(geometric_recovery_files) - 1:
        line += " \\\n"
    else:
        line += "\n"
    new_lines.append(line)

# Replace section
new_content = lines[:start_idx] + new_lines + lines[end_idx:]

# Write back
with open(makefile_path, 'w') as f:
    f.writelines(new_content)

print(f"✓ Makefile updated with {len(new_lines)} files")