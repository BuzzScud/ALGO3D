/**
 * @file geometric_utils.c
 * @brief Utility functions for k↔position mapping with +1 bit extension
 */

#include "geometric_recovery.h"
#include <string.h>
#include "math/arithmetic.h"
#include "math/transcendental.h"

/**
 * Map k (BIGNUM) to position in high-dimensional space
 * 
 * CRITICAL: Uses 257+ bits (256 + 1 for boundary crossing)
 * Each additional scalar adds +1 bit.
 */
void map_k_to_position(
    BIGNUM* k,
    double* position_out,
    uint32_t num_dimensions
) {
    // Extract k bytes (use 257 bits = 33 bytes)
    unsigned char k_bytes[33];
    memset(k_bytes, 0, 33);
    
    int len = BN_num_bytes(k);
    if (len > 33) len = 33;
    BN_bn2bin(k, k_bytes + (33 - len));
    
    // Map 257 bits to num_dimensions
    // Each dimension gets 257/num_dimensions bits
    uint32_t bits_per_dim = 257 / num_dimensions;
    if (bits_per_dim < 1) bits_per_dim = 1;
    
    for (uint32_t d = 0; d < num_dimensions; d++) {
        uint64_t value = 0;
        uint32_t bit_start = d * bits_per_dim;
        
        // Extract bits for this dimension
        for (uint32_t b = 0; b < bits_per_dim && bit_start + b < 257; b++) {
            uint32_t byte_idx = (bit_start + b) / 8;
            uint32_t bit_idx = (bit_start + b) % 8;
            
            if (byte_idx < 33 && (k_bytes[byte_idx] & (1 << (7 - bit_idx)))) {
                value |= (1ULL << b);
            }
        }
        
        // Normalize to [-1, 1] (centered at 0 for quadrant detection)
        uint64_t max_value = (1ULL << bits_per_dim) - 1;
        position_out[d] = 2.0 * ((double)value / (double)max_value) - 1.0;
    }
}

/**
 * Map position to k (BIGNUM) with +1 bit extension
 * 
 * CRITICAL: Generates 257-bit k (256 + 1 for boundary)
 */
/**
 * Find k nearest anchors to a position
 */
static void find_k_nearest_anchors(
    const double* position,
    const double** anchor_positions,
    uint32_t num_anchors,
    uint32_t num_dimensions,
    uint32_t k,
    uint32_t* nearest_indices,
    double* nearest_distances
) {
    // Initialize with worst possible values
    for (uint32_t i = 0; i < k; i++) {
        nearest_indices[i] = 0;
        nearest_distances[i] = 1e100;
    }
    
    // Find k nearest
    for (uint32_t a = 0; a < num_anchors; a++) {
        // Compute distance
        double dist = 0.0;
        for (uint32_t d = 0; d < num_dimensions; d++) {
            double diff = position[d] - anchor_positions[a][d];
            dist += diff * diff;
        }
        dist = math_sqrt(dist);
        
        // Insert if closer than current k-th nearest
        for (uint32_t i = 0; i < k; i++) {
            if (dist < nearest_distances[i]) {
                // Shift down
                for (uint32_t j = k - 1; j > i; j--) {
                    nearest_indices[j] = nearest_indices[j - 1];
                    nearest_distances[j] = nearest_distances[j - 1];
                }
                // Insert
                nearest_indices[i] = a;
                nearest_distances[i] = dist;
                break;
            }
        }
    }
}

/**
 * Triangulate k from anchors using weighted interpolation
 * 
 * This is the CORRECT approach - use actual anchor k values!
 */
BIGNUM* triangulate_k_from_anchors(
    const double* position,
    const double** anchor_positions,
    const BIGNUM** anchor_k_values,
    uint32_t num_anchors,
    uint32_t num_dimensions,
    EC_GROUP* ec_group
) {
    if (num_anchors == 0) return NULL;
    
    // Find 3 nearest anchors (or fewer if not enough anchors)
    uint32_t k = (num_anchors >= 3) ? 3 : num_anchors;
    uint32_t nearest[3];
    double distances[3];
    
    find_k_nearest_anchors(position, anchor_positions, num_anchors, 
                          num_dimensions, k, nearest, distances);
    
    // Compute weights (inverse distance)
    double weights[3];
    double total_weight = 0.0;
    
    for (uint32_t i = 0; i < k; i++) {
        // Use inverse distance squared for sharper weighting
        weights[i] = 1.0 / (distances[i] * distances[i] + 1e-10);
        total_weight += weights[i];
    }
    
    // Normalize weights
    for (uint32_t i = 0; i < k; i++) {
        weights[i] /= total_weight;
    }
    
    // Interpolate k values using weighted sum
    // k_result = w1*k1 + w2*k2 + w3*k3
    
    BIGNUM* result = BN_new();
    BN_zero(result);
    
    BIGNUM* order = BN_new();
    BN_CTX* ctx = BN_CTX_new();
    EC_GROUP_get_order(ec_group, order, ctx);
    
    // Scale factor for fixed-point arithmetic (use 10^9 for precision)
    const uint64_t SCALE = 1000000000ULL;
    
    for (uint32_t i = 0; i < k; i++) {
        // Compute weighted_k = k * weight
        BIGNUM* weighted_k = BN_dup(anchor_k_values[nearest[i]]);
        
        // Multiply by scaled weight
        uint64_t scaled_weight = (uint64_t)(weights[i] * SCALE);
        BN_mul_word(weighted_k, scaled_weight);
        
        // Add to result
        BN_add(result, result, weighted_k);
        BN_free(weighted_k);
    }
    
    // Divide by scale factor
    BN_div_word(result, SCALE);
    
    // Reduce modulo order
    BN_mod(result, result, order, ctx);
    
    BN_free(order);
    BN_CTX_free(ctx);
    
    return result;
}

/**
 * Legacy function - kept for compatibility but should not be used
 * Use triangulate_k_from_anchors() instead!
 */
BIGNUM* map_position_to_k(
    const double* position,
    uint32_t num_dimensions
) {
    (void)position;
    (void)num_dimensions;
    // This function is DEPRECATED and should not be used!
    // It creates arbitrary k values that have no relationship to anchor k values.
    // Use triangulate_k_from_anchors() instead!
    
    fprintf(stderr, "WARNING: map_position_to_k() is deprecated! Use triangulate_k_from_anchors()\n");
    
    // Return NULL to force proper usage
    return NULL;
}

/**
 * Compute Hamming distance between two k values
 * 
 * Returns distance in range [0, 1] where 0 = identical, 1 = completely different
 */
double compute_k_hamming_distance(BIGNUM* k1, BIGNUM* k2) {
    unsigned char k1_bytes[33], k2_bytes[33];
    memset(k1_bytes, 0, 33);
    memset(k2_bytes, 0, 33);
    
    int len1 = BN_num_bytes(k1);
    int len2 = BN_num_bytes(k2);
    if (len1 > 33) len1 = 33;
    if (len2 > 33) len2 = 33;
    
    BN_bn2bin(k1, k1_bytes + (33 - len1));
    BN_bn2bin(k2, k2_bytes + (33 - len2));
    
    uint32_t different_bits = 0;
    for (int i = 0; i < 33; i++) {
        unsigned char xor_byte = k1_bytes[i] ^ k2_bytes[i];
        for (int b = 0; b < 8; b++) {
            if (xor_byte & (1 << b)) different_bits++;
        }
    }
    
    // 257 bits total
    return (double)different_bits / 257.0;
}
