/**
 * @file nonce_generation.h
 * @brief Deterministic nonce generation for proof-of-work systems
 * 
 * This module implements a deterministic nonce generation algorithm for
 * Bitcoin mining and other proof-of-work systems. It combines:
 * - Symbolic Field Theory (SFT) for seed prime selection
 * - Tetration for difficulty scaling
 * - Entropy reduction for search space bounds
 * 
 * The algorithm is deterministic (reproducible) and entropy-guided for
 * efficient search space exploration.
 * 
 * Applications:
 * - Bitcoin mining (testnet and mainnet)
 * - Other proof-of-work cryptocurrencies
 * - Computational puzzles requiring deterministic solutions
 */

#ifndef NONCE_GENERATION_H
#define NONCE_GENERATION_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Nonce generation configuration
 */
typedef struct {
    uint64_t block_height;      /**< Block height (used as seed) */
    uint32_t difficulty_bits;   /**< Target difficulty in bits */
    uint32_t tetration_depth;   /**< Depth of tetration stack (default: 3) */
    uint32_t max_reassessments; /**< Maximum reassessment attempts (default: 5) */
    double entropy_cut_min;     /**< Minimum entropy cut (default: 0.18) */
    double entropy_cut_max;     /**< Maximum entropy cut (default: 0.45) */
} NonceConfig;

/**
 * @brief Nonce generation result
 */
typedef struct {
    uint64_t nonce;             /**< Generated nonce value */
    uint64_t seed_prime;        /**< Seed prime used */
    uint32_t reassessments;     /**< Number of reassessments performed */
    bool success;               /**< Whether generation succeeded */
    double final_entropy;       /**< Final entropy after reduction */
} NonceResult;

/**
 * @brief Initialize nonce configuration with defaults
 * 
 * Sets reasonable defaults for all configuration parameters:
 * - tetration_depth = 3
 * - max_reassessments = 5
 * - entropy_cut_min = 0.18 (18% reduction)
 * - entropy_cut_max = 0.45 (45% reduction)
 * 
 * @param config Configuration structure to initialize
 * @param block_height Block height to use as seed
 * @param difficulty_bits Target difficulty in bits
 */
void nonce_config_init(NonceConfig* config, uint64_t block_height, 
                       uint32_t difficulty_bits);

/**
 * @brief Generate deterministic nonce
 * 
 * Main function for generating a nonce using the deterministic algorithm.
 * 
 * Algorithm:
 * 1. Use SFT to find seed prime from block_height
 * 2. Build tetration stack (depth 3) from seed
 * 3. Apply entropy reduction based on difficulty
 * 4. Generate nonce candidate
 * 5. Reassess if needed (up to max_reassessments times)
 * 
 * @param config Nonce generation configuration
 * @param result Output result structure
 * @return true if nonce generated successfully, false otherwise
 * 
 * Example:
 * ```c
 * NonceConfig config;
 * nonce_config_init(&config, 12345, 24);  // Block 12345, 24-bit difficulty
 * 
 * NonceResult result;
 * if (nonce_generate_deterministic(&config, &result)) {
 *     printf("Nonce: %lu\n", result.nonce);
 *     printf("Seed prime: %lu\n", result.seed_prime);
 *     printf("Reassessments: %u\n", result.reassessments);
 * }
 * ```
 */
bool nonce_generate_deterministic(const NonceConfig* config, NonceResult* result);

/**
 * @brief Build tetration stack from seed prime
 * 
 * Creates a tetration stack: p^(p^(p^...)) to specified depth.
 * Uses modular arithmetic to prevent overflow.
 * 
 * @param seed_prime Seed prime number
 * @param depth Depth of tetration (typically 3)
 * @param modulus Modulus for overflow prevention
 * @return Top value of tetration stack
 * 
 * Example:
 * - depth=1: p
 * - depth=2: p^p
 * - depth=3: p^(p^p)
 */
uint64_t nonce_build_tetration_stack(uint64_t seed_prime, uint32_t depth,
                                      uint64_t modulus);

/**
 * @brief Build tetration stack using CrystallineAbacus (NEW API)
 * 
 * Uses arbitrary precision arithmetic for large intermediate values.
 * Returns CrystallineAbacus* that must be freed by caller.
 * 
 * @param seed_prime Seed prime value
 * @param depth Tetration depth
 * @param modulus Modulus for reduction
 * @param ctx Clock context for operations
 * @return CrystallineAbacus* result (caller must free) or NULL on error
 */
// Include the actual type definitions
#include "math/abacus.h"
#include "math/types.h"

CrystallineAbacus* nonce_build_tetration_stack_abacus(
    uint64_t seed_prime, 
    uint32_t depth,
    uint64_t modulus,
    ClockContext* ctx);

/**
 * @brief Apply difficulty-based bounds using entropy reduction
 * 
 * Reduces the search space based on difficulty target using entropy
 * reduction principles. Higher difficulty = more entropy reduction.
 * 
 * @param tetration_value Value from tetration stack
 * @param difficulty_bits Target difficulty in bits
 * @param entropy_cut_min Minimum entropy cut (0.0 to 1.0)
 * @param entropy_cut_max Maximum entropy cut (0.0 to 1.0)
 * @return Bounded nonce value
 */
uint64_t nonce_apply_difficulty_bounds(uint64_t tetration_value,
                                        uint32_t difficulty_bits,
                                        double entropy_cut_min,
                                        double entropy_cut_max);

/**
 * @brief Calculate entropy reduction factor
 * 
 * Determines how much to reduce entropy based on difficulty.
 * Uses golden ratio (φ) for smooth scaling.
 * 
 * @param difficulty_bits Target difficulty in bits
 * @param cut_min Minimum cut percentage
 * @param cut_max Maximum cut percentage
 * @return Entropy reduction factor (0.0 to 1.0)
 */
double nonce_calculate_entropy_reduction(uint32_t difficulty_bits,
                                          double cut_min, double cut_max);

/**
 * @brief Reassess nonce with adjusted parameters
 * 
 * If initial nonce doesn't meet requirements, reassess with adjusted
 * entropy parameters. Uses golden ratio for parameter adjustment.
 * 
 * @param config Original configuration
 * @param attempt Reassessment attempt number (1-based)
 * @param previous_nonce Previous nonce value
 * @return New nonce candidate
 */
uint64_t nonce_reassess(const NonceConfig* config, uint32_t attempt,
                        uint64_t previous_nonce);

/**
 * @brief Validate nonce meets difficulty target
 * 
 * Checks if a nonce value meets the specified difficulty requirement.
 * This is a simplified check - actual Bitcoin mining would hash the
 * block header and check leading zeros.
 * 
 * @param nonce Nonce value to validate
 * @param difficulty_bits Target difficulty in bits
 * @return true if nonce meets difficulty, false otherwise
 */
bool nonce_validate_difficulty(uint64_t nonce, uint32_t difficulty_bits);

#ifdef __cplusplus
}
#endif

#endif /* NONCE_GENERATION_H */
