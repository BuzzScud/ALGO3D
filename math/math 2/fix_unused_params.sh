#!/bin/bash
# Fix unused parameter warnings by adding (void) casts

echo "Fixing unused parameter warnings..."

# Fix anchor_adjustment.c
cat > /tmp/anchor_fix.txt << 'EOF'
    (void)vertex_positions;  // Unused in this implementation
    (void)num_vertices;      // Unused in this implementation
EOF

sed -i '63i\    (void)vertex_positions;  \/\/ Unused in this implementation\n    (void)num_vertices;      \/\/ Unused in this implementation' \
    algorithms/src/blind_recovery/anchor_adjustment.c

# Fix multi_scale_analysis.c
sed -i '97i\    (void)structure;  \/\/ Unused in this implementation' \
    algorithms/src/blind_recovery/multi_scale_analysis.c

# Fix model_expansion.c
sed -i '97i\    (void)target_solid;  \/\/ Unused in this implementation' \
    algorithms/src/blind_recovery/model_expansion.c

# Fix variance_analysis.c - compute_axis_mean
sed -i '49i\    (void)axis;\n    (void)num_dimensions;\n    (void)dimension_sizes;' \
    algorithms/src/blind_recovery/variance_analysis.c

# Fix variance_analysis.c - compute_axis_variance (need to find line number)
LINE=$(grep -n "double compute_axis_variance" algorithms/src/blind_recovery/variance_analysis.c | tail -1 | cut -d: -f1)
if [ ! -z "$LINE" ]; then
    sed -i "$((LINE+3))i\    (void)axis;\n    (void)num_dimensions;\n    (void)dimension_sizes;" \
        algorithms/src/blind_recovery/variance_analysis.c
fi

# Fix cross_correlation.c
sed -i '51i\    (void)num_dimensions;  \/\/ Unused in this implementation' \
    algorithms/src/blind_recovery/cross_correlation.c

# Fix geometric_utils.c
sed -i '188i\    (void)position;\n    (void)num_dimensions;' \
    algorithms/src/geometric_recovery/geometric_utils.c

# Fix geometric_recovery.c - multiple functions
# find_torus_intersection_curve
LINE=$(grep -n "PlatonicModel\* model" algorithms/src/geometric_recovery/geometric_recovery.c | grep "find_torus_intersection_curve" | cut -d: -f1)
if [ ! -z "$LINE" ]; then
    sed -i "$((LINE+1))i\    (void)model;  \/\/ Unused in this implementation" \
        algorithms/src/geometric_recovery/geometric_recovery.c
fi

# search_torus_orbit
LINE=$(grep -n "const TorusIntersectionCurve\* curve" algorithms/src/geometric_recovery/geometric_recovery.c | head -1 | cut -d: -f1)
if [ ! -z "$LINE" ]; then
    sed -i "$((LINE+4))i\    (void)curve;  \/\/ Unused in this implementation" \
        algorithms/src/geometric_recovery/geometric_recovery.c
fi

# multi_scale_fractal_search
LINE=$(grep -n "const TorusOrbit\* tori" algorithms/src/geometric_recovery/geometric_recovery.c | head -1 | cut -d: -f1)
if [ ! -z "$LINE" ]; then
    sed -i "$((LINE+2))i\    (void)tori;\n    (void)num_tori;" \
        algorithms/src/geometric_recovery/geometric_recovery.c
fi

# Fix cllm_inference_transformer.c
sed -i '82i\    (void)seq_len;  \/\/ Unused in single-token inference' \
    src/ai/cllm_inference_transformer.c

# Remove unused variable declarations
sed -i 's/bool has_nan_out = false;/bool has_nan_out __attribute__((unused)) = false;/' \
    src/ai/cllm_inference_transformer.c
sed -i 's/bool has_nan_gelu = false;/bool has_nan_gelu __attribute__((unused)) = false;/' \
    src/ai/cllm_inference_transformer.c
sed -i 's/bool has_nan_h1 = false;/bool has_nan_h1 __attribute__((unused)) = false;/' \
    src/ai/cllm_inference_transformer.c

# Fix cllm_training_threaded.c unused variables
sed -i 's/double\* ffn_w1 = model/double* ffn_w1 __attribute__((unused)) = model/' \
    src/ai/cllm_training_threaded.c
sed -i 's/double\* ffn_b1 = model/double* ffn_b1 __attribute__((unused)) = model/' \
    src/ai/cllm_training_threaded.c

# Fix cllm_vocabulary.c
sed -i 's/static void to_lowercase/static void to_lowercase __attribute__((unused))/' \
    src/ai/cllm_vocabulary.c

# Fix format warning in cllm_vocabulary.c
sed -i 's/printf("Total Tokens Processed: %lu/printf("Total Tokens Processed: %u/' \
    src/ai/cllm_vocabulary.c

# Fix cllm_blind_recovery.c type comparison warnings
sed -i 's/if (pos->ring >= 0 &&/if (/' \
    src/ai/cllm_blind_recovery.c
sed -i 's/if (pos->ring < 0 ||/if (/' \
    src/ai/cllm_blind_recovery.c

# Fix cllm_root_word_modeling.c MATH_PHI redefinition
sed -i '/^#define MATH_PHI 1.618033988749895/d' \
    src/ai/cllm_root_word_modeling.c

echo "✓ All unused parameter warnings fixed"
echo "Rebuild with: make clean && make"