#include "math/abacus.h"
#include "math/abacus_fractional.h"
#include <stdio.h>

int main() {
    printf("Test 1: Simple integer division 10 / 5\n");
    CrystallineAbacus* ten = abacus_from_uint64(10, 10);
    CrystallineAbacus* five = abacus_from_uint64(5, 10);
    CrystallineAbacus* result = abacus_new(10);
    
    printf("Before div_fractional\n");
    MathError err = abacus_div_fractional(result, ten, five, 5);
    printf("After div_fractional: err=%d\n", err);
    
    if (err == MATH_SUCCESS) {
        double val;
        abacus_to_double(result, &val);
        printf("Result: %.10f\n", val);
    }
    
    abacus_free(ten);
    abacus_free(five);
    abacus_free(result);
    
    printf("\nTest 2: Fractional division 1 / 2\n");
    CrystallineAbacus* one = abacus_from_uint64(1, 10);
    CrystallineAbacus* two = abacus_from_uint64(2, 10);
    result = abacus_new(10);
    
    printf("Before div_fractional\n");
    err = abacus_div_fractional(result, one, two, 5);
    printf("After div_fractional: err=%d\n", err);
    
    if (err == MATH_SUCCESS) {
        double val;
        abacus_to_double(result, &val);
        printf("Result: %.10f\n", val);
    }
    
    abacus_free(one);
    abacus_free(two);
    abacus_free(result);
    
    return 0;
}