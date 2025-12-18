/**
 * @file cllm_lattice_entropy.c
 * @brief Implementation of lattice entropy calculation Γ(n,d)
 */

#include "math/transcendental.h"
#include "math/arithmetic.h"
#include "math/prime.h"  // For prime_validate_by_clock()
#include "ai/cllm_lattice_entropy.h"
#include <string.h>
#include <stdio.h>

// Forward declaration for clock lattice function

/**
 * @brief Small epsilon for floating point comparisons
 */
#define ENTROPY_EPSILON 1e-10

/**
 * @brief Initialize entropy calculation context
 */
bool entropy_context_init(EntropyContext *ctx) {
    if (!ctx) {
        return false;
    }
    
    memset(ctx, 0, sizeof(EntropyContext));
    
    // Initialize all cache entries as invalid
    for (int i = 0; i < ENTROPY_CACHE_SIZE; i++) {
        ctx->cache[i].valid = false;
    }
    
    return true;
}

/**
 * @brief Destroy entropy calculation context
 */
void entropy_context_destroy(EntropyContext *ctx) {
    if (!ctx) {
        return;
    }
    
    // Clear cache
    entropy_cache_clear(ctx);
}

/**
 * @brief Count primes in a specific dimension up to position n
 * 
 * This function counts how many primes exist in the lattice structure
 * within dimension d up to position n. The count is used to calculate
 * probability distributions for entropy.
 */
uint64_t count_primes_in_dimension(uint64_t n, uint32_t d) {
    if (d == 0 || d > MAX_ENTROPY_DIMENSION) {
        return 0;
    }
    
    if (n == 0) {
        return 0;
    }
    
    uint64_t count = 0;
    
    // For dimension 1, count primes up to n using prime lattice
    if (d == 1) {
        // Use prime counting approximation: π(n) ≈ n / ln(n)
        if (n < 2) {
            return 0;
        }
        
        // For small n, count exactly
        if (n < 100) {
            for (uint64_t i = 2; i <= n; i++) {
                // Internal: Trust deterministic clock lattice
                if (prime_validate_by_clock(i)) {
                    count++;
                }
            }
            return count;
        }
        
        // For larger n, use approximation
        double ln_n = math_log((double)n);
        count = (uint64_t)((double)n / ln_n);
        return count;
    }
    
    // For higher dimensions, use recursive structure
    // Each dimension d has approximately π(n^(1/d)) primes
    double root_n = math_pow((double)n, 1.0 / (double)d);
    
    if (root_n < 2.0) {
        return 0;
    }
    
    double ln_root_n = math_log(root_n);
    count = (uint64_t)(root_n / ln_root_n);
    
    return count;
}

/**
 * @brief Calculate entropy for a single lattice point
 * 
 * Computes local entropy based on prime distribution around point n in dimension d.
 */
double calculate_point_entropy(uint64_t n, uint32_t d) {
    if (d == 0 || d > MAX_ENTROPY_DIMENSION) {
        return 0.0;
    }
    
    if (n == 0) {
        return 0.0;
    }
    
    // Count primes in this dimension
    uint64_t prime_count = count_primes_in_dimension(n, d);
    
    if (prime_count == 0) {
        return 0.0;
    }
    
    // Calculate probability of finding a prime at position n
    double p_prime = (double)prime_count / (double)n;
    double p_composite = 1.0 - p_prime;
    
    // Calculate Shannon entropy: H = -Σ p(i) * log₂(p(i))
    double entropy = 0.0;
    
    if (p_prime > ENTROPY_EPSILON) {
        entropy -= p_prime * math_log2(p_prime);
    }
    
    if (p_composite > ENTROPY_EPSILON) {
        entropy -= p_composite * math_log2(p_composite);
    }
    
    return entropy;
}

/**
 * @brief Calculate full lattice entropy Γ(n,d)
 * 
 * Main entropy function that integrates into L(n,d,k,λ,ω,ψ).
 * Measures information content and structural complexity.
 */
double calculate_lattice_entropy(EntropyContext *ctx, uint64_t n, uint32_t d) {
    if (d == 0 || d > MAX_ENTROPY_DIMENSION) {
        return 0.0;
    }
    
    if (n == 0) {
        return 0.0;
    }
    
    // If context provided, try cached version
    if (ctx) {
        return calculate_lattice_entropy_cached(ctx, n, d);
    }
    
    // Calculate point entropy
    double point_entropy = calculate_point_entropy(n, d);
    
    // Add dimensional contribution
    // Higher dimensions have more structural complexity
    double dimensional_factor = math_log2((double)(d + 1));
    
    // Calculate position-dependent contribution
    // Entropy increases logarithmically with position due to more possible states
    double position_factor = math_log2((double)(n + 1));
    
    // Combine all factors
    // Normalize by dividing by a reference scale to keep values reasonable
    double total_entropy = (point_entropy * dimensional_factor * position_factor) / 10.0;
    
    return total_entropy;
}

/**
 * @brief Calculate hash for cache lookup
 */
static inline uint32_t entropy_cache_hash(uint64_t n, uint32_t d) {
    // Simple hash combining n and d
    uint64_t hash = n * 31 + d;
    return (uint32_t)(hash % ENTROPY_CACHE_SIZE);
}

/**
 * @brief Calculate entropy with caching
 */
double calculate_lattice_entropy_cached(EntropyContext *ctx, uint64_t n, uint32_t d) {
    if (!ctx) {
        return calculate_lattice_entropy(NULL, n, d);
    }
    
    if (d == 0 || d > MAX_ENTROPY_DIMENSION) {
        return 0.0;
    }
    
    // Calculate cache index
    uint32_t cache_idx = entropy_cache_hash(n, d);
    
    // Check cache
    EntropyCacheEntry *entry = &ctx->cache[cache_idx];
    if (entry->valid && entry->n == n && entry->d == d) {
        ctx->cache_hits++;
        return entry->entropy;
    }
    
    // Cache miss - calculate entropy
    ctx->cache_misses++;
    ctx->total_calculations++;
    
    double entropy = calculate_lattice_entropy(NULL, n, d);
    
    // Update cache
    entry->n = n;
    entry->d = d;
    entry->entropy = entropy;
    entry->valid = true;
    
    return entropy;
}

/**
 * @brief Clear entropy cache
 */
void entropy_cache_clear(EntropyContext *ctx) {
    if (!ctx) {
        return;
    }
    
    for (int i = 0; i < ENTROPY_CACHE_SIZE; i++) {
        ctx->cache[i].valid = false;
    }
    
    ctx->cache_hits = 0;
    ctx->cache_misses = 0;
}

/**
 * @brief Get entropy cache statistics
 */
void entropy_get_cache_stats(const EntropyContext *ctx, double *hit_rate, uint64_t *total_calcs) {
    if (!ctx) {
        if (hit_rate) *hit_rate = 0.0;
        if (total_calcs) *total_calcs = 0;
        return;
    }
    
    uint64_t total = ctx->cache_hits + ctx->cache_misses;
    
    if (hit_rate) {
        *hit_rate = (total > 0) ? ((double)ctx->cache_hits / (double)total) : 0.0;
    }
    
    if (total_calcs) {
        *total_calcs = ctx->total_calculations;
    }
}

/**
 * @brief Calculate normalized entropy
 * 
 * Returns entropy normalized to [0, 1] range.
 * Maximum entropy is log₂(d) for dimension d.
 */
double calculate_normalized_entropy(EntropyContext *ctx, uint64_t n, uint32_t d) {
    if (d == 0 || d > MAX_ENTROPY_DIMENSION) {
        return 0.0;
    }
    
    double entropy = calculate_lattice_entropy(ctx, n, d);
    
    // Maximum possible entropy for dimension d
    double max_entropy = math_log2((double)(d + 1));
    
    if (max_entropy < ENTROPY_EPSILON) {
        return 0.0;
    }
    
    // Normalize to [0, 1]
    double normalized = entropy / max_entropy;
    
    // Clamp to valid range
    if (normalized < 0.0) normalized = 0.0;
    if (normalized > 1.0) normalized = 1.0;
    
    return normalized;
}