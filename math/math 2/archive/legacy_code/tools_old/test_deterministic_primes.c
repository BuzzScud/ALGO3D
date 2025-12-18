/**
 * Test deterministic prime generation
 * 
 * Verifies the hybrid deterministic prime generation system.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "../include/clock_lattice.h"

int main(void) {
    printf("Testing Deterministic Prime Generation\n");
    printf("======================================\n\n");
    
    // Test 1: First 12 primes (Ring 0)
    printf("Test 1: First 12 primes (Ring 0 - Hours)\n");
    for (uint32_t i = 1; i <= 12; i++) {
        uint64_t prime = get_prime_at_index_deterministic(i);
        printf("Prime %2u: %lu\n", i, prime);
    }
    printf("\n");
    
    // Test 2: Primes 13-20 (Ring 1 start)
    printf("Test 2: Primes 13-20 (Ring 1 - Minutes start)\n");
    for (uint32_t i = 13; i <= 20; i++) {
        uint64_t prime = get_prime_at_index_deterministic(i);
        printf("Prime %2u: %lu\n", i, prime);
    }
    printf("\n");
    
    // Test 3: Primes around ring boundaries
    printf("Test 3: Ring boundary primes\n");
    printf("Prime 12 (Ring 0 end):  %lu\n", get_prime_at_index_deterministic(12));
    printf("Prime 13 (Ring 1 start): %lu\n", get_prime_at_index_deterministic(13));
    printf("Prime 72 (Ring 1 end):   %lu\n", get_prime_at_index_deterministic(72));
    printf("Prime 73 (Ring 2 start): %lu\n", get_prime_at_index_deterministic(73));
    printf("Prime 132 (Ring 2 end):  %lu\n", get_prime_at_index_deterministic(132));
    printf("Prime 133 (Ring 3 start): %lu\n", get_prime_at_index_deterministic(133));
    printf("Prime 232 (Ring 3 end):  %lu\n", get_prime_at_index_deterministic(232));
    printf("\n");
    
    // Test 4: Beyond lookup table (cache generation)
    printf("Test 4: Beyond lookup table (primes 233-240)\n");
    for (uint32_t i = 233; i <= 240; i++) {
        uint64_t prime = get_prime_at_index_deterministic(i);
        printf("Prime %u: %lu\n", i, prime);
    }
    printf("\n");
    
    // Test 5: Batch operation
    printf("Test 5: Batch operation (primes 100-110)\n");
    uint64_t batch[11];
    uint32_t count = get_primes_batch(100, 11, batch);
    printf("Retrieved %u primes:\n", count);
    for (uint32_t i = 0; i < count; i++) {
        printf("Prime %u: %lu\n", 100 + i, batch[i]);
    }
    printf("\n");
    
    // Test 6: Cache statistics
    uint32_t cache_size, cache_capacity;
    get_prime_cache_stats(&cache_size, &cache_capacity);
    printf("Test 6: Cache statistics\n");
    printf("Cache size: %u primes\n", cache_size);
    printf("Cache capacity: %u primes\n", cache_capacity);
    printf("\n");
    
    // Test 7: Clock lattice validation
    printf("Test 7: Clock lattice validation\n");
    uint64_t test_primes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37};
    for (int i = 0; i < 12; i++) {
        bool valid = validate_prime_by_clock_position(test_primes[i]);
        printf("Prime %lu: %s\n", test_primes[i], valid ? "VALID" : "INVALID");
    }
    printf("\n");
    
    // Test 8: Non-primes should fail validation
    printf("Test 8: Non-prime validation (should fail)\n");
    uint64_t non_primes[] = {4, 6, 8, 9, 10, 12, 14, 15, 16, 18};
    for (int i = 0; i < 10; i++) {
        bool valid = validate_prime_by_clock_position(non_primes[i]);
        printf("Number %lu: %s\n", non_primes[i], valid ? "VALID" : "INVALID");
    }
    printf("\n");
    
    // Cleanup
    cleanup_prime_cache();
    
    printf("All tests complete!\n");
    return 0;
}
