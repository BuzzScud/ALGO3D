#include "math/abacus.h"
#include "math/abacus_fractional.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void print_abacus(const char* label, const CrystallineAbacus* a) {
    double val;
    if (abacus_to_double(a, &val) == MATH_SUCCESS) {
        printf("%s: %.15f\n", label, val);
    } else {
        printf("%s: [conversion error]\n", label);
    }
}

int main() {
    printf("=== Debugging Newton-Raphson Square Root ===\n\n");
    
    // Test sqrt(10)
    uint32_t base = 10;
    CrystallineAbacus* a = abacus_from_uint64(10, base);
    CrystallineAbacus* x = abacus_from_uint64(5, base);  // Initial guess: 10/2 = 5
    CrystallineAbacus* two = abacus_from_uint64(2, base);
    CrystallineAbacus* quotient = abacus_new(base);
    CrystallineAbacus* sum = abacus_new(base);
    CrystallineAbacus* x_next = abacus_new(base);
    
    printf("Input: 10\n");
    printf("Expected: %.15f\n", sqrt(10.0));
    printf("Initial guess: 5\n\n");
    
    for (uint32_t i = 0; i < 10; i++) {
        printf("Iteration %u:\n", i + 1);
        print_abacus("  x", x);
        
        // quotient = a / x
        MathError err = abacus_div_fractional(quotient, a, x, 20);
        if (err != MATH_SUCCESS) {
            printf("  ERROR: div_fractional failed with code %d\n", err);
            break;
        }
        print_abacus("  a/x", quotient);
        
        // sum = x + quotient
        err = abacus_add(sum, x, quotient);
        if (err != MATH_SUCCESS) {
            printf("  ERROR: add failed with code %d\n", err);
            break;
        }
        print_abacus("  x + a/x", sum);
        
        // x_next = sum / 2
        err = abacus_div_fractional(x_next, sum, two, 20);
        if (err != MATH_SUCCESS) {
            printf("  ERROR: div by 2 failed with code %d\n", err);
            break;
        }
        print_abacus("  (x + a/x)/2", x_next);
        
        // Check convergence
        double x_val, x_next_val;
        abacus_to_double(x, &x_val);
        abacus_to_double(x_next, &x_next_val);
        double change = fabs(x_next_val - x_val);
        printf("  Change: %.2e\n", change);
        
        // Update x for next iteration
        if (x->capacity < x_next->num_beads) {
            free(x->beads);
            x->beads = (AbacusBead*)calloc(x_next->num_beads, sizeof(AbacusBead));
            x->capacity = x_next->num_beads;
        }
        memcpy(x->beads, x_next->beads, x_next->num_beads * sizeof(AbacusBead));
        x->num_beads = x_next->num_beads;
        x->min_exponent = x_next->min_exponent;
        x->negative = x_next->negative;
        
        printf("\n");
        
        if (change < 1e-15) {
            printf("Converged!\n");
            break;
        }
    }
    
    printf("Final result:\n");
    print_abacus("  x", x);
    
    abacus_free(a);
    abacus_free(x);
    abacus_free(two);
    abacus_free(quotient);
    abacus_free(sum);
    abacus_free(x_next);
    
    return 0;
}