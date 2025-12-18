/**
 * search_recovery_v2.c - Enhanced Search-Based K Recovery Implementation
 */

#include "search_recovery_v2.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define PHI ((1.0 + sqrt(5.0)) / 2.0)
#define PI M_PI

SearchRecoveryV2Context* init_search_recovery_v2(
    PlatonicOverlay* overlay,
    uint64_t min_k,
    uint64_t max_k
) {
    SearchRecoveryV2Context* ctx = calloc(1, sizeof(SearchRecoveryV2Context));
    if (!ctx) return NULL;
    
    ctx->overlay = overlay;
    ctx->min_k = min_k;
    ctx->max_k = max_k;
    
    // SIMPLIFIED: Use same configuration for all quadrants
    // Based on v1 baseline but with wider initial range
    // 3 layers: ±100 (coarse), ±25 (medium), ±10 (fine)
    
    SearchLayer universal_config[3] = {
        {.range = 100, .step = 10, .max_iter = 50},  // Coarse: ±100, step 10
        {.range = 25, .step = 2, .max_iter = 50},    // Medium: ±25, step 2
        {.range = 10, .step = 1, .max_iter = 50}     // Fine: ±10, step 1
    };
    
    // Copy to all quadrant configs
    for (int i = 0; i < 3; i++) {
        ctx->q1_layers[i] = universal_config[i];
        ctx->q2_layers[i] = universal_config[i];
        ctx->q3_layers[i] = universal_config[i];
        ctx->q4_layers[i] = universal_config[i];
    }
    
    // Q1 gets one extra ultra-fine layer since it's the optimal zone
    ctx->q1_layers[3] = (SearchLayer){.range = 3, .step = 1, .max_iter = 20};
    
    // Confidence thresholds
    ctx->high_confidence_threshold = 0.8;
    ctx->low_confidence_threshold = 0.5;
    
    return ctx;
}

void free_search_recovery_v2(SearchRecoveryV2Context* ctx) {
    if (ctx) {
        free(ctx);
    }
}

Quadrant get_quadrant(double angle) {
    // Normalize to [0, 2π)
    double normalized = fmod(angle, 2.0 * PI);
    if (normalized < 0) normalized += 2.0 * PI;
    
    // Convert to degrees for clarity
    double degrees = normalized * 180.0 / PI;
    
    if (degrees < 90.0) return QUADRANT_Q1;
    if (degrees < 180.0) return QUADRANT_Q2;
    if (degrees < 270.0) return QUADRANT_Q3;
    return QUADRANT_Q4;
}

int find_3_nearest_anchors(
    ClockPosition target,
    PlatonicOverlay* overlay,
    AnchorInfo anchors[3]
) {
    if (!overlay) return 0;
    
    // Use Icosahedron vertices as anchors
    PlatonicSolid* icosa = &overlay->solids[SOLID_ICOSAHEDRON];
    
    // Find all distances
    typedef struct {
        double distance;
        int index;
    } DistanceIndex;
    
    DistanceIndex* distances = malloc(icosa->num_vertices * sizeof(DistanceIndex));
    if (!distances) return 0;
    
    for (uint32_t i = 0; i < icosa->num_vertices; i++) {
        double dx = target.angle - icosa->clock_positions[i].angle;
        double dy = target.radius - icosa->clock_positions[i].radius;
        distances[i].distance = sqrt(dx*dx + dy*dy);
        distances[i].index = i;
    }
    
    // Sort to find 3 nearest (simple bubble sort for small n)
    for (uint32_t i = 0; i < 3 && i < icosa->num_vertices; i++) {
        for (uint32_t j = i + 1; j < icosa->num_vertices; j++) {
            if (distances[j].distance < distances[i].distance) {
                DistanceIndex temp = distances[i];
                distances[i] = distances[j];
                distances[j] = temp;
            }
        }
    }
    
    // Fill anchor info
    int num_found = (icosa->num_vertices < 3) ? icosa->num_vertices : 3;
    for (int i = 0; i < num_found; i++) {
        int idx = distances[i].index;
        anchors[i].position = icosa->clock_positions[idx];
        anchors[i].distance = distances[i].distance;
        
        // FIXED: Use anchor index to estimate k range
        // Icosahedron has 12 vertices evenly distributed
        // Map them to k range [0, max_k]
        // This gives us anchors at k ≈ 0, 25, 50, 75, 100, 125, 150, 175, 200, 225, 250, 275
        anchors[i].k_estimate = (uint64_t)((double)idx * 300.0 / 12.0);
    }
    
    free(distances);
    return num_found;
}

uint64_t compute_weighted_k_estimate(
    AnchorInfo anchors[3],
    int num_anchors
) {
    if (num_anchors == 0) return 0;
    
    // Compute weights (inverse distance)
    double total_weight = 0.0;
    double weights[3];
    
    for (int i = 0; i < num_anchors; i++) {
        // Avoid division by zero
        if (anchors[i].distance < 0.001) {
            return anchors[i].k_estimate;
        }
        weights[i] = 1.0 / anchors[i].distance;
        total_weight += weights[i];
    }
    
    // Normalize weights
    for (int i = 0; i < num_anchors; i++) {
        weights[i] /= total_weight;
    }
    
    // Weighted average
    double weighted_sum = 0.0;
    for (int i = 0; i < num_anchors; i++) {
        weighted_sum += weights[i] * anchors[i].k_estimate;
    }
    
    return (uint64_t)weighted_sum;
}

ConfidenceMetrics compute_confidence(
    ClockPosition target,
    AnchorInfo anchors[3],
    int num_anchors,
    double angle_error
) {
    ConfidenceMetrics metrics = {0};
    
    if (num_anchors == 0) {
        return metrics;
    }
    
    // Distance to nearest anchor
    metrics.distance_to_anchor = anchors[0].distance;
    
    // Angle error (normalized to [0, 1])
    metrics.angle_error = angle_error / PI;  // Max error is π
    
    // Anchor consistency (variance in k estimates)
    if (num_anchors >= 2) {
        double mean_k = 0.0;
        for (int i = 0; i < num_anchors; i++) {
            mean_k += anchors[i].k_estimate;
        }
        mean_k /= num_anchors;
        
        double variance = 0.0;
        for (int i = 0; i < num_anchors; i++) {
            double diff = anchors[i].k_estimate - mean_k;
            variance += diff * diff;
        }
        variance /= num_anchors;
        
        // Normalize consistency (lower variance = higher consistency)
        metrics.anchor_consistency = 1.0 / (1.0 + sqrt(variance) / 100.0);
    } else {
        metrics.anchor_consistency = 0.5;
    }
    
    // Overall confidence (weighted combination)
    double w1 = 0.4;  // Distance weight
    double w2 = 0.4;  // Angle error weight
    double w3 = 0.2;  // Consistency weight
    
    double distance_score = 1.0 / (1.0 + metrics.distance_to_anchor);
    double error_score = 1.0 - metrics.angle_error;
    
    metrics.overall = w1 * distance_score + 
                      w2 * error_score + 
                      w3 * metrics.anchor_consistency;
    
    return metrics;
}

uint64_t recursive_search_k(
    ClockPosition target,
    uint64_t center_k,
    SearchLayer* layers,
    int num_layers,
    double* final_error
) {
    uint64_t best_k = center_k;
    double best_error = 1e9;
    
    // Normalize target angle
    double target_angle = fmod(target.angle, 2.0 * PI);
    if (target_angle < 0) target_angle += 2.0 * PI;
    
    // Iterate through layers
    for (int layer = 0; layer < num_layers; layer++) {
        SearchLayer* config = &layers[layer];
        
        // Search range for this layer
        int64_t min_offset = -config->range;
        int64_t max_offset = config->range;
        
        // Search with current step size
        for (int64_t offset = min_offset; offset <= max_offset; offset += config->step) {
            int64_t candidate_k = (int64_t)best_k + offset;
            if (candidate_k < 0) continue;
            
            uint64_t k = (uint64_t)candidate_k;
            
            // Forward mapping: θ = k·π(1+√5)
            double computed_angle = fmod((double)k * PI * PHI, 2.0 * PI);
            if (computed_angle < 0) computed_angle += 2.0 * PI;
            
            // Compute error (handle wraparound)
            double error = fabs(computed_angle - target_angle);
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

uint64_t enhanced_search_k(
    ClockPosition target,
    SearchRecoveryV2Context* ctx,
    ConfidenceMetrics* metrics
) {
    if (!ctx) {
        if (metrics) memset(metrics, 0, sizeof(ConfidenceMetrics));
        return 0;
    }
    
    // Find 3 nearest anchors
    AnchorInfo anchors[3];
    int num_anchors = find_3_nearest_anchors(target, ctx->overlay, anchors);
    
    // Compute weighted k estimate
    uint64_t center_k = compute_weighted_k_estimate(anchors, num_anchors);
    
    // Determine quadrant
    Quadrant quad = get_quadrant(target.angle);
    
    // Select appropriate layers
    SearchLayer* layers;
    int num_layers;
    
    switch (quad) {
        case QUADRANT_Q1:
            layers = ctx->q1_layers;
            num_layers = 4;
            break;
        case QUADRANT_Q2:
            layers = ctx->q2_layers;
            num_layers = 3;
            break;
        case QUADRANT_Q3:
            layers = ctx->q3_layers;
            num_layers = 3;
            break;
        case QUADRANT_Q4:
            layers = ctx->q4_layers;
            num_layers = 3;
            break;
        default:
            layers = ctx->q2_layers;
            num_layers = 3;
    }
    
    // Perform recursive search
    double error = 0.0;
    uint64_t recovered_k = recursive_search_k(target, center_k, layers, num_layers, &error);
    
    // Compute confidence
    if (metrics) {
        *metrics = compute_confidence(target, anchors, num_anchors, error);
    }
    
    // Update statistics
    ctx->total_searches++;
    if (error < 0.01) {  // Success threshold
        ctx->successful_searches++;
    }
    
    return recovered_k;
}

uint64_t q1_optimized_search_k(
    ClockPosition target,
    SearchRecoveryV2Context* ctx,
    ConfidenceMetrics* metrics
) {
    // Force Q1 configuration regardless of actual quadrant
    // This is for testing Q1 optimization
    
    if (!ctx) {
        if (metrics) memset(metrics, 0, sizeof(ConfidenceMetrics));
        return 0;
    }
    
    // Find 3 nearest anchors
    AnchorInfo anchors[3];
    int num_anchors = find_3_nearest_anchors(target, ctx->overlay, anchors);
    
    // Compute weighted k estimate
    uint64_t center_k = compute_weighted_k_estimate(anchors, num_anchors);
    
    // Use Q1 layers (4 layers for maximum precision)
    double error = 0.0;
    uint64_t recovered_k = recursive_search_k(target, center_k, ctx->q1_layers, 4, &error);
    
    // Compute confidence
    if (metrics) {
        *metrics = compute_confidence(target, anchors, num_anchors, error);
    }
    
    return recovered_k;
}