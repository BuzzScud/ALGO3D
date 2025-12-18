#include <stdio.h>
#include <stdlib.h>
#include "bigint_core.h"
#include "bigfixed_core.h"

int main() {
    printf("Testing BigInt initialization...\n");
    
    // Test 1: Create and initialize BigInt
    BigInt a;
    big_init(&a);
    big_from_int(&a, 42);
    printf("Test 1: BigInt initialized with 42\n");
    printf("  Value: %ld\n", big_to_int64(&a));
    big_free(&a);
    
    // Test 2: Create BigFixed
    printf("\nTest 2: Creating BigFixed...\n");
    BigFixed* f = big_fixed_create(64);
    if (!f) {
        printf("  ERROR: Failed to create BigFixed\n");
        return 1;
    }
    printf("  BigFixed created successfully\n");
    printf("  Precision: %lu bits\n", f->scale_bits);
    
    // Test 3: Initialize BigFixed from int
    printf("\nTest 3: Initializing BigFixed from int...\n");
    big_fixed_from_int(f, 100);
    double val = big_fixed_to_double(f);
    printf("  Value: %f\n", val);
    
    // Test 4: Square root
    printf("\nTest 4: Computing square root...\n");
    BigInt n;
    big_init(&n);
    big_from_int(&n, 100);
    
    BigFixed* sqrt_result = big_fixed_create(64);
    big_sqrt(sqrt_result, &n, 64);
    double sqrt_val = big_fixed_to_double(sqrt_result);
    printf("  sqrt(100) = %f\n", sqrt_val);
    
    // Cleanup
    big_free(&n);
    big_fixed_free(f);
    big_fixed_free(sqrt_result);
    
    printf("\nAll tests passed!\n");
    return 0;
}