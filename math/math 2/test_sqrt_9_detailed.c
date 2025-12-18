#include "math/abacus_fractional.h"
#include "math/abacus.h"
#include <stdio.h>
#include <string.h>

int main() {
    printf("Test: sqrt(9) with detailed logging\n");
    
    uint32_t base = 10;
    CrystallineAbacus* a = abacus_from_uint64(9, base);
    
    /* Initial guess */
    uint64_t a_uint;
    MathError err = abacus_to_uint64(a, &a_uint);
    printf("a_uint = %lu, err = %d\n", a_uint, err);
    
    uint64_t guess_uint = a_uint / 2;
    printf("Initial guess: %lu\n", guess_uint);
    
    CrystallineAbacus* x = abacus_from_uint64(guess_uint, base);
    CrystallineAbacus* two = abacus_from_uint64(2, base);
    CrystallineAbacus* quotient = abacus_new(base);
    CrystallineAbacus* sum = abacus_new(base);
    CrystallineAbacus* x_next = abacus_new(base);
    
    printf("Starting Newton-Raphson iterations...\n");
    
    for (uint32_t i = 0; i < 3; i++) {
        printf("\nIteration %u:\n", i + 1);
        
        double x_val;
        abacus_to_double(x, &x_val);
        printf("  x = %.10f\n", x_val);
        
        /* quotient = a / x */
        printf("  Computing a / x...\n");
        fflush(stdout);
        err = abacus_div_fractional(quotient, a, x, 20);
        printf("  a / x returned: %d\n", err);
        fflush(stdout);
        
        if (err != MATH_SUCCESS) {
            printf("  ERROR in division!\n");
            break;
        }
        
        double q_val;
        abacus_to_double(quotient, &q_val);
        printf("  quotient = %.10f\n", q_val);
        
        /* sum = x + quotient */
        printf("  Computing x + quotient...\n");
        fflush(stdout);
        err = abacus_add(sum, x, quotient);
        printf("  x + quotient returned: %d\n", err);
        fflush(stdout);
        
        if (err != MATH_SUCCESS) {
            printf("  ERROR in addition!\n");
            break;
        }
        
        double sum_val;
        abacus_to_double(sum, &sum_val);
        printf("  sum = %.10f\n", sum_val);
        
        /* x_next = sum / 2 */
        printf("  Computing sum / 2...\n");
        fflush(stdout);
        err = abacus_div_fractional(x_next, sum, two, 20);
        printf("  sum / 2 returned: %d\n", err);
        fflush(stdout);
        
        if (err != MATH_SUCCESS) {
            printf("  ERROR in division by 2!\n");
            break;
        }
        
        double x_next_val;
        abacus_to_double(x_next, &x_next_val);
        printf("  x_next = %.10f\n", x_next_val);
        
        /* Update x */
        printf("  Updating x...\n");
        fflush(stdout);
        if (x->capacity < x_next->num_beads) {
            free(x->beads);
            x->beads = (AbacusBead*)calloc(x_next->num_beads, sizeof(AbacusBead));
            x->capacity = x_next->num_beads;
        }
        memcpy(x->beads, x_next->beads, x_next->num_beads * sizeof(AbacusBead));
        x->num_beads = x_next->num_beads;
        x->min_exponent = x_next->min_exponent;
        x->negative = x_next->negative;
        printf("  x updated\n");
        fflush(stdout);
    }
    
    printf("\nFinal result:\n");
    double final_val;
    abacus_to_double(x, &final_val);
    printf("  sqrt(9) = %.10f\n", final_val);
    
    abacus_free(a);
    abacus_free(x);
    abacus_free(two);
    abacus_free(quotient);
    abacus_free(sum);
    abacus_free(x_next);
    
    return 0;
}