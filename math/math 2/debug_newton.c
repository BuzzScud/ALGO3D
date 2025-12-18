#include "math/abacus_fractional.h"
#include "math/abacus.h"
#include <stdio.h>

int main() {
    printf("=== Debugging Newton-Raphson ===\n\n");
    
    CrystallineAbacus* a = abacus_from_uint64(10, 10);
    CrystallineAbacus* x = abacus_from_uint64(3, 10);  // Initial guess
    CrystallineAbacus* two = abacus_from_uint64(2, 10);
    CrystallineAbacus* quotient = abacus_new(10);
    CrystallineAbacus* sum = abacus_new(10);
    CrystallineAbacus* x_next = abacus_new(10);
    
    printf("Initial: x = ");
    abacus_print(x);
    
    for (int i = 0; i < 3; i++) {
        printf("\nIteration %d:\n", i + 1);
        
        // quotient = a / x
        MathError err = abacus_div_fractional(quotient, a, x, 10);
        printf("  a / x = ");
        abacus_print(quotient);
        
        // sum = x + quotient
        err = abacus_add(sum, x, quotient);
        printf("  x + (a/x) = ");
        abacus_print(sum);
        
        // x_next = sum / 2
        err = abacus_div_fractional(x_next, sum, two, 10);
        printf("  (x + a/x) / 2 = ");
        abacus_print(x_next);
        
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
        
        printf("  Updated x = ");
        abacus_print(x);
    }
    
    abacus_free(a);
    abacus_free(x);
    abacus_free(two);
    abacus_free(quotient);
    abacus_free(sum);
    abacus_free(x_next);
    
    return 0;
}