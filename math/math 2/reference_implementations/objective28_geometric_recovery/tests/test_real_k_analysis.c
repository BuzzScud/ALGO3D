#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/obj_mac.h>
#include "../include/geometric_recovery.h"

int main(void) {
    printf("=== REAL K POSITION ANALYSIS ===\n\n");
    
    // Initialize curve
    EC_GROUP* curve = EC_GROUP_new_by_curve_name(NID_secp128r1);
    if (!curve) {
        fprintf(stderr, "Failed to create curve\n");
        return 1;
    }
    
    BN_CTX* bn_ctx = BN_CTX_new();
    BIGNUM* order = BN_new();
    EC_GROUP_get_order(curve, order, bn_ctx);
    const EC_POINT* G = EC_GROUP_get0_generator(curve);
    
    // Generate test k and Q
    BIGNUM* real_k = BN_new();
    BN_rand_range(real_k, order);
    
    EC_POINT* real_Q = EC_POINT_new(curve);
    EC_POINT_mul(curve, real_Q, real_k, NULL, NULL, bn_ctx);
    
    char* k_hex = BN_bn2hex(real_k);
    printf("Real k: %s\n\n", k_hex);
    OPENSSL_free(k_hex);
    
    // Generate 100 anchors
    printf("Generating 100 anchors...\n");
    BIGNUM** anchor_k = malloc(100 * sizeof(BIGNUM*));
    EC_POINT** anchor_Q = malloc(100 * sizeof(EC_POINT*));
    
    for (int i = 0; i < 100; i++) {
        anchor_k[i] = BN_new();
        BN_rand_range(anchor_k[i], order);
        anchor_Q[i] = EC_POINT_new(curve);
        EC_POINT_mul(curve, anchor_Q[i], anchor_k[i], NULL, NULL, bn_ctx);
    }
    
    // Create recovery context with high resolution
    printf("Creating recovery context (2^24 vertices)...\n");
    GeometricRecoveryContext* ctx = geometric_recovery_create(curve, 100, 13);
    
    for (int i = 0; i < 100; i++) {
        geometric_recovery_add_anchor(ctx, anchor_k[i], anchor_Q[i]);
    }
    
    printf("Initializing...\n");
    geometric_recovery_initialize(ctx);
    printf("Initialized!\n\n");
    
    // Map real k to position
    double real_k_position[13];
    map_k_to_position(real_k, real_k_position, 13);
    
    printf("=== REAL K POSITION IN 13D ===\n");
    for (int d = 0; d < 13; d++) {
        printf("Dim %2d: %.10f\n", d, real_k_position[d]);
    }
    
    // Find nearest anchors to real k
    printf("\n=== FINDING NEAREST ANCHORS TO REAL K ===\n");
    
    typedef struct {
        int index;
        double distance;
        double position[13];
    } AnchorInfo;
    
    AnchorInfo* anchor_info = malloc(100 * sizeof(AnchorInfo));
    
    for (int i = 0; i < 100; i++) {
        anchor_info[i].index = i;
        
        // Compute distance
        double dist = 0.0;
        for (int d = 0; d < 13; d++) {
            double diff = real_k_position[d] - ctx->anchor_k_positions[i][d];
            dist += diff * diff;
            anchor_info[i].position[d] = ctx->anchor_k_positions[i][d];
        }
        anchor_info[i].distance = sqrt(dist);
    }
    
    // Sort by distance
    for (int i = 0; i < 99; i++) {
        for (int j = i + 1; j < 100; j++) {
            if (anchor_info[j].distance < anchor_info[i].distance) {
                AnchorInfo temp = anchor_info[i];
                anchor_info[i] = anchor_info[j];
                anchor_info[j] = temp;
            }
        }
    }
    
    printf("\nNearest 10 anchors:\n");
    for (int i = 0; i < 10; i++) {
        char* anchor_hex = BN_bn2hex(anchor_k[anchor_info[i].index]);
        printf("%2d. Anchor[%3d] = %s\n", i+1, anchor_info[i].index, anchor_hex);
        printf("    Distance: %.10f\n", anchor_info[i].distance);
        OPENSSL_free(anchor_hex);
    }
    
    // Analyze the 3 nearest (the partition boundaries)
    printf("\n=== PARTITION ANALYSIS (3 Nearest Anchors) ===\n");
    
    for (int i = 0; i < 3; i++) {
        char* anchor_hex = BN_bn2hex(anchor_k[anchor_info[i].index]);
        printf("\nVertex %d: %s\n", i+1, anchor_hex);
        printf("Distance to real k: %.10f\n", anchor_info[i].distance);
        printf("Position:\n");
        for (int d = 0; d < 13; d++) {
            printf("  Dim %2d: %.10f (diff from real k: %+.10f)\n", 
                   d, anchor_info[i].position[d], 
                   anchor_info[i].position[d] - real_k_position[d]);
        }
        OPENSSL_free(anchor_hex);
    }
    
    // Compute distances between the 3 vertices
    printf("\n=== DISTANCES BETWEEN PARTITION VERTICES ===\n");
    for (int i = 0; i < 3; i++) {
        for (int j = i + 1; j < 3; j++) {
            double dist = 0.0;
            for (int d = 0; d < 13; d++) {
                double diff = anchor_info[i].position[d] - anchor_info[j].position[d];
                dist += diff * diff;
            }
            dist = sqrt(dist);
            printf("Distance between vertex %d and vertex %d: %.10f\n", i+1, j+1, dist);
        }
    }
    
    // Compute centroid of the 3 vertices
    printf("\n=== PARTITION CENTROID ===\n");
    double centroid[13];
    for (int d = 0; d < 13; d++) {
        centroid[d] = (anchor_info[0].position[d] + 
                       anchor_info[1].position[d] + 
                       anchor_info[2].position[d]) / 3.0;
    }
    
    double centroid_dist = 0.0;
    for (int d = 0; d < 13; d++) {
        double diff = real_k_position[d] - centroid[d];
        centroid_dist += diff * diff;
    }
    centroid_dist = sqrt(centroid_dist);
    
    printf("Centroid position:\n");
    for (int d = 0; d < 13; d++) {
        printf("  Dim %2d: %.10f (diff from real k: %+.10f)\n", 
               d, centroid[d], centroid[d] - real_k_position[d]);
    }
    printf("Distance from real k to centroid: %.10f\n", centroid_dist);
    
    // Check if real k is inside or outside the simplex
    printf("\n=== IS REAL K INSIDE THE PARTITION? ===\n");
    
    // Compute barycentric coordinates
    // If all coordinates are positive and sum to 1, point is inside
    double bary[3];
    double total_area = 0.0;
    
    // This is a simplified 2D barycentric calculation
    // For proper 13D, we'd need more complex math
    printf("(Simplified analysis - full 13D barycentric calculation needed)\n");
    
    // Instead, check if real k is closer to centroid than to any vertex
    bool inside = true;
    for (int i = 0; i < 3; i++) {
        if (anchor_info[i].distance < centroid_dist) {
            inside = false;
            break;
        }
    }
    
    if (inside) {
        printf("✓ Real k appears to be INSIDE the partition (closer to centroid than vertices)\n");
    } else {
        printf("✗ Real k appears to be OUTSIDE the partition (closer to a vertex than centroid)\n");
    }
    
    // Analyze vectors from centroid to vertices
    printf("\n=== VECTOR ANALYSIS ===\n");
    printf("Vectors from centroid to each vertex:\n");
    
    for (int i = 0; i < 3; i++) {
        printf("\nVertex %d vector:\n", i+1);
        double magnitude = 0.0;
        for (int d = 0; d < 13; d++) {
            double component = anchor_info[i].position[d] - centroid[d];
            magnitude += component * component;
            printf("  Dim %2d: %+.10f\n", d, component);
        }
        magnitude = sqrt(magnitude);
        printf("  Magnitude: %.10f\n", magnitude);
    }
    
    // Vector from centroid to real k
    printf("\nVector from centroid to real k:\n");
    double real_k_magnitude = 0.0;
    for (int d = 0; d < 13; d++) {
        double component = real_k_position[d] - centroid[d];
        real_k_magnitude += component * component;
        printf("  Dim %2d: %+.10f\n", d, component);
    }
    real_k_magnitude = sqrt(real_k_magnitude);
    printf("  Magnitude: %.10f\n", real_k_magnitude);
    
    // Check if distances are uniform (indicating partition boundaries)
    printf("\n=== UNIFORM DISTANCE CHECK ===\n");
    double avg_vertex_dist = (anchor_info[0].distance + 
                              anchor_info[1].distance + 
                              anchor_info[2].distance) / 3.0;
    
    printf("Average distance to vertices: %.10f\n", avg_vertex_dist);
    printf("Standard deviation: ");
    
    double variance = 0.0;
    for (int i = 0; i < 3; i++) {
        double diff = anchor_info[i].distance - avg_vertex_dist;
        variance += diff * diff;
    }
    variance /= 3.0;
    double stddev = sqrt(variance);
    printf("%.10f\n", stddev);
    
    if (stddev < 0.01) {
        printf("✓ Distances are UNIFORM - vertices likely define partition boundaries\n");
    } else {
        printf("✗ Distances are NOT uniform - vertices may not be partition boundaries\n");
    }
    
    // Cleanup
    free(anchor_info);
    for (int i = 0; i < 100; i++) {
        BN_free(anchor_k[i]);
        EC_POINT_free(anchor_Q[i]);
    }
    free(anchor_k);
    free(anchor_Q);
    BN_free(real_k);
    EC_POINT_free(real_Q);
    BN_free(order);
    BN_CTX_free(bn_ctx);
    geometric_recovery_free(ctx);
    EC_GROUP_free(curve);
    
    printf("\n=== ANALYSIS COMPLETE ===\n");
    return 0;
}