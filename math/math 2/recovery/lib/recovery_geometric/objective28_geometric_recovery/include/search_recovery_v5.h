#ifndef SEARCH_RECOVERY_V5_H
#define SEARCH_RECOVERY_V5_H

#include "search_recovery_v2.h"

/**
 * V5: Radius-Aware Search
 * 
 * Key improvement: Uses BOTH angle and radius to disambiguate k values
 * 
 * Breakthrough discovery: Radius is a primary discriminating factor
 * - Success avg radius: 0.7750
 * - Failure avg radius: 0.2750
 * - Difference: 0.5000 (HUGE!)
 */

// Error weights for combined metric
typedef struct {
    double angle_weight;   // Weight for angle error [0, 1]
    double radius_weight;  // Weight for radius error [0, 1]
} ErrorWeights;

// Enhanced confidence metrics with radius
typedef struct {
    double angle_error;
    double radius_error;
    double combined_error;
    double angle_confidence;
    double radius_confidence;
    double overall_confidence;
} RadiusConfidenceMetrics;

/**
 * Compute combined error using both angle and radius
 */
double compute_combined_error(
    ClockPosition target,
    ClockPosition candidate,
    ErrorWeights weights
);

/**
 * Compute adaptive weights based on target radius
 * 
 * High radius (≥0.75): Trust radius more (w_radius = 0.7)
 * Mid radius (0.5-0.75): Balanced (w_radius = 0.5)
 * Low radius (<0.5): Trust angle more (w_radius = 0.3)
 */
ErrorWeights compute_adaptive_weights(double target_radius);

/**
 * Enhanced search with radius awareness
 * 
 * Uses both angle and radius to find k
 * Expected improvement: 20% → 35-55% success rate
 */
uint64_t radius_aware_search_k(
    ClockPosition target,
    SearchRecoveryV2Context* ctx,
    RadiusConfidenceMetrics* metrics
);

/**
 * Multi-layer search with radius awareness
 * 
 * Layer 1: ±100 range, step 10
 * Layer 2: ±25 range, step 2
 * Layer 3: ±10 range, step 1
 * 
 * Each layer uses combined angle+radius error
 */
uint64_t radius_aware_multi_layer_search(
    ClockPosition target,
    uint64_t center_k,
    SearchRecoveryV2Context* ctx,
    RadiusConfidenceMetrics* metrics
);

/**
 * Compute radius-aware confidence metrics
 */
RadiusConfidenceMetrics compute_radius_confidence(
    ClockPosition target,
    ClockPosition best,
    double combined_error
);

#endif // SEARCH_RECOVERY_V5_H
