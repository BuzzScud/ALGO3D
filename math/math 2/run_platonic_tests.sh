#!/bin/bash

# Set library path
export LD_LIBRARY_PATH=/workspace:/workspace/algorithms:$LD_LIBRARY_PATH

echo "=================================="
echo "PLATONIC MODEL TEST SUITE"
echo "=================================="
echo ""

# Track results
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# Test array
TESTS=(
    "test_tetrahedron"
    "test_cube"
    "test_octahedron"
    "test_dodecahedron"
    "test_icosahedron"
    "test_blind_recovery"
    "test_harmonic"
)

# Run each test
for test in "${TESTS[@]}"; do
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    echo "Running $test..."
    echo "----------------------------------"
    
    if ./tools/$test; then
        PASSED_TESTS=$((PASSED_TESTS + 1))
        echo "✓ $test PASSED"
    else
        FAILED_TESTS=$((FAILED_TESTS + 1))
        echo "✗ $test FAILED"
    fi
    
    echo ""
done

# Summary
echo "=================================="
echo "TEST SUMMARY"
echo "=================================="
echo "Total tests: $TOTAL_TESTS"
echo "Passed: $PASSED_TESTS"
echo "Failed: $FAILED_TESTS"
echo ""

if [ $FAILED_TESTS -eq 0 ]; then
    echo "✓ ALL TESTS PASSED!"
    exit 0
else
    echo "✗ SOME TESTS FAILED"
    exit 1
fi