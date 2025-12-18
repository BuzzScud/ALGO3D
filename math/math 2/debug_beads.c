#include "math/abacus_fractional.h"
#include "math/abacus.h"
#include <stdio.h>

void print_beads(const CrystallineAbacus* a, const char* label) {
    printf("%s:\n", label);
    printf("  num_beads: %zu, min_exponent: %d\n", a->num_beads, a->min_exponent);
    for (size_t i = 0; i < a->num_beads; i++) {
        printf("  bead[%zu]: value=%u, exponent=%d\n", 
               i, a->beads[i].value, a->beads[i].weight_exponent);
    }
}

int main() {
    printf("=== Testing Fractional Division Beads ===\n\n");
    
    CrystallineAbacus* ten = abacus_from_uint64(10, 10);
    CrystallineAbacus* three = abacus_from_uint64(3, 10);
    CrystallineAbacus* result = abacus_new(10);
    
    printf("Input:\n");
    print_beads(ten, "10");
    print_beads(three, "3");
    
    MathError err = abacus_div_fractional(result, ten, three, 5);
    
    printf("\nResult of 10 / 3:\n");
    print_beads(result, "result");
    
    double val;
    abacus_to_double(result, &val);
    printf("\nAs double: %.10f\n", val);
    
    abacus_free(ten);
    abacus_free(three);
    abacus_free(result);
    
    return 0;
}
