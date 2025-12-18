#include "math/abacus_fractional.h"
#include "math/abacus.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    printf("=== Detailed Newton-Raphson Debug ===\n\n");
    
    CrystallineAbacus* a = abacus_from_uint64(10, 10);
    CrystallineAbacus* x = abacus_from_uint64(3, 10);
    CrystallineAbacus* two = abacus_from_uint64(2, 10);
    
    printf("Target: sqrt(10) = 3.162277660...\n");
    printf("Initial guess: x = 3\n\n");
    
    for (int iter = 0; iter < 3; iter++) {
        printf("--- Iteration %d ---\n", iter + 1);
        print_beads(x, "x");
        
        // Step 1: quotient = a / x
        CrystallineAbacus* quotient = abacus_new(10);
        MathError err = abacus_div_fractional(quotient, a, x, 10);
        print_beads(quotient, "a/x");
        
        // Step 2: sum = x + quotient
        CrystallineAbacus* sum = abacus_new(10);
        err = abacus_add(sum, x, quotient);
        print_beads(sum, "x + a/x");
        
        // Step 3: x_next = sum / 2
        CrystallineAbacus* x_next = abacus_new(10);
        err = abacus_div_fractional(x_next, sum, two, 10);
        print_beads(x_next, "(x + a/x)/2");
        
        // Update x
        if (x->capacity < x_next->num_beads) {
            free(x->beads);
            x->beads = (AbacusBead*)calloc(x_next->num_beads, sizeof(AbacusBead));
            x->capacity = x_next->num_beads;
        }
        memcpy(x->beads, x_next->beads, x_next->num_beads * sizeof(AbacusBead));
        x->num_beads = x_next->num_beads;
        x->min_exponent = x_next->min_exponent;
        x->negative = x_next->negative;
        
        abacus_free(quotient);
        abacus_free(sum);
        abacus_free(x_next);
        
        printf("\n");
    }
    
    print_beads(x, "Final x");
    
    abacus_free(a);
    abacus_free(x);
    abacus_free(two);
    
    return 0;
}
