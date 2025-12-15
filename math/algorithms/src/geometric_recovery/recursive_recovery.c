/**
 * @file recursive_recovery.c
 * @brief Recursive Recovery with Hierarchical Refinement
 * 
 * UNIVERSAL GEOMETRIC MATHEMATICS - Works with ANY system!
 * 
 * Implements hierarchical refinement of recovery bounds using:
 * - Recursive subdivision
 * - Adaptive depth control
 * - Confidence scoring
 * - Early termination
 * 
 * NO crypto-specific dependencies - works with raw uint64_t data.
 * 
 * Thesis Reference: Chapter 16 - Geometric Recovery
 */

#include "geometric_recovery/recursive_recovery.h"
#include "geometric_recovery/tetration_attractors.h"
#include "math/clock_lattice_13d.h"
#include "math/transcendental.h"
#include "math/types.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

// ============================================================================
// STRUCTURES
// ============================================================================

/**
 * Recursive recovery context
 */
struct RecursiveRecoveryContext {
    uint64_t n;                     // System size
    int max_depth;                  // Maximum recursion depth
    double confidence_threshold;    // Minimum confidence to stop
    
    TetrationSystem* attractors;    // Tetration attractors for guidance
    
    // Statistics
    int total_subdivisions;
    int early_terminations;
    double avg_confidence;
};

// ============================================================================
// CONTEXT MANAGEMENT
// ============================================================================

RecursiveRecoveryContext* recursive_recovery_create(
    uint64_t n,
    int max_depth,
    double confidence_threshold,
    TetrationSystem* attractors
) {
    if (n == 0 || max_depth <= 0) {
        return NULL;
    }
    
    RecursiveRecoveryContext* ctx = (RecursiveRecoveryContext*)calloc(
        1, sizeof(RecursiveRecoveryContext)
    );
    if (!ctx) {
        return NULL;
    }
    
    ctx->n = n;
    ctx->max_depth = max_depth;
    ctx->confidence_threshold = confidence_threshold;
    
    // Use provided attractors or create new ones
    if (attractors) {
        ctx->attractors = attractors;
    } else {
        // Create tetration attractors for guidance
        ctx->attractors = tetration_system_create(6, 31);  // 6 bases, 31 depths
        if (ctx->attractors) {
            tetration_system_compute(ctx->attractors);
        }
    }
    
    ctx->total_subdivisions = 0;
    ctx->early_terminations = 0;
    ctx->avg_confidence = 0.0;
    
    return ctx;
}

void recursive_recovery_free(RecursiveRecoveryContext* ctx) {
    if (!ctx) {
        return;
    }
    
    if (ctx->attractors) {
        tetration_system_free(ctx->attractors);
    }
    
    free(ctx);
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * Compute confidence score for a range
 * 
 * Based on:
 * - Range size (smaller = higher confidence)
 * - Attractor proximity
 * - Depth (deeper = higher confidence)
 */
static double compute_confidence(
    RecursiveRecoveryContext* ctx,
    uint64_t min,
    uint64_t max,
    int depth
) {
    // Base confidence from range size
    uint64_t range = max - min;
    double size_confidence = 1.0 - ((double)range / (double)ctx->n);
    
    // Depth bonus (deeper = more refined)
    double depth_bonus = (double)depth / (double)ctx->max_depth;
    
    // Combine
    double confidence = (size_confidence * 0.7) + (depth_bonus * 0.3);
    
    // Clamp to [0, 1]
    if (confidence < 0.0) confidence = 0.0;
    if (confidence > 1.0) confidence = 1.0;
    
    return confidence;
}

/**
 * Check if we should subdivide further
 */
static bool should_subdivide(
    RecursiveRecoveryContext* ctx,
    RecoveryBounds bounds
) {
    // Stop if max depth reached
    if (bounds.depth >= ctx->max_depth) {
        return false;
    }
    
    // Stop if confidence is high enough
    if (bounds.confidence >= ctx->confidence_threshold) {
        ctx->early_terminations++;
        return false;
    }
    
    // Stop if range is too small (< 100)
    if (bounds.max - bounds.min < 100) {
        return false;
    }
    
    return true;
}

/**
 * Subdivide range into smaller ranges
 */
static void subdivide_range(
    RecursiveRecoveryContext* ctx,
    RecoveryBounds parent,
    RecoveryBounds* left,
    RecoveryBounds* right
) {
    uint64_t mid = parent.min + (parent.max - parent.min) / 2;
    
    // Left half
    left->min = parent.min;
    left->max = mid;
    left->depth = parent.depth + 1;
    left->confidence = compute_confidence(ctx, left->min, left->max, left->depth);
    
    // Right half
    right->min = mid;
    right->max = parent.max;
    right->depth = parent.depth + 1;
    right->confidence = compute_confidence(ctx, right->min, right->max, right->depth);
    
    ctx->total_subdivisions++;
}

// ============================================================================
// RECURSIVE REFINEMENT
// ============================================================================

/**
 * Recursively refine bounds
 */
RecoveryBounds recursive_recovery_refine(
    RecursiveRecoveryContext* ctx,
    uint64_t target,
    RecoveryBounds initial
) {
    // Check if we should stop
    if (!should_subdivide(ctx, initial)) {
        return initial;
    }
    
    // Subdivide into left and right
    RecoveryBounds left, right;
    subdivide_range(ctx, initial, &left, &right);
    
    // Map target to 13D position
    double target_pos[13];
    clock_map_value_to_lattice_13d(target, target_pos);
    
    // Map midpoints to 13D positions
    double left_mid_pos[13], right_mid_pos[13];
    uint64_t left_mid = left.min + (left.max - left.min) / 2;
    uint64_t right_mid = right.min + (right.max - right.min) / 2;
    clock_map_value_to_lattice_13d(left_mid, left_mid_pos);
    clock_map_value_to_lattice_13d(right_mid, right_mid_pos);
    
    // Compute distances
    double left_dist = math_distance_13d(target_pos, left_mid_pos);
    double right_dist = math_distance_13d(target_pos, right_mid_pos);
    
    // Choose closer half
    if (left_dist < right_dist) {
        // Target is closer to left half
        return recursive_recovery_refine(ctx, target, left);
    } else {
        // Target is closer to right half
        return recursive_recovery_refine(ctx, target, right);
    }
}

/**
 * Execute recursive recovery
 */
int recursive_recovery_execute(
    RecursiveRecoveryContext* ctx,
    uint64_t target,
    uint64_t* result_min,
    uint64_t* result_max,
    double* confidence
) {
    if (!ctx || !result_min || !result_max || !confidence) {
        return -1;
    }
    
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║  RECURSIVE RECOVERY                                      ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");
    printf("\n");
    printf("Target: %lu\n", target);
    printf("System size: %lu\n", ctx->n);
    printf("Max depth: %d\n", ctx->max_depth);
    printf("\n");
    
    // Initialize with full range
    RecoveryBounds initial;
    initial.min = 0;
    initial.max = ctx->n;
    initial.depth = 0;
    initial.confidence = 0.0;
    
    // Recursively refine
    RecoveryBounds final = recursive_recovery_refine(ctx, target, initial);
    
    // Return results
    *result_min = final.min;
    *result_max = final.max;
    *confidence = final.confidence;
    
    // Compute reduction factor
    uint64_t range = final.max - final.min;
    double reduction = (double)ctx->n / (double)range;
    
    printf("Results:\n");
    printf("  Bounds: [%lu, %lu]\n", final.min, final.max);
    printf("  Range: %lu\n", range);
    printf("  Reduction: %.2fx\n", reduction);
    printf("  Confidence: %.2f%%\n", final.confidence * 100.0);
    printf("  Depth reached: %d\n", final.depth);
    printf("\n");
    
    printf("Statistics:\n");
    printf("  Total subdivisions: %d\n", ctx->total_subdivisions);
    printf("  Early terminations: %d\n", ctx->early_terminations);
    printf("\n");
    
    return 0;
}

/**
 * Get statistics
 */
void recursive_recovery_get_stats(
    const RecursiveRecoveryContext* ctx,
    int* total_subdivisions,
    int* early_terminations
) {
    if (!ctx) {
        return;
    }
    
    if (total_subdivisions) {
        *total_subdivisions = ctx->total_subdivisions;
    }
    if (early_terminations) {
        *early_terminations = ctx->early_terminations;
    }
}

// ============================================================================
// WRAPPER FUNCTIONS FOR HEADER API
// ============================================================================

/**
 * Wrapper for recursive_refine to match header API
 */
bool recursive_refine(
    RecursiveRecoveryContext* ctx,
    uint64_t target,
    RecoveryBounds initial,
    RecoveryBounds* result_out
) {
    if (!ctx || !result_out) return false;
    
    *result_out = recursive_recovery_refine(ctx, target, initial);
    return true;
}

/**
 * Get reduction factor achieved
 */
double recursive_recovery_get_reduction_factor(
    RecoveryBounds initial,
    RecoveryBounds refined
) {
    uint64_t initial_size = initial.max - initial.min + 1;
    uint64_t refined_size = refined.max - refined.min + 1;
    
    if (refined_size == 0) return 0.0;
    
    return (double)initial_size / (double)refined_size;
}

/**
 * Check if bounds contain a specific value
 */
bool recursive_recovery_bounds_contain(
    RecoveryBounds bounds,
    uint64_t value
) {
    return (value >= bounds.min && value <= bounds.max);
}

/**
 * Get size of bounds
 */
uint64_t recursive_recovery_bounds_size(RecoveryBounds bounds) {
    return bounds.max - bounds.min + 1;
}

/**
 * Print recovery bounds
 */
void recursive_recovery_print_bounds(
    RecoveryBounds bounds,
    const char* label
) {
    printf("%s:\n", label ? label : "Bounds");
    printf("  Range: [%lu, %lu]\n", bounds.min, bounds.max);
    printf("  Size: %lu\n", recursive_recovery_bounds_size(bounds));
    printf("  Depth: %d\n", bounds.depth);
    printf("  Confidence: %.2f%%\n", bounds.confidence * 100.0);
}

/**
 * Destroy recursive recovery context (alias for free)
 */
void recursive_recovery_destroy(RecursiveRecoveryContext* ctx) {
    recursive_recovery_free(ctx);
}