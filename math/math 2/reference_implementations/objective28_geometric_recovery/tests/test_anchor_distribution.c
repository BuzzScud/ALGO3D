#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <openssl/bn.h>
#include "../include/geometric_recovery.h"

// Helper to compute distance in 13D space
static double compute_distance_13d(const double* p1, const double* p2) {
    double sum = 0.0;
    for (int i = 0; i < 13; i++) {
        double diff = p1[i] - p2[i];
        sum += diff * diff;
    }
    return sqrt(sum);
}

// Helper to compute centroid of points
static void compute_centroid(const double points[][13], int num_points, double* centroid) {
    for (int i = 0; i < 13; i++) {
        centroid[i] = 0.0;
        for (int j = 0; j < num_points; j++) {
            centroid[i] += points[j][i];
        }
        centroid[i] /= num_points;
    }
}

// Helper to compute variance along each dimension
static void compute_variance(const double points[][13], int num_points, const double* centroid, double* variance) {
    for (int i = 0; i < 13; i++) {
        variance[i] = 0.0;
        for (int j = 0; j < num_points; j++) {
            double diff = points[j][i] - centroid[i];
            variance[i] += diff * diff;
        }
        variance[i] /= num_points;
    }
}

int main(void) {
    printf("=== Anchor Distribution Analysis ===\n\n");
    
    // Initialize OpenSSL
    BN_CTX* bn_ctx = BN_CTX_new();
    if (!bn_ctx) {
        fprintf(stderr, "Failed to create BN_CTX\n");
        return 1;
    }
    
    // Generate anchors
    int num_anchors = 100;
    printf("Generating %d anchors...\n", num_anchors);
    
    BIGNUM** anchors = malloc(num_anchors * sizeof(BIGNUM*));
    if (!anchors) {
        fprintf(stderr, "Failed to allocate anchors\n");
        BN_CTX_free(bn_ctx);
        return 1;
    }
    
    for (int i = 0; i < num_anchors; i++) {
        anchors[i] = BN_new();
        uint32_t val = (uint32_t)rand() | ((uint32_t)rand() << 16);
        BN_set_word(anchors[i], val);
    }
    
    // Map anchors to 13D space
    double (*anchor_positions)[13] = malloc(num_anchors * sizeof(double[13]));
    if (!anchor_positions) {
        fprintf(stderr, "Failed to allocate anchor positions\n");
        for (int i = 0; i < num_anchors; i++) {
            BN_free(anchors[i]);
        }
        free(anchors);
        BN_CTX_free(bn_ctx);
        return 1;
    }
    
    for (int i = 0; i < num_anchors; i++) {
        map_k_to_position(anchors[i], anchor_positions[i], 13);
    }
    
    // Compute statistics
    double centroid[13];
    double variance[13];
    compute_centroid(anchor_positions, num_anchors, centroid);
    compute_variance(anchor_positions, num_anchors, centroid, variance);
    
    printf("\n--- Anchor Distribution Statistics ---\n");
    printf("Centroid:\n");
    for (int i = 0; i < 13; i++) {
        printf("  Dim %2d: %12.6f\n", i, centroid[i]);
    }
    
    printf("\nVariance per dimension:\n");
    for (int i = 0; i < 13; i++) {
        printf("  Dim %2d: %12.6f (stddev: %12.6f)\n", i, variance[i], sqrt(variance[i]));
    }
    
    // Compute pairwise distances
    printf("\n--- Pairwise Distance Analysis ---\n");
    double min_dist = 1e100;
    double max_dist = 0.0;
    double sum_dist = 0.0;
    int num_pairs = 0;
    
    for (int i = 0; i < num_anchors; i++) {
        for (int j = i + 1; j < num_anchors; j++) {
            double dist = compute_distance_13d(anchor_positions[i], anchor_positions[j]);
            if (dist < min_dist) min_dist = dist;
            if (dist > max_dist) max_dist = dist;
            sum_dist += dist;
            num_pairs++;
        }
    }
    
    double avg_dist = sum_dist / num_pairs;
    printf("Min pairwise distance: %12.6f\n", min_dist);
    printf("Max pairwise distance: %12.6f\n", max_dist);
    printf("Avg pairwise distance: %12.6f\n", avg_dist);
    
    // Now analyze the real k position
    printf("\n=== Real K Position Analysis ===\n");
    
    BIGNUM* real_k = BN_new();
    BN_set_word(real_k, 0x12345678);
    double real_k_pos[13];
    map_k_to_position(real_k, real_k_pos, 13);
    
    char* k_hex = BN_bn2hex(real_k);
    printf("\nReal k value: 0x%s\n", k_hex);
    OPENSSL_free(k_hex);
    
    printf("Real k position in 13D:\n");
    for (int i = 0; i < 13; i++) {
        printf("  Dim %2d: %12.6f\n", i, real_k_pos[i]);
    }
    
    // Find nearest anchors to real k
    printf("\n--- Nearest Anchors to Real K ---\n");
    
    typedef struct {
        int index;
        double distance;
    } AnchorDistance;
    
    AnchorDistance* distances = malloc(num_anchors * sizeof(AnchorDistance));
    if (!distances) {
        fprintf(stderr, "Failed to allocate distances\n");
        BN_free(real_k);
        free(anchor_positions);
        for (int i = 0; i < num_anchors; i++) {
            BN_free(anchors[i]);
        }
        free(anchors);
        BN_CTX_free(bn_ctx);
        return 1;
    }
    
    for (int i = 0; i < num_anchors; i++) {
        distances[i].index = i;
        distances[i].distance = compute_distance_13d(real_k_pos, anchor_positions[i]);
    }
    
    // Sort by distance
    for (int i = 0; i < num_anchors - 1; i++) {
        for (int j = i + 1; j < num_anchors; j++) {
            if (distances[j].distance < distances[i].distance) {
                AnchorDistance temp = distances[i];
                distances[i] = distances[j];
                distances[j] = temp;
            }
        }
    }
    
    printf("Top 10 nearest anchors:\n");
    for (int i = 0; i < 10 && i < num_anchors; i++) {
        int idx = distances[i].index;
        char* anchor_hex = BN_bn2hex(anchors[idx]);
        printf("  %2d. Anchor[%3d] = 0x%s, distance = %12.6f\n",
               i + 1, idx, anchor_hex, distances[i].distance);
        OPENSSL_free(anchor_hex);
    }
    
    // Analyze the 3 nearest anchors (the shared vertices)
    printf("\n--- Shared Vertices Analysis ---\n");
    printf("The 3 nearest anchors form the simplex containing real k:\n");
    
    for (int i = 0; i < 3 && i < num_anchors; i++) {
        int idx = distances[i].index;
        char* anchor_hex = BN_bn2hex(anchors[idx]);
        printf("\nVertex %d:\n", i + 1);
        printf("  Anchor value: 0x%s\n", anchor_hex);
        printf("  Distance to real k: %12.6f\n", distances[i].distance);
        printf("  Position:\n");
        for (int j = 0; j < 13; j++) {
            printf("    Dim %2d: %12.6f\n", j, anchor_positions[idx][j]);
        }
        OPENSSL_free(anchor_hex);
    }
    
    // Compute the centroid of the 3 nearest anchors
    double simplex_centroid[13];
    for (int i = 0; i < 13; i++) {
        simplex_centroid[i] = 0.0;
        for (int j = 0; j < 3 && j < num_anchors; j++) {
            int idx = distances[j].index;
            simplex_centroid[i] += anchor_positions[idx][i];
        }
        simplex_centroid[i] /= 3.0;
    }
    
    double dist_to_simplex_centroid = compute_distance_13d(real_k_pos, simplex_centroid);
    printf("\nSimplex centroid distance to real k: %12.6f\n", dist_to_simplex_centroid);
    
    // Check if real k is inside or outside the simplex
    printf("\n--- Real K Position Relative to Simplex ---\n");
    printf("Comparing real k position to simplex centroid:\n");
    for (int i = 0; i < 13; i++) {
        double diff = real_k_pos[i] - simplex_centroid[i];
        printf("  Dim %2d: real_k = %12.6f, centroid = %12.6f, diff = %12.6f\n",
               i, real_k_pos[i], simplex_centroid[i], diff);
    }
    
    // Analyze the distances between the 3 nearest anchors
    printf("\n--- Distances Between Shared Vertices ---\n");
    for (int i = 0; i < 3 && i < num_anchors; i++) {
        for (int j = i + 1; j < 3 && j < num_anchors; j++) {
            int idx_i = distances[i].index;
            int idx_j = distances[j].index;
            double dist = compute_distance_13d(anchor_positions[idx_i], anchor_positions[idx_j]);
            printf("  Distance between vertex %d and vertex %d: %12.6f\n", i + 1, j + 1, dist);
        }
    }
    
    // Cleanup
    free(distances);
    BN_free(real_k);
    free(anchor_positions);
    for (int i = 0; i < num_anchors; i++) {
        BN_free(anchors[i]);
    }
    free(anchors);
    BN_CTX_free(bn_ctx);
    
    printf("\n=== Analysis Complete ===\n");
    return 0;
}