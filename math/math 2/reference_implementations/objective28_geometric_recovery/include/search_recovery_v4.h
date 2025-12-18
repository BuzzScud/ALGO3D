/**
 * search_recovery_v4.h - Direct K-Based Anchor Selection
 * 
 * THE FIX: Select anchors by k value, not by angle!
 * 
 * This ensures all k values are within 12.5 units of an anchor,
 * which guarantees the search will find them.
 */

#ifndef SEARCH_RECOVERY_V4_H
#define SEARCH_RECOVERY_V4_H

#include <stdint.h>
#include <stdbool.h>

// Search layer configuration
typedef struct {
    int64_t range;
    uint64_t step;
} SearchLayerV4;

// Recovery context with direct k-based anchors
typedef struct {
    uint64_t* anchors;       // Array of anchor k values
    int num_anchors;         // Number of anchors (12)
    uint64_t anchor_spacing; // Spacing between anchors (25)
    
    uint64_t min_k;
    uint64_t max_k;
    
    SearchLayerV4 layers[3];
    int num_layers;
    
    // Statistics
    uint64_t total_searches;
    uint64_t successful_searches;
} SearchRecoveryV4Context;

/**
 * Initialize v4 context with direct k-based anchors
 */
SearchRecoveryV4Context* init_search_recovery_v4(
    uint64_t min_k,
    uint64_t max_k
);

/**
 * Free v4 context
 */
void free_search_recovery_v4(SearchRecoveryV4Context* ctx);

/**
 * Find nearest anchor by k value (NOT by angle!)
 * 
 * This is the KEY FIX that should improve success rate to 90-100%.
 */
uint64_t find_nearest_anchor_k(
    SearchRecoveryV4Context* ctx,
    uint64_t target_k
);

/**
 * Recover k with direct anchor selection
 * 
 * For testing: We know the target k, so we can select the correct anchor.
 * Expected: 90-100% success rate
 */
uint64_t recover_k_direct_anchors(
    uint64_t target_k,
    SearchRecoveryV4Context* ctx,
    double* confidence
);

/**
 * Recover k from angle (production mode)
 * 
 * In production, we only have Q (which gives us angle).
 * We must estimate k from angle, then find nearest anchor.
 * Expected: Lower success rate than direct, but still better than v2
 */
uint64_t recover_k_from_angle_v4(
    double target_angle,
    SearchRecoveryV4Context* ctx,
    double* confidence
);

#endif // SEARCH_RECOVERY_V4_H