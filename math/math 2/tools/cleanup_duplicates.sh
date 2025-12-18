#!/bin/bash
# Remove duplicate prime_float_math.h includes

echo "=== Cleaning up duplicate includes ==="

for file in $(find crystalline algorithms recovery -name "*.c" -o -name "*.h"); do
    # Check if file has prime_float_math.h
    if grep -q "prime_float_math.h" "$file"; then
        # Remove all prime_float_math.h includes
        sed -i '/prime_float_math\.h/d' "$file"
        
        # Add single include at the top (after other includes)
        # Find the last #include line
        last_include=$(grep -n "^#include" "$file" | tail -1 | cut -d: -f1)
        
        if [ -n "$last_include" ]; then
            # Insert after last include
            sed -i "${last_include}a#include "prime_float_math.h"" "$file"
        fi
    fi
done

echo "=== Cleanup complete ==="
