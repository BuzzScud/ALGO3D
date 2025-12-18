#!/bin/bash

# OBJECTIVE 28: Deployment Test Suite
# Comprehensive testing before production deployment

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Test results tracking
TESTS_PASSED=0
TESTS_FAILED=0
TESTS_TOTAL=0

# Function to print section headers
print_header() {
    echo -e "\n${BLUE}╔════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${BLUE}║  $1${NC}"
    echo -e "${BLUE}╚════════════════════════════════════════════════════════════╝${NC}\n"
}

# Function to print test results
print_result() {
    local test_name=$1
    local result=$2
    TESTS_TOTAL=$((TESTS_TOTAL + 1))
    
    if [ "$result" = "PASS" ]; then
        echo -e "${GREEN}✓${NC} $test_name"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} $test_name"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
}

# Function to run a test
run_test() {
    local test_name=$1
    local test_command=$2
    
    echo -e "${YELLOW}Running:${NC} $test_name"
    if eval "$test_command" > /dev/null 2>&1; then
        print_result "$test_name" "PASS"
        return 0
    else
        print_result "$test_name" "FAIL"
        return 1
    fi
}

# Start testing
echo -e "${BLUE}╔════════════════════════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║  OBJECTIVE 28: DEPLOYMENT TEST SUITE                      ║${NC}"
echo -e "${BLUE}║  Geometric Recovery Algorithm - Production Readiness       ║${NC}"
echo -e "${BLUE}╚════════════════════════════════════════════════════════════╝${NC}"

# Change to reference implementation directory
cd "$(dirname "$0")"

# Test 1: Build System
print_header "Phase 1: Build System Verification"

echo "Cleaning previous builds..."
make clean > /dev/null 2>&1 || true

echo "Building library..."
if make lib > build_test.log 2>&1; then
    print_result "Library Build" "PASS"
else
    print_result "Library Build" "FAIL"
    echo "Build log:"
    tail -20 build_test.log
    exit 1
fi

# Test 2: Library Verification
print_header "Phase 2: Library Verification"

if [ -f "lib/libgeometric_recovery.a" ]; then
    print_result "Static Library Created" "PASS"
else
    print_result "Static Library Created" "FAIL"
    exit 1
fi

# Check library symbols
if nm lib/libgeometric_recovery.a | grep -q "geometric_recovery"; then
    print_result "Library Symbols Present" "PASS"
else
    print_result "Library Symbols Present" "FAIL"
fi

# Test 3: Test Data Generation
print_header "Phase 3: Test Data Generation"

echo "Building test data generators..."
cd tools
make clean > /dev/null 2>&1 || true
if make > ../tools_build.log 2>&1; then
    print_result "Test Tools Build" "PASS"
else
    print_result "Test Tools Build" "FAIL"
    cat ../tools_build.log
    exit 1
fi
cd ..

# Generate fresh test data
echo "Generating test data for 64-bit keys..."
if ./tools/generate_comprehensive_test_data 64 10 > /dev/null 2>&1; then
    print_result "64-bit Test Data Generation" "PASS"
else
    print_result "64-bit Test Data Generation" "FAIL"
fi

echo "Generating test data for 128-bit keys..."
if ./tools/generate_comprehensive_test_data 128 10 > /dev/null 2>&1; then
    print_result "128-bit Test Data Generation" "PASS"
else
    print_result "128-bit Test Data Generation" "FAIL"
fi

echo "Generating test data for 256-bit keys..."
if ./tools/generate_comprehensive_test_data 256 10 > /dev/null 2>&1; then
    print_result "256-bit Test Data Generation" "PASS"
else
    print_result "256-bit Test Data Generation" "FAIL"
fi

# Test 4: Core Algorithm Tests
print_header "Phase 4: Core Algorithm Validation"

echo "Running comprehensive recovery tests..."
if ./tools/test_real_recovery_comprehensive > test_results/deployment_test_output.txt 2>&1; then
    print_result "Comprehensive Recovery Tests" "PASS"
    
    # Check capture rates
    if grep -q "MEETS TARGET" test_results/deployment_test_output.txt; then
        print_result "Target Capture Rate (>60%)" "PASS"
    else
        print_result "Target Capture Rate (>60%)" "FAIL"
    fi
else
    print_result "Comprehensive Recovery Tests" "FAIL"
    tail -50 test_results/deployment_test_output.txt
fi

# Test 5: Micro-Model Integration
print_header "Phase 5: Micro-Model Integration"

if [ -f "test_model.bin" ]; then
    print_result "Micro-Model File Present" "PASS"
else
    print_result "Micro-Model File Present" "FAIL"
fi

# Check if micro-model is being used in tests
if grep -q "Using micro-model" test_results/deployment_test_output.txt; then
    print_result "Micro-Model Integration Active" "PASS"
else
    print_result "Micro-Model Integration Active" "FAIL"
fi

# Test 6: Performance Benchmarks
print_header "Phase 6: Performance Verification"

# Check for performance improvements in output
if grep -q "improvement" test_results/deployment_test_output.txt; then
    print_result "Performance Metrics Captured" "PASS"
else
    print_result "Performance Metrics Captured" "FAIL"
fi

# Test 7: Documentation Verification
print_header "Phase 7: Documentation Check"

required_docs=(
    "README.md"
    "ALGORITHM_EXPLAINED.md"
    "ARCHITECTURE.md"
    "FINAL_SUMMARY.md"
)

for doc in "${required_docs[@]}"; do
    if [ -f "$doc" ]; then
        print_result "Documentation: $doc" "PASS"
    else
        print_result "Documentation: $doc" "FAIL"
    fi
done

# Test 8: API Verification
print_header "Phase 8: API Completeness"

# Check for key API functions in headers
if [ -f "include/geometric_recovery.h" ]; then
    print_result "Main Header Present" "PASS"
    
    # Check for essential functions
    if grep -q "geometric_recovery_init" include/geometric_recovery.h; then
        print_result "API: Init Function" "PASS"
    else
        print_result "API: Init Function" "FAIL"
    fi
    
    if grep -q "geometric_recovery_process" include/geometric_recovery.h; then
        print_result "API: Process Function" "PASS"
    else
        print_result "API: Process Function" "FAIL"
    fi
else
    print_result "Main Header Present" "FAIL"
fi

# Final Summary
print_header "DEPLOYMENT TEST SUMMARY"

echo -e "Total Tests:  ${TESTS_TOTAL}"
echo -e "Passed:       ${GREEN}${TESTS_PASSED}${NC}"
echo -e "Failed:       ${RED}${TESTS_FAILED}${NC}"

if [ $TESTS_FAILED -eq 0 ]; then
    echo -e "\n${GREEN}╔════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${GREEN}║  ✓ ALL TESTS PASSED - READY FOR DEPLOYMENT                ║${NC}"
    echo -e "${GREEN}╚════════════════════════════════════════════════════════════╝${NC}\n"
    exit 0
else
    echo -e "\n${RED}╔════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${RED}║  ✗ SOME TESTS FAILED - REVIEW REQUIRED                    ║${NC}"
    echo -e "${RED}╚════════════════════════════════════════════════════════════╝${NC}\n"
    exit 1
fi