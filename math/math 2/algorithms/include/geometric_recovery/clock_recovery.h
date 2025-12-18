/**
 * @file clock_recovery.h
 * @brief Clock Inverse Mapping for Recovery
 * 
 * UNIVERSAL GEOMETRIC MATHEMATICS - Works with ANY system!
 * 
 * Implements clock inverse mapping:
 * - ClockPosition → scalar value
 * - Platonic anchor refinement
 * - Weighted interpolation
 * - Works for ANY clock operation
 * 
 * NO crypto-specific dependencies - works with raw uint64_t data.
 */

#ifndef CLOCK_RECOVERY_H
#define CLOCK_RECOVERY_H

#include <stdint.h>
#include <stdbool.h>
#include "math/types.h"  // For ClockPosition

// ClockPosition is defined in math/types.h
// We use the standard definition from the math library

/**
 * Clock recovery context
 */
typedef struct ClockRecoveryContext ClockRecoveryContext;

/**
 * Create clock recovery context
 * 
 * @param n System size
 * @param num_anchors Number of anchor points
 * @return New context or NULL on error
 */
ClockRecoveryContext* clock_recovery_create(
    uint64_t n,
    int num_anchors
);

/**
 * Add anchor point
 * 
 * @param ctx Context
 * @param value Known value
 * @param position Clock position for this value
 * @return true on success, false on error
 */
bool clock_recovery_add_anchor(
    ClockRecoveryContext* ctx,
    uint64_t value,
    ClockPosition position
);

/**
 * Recover value from clock position
 * 
 * Uses inverse mapping with anchor refinement:
 * 1. Base inverse from angle: k = angle / (π × φ)
 * 2. Ring correction: offset based on ring
 * 3. Position correction: within-ring adjustment
 * 4. Anchor refinement: weighted average
 * 5. Combine: k = weighted_blend(computed, anchor_based)
 * 
 * @param ctx Context
 * @param position Clock position
 * @param value_out Pointer to store recovered value
 * @return true on success, false on error
 */
bool clock_inverse_map(
    ClockRecoveryContext* ctx,
    ClockPosition position,
    uint64_t* value_out
);

/**
 * Compute distance between two clock positions
 * 
 * @param pos1 First position
 * @param pos2 Second position
 * @return Distance
 */
double clock_position_distance(
    ClockPosition pos1,
    ClockPosition pos2
);

/**
 * Map value to clock position
 * 
 * @param value Value to map
 * @param n System size
 * @return Clock position
 */
ClockPosition value_to_clock_position(
    uint64_t value,
    uint64_t n
);

/**
 * Get confidence score
 * 
 * @param ctx Context
 * @return Confidence score [0.0, 1.0]
 */
double clock_recovery_get_confidence(const ClockRecoveryContext* ctx);

/**
 * Destroy clock recovery context
 * 
 * @param ctx Context to destroy
 */
void clock_recovery_destroy(ClockRecoveryContext* ctx);

#endif // CLOCK_RECOVERY_H