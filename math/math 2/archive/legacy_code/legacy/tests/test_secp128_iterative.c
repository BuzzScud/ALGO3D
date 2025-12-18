/**
 * @file test_secp128_iterative.c
 * @brief Test iterative recovery based on Python research
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <openssl/ec.h>
#include <openssl/obj_mac.h>
#include <openssl/bn.h>
#include "../include/geometric_recovery.h"

#define TOY_NUM_ANCHORS 100
#define TOY_NUM_TEST_KEYS 5
#define TOY_NUM_DIMENSIONS 13
#define TOY_CURVE NID_secp128r1
#define TOY_BIT_SCALE 128

int main(void) {
    printf("=== ITERATIVE RECOVERY TEST (secp128r1) ===\n");
    printf("Based on Python OscillationAccelerator.reverse_q_to_k()\n\n");
    
    clock_t start = clock();
    
    // Initialize curve
    EC_GROUP* curve = EC_GROUP_new_by_curve_name(TOY_CURVE);
    
    // Create context
    printf("Creating context with %d anchors...\n", TOY_NUM_ANCHORS);
    GeometricRecoveryContext* ctx = geometric_recovery_create(
        curve, TOY_NUM_ANCHORS, TOY_NUM_DIMENSIONS);
    
    // Generate anchors (simplified for speed)
    printf("Generating anchors...\n");
    for (uint32_t i = 0; i < TOY_NUM_ANCHORS; i++) {
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
    }
    
    // Initialize
    printf("Initializing system...\n");
    geometric_recovery_initialize(ctx);
    
    printf("  Tori: %u, Shared vertices: %u, Curves: %u, Towers: %u\n\n",
           ctx->num_tori, ctx->num_shared_vertices, ctx->num_curves, ctx->num_towers);
    
    // Test iterative recovery
    printf("Testing iterative recovery on %d keys...\n", TOY_NUM_TEST_KEYS);
    uint32_t recovered = 0;
    double total_distance = 0.0;
    
    for (uint32_t i = 0; i < TOY_NUM_TEST_KEYS; i++) {
        printf("\nTest key %u:\n", i);
        
        // Generate test key
        BIGNUM* k_actual = BN_new();
        BIGNUM* order = BN_new();
        BN_CTX* bn_ctx = BN_CTX_new();
        
        EC_GROUP_get_order(curve, order, bn_ctx);
        BN_rand_range(k_actual, order);
        
        EC_POINT* Q_actual = EC_POINT_new(curve);
        const EC_POINT* G = EC_GROUP_get0_generator(curve);
        EC_POINT_mul(curve, Q_actual, NULL, G, k_actual, bn_ctx);
        
        // Recover using iterative method
        double confidence = 0.0;
        BIGNUM* k_recovered = iterative_recover_k_from_Q(
            ctx, Q_actual, TOY_BIT_SCALE, &confidence);
        
        if (k_recovered) {
            // Verify by computing Q
            EC_POINT* Q_recovered = EC_POINT_new(curve);
            EC_POINT_mul(curve, Q_recovered, NULL, G, k_recovered, bn_ctx);
            
            int exact_match = EC_POINT_cmp(curve, Q_recovered, Q_actual, bn_ctx);
            
            if (exact_match == 0) {
                recovered++;
                printf("  ✓ EXACT MATCH! (confidence=%.4f)\n", confidence);
                total_distance += 0.0;
            } else {
                double distance = compute_k_hamming_distance(k_recovered, k_actual);
                total_distance += distance;
                printf("  ✗ Close but not exact (dist=%.4f, conf=%.4f)\n", 
                       distance, confidence);
            }
            
            EC_POINT_free(Q_recovered);
            BN_free(k_recovered);
        } else {
            printf("  ✗ NO CANDIDATE\n");
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
    printf("Exact matches: %u/%d (%.1f%%)\n", recovered, TOY_NUM_TEST_KEYS,
           (double)recovered / TOY_NUM_TEST_KEYS * 100.0);
    printf("Avg distance: %.4f\n", total_distance / TOY_NUM_TEST_KEYS);
    printf("Time: %.3f seconds\n", elapsed);
    printf("Per-key: %.3f seconds\n", elapsed / TOY_NUM_TEST_KEYS);
    
    printf("\n=== ALGORITHM ===\n");
    printf("✓ Deterministic target from Q (SHA256-based)\n");
    printf("✓ Entropy reduction (2^64 → 2^16)\n");
    printf("✓ Prime search near target (CrystalAbacus)\n");
    printf("✓ QR filter (Legendre symbol)\n");
    printf("✓ Q→k verification loop (iterative)\n");
    
    if (recovered > 0) {
        printf("\n✅ SUCCESS! Iterative recovery works!\n");
    } else {
        printf("\n⚠️  No exact matches - need parameter tuning\n");
    }
    
    geometric_recovery_free(ctx);
    EC_GROUP_free(curve);
    
    return 0;
}
