/**
 * @file q_validation.c
 * @brief Q-based validation with elliptic curve constraint
 */

#include "geometric_recovery.h"
#include <openssl/ec.h>
#include <openssl/bn.h>
#include "math/arithmetic.h"
#include "math/transcendental.h"

/**
 * Validate candidate k by computing Q and measuring distance
 * 
 * This enforces the actual k→Q elliptic curve constraint.
 * Returns distance in range [0, 1] where 0 = perfect match.
 */
double validate_candidate_k(
    BIGNUM* candidate_k,
    EC_GROUP* ec_group,
    EC_POINT* target_Q
) {
    // Compute Q from candidate k: Q = k * G
    const EC_POINT* G = EC_GROUP_get0_generator(ec_group);
    EC_POINT* candidate_Q = EC_POINT_new(ec_group);
    BN_CTX* ctx = BN_CTX_new();
    
    EC_POINT_mul(ec_group, candidate_Q, NULL, G, candidate_k, ctx);
    
    // Measure distance between candidate Q and target Q
    double distance = measure_ec_point_distance(ec_group, candidate_Q, target_Q);
    
    EC_POINT_free(candidate_Q);
    BN_CTX_free(ctx);
    
    return distance;
}

/**
 * Measure distance between two EC points
 * 
 * Uses normalized Euclidean distance in affine coordinates.
 * Returns distance in range [0, 1] where 0 = identical points.
 */
double measure_ec_point_distance(
    EC_GROUP* ec_group,
    EC_POINT* point1,
    EC_POINT* point2
) {
    BN_CTX* ctx = BN_CTX_new();
    BIGNUM* x1 = BN_new();
    BIGNUM* y1 = BN_new();
    BIGNUM* x2 = BN_new();
    BIGNUM* y2 = BN_new();
    
    // Get affine coordinates
    EC_POINT_get_affine_coordinates(ec_group, point1, x1, y1, ctx);
    EC_POINT_get_affine_coordinates(ec_group, point2, x2, y2, ctx);
    
    // Convert to bytes for comparison
    unsigned char x1_bytes[32], y1_bytes[32];
    unsigned char x2_bytes[32], y2_bytes[32];
    
    memset(x1_bytes, 0, 32);
    memset(y1_bytes, 0, 32);
    memset(x2_bytes, 0, 32);
    memset(y2_bytes, 0, 32);
    
    int len;
    len = BN_num_bytes(x1);
    if (len > 32) len = 32;
    BN_bn2bin(x1, x1_bytes + (32 - len));
    
    len = BN_num_bytes(y1);
    if (len > 32) len = 32;
    BN_bn2bin(y1, y1_bytes + (32 - len));
    
    len = BN_num_bytes(x2);
    if (len > 32) len = 32;
    BN_bn2bin(x2, x2_bytes + (32 - len));
    
    len = BN_num_bytes(y2);
    if (len > 32) len = 32;
    BN_bn2bin(y2, y2_bytes + (32 - len));
    
    // Compute Hamming distance for x and y coordinates
    uint32_t different_bits = 0;
    for (int i = 0; i < 32; i++) {
        unsigned char xor_x = x1_bytes[i] ^ x2_bytes[i];
        unsigned char xor_y = y1_bytes[i] ^ y2_bytes[i];
        
        for (int b = 0; b < 8; b++) {
            if (xor_x & (1 << b)) different_bits++;
            if (xor_y & (1 << b)) different_bits++;
        }
    }
    
    // Normalize to [0, 1] (512 bits total: 256 for x + 256 for y)
    double distance = (double)different_bits / 512.0;
    
    BN_free(x1);
    BN_free(y1);
    BN_free(x2);
    BN_free(y2);
    BN_CTX_free(ctx);
    
    return distance;
}
