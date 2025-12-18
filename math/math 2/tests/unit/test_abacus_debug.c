/**
 * @file test_abacus_debug.c
 * @brief Debug test to understand abacus behavior
 */

#include <stdio.h>
#include <stdlib.h>
#include "../../math/include/math/abacus.h"

void print_abacus_details(const char* name, CrystallineAbacus* abacus) {
    printf("%s:\n", name);
    printf("  base: %u\n", abacus->base);
    printf("  num_beads: %zu\n", abacus->num_beads);
    printf("  negative: %d\n", abacus->negative);
    printf("  min_exponent: %d\n", abacus->min_exponent);
    printf("  max_exponent: %d\n", abacus->max_exponent);
    printf("  is_sparse: %d\n", abacus->is_sparse);
    
    double val;
    abacus_to_double(abacus, &val);
    printf("  value: %.10f\n", val);
    
    // Print string representation
    char* str = abacus_to_string(abacus);
    if (str) {
        printf("  string: %s\n", str);
        free(str);
    }
    printf("\n");
}

int main(void) {
    printf("=== Debugging CrystallineAbacus Multiplication ===\n\n");
    
    // Test with integers first
    printf("Test 1: Integer multiplication (2 * 3)\n");
    CrystallineAbacus* a = abacus_from_uint64(2, 12);
    CrystallineAbacus* b = abacus_from_uint64(3, 12);
    
    print_abacus_details("a (integer 2)", a);
    print_abacus_details("b (integer 3)", b);
    
    CrystallineAbacus* result = abacus_new(12);
    MathError err = abacus_mul(result, a, b);
    
    printf("Multiplication result (err=%d):\n", err);
    print_abacus_details("result", result);
    
    double result_val;
    abacus_to_double(result, &result_val);
    printf("Expected: 6.0, Got: %.10f\n\n", result_val);
    
    abacus_free(a);
    abacus_free(b);
    abacus_free(result);
    
    // Test with fractional numbers
    printf("Test 2: Fractional multiplication (2.0 * 3.0 with precision)\n");
    a = abacus_from_double(2.0, 12, 10);
    b = abacus_from_double(3.0, 12, 10);
    
    print_abacus_details("a (double 2.0, precision 10)", a);
    print_abacus_details("b (double 3.0, precision 10)", b);
    
    result = abacus_new(12);
    abacus_set_precision(result, 10);
    
    err = abacus_mul(result, a, b);
    
    printf("Multiplication result (err=%d):\n", err);
    print_abacus_details("result", result);
    
    abacus_to_double(result, &result_val);
    printf("Expected: 6.0, Got: %.10f\n\n", result_val);
    
    abacus_free(a);
    abacus_free(b);
    abacus_free(result);
    
    return 0;
}