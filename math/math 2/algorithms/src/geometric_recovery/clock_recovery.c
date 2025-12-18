/**
 * @file clock_recovery.c
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

#include "geometric_recovery/clock_recovery.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Mathematical constants
#define PI 3.141592653589793
#define PHI 1.618033988749895  // Golden ratio: (1+√5)/2
#define PI_PHI (PI * PHI)      // π×φ metric

// clock lattice structure
#define RING_0_POSITIONS 12    // Hours
#define RING_1_POSITIONS 60    // Minutes
#define RING_2_POSITIONS 60    // Seconds
#define RING_3_POSITIONS 100   // Milliseconds

// Ring radii
#define RING_0_RADIUS 0.25
#define RING_1_RADIUS 0.50
#define RING_2_RADIUS 0.75
#define RING_3_RADIUS 1.00

/**
 * Anchor point structure
 */
typedef struct {
    uint64_t value;         // Known value
    ClockPosition position; // Clock position for this value
    double weight;          // Weight for interpolation
} AnchorPoint;

/**
 * Clock recovery context structure
 */
struct ClockRecoveryContext {
    uint64_t n;                 // System size
    AnchorPoint* anchors;       // Anchor points
    int num_anchors;            // Number of anchors
    int max_anchors;            // Maximum anchors
    double confidence;          // Recovery confidence
};

// ============================================================================
// FORWARD MAPPING: value → ClockPosition
// ============================================================================

/**
 * Compute angular position using π×φ metric
 * θ = k·π·φ
 */
static double compute_pi_phi_angle(uint64_t value) {
    // θ = value·π·φ
    double angle = (double)value * PI_PHI;
    
    // Normalize to [0, 2π)
    double two_pi = 2.0 * PI;
    while (angle >= two_pi) {
        angle -= two_pi;
    }
    while (angle < 0) {
        angle += two_pi;
    }
    
    return angle;
}

/**
 * Map angle to base-60 ring structure
 */
static ClockPosition map_angle_to_ring(double angle) {
    ClockPosition pos;
    
    // Normalize angle to [0, 2π)
    double two_pi = 2.0 * PI;
    while (angle >= two_pi) angle -= two_pi;
    while (angle < 0) angle += two_pi;
    
    // Determine ring based on angle magnitude
    double normalized = angle / two_pi;  // [0, 1)
    
    // Map to ring structure using modular relationships
    if (normalized < 0.25) {
        // Ring 0: Hours (12 positions)
        pos.ring = 0;
        pos.position = (int)(normalized * 12.0 * 4.0) % 12 + 1;
        pos.angle = angle;
        pos.radius = RING_0_RADIUS;
    } else if (normalized < 0.50) {
        // Ring 1: Minutes (60 positions)
        pos.ring = 1;
        pos.position = (int)((normalized - 0.25) * 60.0 * 4.0) % 60 + 1;
        pos.angle = angle;
        pos.radius = RING_1_RADIUS;
    } else if (normalized < 0.75) {
        // Ring 2: Seconds (60 positions)
        pos.ring = 2;
        pos.position = (int)((normalized - 0.50) * 60.0 * 4.0) % 60 + 1;
        pos.angle = angle;
        pos.radius = RING_2_RADIUS;
    } else {
        // Ring 3: Milliseconds (100 positions)
        pos.ring = 3;
        pos.position = (int)((normalized - 0.75) * 100.0 * 4.0) % 100 + 1;
        pos.angle = angle;
        pos.radius = RING_3_RADIUS;
    }
    
    return pos;
}

/**
 * Map value to clock position
 */
ClockPosition value_to_clock_position(uint64_t value, uint64_t n) {
    (void)n;  // Unused in this implementation
    
    // Compute θ = value·π·φ
    double angle = compute_pi_phi_angle(value);
    
    // Map to ring structure
    return map_angle_to_ring(angle);
}

// ============================================================================
// INVERSE MAPPING: ClockPosition → value
// ============================================================================

/**
 * Compute base inverse from angle
 * value ≈ θ / (π·φ)
 */
static uint64_t base_inverse_from_angle(double angle) {
    // value ≈ θ / (π·φ)
    double value_estimate = angle / PI_PHI;
    
    // Round to nearest integer
    return (uint64_t)(value_estimate + 0.5);
}

/**
 * Apply ring correction
 * Adjust based on which ring the position is on
 */
static uint64_t apply_ring_correction(uint64_t base_value, ClockPosition pos, uint64_t n) {
    // Ring-specific corrections based on base-60 structure
    uint64_t correction = 0;
    
    switch (pos.ring) {
        case 0:  // Hours (12 positions)
            correction = (pos.position - 1) * (n / 12);
            break;
        case 1:  // Minutes (60 positions)
            correction = (pos.position - 1) * (n / 60);
            break;
        case 2:  // Seconds (60 positions)
            correction = (pos.position - 1) * (n / 60);
            break;
        case 3:  // Milliseconds (100 positions)
            correction = (pos.position - 1) * (n / 100);
            break;
    }
    
    return (base_value + correction) % n;
}

/**
 * Find nearest anchors to target position
 */
static void find_nearest_anchors(
    ClockRecoveryContext* ctx,
    ClockPosition target,
    AnchorPoint** nearest,
    int* num_nearest,
    int max_nearest
) {
    if (ctx->num_anchors == 0) {
        *num_nearest = 0;
        return;
    }
    
    // Allocate temporary array for distances
    typedef struct {
        int index;
        double distance;
    } AnchorDistance;
    
    AnchorDistance* distances = malloc(ctx->num_anchors * sizeof(AnchorDistance));
    if (!distances) {
        *num_nearest = 0;
        return;
    }
    
    // Compute distances to all anchors
    for (int i = 0; i < ctx->num_anchors; i++) {
        distances[i].index = i;
        distances[i].distance = clock_position_distance(target, ctx->anchors[i].position);
    }
    
    // Sort by distance (simple bubble sort for small arrays)
    for (int i = 0; i < ctx->num_anchors - 1; i++) {
        for (int j = 0; j < ctx->num_anchors - i - 1; j++) {
            if (distances[j].distance > distances[j + 1].distance) {
                AnchorDistance temp = distances[j];
                distances[j] = distances[j + 1];
                distances[j + 1] = temp;
            }
        }
    }
    
    // Select nearest anchors
    int count = (ctx->num_anchors < max_nearest) ? ctx->num_anchors : max_nearest;
    for (int i = 0; i < count; i++) {
        nearest[i] = &ctx->anchors[distances[i].index];
    }
    *num_nearest = count;
    
    free(distances);
}

/**
 * Compute weighted average from anchors
 */
static uint64_t weighted_anchor_average(
    AnchorPoint** anchors,
    int num_anchors,
    ClockPosition target
) {
    if (num_anchors == 0) {
        return 0;
    }
    
    // Compute weights based on inverse distance
    double total_weight = 0.0;
    double weighted_sum = 0.0;
    
    for (int i = 0; i < num_anchors; i++) {
        double dist = clock_position_distance(target, anchors[i]->position);
        
        // Avoid division by zero
        if (dist < 1e-10) {
            // Target is very close to this anchor
            return anchors[i]->value;
        }
        
        double weight = 1.0 / (dist * dist);  // Inverse square distance
        total_weight += weight;
        weighted_sum += weight * (double)anchors[i]->value;
    }
    
    if (total_weight < 1e-10) {
        return anchors[0]->value;
    }
    
    return (uint64_t)(weighted_sum / total_weight + 0.5);
}

// ============================================================================
// PUBLIC API
// ============================================================================

/**
 * Create clock recovery context
 */
ClockRecoveryContext* clock_recovery_create(uint64_t n, int num_anchors) {
    ClockRecoveryContext* ctx = malloc(sizeof(ClockRecoveryContext));
    if (!ctx) {
        return NULL;
    }
    
    ctx->n = n;
    ctx->num_anchors = 0;
    ctx->max_anchors = num_anchors;
    ctx->confidence = 0.0;
    
    ctx->anchors = malloc(num_anchors * sizeof(AnchorPoint));
    if (!ctx->anchors) {
        free(ctx);
        return NULL;
    }
    
    return ctx;
}

/**
 * Add anchor point
 */
bool clock_recovery_add_anchor(
    ClockRecoveryContext* ctx,
    uint64_t value,
    ClockPosition position
) {
    if (!ctx || ctx->num_anchors >= ctx->max_anchors) {
        return false;
    }
    
    ctx->anchors[ctx->num_anchors].value = value;
    ctx->anchors[ctx->num_anchors].position = position;
    ctx->anchors[ctx->num_anchors].weight = 1.0;
    ctx->num_anchors++;
    
    return true;
}

/**
 * Recover value from clock position
 */
bool clock_inverse_map(
    ClockRecoveryContext* ctx,
    ClockPosition position,
    uint64_t* value_out
) {
    if (!ctx || !value_out) {
        return false;
    }
    
    // Step 1: Base inverse from angle
    uint64_t base_value = base_inverse_from_angle(position.angle);
    
    // Step 2: Apply ring correction
    uint64_t corrected_value = apply_ring_correction(base_value, position, ctx->n);
    
    // Step 3: Anchor refinement (if anchors available)
    if (ctx->num_anchors > 0) {
        // Find nearest anchors
        AnchorPoint* nearest[8];  // Use up to 8 nearest anchors
        int num_nearest = 0;
        find_nearest_anchors(ctx, position, nearest, &num_nearest, 8);
        
        if (num_nearest > 0) {
            // Compute anchor-based estimate
            uint64_t anchor_value = weighted_anchor_average(nearest, num_nearest, position);
            
            // Blend computed and anchor-based estimates
            // Weight more towards anchors if we have many nearby
            double anchor_weight = (double)num_nearest / 8.0;
            double computed_weight = 1.0 - anchor_weight;
            
            *value_out = (uint64_t)(
                computed_weight * (double)corrected_value +
                anchor_weight * (double)anchor_value
            );
            
            // Update confidence based on anchor proximity
            double min_distance = clock_position_distance(position, nearest[0]->position);
            ctx->confidence = 1.0 / (1.0 + min_distance);
        } else {
            *value_out = corrected_value;
            ctx->confidence = 0.5;  // Medium confidence without anchors
        }
    } else {
        *value_out = corrected_value;
        ctx->confidence = 0.3;  // Low confidence without anchors
    }
    
    return true;
}

/**
 * Compute distance between two clock positions
 */
double clock_position_distance(ClockPosition pos1, ClockPosition pos2) {
    // Distance in π×φ metric is based on angular separation
    double angle_diff = math_abs(pos1.angle - pos2.angle);
    
    // Normalize to [0, π]
    if (angle_diff > PI) {
        angle_diff = 2.0 * PI - angle_diff;
    }
    
    // Include radius difference
    double radius_diff = math_abs(pos1.radius - pos2.radius);
    
    // Combined distance
    return math_sqrt(angle_diff * angle_diff + radius_diff * radius_diff);
}

/**
 * Get confidence score
 */
double clock_recovery_get_confidence(const ClockRecoveryContext* ctx) {
    if (!ctx) {
        return 0.0;
    }
    return ctx->confidence;
}

/**
 * Destroy clock recovery context
 */
void clock_recovery_destroy(ClockRecoveryContext* ctx) {
    if (ctx) {
        if (ctx->anchors) {
            free(ctx->anchors);
        }
        free(ctx);
    }
}