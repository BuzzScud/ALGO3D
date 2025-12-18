#include "math/abacus_fractional.h"
#include "math/abacus.h"
#include <stdio.h>

int main() {
    printf("Test: 9 / 4\n");
    
    CrystallineAbacus* nine = abacus_from_uint64(9, 10);
    CrystallineAbacus* four = abacus_from_uint64(4, 10);
    CrystallineAbacus* result = abacus_new(10);
    
    printf("Calling abacus_div_fractional...\n");
    MathError err = abacus_div_fractional(result, nine, four, 10);
    printf("Returned: %d\n", err);
    
    if (err == MATH_SUCCESS) {
        double val;
        abacus_to_double(result, &val);
        printf("Result: %.10f\n", val);
    }
    
    abacus_free(nine);
    abacus_free(four);
    abacus_free(result);
    
    return 0;
}