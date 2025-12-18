#!/bin/bash
# Comprehensive Library Analysis Tool
# Analyzes all files for prime-related functions and O(1) optimization opportunities

echo "=========================================="
echo "COMPREHENSIVE LIBRARY ANALYSIS"
echo "=========================================="
echo ""

echo "=== ANALYZING ALGORITHMS LIBRARY ==="
echo ""

# Search for prime-related function calls
echo "1. Prime-related function usage:"
grep -r "prime_nth\|prime_generate\|prime_is_prime\|is_prime" algorithms/src/ --include="*.c" | \
    grep -v "\.o:" | \
    wc -l
echo "   files with prime functions found"
echo ""

# List files with prime functions
echo "2. Files using prime functions:"
grep -r "prime_nth\|prime_generate\|prime_is_prime\|is_prime" algorithms/src/ --include="*.c" -l | \
    grep -v "\.o$" | \
    sort
echo ""

# Search for math.h usage (should be eliminated)
echo "3. Files using math.h (should use crystalline math):"
grep -r "#include.*math\.h" algorithms/src/ --include="*.c" | \
    grep -v "prime_float_math\|prime_math" | \
    wc -l
echo "   files found"
echo ""

echo "=== ANALYZING NEW MATH LIBRARY ==="
echo ""

# List all math library source files
echo "4. Math library source files:"
find math/src -name "*.c" -type f | wc -l
echo "   files found"
echo ""

# Check for any remaining math.h usage in math library
echo "5. Math library math.h usage (should be ZERO):"
grep -r "#include.*<math\.h>" math/src/ --include="*.c" | wc -l
echo "   files found (should be 0)"
echo ""

# List all test files
echo "6. Test files:"
find math/tests -name "*.c" -type f | wc -l
echo "   test files found"
echo ""

echo "=== INTEGRATION OPPORTUNITIES ==="
echo ""

# Files that could benefit from O(1) formula
echo "7. Files with trial division (optimization candidates):"
grep -r "i \* i <= n\|i <= limit\|sqrt" algorithms/src/ --include="*.c" -l | \
    grep -v "\.o$" | \
    wc -l
echo "   files found"
echo ""

echo "=========================================="
echo "ANALYSIS COMPLETE"
echo "=========================================="