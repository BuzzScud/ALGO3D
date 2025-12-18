#include "math/abacus.h"
#include "math/abacus_fractional.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_abacus_detailed(const char* label, const CrystallineAbacus* a) {
    printf("%s:\n", label);
    printf("  num_beads: %zu\n", a->num_beads);
    printf("  min_exponent: %d\n", a->min_exponent);
    printf("  negative: %d\n", a->negative);
    printf("  beads: ");
    for (size_t i = 0; i < a->num_beads && i < 10; i++) {
        printf("[%u @ exp=%d] ", a->beads[i].value, a->beads[i].weight_exponent);
    }
    printf("\n");
    
    double val;
    if (abacus_to_double(a, &val) == MATH_SUCCESS) {
        printf("  value: %.15f\n", val);
    }
}

int main() {
    printf("=== Testing abacus_div_fractional ===\n\n");
    
    // Test: 7 / 2 = 3.5
    printf("Test 1: 7 / 2 = 3.5\n");
    CrystallineAbacus* seven = abacus_from_uint64(7, 10);
    CrystallineAbacus* two = abacus_from_uint64(2, 10);
    CrystallineAbacus* result = abacus_new(10);
    
    print_abacus_detailed("Input 7", seven);
    print_abacus_detailed("Input 2", two);
    
    MathError err = abacus_div_fractional(result, seven, two, 10);
    if (err == MATH_SUCCESS) {
        print_abacus_detailed("Result", result);
    } else {
        printf("ERROR: %d\n", err);
    }
    
    abacus_free(seven);
    abacus_free(two);
    abacus_free(result);
    
    printf("\n");
    
    // Test: 6.333... / 2 = 3.166...
    printf("Test 2: 6.333... / 2\n");
    CrystallineAbacus* six = abacus_from_uint64(6, 10);
    CrystallineAbacus* third = abacus_new(10);
    
    // Create 0.333... manually
    third->num_beads = 3;
    third->beads[0].value = 3;
    third->beads[0].weight_exponent = -1;
    third->beads[1].value = 3;
    third->beads[1].weight_exponent = -2;
    third->beads[2].value = 3;
    third->beads[2].weight_exponent = -3;
    third->min_exponent = -3;
    
    CrystallineAbacus* sum = abacus_new(10);
    err = abacus_add(sum, six, third);
    
    print_abacus_detailed("Input 6.333...", sum);
    
    two = abacus_from_uint64(2, 10);
    result = abacus_new(10);
    
    err = abacus_div_fractional(result, sum, two, 10);
    if (err == MATH_SUCCESS) {
        print_abacus_detailed("Result", result);
    } else {
        printf("ERROR: %d\n", err);
    }
    
    abacus_free(six);
    abacus_free(third);
    abacus_free(sum);
    abacus_free(two);
    abacus_free(result);
    
    return 0;
}