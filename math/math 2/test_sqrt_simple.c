#include "math/abacus_fractional.h"
#include "math/abacus.h"
#include <stdio.h>

int main() {
    printf("Test: sqrt(4) with 10 iterations\n");
    
    CrystallineAbacus* four = abacus_from_uint64(4, 10);
    CrystallineAbacus* result = abacus_new(10);
    
    MathError err = abacus_sqrt_pure(result, four, 10);
    printf("Error: %d\n", err);
    
    if (err == MATH_SUCCESS) {
        double val;
        abacus_to_double(result, &val);
        printf("sqrt(4) = %.15f\n", val);
    }
    
    abacus_free(four);
    abacus_free(result);
    
    printf("\nTest: sqrt(9) with 10 iterations\n");
    
    CrystallineAbacus* nine = abacus_from_uint64(9, 10);
    result = abacus_new(10);
    
    printf("Calling abacus_sqrt_pure...\n");
    fflush(stdout);
    err = abacus_sqrt_pure(result, nine, 10);
    printf("Returned with error: %d\n", err);
    
    if (err == MATH_SUCCESS) {
        double val;
        abacus_to_double(result, &val);
        printf("sqrt(9) = %.15f\n", val);
    }
    
    abacus_free(nine);
    abacus_free(result);
    
    return 0;
}