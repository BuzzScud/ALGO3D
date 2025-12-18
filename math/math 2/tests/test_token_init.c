/*
 * test_token_init.c - Test token initialization specifically
 */

#include "cllm_pure_crystalline.h"
#include <stdio.h>
#include <time.h>

int main(void) {
    printf("Test 1: Getting first prime...\n");
    fflush(stdout);
    
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    uint64_t p1 = crystalline_get_nth_prime(1);
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    
    printf("First prime: %lu (took %.6f seconds)\n", p1, elapsed);
    fflush(stdout);
    
    printf("\nTest 2: Getting 100 primes...\n");
    fflush(stdout);
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    for (int i = 1; i <= 100; i++) {
        uint64_t p = crystalline_get_nth_prime(i);
        if (i % 10 == 0) {
            printf("  Prime %d: %lu\n", i, p);
            fflush(stdout);
        }
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    
    printf("Got 100 primes in %.6f seconds (%.2f ms per prime)\n", elapsed, elapsed * 1000.0 / 100.0);
    fflush(stdout);
    
    return 0;
}