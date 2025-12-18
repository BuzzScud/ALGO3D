#include "math/abacus_fractional.h"
#include "math/abacus.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef fabs
#define fabs(x) __builtin_fabs(x)
#endif

bool abacus_approx_equal(const CrystallineAbacus* abacus, double expected, double epsilon) {
    double actual;
    if (abacus_to_double(abacus, &actual) != MATH_SUCCESS) {
        return false;
    }
    return fabs(actual - expected) < epsilon;
}

void test_div_fractional_simple() {
    printf("\n=== Test: Simple Fractional Division ===\n");
    
    printf("Test 1: 1 / 2 = 0.5\n");
    
    CrystallineAbacus* one = abacus_from_uint64(1, 10);
    CrystallineAbacus* two = abacus_from_uint64(2, 10);
    CrystallineAbacus* result = abacus_new(10);
    
    printf("Calling abacus_div_fractional...\n");
    MathError err = abacus_div_fractional(result, one, two, 10);
    printf("Returned with err=%d\n", err);
    
    if (err == MATH_SUCCESS && abacus_approx_equal(result, 0.5, 0.0001)) {
        printf("  ✓ PASS: Result = 0.5\n");
    } else {
        double actual;
        abacus_to_double(result, &actual);
        printf("  ✗ FAIL: Expected 0.5, got %.10f\n", actual);
    }
    
    abacus_free(one);
    abacus_free(two);
    abacus_free(result);
}

int main() {
    printf("Running first test only...\n");
    test_div_fractional_simple();
    printf("Test complete!\n");
    return 0;
}