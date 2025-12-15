/**
 * @file recursive_recovery.h
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
 */

#ifndef RECURSIVE_RECOVERY_H
#define RECURSIVE_RECOVERY_H

#include <stdint.h>
#include <stdbool.h>

// Forward declarations
typedef struct RecursiveRecoveryContext RecursiveRecoveryContext;
typedef struct TetrationSystem TetrationSystem;

/**
 * Recovery bounds at a specific recursion depth
 */
typedef struct {
    uint64_t min;           // Minimum value in range
    uint64_t max;           // Maximum value in range
    int depth;              // Current recursion depth
    double confidence;      // Confidence score [0.0, 1.0]
} RecoveryBounds;

/**
 * Create recursive recovery context
 * 
 * @param n System size
 * @param max_depth Maximum recursion depth
 * @param confidence_threshold Minimum confidence to stop recursion
 * @param attractors Tetration attractors for guidance (optional)
 * @return New context or NULL on error
 */
RecursiveRecoveryContext* recursive_recovery_create(
    uint64_t n,
    int max_depth,
    double confidence_threshold,
    TetrationSystem* attractors
);

/**
 * Recursively refine recovery bounds
 * 
 * Subdivides the search space and uses attractors to guide
 * the refinement process.
 * 
 * @param ctx Context
 * @param target Target output value to recover input for
 * @param initial Initial bounds to refine
 * @param result_out Pointer to store refined bounds
 * @return true on success, false on error
 */
bool recursive_refine(
    RecursiveRecoveryContext* ctx,
    uint64_t target,
    RecoveryBounds initial,
    RecoveryBounds* result_out
);

/**
 * Get reduction factor achieved
 * 
 * @param initial Initial bounds
 * @param refined Refined bounds
 * @return Reduction factor (original_size / refined_size)
 */
double recursive_recovery_get_reduction_factor(
    RecoveryBounds initial,
    RecoveryBounds refined
);

/**
 * Check if bounds contain a specific value
 * 
 * @param bounds Bounds to check
 * @param value Value to check
 * @return true if value is in bounds, false otherwise
 */
bool recursive_recovery_bounds_contain(
    RecoveryBounds bounds,
    uint64_t value
);

/**
 * Get size of bounds
 * 
 * @param bounds Bounds
 * @return Size (max - min + 1)
 */
uint64_t recursive_recovery_bounds_size(RecoveryBounds bounds);

/**
 * Print recovery bounds
 * 
 * @param bounds Bounds to print
 * @param label Label for output
 */
void recursive_recovery_print_bounds(
    RecoveryBounds bounds,
    const char* label
);

/**
 * Destroy recursive recovery context
 * 
 * @param ctx Context to destroy
 */
void recursive_recovery_destroy(RecursiveRecoveryContext* ctx);

#endif // RECURSIVE_RECOVERY_H