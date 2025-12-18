#include "math/abacus_fractional.h"
#include "math/abacus.h"
#include <stdio.h>

int main() {
    printf("Test: 6.0000020816 / 2\n");
    
    // Create 6.0000020816
    CrystallineAbacus* num = abacus_new(10);
    num->num_beads = 11;
    num->beads[0].value = 6;
    num->beads[0].weight_exponent = -10;
    num->beads[1].value = 1;
    num->beads[1].weight_exponent = -9;
    num->beads[2].value = 8;
    num->beads[2].weight_exponent = -8;
    num->beads[3].value = 0;
    num->beads[3].weight_exponent = -7;
    num->beads[4].value = 2;
    num->beads[4].weight_exponent = -6;
    num->beads[5].value = 0;
    num->beads[5].weight_exponent = -5;
    num->beads[6].value = 0;
    num->beads[6].weight_exponent = -4;
    num->beads[7].value = 0;
    num->beads[7].weight_exponent = -3;
    num->beads[8].value = 0;
    num->beads[8].weight_exponent = -2;
    num->beads[9].value = 0;
    num->beads[9].weight_exponent = -1;
    num->beads[10].value = 6;
    num->beads[10].weight_exponent = 0;
    num->min_exponent = -10;
    
    CrystallineAbacus* two = abacus_from_uint64(2, 10);
    CrystallineAbacus* result = abacus_new(10);
    
    double num_val;
    abacus_to_double(num, &num_val);
    printf("Input: %.15f\n", num_val);
    
    printf("Calling abacus_div_fractional...\n");
    
    MathError err = abacus_div_fractional(result, num, two, 20);
    
    printf("Returned: %d\n", err);
    
    if (err == MATH_SUCCESS) {
        double val;
        abacus_to_double(result, &val);
        printf("Result: %.15f\n", val);
        printf("Expected: %.15f\n", num_val / 2.0);
    }
    
    abacus_free(num);
    abacus_free(two);
    abacus_free(result);
    
    return 0;
}