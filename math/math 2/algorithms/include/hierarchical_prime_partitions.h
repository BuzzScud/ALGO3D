#ifndef HIERARCHICAL_PRIME_PARTITIONS_H
#define HIERARCHICAL_PRIME_PARTITIONS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdatomic.h>
#include "lattice_sphere_positions.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Hierarchical Abacus System
 * 
 * STUB IMPLEMENTATION: Minimal interface for compilation
 * - Will be properly implemented with NEW math library later
 * - Currently just provides function signatures
 */

/**
 * Hierarchical Abacus (simplified)
 */
typedef struct HierarchicalAbacus {
    // Simplified structure - to be redesigned
    int symmetry_group;
    uint64_t primes_generated;
    
    // Statistics (atomic for thread safety)
    _Atomic(uint64_t) total_primes_generated;
    _Atomic(uint64_t) cache_hits;
    _Atomic(uint64_t) cache_misses;
    _Atomic(uint64_t) parent_lookups;
    
} HierarchicalAbacus;

/**
 * Create hierarchical abacus
 */
HierarchicalAbacus* hierarchical_abacus_create(const LatticePartition* partition,
                                                const void* parent);

/**
 * Free hierarchical abacus
 */
void hierarchical_abacus_free(HierarchicalAbacus* abacus);

/**
 * Get next prime in partition
 */
int hierarchical_abacus_next_prime(HierarchicalAbacus* abacus, void* out_prime);

/**
 * Check if number is prime
 */
bool hierarchical_abacus_is_prime(HierarchicalAbacus* abacus, const void* n);

/**
 * Check if prime is in partition
 */
bool hierarchical_abacus_in_partition(const HierarchicalAbacus* abacus, 
                                      const void* prime);

/**
 * Add prime to cache
 */
int hierarchical_abacus_cache_prime(HierarchicalAbacus* abacus, const void* prime);

/**
 * Lookup prime in cache
 */
bool hierarchical_abacus_cache_lookup(HierarchicalAbacus* abacus, const void* prime);

/**
 * Get statistics
 */
void hierarchical_abacus_get_stats(const HierarchicalAbacus* abacus,
                                   uint64_t* out_generated,
                                   uint64_t* out_cache_hits,
                                   uint64_t* out_cache_misses,
                                   uint64_t* out_parent_lookups);

/**
 * Generate multiple primes
 */
int hierarchical_abacus_generate_primes(HierarchicalAbacus* abacus, size_t count);

#ifdef __cplusplus
}
#endif

#endif // HIERARCHICAL_PRIME_PARTITIONS_H