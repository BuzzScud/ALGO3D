/**
 * search_recovery.h - Search-Based K Recovery
 * 
 * Instead of trying to invert the formula θ = k·π(1+√5), we use SEARCH:
 * 1. Binary search on k values
 * 2. For each candidate k, compute θ using forward mapping
 * 3. Compare with target θ
 * 4. Narrow search range
 * 5. Use anchors and quadrant info to guide search
 * 
 * This is the correct "recursive torus discovery" approach!
 */

#ifndef SEARCH_RECOVERY_H
#define SEARCH_RECOVERY_H

#include <stdint.h>
#include <stdbool.h>
#include "clock_recovery.h"
#include "platonic_solids.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Search recovery context
 */
typedef struct {
    PlatonicOverlay* overlay;
    uint64_t min_k;  // Minimum k to search
    uint64_t max_k;  // Maximum k to search
    uint32_t max_iterations;
    double tolerance;
    
    // Statistics
    uint32_t total_searches;
    uint32_t successful_searches;
    uint64_t total_iterations;
} SearchRecoveryContext;

/**
 * Initialize search recovery context
 */
SearchRecoveryContext* init_search_recovery(
    PlatonicOverlay* overlay,
    uint64_t min_k,
    uint64_t max_k
);

/**
 * Free search recovery context
 */
void free_search_recovery(SearchRecoveryContext* ctx);

/**
 * Binary search for k given target clock position
 * Uses forward mapping: θ = k·π(1+√5)
 */
uint64_t binary_search_k(
    ClockPosition target,
    uint64_t min_k,
    uint64_t max_k,
    uint32_t max_iterations,
    double* final_error
);

/**
 * Anchor-guided search for k
 * Uses known k values (anchors) to narrow search range
 */
uint64_t anchor_guided_search_k(
    ClockPosition target,
    SearchRecoveryContext* ctx,
    double* confidence
);

/**
 * Quadrant-aware search for k
 * Uses quadrant information to set initial search bounds
 */
uint64_t quadrant_search_k(
    ClockPosition target,
    SearchRecoveryContext* ctx,
    double* confidence
);

/**
 * Recursive torus search for k
 * This is the full recursive algorithm:
 * 1. Search at current resolution
 * 2. Detect oscillation boundaries
 * 3. Create new torus from oscillation
 * 4. Recurse with finer resolution
 */
uint64_t recursive_torus_search_k(
    ClockPosition target,
    SearchRecoveryContext* ctx,
    uint32_t recursion_depth,
    double* confidence
);

/**
 * Batch search recovery
 */
void batch_search_recovery(
    ClockPosition* targets,
    uint32_t num_targets,
    uint64_t* recovered_k_values,
    double* confidence_scores,
    SearchRecoveryContext* ctx
);

/**
 * Get search statistics
 */
void get_search_statistics(
    SearchRecoveryContext* ctx,
    uint32_t* total,
    uint32_t* successful,
    double* success_rate,
    double* avg_iterations
);

#ifdef __cplusplus
}
#endif

#endif // SEARCH_RECOVERY_H
