#include "math/abacus_fractional.h"
#include "math/abacus.h"
#include <stdio.h>

int main() {
    printf("Test: 6.25 / 2 using abacus_div_fractional\n");
    
    // Create 6.25 = 6 + 0.25
    CrystallineAbacus* num = abacus_new(10);
    num->num_beads = 3;
    num->beads[0].value = 6;
    num->beads[0].weight_exponent = 0;
    num->beads[1].value = 2;
    num->beads[1].weight_exponent = -1;
    num->beads[2].value = 5;
    num->beads[2].weight_exponent = -2;
    num->min_exponent = -2;
    
    CrystallineAbacus* two = abacus_from_uint64(2, 10);
    CrystallineAbacus* result = abacus_new(10);
    
    double num_val;
    abacus_to_double(num, &num_val);
    printf("Input: %.10f\n", num_val);
    
    printf("Calling abacus_div_fractional...\n");
    fflush(stdout);
    
    MathError err = abacus_div_fractional(result, num, two, 20);
    
    printf("Returned: %d\n", err);
    
    if (err == MATH_SUCCESS) {
        double val;
        abacus_to_double(result, &val);
        printf("6.25 / 2 = %.15f\n", val);
    } else {
        printf("ERROR: %d\n", err);
    }
    
    abacus_free(num);
    abacus_free(two);
    abacus_free(result);
    
    return 0;
}