#!/bin/bash

echo "========================================"
echo "  Crystalline CLLM Test Suite Runner"
echo "========================================"
echo ""

TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# Function to run a test
run_test() {
    local test_name=$1
    
    if [ ! -f "tests/$test_name" ]; then
        echo "⊘ SKIP: $test_name (not built)"
        return
    fi
    
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    
    echo "Running: $test_name"
    if (cd tests && LD_LIBRARY_PATH=..:../algorithms:$LD_LIBRARY_PATH ./$test_name > /dev/null 2>&1); then
        echo "✓ PASS: $test_name"
        PASSED_TESTS=$((PASSED_TESTS + 1))
    else
        echo "✗ FAIL: $test_name"
        FAILED_TESTS=$((FAILED_TESTS + 1))
    fi
    echo ""
}

echo "=== Unit Tests ==="
echo ""
run_test "unit/test_softmax_backward"
run_test "unit/test_attention_cache"

echo "=== Integration Tests ==="
echo ""
run_test "integration/test_forward_backward"
run_test "integration/test_lr_scheduling"

echo "=== Diagnostic Tests ==="
echo ""
run_test "test_simple_init"
run_test "test_token_init"

echo "========================================"
echo "  Test Results Summary"
echo "========================================"
echo "Total Tests:  $TOTAL_TESTS"
echo "Passed:       $PASSED_TESTS"
echo "Failed:       $FAILED_TESTS"
echo ""

if [ $FAILED_TESTS -eq 0 ]; then
    echo "✓ All tests passed!"
    exit 0
else
    echo "✗ Some tests failed"
    exit 1
fi