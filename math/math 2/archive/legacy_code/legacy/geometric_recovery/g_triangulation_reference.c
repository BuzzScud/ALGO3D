/**
 * g_triangulation.c - G Triangulation and Iterative Refinement
 * 
 * Implements the correct approach:
 * 1. Start with known G (publicly available)
 * 2. Map G to clock lattice position
 * 3. Adjust Platonic solid anchors relative to G
 * 4. For each known (k, Q) pair, estimate k from adjusted anchors
 * 5. Track oscillation in k estimates
 * 6. Re-triangulate G from oscillation patterns
 * 7. Repeat until convergence
 * 
 * PHASE 2: Consolidated constants
 * - Using MATH_PHI, MATH_PI, MATH_TWO_PI from math/types.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <openssl/ec.h>
#include <openssl/bn.h>
#include "math/arithmetic.h"
#include "math/transcendental.h"
#include "../../math/include/math/types.h"  // PHASE 2: For MATH_PHI, MATH_PI, MATH_TWO_PI

#define MATH_PHI MATH_PHI

// 13 dimensional frequencies
static const uint64_t DIMENSIONAL_FREQUENCIES[13] = {
    3, 7, 31, 12, 19, 5, 11, 13, 17, 23, 29, 37, 41
};

typedef struct {
    double position[13];        // Position in 13D clock lattice
    uint64_t k_estimate;        // Estimated k value
    double confidence;          // Confidence score
} Anchor;

typedef struct {
    const EC_GROUP* group;      // EC group
    EC_POINT* G;                // Generator point (known)
    double G_position[13];      // G position in clock lattice
    
    Anchor* anchors;            // Platonic solid anchors
    int num_anchors;            // Number of anchors (50)
    
    // Known (k, Q) pairs for training
    int num_training_pairs;
    uint64_t* training_k;
    EC_POINT** training_Q;
    
    // Oscillation tracking
    double** k_estimates_history;  // [iteration][pair_index]
    int max_iterations;
    int current_iteration;
    
    // Convergence metrics
    double g_movement;          // How much G moved in last iteration
    double k_oscillation;       // Average oscillation in k estimates
    bool converged;
} GTriangulationContext;

/**
 * Map EC point to 13D clock lattice position
 */
void map_ec_point_to_lattice(
    const EC_POINT* point,
    const EC_GROUP* group,
    double position[13]
) {
    BIGNUM* x = BN_new();
    BIGNUM* y = BN_new();
    
    if (!EC_POINT_get_affine_coordinates(group, point, x, y, NULL)) {
        BN_free(x);
        BN_free(y);
        return;
    }
    
    char* x_str = BN_bn2hex(x);
    char* y_str = BN_bn2hex(y);
    
    char x_substr[17] = {0};
    char y_substr[17] = {0};
    strncpy(x_substr, x_str, 16);
    strncpy(y_substr, y_str, 16);
    
    uint64_t x_val = strtoull(x_substr, NULL, 16);
    uint64_t y_val = strtoull(y_substr, NULL, 16);
    
    for (int d = 0; d < 13; d++) {
        double freq = (double)DIMENSIONAL_FREQUENCIES[d];
        double angle = (double)(x_val % 360) * MATH_PI / 180.0;
        position[d] = math_cos(angle * freq) * math_pow(MATH_PHI, d % 5);
        
        double y_angle = (double)(y_val % 360) * MATH_PI / 180.0;
        position[d] += math_sin(y_angle * freq) * math_pow(MATH_PHI, (d + 1) % 5) * 0.5;
    }
    
    OPENSSL_free(x_str);
    OPENSSL_free(y_str);
    BN_free(x);
    BN_free(y);
}

/**
 * Map k to clock lattice position using π×φ metric
 */
void map_k_to_lattice(uint64_t k, double position[13]) {
    double base_angle = (double)k * MATH_PI * MATH_PHI;
    while (base_angle < 0) base_angle += MATH_TWO_PI;
    while (base_angle >= MATH_TWO_PI) base_angle -= MATH_TWO_PI;
    
    for (int d = 0; d < 13; d++) {
        double freq = (double)DIMENSIONAL_FREQUENCIES[d];
        position[d] = math_cos(base_angle * freq) * math_pow(MATH_PHI, d % 5);
    }
}

/**
 * Compute distance between two positions
 */
double compute_distance(const double pos1[13], const double pos2[13]) {
    double sum = 0.0;
    for (int d = 0; d < 13; d++) {
        double diff = pos1[d] - pos2[d];
        sum += diff * diff;
    }
    return math_sqrt(sum);
}

/**
 * Generate Platonic solid anchors (50 vertices)
 */
void generate_platonic_anchors(Anchor* anchors, int* num_anchors) {
    int idx = 0;
    
    // Tetrahedron: 4 vertices
    for (int v = 0; v < 4; v++) {
        double angle = v * MATH_TWO_PI / 4.0;
        for (int d = 0; d < 13; d++) {
            double phi_d = (double)DIMENSIONAL_FREQUENCIES[d];
            anchors[idx].position[d] = math_cos(angle * phi_d) * math_pow(MATH_PHI, d % 3);
        }
        anchors[idx].k_estimate = 0;
        anchors[idx].confidence = 0.0;
        idx++;
    }
    
    // Cube: 8 vertices
    for (int v = 0; v < 8; v++) {
        int x = (v & 1) ? 1 : -1;
        int y = (v & 2) ? 1 : -1;
        int z = (v & 4) ? 1 : -1;
        for (int d = 0; d < 13; d++) {
            double phi_d = (double)DIMENSIONAL_FREQUENCIES[d];
            anchors[idx].position[d] = 
                (x * math_cos(phi_d) + y * math_sin(phi_d) + z * math_cos(2.0 * phi_d)) / math_sqrt(3.0);
        }
        anchors[idx].k_estimate = 0;
        anchors[idx].confidence = 0.0;
        idx++;
    }
    
    // Octahedron: 6 vertices
    for (int v = 0; v < 6; v++) {
        double angle = v * MATH_TWO_PI / 6.0;
        for (int d = 0; d < 13; d++) {
            double phi_d = (double)DIMENSIONAL_FREQUENCIES[d];
            anchors[idx].position[d] = math_cos(angle * phi_d) * math_pow(MATH_PHI, d % 2);
        }
        anchors[idx].k_estimate = 0;
        anchors[idx].confidence = 0.0;
        idx++;
    }
    
    // Dodecahedron: 20 vertices
    for (int v = 0; v < 20; v++) {
        double angle = v * MATH_TWO_PI / 20.0;
        for (int d = 0; d < 13; d++) {
            double phi_d = (double)DIMENSIONAL_FREQUENCIES[d];
            anchors[idx].position[d] = math_cos(angle * phi_d * MATH_PHI) * math_pow(MATH_PHI, d % 5);
        }
        anchors[idx].k_estimate = 0;
        anchors[idx].confidence = 0.0;
        idx++;
    }
    
    // Icosahedron: 12 vertices
    for (int v = 0; v < 12; v++) {
        double angle = v * MATH_TWO_PI / 12.0;
        for (int d = 0; d < 13; d++) {
            double phi_d = (double)DIMENSIONAL_FREQUENCIES[d];
            anchors[idx].position[d] = math_cos(angle * phi_d) * math_pow(MATH_PHI, d % 4);
        }
        anchors[idx].k_estimate = 0;
        anchors[idx].confidence = 0.0;
        idx++;
    }
    
    *num_anchors = 50;
}

/**
 * Adjust anchors relative to G position and assign k estimates
 */
void adjust_anchors_relative_to_g(
    Anchor* anchors,
    int num_anchors,
    const double g_position[13]
) {
    // For each anchor, adjust its position relative to G
    // This creates a "coordinate system" centered on G
    for (int i = 0; i < num_anchors; i++) {
        for (int d = 0; d < 13; d++) {
            // Shift anchor position by G
            anchors[i].position[d] += g_position[d];
        }
        
        // Assign k estimate based on anchor index
        // Map 50 anchors to k range [0, 300]
        // This gives us anchors at k ≈ 0, 6, 12, 18, 24, 30, ...
        anchors[i].k_estimate = (uint64_t)((double)i * 300.0 / 50.0);
        anchors[i].confidence = 1.0;
    }
}

/**
 * Create G triangulation context
 */
GTriangulationContext* create_g_triangulation_context(
    int curve_nid,
    int num_training_pairs,
    const uint64_t* training_k,
    int max_iterations
) {
    GTriangulationContext* ctx = (GTriangulationContext*)calloc(1, sizeof(GTriangulationContext));
    if (!ctx) return NULL;
    
    // Get EC group and generator
    ctx->group = EC_GROUP_new_by_curve_name(curve_nid);
    if (!ctx->group) {
        free(ctx);
        return NULL;
    }
    
    const EC_POINT* G_const = EC_GROUP_get0_generator(ctx->group);
    ctx->G = EC_POINT_dup(G_const, ctx->group);
    
    // Map G to clock lattice
    map_ec_point_to_lattice(ctx->G, ctx->group, ctx->G_position);
    
    // Generate Platonic solid anchors
    ctx->anchors = (Anchor*)calloc(50, sizeof(Anchor));
    generate_platonic_anchors(ctx->anchors, &ctx->num_anchors);
    
    // Adjust anchors relative to G
    adjust_anchors_relative_to_g(ctx->anchors, ctx->num_anchors, ctx->G_position);
    
    // Store training pairs
    ctx->num_training_pairs = num_training_pairs;
    ctx->training_k = (uint64_t*)malloc(num_training_pairs * sizeof(uint64_t));
    ctx->training_Q = (EC_POINT**)malloc(num_training_pairs * sizeof(EC_POINT*));
    
    for (int i = 0; i < num_training_pairs; i++) {
        ctx->training_k[i] = training_k[i];
        
        // Compute Q = k * G
        ctx->training_Q[i] = EC_POINT_new(ctx->group);
        BIGNUM* k_bn = BN_new();
        BN_set_word(k_bn, training_k[i]);
        EC_POINT_mul(ctx->group, ctx->training_Q[i], k_bn, NULL, NULL, NULL);
        BN_free(k_bn);
    }
    
    // Allocate oscillation tracking
    ctx->max_iterations = max_iterations;
    ctx->current_iteration = 0;
    ctx->k_estimates_history = (double**)malloc(max_iterations * sizeof(double*));
    for (int i = 0; i < max_iterations; i++) {
        ctx->k_estimates_history[i] = (double*)calloc(num_training_pairs, sizeof(double));
    }
    
    ctx->converged = false;
    
    return ctx;
}

/**
 * Free G triangulation context
 */
void free_g_triangulation_context(GTriangulationContext* ctx) {
    if (!ctx) return;
    
    if (ctx->anchors) free(ctx->anchors);
    if (ctx->training_k) free(ctx->training_k);
    
    if (ctx->training_Q) {
        for (int i = 0; i < ctx->num_training_pairs; i++) {
            if (ctx->training_Q[i]) EC_POINT_free(ctx->training_Q[i]);
        }
        free(ctx->training_Q);
    }
    
    if (ctx->k_estimates_history) {
        for (int i = 0; i < ctx->max_iterations; i++) {
            if (ctx->k_estimates_history[i]) free(ctx->k_estimates_history[i]);
        }
        free(ctx->k_estimates_history);
    }
    
    if (ctx->G) EC_POINT_free(ctx->G);
    if (ctx->group) EC_GROUP_free((EC_GROUP*)ctx->group);
    
    free(ctx);
}

/**
 * Estimate k from Q using adjusted anchors
 */
uint64_t estimate_k_from_q(
    GTriangulationContext* ctx,
    const EC_POINT* Q
) {
    // Map Q to lattice
    double q_position[13];
    map_ec_point_to_lattice(Q, ctx->group, q_position);
    
    // Find 3 nearest anchors
    int nearest_indices[3] = {0, 0, 0};
    double nearest_distances[3] = {1e9, 1e9, 1e9};
    
    for (int i = 0; i < ctx->num_anchors; i++) {
        double dist = compute_distance(q_position, ctx->anchors[i].position);
        
        for (int j = 0; j < 3; j++) {
            if (dist < nearest_distances[j]) {
                // Shift down
                for (int k = 2; k > j; k--) {
                    nearest_indices[k] = nearest_indices[k-1];
                    nearest_distances[k] = nearest_distances[k-1];
                }
                nearest_indices[j] = i;
                nearest_distances[j] = dist;
                break;
            }
        }
    }
    
    // Weighted average of anchor k estimates
    double total_weight = 0.0;
    double weighted_sum = 0.0;
    
    for (int i = 0; i < 3; i++) {
        if (nearest_distances[i] < 0.001) {
            return ctx->anchors[nearest_indices[i]].k_estimate;
        }
        double weight = 1.0 / nearest_distances[i];
        weighted_sum += weight * ctx->anchors[nearest_indices[i]].k_estimate;
        total_weight += weight;
    }
    
    return (uint64_t)(weighted_sum / total_weight);
}

/**
 * Perform one iteration of refinement
 */
void perform_refinement_iteration(GTriangulationContext* ctx) {
    if (ctx->current_iteration >= ctx->max_iterations) return;
    
    // Step 1: For each training pair, find nearest anchors and adjust their k estimates
    for (int i = 0; i < ctx->num_training_pairs; i++) {
        // Map Q to lattice
        double q_position[13];
        map_ec_point_to_lattice(ctx->training_Q[i], ctx->group, q_position);
        
        // Find nearest anchor
        int nearest_idx = 0;
        double min_dist = 1e9;
        
        for (int j = 0; j < ctx->num_anchors; j++) {
            double dist = compute_distance(q_position, ctx->anchors[j].position);
            if (dist < min_dist) {
                min_dist = dist;
                nearest_idx = j;
            }
        }
        
        // Adjust nearest anchor's k estimate towards the known k
        // Use exponential moving average for smooth convergence
        double alpha = 0.3;  // Learning rate
        uint64_t known_k = ctx->training_k[i];
        uint64_t current_estimate = ctx->anchors[nearest_idx].k_estimate;
        
        // Update anchor k estimate
        ctx->anchors[nearest_idx].k_estimate = 
            (uint64_t)((1.0 - alpha) * current_estimate + alpha * known_k);
        
        // Update confidence based on distance
        ctx->anchors[nearest_idx].confidence = 1.0 / (1.0 + min_dist);
    }
    
    // Step 2: For each training pair, estimate k using updated anchors
    for (int i = 0; i < ctx->num_training_pairs; i++) {
        uint64_t k_estimated = estimate_k_from_q(ctx, ctx->training_Q[i]);
        ctx->k_estimates_history[ctx->current_iteration][i] = (double)k_estimated;
    }
    
    // Step 3: Compute oscillation metrics
    if (ctx->current_iteration > 0) {
        double total_oscillation = 0.0;
        for (int i = 0; i < ctx->num_training_pairs; i++) {
            double prev = ctx->k_estimates_history[ctx->current_iteration - 1][i];
            double curr = ctx->k_estimates_history[ctx->current_iteration][i];
            total_oscillation += math_abs(curr - prev);
        }
        ctx->k_oscillation = total_oscillation / ctx->num_training_pairs;
    }
    
    ctx->current_iteration++;
}

/**
 * Check for convergence
 */
bool check_convergence(GTriangulationContext* ctx, double threshold) {
    if (ctx->current_iteration < 2) return false;
    
    // Check if k oscillation is below threshold
    if (ctx->k_oscillation < threshold) {
        ctx->converged = true;
        return true;
    }
    
    return false;
}
