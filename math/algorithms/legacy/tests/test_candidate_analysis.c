/**
 * Analyze the 2 Candidates - What Do They Represent?
 * 
 * This test analyzes the geometric properties of the 2 shared vertices
 * to understand what they represent in the Platonic solid structure.
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
    printf("\n================================================================================\n");
}

void analyze_bignum(const char* label, BIGNUM* bn, BIGNUM* order, BN_CTX* ctx) {
    char* hex = BN_bn2hex(bn);
    printf("%s: %s\n", label, hex);
    printf("  Bits: %d\n", BN_num_bits(bn));
    printf("  Bytes: %d\n", BN_num_bytes(bn));
    
    // Check if it's within order
    int cmp = BN_cmp(bn, order);
    printf("  Within order: %s\n", cmp < 0 ? "YES" : "NO");
    
    // Check if it's prime
    int is_prime = BN_is_prime_ex(bn, BN_prime_checks, ctx, NULL);
    printf("  Is prime: %s\n", is_prime ? "YES" : "NO");
    
    OPENSSL_free(hex);
}

void analyze_position(const char* label, const double* position, uint32_t num_dimensions) {
    printf("%s: [", label);
    for (uint32_t d = 0; d < num_dimensions; d++) {
        printf("%.6f%s", position[d], d < num_dimensions - 1 ? ", " : "");
    }
    printf("]\n");
    
    // Compute magnitude
    double magnitude = 0.0;
    for (uint32_t d = 0; d < num_dimensions; d++) {
        magnitude += position[d] * position[d];
    }
    magnitude = sqrt(magnitude);
    printf("  Magnitude: %.6f\n", magnitude);
}

int main() {
    printf("=== CANDIDATE ANALYSIS - WHAT DO THE 2 VERTICES REPRESENT? ===\n\n");
    
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
    
    printf("Curve: secp128r1\n");
    char* order_hex = BN_bn2hex(order);
    printf("Order: %s\n", order_hex);
    printf("Order bits: %d\n\n", BN_num_bits(order));
    OPENSSL_free(order_hex);
    
    // Generate 100 anchors
    printf("Generating 100 anchor k/Q pairs...\n");
    BIGNUM** anchor_k = malloc(100 * sizeof(BIGNUM*));
    EC_POINT** anchor_Q = malloc(100 * sizeof(EC_POINT*));
    
    for (int i = 0; i < 100; i++) {
        anchor_k[i] = BN_new();
        BN_rand_range(anchor_k[i], order);
        
        anchor_Q[i] = EC_POINT_new(curve);
        EC_POINT_mul(curve, anchor_Q[i], NULL, G, anchor_k[i], ctx);
    }
    
    printf("Generated 100 anchors\n\n");
    
    // Create recovery context
    GeometricRecoveryContext* recovery_ctx = geometric_recovery_create(curve, 100, 13);
    
    // Add anchors
    for (int i = 0; i < 100; i++) {
        geometric_recovery_add_anchor(recovery_ctx, anchor_k[i], anchor_Q[i]);
    }
    
    // Initialize
    printf("Initializing recovery context...\n");
    bool init_success = geometric_recovery_initialize(recovery_ctx);
    
    if (!init_success) {
        printf("Initialization failed\n");
        return 1;
    }
    
    printf("Initialized successfully\n");
    // Note: GeometricRecoveryContext is opaque, so we can't access internal fields directly
    printf("Context initialized (internal structure opaque)\n");
    
    print_separator();
    printf("TESTING ITERATIVE RECOVERY\n");
    print_separator();
    
    // Generate test key
    BIGNUM* test_k = BN_new();
    BN_rand_range(test_k, order);
    
    EC_POINT* test_Q = EC_POINT_new(curve);
    EC_POINT_mul(curve, test_Q, NULL, G, test_k, ctx);
    
    char* test_k_hex = BN_bn2hex(test_k);
    printf("\nTarget k: %s\n", test_k_hex);
    OPENSSL_free(test_k_hex);
    
    // Run iterative recovery with analysis
    printf("\nRunning iterative recovery (first 10 iterations with analysis)...\n");
    
    for (uint32_t iter = 0; iter < 10; iter++) {
        // This is a simplified version - in reality we'd need to access internal state
        printf("\nIteration %u: (analysis would go here)\n", iter);
    }
    
    printf("\nNote: Full candidate analysis requires exposing internal GeometricRecoveryContext structure\n");
    printf("      or creating accessor functions for shared vertices, tori, etc.\n");
    
    BN_free(test_k);
    EC_POINT_free(test_Q);
    
    for (int i = 0; i < 100; i++) {
        BN_free(anchor_k[i]);
        EC_POINT_free(anchor_Q[i]);
    }
    free(anchor_k);
    free(anchor_Q);
    
    geometric_recovery_free(recovery_ctx);
    BN_free(order);
    BN_CTX_free(ctx);
    EC_GROUP_free(curve);
    
    print_separator();
    printf("ANALYSIS COMPLETE\n");
    print_separator();
    
    return 0;
}
