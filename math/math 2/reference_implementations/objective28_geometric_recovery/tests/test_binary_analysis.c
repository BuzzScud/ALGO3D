#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/obj_mac.h>

int main() {
    printf("=== BINARY PATTERN ANALYSIS ===\n\n");
    
    // Setup
    EC_GROUP* curve = EC_GROUP_new_by_curve_name(NID_secp128r1);
    BIGNUM* order = BN_new();
    EC_GROUP_get_order(curve, order, NULL);
    
    // Generate actual k
    BIGNUM* actual_k = BN_new();
    BN_rand_range(actual_k, order);
    
    char* k_hex = BN_bn2hex(actual_k);
    printf("Actual k: %s\n", k_hex);
    
    unsigned char k_bytes[16];
    BN_bn2binpad(actual_k, k_bytes, 16);
    printf("First 8 bytes: ");
    for (int i = 0; i < 8; i++) {
        printf("%02X ", k_bytes[i]);
    }
    printf("\n\n");
    OPENSSL_free(k_hex);
    
    // Simulate what triangulation does
    printf("Simulating triangulation with 3 random anchors:\n");
    BIGNUM* anchors[3];
    double weights[3] = {0.5, 0.3, 0.2};  // Typical weights
    
    for (int i = 0; i < 3; i++) {
        anchors[i] = BN_new();
        BN_rand_range(anchors[i], order);
        
        char* a_hex = BN_bn2hex(anchors[i]);
        printf("  Anchor %d (weight %.1f): %s\n", i, weights[i], a_hex);
        OPENSSL_free(a_hex);
    }
    
    // Weighted average (what triangulation does)
    BIGNUM* result = BN_new();
    BN_CTX* ctx = BN_CTX_new();
    
    BN_zero(result);
    for (int i = 0; i < 3; i++) {
        BIGNUM* temp = BN_new();
        BN_copy(temp, anchors[i]);
        BN_mul_word(temp, (BN_ULONG)(weights[i] * 1000000));
        BN_add(result, result, temp);
        BN_free(temp);
    }
    BN_div_word(result, 1000000);
    
    char* result_hex = BN_bn2hex(result);
    printf("\nWeighted average result: %s\n", result_hex);
    
    unsigned char result_bytes[16];
    BN_bn2binpad(result, result_bytes, 16);
    printf("First 8 bytes: ");
    for (int i = 0; i < 8; i++) {
        printf("%02X ", result_bytes[i]);
    }
    printf("\n\n");
    OPENSSL_free(result_hex);
    
    // Now test with SAME anchors but different weights
    printf("Same anchors, different weights (0.6, 0.25, 0.15):\n");
    double weights2[3] = {0.6, 0.25, 0.15};
    
    BIGNUM* result2 = BN_new();
    BN_zero(result2);
    for (int i = 0; i < 3; i++) {
        BIGNUM* temp = BN_new();
        BN_copy(temp, anchors[i]);
        BN_mul_word(temp, (BN_ULONG)(weights2[i] * 1000000));
        BN_add(result2, result2, temp);
        BN_free(temp);
    }
    BN_div_word(result2, 1000000);
    
    char* result2_hex = BN_bn2hex(result2);
    printf("Result: %s\n", result2_hex);
    
    unsigned char result2_bytes[16];
    BN_bn2binpad(result2, result2_bytes, 16);
    printf("First 8 bytes: ");
    for (int i = 0; i < 8; i++) {
        printf("%02X ", result2_bytes[i]);
    }
    printf("\n\n");
    OPENSSL_free(result2_hex);
    
    // Compare
    printf("Comparing results:\n");
    printf("Result 1: ");
    for (int i = 0; i < 16; i++) printf("%02X", result_bytes[i]);
    printf("\nResult 2: ");
    for (int i = 0; i < 16; i++) printf("%02X", result2_bytes[i]);
    printf("\n");
    
    int same_bytes = 0;
    for (int i = 0; i < 16; i++) {
        if (result_bytes[i] == result2_bytes[i]) same_bytes++;
    }
    printf("Same bytes: %d / 16\n", same_bytes);
    
    // Cleanup
    for (int i = 0; i < 3; i++) BN_free(anchors[i]);
    BN_free(actual_k);
    BN_free(result);
    BN_free(result2);
    BN_free(order);
    BN_CTX_free(ctx);
    EC_GROUP_free(curve);
    
    return 0;
}
