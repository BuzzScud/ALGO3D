#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/obj_mac.h>
#include "../include/geometric_recovery.h"

#define NUM_TEST_K 10
#define NUM_ANCHORS 100
#define PI 3.14159265358979323846
#define GOLDEN_RATIO 1.61803398874989484820

// Compute distance using π×φ metric
double compute_pi_phi_distance(const double* p1, const double* p2, int dims) {
    double sum = 0.0;
    for (int d = 0; d < dims; d++) {
        double diff = p1[d] - p2[d];
        sum += diff * diff;
    }
    // Use π×φ as the unit of measure
    return sqrt(sum) / (PI * GOLDEN_RATIO);
}

// Compute distance from origin (zero point)
double compute_distance_from_zero(const double* p, int dims) {
    double sum = 0.0;
    for (int d = 0; d < dims; d++) {
        sum += p[d] * p[d];
    }
    return sqrt(sum) / (PI * GOLDEN_RATIO);
}

int main(void) {
    printf("=== COMPREHENSIVE GRAPH STRUCTURE ANALYSIS ===\n");
    printf("Using π×φ metric for distance measurements\n");
    printf("π×φ = %.15f\n\n", PI * GOLDEN_RATIO);
    
    // Initialize curve
    EC_GROUP* curve = EC_GROUP_new_by_curve_name(NID_secp128r1);
    BN_CTX* bn_ctx = BN_CTX_new();
    BIGNUM* order = BN_new();
    EC_GROUP_get_order(curve, order, bn_ctx);
    const EC_POINT* G = EC_GROUP_get0_generator(curve);
    
    // Generate 10 test k values
    printf("Generating %d test k values...\n", NUM_TEST_K);
    BIGNUM** test_k = malloc(NUM_TEST_K * sizeof(BIGNUM*));
    EC_POINT** test_Q = malloc(NUM_TEST_K * sizeof(EC_POINT*));
    double test_k_positions[NUM_TEST_K][13];
    
    for (int i = 0; i < NUM_TEST_K; i++) {
        test_k[i] = BN_new();
        BN_rand_range(test_k[i], order);
        test_Q[i] = EC_POINT_new(curve);
        EC_POINT_mul(curve, test_Q[i], test_k[i], NULL, NULL, bn_ctx);
        
        // Map to 13D position
        map_k_to_position(test_k[i], test_k_positions[i], 13);
        
        char* k_hex = BN_bn2hex(test_k[i]);
        printf("  Test k[%d]: %s\n", i, k_hex);
        OPENSSL_free(k_hex);
    }
    
    // Generate anchors
    printf("\nGenerating %d anchors...\n", NUM_ANCHORS);
    BIGNUM** anchor_k = malloc(NUM_ANCHORS * sizeof(BIGNUM*));
    EC_POINT** anchor_Q = malloc(NUM_ANCHORS * sizeof(EC_POINT*));
    
    for (int i = 0; i < NUM_ANCHORS; i++) {
        anchor_k[i] = BN_new();
        BN_rand_range(anchor_k[i], order);
        anchor_Q[i] = EC_POINT_new(curve);
        EC_POINT_mul(curve, anchor_Q[i], anchor_k[i], NULL, NULL, bn_ctx);
    }
    
    // Create recovery context
    printf("Creating recovery context (2^24 vertices)...\n");
    GeometricRecoveryContext* ctx = geometric_recovery_create(curve, NUM_ANCHORS, 13);
    
    for (int i = 0; i < NUM_ANCHORS; i++) {
        geometric_recovery_add_anchor(ctx, anchor_k[i], anchor_Q[i]);
    }
    
    printf("Initializing...\n");
    geometric_recovery_initialize(ctx);
    printf("Initialized!\n\n");
    
    // === ANALYSIS 1: Distance from Zero ===
    printf("=== ANALYSIS 1: DISTANCE FROM ZERO (π×φ metric) ===\n\n");
    
    printf("Test k distances from zero:\n");
    for (int i = 0; i < NUM_TEST_K; i++) {
        double dist = compute_distance_from_zero(test_k_positions[i], 13);
        printf("  k[%d]: %.10f\n", i, dist);
    }
    
    printf("\nAnchor distances from zero (first 10):\n");
    for (int i = 0; i < 10; i++) {
        double dist = compute_distance_from_zero(ctx->anchor_k_positions[i], 13);
        printf("  Anchor[%d]: %.10f\n", i, dist);
    }
    
    // Check if all are equidistant from zero
    printf("\n--- Checking if all points are equidistant from zero ---\n");
    double test_k_avg_dist = 0.0;
    for (int i = 0; i < NUM_TEST_K; i++) {
        test_k_avg_dist += compute_distance_from_zero(test_k_positions[i], 13);
    }
    test_k_avg_dist /= NUM_TEST_K;
    
    double anchor_avg_dist = 0.0;
    for (int i = 0; i < NUM_ANCHORS; i++) {
        anchor_avg_dist += compute_distance_from_zero(ctx->anchor_k_positions[i], 13);
    }
    anchor_avg_dist /= NUM_ANCHORS;
    
    printf("Average distance from zero:\n");
    printf("  Test k: %.10f\n", test_k_avg_dist);
    printf("  Anchors: %.10f\n", anchor_avg_dist);
    printf("  Difference: %.10f\n", fabs(test_k_avg_dist - anchor_avg_dist));
    
    // === ANALYSIS 2: Pairwise Distances Between Test K ===
    printf("\n=== ANALYSIS 2: PAIRWISE DISTANCES BETWEEN TEST K (π×φ metric) ===\n\n");
    
    printf("Distance matrix:\n");
    printf("     ");
    for (int j = 0; j < NUM_TEST_K; j++) {
        printf("  k[%d]  ", j);
    }
    printf("\n");
    
    for (int i = 0; i < NUM_TEST_K; i++) {
        printf("k[%d] ", i);
        for (int j = 0; j < NUM_TEST_K; j++) {
            if (i == j) {
                printf(" 0.0000 ");
            } else {
                double dist = compute_pi_phi_distance(test_k_positions[i], test_k_positions[j], 13);
                printf(" %.4f ", dist);
            }
        }
        printf("\n");
    }
    
    // === ANALYSIS 3: Nearest Anchors for Each Test K ===
    printf("\n=== ANALYSIS 3: NEAREST ANCHORS FOR EACH TEST K ===\n\n");
    
    for (int t = 0; t < NUM_TEST_K; t++) {
        printf("Test k[%d]:\n", t);
        
        // Find 3 nearest anchors
        typedef struct {
            int index;
            double euclidean_dist;
            double pi_phi_dist;
        } AnchorDist;
        
        AnchorDist* dists = malloc(NUM_ANCHORS * sizeof(AnchorDist));
        
        for (int a = 0; a < NUM_ANCHORS; a++) {
            dists[a].index = a;
            
            // Euclidean distance
            double euc_sum = 0.0;
            for (int d = 0; d < 13; d++) {
                double diff = test_k_positions[t][d] - ctx->anchor_k_positions[a][d];
                euc_sum += diff * diff;
            }
            dists[a].euclidean_dist = sqrt(euc_sum);
            
            // π×φ distance
            dists[a].pi_phi_dist = compute_pi_phi_distance(test_k_positions[t], ctx->anchor_k_positions[a], 13);
        }
        
        // Sort by Euclidean distance
        for (int i = 0; i < NUM_ANCHORS - 1; i++) {
            for (int j = i + 1; j < NUM_ANCHORS; j++) {
                if (dists[j].euclidean_dist < dists[i].euclidean_dist) {
                    AnchorDist temp = dists[i];
                    dists[i] = dists[j];
                    dists[j] = temp;
                }
            }
        }
        
        printf("  3 Nearest anchors:\n");
        for (int i = 0; i < 3; i++) {
            printf("    Anchor[%3d]: Euclidean=%.6f, π×φ=%.6f\n", 
                   dists[i].index, dists[i].euclidean_dist, dists[i].pi_phi_dist);
        }
        
        // Check if π×φ distances are uniform
        double avg_pi_phi = (dists[0].pi_phi_dist + dists[1].pi_phi_dist + dists[2].pi_phi_dist) / 3.0;
        double variance = 0.0;
        for (int i = 0; i < 3; i++) {
            double diff = dists[i].pi_phi_dist - avg_pi_phi;
            variance += diff * diff;
        }
        variance /= 3.0;
        double stddev = sqrt(variance);
        
        printf("  π×φ distance statistics:\n");
        printf("    Average: %.6f\n", avg_pi_phi);
        printf("    Std dev: %.6f\n", stddev);
        if (stddev < 0.01) {
            printf("    ✓ UNIFORM - vertices are equidistant in π×φ metric!\n");
        }
        
        free(dists);
        printf("\n");
    }
    
    // === ANALYSIS 4: Circular/Spherical Structure ===
    printf("=== ANALYSIS 4: CIRCULAR/SPHERICAL STRUCTURE ===\n\n");
    
    printf("Analyzing if points lie on a hypersphere...\n");
    
    // Compute variance in distance from zero
    double variance_from_zero = 0.0;
    double avg_dist_from_zero = 0.0;
    
    for (int i = 0; i < NUM_TEST_K; i++) {
        double dist = compute_distance_from_zero(test_k_positions[i], 13);
        avg_dist_from_zero += dist;
    }
    avg_dist_from_zero /= NUM_TEST_K;
    
    for (int i = 0; i < NUM_TEST_K; i++) {
        double dist = compute_distance_from_zero(test_k_positions[i], 13);
        double diff = dist - avg_dist_from_zero;
        variance_from_zero += diff * diff;
    }
    variance_from_zero /= NUM_TEST_K;
    double stddev_from_zero = sqrt(variance_from_zero);
    
    printf("Distance from zero statistics:\n");
    printf("  Average: %.10f\n", avg_dist_from_zero);
    printf("  Std dev: %.10f\n", stddev_from_zero);
    printf("  Coefficient of variation: %.6f%%\n", (stddev_from_zero / avg_dist_from_zero) * 100.0);
    
    if (stddev_from_zero / avg_dist_from_zero < 0.01) {
        printf("  ✓ Points lie on a HYPERSPHERE (all equidistant from origin)!\n");
        printf("  Hypersphere radius: %.10f (in π×φ units)\n", avg_dist_from_zero);
    } else {
        printf("  ✗ Points do NOT lie on a perfect hypersphere\n");
    }
    
    // === ANALYSIS 5: Graph Hop Distances ===
    printf("\n=== ANALYSIS 5: GRAPH HOP DISTANCES ===\n\n");
    printf("Analyzing graph structure of Platonic model...\n");
    printf("Model vertices: %u\n", ctx->model->num_vertices);
    printf("Model dimensions: %u\n", ctx->model->num_dimensions);
    
    // For each test k, find which graph vertex it's closest to
    printf("\nMapping test k to graph vertices:\n");
    for (int t = 0; t < NUM_TEST_K; t++) {
        // Find closest vertex in the Platonic model graph
        double min_dist = 1e100;
        uint32_t closest_vertex = 0;
        
        // Sample first 1000 vertices (full scan would be too slow)
        for (uint32_t v = 0; v < 1000 && v < ctx->model->num_vertices; v++) {
            double dist = 0.0;
            for (int d = 0; d < 13; d++) {
                double vertex_coord = ctx->model->vertex_positions[v * 13 + d];
                double diff = test_k_positions[t][d] - vertex_coord;
                dist += diff * diff;
            }
            dist = sqrt(dist);
            
            if (dist < min_dist) {
                min_dist = dist;
                closest_vertex = v;
            }
        }
        
        printf("  k[%d] → Vertex %u (distance: %.6f)\n", t, closest_vertex, min_dist);
    }
    
    // === ANALYSIS 6: Oscillation Pattern ===
    printf("\n=== ANALYSIS 6: OSCILLATION PATTERN ANALYSIS ===\n\n");
    
    printf("Analyzing oscillation around centroids...\n");
    
    for (int t = 0; t < NUM_TEST_K; t++) {
        // Find 3 nearest anchors
        int nearest[3];
        double nearest_dists[3] = {1e100, 1e100, 1e100};
        
        for (int a = 0; a < NUM_ANCHORS; a++) {
            double dist = 0.0;
            for (int d = 0; d < 13; d++) {
                double diff = test_k_positions[t][d] - ctx->anchor_k_positions[a][d];
                dist += diff * diff;
            }
            dist = sqrt(dist);
            
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
        
        // Compute centroid
        double centroid[13];
        for (int d = 0; d < 13; d++) {
            centroid[d] = (ctx->anchor_k_positions[nearest[0]][d] +
                          ctx->anchor_k_positions[nearest[1]][d] +
                          ctx->anchor_k_positions[nearest[2]][d]) / 3.0;
        }
        
        // Compute distance from k to centroid
        double dist_to_centroid = 0.0;
        for (int d = 0; d < 13; d++) {
            double diff = test_k_positions[t][d] - centroid[d];
            dist_to_centroid += diff * diff;
        }
        dist_to_centroid = sqrt(dist_to_centroid);
        
        // Compute average distance from vertices to centroid
        double avg_vertex_to_centroid = 0.0;
        for (int i = 0; i < 3; i++) {
            double dist = 0.0;
            for (int d = 0; d < 13; d++) {
                double diff = ctx->anchor_k_positions[nearest[i]][d] - centroid[d];
                dist += diff * diff;
            }
            avg_vertex_to_centroid += sqrt(dist);
        }
        avg_vertex_to_centroid /= 3.0;
        
        double oscillation_ratio = dist_to_centroid / avg_vertex_to_centroid;
        
        printf("  k[%d]:\n", t);
        printf("    Distance to centroid: %.6f\n", dist_to_centroid);
        printf("    Avg vertex to centroid: %.6f\n", avg_vertex_to_centroid);
        printf("    Oscillation ratio: %.6f ", oscillation_ratio);
        
        if (oscillation_ratio < 0.5) {
            printf("(near center)\n");
        } else if (oscillation_ratio < 1.0) {
            printf("(inside partition)\n");
        } else {
            printf("(near/outside boundary)\n");
        }
    }
    
    // Cleanup
    for (int i = 0; i < NUM_TEST_K; i++) {
        BN_free(test_k[i]);
        EC_POINT_free(test_Q[i]);
    }
    free(test_k);
    free(test_Q);
    
    for (int i = 0; i < NUM_ANCHORS; i++) {
        BN_free(anchor_k[i]);
        EC_POINT_free(anchor_Q[i]);
    }
    free(anchor_k);
    free(anchor_Q);
    
    geometric_recovery_free(ctx);
    BN_free(order);
    BN_CTX_free(bn_ctx);
    EC_GROUP_free(curve);
    
    printf("\n=== ANALYSIS COMPLETE ===\n");
    return 0;
}