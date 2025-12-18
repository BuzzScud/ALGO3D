#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/ec.h>
#include <openssl/bn.h>
#include <openssl/obj_mac.h>
#include "geometric_recovery.h"

// Test with 100 anchors - validate if single candidate is correct
int main() {
    printf("=== SINGLE CANDIDATE VALIDATION TEST ===\n");
    printf("Testing if geometric reduction produces CORRECT candidate\n\n");
    
    // Use secp128r1 for fast testing
    EC_GROUP* curve = EC_GROUP_new_by_curve_name(NID_secp128r1);
    if (!curve) {
        fprintf(stderr, "Failed to create curve\n");
        return 1;
    }
    
    const EC_POINT* G = EC_GROUP_get0_generator(curve);
    BIGNUM* order = BN_new();
    BN_CTX* ctx = BN_CTX_new();
    EC_GROUP_get_order(curve, order, ctx);
    
    // Create recovery context (curve, num_anchors, num_dimensions)
    GeometricRecoveryContext* recovery_ctx = geometric_recovery_create(curve, 100, 13);
    if (!recovery_ctx) {
        fprintf(stderr, "Failed to create recovery context\n");
        return 1;
    }
    
    // Generate 100 known k/Q pairs as anchors
    printf("Generating 100 anchor k/Q pairs...\n");
    clock_t gen_start = clock();
    
    BIGNUM** anchor_k = malloc(100 * sizeof(BIGNUM*));
    EC_POINT** anchor_Q = malloc(100 * sizeof(EC_POINT*));
    
    for (int i = 0; i < 100; i++) {
        anchor_k[i] = BN_new();
        BN_rand_range(anchor_k[i], order);
        
        anchor_Q[i] = EC_POINT_new(curve);
        EC_POINT_mul(curve, anchor_Q[i], NULL, G, anchor_k[i], ctx);
        
        // Add to recovery context
        geometric_recovery_add_anchor(recovery_ctx, anchor_k[i], anchor_Q[i]);
    }
    
    clock_t gen_end = clock();
    printf("Generated in %.3f seconds\n\n", (double)(gen_end - gen_start) / CLOCKS_PER_SEC);
    
    // Initialize recovery (detect tori, find intersections)
    printf("Initializing recovery (detecting tori, finding intersections)...\n");
    clock_t init_start = clock();
    
    bool init_success = geometric_recovery_initialize(recovery_ctx);
    
    clock_t init_end = clock();
    printf("Initialized in %.3f seconds\n", (double)(init_end - init_start) / CLOCKS_PER_SEC);
    
    if (!init_success) {
        printf("❌ Initialization failed\n");
        geometric_recovery_free(recovery_ctx);
        return 1;
    }
    
    printf("✅ Initialization successful\n\n");
    
    // Generate 1 test key (unknown k, known Q)
    printf("Generating 1 test key...\n");
    BIGNUM* test_k_actual = BN_new();
    BN_rand_range(test_k_actual, order);
    
    EC_POINT* test_Q = EC_POINT_new(curve);
    EC_POINT_mul(curve, test_Q, NULL, G, test_k_actual, ctx);
    
    // Print actual k for reference
    char* k_hex = BN_bn2hex(test_k_actual);
    printf("Actual k: %s\n\n", k_hex);
    OPENSSL_free(k_hex);
    
    // Run geometric recovery
    printf("Running geometric recovery...\n");
    clock_t recovery_start = clock();
    
    double confidence = 0.0;
    BIGNUM* recovered_k = geometric_recovery_recover_k(recovery_ctx, test_Q, &confidence);
    
    clock_t recovery_end = clock();
    double recovery_time = (double)(recovery_end - recovery_start) / CLOCKS_PER_SEC;
    
    printf("Recovery completed in %.3f seconds\n", recovery_time);
    printf("Confidence: %.6f\n\n", confidence);
    
    // Analyze results
    printf("=== RESULTS ===\n");
    
    if (!recovered_k) {
        printf("❌ NO CANDIDATE GENERATED\n");
        printf("Algorithm failed to reduce search space\n");
        
        geometric_recovery_free(recovery_ctx);
        return 1;
    }
    
    printf("✅ REDUCED TO SINGLE CANDIDATE!\n\n");
    
    // Validate: Is this candidate correct?
    EC_POINT* candidate_Q = EC_POINT_new(curve);
    EC_POINT_mul(curve, candidate_Q, NULL, G, recovered_k, ctx);
    
    // Compare with actual Q
    int match = EC_POINT_cmp(curve, candidate_Q, test_Q, ctx);
    
    if (match == 0) {
        printf("🎉 CANDIDATE IS CORRECT! 🎉\n");
        printf("recovered_k * G == test_Q\n\n");
        
        // Measure distance (should be 0)
        BIGNUM* diff = BN_new();
        BN_sub(diff, test_k_actual, recovered_k);
        BN_mod(diff, diff, order, ctx);
        
        char* diff_hex = BN_bn2hex(diff);
        printf("Distance: %s\n", diff_hex);
        printf("(should be 0)\n\n");
        
        printf("=== SUCCESS ===\n");
        printf("✅ Geometric reduction works perfectly!\n");
        printf("✅ Single candidate is the correct k value!\n");
        
        OPENSSL_free(diff_hex);
        BN_free(diff);
        
    } else {
        printf("❌ CANDIDATE IS WRONG!\n\n");
        
        // Measure how far off
        BIGNUM* diff = BN_new();
        BN_sub(diff, test_k_actual, recovered_k);
        BN_mod(diff, diff, order, ctx);
        
        char* actual_hex = BN_bn2hex(test_k_actual);
        char* candidate_hex = BN_bn2hex(recovered_k);
        char* diff_hex = BN_bn2hex(diff);
        
        printf("Actual k:    %s\n", actual_hex);
        printf("Candidate k: %s\n", candidate_hex);
        printf("Difference:  %s\n\n", diff_hex);
        
        // Compute Hamming distance
        int bits = BN_num_bits(order);
        int hamming = 0;
        for (int i = 0; i < bits; i++) {
            int bit_actual = BN_is_bit_set(test_k_actual, i);
            int bit_candidate = BN_is_bit_set(recovered_k, i);
            if (bit_actual != bit_candidate) hamming++;
        }
        
        printf("Hamming distance: %d/%d bits (%.1f%%)\n", 
               hamming, bits, (double)hamming / bits * 100.0);
        
        // Determine polarity
        int cmp = BN_cmp(recovered_k, test_k_actual);
        if (cmp > 0) {
            printf("Polarity: POSITIVE (candidate > actual)\n");
        } else if (cmp < 0) {
            printf("Polarity: NEGATIVE (candidate < actual)\n");
        } else {
            printf("Polarity: ZERO (should not happen)\n");
        }
        
        printf("\n=== ANALYSIS ===\n");
        printf("The algorithm successfully reduced to a single candidate,\n");
        printf("but the candidate is incorrect. This means:\n");
        printf("1. Torus detection is working (reduced search space)\n");
        printf("2. Intersection finding is working (found candidate)\n");
        printf("3. BUT: The geometric mapping is off by %.1f%%\n", (double)hamming / bits * 100.0);
        printf("\nNeed to debug:\n");
        printf("- Torus center calculation\n");
        printf("- Intersection mathematics\n");
        printf("- Triangulation from anchors\n");
        
        OPENSSL_free(actual_hex);
        OPENSSL_free(candidate_hex);
        OPENSSL_free(diff_hex);
        BN_free(diff);
    }
    
    EC_POINT_free(candidate_Q);
    
    // Cleanup
    for (int i = 0; i < 100; i++) {
        BN_free(anchor_k[i]);
        EC_POINT_free(anchor_Q[i]);
    }
    free(anchor_k);
    free(anchor_Q);
    
    BN_free(test_k_actual);
    EC_POINT_free(test_Q);
    BN_free(order);
    BN_CTX_free(ctx);
    EC_GROUP_free(curve);
    
    geometric_recovery_free(recovery_ctx);
    
    return 0;
}
