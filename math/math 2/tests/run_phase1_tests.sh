#!/bin/bash

# Phase 1 Test Runner
# Builds and runs all Phase 1 tests

set -e

CC=gcc
CFLAGS="-Wall -Wextra -g -O0 -I../include -mavx2 -mfma"
LDFLAGS="-L.. -lcrystalline -lcllm -lm -pthread -Wl,-rpath,'\$ORIGIN/..'"

echo ""
echo "========================================"
echo "  Phase 1 Test Suite"
echo "========================================"
echo ""

# Build libraries first
echo "Building libraries..."
cd ..
make clean > /dev/null 2>&1 || true
make -j$(nproc) > /dev/null 2>&1
cd tests

# Phase 1 tests
PHASE1_TESTS=(
    "unit/test_phase1_day1"
    "unit/test_phase1_day2_simple"
    "unit/test_thread_allocation"
    "unit/test_angular_position"
    "unit/test_message_system"
    "unit/test_control_process"
)

total_tests=0
passed_tests=0
failed_tests=0

for test_path in "${PHASE1_TESTS[@]}"; do
    test_name=$(basename "$test_path")
    test_src="${test_path}.c"
    
    if [ ! -f "$test_src" ]; then
        echo "⚠ Skipping $test_name (source not found)"
        continue
    fi
    
    echo "Building $test_name..."
    if $CC $CFLAGS -o "$test_path" "$test_src" $LDFLAGS 2>&1 | grep -v "warning:"; then
        echo "✓ $test_name built successfully"
        
        echo "Running $test_name..."
        if ./"$test_path"; then
            echo "✓ $test_name PASSED"
            ((passed_tests++))
        else
            echo "✗ $test_name FAILED"
            ((failed_tests++))
        fi
        echo ""
    else
        echo "✗ $test_name build FAILED"
        ((failed_tests++))
        echo ""
    fi
    
    ((total_tests++))
done

echo ""
echo "========================================"
echo "  Test Summary"
echo "========================================"
echo "Total tests:  $total_tests"
echo "Passed:       $passed_tests"
echo "Failed:       $failed_tests"
echo "========================================"
echo ""

if [ $failed_tests -eq 0 ]; then
    echo "✓ All tests passed!"
    exit 0
else
    echo "✗ Some tests failed"
    exit 1
fi