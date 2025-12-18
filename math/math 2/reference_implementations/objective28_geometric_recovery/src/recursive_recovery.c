#include "recursive_recovery.h"
#include "geometric_recovery.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

// Compute π×φ distance
double pi_phi_distance(const double* p1, const double* p2, int dims) {
    double sum = 0.0;
    for (int d = 0; d < dims; d++) {
        double diff = p1[d] - p2[d];
        sum += diff * diff;
    }
    return sqrt(sum) / PI_PHI;
}

// Compute distance from origin
double distance_from_origin(const double* p, int dims) {
    double sum = 0.0;
    for (int d = 0; d < dims; d++) {
        sum += p[d] * p[d];
    }
    return sqrt(sum) / PI_PHI;
}

// Create recursive recovery context
RecursiveRecoveryContext* recursive_recovery_create(
    EC_GROUP* ec_group,
    RecursionParams* params
) {
    RecursiveRecoveryContext* ctx = calloc(1, sizeof(RecursiveRecoveryContext));
    if (!ctx) return NULL;
    
    ctx->ec_group = ec_group;
    ctx->bn_ctx = BN_CTX_new();
    ctx->order = BN_new();
    EC_GROUP_get_order(ec_group, ctx->order, ctx->bn_ctx);
    
    // Copy parameters
    ctx->params = malloc(sizeof(RecursionParams));
    memcpy(ctx->params, params, sizeof(RecursionParams));
    
    // Initialize torus state
    ctx->current_torus = calloc(1, sizeof(TorusState));
    ctx->current_torus->resolution_level = params->min_resolution;
    ctx->current_torus->num_anchors = params->min_anchors;
    ctx->current_torus->num_dimensions = params->min_dimensions;
    ctx->current_torus->recursion_depth = 0;
    
    // Allocate arrays
    ctx->anchor_k = malloc(params->max_anchors * sizeof(BIGNUM*));
    ctx->anchor_Q = malloc(params->max_anchors * sizeof(EC_POINT*));
    ctx->anchor_positions = malloc(params->max_anchors * sizeof(double*));
    
    ctx->known_k_samples = malloc(1000 * sizeof(BIGNUM*));
    ctx->known_k_positions = malloc(1000 * sizeof(double*));
    ctx->num_samples = 0;
    
    return ctx;
}

// Free context
void recursive_recovery_free(RecursiveRecoveryContext* ctx) {
    if (!ctx) return;
    
    for (uint32_t i = 0; i < ctx->num_anchors; i++) {
        if (ctx->anchor_k[i]) BN_free(ctx->anchor_k[i]);
        if (ctx->anchor_Q[i]) EC_POINT_free(ctx->anchor_Q[i]);
        if (ctx->anchor_positions[i]) free(ctx->anchor_positions[i]);
    }
    free(ctx->anchor_k);
    free(ctx->anchor_Q);
    free(ctx->anchor_positions);
    
    for (uint32_t i = 0; i < ctx->num_samples; i++) {
        if (ctx->known_k_samples[i]) BN_free(ctx->known_k_samples[i]);
        if (ctx->known_k_positions[i]) free(ctx->known_k_positions[i]);
    }
    free(ctx->known_k_samples);
    free(ctx->known_k_positions);
    
    if (ctx->current_torus) free(ctx->current_torus);
    if (ctx->params) free(ctx->params);
    if (ctx->order) BN_free(ctx->order);
    if (ctx->bn_ctx) BN_CTX_free(ctx->bn_ctx);
    
    free(ctx);
}

// Add known k sample
void recursive_recovery_add_sample(
    RecursiveRecoveryContext* ctx,
    BIGNUM* k,
    EC_POINT* Q
) {
    if (ctx->num_samples >= 1000) return;
    
    uint32_t idx = ctx->num_samples++;
    ctx->known_k_samples[idx] = BN_dup(k);
    
    // Map to position
    ctx->known_k_positions[idx] = malloc(ctx->current_torus->num_dimensions * sizeof(double));
    map_k_to_position(k, ctx->known_k_positions[idx], ctx->current_torus->num_dimensions);
}

// Analyze partition boundaries
double analyze_partition_boundaries(
    RecursiveRecoveryContext* ctx,
    double** positions,
    int num_positions
) {
    int num_uniform = 0;
    int dims = ctx->current_torus->num_dimensions;
    
    for (int p = 0; p < num_positions; p++) {
        // Find 3 nearest anchors using π×φ metric
        int nearest[3];
        double nearest_dists[3] = {1e100, 1e100, 1e100};
        
        for (uint32_t a = 0; a < ctx->num_anchors; a++) {
            double dist = pi_phi_distance(positions[p], ctx->anchor_positions[a], dims);
            
            for (int i = 0; i < 3; i++) {
                if (dist < nearest_dists[i]) {
                    for (int j = 2; j > i; j--) {
                        nearest[j] = nearest[j-1];
                        nearest_dists[j] = nearest_dists[j-1];
                    }
                    nearest[i] = a;
                    nearest_dists[i] = dist;
                    break;
                }
            }
        }
        
        // Check if distances are uniform
        double avg_dist = (nearest_dists[0] + nearest_dists[1] + nearest_dists[2]) / 3.0;
        double variance = 0.0;
        for (int i = 0; i < 3; i++) {
            double diff = nearest_dists[i] - avg_dist;
            variance += diff * diff;
        }
        variance /= 3.0;
        double stddev = sqrt(variance);
        
        if (stddev < 0.01) {
            num_uniform++;
        }
    }
    
    return (double)num_uniform / num_positions;
}

// Detect multiple tori
bool detect_multiple_tori(
    RecursiveRecoveryContext* ctx,
    TorusState* torus
) {
    // Measure distance distribution from torus center
    double min_dist = 1e100;
    double max_dist = 0.0;
    int dims = torus->num_dimensions;
    
    for (uint32_t i = 0; i < ctx->num_samples; i++) {
        double dist = pi_phi_distance(ctx->known_k_positions[i], torus->center, dims);
        if (dist < min_dist) min_dist = dist;
        if (dist > max_dist) max_dist = dist;
    }
    
    double range = max_dist - min_dist;
    
    // If range > 2×minor_radius, multiple tori detected
    return (range > (2.0 * torus->minor_radius));
}

// Scale up parameters
void scale_up_parameters(RecursiveRecoveryContext* ctx) {
    TorusState* state = ctx->current_torus;
    RecursionParams* params = ctx->params;
    
    printf("  Scaling up parameters:\n");
    
    // Scale resolution exponentially
    if (state->resolution_level < params->max_resolution) {
        uint32_t old_res = state->resolution_level;
        state->resolution_level *= 4;  // 2^24 → 2^26 → 2^28
        if (state->resolution_level > params->max_resolution) {
            state->resolution_level = params->max_resolution;
        }
        printf("    Resolution: %u → %u\n", old_res, state->resolution_level);
    }
    
    // Scale anchors
    if (state->num_anchors < params->max_anchors) {
        uint32_t old_anchors = state->num_anchors;
        state->num_anchors *= 10;  // 100 → 1000 → 10000
        if (state->num_anchors > params->max_anchors) {
            state->num_anchors = params->max_anchors;
        }
        printf("    Anchors: %u → %u\n", old_anchors, state->num_anchors);
    }
    
    // Scale dimensions if entropy is very high
    if (state->entropy > 0.15 && state->num_dimensions < params->max_dimensions) {
        uint32_t old_dims = state->num_dimensions;
        state->num_dimensions *= 2;  // 13 → 26
        if (state->num_dimensions > params->max_dimensions) {
            state->num_dimensions = params->max_dimensions;
        }
        printf("    Dimensions: %u → %u (high entropy)\n", old_dims, state->num_dimensions);
    }
    
    state->recursion_depth++;
}

// Map torus recursive
TorusState* map_torus_recursive(
    RecursiveRecoveryContext* ctx,
    int current_depth
) {
    printf("\n=== RECURSION LEVEL %d ===\n", current_depth);
    printf("Parameters: %uD, %u anchors, 2^%d vertices\n",
           ctx->current_torus->num_dimensions,
           ctx->current_torus->num_anchors,
           (int)log2(ctx->current_torus->resolution_level));
    
    // Check recursion depth
    if (current_depth >= ctx->params->max_recursion_depth) {
        printf("✗ Maximum recursion depth reached\n");
        return NULL;
    }
    
    int dims = ctx->current_torus->num_dimensions;
    
    // Compute torus center
    for (int d = 0; d < dims; d++) {
        ctx->current_torus->center[d] = 0.0;
        for (uint32_t i = 0; i < ctx->num_samples; i++) {
            ctx->current_torus->center[d] += ctx->known_k_positions[i][d];
        }
        ctx->current_torus->center[d] /= ctx->num_samples;
    }
    
    // Compute major radius
    ctx->current_torus->major_radius = distance_from_origin(ctx->current_torus->center, dims);
    
    // Compute minor radius
    ctx->current_torus->minor_radius = 0.0;
    for (uint32_t i = 0; i < ctx->num_samples; i++) {
        ctx->current_torus->minor_radius += pi_phi_distance(
            ctx->known_k_positions[i],
            ctx->current_torus->center,
            dims
        );
    }
    ctx->current_torus->minor_radius /= ctx->num_samples;
    
    // Compute entropy
    double variance = 0.0;
    for (uint32_t i = 0; i < ctx->num_samples; i++) {
        double dist = pi_phi_distance(ctx->known_k_positions[i], ctx->current_torus->center, dims);
        double diff = dist - ctx->current_torus->minor_radius;
        variance += diff * diff;
    }
    variance /= ctx->num_samples;
    ctx->current_torus->entropy = sqrt(variance);
    ctx->current_torus->is_stable = (ctx->current_torus->entropy < ctx->params->entropy_threshold);
    
    printf("Torus properties:\n");
    printf("  Major radius: %.6f\n", ctx->current_torus->major_radius);
    printf("  Minor radius: %.6f\n", ctx->current_torus->minor_radius);
    printf("  Entropy: %.6f %s\n", ctx->current_torus->entropy,
           ctx->current_torus->is_stable ? "(STABLE ✓)" : "(UNSTABLE ✗)");
    
    // Analyze partition boundaries
    ctx->current_torus->uniform_percentage = analyze_partition_boundaries(
        ctx,
        ctx->known_k_positions,
        ctx->num_samples
    );
    
    printf("  Uniform distances: %.1f%%\n", ctx->current_torus->uniform_percentage * 100.0);
    
    // Detect multiple tori
    bool multiple_tori = detect_multiple_tori(ctx, ctx->current_torus);
    printf("  Multiple tori: %s\n", multiple_tori ? "YES" : "NO");
    
    // Decision: recurse or return
    bool needs_recursion = false;
    const char* reason = NULL;
    
    if (!ctx->current_torus->is_stable) {
        needs_recursion = true;
        reason = "High entropy";
    } else if (ctx->current_torus->uniform_percentage < ctx->params->uniform_threshold) {
        needs_recursion = true;
        reason = "Low uniform percentage";
    } else if (multiple_tori) {
        needs_recursion = true;
        reason = "Multiple tori detected";
    }
    
    if (needs_recursion) {
        printf("\n→ RECURSION REQUIRED: %s\n", reason);
        scale_up_parameters(ctx);
        
        // Recurse
        return map_torus_recursive(ctx, current_depth + 1);
    } else {
        printf("\n✓ TORUS STABLE - Ready for recovery\n");
        return ctx->current_torus;
    }
}

// Recover k with torus
BIGNUM* recover_k_with_torus(
    RecursiveRecoveryContext* ctx,
    EC_POINT* target_Q,
    TorusState* stable_torus
) {
    printf("\n=== K RECOVERY WITH STABLE TORUS ===\n");
    
    // TODO: Implement full recovery using:
    // 1. Generate deterministic target from Q
    // 2. Project onto torus surface
    // 3. Find partition using π×φ metric
    // 4. Triangulate with uniform anchors
    // 5. Generate candidates
    // 6. Verify with Q
    
    printf("Recovery implementation pending...\n");
    
    return NULL;
}

// Main entry point
BIGNUM* recursive_k_recovery(
    EC_GROUP* ec_group,
    EC_POINT* target_Q,
    BIGNUM** known_k_samples,
    EC_POINT** known_Q_samples,
    int num_samples,
    RecursionParams* params
) {
    printf("=== RECURSIVE K RECOVERY ===\n");
    printf("Samples: %d\n", num_samples);
    printf("Initial parameters: %uD, %u anchors, 2^%d resolution\n\n",
           params->min_dimensions, params->min_anchors, (int)log2(params->min_resolution));
    
    // Create context
    RecursiveRecoveryContext* ctx = recursive_recovery_create(ec_group, params);
    if (!ctx) return NULL;
    
    // Add samples
    printf("Adding %d known k samples...\n", num_samples);
    for (int i = 0; i < num_samples; i++) {
        recursive_recovery_add_sample(ctx, known_k_samples[i], known_Q_samples[i]);
    }
    
    // Generate initial anchors
    printf("Generating %u initial anchors...\n", ctx->current_torus->num_anchors);
    ctx->num_anchors = ctx->current_torus->num_anchors;
    for (uint32_t i = 0; i < ctx->num_anchors; i++) {
        ctx->anchor_k[i] = BN_new();
        BN_rand_range(ctx->anchor_k[i], ctx->order);
        ctx->anchor_Q[i] = EC_POINT_new(ec_group);
        EC_POINT_mul(ec_group, ctx->anchor_Q[i], ctx->anchor_k[i], NULL, NULL, ctx->bn_ctx);
        
        ctx->anchor_positions[i] = malloc(ctx->current_torus->num_dimensions * sizeof(double));
        map_k_to_position(ctx->anchor_k[i], ctx->anchor_positions[i], ctx->current_torus->num_dimensions);
    }
    
    // Map torus recursively
    TorusState* stable_torus = map_torus_recursive(ctx, 0);
    
    if (!stable_torus) {
        printf("\n✗ Failed to find stable torus\n");
        recursive_recovery_free(ctx);
        return NULL;
    }
    
    // Recover k using stable torus
    BIGNUM* recovered_k = recover_k_with_torus(ctx, target_Q, stable_torus);
    
    recursive_recovery_free(ctx);
    return recovered_k;
}