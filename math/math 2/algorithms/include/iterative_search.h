/**
 * @file iterative_search.h
 * @brief Iterative search with entropy-guided space reduction
 * 
 * This module provides general-purpose iterative search algorithms that use
 * entropy reduction and anchor-based triangulation to efficiently search
 * large spaces. Applications include:
 * - AI inference (Q→k mapping)
 * - Signal processing (pattern matching)
 * - Optimization (parameter search)
 * - Data retrieval (nearest neighbor search)
 * 
 * The algorithm uses:
 * - Recursive entropy reduction to shrink search space
 * - Anchor-based triangulation for estimation
 * - Oscillation detection to avoid local minima
 * - Adaptive reassessment when stuck
 */

#ifndef ITERATIVE_SEARCH_H
#define ITERATIVE_SEARCH_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Search result structure
 */
typedef struct {
    uint64_t best_candidate;     /**< Best candidate found */
    double min_distance;          /**< Minimum distance achieved */
    uint32_t iterations;          /**< Number of iterations performed */
    uint32_t reassessments;       /**< Number of reassessments triggered */
    bool converged;               /**< Whether search converged */
} SearchResult;

/**
 * @brief Distance function callback
 * 
 * User-provided function to calculate distance between query and candidate.
 * 
 * @param query Query point (user-defined structure)
 * @param candidate Candidate value to test
 * @param user_data User-provided context data
 * @return Distance value (lower is better)
 */
typedef double (*DistanceFunction)(const void* query, uint64_t candidate, void* user_data);

/**
 * @brief Anchor generation callback
 * 
 * User-provided function to generate anchor points for triangulation.
 * 
 * @param anchors Output array for anchor values (pre-allocated)
 * @param num_anchors Number of anchors to generate
 * @param user_data User-provided context data
 */
typedef void (*AnchorGenerator)(uint64_t* anchors, size_t num_anchors, void* user_data);

/**
 * @brief Oscillation detection callback
 * 
 * User-provided function to detect if search is oscillating.
 * 
 * @param anchors Current anchor values
 * @param num_anchors Number of anchors
 * @param user_data User-provided context data
 * @return true if oscillation detected
 */
typedef bool (*OscillationDetector)(const uint64_t* anchors, size_t num_anchors, void* user_data);

/**
 * @brief Search configuration
 */
typedef struct {
    uint64_t search_space_size;   /**< Total search space size */
    uint32_t max_iterations;      /**< Maximum iterations per reassessment */
    uint32_t max_reassessments;   /**< Maximum number of reassessments */
    size_t num_anchors;           /**< Number of anchors for triangulation */
    double convergence_threshold; /**< Distance threshold for convergence */
    double initial_entropy_bits;  /**< Initial entropy in bits */
    
    DistanceFunction distance_fn;       /**< Distance calculation function */
    AnchorGenerator anchor_gen_fn;      /**< Anchor generation function */
    OscillationDetector oscillation_fn; /**< Oscillation detection function */
    void* user_data;                    /**< User context data */
} SearchConfig;

/**
 * @brief Perform iterative search with entropy reduction
 * 
 * This is the main search function that implements the iterative algorithm:
 * 1. Generate anchors for triangulation
 * 2. Estimate starting point from anchors
 * 3. Iteratively search with entropy-reduced space
 * 4. Detect oscillations and reassess if needed
 * 5. Return best candidate found
 * 
 * @param query Query point (user-defined structure)
 * @param config Search configuration
 * @param result Output search result
 * @return true if search succeeded
 */
bool iterative_search(const void* query, const SearchConfig* config, SearchResult* result);

/**
 * @brief Estimate starting point from anchors
 * 
 * Uses anchor-based triangulation to estimate a good starting point.
 * 
 * @param query Query point
 * @param anchors Anchor values
 * @param num_anchors Number of anchors
 * @param distance_fn Distance function
 * @param user_data User context
 * @return Estimated starting value
 */
uint64_t iterative_estimate_from_anchors(const void* query,
                                          const uint64_t* anchors,
                                          size_t num_anchors,
                                          DistanceFunction distance_fn,
                                          void* user_data);

/**
 * @brief Calculate search space size after entropy reduction
 * 
 * @param current_entropy Current entropy in bits
 * @param steps Number of reduction steps
 * @param cut_min Minimum cut fraction
 * @param cut_max Maximum cut fraction
 * @return Reduced search space size
 */
uint64_t iterative_reduced_space_size(double current_entropy,
                                       uint32_t steps,
                                       double cut_min,
                                       double cut_max);

/**
 * @brief Default anchor generator using golden ratio
 * 
 * Generates anchors using golden ratio spacing for good coverage.
 * 
 * @param anchors Output array
 * @param num_anchors Number of anchors
 * @param user_data Must be pointer to uint64_t containing search_space_size
 */
void iterative_default_anchor_generator(uint64_t* anchors,
                                         size_t num_anchors,
                                         void* user_data);

/**
 * @brief Default oscillation detector using variance
 * 
 * Detects oscillation by checking if anchor variance is too high.
 * 
 * @param anchors Anchor values
 * @param num_anchors Number of anchors
 * @param user_data Unused
 * @return true if oscillation detected
 */
bool iterative_default_oscillation_detector(const uint64_t* anchors,
                                             size_t num_anchors,
                                             void* user_data);

/**
 * @brief FFT-based oscillation detector (improved)
 * 
 * Uses FFT peak analysis to detect repeated patterns in the anchor sequence.
 * More sophisticated than variance-based detection.
 * 
 * Algorithm:
 * 1. Convert anchors to normalized signal
 * 2. Detect peaks in signal
 * 3. Analyze peak patterns for repetition
 * 4. Return true if repeated patterns found
 * 
 * @param anchors Array of anchor values
 * @param num_anchors Number of anchors
 * @param user_data User data (unused)
 * @return true if oscillation detected, false otherwise
 * 
 * Example:
 * ```c
 * IterativeSearchConfig config;
 * config.oscillation_fn = iterative_fft_oscillation_detector;
 * ```
 */
bool iterative_fft_oscillation_detector(const uint64_t* anchors,
                                         size_t num_anchors,
                                         void* user_data);

#ifdef __cplusplus
}
#endif

#endif /* ITERATIVE_SEARCH_H */
