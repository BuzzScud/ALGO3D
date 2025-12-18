#!/bin/bash

# Script to run all algorithm tests
cd /workspace/algorithms/tests
export LD_LIBRARY_PATH=/workspace:/workspace/algorithms:$LD_LIBRARY_PATH

echo "========================================"
echo "Running All Algorithm Tests"
echo "========================================"
echo ""

total_tests=0
passed_tests=0
failed_tests=0

# Array of test executables
tests=(
    "test_sphere_threading"
    "test_visualization"
    "test_memory_management"
    "test_threading"
    "test_shared_memory"
    "test_lock_free_queue"
    "test_sphere_packing"
    "test_hierarchical_primes"
    "test_hierarchical_structures"
    "test_batch_processing"
)

# Run each test
for test in "${tests[@]}"; do
    if [ -x "./$test" ]; then
        echo "----------------------------------------"
        echo "Running: $test"
        echo "----------------------------------------"
        if ./$test; then
            ((passed_tests++))
        else
            ((failed_tests++))
            echo "❌ FAILED: $test"
        fi
        ((total_tests++))
        echo ""
    else
        echo "⚠️  Test not found or not executable: $test"
        echo ""
    fi
done

echo "========================================"
echo "Test Summary"
echo "========================================"
echo "Total tests run: $total_tests"
echo "Passed: $passed_tests"
echo "Failed: $failed_tests"
echo ""

if [ $failed_tests -eq 0 ]; then
    echo "✅ All tests passed!"
    exit 0
else
    echo "❌ Some tests failed"
    exit 1
fi