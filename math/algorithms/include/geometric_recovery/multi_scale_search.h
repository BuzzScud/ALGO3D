/**
 * @file multi_scale_search.h
 * @brief Multi-Scale Fractal Search for Geometric Recovery
 * 
 * UNIVERSAL GEOMETRIC MATHEMATICS - Works with ANY system!
 * 
 * Implements multi-scale search:
 * - Search at multiple scales (coarse to fine)
 * - Fractal refinement at each scale
 * - Attractor-guided search
 * - Adaptive scale selection
 * 
 * NO crypto-specific dependencies - works with raw uint64_t data.
 * 
 * Thesis Reference: Section 16.4 - Multi-Scale Search
 */

#ifndef MULTI_SCALE_SEARCH_H
#define MULTI_SCALE_SEARCH_H

#include <stdint.h>
#include <stdbool.h>

// Forward declarations
typedef struct TetrationSystem TetrationSystem;

// Include fractal_bounds for FractalPartition type
#include "geometric_recovery/fractal_bounds.h"

#define MULTI_SCALE_MAX_DIMENSIONS 13
#define MULTI_SCALE_MAX_SCALES 10

/**
 * Scale level for multi-scale search
 */
typedef struct {
    uint32_t scale_index;               // 0, 1, 2, ... (0 = full scale)
    double scale_factor;                // 1.0, 0.5, 0.25, ...
    uint32_t num_candidates;            // Candidates at this scale
    uint64_t* candidates;               // Candidate values
    double* scores;                     // Score for each candidate
    double best_score;                  // Best score at this scale
    uint64_t best_candidate;            // Best candidate at this scale
} ScaleLevel;

/**
 * Multi-scale search context
 */
typedef struct MultiScaleSearchContext MultiScaleSearchContext;

/**
 * Create multi-scale search context
 * 
 * @param n System size
 * @param max_scales Maximum number of scales
 * @param attractors Tetration attractors for guidance (optional)
 * @return New context or NULL on error
 */
MultiScaleSearchContext* multi_scale_search_create(
    uint64_t n,
    uint32_t max_scales,
    TetrationSystem* attractors
);

/**
 * Perform multi-scale search
 * 
 * Searches at multiple scales from coarse to fine,
 * using attractors and fractal bounds for guidance.
 * 
 * @param ctx Context
 * @param start_position Starting position in 13D space
 * @param target Target output value
 * @param bounds Fractal partition bounds (optional)
 * @param result_out Pointer to store best candidate
 * @return true on success, false on error
 */
bool multi_scale_search(
    MultiScaleSearchContext* ctx,
    const double start_position[MULTI_SCALE_MAX_DIMENSIONS],
    uint64_t target,
    const FractalPartition* bounds,
    uint64_t* result_out
);

/**
 * Get scale level results
 * 
 * @param ctx Context
 * @param scale_index Scale index
 * @return Scale level or NULL if invalid
 */
const ScaleLevel* multi_scale_get_level(
    const MultiScaleSearchContext* ctx,
    uint32_t scale_index
);

/**
 * Get number of scales used
 * 
 * @param ctx Context
 * @return Number of scales
 */
uint32_t multi_scale_get_num_scales(const MultiScaleSearchContext* ctx);

/**
 * Get best candidate across all scales
 * 
 * @param ctx Context
 * @param candidate_out Pointer to store best candidate
 * @param score_out Pointer to store best score
 * @return true on success, false on error
 */
bool multi_scale_get_best(
    const MultiScaleSearchContext* ctx,
    uint64_t* candidate_out,
    double* score_out
);

/**
 * Print multi-scale search results
 * 
 * @param ctx Context
 */
void print_multi_scale_results(const MultiScaleSearchContext* ctx);

/**
 * Destroy multi-scale search context
 * 
 * @param ctx Context to destroy
 */
void multi_scale_search_destroy(MultiScaleSearchContext* ctx);

#endif // MULTI_SCALE_SEARCH_H