#include "math/abacus_fractional.h"
#include "math/abacus.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("Test: sqrt(4) = 2.0\n");
    
    CrystallineAbacus* four = abacus_from_uint64(4, 10);
    CrystallineAbacus* result = abacus_new(10);
    
    printf("Calling abacus_sqrt_pure...\n");
    MathError err = abacus_sqrt_pure(result, four, 10);
    printf("Returned with err=%d\n", err);
    
    if (err == MATH_SUCCESS) {
        double val;
        abacus_to_double(result, &val);
        printf("Result: %.10f\n", val);
    } else {
        printf("ERROR: %d\n", err);
    }
    
    abacus_free(four);
    abacus_free(result);
    
    return 0;
}