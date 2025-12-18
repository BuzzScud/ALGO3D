#include "math/abacus.h"
#include <stdio.h>

void print_beads(const CrystallineAbacus* a, const char* label) {
    printf("%s:\n", label);
    for (size_t i = 0; i < a->num_beads; i++) {
        printf("  bead[%zu]: value=%u, exp=%d\n", 
               i, a->beads[i].value, a->beads[i].weight_exponent);
    }
    double val;
    abacus_to_double(a, &val);
    printf("  As double: %.10f\n", val);
}

int main() {
    printf("=== Testing Addition with Fractional Beads ===\n\n");
    
    // Create 3.0
    CrystallineAbacus* three = abacus_from_uint64(3, 10);
    
    // Create 3.333... using from_double
    CrystallineAbacus* three_point_three = abacus_from_double(3.333333, 10, 6);
    
    // Add them
    CrystallineAbacus* sum = abacus_new(10);
    MathError err = abacus_add(sum, three, three_point_three);
    
    printf("Input 1:\n");
    print_beads(three, "3.0");
    
    printf("\nInput 2:\n");
    print_beads(three_point_three, "3.333...");
    
    printf("\nSum:\n");
    print_beads(sum, "3.0 + 3.333...");
    
    printf("\nExpected: 6.333...\n");
    
    abacus_free(three);
    abacus_free(three_point_three);
    abacus_free(sum);
    
    return 0;
}
