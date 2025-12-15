/**
 * @file test_secp128_corrected.c
 * @brief Corrected iterative recovery - no prime/QR requirements
 * 
 * Key fixes based on analysis:
 * 1. Bitcoin k is NOT prime - just 128-bit integer
 * 2. No QR filter during recovery - only for lattice structure
 * 3. Larger search space: 2^20 candidates
 * 4. Pure Q verification: k*G == Q
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <openssl/ec.h>
#include <openssl/obj_mac.h>
#include <openssl/bn.h>
#include <openssl/sha.h>

#define TOY_CURVE NID_secp128r1
#define TOY_BIT_SCALE 128
#define SEARCH_SPACE_BITS 20  // 2^20 = 1M candidates

/**
 * Generate deterministic target from Q using SHA256
 */
static BIGNUM* generate_target_from_Q(EC_GROUP* curve, EC_POINT* Q) {
    BIGNUM* x = BN_new();
    BIGNUM* y = BN_new();
    BN_CTX* ctx = BN_CTX_new();
    
    EC_POINT_get_affine_coordinates(curve, Q, x, y, ctx);
    
    unsigned char x_bytes[32], y_bytes[32];
    memset(x_bytes, 0, 32);
    memset(y_bytes, 0, 32);
    
    int x_len = BN_num_bytes(x);
    int y_len = BN_num_bytes(y);
    if (x_len > 32) x_len = 32;
    if (y_len > 32) y_len = 32;
    
    BN_bn2bin(x, x_bytes + (32 - x_len));
    BN_bn2bin(y, y_bytes + (32 - y_len));
    
    unsigned char combined[64];
    memcpy(combined, x_bytes, 32);
    memcpy(combined + 32, y_bytes, 32);
    
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(combined, 64, hash);
    
    BIGNUM* target = BN_new();
    BN_bin2bn(hash, SHA256_DIGEST_LENGTH, target);
    
    // Ensure in valid range
    BIGNUM* order = BN_new();
    EC_GROUP_get_order(curve, order, ctx);
    BN_mod(target, target, order, ctx);
    
    BN_free(x);
    BN_free(y);
    BN_free(order);
    BN_CTX_free(ctx);
    
    return target;
}

int main(void) {
    printf("=== CORRECTED ITERATIVE RECOVERY (secp128r1) ===\n");
    printf("Fixes:\n");
    printf("  - No prime requirement (k is just 128-bit integer)\n");
    printf("  - No QR filter (only for lattice, not recovery)\n");
    printf("  - Larger search: 2^%d = %llu candidates\n", SEARCH_SPACE_BITS, 1ULL << SEARCH_SPACE_BITS);
    printf("  - Pure Q verification: k*G == Q\n\n");
    
    clock_t start = clock();
    
    EC_GROUP* curve = EC_GROUP_new_by_curve_name(TOY_CURVE);
    
    printf("Testing on 5 keys...\n");
    uint32_t recovered = 0;
    double total_time = 0.0;
    
    for (uint32_t test = 0; test < 5; test++) {
        printf("\nTest key %u:\n", test);
        
        // Generate test key
        BIGNUM* k_actual = BN_new();
        BIGNUM* order = BN_new();
        BN_CTX* ctx = BN_CTX_new();
        
        EC_GROUP_get_order(curve, order, ctx);
        BN_rand_range(k_actual, order);
        
        EC_POINT* Q_actual = EC_POINT_new(curve);
        const EC_POINT* G = EC_GROUP_get0_generator(curve);
        EC_POINT_mul(curve, Q_actual, NULL, G, k_actual, ctx);
        
        // Generate deterministic target from Q
        clock_t key_start = clock();
        BIGNUM* target = generate_target_from_Q(curve, Q_actual);
        
        printf("  Target generated from Q\n");
        printf("  Searching 2^%d candidates...\n", SEARCH_SPACE_BITS);
        
        // Search
        bool found = false;
        uint64_t search_limit = 1ULL << SEARCH_SPACE_BITS;
        
        for (uint64_t offset = 0; offset < search_limit; offset++) {
            BIGNUM* candidate_k = BN_dup(target);
            BN_add_word(candidate_k, offset);
            BN_mod(candidate_k, candidate_k, order, ctx);
            
            // Verify: candidate_k * G == Q_actual?
            EC_POINT* candidate_Q = EC_POINT_new(curve);
            EC_POINT_mul(curve, candidate_Q, NULL, G, candidate_k, ctx);
            
            int match = EC_POINT_cmp(curve, candidate_Q, Q_actual, ctx);
            
            if (match == 0) {
                // FOUND IT!
                clock_t key_end = clock();
                double key_time = (double)(key_end - key_start) / CLOCKS_PER_SEC;
                
                printf("  ✓ FOUND at offset %lu (%.3f seconds)\n", offset, key_time);
                
                recovered++;
                total_time += key_time;
                found = true;
                
                EC_POINT_free(candidate_Q);
                BN_free(candidate_k);
                break;
            }
            
            EC_POINT_free(candidate_Q);
            BN_free(candidate_k);
            
            if ((offset + 1) % 100000 == 0) {
                printf("    Searched %lu/%lu...\n", offset + 1, search_limit);
            }
        }
        
        if (!found) {
            clock_t key_end = clock();
            double key_time = (double)(key_end - key_start) / CLOCKS_PER_SEC;
            printf("  ✗ NOT FOUND in 2^%d candidates (%.3f seconds)\n", 
                   SEARCH_SPACE_BITS, key_time);
            total_time += key_time;
        }
        
        BN_free(target);
        BN_free(k_actual);
        BN_free(order);
        EC_POINT_free(Q_actual);
        BN_CTX_free(ctx);
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("\n=== RESULTS ===\n");
    printf("Exact matches: %u/5 (%.1f%%)\n", recovered, (double)recovered / 5 * 100.0);
    printf("Total time: %.3f seconds\n", elapsed);
    printf("Avg time per key: %.3f seconds\n", total_time / 5);
    
    if (recovered > 0) {
        printf("Avg time per FOUND key: %.3f seconds\n", total_time / recovered);
    }
    
    printf("\n=== ANALYSIS ===\n");
    if (recovered == 5) {
        printf("✅ 100%% SUCCESS! Pure brute force works!\n");
        printf("Search space 2^%d is sufficient for secp128r1\n", SEARCH_SPACE_BITS);
    } else if (recovered > 0) {
        printf("⚠️  Partial success (%u/5)\n", recovered);
        printf("Need larger search space (try 2^24 or 2^28)\n");
    } else {
        printf("❌ No matches found\n");
        printf("Target generation may be incorrect\n");
        printf("Or search space 2^%d too small\n", SEARCH_SPACE_BITS);
    }
    
    EC_GROUP_free(curve);
    
    return 0;
}
