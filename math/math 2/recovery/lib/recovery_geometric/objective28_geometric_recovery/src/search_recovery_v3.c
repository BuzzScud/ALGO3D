/**
 * search_recovery_v3.c - Enhanced Search with 24-Anchor Grid
 * 
 * Improvements over v2:
 * - Uses 24 anchors instead of 12 for finer granularity
 * - Better k estimation from weighted average
 * - Expected improvement: 20% → 30% success rate
 */

#include "search_recovery_v3.h"
#include "anchor_grid_24.h"
#include <stdlib.h>
#include <string.h>
#include "math/arithmetic.h"
#include "math/transcendental.h"

#define PHI ((1.0 + math_sqrt(5.0)) / 2.0)
#define PI M_PI

SearchRecoveryV3Context* init_search_recovery_v3(uint64_t min_k, uint64_t max_k) {
    SearchRecoveryV3Context* ctx = calloc(1, sizeof(SearchRecoveryV3Context));
    if (!ctx) return NULL;
    
    ctx->min_k = min_k;
    ctx->max_k = max_k;
    
    // Create 24-anchor grid
    ctx->anchor_grid = create_anchor_grid_24(max_k);
    if (!ctx->anchor_grid) {
        free(ctx);
        return NULL;
    }
    
    // Same 3-layer configuration as v2 (proven to work)
    ctx->layers[0] = (SearchLayerV3){.range = 100, .step = 10, .max_iter = 50};
    ctx->layers[1] = (SearchLayerV3){.range = 25, .step = 2, .max_iter = 50};
    ctx->layers[2] = (SearchLayerV3){.range = 10, .step = 1, .max_iter = 50};
    ctx->num_layers = 3;
    
    return ctx;
}

void free_search_recovery_v3(SearchRecoveryV3Context* ctx) {
    if (ctx) {
        if (ctx->anchor_grid) {
            free_anchor_grid_24(ctx->anchor_grid);
        }
        free(ctx);
    }
}

static uint64_t recursive_search_v3(
    double target_angle,
    uint64_t center_k,
    SearchLayerV3* layers,
    int num_layers,
    double* final_error
) {
    uint64_t best_k = center_k;
    double best_error = 1e9;
    
    // Normalize target angle
    double normalized = math_fmod(target_angle, 2.0 * PI);
    if (normalized < 0) normalized += 2.0 * PI;
    
    // Iterate through layers
    for (int layer = 0; layer < num_layers; layer++) {
        SearchLayerV3* config = &layers[layer];
        
        // Search range for this layer
        int64_t min_offset = -config->range;
        int64_t max_offset = config->range;
        
        // Search with current step size
        for (int64_t offset = min_offset; offset <= max_offset; offset += config->step) {
            int64_t candidate_k = (int64_t)best_k + offset;
            if (candidate_k < 0) continue;
            
            uint64_t k = (uint64_t)candidate_k;
            
            // Forward mapping: θ = k·π(1+√5)
            double computed_angle = math_fmod((double)k * PI * PHI, 2.0 * PI);
            if (computed_angle < 0) computed_angle += 2.0 * PI;
            
            // Compute error (handle wraparound)
            double error = math_abs(computed_angle - normalized);
            if (error > PI) error = 2.0 * PI - error;
            
            // Update best
            if (error < best_error) {
                best_error = error;
                best_k = k;
            }
        }
        
        // Center next layer on current best
        center_k = best_k;
    }
    
    if (final_error) *final_error = best_error;
    return best_k;
}

uint64_t search_with_24_anchors(
    double target_angle,
    SearchRecoveryV3Context* ctx,
    double* confidence
) {
    if (!ctx || !ctx->anchor_grid) {
        if (confidence) *confidence = 0.0;
        return 0;
    }
    
    // Find 3 nearest anchors from 24-anchor grid
    Anchor24* nearest[3];
    int num_found = find_nearest_anchors_24(ctx->anchor_grid, target_angle, nearest);
    
    if (num_found == 0) {
        if (confidence) *confidence = 0.0;
        return 0;
    }
    
    // Compute weighted k estimate
    uint64_t center_k = compute_weighted_k_24(nearest, num_found, target_angle);
    
    // Perform 3-layer recursive search
    double error = 0.0;
    uint64_t recovered_k = recursive_search_v3(
        target_angle,
        center_k,
        ctx->layers,
        ctx->num_layers,
        &error
    );
    
    // Compute confidence
    if (confidence) {
        // Confidence based on error (lower error = higher confidence)
        if (error < 0.01) {
            *confidence = 1.0 - (error / 0.01);
        } else {
            *confidence = 0.0;
        }
    }
    
    // Update statistics
    ctx->total_searches++;
    if (error < 0.01) {
        ctx->successful_searches++;
    }
    
    return recovered_k;
}
