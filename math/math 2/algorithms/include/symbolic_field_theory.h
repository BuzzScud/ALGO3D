/**
 * @file symbolic_field_theory.h
 * @brief Symbolic Field Theory (SFT) for deterministic prime discovery
 * 
 * This module implements a novel approach to prime discovery using symbolic
 * field theory. It finds "collapse zones" where primes are likely to occur
 * by analyzing the hybrid field function ψ(x) and collapse metric κ(x).
 * 
 * Mathematical Foundation:
 * - ω(x) = number of distinct prime factors of x
 * - ψ(x) = (x mod 6) - ω(x)  [hybrid field function]
 * - κ(x) = ⌊(x - ψ)² / x⌋     [collapse metric]
 * 
 * Key Insight: Primes occur at local minima of κ(x)
 * 
 * This is more efficient than sieve methods for finding sparse primes
 * in large ranges, and is completely deterministic (not probabilistic).
 */

#ifndef SYMBOLIC_FIELD_THEORY_H
#define SYMBOLIC_FIELD_THEORY_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Collapse zone structure
 * 
 * Represents a region where the collapse metric κ(x) has a local minimum,
 * indicating high probability of prime occurrence.
 */
typedef struct {
    uint64_t start;        /**< Start of collapse zone */
    uint64_t end;          /**< End of collapse zone */
    uint64_t min_kappa;    /**< Minimum κ value in zone */
    uint64_t candidate;    /**< Prime candidate at minimum */
} CollapseZone;

/**
 * @brief Calculate ω(x) - number of distinct prime factors
 * 
 * Uses crystalline factorization to count distinct prime factors.
 * 
 * @param x Number to analyze
 * @return Number of distinct prime factors
 * 
 * Examples:
 * - ω(12) = 2  (factors: 2, 3)
 * - ω(30) = 3  (factors: 2, 3, 5)
 * - ω(17) = 1  (prime)
 */
uint32_t sft_omega(uint64_t x);

/**
 * @brief Calculate ψ(x) - hybrid field function
 * 
 * Combines modular arithmetic with prime factor counting:
 * ψ(x) = (x mod 6) - ω(x)
 * 
 * This function creates a "field" where primes have characteristic values.
 * 
 * @param x Number to analyze
 * @return Hybrid field value
 */
int64_t sft_psi_hybrid(uint64_t x);

/**
 * @brief Calculate κ(x) - collapse metric
 * 
 * Measures how far x is from its field value:
 * κ(x) = ⌊(x - ψ(x))² / x⌋
 * 
 * Local minima of κ(x) indicate prime candidates.
 * 
 * @param x Number to analyze
 * @param psi Pre-calculated ψ(x) value (for efficiency)
 * @return Collapse metric value
 */
uint64_t sft_kappa(uint64_t x, int64_t psi);

/**
 * @brief Find collapse zones in a range
 * 
 * Scans a range for local minima of κ(x), which indicate regions
 * where primes are likely to occur.
 * 
 * @param start Start of range (inclusive)
 * @param end End of range (inclusive)
 * @param zones Output array for collapse zones
 * @param max_zones Maximum number of zones to find
 * @return Number of collapse zones found
 * 
 * Example:
 * ```c
 * CollapseZone zones[100];
 * size_t count = sft_find_collapse_zones(1000, 2000, zones, 100);
 * for (size_t i = 0; i < count; i++) {
 *     printf("Zone %zu: [%lu, %lu], candidate: %lu\n",
 *            i, zones[i].start, zones[i].end, zones[i].candidate);
 * }
 * ```
 */
size_t sft_find_collapse_zones(uint64_t start, uint64_t end,
                                CollapseZone* zones, size_t max_zones);

/**
 * @brief Deterministic prime discovery using SFT
 * 
 * Main function for finding primes using symbolic field theory.
 * More efficient than sieve for sparse primes in large ranges.
 * 
 * Algorithm:
 * 1. Find collapse zones in range
 * 2. Test candidates at zone minima
 * 3. Verify primality using crystalline methods
 * 4. Return verified primes
 * 
 * @param start Start of range (inclusive)
 * @param end End of range (inclusive)
 * @param primes Output array for discovered primes
 * @param max_primes Maximum number of primes to find
 * @return Number of primes discovered
 * 
 * Example:
 * ```c
 * uint64_t primes[1000];
 * size_t count = sft_deterministic_prime_map(10000, 20000, primes, 1000);
 * printf("Found %zu primes\n", count);
 * ```
 * 
 * Performance:
 * - O(n) for finding collapse zones
 * - O(k log k) for primality testing (k = number of zones)
 * - More efficient than O(n log log n) sieve for sparse primes
 */
size_t sft_deterministic_prime_map(uint64_t start, uint64_t end,
                                    uint64_t* primes, size_t max_primes);

/**
 * @brief Check if a number is in a collapse zone
 * 
 * Helper function to determine if a number is near a local minimum
 * of the collapse metric.
 * 
 * @param x Number to check
 * @param window_size Size of window to check around x
 * @return true if x is in a collapse zone, false otherwise
 */
bool sft_is_in_collapse_zone(uint64_t x, uint32_t window_size);

/**
 * @brief Get collapse metric gradient
 * 
 * Calculates the rate of change of κ(x) at a point.
 * Useful for identifying local minima more precisely.
 * 
 * @param x Point to evaluate
 * @return Gradient (positive = increasing, negative = decreasing)
 */
int64_t sft_kappa_gradient(uint64_t x);

#ifdef __cplusplus
}
#endif

#endif /* SYMBOLIC_FIELD_THEORY_H */
