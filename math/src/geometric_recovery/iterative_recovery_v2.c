/**
 * Iterative Recovery with Dynamic Scaling - Version 2
 * 
 * Implements all four phases:
 * 1. Verification loop (checks if k*G == Q)
 * 2. Endianness and truncation (257 bits, reverse, truncate)
 * 3. Dynamic scaling (scale up until oscillations stabilize)
 * 4. Entropy reduction (from user's Python code)
 */

#include "geometric_recovery.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"
#include <string.h>
#include <stdio.h>
#include <time.h>

// ============================================================================
// PHASE 2: ENDIANNESS AND TRUNCATION
// ============================================================================

/**
 * Triangulate k from anchors with correct endianness and truncation
 * 
 * Key changes:
 * - Use 257 bits (+1 for boundary crossing)
 * - Compute in REVERSE (as user specified)
 * - Truncate to actual order size
 */
BIGNUM* triangulate_k_with_truncation(
    const double* position,
    const double** anchor_positions,
    const BIGNUM** anchor_k_values,
    uint32_t num_anchors,
    uint32_t num_dimensions,
    EC_GROUP* ec_group
) {
    if (num_anchors == 0) return NULL;
    
    // Find 3 nearest anchors
    uint32_t k = (num_anchors >= 3) ? 3 : num_anchors;
    uint32_t nearest[3];
    double distances[3];
    
    // Initialize with worst possible values
    for (uint32_t i = 0; i < k; i++) {
        nearest[i] = 0;
        distances[i] = 1e100;
    }
    
    // Find k nearest
    for (uint32_t a = 0; a < num_anchors; a++) {
        double dist = 0.0;
        for (uint32_t d = 0; d < num_dimensions; d++) {
            double diff = position[d] - anchor_positions[a][d];
            dist += diff * diff;
        }
        dist = math_sqrt(dist);
        
        for (uint32_t i = 0; i < k; i++) {
            if (dist < distances[i]) {
                for (uint32_t j = k - 1; j > i; j--) {
                    nearest[j] = nearest[j - 1];
                    distances[j] = distances[j - 1];
                }
                nearest[i] = a;
                distances[i] = dist;
                break;
            }
        }
    }
    
    // Compute weights (inverse distance squared)
    double weights[3];
    double total_weight = 0.0;
    
    for (uint32_t i = 0; i < k; i++) {
        weights[i] = 1.0 / (distances[i] * distances[i] + 1e-10);
        total_weight += weights[i];
    }
    
    // Normalize weights
    for (uint32_t i = 0; i < k; i++) {
        weights[i] /= total_weight;
    }
    
    // Get order for modular arithmetic
    BIGNUM* order = BN_new();
    BN_CTX* ctx = BN_CTX_new();
    EC_GROUP_get_order(ec_group, order, ctx);
    
    // PHASE 2: Use 257 bits (+1 for boundary crossing)
    unsigned char k_bytes[33];  // 257 bits = 33 bytes
    memset(k_bytes, 0, 33);
    
    // Interpolate k values using weighted sum
    BIGNUM* result = BN_new();
    BN_zero(result);
    
    const uint64_t SCALE = 1000000000ULL;
    
    for (uint32_t i = 0; i < k; i++) {
        BIGNUM* weighted_k = BN_dup(anchor_k_values[nearest[i]]);
        uint64_t scaled_weight = (uint64_t)(weights[i] * SCALE);
        BN_mul_word(weighted_k, scaled_weight);
        BN_add(result, result, weighted_k);
        BN_free(weighted_k);
    }
    
    BN_div_word(result, SCALE);
    
    // Convert to bytes (257 bits)
    int num_bytes = BN_num_bytes(result);
    if (num_bytes > 33) num_bytes = 33;
    BN_bn2bin(result, k_bytes + (33 - num_bytes));
    
    // PHASE 2: Compute in REVERSE (as user specified)
    unsigned char k_bytes_reversed[33];
    for (int i = 0; i < 33; i++) {
        k_bytes_reversed[i] = k_bytes[32 - i];
    }
    
    // Convert back to BIGNUM
    BN_free(result);
    result = BN_new();
    BN_bin2bn(k_bytes_reversed, 33, result);
    
    // PHASE 2: TRUNCATE to order size
    BN_mod(result, result, order, ctx);
    
    BN_free(order);
    BN_CTX_free(ctx);
    
    return result;
}

// ============================================================================
// PHASE 4: ENTROPY REDUCTION
// ============================================================================

/**
 * Reduce entropy iteratively (from user's Python code)
 * 
 * From OscillationAccelerator.reduce_entropy():
 *   current_entropy = initial_space
 *   while current_entropy > threshold:
 *       current_entropy //= 2
 */
uint64_t reduce_entropy(uint64_t initial_space, uint64_t threshold) {
    uint64_t current_entropy = initial_space;
    
    while (current_entropy > threshold) {
        current_entropy /= 2;
    }
    
    return current_entropy;
}

/**
 * Generate deterministic target from Q (from user's Python code)
 */
__attribute__((unused))
static BIGNUM* generate_target_from_Q_v2(
    EC_POINT* Q,
    EC_GROUP* ec_group,
    uint32_t bit_scale
) {
    BN_CTX* ctx = BN_CTX_new();
    
    // Extract Q coordinates
    BIGNUM* Q_x = BN_new();
    BIGNUM* Q_y = BN_new();
    EC_POINT_get_affine_coordinates(ec_group, Q, Q_x, Q_y, ctx);
    
    // Combine Q_x and Q_y to create deterministic target
    BIGNUM* target = BN_new();
    BN_copy(target, Q_x);
    BN_mul_word(target, 31337);  // Prime multiplier
    BN_add(target, target, Q_y);
    
    // Scale to bit_scale
    BIGNUM* scale = BN_new();
    BN_set_word(scale, 1);
    BN_lshift(scale, scale, bit_scale / 2);
    BN_mod(target, target, scale, ctx);
    
    BN_free(Q_x);
    BN_free(Q_y);
    BN_free(scale);
    BN_CTX_free(ctx);
    
    return target;
}

// ============================================================================
// PHASE 1: VERIFICATION LOOP
// ============================================================================

/**
 * Verify if candidate k produces target Q
 * 
 * CRITICAL: This is the missing piece!
 * Check if candidate_k * G == target_Q
 */
bool verify_candidate_produces_Q(
    BIGNUM* candidate_k,
    EC_POINT* target_Q,
    EC_GROUP* ec_group
) {
    if (!candidate_k || !target_Q) return false;
    
    BN_CTX* ctx = BN_CTX_new();
    const EC_POINT* G = EC_GROUP_get0_generator(ec_group);
    
    // Compute candidate_Q = candidate_k * G
    EC_POINT* candidate_Q = EC_POINT_new(ec_group);
    EC_POINT_mul(ec_group, candidate_Q, NULL, G, candidate_k, ctx);
    
    // Compare with target_Q
    int match = EC_POINT_cmp(ec_group, candidate_Q, target_Q, ctx);
    
    EC_POINT_free(candidate_Q);
    BN_CTX_free(ctx);
    
    return (match == 0);
}

/**
 * Measure distance between two points (for oscillation tracking)
 */
double measure_point_distance(
    EC_POINT* Q1,
    EC_POINT* Q2,
    EC_GROUP* ec_group
) {
    BN_CTX* ctx = BN_CTX_new();
    
    BIGNUM* x1 = BN_new();
    BIGNUM* y1 = BN_new();
    BIGNUM* x2 = BN_new();
    BIGNUM* y2 = BN_new();
    
    EC_POINT_get_affine_coordinates(ec_group, Q1, x1, y1, ctx);
    EC_POINT_get_affine_coordinates(ec_group, Q2, x2, y2, ctx);
    
    // Compute Euclidean distance
    BIGNUM* dx = BN_new();
    BIGNUM* dy = BN_new();
    BN_sub(dx, x1, x2);
    BN_sub(dy, y1, y2);
    
    // Convert to double for distance calculation
    char* dx_str = BN_bn2dec(dx);
    char* dy_str = BN_bn2dec(dy);
    double dx_val = atof(dx_str);
    double dy_val = atof(dy_str);
    OPENSSL_free(dx_str);
    OPENSSL_free(dy_str);
    
    double distance = math_sqrt(dx_val * dx_val + dy_val * dy_val);
    
    BN_free(x1);
    BN_free(y1);
    BN_free(x2);
    BN_free(y2);
    BN_free(dx);
    BN_free(dy);
    BN_CTX_free(ctx);
    
    return distance;
}

/**
 * PHASE 1: Iterative recovery with verification loop
 * 
 * This is the CRITICAL missing piece!
 * Iterate until we find k that produces target_Q
 */
BIGNUM* geometric_recovery_iterative(
    GeometricRecoveryContext* ctx,
    EC_POINT* target_Q,
    uint32_t max_iterations,
    double* confidence_out,
    uint32_t* iterations_out
) {
    if (!ctx || !target_Q) return NULL;
    
    const EC_POINT* G = EC_GROUP_get0_generator(ctx->ec_group);
    BN_CTX* bn_ctx = BN_CTX_new();
    
    double previous_distance = 1e100;
    double step_size = 1.0;
    
    printf("Starting iterative recovery (max %u iterations)...\n", max_iterations);
    
    for (uint32_t iteration = 0; iteration < max_iterations; iteration++) {
        // Generate candidate using triangulation with truncation
        uint32_t vertex_idx = iteration % ctx->num_shared_vertices;
        if (vertex_idx >= ctx->num_shared_vertices) vertex_idx = 0;
        
        BIGNUM* candidate_k = triangulate_k_with_truncation(
            ctx->shared_vertices[vertex_idx].position,
            (const double**)ctx->anchor_k_positions,
            (const BIGNUM**)ctx->anchor_k_values,
            ctx->num_anchors,
            ctx->num_dimensions,
            ctx->ec_group
        );
        
        if (!candidate_k) continue;
        
        // PHASE 1: VERIFY candidate produces target_Q
        if (verify_candidate_produces_Q(candidate_k, target_Q, ctx->ec_group)) {
            // SUCCESS! Found exact match!
            printf("✅ FOUND EXACT MATCH at iteration %u!\n", iteration);
            BN_CTX_free(bn_ctx);
            *confidence_out = 1.0;
            *iterations_out = iteration + 1;
            return candidate_k;
        }
        
        // Not a match, measure distance for oscillation tracking
        EC_POINT* candidate_Q = EC_POINT_new(ctx->ec_group);
        EC_POINT_mul(ctx->ec_group, candidate_Q, NULL, G, candidate_k, bn_ctx);
        
        double distance = measure_point_distance(candidate_Q, target_Q, ctx->ec_group);
        
        // Track oscillation
        if (distance > previous_distance) {
            // Oscillating - reduce step size
            step_size *= 0.5;
            if ((iteration + 1) % 100 == 0) {
                printf("  Iteration %u: distance=%.6f (oscillating, step=%.3f)\n", 
                       iteration + 1, distance, step_size);
            }
        } else {
            // Converging
            if ((iteration + 1) % 100 == 0) {
                printf("  Iteration %u: distance=%.6f (converging)\n", 
                       iteration + 1, distance);
            }
        }
        
        previous_distance = distance;
        
        EC_POINT_free(candidate_Q);
        BN_free(candidate_k);
    }
    
    BN_CTX_free(bn_ctx);
    
    printf("⚠️ No exact match found in %u iterations\n", max_iterations);
    *confidence_out = 0.0;
    *iterations_out = max_iterations;
    
    return NULL;
}

// ============================================================================
// PHASE 3: DYNAMIC SCALING
// ============================================================================

/**
 * Measure oscillation magnitude in the system
 */
double measure_oscillation_magnitude(GeometricRecoveryContext* ctx) {
    if (!ctx || ctx->num_tori == 0) return 0.0;
    
    double total_variance = 0.0;
    
    for (uint32_t t = 0; t < ctx->num_tori; t++) {
        total_variance += ctx->tori[t].radius;
    }
    
    return total_variance / ctx->num_tori;
}

/**
 * PHASE 3: Recovery with dynamic scaling
 * 
 * Start with low complexity, scale up until oscillations stabilize
 */
BIGNUM* geometric_recovery_with_dynamic_scaling(
    EC_POINT* target_Q,
    EC_GROUP* ec_group,
    uint32_t initial_anchors,
    uint32_t initial_dimensions,
    double* confidence_out,
    uint32_t* final_dimensions_out,
    uint32_t* final_anchors_out
) {
    uint32_t dimensions = initial_dimensions;
    uint32_t num_anchors = initial_anchors;
    uint32_t max_scale_iterations = 5;
    const double STABILITY_THRESHOLD = 0.1;
    
    printf("\n=== DYNAMIC SCALING RECOVERY ===\n");
    
    for (uint32_t scale = 0; scale < max_scale_iterations; scale++) {
        printf("\n--- Scale %u: %uD, %u anchors ---\n", scale, dimensions, num_anchors);
        
        // Create context at current scale
        GeometricRecoveryContext* ctx = geometric_recovery_create(
            ec_group, num_anchors, dimensions
        );
        
        if (!ctx) {
            fprintf(stderr, "Failed to create context at scale %u\n", scale);
            continue;
        }
        
        // Generate anchors (simplified - in production, use actual anchor generation)
        // For now, we'll use the existing anchors from the test
        // This is a placeholder - real implementation would generate at this scale
        
        // Initialize (detect tori, find intersections)
        bool init_success = geometric_recovery_initialize(ctx);
        
        if (!init_success) {
            fprintf(stderr, "Initialization failed at scale %u\n", scale);
            geometric_recovery_free(ctx);
            continue;
        }
        
        printf("Initialized: %u tori, %u shared vertices\n", 
               ctx->num_tori, ctx->num_shared_vertices);
        
        // Attempt recovery with verification loop
        uint32_t iterations = 0;
        BIGNUM* recovered_k = geometric_recovery_iterative(
            ctx, target_Q, 1000, confidence_out, &iterations
        );
        
        if (recovered_k) {
            // SUCCESS!
            printf("\n✅ RECOVERED at scale %u (%uD, %u anchors, %u iterations)\n", 
                   scale, dimensions, num_anchors, iterations);
            *final_dimensions_out = dimensions;
            *final_anchors_out = num_anchors;
            geometric_recovery_free(ctx);
            return recovered_k;
        }
        
        // Measure oscillation magnitude
        double oscillation_mag = measure_oscillation_magnitude(ctx);
        printf("Oscillation magnitude: %.6f\n", oscillation_mag);
        
        if (oscillation_mag < STABILITY_THRESHOLD) {
            // Oscillations stable but no solution found
            printf("⚠️ Oscillations stable (%.6f < %.6f) but no solution\n", 
                   oscillation_mag, STABILITY_THRESHOLD);
            printf("   This scale is correct, but need more iterations or better anchors\n");
            geometric_recovery_free(ctx);
            break;
        }
        
        // Oscillations NOT stable - SCALE UP
        printf("📈 Oscillations not stable (%.6f >= %.6f) - scaling up\n", 
               oscillation_mag, STABILITY_THRESHOLD);
        
        dimensions *= 2;      // 13 → 26 → 52 → 104 → 208
        num_anchors *= 10;    // 100 → 1K → 10K → 100K → 1M
        
        geometric_recovery_free(ctx);
    }
    
    printf("\n❌ No solution found after %u scale iterations\n", max_scale_iterations);
    *confidence_out = 0.0;
    *final_dimensions_out = dimensions;
    *final_anchors_out = num_anchors;
    
    return NULL;
}
