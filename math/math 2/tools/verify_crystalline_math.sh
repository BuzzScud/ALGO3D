#!/bin/bash
# Verification script for OBJECTIVE 5: Verify Crystalline Math Integration
# This script checks for any remaining math.h usage and verifies crystalline math integration

set -e

echo "=========================================="
echo "OBJECTIVE 5: Crystalline Math Verification"
echo "=========================================="
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

ERRORS=0
WARNINGS=0

echo "Phase 1: Checking for math.h usage..."
echo "--------------------------------------"

# Check for math.h includes in source files (excluding tests and tools)
MATH_H_FILES=$(find src app -type f \( -name "*.c" -o -name "*.h" \) -exec grep -l "#include <math.h>" {} \; 2>/dev/null || true)

if [ -n "$MATH_H_FILES" ]; then
    echo -e "${RED}ERROR: Found math.h usage in production code:${NC}"
    echo "$MATH_H_FILES"
    ERRORS=$((ERRORS + 1))
else
    echo -e "${GREEN}✓ No math.h found in production code${NC}"
fi

echo ""
echo "Phase 2: Checking for standard math functions..."
echo "------------------------------------------------"

# List of forbidden math.h functions
FORBIDDEN_FUNCS=(
    "sqrt" "pow" "exp" "log" "log10" "log2"
    "sin" "cos" "tan" "asin" "acos" "atan" "atan2"
    "sinh" "cosh" "tanh" "asinh" "acosh" "atanh"
    "floor" "ceil" "round" "trunc" "fabs" "fmod"
    "fmin" "fmax" "hypot" "cbrt" "erf" "erfc"
)

for func in "${FORBIDDEN_FUNCS[@]}"; do
    # Look for function calls (not prime_* versions, not in comments, not in strings)
    MATCHES=$(find src app -type f \( -name "*.c" -o -name "*.h" \) -exec grep -n "\b${func}\s*(" {} + 2>/dev/null | \
              grep -v "prime_${func}" | \
              grep -v "^\s*//" | \
              grep -v "^\s*\*" | \
              grep -v "/\*.*${func}.*\*/" | \
              grep -v "//.*${func}" | \
              grep -v "\* .*${func}" | \
              grep -v "printf.*${func}" | \
              grep -v "&quot;.*${func}.*&quot;" || true)
    
    if [ -n "$MATCHES" ]; then
        # Further filter: only show actual function calls (has opening parenthesis on same line)
        REAL_CALLS=$(echo "$MATCHES" | grep "${func}(" || true)
        if [ -n "$REAL_CALLS" ]; then
            echo -e "${RED}ERROR: Found usage of ${func}():${NC}"
            echo "$REAL_CALLS" | head -5
            ERRORS=$((ERRORS + 1))
        fi
    fi
done

if [ $ERRORS -eq 0 ]; then
    echo -e "${GREEN}✓ No forbidden math functions found${NC}"
fi

echo ""
echo "Phase 3: Verifying prime_* function usage..."
echo "--------------------------------------------"

# Check for prime_* function usage
PRIME_FUNCS=$(find src app -type f \( -name "*.c" -o -name "*.h" \) -exec grep -o "prime_[a-z_]*(" {} + 2>/dev/null | sort -u || true)

if [ -n "$PRIME_FUNCS" ]; then
    echo -e "${GREEN}✓ Found crystalline math functions:${NC}"
    echo "$PRIME_FUNCS" | head -20
    FUNC_COUNT=$(echo "$PRIME_FUNCS" | wc -l)
    echo "Total: $FUNC_COUNT unique prime_* functions"
else
    echo -e "${YELLOW}WARNING: No prime_* functions found${NC}"
    WARNINGS=$((WARNINGS + 1))
fi

echo ""
echo "Phase 4: Checking library dependencies..."
echo "-----------------------------------------"

# Check if libraries exist
LIBS=("libcrystalline.so" "libalgorithms.so" "libcllm.so")

for lib in "${LIBS[@]}"; do
    if [ -f "$lib" ]; then
        echo -e "${GREEN}✓ Found $lib${NC}"
        
        # Check for math.h symbols in the library (exclude our own big_* and prime_* functions)
        MATH_SYMBOLS=$(nm -D "$lib" 2>/dev/null | grep -E "\ssqrt@|\spow@|\sexp@|\slog@|\ssin@|\scos@|\stan@|\sfloor@|\sceil@|\sfabs@" || true)
        
        if [ -n "$MATH_SYMBOLS" ]; then
            echo -e "${RED}ERROR: $lib contains math.h symbols:${NC}"
            echo "$MATH_SYMBOLS" | head -5
            ERRORS=$((ERRORS + 1))
        else
            echo -e "${GREEN}  ✓ No math.h symbols in $lib${NC}"
        fi
    else
        echo -e "${YELLOW}WARNING: $lib not found (may need to build)${NC}"
        WARNINGS=$((WARNINGS + 1))
    fi
done

echo ""
echo "Phase 5: Checking header file includes..."
echo "-----------------------------------------"

# Check for prime_math.h and prime_float_math.h usage
PRIME_MATH_INCLUDES=$(find src app -type f \( -name "*.c" -o -name "*.h" \) -exec grep -l "prime_math.h\|prime_float_math.h" {} \; 2>/dev/null | wc -l)

echo "Files using crystalline math headers: $PRIME_MATH_INCLUDES"

if [ $PRIME_MATH_INCLUDES -gt 0 ]; then
    echo -e "${GREEN}✓ Crystalline math headers are being used${NC}"
else
    echo -e "${YELLOW}WARNING: No files using crystalline math headers${NC}"
    WARNINGS=$((WARNINGS + 1))
fi

echo ""
echo "=========================================="
echo "VERIFICATION SUMMARY"
echo "=========================================="
echo "Errors: $ERRORS"
echo "Warnings: $WARNINGS"
echo ""

if [ $ERRORS -eq 0 ]; then
    echo -e "${GREEN}✓ OBJECTIVE 5 VERIFICATION PASSED${NC}"
    exit 0
else
    echo -e "${RED}✗ OBJECTIVE 5 VERIFICATION FAILED${NC}"
    exit 1
fi