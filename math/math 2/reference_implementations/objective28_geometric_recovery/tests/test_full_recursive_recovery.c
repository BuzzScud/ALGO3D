#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/obj_mac.h>
#include "../include/recursive_recovery.h"
#include "../include/geometric_recovery.h"

int main(void) {
    printf("=== FULL RECURSIVE RECOVERY ALGORITHM TEST ===\n\n");
    
    // Initialize
    EC_GROUP* curve = EC_GROUP_new_by_curve_name(NID_secp128r1);
    BN_CTX* bn_ctx = BN_CTX_new();
    BIGNUM* order = BN_new();
    EC_GROUP_get_order(curve, order, bn_ctx);
    
    // Generate target k and Q
    printf("Generating target k/Q pair...\n");
    BIGNUM* target_k = BN_new();
    BN_rand_range(target_k, order);
    EC_POINT* target_Q = EC_POINT_new(curve);
    EC_POINT_mul(curve, target_Q, target_k, NULL, NULL, bn_ctx);
    
    char* k_hex = BN_bn2hex(target_k);
    printf("Target k: %s\n\n", k_hex);
    OPENSSL_free(k_hex);
    
    // Generate known k samples for torus mapping
    int num_samples = 100;
    printf("Generating %d known k samples for torus mapping...\n", num_samples);
    
    BIGNUM** known_k = malloc(num_samples * sizeof(BIGNUM*));
    EC_POINT** known_Q = malloc(num_samples * sizeof(EC_POINT*));
    
    for (int i = 0; i < num_samples; i++) {
        known_k[i] = BN_new();
        BN_rand_range(known_k[i], order);
        known_Q[i] = EC_POINT_new(curve);
        EC_POINT_mul(curve, known_Q[i], known_k[i], NULL, NULL, bn_ctx);
    }
    printf("Generated %d samples\n\n", num_samples);
    
    // Set up recursion parameters
    RecursionParams params = {
        .min_resolution = 16777216,      // 2^24
        .max_resolution = 268435456,     // 2^28
        .min_anchors = 100,
        .max_anchors = 10000,
        .min_dimensions = 13,
        .max_dimensions = 26,
        .entropy_threshold = 0.05,
        .uniform_threshold = 0.50,
        .max_recursion_depth = 5
    };
    
    printf("Recursion parameters:\n");
    printf("  Resolution: 2^24 → 2^28\n");
    printf("  Anchors: 100 → 10000\n");
    printf("  Dimensions: 13 → 26\n");
    printf("  Entropy threshold: %.2f\n", params.entropy_threshold);
    printf("  Uniform threshold: %.0f%%\n", params.uniform_threshold * 100);
    printf("  Max recursion depth: %d\n\n", params.max_recursion_depth);
    
    // Run recursive recovery
    clock_t start = clock();
    
    BIGNUM* recovered_k = recursive_k_recovery(
        curve,
        target_Q,
        known_k,
        known_Q,
        num_samples,
        &params
    );
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    // Check result
    printf("\n=== RESULTS ===\n");
    printf("Time elapsed: %.2f seconds\n", elapsed);
    
    if (recovered_k) {
        char* recovered_hex = BN_bn2hex(recovered_k);
        printf("Recovered k: %s\n", recovered_hex);
        OPENSSL_free(recovered_hex);
        
        // Verify
        EC_POINT* recovered_Q = EC_POINT_new(curve);
        EC_POINT_mul(curve, recovered_Q, recovered_k, NULL, NULL, bn_ctx);
        
        if (EC_POINT_cmp(curve, recovered_Q, target_Q, bn_ctx) == 0) {
            printf("✓ SUCCESS - Recovered k is correct!\n");
        } else {
            printf("✗ FAILURE - Recovered k is incorrect\n");
        }
        
        EC_POINT_free(recovered_Q);
        BN_free(recovered_k);
    } else {
        printf("✗ Recovery failed - no k found\n");
    }
    
    // Cleanup
    for (int i = 0; i < num_samples; i++) {
        BN_free(known_k[i]);
        EC_POINT_free(known_Q[i]);
    }
    free(known_k);
    free(known_Q);
    
    BN_free(target_k);
    EC_POINT_free(target_Q);
    BN_free(order);
    BN_CTX_free(bn_ctx);
    EC_GROUP_free(curve);
    
    printf("\n=== TEST COMPLETE ===\n");
    return 0;
}