#!/bin/bash
# Fix remaining math.h violations found in OBJECTIVE 5 verification

set -e

echo "=========================================="
echo "Fixing Remaining math.h Violations"
echo "=========================================="
echo ""

# List of files with violations
FILES_TO_FIX=(
    "src/geometry/clock_lattice.c"
    "src/ai/cllm_training_functions.c"
    "src/ai/cllm_blind_recovery.c"
    "src/ai/cllm_attention.c"
    "src/ai/cllm_batch_splitting.c"
    "src/ai/cllm_create.c"
    "src/ai/cllm_entropy_work_distribution.c"
    "src/ai/cllm_training_threaded.c"
    "src/ai/cllm_layernorm.c"
    "src/ai/cllm_entropy_integration.c"
    "src/ai/cllm_adaptive_hierarchy.c"
    "src/ai/cllm_cache_aware_distribution.c"
    "src/ai/cllm_entropy_allocation.c"
    "src/ai/cllm_work_distribution.c"
    "src/ai/cllm_embedding.c"
    "src/ai/platonic/cllm_platonic_dodecahedron.c"
    "src/ai/platonic/cllm_platonic_core.c"
    "src/ai/platonic/cllm_platonic_harmonic.c"
    "src/ai/platonic/cllm_platonic_cube.c"
    "src/ai/platonic/cllm_platonic_tetrahedron.c"
    "src/ai/platonic/cllm_platonic_octahedron.c"
    "src/ai/platonic/cllm_platonic_recovery.c"
    "src/ai/platonic/cllm_platonic_icosahedron.c"
    "app/ui/crystalline_visualization.c"
)

echo "Step 1: Removing #include <math.h> from all files..."
for file in "${FILES_TO_FIX[@]}"; do
    if [ -f "$file" ]; then
        # Remove math.h include
        sed -i '/#include <math\.h>/d' "$file"
        
        # Add prime_float_math.h if not present
        if ! grep -q "prime_float_math.h" "$file"; then
            # Find the last #include line and add after it
            sed -i '/^#include/a #include "transcendental/prime_float_math.h"' "$file" | head -1
        fi
        
        echo "  ✓ Fixed $file"
    fi
done

echo ""
echo "Step 2: Replacing math.h functions with prime_* equivalents..."

# Function replacements
declare -A REPLACEMENTS=(
    ["\\bsqrt("]="prime_sqrt("
    ["\\bpow("]="prime_pow("
    ["\\bexp("]="prime_exp("
    ["\\blog("]="prime_log("
    ["\\bsin("]="prime_sin("
    ["\\bcos("]="prime_cos("
    ["\\btan("]="prime_tan("
    ["\\btanh("]="prime_tanh("
    ["\\bfabs("]="prime_fabs("
    ["\\bfloor("]="prime_floor("
    ["\\bceil("]="prime_ceil("
    ["\\bfmin("]="prime_fmin("
    ["\\bfmax("]="prime_fmax("
)

for file in "${FILES_TO_FIX[@]}"; do
    if [ -f "$file" ]; then
        for pattern in "${!REPLACEMENTS[@]}"; do
            replacement="${REPLACEMENTS[$pattern]}"
            sed -i "s/${pattern}/${replacement}/g" "$file"
        done
    fi
done

echo ""
echo "Step 3: Verifying fixes..."
REMAINING=$(find src app -type f \( -name "*.c" -o -name "*.h" \) -exec grep -l "#include <math.h>" {} \; 2>/dev/null | wc -l)

echo "Files still with math.h: $REMAINING"

if [ $REMAINING -eq 0 ]; then
    echo "✓ All math.h includes removed!"
else
    echo "⚠ Some files still have math.h"
fi

echo ""
echo "=========================================="
echo "Fix Complete"
echo "=========================================="