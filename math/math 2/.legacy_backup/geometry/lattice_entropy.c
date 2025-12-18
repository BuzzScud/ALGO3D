/**
 * Lattice Entropy System Implementation
 * 
 * Implements Γ(n,d) = log₂(count of abacus primes / entropy of lattice points)
 * This measures the information density of the crystalline lattice structure.
 */

#include "../include/lattice_entropy.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"
#include "math/types.h"
#include "../include/crystal_abacus.h"
#include "math/clock.h"
#include "math/prime.h"  // For prime_validate_by_clock()
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Entropy cache for performance optimization
 */
#define ENTROPY_CACHE_SIZE 13  // Dimensions 0-12
static double entropy_cache[ENTROPY_CACHE_SIZE];
static bool cache_initialized = false;

/**
 * Count primes in a given dimension
 * 
 * For the crystalline lattice, dimension is determined by:
 * d = (prime mod 12) / 12 * 12 = prime mod 12
 * 
 * This counts primes that have the same (prime mod 12) value.
 */
uint64_t count_primes_in_dimension(uint32_t d, uint64_t max_prime) {
    if (d >= 12) return 0;  // Only 12 dimensions (0-11)
    if (max_prime < 2) return 0;
    
    uint64_t count = 0;
    
    // Count primes up to max_prime that map to dimension d
    for (uint64_t p = 2; p <= max_prime; p++) {
        // Internal: Trust deterministic clock lattice
        if (prime_validate_by_clock(p)) {
            uint32_t prime_dim = p % 12;
            if (prime_dim == d) {
                count++;
            }
        }
    }
    
    return count;
}

/**
 * Calculate Shannon entropy of lattice points
 * 
 * H = -Σ p(i) log₂ p(i)
 * 
 * Where p(i) is the probability of a prime being in dimension i.
 */
double calculate_point_entropy(uint32_t n, uint32_t d, uint64_t max_prime) {
    (void)n;  // Reserved for future use
    
    if (d >= 12) return 0.0;
    if (max_prime < 2) return 0.0;
    
    // Count primes in each dimension
    uint64_t counts[12] = {0};
    uint64_t total_primes = 0;
    
    for (uint64_t p = 2; p <= max_prime; p++) {
        // Internal: Trust deterministic clock lattice
        if (prime_validate_by_clock(p)) {
            uint32_t prime_dim = p % 12;
            counts[prime_dim]++;
            total_primes++;
        }
    }
    
    if (total_primes == 0) return 0.0;
    
    // Calculate Shannon entropy
    double entropy = 0.0;
    for (int i = 0; i < 12; i++) {
        if (counts[i] > 0) {
            double p_i = (double)counts[i] / (double)total_primes;
            entropy -= p_i * (math_log(p_i) / math_log(2.0));
        }
    }
    
    return entropy;
}

/**
 * Calculate complete lattice entropy Γ(n,d)
 * 
 * Γ(n,d) = log₂(count of abacus primes in dimension / entropy of lattice points)
 * 
 * This measures the information density of the lattice.
 */
double calculate_lattice_entropy(uint32_t n, uint32_t d) {
    if (d >= 12) return 0.0;
    
    // Use a reasonable max_prime for calculation
    // For performance, we use 10000 as default
    uint64_t max_prime = 10000;
    
    // Count primes in dimension d
    uint64_t prime_count = count_primes_in_dimension(d, max_prime);
    
    if (prime_count == 0) {
        // If no primes in dimension, return small value
        return 0.1;
    }
    
    // Calculate lattice point entropy
    double point_entropy = calculate_point_entropy(n, d, max_prime);
    
    if (point_entropy <= 0.0) {
        // If entropy is zero or negative, use prime count only
        return math_log((double)prime_count) / math_log(2.0);
    }
    
    // Complete formula: log₂(count / entropy)
    double ratio = (double)prime_count / point_entropy;
    double gamma = math_log(ratio) / math_log(2.0);
    
    return gamma;
}

/**
 * Calculate lattice density (simplified version)
 * 
 * Faster approximation for performance-critical paths.
 * Uses only prime counting without full entropy calculation.
 */
double calculate_lattice_density(uint32_t n, uint32_t d) {
    (void)n;  // Reserved for future use
    
    if (d == 0) return 0.0;
    if (d >= 12) return 0.0;
    
    // Simplified: just count primes up to d
    uint64_t count = 0;
    for (uint64_t i = 2; i <= d; i++) {
        // Internal: Trust deterministic clock lattice
        if (prime_validate_by_clock(i)) count++;
    }
    
    if (count == 0) return 0.0;
    
    double density = (double)count / (double)d;
    return math_log(density) / math_log(2.0);
}

/**
 * Get cached entropy value
 */
double get_cached_entropy(uint32_t n, uint32_t d) {
    if (d >= ENTROPY_CACHE_SIZE) {
        // Out of cache range, calculate directly
        return calculate_lattice_entropy(n, d);
    }
    
    // Initialize cache if needed
    if (!cache_initialized) {
        initialize_entropy_cache();
    }
    
    return entropy_cache[d];
}

/**
 * Initialize entropy cache
 * 
 * Pre-calculates entropy for dimensions 0-12.
 */
void initialize_entropy_cache(void) {
    if (cache_initialized) return;
    
    printf("Initializing lattice entropy cache...\n");
    
    for (uint32_t d = 0; d < ENTROPY_CACHE_SIZE; d++) {
        entropy_cache[d] = calculate_lattice_entropy(0, d);
        printf("  Γ(0,%u) = %.6f\n", d, entropy_cache[d]);
    }
    
    cache_initialized = true;
    printf("✓ Entropy cache initialized\n");
}

/**
 * Clear entropy cache
 */
void clear_entropy_cache(void) {
    memset(entropy_cache, 0, sizeof(entropy_cache));
    cache_initialized = false;
}