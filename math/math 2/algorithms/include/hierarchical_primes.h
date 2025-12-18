#ifndef ALGORITHMS_HIERARCHICAL_PRIMES_H
#define ALGORITHMS_HIERARCHICAL_PRIMES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdatomic.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Hierarchical Prime Generation System
 * 
 * Demonstrates 12-fold symmetry in prime distribution based on the
 * mathematical principle that primes > 3 fall into 4 residue classes mod 12:
 * - 1 mod 12 (e.g., 13, 37, 61, 73, 97, ...)
 * - 5 mod 12 (e.g., 5, 17, 29, 41, 53, ...)
 * - 7 mod 12 (e.g., 7, 19, 31, 43, 67, ...)
 * - 11 mod 12 (e.g., 11, 23, 47, 59, 71, ...)
 * 
 * This creates a natural 12-fold symmetry in prime distribution,
 * fundamental to the crystalline lattice architecture.
 * 
 * Key Features:
 * - Hierarchical prime generation with parent-child relationships
 * - Partition-based prime generation (range boundaries)
 * - Symmetry group filtering (mod 12)
 * - Prime caching for efficiency
 * - Statistics tracking
 * - Thread-safe operations
 */

/**
 * Prime Range Partition
 * 
 * Defines a range of numbers for prime generation with symmetry filtering.
 */
typedef struct {
    uint64_t range_start;        // Start of range (inclusive)
    uint64_t range_end;          // End of range (inclusive)
    int symmetry_group;          // 0-11 (which residue class mod 12)
    bool filter_by_symmetry;     // If true, only generate primes ≡ symmetry_group (mod 12)
} PrimePartition;

/**
 * Hierarchical Prime Generator
 * 
 * Maintains state for prime generation within a partition,
 * with optional parent reference for hierarchical coordination.
 */
typedef struct HierarchicalPrimeGen {
    // Partition information
    PrimePartition partition;
    
    // Prime cache (within partition)
    uint64_t* cached_primes;
    size_t cache_size;
    size_t cache_capacity;
    
    // Current state
    uint64_t current_candidate;
    uint64_t primes_generated;
    
    // Parent generator (read-only reference, not owned)
    const struct HierarchicalPrimeGen* parent;
    
    // Statistics (atomic for thread safety)
    _Atomic(uint64_t) total_primes_generated;
    _Atomic(uint64_t) cache_hits;
    _Atomic(uint64_t) cache_misses;
    _Atomic(uint64_t) parent_lookups;
    
} HierarchicalPrimeGen;

/**
 * Create hierarchical prime generator
 * 
 * @param range_start Start of prime range (inclusive)
 * @param range_end End of prime range (inclusive)
 * @param symmetry_group Symmetry group (0-11, or -1 for no filtering)
 * @param parent Parent generator (can be NULL for root)
 * @return Allocated generator, or NULL on error
 */
HierarchicalPrimeGen* hierarchical_prime_create(uint64_t range_start,
                                                 uint64_t range_end,
                                                 int symmetry_group,
                                                 const HierarchicalPrimeGen* parent);

/**
 * Free hierarchical prime generator
 * 
 * Note: Does NOT free parent generator (not owned)
 * 
 * @param gen Generator to free
 */
void hierarchical_prime_free(HierarchicalPrimeGen* gen);

/**
 * Get next prime in partition
 * 
 * Generates the next prime within the partition boundaries that
 * matches the symmetry group (if filtering enabled).
 * 
 * @param gen Generator to use
 * @param out_prime Output: next prime
 * @return true if prime found, false if no more primes in partition
 */
bool hierarchical_prime_next(HierarchicalPrimeGen* gen, uint64_t* out_prime);

/**
 * Check if number is prime
 * 
 * First checks local cache, then parent cache, then performs primality test.
 * 
 * @param gen Generator to use
 * @param n Number to test
 * @return true if prime, false otherwise
 */
bool hierarchical_prime_is_prime(HierarchicalPrimeGen* gen, uint64_t n);

/**
 * Check if prime is in partition
 * 
 * Verifies that prime is within partition boundaries and matches
 * symmetry group (if filtering enabled).
 * 
 * @param gen Generator to use
 * @param prime Prime to check
 * @return true if in partition, false otherwise
 */
bool hierarchical_prime_in_partition(const HierarchicalPrimeGen* gen, uint64_t prime);

/**
 * Add prime to cache
 * 
 * Adds a prime to the local cache for faster future lookups.
 * 
 * @param gen Generator to use
 * @param prime Prime to cache
 * @return true on success, false on error
 */
bool hierarchical_prime_cache(HierarchicalPrimeGen* gen, uint64_t prime);

/**
 * Lookup prime in cache
 * 
 * Searches local cache, then parent cache (if available).
 * 
 * @param gen Generator to use
 * @param prime Prime to lookup
 * @return true if found in cache, false otherwise
 */
bool hierarchical_prime_cache_lookup(HierarchicalPrimeGen* gen, uint64_t prime);

/**
 * Get statistics
 * 
 * @param gen Generator to query
 * @param out_generated Output: total primes generated
 * @param out_cache_hits Output: cache hits
 * @param out_cache_misses Output: cache misses
 * @param out_parent_lookups Output: parent lookups
 */
void hierarchical_prime_get_stats(const HierarchicalPrimeGen* gen,
                                   uint64_t* out_generated,
                                   uint64_t* out_cache_hits,
                                   uint64_t* out_cache_misses,
                                   uint64_t* out_parent_lookups);

/**
 * Reset statistics
 * 
 * @param gen Generator to reset
 */
void hierarchical_prime_reset_stats(HierarchicalPrimeGen* gen);

/**
 * Get cache efficiency
 * 
 * Returns cache hit rate as a percentage (0.0 to 100.0).
 * 
 * @param gen Generator to query
 * @return Cache hit rate percentage
 */
double hierarchical_prime_cache_efficiency(const HierarchicalPrimeGen* gen);

/**
 * Prefill cache with primes
 * 
 * Generates and caches primes up to a limit within the partition.
 * Useful for initialization.
 * 
 * @param gen Generator to use
 * @param max_primes Maximum number of primes to generate
 * @return Number of primes generated
 */
size_t hierarchical_prime_prefill_cache(HierarchicalPrimeGen* gen, size_t max_primes);

/**
 * Get symmetry group distribution
 * 
 * Analyzes the distribution of primes across the 12 symmetry groups
 * within a given range.
 * 
 * @param range_start Start of range
 * @param range_end End of range
 * @param distribution Output: array of 12 counts (one per symmetry group)
 * @return Total number of primes found
 */
size_t hierarchical_prime_symmetry_distribution(uint64_t range_start,
                                                 uint64_t range_end,
                                                 size_t distribution[12]);

/**
 * Verify 12-fold symmetry
 * 
 * Checks that primes > 3 only appear in residue classes 1, 5, 7, 11 (mod 12).
 * 
 * @param range_start Start of range
 * @param range_end End of range
 * @return true if all primes follow 12-fold symmetry, false otherwise
 */
bool hierarchical_prime_verify_symmetry(uint64_t range_start, uint64_t range_end);

/**
 * Print generator info (for debugging)
 * 
 * @param gen Generator to print
 * @param name Optional name
 */
void hierarchical_prime_print_info(const HierarchicalPrimeGen* gen, const char* name);

#ifdef __cplusplus
}
#endif

#endif /* ALGORITHMS_HIERARCHICAL_PRIMES_H */
