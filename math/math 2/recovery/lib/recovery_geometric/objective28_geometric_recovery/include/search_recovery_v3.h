/**
 * search_recovery_v3.h - Enhanced Search with 24-Anchor Grid
 */

#ifndef SEARCH_RECOVERY_V3_H
#define SEARCH_RECOVERY_V3_H

#include "anchor_grid_24.h"
#include <stdint.h>

// Search layer configuration
typedef struct {
    int64_t range;      // Search range (±range)
    uint64_t step;      // Step size
    uint32_t max_iter;  // Max iterations
} SearchLayerV3;

// Search context with 24-anchor grid
typedef struct {
    AnchorGrid24* anchor_grid;  // 24-anchor grid
    
    uint64_t min_k;
    uint64_t max_k;
    
    SearchLayerV3 layers[3];    // 3-layer search
    int num_layers;
    
    // Statistics
    uint64_t total_searches;
    uint64_t successful_searches;
} SearchRecoveryV3Context;

/**
 * Initialize v3 search context with 24-anchor grid
 */
SearchRecoveryV3Context* init_search_recovery_v3(
    uint64_t min_k,
    uint64_t max_k
);

/**
 * Free v3 search context
 */
void free_search_recovery_v3(SearchRecoveryV3Context* ctx);

/**
 * Search with 24-anchor grid
 * 
 * Uses 24 anchors for finer granularity in k estimation.
 * Expected to improve success rate from 20% to 30%.
 */
uint64_t search_with_24_anchors(
    double target_angle,
    SearchRecoveryV3Context* ctx,
    double* confidence
);

#endif // SEARCH_RECOVERY_V3_H
