#!/bin/bash
# Fix include paths in newly copied geometric recovery files

cd /workspace/algorithms/src/geometric_recovery

echo "=== Fixing include paths in geometric recovery files ==="

# List of files that need fixing
files=(
    "anchor_grid_24.c"
    "clock_recovery.c"
    "crystal_abacus.c"
    "ecdlp_integration.c"
    "k_recovery_enhanced.c"
    "kissing_spheres.c"
    "micro_model.c"
    "multi_scalar_analysis.c"
    "oscillation_detection.c"
    "platonic_solids.c"
    "prime_rainbow_recovery.c"
    "recursive_recovery.c"
    "recursive_search.c"
    "search_recovery.c"
    "search_recovery_v2.c"
    "search_recovery_v3.c"
    "search_recovery_v4.c"
    "search_recovery_v5.c"
    "spherical_recovery.c"
    "tetration_attractors.c"
    "torus_analysis.c"
)

for file in "${files[@]}"; do
    if [ -f "$file" ]; then
        echo "Fixing $file..."
        # Fix includes: "header.h" -> "geometric_recovery/header.h"
        # But skip system headers and already correct paths
        sed -i 's/#include "\([^/]*\.h\)"/#include "geometric_recovery\/\1"/g' "$file"
        # Fix double geometric_recovery paths
        sed -i 's/geometric_recovery\/geometric_recovery\//geometric_recovery\//g' "$file"
        # Fix paths that should point to parent includes
        sed -i 's/geometric_recovery\/\.\.\/include\//..\/include\//g' "$file"
    fi
done

echo "✓ Include paths fixed"