/**
 * search_recovery_v2.h - Enhanced Search-Based K Recovery
 * 
 * Improvements over v1:
 * 1. Quadrant-aware search ranges
 * 2. 3-anchor averaging for better center estimate
 * 3. 3-layer recursive search (coarse, medium, fine)
 * 4. Confidence-based adaptive search
 * 5. Q1 optimization for best quadrant
 */

#ifndef SEARCH_RECOVERY_V2_H
#define SEARCH_RECOVERY_V2_H

#include "clock_recovery.h"
#include "platonic_solids.h"
#include <stdint.h>
#include <stdbool.h>

// Quadrant definitions
typedef enum {
    QUADRANT_Q1 = 0,  // TOP-RIGHT (0-90°): Small k, optimal zone
    QUADRANT_Q2 = 1,  // TOP-LEFT (90-180°): Medium k
    QUADRANT_Q3 = 2,  // BOTTOM-LEFT (180-270°): Large k
    QUADRANT_Q4 = 3   // BOTTOM-RIGHT (270-360°): Wrap-around k
} Quadrant;

// Search layer configuration
typedef struct {
    int64_t range;      // Search range (±range)
    uint64_t step;      // Step size
    uint32_t max_iter;  // Max iterations
} SearchLayer;

// Confidence metrics
typedef struct {
    double distance_to_anchor;  // Distance to nearest anchor
    double angle_error;         // Angle error at current layer
    double anchor_consistency;  // Consistency across 3 anchors
    double overall;             // Overall confidence [0, 1]
} ConfidenceMetrics;

// Enhanced search context
typedef struct {
    PlatonicOverlay* overlay;
    
    // Search bounds
    uint64_t min_k;
    uint64_t max_k;
    
    // Quadrant-specific configurations
    SearchLayer q1_layers[4];  // Q1: 4 layers for optimal zone
    SearchLayer q2_layers[3];  // Q2: 3 layers
    SearchLayer q3_layers[3];  // Q3: 3 layers
    SearchLayer q4_layers[3];  // Q4: 3 layers
    
    // Adaptive parameters
    double high_confidence_threshold;  // 0.8
    double low_confidence_threshold;   // 0.5
    
    // Statistics
    uint64_t total_searches;
    uint64_t successful_searches;
    uint64_t total_iterations;
} SearchRecoveryV2Context;

// Anchor information
typedef struct {
    ClockPosition position;
    double distance;
    uint64_t k_estimate;
} AnchorInfo;

/**
 * Initialize enhanced search recovery context
 */
SearchRecoveryV2Context* init_search_recovery_v2(
    PlatonicOverlay* overlay,
    uint64_t min_k,
    uint64_t max_k
);

/**
 * Free search recovery context
 */
void free_search_recovery_v2(SearchRecoveryV2Context* ctx);

/**
 * Determine quadrant from angle
 */
Quadrant get_quadrant(double angle);

/**
 * Find 3 nearest anchors to target position
 */
int find_3_nearest_anchors(
    ClockPosition target,
    PlatonicOverlay* overlay,
    AnchorInfo anchors[3]
);

/**
 * Compute weighted average k estimate from 3 anchors
 */
uint64_t compute_weighted_k_estimate(
    AnchorInfo anchors[3],
    int num_anchors
);

/**
 * Compute confidence metrics
 */
ConfidenceMetrics compute_confidence(
    ClockPosition target,
    AnchorInfo anchors[3],
    int num_anchors,
    double angle_error
);

/**
 * Multi-layer recursive search
 */
uint64_t recursive_search_k(
    ClockPosition target,
    uint64_t center_k,
    SearchLayer* layers,
    int num_layers,
    double* final_error
);

/**
 * Enhanced search with all improvements
 */
uint64_t enhanced_search_k(
    ClockPosition target,
    SearchRecoveryV2Context* ctx,
    ConfidenceMetrics* metrics
);

/**
 * Q1-optimized search (for best quadrant)
 */
uint64_t q1_optimized_search_k(
    ClockPosition target,
    SearchRecoveryV2Context* ctx,
    ConfidenceMetrics* metrics
);

#endif // SEARCH_RECOVERY_V2_H