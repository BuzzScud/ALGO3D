#include "math/abacus_fractional.h"
#include "math/abacus.h"
#include <stdio.h>

void print_beads(const CrystallineAbacus* a, const char* label) {
    printf("%s: ", label);
    for (size_t i = 0; i < a->num_beads; i++) {
        printf("[%u@%d] ", a->beads[i].value, a->beads[i].weight_exponent);
    }
    double val;
    abacus_to_double(a, &val);
    printf(" = %.10f\n", val);
}

int main() {
    printf("=== Testing 6.333... / 2 ===\n\n");
    
    // Create 6.333...
    CrystallineAbacus* dividend = abacus_from_double(6.333333, 10, 6);
    CrystallineAbacus* two = abacus_from_uint64(2, 10);
    CrystallineAbacus* result = abacus_new(10);
    
    print_beads(dividend, "Dividend (6.333...)");
    print_beads(two, "Divisor (2)");
    
    MathError err = abacus_div_fractional(result, dividend, two, 10);
    
    printf("\nAfter division:\n");
    print_beads(result, "Result");
    
    printf("\nExpected: 3.1666... (6.333.../2)\n");
    
    abacus_free(dividend);
    abacus_free(two);
    abacus_free(result);
    
    return 0;
}
