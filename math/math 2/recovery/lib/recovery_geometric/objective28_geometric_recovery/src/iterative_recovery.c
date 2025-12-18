/**
 * @file iterative_recovery.c
 * @brief Iterative Q→k recovery with verification (from Python research)
 * 
 * Based on your Python OscillationAccelerator.reverse_q_to_k():
 * 1. Generate deterministic target from Q
 * 2. Reduce entropy recursively
 * 3. Find prime near target
 * 4. Verify QR condition
 * 5. Iterate until convergence
 */

#include "geometric_recovery.h"
#include "prime_types.h"
#include <openssl/sha.h>
#include <stdlib.h>
#include <string.h>
#include "math/arithmetic.h"
#include "math/transcendental.h"

/**
 * Generate large deterministic target from Q (from your Python code)
 */
static BIGNUM* generate_target_from_Q(
    EC_GROUP* ec_group,
    EC_POINT* Q,
    uint32_t bit_scale
) {
    // Get Q coordinates
    BIGNUM* x = BN_new();
    BIGNUM* y = BN_new();
    BN_CTX* ctx = BN_CTX_new();
    
    EC_POINT_get_affine_coordinates(ec_group, Q, x, y, ctx);
    
    // Hash Q to get deterministic seed
    unsigned char x_bytes[32], y_bytes[32];
    memset(x_bytes, 0, 32);
    memset(y_bytes, 0, 32);
    
    int x_len = BN_num_bytes(x);
    int y_len = BN_num_bytes(y);
    if (x_len > 32) x_len = 32;
    if (y_len > 32) y_len = 32;
    
    BN_bn2bin(x, x_bytes + (32 - x_len));
    BN_bn2bin(y, y_bytes + (32 - y_len));
    
    // Concatenate and hash
    unsigned char combined[64];
    memcpy(combined, x_bytes, 32);
    memcpy(combined + 32, y_bytes, 32);
    
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(combined, 64, hash);
    
    // Generate large target from hash
    uint32_t needed_bytes = (bit_scale + 7) / 8;
    unsigned char* target_bytes = calloc(needed_bytes, 1);
    
    // Expand hash if needed
    uint32_t seed = 0;
    for (uint32_t offset = 0; offset < needed_bytes; offset += SHA256_DIGEST_LENGTH) {
        unsigned char seed_bytes[4];
        seed_bytes[0] = (seed >> 24) & 0xFF;
        seed_bytes[1] = (seed >> 16) & 0xFF;
        seed_bytes[2] = (seed >> 8) & 0xFF;
        seed_bytes[3] = seed & 0xFF;
        
        unsigned char temp_hash[SHA256_DIGEST_LENGTH];
        SHA256_CTX sha_ctx;
        SHA256_Init(&sha_ctx);
        SHA256_Update(&sha_ctx, combined, 64);
        SHA256_Update(&sha_ctx, seed_bytes, 4);
        SHA256_Final(temp_hash, &sha_ctx);
        
        uint32_t copy_len = (offset + SHA256_DIGEST_LENGTH > needed_bytes) ? 
                           (needed_bytes - offset) : SHA256_DIGEST_LENGTH;
        memcpy(target_bytes + offset, temp_hash, copy_len);
        seed++;
    }
    
    // Create BIGNUM from target bytes
    BIGNUM* target = BN_new();
    BN_bin2bn(target_bytes, needed_bytes, target);
    
    // Ensure target is in valid range [2^(bit_scale-1), 2^bit_scale - 1]
    BIGNUM* min_val = BN_new();
    BIGNUM* max_val = BN_new();
    BN_set_bit(min_val, bit_scale - 1);
    BN_set_bit(max_val, bit_scale);
    BN_sub_word(max_val, 1);
    
    // Adjust target to range
    BIGNUM* range = BN_new();
    BN_sub(range, max_val, min_val);
    BN_mod(target, target, range, ctx);
    BN_add(target, target, min_val);
    
    free(target_bytes);
    BN_free(x);
    BN_free(y);
    BN_free(min_val);
    BN_free(max_val);
    BN_free(range);
    BN_CTX_free(ctx);
    
    return target;
}

/**
 * Find prime near target (from your CrystalAbacus.find_prime_near)
 */
static BIGNUM* find_prime_near(BIGNUM* target) {
    BIGNUM* candidate = BN_dup(target);
    
    // Make odd
    if (!BN_is_odd(candidate)) {
        BN_add_word(candidate, 1);
    }
    
    // Search for next prime (up to 1000 attempts)
    for (int offset = 0; offset < 1000; offset++) {
        // Check if prime
        int is_prime = BN_is_prime_ex(candidate, BN_prime_checks, NULL, NULL);
        
        if (is_prime) {
            return candidate;
        }
        
        BN_add_word(candidate, 2);
    }
    
    BN_free(candidate);
    return NULL;
}

/**
 * Check Legendre symbol (QR condition from your Python code)
 */
static int check_legendre_symbol(BIGNUM* a, BIGNUM* p) {
    // Compute a^((p-1)/2) mod p
    // Result: 1 (QR), -1 (NQR), 0 (divisible)
    
    BN_CTX* ctx = BN_CTX_new();
    BIGNUM* exp = BN_new();
    BIGNUM* result = BN_new();
    
    // exp = (p - 1) / 2
    BN_copy(exp, p);
    BN_sub_word(exp, 1);
    BN_rshift1(exp, exp);
    
    // result = a^exp mod p
    BN_mod_exp(result, a, exp, p, ctx);
    
    int symbol = 0;
    if (BN_is_one(result)) {
        symbol = 1;  // Quadratic residue
    } else {
        BIGNUM* p_minus_1 = BN_new();
        BN_copy(p_minus_1, p);
        BN_sub_word(p_minus_1, 1);
        
        if (BN_cmp(result, p_minus_1) == 0) {
            symbol = -1;  // Non-residue
        }
        
        BN_free(p_minus_1);
    }
    
    BN_free(exp);
    BN_free(result);
    BN_CTX_free(ctx);
    
    return symbol;
}

/**
 * Iterative Q→k recovery with verification (MAIN ALGORITHM)
 * 
 * This implements your Python reverse_q_to_k() with full verification.
 */
BIGNUM* iterative_recover_k_from_Q(
    GeometricRecoveryContext* ctx,
    EC_POINT* target_Q,
    uint32_t bit_scale,
    double* confidence_out
) {
    printf("  Starting iterative recovery (bit_scale=%u)...\n", bit_scale);
    
    // Step 1: Generate deterministic target from Q
    BIGNUM* target = generate_target_from_Q(ctx->ec_group, target_Q, bit_scale);
    
    // Step 2: Reduce entropy (from your OscillationAccelerator)
    uint64_t search_space = 1ULL << (bit_scale / 2);
    uint64_t threshold = 1ULL << 16;  // 2^16 as in your code
    
    uint64_t reduced_space = search_space;
    while (reduced_space > threshold) {
        reduced_space /= 2;
    }
    
    printf("    Entropy reduced: 2^%u → 2^16 (%lu candidates)\n", 
           bit_scale / 2, reduced_space);
    
    // Step 3: Search in reduced space
    BIGNUM* best_k = NULL;
    double best_distance = 1.0;
    uint32_t iterations = 0;
    uint32_t max_iterations = (reduced_space < 10000) ? reduced_space : 10000;
    
    for (uint64_t offset = 0; offset < max_iterations; offset++) {
        iterations++;
        
        // Generate candidate: target + offset
        BIGNUM* candidate_target = BN_dup(target);
        BN_add_word(candidate_target, offset);
        
        // Find prime near this target (from your CrystalAbacus)
        BIGNUM* candidate_k = find_prime_near(candidate_target);
        BN_free(candidate_target);
        
        if (!candidate_k) continue;
        
        // Step 4: Verify QR condition (from your Python code)
        BIGNUM* Q_x = BN_new();
        EC_POINT_get_affine_coordinates(ctx->ec_group, target_Q, Q_x, NULL, NULL);
        
        int legendre = check_legendre_symbol(Q_x, candidate_k);
        BN_free(Q_x);
        
        if (legendre != 1) {
            BN_free(candidate_k);
            continue;  // QR filter failed
        }
        
        // Step 5: CRITICAL - Verify candidate produces target_Q
        EC_POINT* candidate_Q = EC_POINT_new(ctx->ec_group);
        const EC_POINT* G = EC_GROUP_get0_generator(ctx->ec_group);
        BN_CTX* bn_ctx = BN_CTX_new();
        
        EC_POINT_mul(ctx->ec_group, candidate_Q, NULL, G, candidate_k, bn_ctx);
        
        // Check if candidate_Q == target_Q
        int cmp = EC_POINT_cmp(ctx->ec_group, candidate_Q, target_Q, bn_ctx);
        
        if (cmp == 0) {
            // FOUND IT!
            printf("    ✓ FOUND k at iteration %u (offset=%lu)\n", iterations, offset);
            
            EC_POINT_free(candidate_Q);
            BN_CTX_free(bn_ctx);
            BN_free(target);
            
            if (confidence_out) {
                *confidence_out = 1.0;  // Perfect match
            }
            
            return candidate_k;
        }
        
        // Not exact match, but track best candidate
        double distance = measure_ec_point_distance(ctx->ec_group, candidate_Q, target_Q);
        
        if (distance < best_distance) {
            best_distance = distance;
            if (best_k) BN_free(best_k);
            best_k = BN_dup(candidate_k);
        }
        
        EC_POINT_free(candidate_Q);
        BN_CTX_free(bn_ctx);
        BN_free(candidate_k);
        
        // Progress update
        if (iterations % 1000 == 0) {
            printf("    Iteration %u: best_distance=%.4f\n", iterations, best_distance);
        }
    }
    
    BN_free(target);
    
    printf("    No exact match found in %u iterations\n", iterations);
    printf("    Best distance: %.4f\n", best_distance);
    
    if (confidence_out) {
        *confidence_out = 1.0 - best_distance;
    }
    
    return best_k;  // Return best candidate even if not exact
}
