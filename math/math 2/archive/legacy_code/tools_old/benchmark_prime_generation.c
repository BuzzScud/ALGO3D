/**
 * Benchmark deterministic prime generation vs traditional methods
 * 
 * Compares:
 * 1. Lookup table (O(1))
 * 2. Cached generation (O(1) amortized)
 * 3. Traditional sieve (O(n log log n))
 */

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <stdbool.h>
#include "../include/clock_lattice.h"

// Simple primality test for comparison
static bool is_prime_trial_division(uint64_t n) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    
    for (uint64_t i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) {
            return false;
        }
    }
    return true;
}

// Measure time in microseconds
static uint64_t get_time_us(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}

int main(void) {
    printf("Prime Generation Performance Benchmark\n");
    printf("======================================\n\n");
    
    // Benchmark 1: Lookup table (first 232 primes)
    printf("Benchmark 1: Lookup Table (First 232 Primes)\n");
    printf("--------------------------------------------\n");
    
    uint64_t start = get_time_us();
    for (uint32_t i = 1; i <= 232; i++) {
        uint64_t prime = get_prime_at_index_deterministic(i);
        (void)prime;  // Prevent optimization
    }
    uint64_t end = get_time_us();
    uint64_t lookup_time = end - start;
    
    printf("Generated 232 primes in %lu microseconds\n", lookup_time);
    printf("Average: %.2f microseconds per prime\n", (double)lookup_time / 232.0);
    printf("Complexity: O(1) per prime\n\n");
    
    // Benchmark 2: Cached generation (primes 1-1000)
    printf("Benchmark 2: Cached Generation (Primes 1-1000)\n");
    printf("----------------------------------------------\n");
    
    start = get_time_us();
    for (uint32_t i = 1; i <= 1000; i++) {
        uint64_t prime = get_prime_at_index_deterministic(i);
        (void)prime;
    }
    end = get_time_us();
    uint64_t cached_time = end - start;
    
    printf("Generated 1000 primes in %lu microseconds\n", cached_time);
    printf("Average: %.2f microseconds per prime\n", (double)cached_time / 1000.0);
    printf("Complexity: O(1) amortized\n\n");
    
    // Benchmark 3: Batch operation (primes 1-1000)
    printf("Benchmark 3: Batch Operation (Primes 1-1000)\n");
    printf("--------------------------------------------\n");
    
    uint64_t batch[1000];
    start = get_time_us();
    uint32_t count = get_primes_batch(1, 1000, batch);
    end = get_time_us();
    uint64_t batch_time = end - start;
    
    printf("Generated %u primes in %lu microseconds\n", count, batch_time);
    printf("Average: %.2f microseconds per prime\n", (double)batch_time / (double)count);
    printf("Speedup vs individual: %.2fx\n", (double)cached_time / (double)batch_time);
    printf("\n");
    
    // Benchmark 4: Trial division (for comparison)
    printf("Benchmark 4: Trial Division (First 100 Primes)\n");
    printf("----------------------------------------------\n");
    
    start = get_time_us();
    uint32_t trial_count = 0;
    uint64_t candidate = 2;
    while (trial_count < 100) {
        if (is_prime_trial_division(candidate)) {
            trial_count++;
        }
        candidate++;
    }
    end = get_time_us();
    uint64_t trial_time = end - start;
    
    printf("Generated 100 primes in %lu microseconds\n", trial_time);
    printf("Average: %.2f microseconds per prime\n", (double)trial_time / 100.0);
    printf("Complexity: O(√n) per prime\n\n");
    
    // Benchmark 5: Clock lattice validation
    printf("Benchmark 5: Clock Lattice Validation (1000 primes)\n");
    printf("---------------------------------------------------\n");
    
    start = get_time_us();
    for (uint32_t i = 0; i < 1000; i++) {
        bool valid = validate_prime_by_clock_position(batch[i]);
        (void)valid;
    }
    end = get_time_us();
    uint64_t validation_time = end - start;
    
    printf("Validated 1000 primes in %lu microseconds\n", validation_time);
    printf("Average: %.2f microseconds per validation\n", (double)validation_time / 1000.0);
    printf("Complexity: O(1) per validation\n\n");
    
    // Summary
    printf("Performance Summary\n");
    printf("===================\n");
    printf("Lookup table:     %.2f μs/prime (O(1))\n", (double)lookup_time / 232.0);
    printf("Cached generation: %.2f μs/prime (O(1) amortized)\n", (double)cached_time / 1000.0);
    printf("Batch operation:  %.2f μs/prime (O(1) amortized)\n", (double)batch_time / 1000.0);
    printf("Trial division:   %.2f μs/prime (O(√n))\n", (double)trial_time / 100.0);
    printf("Clock validation: %.2f μs/prime (O(1))\n", (double)validation_time / 1000.0);
    printf("\n");
    
    // Speedup calculations
    double trial_avg = (double)trial_time / 100.0;
    double lookup_avg = (double)lookup_time / 232.0;
    double cached_avg = (double)cached_time / 1000.0;
    
    printf("Speedup vs Trial Division:\n");
    printf("  Lookup table:     %.0fx faster\n", trial_avg / lookup_avg);
    printf("  Cached generation: %.0fx faster\n", trial_avg / cached_avg);
    printf("\n");
    
    // Cache statistics
    uint32_t cache_size, cache_capacity;
    get_prime_cache_stats(&cache_size, &cache_capacity);
    printf("Final Cache Statistics:\n");
    printf("  Size: %u primes\n", cache_size);
    printf("  Capacity: %u primes\n", cache_capacity);
    printf("  Memory: %lu bytes\n", (uint64_t)cache_capacity * sizeof(uint64_t));
    
    // Cleanup
    cleanup_prime_cache();
    
    return 0;
}
