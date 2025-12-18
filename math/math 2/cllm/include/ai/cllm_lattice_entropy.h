/**
 * @file cllm_lattice_entropy.h
 * @brief Lattice entropy calculation Γ(n,d) for crystalline CLLM
 * 
 * Implements entropy calculation based on prime distribution in lattice dimensions.
 * The entropy Γ(n,d) measures the information content and structural complexity
 * of the crystalline lattice at position n in dimension d.
 * 
 * Mathematical Foundation:
 * Γ(n,d) = -Σ p(i) * log₂(p(i))
 * where p(i) is the probability distribution of primes in dimension d
 */

#ifndef CLLM_LATTICE_ENTROPY_H
#define CLLM_LATTICE_ENTROPY_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Maximum dimension for entropy calculation
 */
#define MAX_ENTROPY_DIMENSION 12

/**
 * @brief Cache size for entropy values (powers of 2 for efficiency)
 */
#define ENTROPY_CACHE_SIZE 1024

/**
 * @brief Entropy cache entry structure
 */
typedef struct {
    uint64_t n;           /**< Position in lattice */
    uint32_t d;           /**< Dimension */
    double entropy;       /**< Cached entropy value */
    bool valid;           /**< Cache entry validity flag */
} EntropyCacheEntry;

/**
 * @brief Entropy calculation context
 */
typedef struct {
    EntropyCacheEntry cache[ENTROPY_CACHE_SIZE];  /**< LRU cache for entropy values */
    uint64_t cache_hits;                          /**< Number of cache hits */
    uint64_t cache_misses;                        /**< Number of cache misses */
    uint64_t total_calculations;                  /**< Total entropy calculations */
} EntropyContext;

/**
 * @brief Initialize entropy calculation context
 * 
 * @param ctx Pointer to entropy context to initialize
 * @return true on success, false on failure
 */
bool entropy_context_init(EntropyContext *ctx);

/**
 * @brief Destroy entropy calculation context
 * 
 * @param ctx Pointer to entropy context to destroy
 */
void entropy_context_destroy(EntropyContext *ctx);

/**
 * @brief Count primes in a specific dimension up to position n
 * 
 * Uses the prime lattice structure to count primes that contribute
 * to the entropy calculation in dimension d.
 * 
 * @param n Position in lattice
 * @param d Dimension (1-12)
 * @return Number of primes in dimension d up to position n
 */
uint64_t count_primes_in_dimension(uint64_t n, uint32_t d);

/**
 * @brief Calculate entropy for a single lattice point
 * 
 * Computes the local entropy at position n in dimension d based on
 * the distribution of primes in the surrounding lattice structure.
 * 
 * @param n Position in lattice
 * @param d Dimension (1-12)
 * @return Point entropy value (0.0 to log₂(d))
 */
double calculate_point_entropy(uint64_t n, uint32_t d);

/**
 * @brief Calculate full lattice entropy Γ(n,d)
 * 
 * This is the main entropy function that integrates into the L(n,d,k,λ,ω,ψ) formula.
 * It measures the information content and structural complexity of the lattice.
 * 
 * Formula: Γ(n,d) = -Σ p(i) * log₂(p(i))
 * where p(i) is the probability of finding a prime in position i
 * 
 * @param ctx Entropy context (can be NULL for no caching)
 * @param n Position in lattice
 * @param d Dimension (1-12)
 * @return Lattice entropy value Γ(n,d)
 */
double calculate_lattice_entropy(EntropyContext *ctx, uint64_t n, uint32_t d);

/**
 * @brief Calculate entropy with caching
 * 
 * Optimized version that uses LRU cache to avoid redundant calculations.
 * 
 * @param ctx Entropy context (must not be NULL)
 * @param n Position in lattice
 * @param d Dimension (1-12)
 * @return Cached or newly calculated entropy value
 */
double calculate_lattice_entropy_cached(EntropyContext *ctx, uint64_t n, uint32_t d);

/**
 * @brief Clear entropy cache
 * 
 * @param ctx Entropy context
 */
void entropy_cache_clear(EntropyContext *ctx);

/**
 * @brief Get entropy cache statistics
 * 
 * @param ctx Entropy context
 * @param hit_rate Output: cache hit rate (0.0 to 1.0)
 * @param total_calcs Output: total calculations performed
 */
void entropy_get_cache_stats(const EntropyContext *ctx, double *hit_rate, uint64_t *total_calcs);

/**
 * @brief Calculate normalized entropy
 * 
 * Returns entropy normalized to [0, 1] range for easier interpretation.
 * 
 * @param ctx Entropy context (can be NULL)
 * @param n Position in lattice
 * @param d Dimension (1-12)
 * @return Normalized entropy value (0.0 to 1.0)
 */
double calculate_normalized_entropy(EntropyContext *ctx, uint64_t n, uint32_t d);

#ifdef __cplusplus
}
#endif

#endif /* CLLM_LATTICE_ENTROPY_H */