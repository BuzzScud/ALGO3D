#!/bin/bash

# Phase 1: Direct Replacements - Automated Migration
# Replace 30 basic math functions with math_* equivalents

echo "=== PHASE 1: DIRECT REPLACEMENTS ==="
echo ""
echo "Backing up files..."
mkdir -p migration_backup_$(date +%Y%m%d_%H%M%S)
cp -r src algorithms app cllm migration_backup_$(date +%Y%m%d_%H%M%S)/

echo ""
echo "Performing replacements..."

# Trigonometric functions
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/.git/*" ! -path "*/math/*" -exec sed -i 's/\bprime_sin(/math_sin(/g' {} +
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/.git/*" ! -path "*/math/*" -exec sed -i 's/\bprime_sinf(/math_sin(/g' {} +
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/.git/*" ! -path "*/math/*" -exec sed -i 's/\bprime_cos(/math_cos(/g' {} +
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/.git/*" ! -path "*/math/*" -exec sed -i 's/\bprime_cosf(/math_cos(/g' {} +
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/.git/*" ! -path "*/math/*" -exec sed -i 's/\bprime_tan(/math_tan(/g' {} +
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/.git/*" ! -path "*/math/*" -exec sed -i 's/\bprime_tanf(/math_tan(/g' {} +
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/.git/*" ! -path "*/math/*" -exec sed -i 's/\bprime_asin(/math_asin(/g' {} +
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/.git/*" ! -path "*/math/*" -exec sed -i 's/\bprime_acos(/math_acos(/g' {} +
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/.git/*" ! -path "*/math/*" -exec sed -i 's/\bprime_atan(/math_atan(/g' {} +
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/.git/*" ! -path "*/math/*" -exec sed -i 's/\bprime_atanf(/math_atan(/g' {} +

# Hyperbolic functions
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/.git/*" ! -path "*/math/*" -exec sed -i 's/\bprime_sinh(/math_sinh(/g' {} +
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/.git/*" ! -path "*/math/*" -exec sed -i 's/\bprime_cosh(/math_cosh(/g' {} +
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/.git/*" ! -path "*/math/*" -exec sed -i 's/\bprime_tanh(/math_tanh(/g' {} +
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/.git/*" ! -path "*/math/*" -exec sed -i 's/\bprime_tanhf(/math_tanh(/g' {} +

# Exponential and logarithmic
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/.git/*" ! -path "*/math/*" -exec sed -i 's/\bprime_exp(/math_exp(/g' {} +
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/.git/*" ! -path "*/math/*" -exec sed -i 's/\bprime_expf(/math_exp(/g' {} +
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/.git/*" ! -path "*/math/*" -exec sed -i 's/\bprime_log(/math_log(/g' {} +
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/.git/*" ! -path "*/math/*" -exec sed -i 's/\bprime_logf(/math_log(/g' {} +
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/.git/*" ! -path "*/math/*" -exec sed -i 's/\bprime_sqrt(/math_sqrt(/g' {} +
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/.git/*" ! -path "*/math/*" -exec sed -i 's/\bprime_sqrtf(/math_sqrt(/g' {} +
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/.git/*" ! -path "*/math/*" -exec sed -i 's/\bprime_pow(/math_pow(/g' {} +
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/.git/*" ! -path "*/math/*" -exec sed -i 's/\bprime_powf(/math_pow(/g' {} +

# Absolute value
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/.git/*" ! -path "*/math/*" -exec sed -i 's/\bprime_abs(/math_abs(/g' {} +
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/.git/*" ! -path "*/math/*" -exec sed -i 's/\bprime_fabs(/math_abs(/g' {} +
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/.git/*" ! -path "*/math/*" -exec sed -i 's/\bprime_fabsf(/math_abs(/g' {} +
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/.git/*" ! -path "*/math/*" -exec sed -i 's/\bprime_abs_int(/math_abs(/g' {} +

# Rounding functions
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/.git/*" ! -path "*/math/*" -exec sed -i 's/\bprime_floor(/math_floor(/g' {} +
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/.git/*" ! -path "*/math/*" -exec sed -i 's/\bprime_ceil(/math_ceil(/g' {} +
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/.git/*" ! -path "*/math/*" -exec sed -i 's/\bprime_round(/math_round(/g' {} +
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/.git/*" ! -path "*/math/*" -exec sed -i 's/\bprime_trunc(/math_trunc(/g' {} +

# Min/Max functions
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/.git/*" ! -path "*/math/*" -exec sed -i 's/\bprime_fmin(/math_min(/g' {} +
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/.git/*" ! -path "*/math/*" -exec sed -i 's/\bprime_fminf(/math_min(/g' {} +
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/.git/*" ! -path "*/math/*" -exec sed -i 's/\bprime_fmax(/math_max(/g' {} +
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/.git/*" ! -path "*/math/*" -exec sed -i 's/\bprime_fmaxf(/math_max(/g' {} +
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/.git/*" ! -path "*/math/*" -exec sed -i 's/\bprime_min_int(/math_min(/g' {} +
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/.git/*" ! -path "*/math/*" -exec sed -i 's/\bprime_max_int(/math_max(/g' {} +
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/.git/*" ! -path "*/math/*" -exec sed -i 's/\bprime_clamp_int(/math_clamp(/g' {} +

# Modulo functions
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/.git/*" ! -path "*/math/*" -exec sed -i 's/\bprime_fmod(/math_fmod(/g' {} +
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/.git/*" ! -path "*/math/*" -exec sed -i 's/\bprime_fmodf(/math_fmod(/g' {} +

# Validation functions
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/.git/*" ! -path "*/math/*" -exec sed -i 's/\bprime_isnan(/math_is_nan(/g' {} +
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/.git/*" ! -path "*/math/*" -exec sed -i 's/\bprime_isnanf(/math_is_nan(/g' {} +
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/.git/*" ! -path "*/math/*" -exec sed -i 's/\bprime_isinf(/math_is_inf(/g' {} +
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/.git/*" ! -path "*/math/*" -exec sed -i 's/\bprime_isinff(/math_is_inf(/g' {} +

echo ""
echo "Replacements complete!"
echo ""
echo "Counting changes..."
echo "Files modified:"
git diff --name-only | wc -l
echo ""
echo "Total changes:"
git diff --stat

echo ""
echo "=== PHASE 1 COMPLETE ==="
echo "Next: Build and verify"
