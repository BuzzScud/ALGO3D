#include "math/abacus_fractional.h"
#include "math/abacus.h"
#include <stdio.h>

int main() {
    printf("Test: sqrt(9)\n");
    
    CrystallineAbacus* nine = abacus_from_uint64(9, 10);
    CrystallineAbacus* result = abacus_new(10);
    
    printf("Calling abacus_sqrt_pure with 5 iterations...\n");
    MathError err = abacus_sqrt_pure(result, nine, 5);
    printf("Returned: %d\n", err);
    
    if (err == MATH_SUCCESS) {
        double val;
        abacus_to_double(result, &val);
        printf("Result: %.10f\n", val);
    }
    
    abacus_free(nine);
    abacus_free(result);
    
    return 0;
}