#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/obj_mac.h>
#include "geometric_recovery.h"

// Analyze binary relationship between two BIGNUMs
void analyze_binary_relationship(BIGNUM* k1, BIGNUM* k2, const char* label1, const char* label2) {
    printf("\n=== Binary Analysis: %s vs %s ===\n", label1, label2);
    
    // Convert to binary
    int bits1 = BN_num_bits(k1);
    int bits2 = BN_num_bits(k2);
    
    printf("Bit lengths: %d vs %d\n", bits1, bits2);
    
    // Get bytes
    unsigned char bytes1[32] = {0};
    unsigned char bytes2[32] = {0};
    BN_bn2bin(k1, bytes1);
    BN_bn2bin(k2, bytes2);
    
    // Hamming distance
    int hamming = 0;
    for (int i = 0; i < 32; i++) {
        unsigned char xor = bytes1[i] ^ bytes2[i];
        for (int b = 0; b < 8; b++) {
            if (xor & (1 << b)) hamming++;
        }
    }
    
    printf("Hamming distance: %d / 256 bits (%.2f%%)\n", hamming, hamming * 100.0 / 256.0);
    
    // XOR analysis (manual since BN_xor doesn't exist)
    printf("XOR pattern (first 16 bytes): ");
    for (int i = 0; i < 16; i++) {
        printf("%02X ", bytes1[i] ^ bytes2[i]);
    }
    printf("\n");
    
    BN_CTX* ctx = BN_CTX_new();
    
    // Check if one is multiple of other
    BIGNUM* gcd = BN_new();
    BN_gcd(gcd, k1, k2, ctx);
    if (BN_cmp(gcd, BN_value_one()) != 0) {
        char* gcd_hex = BN_bn2hex(gcd);
        printf("GCD: %s (NOT coprime!)\n", gcd_hex);
        OPENSSL_free(gcd_hex);
    } else {
        printf("GCD: 1 (coprime)\n");
    }
    
    // Difference
    BIGNUM* diff = BN_new();
    if (BN_cmp(k1, k2) > 0) {
        BN_sub(diff, k1, k2);
    } else {
        BN_sub(diff, k2, k1);
    }
    char* diff_hex = BN_bn2hex(diff);
    printf("Absolute difference: %s\n", diff_hex);
    int diff_bits = BN_num_bits(diff);
    printf("Difference magnitude: %d bits\n", diff_bits);
    OPENSSL_free(diff_hex);
    
    BN_free(gcd);
    BN_free(diff);
    BN_CTX_free(ctx);
}

int main() {
    printf("=== DEEP CANDIDATE ANALYSIS ===\n\n");
    
    // Setup curve
    EC_GROUP* curve = EC_GROUP_new_by_curve_name(NID_secp128r1);
    const EC_POINT* G = EC_GROUP_get0_generator(curve);
    BIGNUM* order = BN_new();
    EC_GROUP_get_order(curve, order, NULL);
    
    // Generate actual k and Q
    printf("Generating actual k/Q pair...\n");
    BIGNUM* actual_k = BN_new();
    BN_rand_range(actual_k, order);
    
    EC_POINT* actual_Q = EC_POINT_new(curve);
    EC_POINT_mul(curve, actual_Q, NULL, G, actual_k, NULL);
    
    char* k_hex = BN_bn2hex(actual_k);
    printf("Actual k: %s\n", k_hex);
    OPENSSL_free(k_hex);
    
    // Generate 100 anchors
    printf("\nGenerating 100 anchors...\n");
    BIGNUM** anchor_k = malloc(100 * sizeof(BIGNUM*));
    EC_POINT** anchor_Q = malloc(100 * sizeof(EC_POINT*));
    
    for (int i = 0; i < 100; i++) {
        anchor_k[i] = BN_new();
        BN_rand_range(anchor_k[i], order);
        anchor_Q[i] = EC_POINT_new(curve);
        EC_POINT_mul(curve, anchor_Q[i], NULL, G, anchor_k[i], NULL);
    }
    
    // Create recovery context
    printf("Creating recovery context...\n");
    GeometricRecoveryContext* ctx = geometric_recovery_create(curve, 100, 13);
    
    for (int i = 0; i < 100; i++) {
        geometric_recovery_add_anchor(ctx, anchor_k[i], anchor_Q[i]);
    }
    
    bool init_success = geometric_recovery_initialize(ctx);
    if (!init_success) {
        printf("❌ Initialization failed\n");
        return 1;
    }
    
    printf("✅ Initialized successfully\n");
    printf("Shared vertices found: %u\n\n", ctx->num_shared_vertices);
    
    // Generate first 5 candidates using the actual iterative recovery
    printf("=== Generating Candidates ===\n");
    
    double confidence;
    uint32_t iterations;
    
    // Run recovery (will generate candidates internally)
    BIGNUM* result = geometric_recovery_iterative(ctx, actual_Q, 5, &confidence, &iterations);
    
    printf("\nRecovery result: %s\n", result ? "Found candidate" : "No candidate");
    
    if (result) {
        char* result_hex = BN_bn2hex(result);
        printf("Result k: %s\n", result_hex);
        OPENSSL_free(result_hex);
        
        analyze_binary_relationship(result, actual_k, "Result", "Actual k");
        
        BN_free(result);
    }
    
    // Cleanup
    for (int i = 0; i < 100; i++) {
        BN_free(anchor_k[i]);
        EC_POINT_free(anchor_Q[i]);
    }
    free(anchor_k);
    free(anchor_Q);
    
    geometric_recovery_free(ctx);
    BN_free(actual_k);
    BN_free(order);
    EC_POINT_free(actual_Q);
    EC_GROUP_free(curve);
    
    return 0;
}
