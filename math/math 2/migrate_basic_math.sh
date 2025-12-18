#!/bin/bash

# Migration Script - Phase 1: Basic Math Operations
# Replaces prime_* functions with math_* equivalents

echo "=== Starting Basic Math Migration ==="
echo "This script will replace prime_* functions with math_* equivalents"
echo ""

# Count before migration
echo "Counting prime_* instances before migration..."
BEFORE_COUNT=$(grep -roh "prime_[a-z_]*(" --include="*.c" --include="*.h" . | grep -v ".git" | grep -v "test" | wc -l)
echo "Total prime_* instances: $BEFORE_COUNT"
echo ""

# Create backup
echo "Creating backup..."
BACKUP_DIR="backup_before_math_migration_$(date +%Y%m%d_%H%M%S)"
mkdir -p "$BACKUP_DIR"
cp -r src algorithms app include cllm "$BACKUP_DIR/" 2>/dev/null
echo "Backup created in: $BACKUP_DIR"
echo ""

# Function to replace in files
replace_in_files() {
    local old_func=$1
    local new_func=$2
    local description=$3
    
    echo "Replacing $old_func → $new_func ($description)..."
    
    # Count occurrences
    local count=$(grep -r "$old_func(" --include="*.c" --include="*.h" . | grep -v ".git" | grep -v "test" | wc -l)
    
    if [ $count -gt 0 ]; then
        # Perform replacement
        find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/.git/*" ! -path "*/test*" -exec sed -i "s/${old_func}(/${new_func}(/g" {} +
        echo "  ✓ Replaced $count instances"
    else
        echo "  - No instances found"
    fi
}

# Basic arithmetic
echo "=== Category 1: Basic Arithmetic ==="
replace_in_files "prime_abs" "math_abs" "absolute value"
replace_in_files "prime_fabs" "math_abs" "floating point absolute"
replace_in_files "prime_fabsf" "math_abs" "float absolute"
replace_in_files "prime_fabs_inline" "math_abs" "inline absolute"
replace_in_files "prime_fabs_custom" "math_abs" "custom absolute"
echo ""

# Trigonometric functions
echo "=== Category 2: Trigonometric Functions ==="
replace_in_files "prime_sin" "math_sin" "sine"
replace_in_files "prime_sinf" "math_sin" "float sine"
replace_in_files "prime_cos" "math_cos" "cosine"
replace_in_files "prime_cosf" "math_cos" "float cosine"
replace_in_files "prime_tan" "math_tan" "tangent"
replace_in_files "prime_tanf" "math_tan" "float tangent"
replace_in_files "prime_asin" "math_asin" "arcsine"
replace_in_files "prime_acos" "math_acos" "arccosine"
replace_in_files "prime_atan" "math_atan" "arctangent"
replace_in_files "prime_atanf" "math_atan" "float arctangent"
echo ""

# Hyperbolic functions
echo "=== Category 3: Hyperbolic Functions ==="
replace_in_files "prime_sinh" "math_sinh" "hyperbolic sine"
replace_in_files "prime_cosh" "math_cosh" "hyperbolic cosine"
replace_in_files "prime_tanh" "math_tanh" "hyperbolic tangent"
replace_in_files "prime_tanhf" "math_tanh" "float hyperbolic tangent"
echo ""

# Exponential and logarithmic
echo "=== Category 4: Exponential and Logarithmic ==="
replace_in_files "prime_exp" "math_exp" "exponential"
replace_in_files "prime_expf" "math_exp" "float exponential"
replace_in_files "prime_log" "math_log" "natural logarithm"
replace_in_files "prime_logf" "math_log" "float logarithm"
replace_in_files "prime_sqrt" "math_sqrt" "square root"
replace_in_files "prime_sqrtf" "math_sqrt" "float square root"
replace_in_files "prime_pow" "math_pow" "power"
replace_in_files "prime_powf" "math_pow" "float power"
echo ""

# Rounding functions
echo "=== Category 5: Rounding Functions ==="
replace_in_files "prime_floor" "math_floor" "floor"
replace_in_files "prime_ceil" "math_ceil" "ceiling"
replace_in_files "prime_round" "math_round" "round"
replace_in_files "prime_trunc" "math_trunc" "truncate"
echo ""

# Min/Max functions
echo "=== Category 6: Min/Max Functions ==="
replace_in_files "prime_fmin" "math_min" "minimum"
replace_in_files "prime_fminf" "math_min" "float minimum"
replace_in_files "prime_fmax" "math_max" "maximum"
replace_in_files "prime_fmaxf" "math_max" "float maximum"
replace_in_files "prime_min_int" "math_min" "integer minimum"
replace_in_files "prime_max_int" "math_max" "integer maximum"
replace_in_files "prime_min_int_local" "math_min" "local integer minimum"
replace_in_files "prime_max_int_local" "math_max" "local integer maximum"
echo ""

# Modulo functions
echo "=== Category 7: Modulo Functions ==="
replace_in_files "prime_fmod" "math_fmod" "floating point modulo"
replace_in_files "prime_fmodf" "math_fmod" "float modulo"
echo ""

# Validation functions
echo "=== Category 8: Validation Functions ==="
replace_in_files "prime_isnan" "math_is_nan" "is NaN"
replace_in_files "prime_isnanf" "math_is_nan" "float is NaN"
replace_in_files "prime_isinf" "math_is_inf" "is infinite"
replace_in_files "prime_isinff" "math_is_inf" "float is infinite"
echo ""

# Count after migration
echo "=== Migration Complete ==="
AFTER_COUNT=$(grep -roh "prime_[a-z_]*(" --include="*.c" --include="*.h" . | grep -v ".git" | grep -v "test" | wc -l)
REPLACED=$((BEFORE_COUNT - AFTER_COUNT))
echo "Before: $BEFORE_COUNT instances"
echo "After:  $AFTER_COUNT instances"
echo "Replaced: $REPLACED instances"
echo ""

echo "Next steps:"
echo "1. Build the project: make clean && make"
echo "2. Fix any compilation errors"
echo "3. Run tests"
echo "4. Review changes: git diff"
echo "5. Commit: git add . && git commit -m 'Migrate basic math operations from prime_* to math_*'"
echo ""
echo "Backup location: $BACKUP_DIR"