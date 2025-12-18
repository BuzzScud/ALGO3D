/**
 * search_recovery.c - Search-Based K Recovery Implementation
 * 
 * The correct approach: Use binary search with forward mapping
 * instead of trying to invert the formula.
 */

#include "search_recovery.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define PHI ((1.0 + sqrt(5.0)) / 2.0)
#define PI M_PI

SearchRecoveryContext* init_search_recovery(
    PlatonicOverlay* overlay,
    uint64_t min_k,
    uint64_t max_k
) {
    SearchRecoveryContext* ctx = calloc(1, sizeof(SearchRecoveryContext));
    if (!ctx) return NULL;
    
    ctx->overlay = overlay;
    ctx->min_k = min_k;
    ctx->max_k = max_k;
    ctx->max_iterations = 100;
    ctx->tolerance = 0.01;  // 0.01 radians ≈ 0.57°
    
    return ctx;
}

void free_search_recovery(SearchRecoveryContext* ctx) {
    if (ctx) {
        free(ctx);
    }
}

uint64_t binary_search_k(
    ClockPosition target,
    uint64_t min_k,
    uint64_t max_k,
    uint32_t max_iterations,
    double* final_error
) {
    uint64_t left = min_k;
    uint64_t right = max_k;
    uint64_t best_k = min_k;
    double best_error = 1e9;
    
    // Normalize target angle to [0, 2π)
    double target_angle = fmod(target.angle, 2.0 * PI);
    if (target_angle < 0) target_angle += 2.0 * PI;
    
    for (uint32_t iter = 0; iter < max_iterations && left <= right; iter++) {
        uint64_t mid = left + (right - left) / 2;
        
        // Compute angle using forward mapping: θ = k·π(1+√5)
        double computed_angle = fmod((double)mid * PI * PHI, 2.0 * PI);
        if (computed_angle < 0) computed_angle += 2.0 * PI;
        
        // Compute error (handle wraparound)
        double error = fabs(computed_angle - target_angle);
        if (error > PI) error = 2.0 * PI - error;
        
        // Update best
        if (error < best_error) {
            best_error = error;
            best_k = mid;
        }
        
        // Binary search logic
        if (computed_angle < target_angle) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    
    if (final_error) *final_error = best_error;
    
    return best_k;
}

uint64_t anchor_guided_search_k(
    ClockPosition target,
    SearchRecoveryContext* ctx,
    double* confidence
) {
    if (!ctx) {
        if (confidence) *confidence = 0.0;
        return 0;
    }
    
    // Use anchors to narrow search range
    uint64_t min_k = ctx->min_k;
    uint64_t max_k = ctx->max_k;
    
    if (ctx->overlay) {
        // Find nearest anchor
        PlatonicSolid* icosa = &ctx->overlay->solids[SOLID_ICOSAHEDRON];
        double min_dist = 1e9;
        int nearest_idx = -1;
        
        for (uint32_t i = 0; i < icosa->num_vertices; i++) {
            double dx = target.angle - icosa->clock_positions[i].angle;
            double dy = target.radius - icosa->clock_positions[i].radius;
            double dist = sqrt(dx*dx + dy*dy);
            
            if (dist < min_dist) {
                min_dist = dist;
                nearest_idx = i;
            }
        }
        
        if (nearest_idx >= 0) {
            // Estimate k range based on nearest anchor
            // Assume k is within ±50 of anchor position
            uint64_t anchor_k_estimate = (uint64_t)(icosa->clock_positions[nearest_idx].angle / (PI * PHI));
            
            if (anchor_k_estimate > 50) {
                min_k = anchor_k_estimate - 50;
            } else {
                min_k = 0;
            }
            max_k = anchor_k_estimate + 50;
            
            // Clamp to context bounds
            if (min_k < ctx->min_k) min_k = ctx->min_k;
            if (max_k > ctx->max_k) max_k = ctx->max_k;
        }
    }
    
    // Binary search in narrowed range
    double error = 0.0;
    uint64_t k = binary_search_k(target, min_k, max_k, ctx->max_iterations, &error);
    
    // Compute confidence based on error
    if (confidence) {
        if (error < ctx->tolerance) {
            *confidence = 1.0 - (error / ctx->tolerance);
        } else {
            *confidence = 0.0;
        }
    }
    
    ctx->total_searches++;
    ctx->total_iterations += ctx->max_iterations;
    
    if (error < ctx->tolerance) {
        ctx->successful_searches++;
    }
    
    return k;
}

uint64_t quadrant_search_k(
    ClockPosition target,
    SearchRecoveryContext* ctx,
    double* confidence
) {
    if (!ctx) {
        if (confidence) *confidence = 0.0;
        return 0;
    }
    
    // Use quadrant to set search bounds
    // Based on discovery that k values are distributed across quadrants
    
    double angle = fmod(target.angle, 2.0 * PI);
    if (angle < 0) angle += 2.0 * PI;
    
    uint64_t min_k = ctx->min_k;
    uint64_t max_k = ctx->max_k;
    
    // Quadrant-based bounds (rough estimates)
    if (angle < PI / 2.0) {
        // Q1: TOP-RIGHT (optimal zone!)
        // k values tend to be small here
        max_k = (ctx->max_k - ctx->min_k) / 4 + ctx->min_k;
    } else if (angle < PI) {
        // Q2: BOTTOM-RIGHT
        min_k = (ctx->max_k - ctx->min_k) / 4 + ctx->min_k;
        max_k = (ctx->max_k - ctx->min_k) / 2 + ctx->min_k;
    } else if (angle < 3.0 * PI / 2.0) {
        // Q3: BOTTOM-LEFT
        min_k = (ctx->max_k - ctx->min_k) / 2 + ctx->min_k;
        max_k = 3 * (ctx->max_k - ctx->min_k) / 4 + ctx->min_k;
    } else {
        // Q4: TOP-LEFT
        min_k = 3 * (ctx->max_k - ctx->min_k) / 4 + ctx->min_k;
    }
    
    // Binary search in quadrant range
    double error = 0.0;
    uint64_t k = binary_search_k(target, min_k, max_k, ctx->max_iterations, &error);
    
    // Compute confidence
    if (confidence) {
        if (error < ctx->tolerance) {
            *confidence = 1.0 - (error / ctx->tolerance);
        } else {
            *confidence = 0.0;
        }
    }
    
    ctx->total_searches++;
    ctx->total_iterations += ctx->max_iterations;
    
    if (error < ctx->tolerance) {
        ctx->successful_searches++;
    }
    
    return k;
}

uint64_t recursive_torus_search_k(
    ClockPosition target,
    SearchRecoveryContext* ctx,
    uint32_t recursion_depth,
    double* confidence
) {
    if (!ctx || recursion_depth > 10) {
        if (confidence) *confidence = 0.0;
        return 0;
    }
    
    // Layer 1: Coarse search
    uint64_t k_coarse = anchor_guided_search_k(target, ctx, NULL);
    
    // Verify with forward mapping
    ClockPosition computed = map_k_to_clock_u64(k_coarse);
    
    double angle_error = fabs(computed.angle - target.angle);
    if (angle_error > PI) angle_error = 2.0 * PI - angle_error;
    
    // If error is small enough, we're done
    if (angle_error < ctx->tolerance) {
        if (confidence) *confidence = 1.0 - (angle_error / ctx->tolerance);
        return k_coarse;
    }
    
    // Layer 2: Fine search around coarse result
    uint64_t min_k = (k_coarse > 10) ? k_coarse - 10 : 0;
    uint64_t max_k = k_coarse + 10;
    
    double error = 0.0;
    uint64_t k_fine = binary_search_k(target, min_k, max_k, ctx->max_iterations, &error);
    
    // Compute confidence
    if (confidence) {
        if (error < ctx->tolerance) {
            *confidence = 1.0 - (error / ctx->tolerance);
        } else {
            *confidence = 0.0;
        }
    }
    
    return k_fine;
}

void batch_search_recovery(
    ClockPosition* targets,
    uint32_t num_targets,
    uint64_t* recovered_k_values,
    double* confidence_scores,
    SearchRecoveryContext* ctx
) {
    for (uint32_t i = 0; i < num_targets; i++) {
        double confidence = 0.0;
        recovered_k_values[i] = recursive_torus_search_k(targets[i], ctx, 0, &confidence);
        
        if (confidence_scores) {
            confidence_scores[i] = confidence;
        }
    }
}

void get_search_statistics(
    SearchRecoveryContext* ctx,
    uint32_t* total,
    uint32_t* successful,
    double* success_rate,
    double* avg_iterations
) {
    if (!ctx) return;
    
    if (total) *total = ctx->total_searches;
    if (successful) *successful = ctx->successful_searches;
    
    if (success_rate) {
        if (ctx->total_searches > 0) {
            *success_rate = (double)ctx->successful_searches / (double)ctx->total_searches;
        } else {
            *success_rate = 0.0;
        }
    }
    
    if (avg_iterations) {
        if (ctx->total_searches > 0) {
            *avg_iterations = (double)ctx->total_iterations / (double)ctx->total_searches;
        } else {
            *avg_iterations = 0.0;
        }
    }
}