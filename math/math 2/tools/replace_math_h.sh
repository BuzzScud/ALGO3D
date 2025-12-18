#!/bin/bash
# Automated script to replace math.h with prime_float_math.h

set -e

echo "=== OBJECTIVE 2E: Replacing math.h with prime_float_math.h ==="
echo ""

# Find all files with math.h
FILES=$(find crystalline algorithms recovery -name "*.c" -o -name "*.h" | xargs grep -l "#include <math.h>" 2>/dev/null || true)

if [ -z "$FILES" ]; then
    echo "No files found with #include <math.h>"
    exit 0
fi

echo "Found $(echo "$FILES" | wc -l) files to process"
echo ""

# Process each file
for file in $FILES; do
    echo "Processing: $file"
    
    # Create backup
    cp "$file" "$file.bak"
    
    # Replace include
    sed -i 's/#include <math\.h>/#include "prime_float_math.h"/g' "$file"
    
    # Replace double precision functions
    sed -i 's/\bsqrt(/prime_sqrt(/g' "$file"
    sed -i 's/\bexp(/prime_exp(/g' "$file"
    sed -i 's/\blog(/prime_log(/g' "$file"
    sed -i 's/\bpow(/prime_pow(/g' "$file"
    sed -i 's/\bfabs(/prime_fabs(/g' "$file"
    sed -i 's/\bsin(/prime_sin(/g' "$file"
    sed -i 's/\bcos(/prime_cos(/g' "$file"
    sed -i 's/\btan(/prime_tan(/g' "$file"
    sed -i 's/\btanh(/prime_tanh(/g' "$file"
    sed -i 's/\batan(/prime_atan(/g' "$file"
    sed -i 's/\batan2(/prime_atan2(/g' "$file"
    sed -i 's/\bacos(/prime_acos(/g' "$file"
    sed -i 's/\bfloor(/prime_floor(/g' "$file"
    sed -i 's/\bceil(/prime_ceil(/g' "$file"
    sed -i 's/\bround(/prime_round(/g' "$file"
    sed -i 's/\bfmod(/prime_fmod(/g' "$file"
    sed -i 's/\bfmax(/prime_fmax(/g' "$file"
    sed -i 's/\bfmin(/prime_fmin(/g' "$file"
    sed -i 's/\bisnan(/prime_isnan(/g' "$file"
    sed -i 's/\bisinf(/prime_isinf(/g' "$file"
    sed -i 's/\blog2(/prime_log2(/g' "$file"
    
    # Replace float precision functions
    sed -i 's/\bsqrtf(/prime_sqrtf(/g' "$file"
    sed -i 's/\bexpf(/prime_expf(/g' "$file"
    sed -i 's/\blogf(/prime_logf(/g' "$file"
    sed -i 's/\bpowf(/prime_powf(/g' "$file"
    sed -i 's/\bfabsf(/prime_fabsf(/g' "$file"
    sed -i 's/\bsinf(/prime_sinf(/g' "$file"
    sed -i 's/\bcosf(/prime_cosf(/g' "$file"
    sed -i 's/\btanf(/prime_tanf(/g' "$file"
    sed -i 's/\btanhf(/prime_tanhf(/g' "$file"
    sed -i 's/\batanf(/prime_atanf(/g' "$file"
    sed -i 's/\batan2f(/prime_atan2f(/g' "$file"
    sed -i 's/\bfmodf(/prime_fmodf(/g' "$file"
    sed -i 's/\bfmaxf(/prime_fmaxf(/g' "$file"
    sed -i 's/\bfminf(/prime_fminf(/g' "$file"
    sed -i 's/\bisnanf(/prime_isnanf(/g' "$file"
    sed -i 's/\bisinff(/prime_isinff(/g' "$file"
done

echo ""
echo "=== Replacement Complete ==="
echo "Processed $(echo "$FILES" | wc -l) files"
echo "Backups saved with .bak extension"
echo ""
echo "Next steps:"
echo "1. Review changes: git diff"
echo "2. Build: make clean && make"
echo "3. Test: make test"
echo "4. If issues, restore: for f in \$(find . -name '*.bak'); do mv \$f \${f%.bak}; done"
