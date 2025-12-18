#!/bin/bash

# Fix all platonic_model files to use prime_* functions instead of math.h

FILES=(
    "algorithms/src/platonic_model/tetration_real.c"
    "algorithms/src/platonic_model/platonic_model_core.c"
    "algorithms/src/platonic_model/platonic_model_oscillations.c"
    "algorithms/src/platonic_model/platonic_model_recovery.c"
    "algorithms/src/platonic_model/platonic_model_scaling.c"
)

for file in "${FILES[@]}"; do
    echo "Fixing $file..."
    
    # Replace math.h include with prime_float_math.h
    sed -i 's/#include <math\.h>/#include "..\/..\/..\/include\/prime_float_math.h"/' "$file"
    
    # Replace math functions with prime_* equivalents
    sed -i 's/\bexp(/prime_exp(/g' "$file"
    sed -i 's/\bexpf(/prime_expf(/g' "$file"
    sed -i 's/\blog(/prime_log(/g' "$file"
    sed -i 's/\blogf(/prime_logf(/g' "$file"
    sed -i 's/\bsqrt(/prime_sqrt(/g' "$file"
    sed -i 's/\bsqrtf(/prime_sqrtf(/g' "$file"
    sed -i 's/\bpow(/prime_pow(/g' "$file"
    sed -i 's/\bpowf(/prime_powf(/g' "$file"
    sed -i 's/\bsin(/prime_sin(/g' "$file"
    sed -i 's/\bsinf(/prime_sinf(/g' "$file"
    sed -i 's/\bcos(/prime_cos(/g' "$file"
    sed -i 's/\bcosf(/prime_cosf(/g' "$file"
    sed -i 's/\btan(/prime_tan(/g' "$file"
    sed -i 's/\btanf(/prime_tanf(/g' "$file"
    sed -i 's/\btanh(/prime_tanh(/g' "$file"
    sed -i 's/\btanhf(/prime_tanhf(/g' "$file"
    sed -i 's/\bfabs(/prime_fabs(/g' "$file"
    sed -i 's/\bfabsf(/prime_fabsf(/g' "$file"
    sed -i 's/\bfloor(/prime_floor(/g' "$file"
    sed -i 's/\bfloorf(/prime_floorf(/g' "$file"
    sed -i 's/\bceil(/prime_ceil(/g' "$file"
    sed -i 's/\bceilf(/prime_ceilf(/g' "$file"
    sed -i 's/\bfmod(/prime_fmod(/g' "$file"
    sed -i 's/\bfmodf(/prime_fmodf(/g' "$file"
    sed -i 's/\batan2(/prime_atan2(/g' "$file"
    sed -i 's/\batan(/prime_atan(/g' "$file"
    sed -i 's/\bacos(/prime_acos(/g' "$file"
    sed -i 's/\blog2(/prime_log2(/g' "$file"
    sed -i 's/\bfmax(/prime_fmax(/g' "$file"
    sed -i 's/\bfmin(/prime_fmin(/g' "$file"
    sed -i 's/\bround(/prime_round(/g' "$file"
    
    echo "  ✓ Fixed $file"
done

echo ""
echo "All files fixed! Now rebuilding..."