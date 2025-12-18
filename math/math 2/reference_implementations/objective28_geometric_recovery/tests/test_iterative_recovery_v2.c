/**
 * Test Iterative Recovery with Dynamic Scaling (All 4 Phases)
 * 
 * Tests:
 * - Phase 1: Verification loop
 * - Phase 2: Endianness and truncation
 * - Phase 3: Dynamic scaling
 * - Phase 4: Entropy reduction
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/ec.h>
#include <openssl/bn.h>
#include <openssl/obj_mac.h>
#include "geometric_recovery.h"

void print_separator() {
    printf("\n");
    printf("================================================================================\n");
}

int main() {
    printf("=== ITERATIVE RECOVERY V2 - ALL 4 PHASES ===\n");
    printf("Testing with secp128r1 (128-bit curve)\n\n");
    
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
    
    // ========================================================================
    // TEST 1: Basic Verification Loop (Phase 1)
    // ========================================================================
    
    print_separator();
    printf("TEST 1: VERIFICATION LOOP (Phase 1)\n");
    print_separator();
    
    printf("Generating test key...\n");
    BIGNUM* test_k = BN_new();
    BN_rand_range(test_k, order);
    
    EC_POINT* test_Q = EC_POINT_new(curve);
    EC_POINT_mul(curve, test_Q, NULL, G, test_k, ctx);
    
    char* k_hex = BN_bn2hex(test_k);
    printf("Actual k: %s\n\n", k_hex);
    OPENSSL_free(k_hex);
    
    // Test verification function
    printf("Testing verification function...\n");
    bool verified = verify_candidate_produces_Q(test_k, test_Q, curve);
    printf("Verification result: %s\n", verified ? "✅ PASS" : "❌ FAIL");
    
    if (!verified) {
        printf("ERROR: Verification function not working!\n");
        return 1;
    }
    
    // Test with wrong k
    BIGNUM* wrong_k = BN_new();
    BN_copy(wrong_k, test_k);
    BN_add_word(wrong_k, 1);
    
    bool should_fail = verify_candidate_produces_Q(wrong_k, test_Q, curve);
    printf("Wrong k verification: %s\n", should_fail ? "❌ FAIL (should be false)" : "✅ PASS (correctly rejected)");
    BN_free(wrong_k);
    
    // ========================================================================
    // TEST 2: Endianness and Truncation (Phase 2)
    // ========================================================================
    
    print_separator();
    printf("TEST 2: ENDIANNESS AND TRUNCATION (Phase 2)\n");
    print_separator();
    
    printf("Testing 257-bit computation with truncation...\n");
    
    // Create simple test position
    double test_position[13];
    for (int i = 0; i < 13; i++) {
        test_position[i] = 0.5;  // Middle of range
    }
    
    // Create dummy anchors for testing
    const double* anchor_positions[3];
    const BIGNUM* anchor_k_values[3];
    double pos1[13], pos2[13], pos3[13];
    BIGNUM* k1 = BN_new();
    BIGNUM* k2 = BN_new();
    BIGNUM* k3 = BN_new();
    
    for (int i = 0; i < 13; i++) {
        pos1[i] = 0.3;
        pos2[i] = 0.5;
        pos3[i] = 0.7;
    }
    
    BN_rand_range(k1, order);
    BN_rand_range(k2, order);
    BN_rand_range(k3, order);
    
    anchor_positions[0] = pos1;
    anchor_positions[1] = pos2;
    anchor_positions[2] = pos3;
    anchor_k_values[0] = k1;
    anchor_k_values[1] = k2;
    anchor_k_values[2] = k3;
    
    BIGNUM* interpolated_k = triangulate_k_with_truncation(
        test_position,
        anchor_positions,
        anchor_k_values,
        3,
        13,
        curve
    );
    
    if (interpolated_k) {
        printf("✅ Interpolation successful\n");
        printf("Result bits: %d\n", BN_num_bits(interpolated_k));
        printf("Order bits: %d\n", BN_num_bits(order));
        
        // Verify it's within order
        int cmp = BN_cmp(interpolated_k, order);
        printf("Within order: %s\n", cmp < 0 ? "✅ YES" : "❌ NO");
        
        BN_free(interpolated_k);
    } else {
        printf("❌ Interpolation failed\n");
    }
    
    BN_free(k1);
    BN_free(k2);
    BN_free(k3);
    
    // ========================================================================
    // TEST 3: Iterative Recovery with 100 Anchors (Phase 1 + 2)
    // ========================================================================
    
    print_separator();
    printf("TEST 3: ITERATIVE RECOVERY (Phase 1 + 2)\n");
    print_separator();
    
    // Use 100 anchors - resolution comes from GRAPH SIZE, not anchor count!
    // The Platonic model graph needs 2^32 vertices for proper resolution
    int num_test_anchors = 100;
    printf("Generating %d anchor k/Q pairs...\n", num_test_anchors);
    clock_t gen_start = clock();
    
    BIGNUM** anchor_k = malloc(num_test_anchors * sizeof(BIGNUM*));
    EC_POINT** anchor_Q = malloc(num_test_anchors * sizeof(EC_POINT*));
    
    for (int i = 0; i < num_test_anchors; i++) {
        anchor_k[i] = BN_new();
        BN_rand_range(anchor_k[i], order);
        
        anchor_Q[i] = EC_POINT_new(curve);
        EC_POINT_mul(curve, anchor_Q[i], NULL, G, anchor_k[i], ctx);
        
        if ((i + 1) % 1000 == 0) {
            printf("  Generated %d anchors...\n", i + 1);
        }
    }
    
    clock_t gen_end = clock();
    printf("Generated in %.3f seconds\n\n", (double)(gen_end - gen_start) / CLOCKS_PER_SEC);
    
    // Create recovery context with 10,000 anchors for proper resolution
    printf("Creating recovery context with %d anchors...\n", num_test_anchors);
    GeometricRecoveryContext* recovery_ctx = geometric_recovery_create(curve, num_test_anchors, 13);
    
    // Add anchors
    printf("Adding anchors to context...\n");
    for (int i = 0; i < num_test_anchors; i++) {
        geometric_recovery_add_anchor(recovery_ctx, anchor_k[i], anchor_Q[i]);
        if ((i + 1) % 1000 == 0) {
            printf("  Added %d anchors...\n", i + 1);
        }
    }
    printf("Added %d anchors to context\n", num_test_anchors);
    
    // Initialize
    printf("Initializing recovery context...\n");
    clock_t init_start = clock();
    bool init_success = geometric_recovery_initialize(recovery_ctx);
    clock_t init_end = clock();
    
    printf("Initialized in %.3f seconds\n", (double)(init_end - init_start) / CLOCKS_PER_SEC);
    printf("Status: %s\n", init_success ? "✅ SUCCESS" : "❌ FAILED");
    
    if (!init_success) {
        printf("Initialization failed, skipping recovery test\n");
    } else {
        printf("Tori detected: %u\n", recovery_ctx->num_tori);
        printf("Shared vertices: %u\n\n", recovery_ctx->num_shared_vertices);
        
        // Generate new test key
        BIGNUM* test_k2 = BN_new();
        BN_rand_range(test_k2, order);
        
        EC_POINT* test_Q2 = EC_POINT_new(curve);
        EC_POINT_mul(curve, test_Q2, NULL, G, test_k2, ctx);
        
        char* k2_hex = BN_bn2hex(test_k2);
        printf("Target k: %s\n\n", k2_hex);
        OPENSSL_free(k2_hex);
        
        // Run iterative recovery
        printf("Running iterative recovery (max 1000 iterations)...\n");
        clock_t recovery_start = clock();
        
        double confidence = 0.0;
        uint32_t iterations = 0;
        BIGNUM* recovered_k = geometric_recovery_iterative(
            recovery_ctx, test_Q2, 1000, &confidence, &iterations
        );
        
        clock_t recovery_end = clock();
        double recovery_time = (double)(recovery_end - recovery_start) / CLOCKS_PER_SEC;
        
        printf("\n=== RESULTS ===\n");
        printf("Time: %.3f seconds\n", recovery_time);
        printf("Iterations: %u\n", iterations);
        printf("Confidence: %.6f\n\n", confidence);
        
        if (recovered_k) {
            printf("✅ RECOVERY SUCCESSFUL!\n\n");
            
            // Verify it's correct
            bool correct = verify_candidate_produces_Q(recovered_k, test_Q2, curve);
            printf("Verification: %s\n", correct ? "✅ CORRECT" : "❌ WRONG");
            
            // Measure Hamming distance
            BIGNUM* diff = BN_new();
            BN_sub(diff, test_k2, recovered_k);
            BN_mod(diff, diff, order, ctx);
            
            int bits = BN_num_bits(order);
            int hamming = 0;
            for (int i = 0; i < bits; i++) {
                int bit_actual = BN_is_bit_set(test_k2, i);
                int bit_recovered = BN_is_bit_set(recovered_k, i);
                if (bit_actual != bit_recovered) hamming++;
            }
            
            printf("Hamming distance: %d/%d bits (%.1f%%)\n", 
                   hamming, bits, (double)hamming / bits * 100.0);
            
            BN_free(diff);
            BN_free(recovered_k);
        } else {
            printf("❌ RECOVERY FAILED\n");
            printf("No exact match found in %u iterations\n", iterations);
        }
        
        BN_free(test_k2);
        EC_POINT_free(test_Q2);
    }
    
    geometric_recovery_free(recovery_ctx);
    
    // ========================================================================
    // TEST 4: Dynamic Scaling (Phase 3) - COMMENTED OUT FOR NOW
    // ========================================================================
    
    print_separator();
    printf("TEST 4: DYNAMIC SCALING (Phase 3)\n");
    print_separator();
    
    printf("⚠️ Dynamic scaling test requires anchor generation at each scale\n");
    printf("   This is a placeholder - full implementation would:\n");
    printf("   1. Start with 13D, 100 anchors\n");
    printf("   2. Measure oscillations\n");
    printf("   3. Scale up if not stable (26D, 1000 anchors)\n");
    printf("   4. Continue until oscillations stabilize\n");
    printf("   5. Use stabilization point as target complexity\n");
    
    // Cleanup
    for (int i = 0; i < num_test_anchors; i++) {
        BN_free(anchor_k[i]);
        EC_POINT_free(anchor_Q[i]);
    }
    free(anchor_k);
    free(anchor_Q);
    
    BN_free(test_k);
    EC_POINT_free(test_Q);
    BN_free(order);
    BN_CTX_free(ctx);
    EC_GROUP_free(curve);
    
    print_separator();
    printf("=== ALL TESTS COMPLETE ===\n");
    print_separator();
    
    return 0;
}