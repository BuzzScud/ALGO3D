#include "../include/search_recovery_v5.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

// Normalize angle to [0, 2π]
static double normalize_angle(double angle) {
    while (angle < 0) angle += 2.0 * M_PI;
    while (angle >= 2.0 * M_PI) angle -= 2.0 * M_PI;
    return angle;
}

// Compute angle difference (shortest path)
static double angle_difference(double a1, double a2) {
    double diff = fabs(a1 - a2);
    if (diff > M_PI) diff = 2.0 * M_PI - diff;
    return diff;
}

double compute_combined_error(
    ClockPosition target,
    ClockPosition candidate,
    ErrorWeights weights
) {
    // Normalize angles
    double target_angle = normalize_angle(target.angle);
    double candidate_angle = normalize_angle(candidate.angle);
    
    // Compute angle error (normalized to [0, 1])
    double angle_error = angle_difference(target_angle, candidate_angle) / M_PI;
    
    // Compute radius error (already in [0, 1] range)
    double radius_error = fabs(target.radius - candidate.radius);
    
    // Combined error with weights
    double combined = weights.angle_weight * angle_error + 
                     weights.radius_weight * radius_error;
    
    return combined;
}

ErrorWeights compute_adaptive_weights(double target_radius) {
    ErrorWeights weights;
    
    if (target_radius >= 0.75) {
        // High radius: Trust radius more
        weights.angle_weight = 0.3;
        weights.radius_weight = 0.7;
    } else if (target_radius >= 0.5) {
        // Mid radius: Balanced
        weights.angle_weight = 0.5;
        weights.radius_weight = 0.5;
    } else {
        // Low radius: Trust angle more (radius less reliable)
        weights.angle_weight = 0.7;
        weights.radius_weight = 0.3;
    }
    
    return weights;
}

RadiusConfidenceMetrics compute_radius_confidence(
    ClockPosition target,
    ClockPosition best,
    double combined_error
) {
    RadiusConfidenceMetrics metrics;
    
    // Compute individual errors
    double target_angle = normalize_angle(target.angle);
    double best_angle = normalize_angle(best.angle);
    
    metrics.angle_error = angle_difference(target_angle, best_angle);
    metrics.radius_error = fabs(target.radius - best.radius);
    metrics.combined_error = combined_error;
    
    // Compute confidences (1 - normalized_error)
    metrics.angle_confidence = 1.0 - (metrics.angle_error / M_PI);
    metrics.radius_confidence = 1.0 - metrics.radius_error;
    metrics.overall_confidence = 1.0 - combined_error;
    
    // Clamp to [0, 1]
    if (metrics.angle_confidence < 0) metrics.angle_confidence = 0;
    if (metrics.radius_confidence < 0) metrics.radius_confidence = 0;
    if (metrics.overall_confidence < 0) metrics.overall_confidence = 0;
    
    return metrics;
}

uint64_t radius_aware_multi_layer_search(
    ClockPosition target,
    uint64_t center_k,
    SearchRecoveryV2Context* ctx,
    RadiusConfidenceMetrics* metrics
) {
    // Compute adaptive weights based on target radius
    ErrorWeights weights = compute_adaptive_weights(target.radius);
    
    uint64_t best_k = center_k;
    double best_error = 1e9;
    ClockPosition best_pos = map_k_to_clock_u64(best_k);
    
    // Layer 1: Wide search (±100, step 10)
    int64_t range = 100;
    int64_t step = 10;
    
    for (int layer = 0; layer < 3; layer++) {
        uint64_t layer_best_k = best_k;
        double layer_best_error = best_error;
        
        for (int64_t offset = -range; offset <= range; offset += step) {
            int64_t test_k = (int64_t)best_k + offset;
            
            // Bounds check
            if (test_k < (int64_t)ctx->min_k || test_k > (int64_t)ctx->max_k) {
                continue;
            }
            
            uint64_t k = (uint64_t)test_k;
            ClockPosition pos = map_k_to_clock_u64(k);
            
            // Compute combined error
            double error = compute_combined_error(target, pos, weights);
            
            if (error < layer_best_error) {
                layer_best_error = error;
                layer_best_k = k;
                best_pos = pos;
            }
        }
        
        best_k = layer_best_k;
        best_error = layer_best_error;
        
        // Refine for next layer
        if (layer == 0) {
            range = 25;
            step = 2;
        } else if (layer == 1) {
            range = 10;
            step = 1;
        }
        
        // Early stopping if confidence is very high
        double confidence = 1.0 - best_error;
        if (confidence > 0.95) {
            break;
        }
    }
    
    // Compute final confidence metrics
    if (metrics) {
        *metrics = compute_radius_confidence(target, best_pos, best_error);
    }
    
    return best_k;
}

uint64_t radius_aware_search_k(
    ClockPosition target,
    SearchRecoveryV2Context* ctx,
    RadiusConfidenceMetrics* metrics
) {
    // Find 3 nearest anchors (reuse v2 logic)
    AnchorInfo anchors[3];
    int num_anchors = find_3_nearest_anchors(target, ctx->overlay, anchors);
    
    if (num_anchors == 0) {
        // Fallback: search from middle of range
        uint64_t center_k = (ctx->min_k + ctx->max_k) / 2;
        return radius_aware_multi_layer_search(target, center_k, ctx, metrics);
    }
    
    // Try search from each anchor, keep best result
    uint64_t best_k = 0;
    double best_error = 1e9;
    RadiusConfidenceMetrics best_metrics;
    
    for (int i = 0; i < num_anchors; i++) {
        uint64_t anchor_k = anchors[i].k_estimate;
        RadiusConfidenceMetrics anchor_metrics;
        
        uint64_t candidate_k = radius_aware_multi_layer_search(
            target, anchor_k, ctx, &anchor_metrics
        );
        
        if (anchor_metrics.combined_error < best_error) {
            best_error = anchor_metrics.combined_error;
            best_k = candidate_k;
            best_metrics = anchor_metrics;
        }
    }
    
    if (metrics) {
        *metrics = best_metrics;
    }
    
    return best_k;
}