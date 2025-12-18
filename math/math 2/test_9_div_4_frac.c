#include "math/abacus_fractional.h"
#include "math/abacus.h"
#include <stdio.h>

int main() {
    printf("Test: 9 / 4 using abacus_div_fractional\n");
    
    CrystallineAbacus* nine = abacus_from_uint64(9, 10);
    CrystallineAbacus* four = abacus_from_uint64(4, 10);
    CrystallineAbacus* result = abacus_new(10);
    
    printf("Calling abacus_div_fractional(9, 4, 20)...\n");
    fflush(stdout);
    
    MathError err = abacus_div_fractional(result, nine, four, 20);
    
    printf("Returned: %d\n", err);
    
    if (err == MATH_SUCCESS) {
        double val;
        abacus_to_double(result, &val);
        printf("9 / 4 = %.15f\n", val);
    } else {
        printf("ERROR: %d\n", err);
    }
    
    abacus_free(nine);
    abacus_free(four);
    abacus_free(result);
    
    return 0;
}