/**
 * @file test_geometric_1k.c
 * @brief Geometric recovery test with 1,000 anchors
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <openssl/ec.h>
#include <openssl/obj_mac.h>
#include <openssl/bn.h>
#include "../include/geometric_recovery.h"

#define NUM_ANCHORS 1000
#define NUM_TEST_KEYS 10

int main(void) {
    printf("=== GEOMETRIC RECOVERY TEST (1K ANCHORS) ===\n");
    printf("Anchors: %d, Test keys: %d, Dimensions: %d\n\n", 
           NUM_ANCHORS, NUM_TEST_KEYS, GEO_NUM_DIMENSIONS);
    
    clock_t start = clock();
    
    EC_GROUP* curve = EC_GROUP_new_by_curve_name(NID_secp256k1);
    
    printf("Creating context...\n");
    GeometricRecoveryContext* ctx = geometric_recovery_create(
        curve, NUM_ANCHORS, GEO_NUM_DIMENSIONS);
    
    printf("Generating %d anchors...\n", NUM_ANCHORS);
    for (uint32_t i = 0; i < NUM_ANCHORS; i++) {
        BIGNUM* k = BN_new();
        BIGNUM* order = BN_new();
        BN_CTX* bn_ctx = BN_CTX_new();
        
        EC_GROUP_get_order(curve, order, bn_ctx);
        BN_rand_range(k, order);
        
        EC_POINT* Q = EC_POINT_new(curve);
        const EC_POINT* G = EC_GROUP_get0_generator(curve);
        EC_POINT_mul(curve, Q, NULL, G, k, bn_ctx);
        
        geometric_recovery_add_anchor(ctx, k, Q);
        
        BN_free(k);
        BN_free(order);
        EC_POINT_free(Q);
        BN_CTX_free(bn_ctx);
        
        if ((i + 1) % 100 == 0) {
            printf("  Generated %u/%d anchors\n", i + 1, NUM_ANCHORS);
        }
    }
    
    printf("\nInitializing system...\n");
    if (!geometric_recovery_initialize(ctx)) {
        printf("ERROR: Initialization failed\n");
        return 1;
    }
    
    printf("  Tori detected: %u\n", ctx->num_tori);
    printf("  Shared vertices: %u\n", ctx->num_shared_vertices);
    printf("  Shared faces: %u\n", ctx->num_shared_faces);
    printf("  Intersection curves: %u\n", ctx->num_curves);
    printf("  Tetration towers: %u\n", ctx->num_towers);
    
    printf("\nTesting recovery on %d keys...\n", NUM_TEST_KEYS);
    uint32_t recovered = 0;
    double total_distance = 0.0;
    double total_confidence = 0.0;
    
    for (uint32_t i = 0; i < NUM_TEST_KEYS; i++) {
        BIGNUM* k_actual = BN_new();
        BIGNUM* order = BN_new();
        BN_CTX* bn_ctx = BN_CTX_new();
        
        EC_GROUP_get_order(curve, order, bn_ctx);
        BN_rand_range(k_actual, order);
        
        EC_POINT* Q_actual = EC_POINT_new(curve);
        const EC_POINT* G = EC_GROUP_get0_generator(curve);
        EC_POINT_mul(curve, Q_actual, NULL, G, k_actual, bn_ctx);
        
        double confidence = 0.0;
        BIGNUM* k_recovered = geometric_recovery_recover_k(ctx, Q_actual, &confidence);
        
        if (k_recovered) {
            double distance = compute_k_hamming_distance(k_recovered, k_actual);
            total_distance += distance;
            total_confidence += confidence;
            
            if (distance < 0.1) {
                recovered++;
                printf("  Key %u: ✓ RECOVERED (dist=%.4f, conf=%.4f)\n", i, distance, confidence);
            } else {
                printf("  Key %u: ✗ FAILED (dist=%.4f, conf=%.4f)\n", i, distance, confidence);
            }
            
            BN_free(k_recovered);
        } else {
            printf("  Key %u: ✗ NO CANDIDATE\n", i);
            total_distance += 1.0;
        }
        
        BN_free(k_actual);
        BN_free(order);
        EC_POINT_free(Q_actual);
        BN_CTX_free(bn_ctx);
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("\n=== RESULTS ===\n");
    printf("Recovery rate: %u/%d (%.1f%%)\n", recovered, NUM_TEST_KEYS,
           (double)recovered / NUM_TEST_KEYS * 100.0);
    printf("Avg Hamming distance: %.4f\n", total_distance / NUM_TEST_KEYS);
    printf("Avg confidence: %.4f\n", total_confidence / NUM_TEST_KEYS);
    printf("Time: %.3f seconds\n", elapsed);
    
    printf("\n=== TARGET ===\n");
    printf("Phase 1 (52D, 10K anchors): 30-50%% recovery\n");
    printf("Current (52D, 1K anchors): %.1f%%\n", (double)recovered / NUM_TEST_KEYS * 100.0);
    
    if (recovered >= NUM_TEST_KEYS * 0.3) {
        printf("\n✅ TARGET ACHIEVED!\n");
    } else {
        printf("\n⚠️  Below target - need more anchors or parameter tuning\n");
    }
    
    geometric_recovery_free(ctx);
    EC_GROUP_free(curve);
    
    return 0;
}
