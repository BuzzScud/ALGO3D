#!/bin/bash

# Fix Header Conflicts - Remove OLD library headers and update includes

echo "=== FIXING HEADER CONFLICTS ==="
echo ""

# Step 1: Remove conflicting OLD library header files
echo "Step 1: Removing OLD library header files..."
rm -f include/prime_lowlevel.h
rm -f include/prime_math.h
rm -f include/prime_math_custom.h
echo "  ✓ Removed prime_lowlevel.h, prime_math.h, prime_math_custom.h"

# Step 2: Update includes in remaining OLD headers to not reference removed headers
echo ""
echo "Step 2: Updating remaining OLD headers..."

# Remove includes of deleted headers from prime_basic.h
if [ -f include/prime_basic.h ]; then
    sed -i '/#include.*"prime_lowlevel.h"/d' include/prime_basic.h
    sed -i '/#include.*"prime_math.h"/d' include/prime_basic.h
    sed -i '/#include.*"prime_math_custom.h"/d' include/prime_basic.h
    echo "  ✓ Updated prime_basic.h"
fi

# Remove includes from prime_lattice.h
if [ -f include/prime_lattice.h ]; then
    sed -i '/#include.*"prime_lowlevel.h"/d' include/prime_lattice.h
    sed -i '/#include.*"prime_math.h"/d' include/prime_lattice.h
    sed -i '/#include.*"prime_math_custom.h"/d' include/prime_lattice.h
    echo "  ✓ Updated prime_lattice.h"
fi

# Step 3: Update source files to include NEW library headers
echo ""
echo "Step 3: Updating source files to use NEW library headers..."

# Add NEW library includes where OLD headers were used
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/.git/*" ! -path "*/math/*" -exec sed -i '
    /#include.*"prime_lowlevel.h"/ {
        a #include "math/arithmetic.h"
        a #include "math/transcendental.h"
        d
    }
' {} +

find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/.git/*" ! -path "*/math/*" -exec sed -i '
    /#include.*"prime_math.h"/ {
        a #include "math/arithmetic.h"
        a #include "math/transcendental.h"
        d
    }
' {} +

find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/.git/*" ! -path "*/math/*" -exec sed -i '
    /#include.*"prime_math_custom.h"/ {
        a #include "math/arithmetic.h"
        a #include "math/transcendental.h"
        d
    }
' {} +

echo "  ✓ Updated source files"

echo ""
echo "=== HEADER CONFLICTS FIXED ==="
echo ""
echo "Deleted files:"
echo "  - include/prime_lowlevel.h"
echo "  - include/prime_math.h"
echo "  - include/prime_math_custom.h"
echo ""
echo "Updated includes to use:"
echo "  - math/arithmetic.h"
echo "  - math/transcendental.h"
echo ""
echo "Next: Rebuild"