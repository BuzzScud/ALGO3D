#!/bin/bash
# Fix prime_float_math.h includes

echo "=== Fixing prime_float_math.h includes ==="

for file in $(find crystalline algorithms recovery -name "*.c" -o -name "*.h"); do
    # Fix malformed includes (missing quotes)
    sed -i 's/#include prime_float_math\.h/#include "prime_float_math.h"/g' "$file"
    
    # Remove duplicate includes (keep only first occurrence)
    awk '!seen[$0]++ || !/prime_float_math\.h/' "$file" > "$file.tmp" && mv "$file.tmp" "$file"
done

echo "=== Fix complete ==="
echo ""
echo "Verifying:"
echo "Files with math.h: $(find crystalline algorithms recovery -name "*.c" -o -name "*.h" | xargs grep -l "#include <math.h>" 2>/dev/null | wc -l)"
echo "Files with prime_float_math.h: $(find crystalline algorithms recovery -name "*.c" -o -name "*.h" | xargs grep -l "prime_float_math.h" 2>/dev/null | wc -l)"
