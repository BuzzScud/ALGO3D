#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/obj_mac.h>
#include "geometric_recovery.h"

int main() {
    printf("=== ACTUAL K POSITION ANALYSIS ===\n\n");
    
    EC_GROUP* curve = EC_GROUP_new_by_curve_name(NID_secp128r1);
    const EC_POINT* G = EC_GROUP_get0_generator(curve);
    BIGNUM* order = BN_new();
    EC_GROUP_get_order(curve, order, NULL);
    
    // Generate actual k and Q
    BIGNUM* actual_k = BN_new();
    BN_rand_range(actual_k, order);
    
    EC_POINT* actual_Q = EC_POINT_new(curve);
    EC_POINT_mul(curve, actual_Q, NULL, G, actual_k, NULL);
    
    char* k_hex = BN_bn2hex(actual_k);
    printf("Actual k: %s\n\n", k_hex);
    OPENSSL_free(k_hex);
    
    // Map actual k to position
    double actual_pos[13];
    map_k_to_position(actual_k, actual_pos, 13);
    
    printf("Actual k position in 13D:\n");
    for (int d = 0; d < 13; d++) {
        printf("  Dim %2d: %.6f\n", d, actual_pos[d]);
    }
    
    // Generate 100 anchors
    printf("\nGenerating 100 anchors...\n");
    BIGNUM** anchor_k = malloc(100 * sizeof(BIGNUM*));
    double** anchor_pos = malloc(100 * sizeof(double*));
    
    for (int i = 0; i < 100; i++) {
        anchor_k[i] = BN_new();
        BN_rand_range(anchor_k[i], order);
        
        anchor_pos[i] = malloc(13 * sizeof(double));
        map_k_to_position(anchor_k[i], anchor_pos[i], 13);
    }
    
    // Find 3 nearest anchors to actual k
    uint32_t nearest[3] = {0, 1, 2};
    double distances[3] = {1e100, 1e100, 1e100};
    
    for (int a = 0; a < 100; a++) {
        double dist = 0.0;
        for (int d = 0; d < 13; d++) {
            double diff = actual_pos[d] - anchor_pos[a][d];
            dist += diff * diff;
        }
        dist = sqrt(dist);
        
        for (int i = 0; i < 3; i++) {
            if (dist < distances[i]) {
                for (int j = 2; j > i; j--) {
                    nearest[j] = nearest[j-1];
                    distances[j] = distances[j-1];
                }
                nearest[i] = a;
                distances[i] = dist;
                break;
            }
        }
    }
    
    printf("\n3 Nearest anchors to actual k:\n");
    for (int i = 0; i < 3; i++) {
        char* a_hex = BN_bn2hex(anchor_k[nearest[i]]);
        printf("  Anchor %d (distance %.4f): %s\n", nearest[i], distances[i], a_hex);
        OPENSSL_free(a_hex);
    }
    
    // Compute weighted average
    double weights[3];
    double total_weight = 0.0;
    for (int i = 0; i < 3; i++) {
        weights[i] = 1.0 / (distances[i] * distances[i] + 1e-10);
        total_weight += weights[i];
    }
    for (int i = 0; i < 3; i++) {
        weights[i] /= total_weight;
    }
    
    printf("\nWeights: %.4f, %.4f, %.4f\n", weights[0], weights[1], weights[2]);
    
    BIGNUM* interpolated = BN_new();
    BN_zero(interpolated);
    
    const uint64_t SCALE = 1000000000ULL;
    for (int i = 0; i < 3; i++) {
        BIGNUM* weighted = BN_dup(anchor_k[nearest[i]]);
        BN_mul_word(weighted, (uint64_t)(weights[i] * SCALE));
        BN_add(interpolated, interpolated, weighted);
        BN_free(weighted);
    }
    BN_div_word(interpolated, SCALE);
    
    char* interp_hex = BN_bn2hex(interpolated);
    printf("Interpolated k: %s\n", interp_hex);
    OPENSSL_free(interp_hex);
    
    // Compare to actual k
    unsigned char actual_bytes[16], interp_bytes[16];
    BN_bn2binpad(actual_k, actual_bytes, 16);
    BN_bn2binpad(interpolated, interp_bytes, 16);
    
    int hamming = 0;
    for (int i = 0; i < 16; i++) {
        unsigned char xor = actual_bytes[i] ^ interp_bytes[i];
        for (int b = 0; b < 8; b++) {
            if (xor & (1 << b)) hamming++;
        }
    }
    
    printf("\nHamming distance: %d / 128 bits (%.2f%%)\n", hamming, hamming * 100.0 / 128.0);
    
    // Cleanup
    for (int i = 0; i < 100; i++) {
        BN_free(anchor_k[i]);
        free(anchor_pos[i]);
    }
    free(anchor_k);
    free(anchor_pos);
    BN_free(actual_k);
    BN_free(interpolated);
    BN_free(order);
    EC_POINT_free(actual_Q);
    EC_GROUP_free(curve);
    
    return 0;
}
