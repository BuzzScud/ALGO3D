#!/bin/bash
# Phase 1: Delete Category 1 Legacy Files (ZERO RISK)

echo "=== PHASE 1: Deleting Category 1 Legacy Files ==="
echo ""

# Math library legacy files (backup copies from Phase 2 refactoring)
echo "Deleting math/src/core/arithmetic_legacy.c..."
rm -f math/src/core/arithmetic_legacy.c

echo "Deleting math/src/compact/compact_arithmetic_legacy.c..."
rm -f math/src/compact/compact_arithmetic_legacy.c

echo "Deleting platonic generator legacy files..."
rm -f math/src/platonic/simplex_generator_legacy.c
rm -f math/src/platonic/hypercube_generator_legacy.c
rm -f math/src/platonic/cross_polytope_generator_legacy.c
rm -f math/src/platonic/dodecahedron_generator_legacy.c
rm -f math/src/platonic/icosahedron_generator_legacy.c
rm -f math/src/platonic/simplex_generator_abacus_legacy.c
rm -f math/src/platonic/hypercube_generator_abacus_legacy.c
rm -f math/src/platonic/cross_polytope_generator_abacus_legacy.c

echo ""
echo "=== VERIFICATION ==="
echo "Checking for remaining legacy files..."
find math/src -name "*_legacy.c" 2>/dev/null

echo ""
echo "✓ Phase 1 Complete: All Category 1 legacy files deleted"