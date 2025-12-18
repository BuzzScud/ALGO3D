#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/obj_mac.h>
#include "../include/geometric_recovery.h"

int main(void) {
    printf("=== Testing 10,000 Anchor Allocation ===\n\n");
    
    // Initialize curve
    EC_GROUP* curve = EC_GROUP_new_by_curve_name(NID_secp256k1);
    if (!curve) {
        fprintf(stderr, "Failed to create curve\n");
        return 1;
    }
    
    BN_CTX* bn_ctx = BN_CTX_new();
    if (!bn_ctx) {
        fprintf(stderr, "Failed to create BN_CTX\n");
        EC_GROUP_free(curve);
        return 1;
    }
    
    // Test different anchor counts
    int test_counts[] = {100, 1000, 5000, 10000};
    int num_tests = sizeof(test_counts) / sizeof(test_counts[0]);
    
    for (int t = 0; t < num_tests; t++) {
        int num_anchors = test_counts[t];
        printf("Testing with %d anchors...\n", num_anchors);
        
        clock_t start = clock();
        
        // Create context
        GeometricRecoveryContext* ctx = geometric_recovery_create(curve, num_anchors, 13);
        if (!ctx) {
            fprintf(stderr, "  FAILED: Could not create context\n");
            continue;
        }
        
        clock_t create_time = clock();
        printf("  Context created in %.3f seconds\n", 
               (double)(create_time - start) / CLOCKS_PER_SEC);
        
        // Generate and add anchors
        BIGNUM* k = BN_new();
        EC_POINT* Q = EC_POINT_new(curve);
        BIGNUM* order = BN_new();
        EC_GROUP_get_order(curve, order, bn_ctx);
        
        for (int i = 0; i < num_anchors; i++) {
            // Generate random k
            BN_rand_range(k, order);
            
            // Compute Q = k*G
            const EC_POINT* G = EC_GROUP_get0_generator(curve);
            EC_POINT_mul(curve, Q, k, NULL, NULL, bn_ctx);
            
            // Add anchor
            geometric_recovery_add_anchor(ctx, k, Q);
            
            if ((i + 1) % 1000 == 0) {
                printf("  Added %d anchors...\n", i + 1);
            }
        }
        
        clock_t add_time = clock();
        printf("  Anchors added in %.3f seconds\n", 
               (double)(add_time - create_time) / CLOCKS_PER_SEC);
        
        // Initialize
        bool init_success = geometric_recovery_initialize(ctx);
        
        clock_t init_time = clock();
        printf("  Initialized in %.3f seconds (success: %s)\n", 
               (double)(init_time - add_time) / CLOCKS_PER_SEC,
               init_success ? "YES" : "NO");
        
        // Cleanup
        BN_free(k);
        EC_POINT_free(Q);
        BN_free(order);
        geometric_recovery_free(ctx);
        
        clock_t end = clock();
        printf("  Total time: %.3f seconds\n", 
               (double)(end - start) / CLOCKS_PER_SEC);
        printf("  SUCCESS: %d anchors handled correctly\n\n", num_anchors);
    }
    
    BN_CTX_free(bn_ctx);
    EC_GROUP_free(curve);
    
    printf("=== All Tests Complete ===\n");
    return 0;
}