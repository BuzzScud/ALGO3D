/**
 * @file test_abacus_simple.c
 * @brief Simple test to debug abacus multiplication
 */

#include <stdio.h>
#include <stdlib.h>
#include "../../math/include/math/abacus.h"

int main(void) {
    printf("Testing basic CrystallineAbacus operations...\n\n");
    
    // Test 1: Create abacus from double
    printf("Test 1: Create from double\n");
    CrystallineAbacus* a = abacus_from_double(2.0, 12, 10);
    CrystallineAbacus* b = abacus_from_double(3.0, 12, 10);
    
    if (!a || !b) {
        printf("  FAIL: Could not create abacus\n");
        return 1;
    }
    
    double a_val, b_val;
    abacus_to_double(a, &a_val);
    abacus_to_double(b, &b_val);
    printf("  a = %.10f\n", a_val);
    printf("  b = %.10f\n", b_val);
    
    // Test 2: Multiply
    printf("\nTest 2: Multiply a * b\n");
    CrystallineAbacus* product = abacus_new(12);
    abacus_set_precision(product, 10);
    
    MathError err = abacus_mul(product, a, b);
    if (err != MATH_SUCCESS) {
        printf("  FAIL: Multiplication error: %d\n", err);
        return 1;
    }
    
    double product_val;
    abacus_to_double(product, &product_val);
    printf("  a * b = %.10f (expected 6.0)\n", product_val);
    
    if (product_val < 5.9 || product_val > 6.1) {
        printf("  FAIL: Incorrect result\n");
        return 1;
    }
    
    printf("  PASS\n");
    
    // Test 3: Add
    printf("\nTest 3: Add a + b\n");
    CrystallineAbacus* sum = abacus_new(12);
    abacus_set_precision(sum, 10);
    
    err = abacus_add(sum, a, b);
    if (err != MATH_SUCCESS) {
        printf("  FAIL: Addition error: %d\n", err);
        return 1;
    }
    
    double sum_val;
    abacus_to_double(sum, &sum_val);
    printf("  a + b = %.10f (expected 5.0)\n", sum_val);
    
    if (sum_val < 4.9 || sum_val > 5.1) {
        printf("  FAIL: Incorrect result\n");
        return 1;
    }
    
    printf("  PASS\n");
    
    // Test 4: Multiply with fractional
    printf("\nTest 4: Multiply with fractional\n");
    CrystallineAbacus* c = abacus_from_double(1.5, 12, 10);
    CrystallineAbacus* d = abacus_from_double(2.5, 12, 10);
    CrystallineAbacus* product2 = abacus_new(12);
    abacus_set_precision(product2, 10);
    
    err = abacus_mul(product2, c, d);
    if (err != MATH_SUCCESS) {
        printf("  FAIL: Multiplication error: %d\n", err);
        return 1;
    }
    
    double product2_val;
    abacus_to_double(product2, &product2_val);
    printf("  1.5 * 2.5 = %.10f (expected 3.75)\n", product2_val);
    
    if (product2_val < 3.74 || product2_val > 3.76) {
        printf("  FAIL: Incorrect result\n");
        return 1;
    }
    
    printf("  PASS\n");
    
    // Cleanup
    abacus_free(a);
    abacus_free(b);
    abacus_free(c);
    abacus_free(d);
    abacus_free(product);
    abacus_free(product2);
    abacus_free(sum);
    
    printf("\n✓ All basic tests passed!\n");
    return 0;
}