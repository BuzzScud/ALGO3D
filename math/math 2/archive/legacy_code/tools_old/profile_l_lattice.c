/*
 * profile_l_lattice.c - Profile L_lattice() function performance
 * 
 * This tool measures the performance of L_lattice() to identify bottlenecks
 */

#include "../include/prime_lattice_core.h"
#include "../include/prime_math_custom.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define MEASURE_START(name) \
    struct timespec start_##name, end_##name; \
    clock_gettime(CLOCK_MONOTONIC, &start_##name);

#define MEASURE_END(name) \
    clock_gettime(CLOCK_MONOTONIC, &end_##name); \
    double elapsed_##name = (end_##name.tv_sec - start_##name.tv_sec) + \
                            (end_##name.tv_nsec - start_##name.tv_nsec) / 1e9;

int main(void) {
    printf("=======================================================\n");
    printf("  L_lattice() Performance Profiling\n");
    printf("=======================================================\n\n");
    
    // Test parameters
    uint64_t test_primes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29};
    int num_test_primes = 10;
    int num_dimensions = 12;
    
    printf("Test 1: Single L_lattice() call\n");
    printf("----------------------------------------\n");
    
    MEASURE_START(single_call);
    double result = L_lattice(7, 0, 1, "a", 3, 3, 4);
    MEASURE_END(single_call);
    
    printf("Result: %.6f\n", result);
    printf("Time: %.6f ms\n\n", elapsed_single_call * 1000.0);
    
    printf("Test 2: 100 L_lattice() calls (same parameters)\n");
    printf("----------------------------------------\n");
    
    MEASURE_START(hundred_calls);
    for (int i = 0; i < 100; i++) {
        result = L_lattice(7, 0, 1, "a", 3, 3, 4);
    }
    MEASURE_END(hundred_calls);
    
    printf("Total time: %.6f ms\n", elapsed_hundred_calls * 1000.0);
    printf("Per call: %.6f ms\n\n", elapsed_hundred_calls * 1000.0 / 100.0);
    
    printf("Test 3: Varying primes (10 primes × 12 dimensions)\n");
    printf("----------------------------------------\n");
    
    MEASURE_START(varying_primes);
    for (int p = 0; p < num_test_primes; p++) {
        for (int d = 0; d < num_dimensions; d++) {
            result = L_lattice(test_primes[p], d, p % 12, "a", 3, 3, 4);
        }
    }
    MEASURE_END(varying_primes);
    
    int total_calls = num_test_primes * num_dimensions;
    printf("Total calls: %d\n", total_calls);
    printf("Total time: %.6f ms\n", elapsed_varying_primes * 1000.0);
    printf("Per call: %.6f ms\n\n", elapsed_varying_primes * 1000.0 / total_calls);
    
    printf("Test 4: Simulating tiny model (100 tokens × 64 dims)\n");
    printf("----------------------------------------\n");
    
    MEASURE_START(tiny_model);
    for (int t = 0; t < 100; t++) {
        uint64_t prime = test_primes[t % num_test_primes];
        for (int d = 0; d < 64; d++) {
            result = L_lattice(prime, d, t % 12, "token", 3, prime, d + 3);
        }
    }
    MEASURE_END(tiny_model);
    
    total_calls = 100 * 64;
    printf("Total calls: %d\n", total_calls);
    printf("Total time: %.6f seconds (%.2f ms)\n", 
           elapsed_tiny_model, elapsed_tiny_model * 1000.0);
    printf("Per call: %.6f ms\n", elapsed_tiny_model * 1000.0 / total_calls);
    printf("Estimated for 10k tokens × 512 dims: %.2f seconds\n\n",
           elapsed_tiny_model * (10000.0 * 512.0) / (100.0 * 64.0));
    
    printf("Test 5: Profile individual components\n");
    printf("----------------------------------------\n");
    
    // Test O_exponent
    MEASURE_START(o_exponent);
    for (int i = 0; i < 1000; i++) {
        O_exponent(7, 1, "a");
    }
    MEASURE_END(o_exponent);
    printf("O_exponent: %.6f ms per 1000 calls (%.6f µs each)\n", 
           elapsed_o_exponent * 1000.0, elapsed_o_exponent * 1000000.0 / 1000.0);
    
    // Test theta_n
    MEASURE_START(theta_n_test);
    for (int i = 0; i < 1000; i++) {
        theta_n(7, 1, "a", 3, 3, 4, false);
    }
    MEASURE_END(theta_n_test);
    printf("theta_n: %.6f ms per 1000 calls (%.6f µs each)\n", 
           elapsed_theta_n_test * 1000.0, elapsed_theta_n_test * 1000000.0 / 1000.0);
    
    // Test nu_lambda
    MEASURE_START(nu_lambda_test);
    for (int i = 0; i < 1000; i++) {
        nu_lambda("a");
    }
    MEASURE_END(nu_lambda_test);
    printf("nu_lambda: %.6f ms per 1000 calls (%.6f µs each)\n", 
           elapsed_nu_lambda_test * 1000.0, elapsed_nu_lambda_test * 1000000.0 / 1000.0);
    
    // Test prime_pow
    MEASURE_START(prime_pow_test);
    for (int i = 0; i < 1000; i++) {
        math_pow(3.0, 2.5);
    }
    MEASURE_END(prime_pow_test);
    printf("prime_pow: %.6f ms per 1000 calls (%.6f µs each)\n", 
           elapsed_prime_pow_test * 1000.0, elapsed_prime_pow_test * 1000000.0 / 1000.0);
    
    // Test prime_cos
    MEASURE_START(prime_cos_test);
    for (int i = 0; i < 1000; i++) {
        math_cos(1.5);
    }
    MEASURE_END(prime_cos_test);
    printf("prime_cos: %.6f ms per 1000 calls (%.6f µs each)\n", 
           elapsed_prime_cos_test * 1000.0, elapsed_prime_cos_test * 1000000.0 / 1000.0);
    
    // Test prime_log
    MEASURE_START(prime_log_test);
    for (int i = 0; i < 1000; i++) {
        math_log(3.0);
    }
    MEASURE_END(prime_log_test);
    printf("prime_log: %.6f ms per 1000 calls (%.6f µs each)\n\n", 
           elapsed_prime_log_test * 1000.0, elapsed_prime_log_test * 1000000.0 / 1000.0);
    
    printf("=======================================================\n");
    printf("  Analysis Complete\n");
    printf("=======================================================\n");
    
    return 0;
}
