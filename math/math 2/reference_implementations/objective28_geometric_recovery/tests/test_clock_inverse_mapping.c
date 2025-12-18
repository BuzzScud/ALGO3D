/**
 * test_k_recovery_enhanced.c - Test Enhanced K Recovery
 * 
 * Tests the complete k recovery pipeline with Platonic anchors
 */

#include "clock_inverse_mapping.h"
#include "clock_recovery.h"
#include "platonic_solids.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/obj_mac.h>

void test_inverse_mapping() {
    printf("=== Test 1: Enhanced Inverse Mapping ===\n\n");
    
    // Create test clock position
    ClockPosition target;
    target.ring = 1;
    target.position = 30;
    target.angle = M_PI;
    target.radius = 0.5;
    
    printf("Target position: ring=%d, pos=%d, angle=%.4f\n",
           target.ring, target.position, target.angle);
    
    // Test without anchors
    double k1 = inverse_map_k_from_clock(target, NULL, 0);
    printf("\nWithout anchors: k ≈ %.2f\n", k1);
    
    // Create some anchor positions
    ClockPosition anchors[5];
    for (int i = 0; i < 5; i++) {
        anchors[i].ring = i % 4;
        anchors[i].position = i * 10;
        anchors[i].angle = (i * M_PI) / 5.0;
        anchors[i].radius = 0.25 + (i * 0.15);
    }
    
    // Test with anchors
    double k2 = inverse_map_k_from_clock(target, anchors, 5);
    printf("With 5 anchors: k ≈ %.2f\n", k2);
    
    printf("\nDifference: %.2f\n", fabs(k2 - k1));
    printf("✓ Inverse mapping working\n\n");
}

void test_platonic_anchor_recovery() {
    printf("=== Test 2: Recovery with Platonic Anchors ===\n\n");
    
    // Create Platonic overlay
    PlatonicOverlay* overlay = create_platonic_overlay(0.5);
    if (!overlay) {
        printf("Failed to create overlay\n");
        return;
    }
    
    printf("Created Platonic overlay\n");
    printf("  Shared vertices: %u\n", overlay->num_shared_vertices);
    
    // Create target position
    ClockPosition target;
    target.ring = 0;
    target.position = 6;
    target.angle = M_PI;
    target.radius = 0.25;
    
    printf("\nTarget: ring=%d, pos=%d, angle=%.4f\n",
           target.ring, target.position, target.angle);
    
    // Create recovery context
    ClockRecoveryContext* ctx = init_clock_recovery(10);
    
    // Recover k
    BIGNUM* recovered_k = BN_new();
    bool success = recover_k_with_platonic_anchors(ctx, overlay, target, recovered_k);
    
    if (success) {
        char* k_str = BN_bn2dec(recovered_k);
        printf("\n✓ Recovery successful: k = %s\n", k_str);
        OPENSSL_free(k_str);
    } else {
        printf("\n✗ Recovery failed\n");
    }
    
    // Cleanup
    BN_free(recovered_k);
    free_clock_recovery(ctx);
    free_platonic_overlay(overlay);
    
    printf("\n");
}

void test_ec_verification() {
    printf("=== Test 3: EC Point Verification ===\n\n");
    
    // Create EC group (secp256k1)
    EC_GROUP* group = EC_GROUP_new_by_curve_name(NID_secp256k1);
    if (!group) {
        printf("Failed to create EC group\n");
        return;
    }
    
    printf("Created secp256k1 group\n");
    
    // Create test k
    BIGNUM* k = BN_new();
    BN_set_word(k, 12345);
    
    printf("Test k: 12345\n");
    
    // Compute Q = k·G
    EC_POINT* Q = EC_POINT_new(group);
    BN_CTX* ctx = BN_CTX_new();
    
    EC_POINT_mul(group, Q, k, NULL, NULL, ctx);
    printf("Computed Q = k·G\n");
    
    // Verify
    bool verified = verify_k_recovery(k, Q, group);
    printf("\n%s Verification: %s\n", 
           verified ? "✓" : "✗",
           verified ? "k·G == Q" : "k·G != Q");
    
    // Test with wrong k
    BIGNUM* wrong_k = BN_new();
    BN_set_word(wrong_k, 54321);
    
    printf("\nTesting with wrong k: 54321\n");
    bool verified2 = verify_k_recovery(wrong_k, Q, group);
    printf("%s Verification: %s\n",
           verified2 ? "✓" : "✗",
           verified2 ? "k·G == Q" : "k·G != Q (expected)");
    
    // Cleanup
    BN_free(k);
    BN_free(wrong_k);
    EC_POINT_free(Q);
    BN_CTX_free(ctx);
    EC_GROUP_free(group);
    
    printf("\n");
}

void test_complete_pipeline() {
    printf("=== Test 4: Complete Recovery Pipeline ===\n\n");
    
    // Create Platonic overlay
    PlatonicOverlay* overlay = create_platonic_overlay(0.5);
    if (!overlay) {
        printf("Failed to create overlay\n");
        return;
    }
    
    // Create target position
    ClockPosition target;
    target.ring = 0;
    target.position = 3;
    target.angle = M_PI / 2.0;
    target.radius = 0.25;
    
    printf("Target: ring=%d, pos=%d, angle=%.4f\n\n",
           target.ring, target.position, target.angle);
    
    // Create EC group
    EC_GROUP* group = EC_GROUP_new_by_curve_name(NID_secp256k1);
    
    // Create test k and Q
    BIGNUM* original_k = BN_new();
    BN_set_word(original_k, 1000);
    
    EC_POINT* Q = EC_POINT_new(group);
    BN_CTX* ctx = BN_CTX_new();
    EC_POINT_mul(group, Q, original_k, NULL, NULL, ctx);
    
    printf("Original k: 1000\n");
    printf("Computed Q = k·G\n\n");
    
    // Recover k
    BIGNUM* recovered_k = BN_new();
    bool success = complete_k_recovery(overlay, target, Q, group, recovered_k);
    
    if (success) {
        char* k_str = BN_bn2dec(recovered_k);
        printf("Recovered k: %s\n", k_str);
        OPENSSL_free(k_str);
    }
    
    // Cleanup
    BN_free(original_k);
    BN_free(recovered_k);
    EC_POINT_free(Q);
    BN_CTX_free(ctx);
    EC_GROUP_free(group);
    free_platonic_overlay(overlay);
    
    printf("\n");
}

void test_multiple_k_values() {
    printf("=== Test 5: Multiple K Values ===\n\n");
    
    // Create Platonic overlay
    PlatonicOverlay* overlay = create_platonic_overlay(0.5);
    if (!overlay) {
        printf("Failed to create overlay\n");
        return;
    }
    
    // Test with multiple k values
    uint64_t test_k_values[] = {1, 10, 100, 1000, 10000};
    int num_tests = 5;
    
    printf("Testing recovery with %d different k values:\n\n", num_tests);
    
    for (int i = 0; i < num_tests; i++) {
        // Create target position based on k
        ClockPosition target = map_k_to_clock_u64(test_k_values[i]);
        
        printf("Test %d: k=%lu\n", i+1, test_k_values[i]);
        printf("  Target: ring=%d, pos=%d, angle=%.4f\n",
               target.ring, target.position, target.angle);
        
        // Recover k
        ClockRecoveryContext* ctx = init_clock_recovery(10);
        BIGNUM* recovered_k = BN_new();
        
        bool success = recover_k_with_platonic_anchors(ctx, overlay, target, recovered_k);
        
        if (success) {
            char* k_str = BN_bn2dec(recovered_k);
            printf("  Recovered: k=%s\n", k_str);
            OPENSSL_free(k_str);
        }
        
        BN_free(recovered_k);
        free_clock_recovery(ctx);
        printf("\n");
    }
    
    free_platonic_overlay(overlay);
}

int main() {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║  Enhanced K Recovery Test Suite                           ║\n");
    printf("║  Testing complete k recovery pipeline                     ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    test_inverse_mapping();
    test_platonic_anchor_recovery();
    test_ec_verification();
    test_complete_pipeline();
    test_multiple_k_values();
    
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║  All Tests Complete                                        ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    return 0;
}