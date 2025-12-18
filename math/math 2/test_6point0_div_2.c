#include "math/abacus_fractional.h"
#include "math/abacus.h"
#include <stdio.h>

int main() {
    printf("Test: 6.0 / 2\n");
    
    CrystallineAbacus* six = abacus_from_uint64(6, 10);
    CrystallineAbacus* two = abacus_from_uint64(2, 10);
    CrystallineAbacus* result = abacus_new(10);
    
    printf("Calling abacus_div_fractional...\n");
    
    MathError err = abacus_div_fractional(result, six, two, 20);
    
    printf("Returned: %d\n", err);
    
    if (err == MATH_SUCCESS) {
        double val;
        abacus_to_double(result, &val);
        printf("Result: %.15f\n", val);
    }
    
    abacus_free(six);
    abacus_free(two);
    abacus_free(result);
    
    return 0;
}