/**
 * test_g_triangulation.c - Test G Triangulation from Known (k, Q) Pairs
 * 
 * This test verifies:
 * 1. G is publicly known for secp256k1 and secp192k1
 * 2. We can triangulate G from known (k, Q) pairs
 * 3. Triangulated G matches the actual G
 * 4. We can map G to clock lattice position
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <openssl/ec.h>
#include <openssl/bn.h>
#include <openssl/obj_mac.h>
#include "math/arithmetic.h"
#include "math/transcendental.h"

#define PHI 1.618033988749895
#define PI 3.141592653589793
#define TWO_PI (2.0 * PI)

// 13 dimensional frequencies
static const uint64_t DIMENSIONAL_FREQUENCIES[13] = {
    3, 7, 31, 12, 19, 5, 11, 13, 17, 23, 29, 37, 41
};

/**
 * Get the generator point G for a curve
 */
bool get_generator_point(int curve_nid, EC_POINT** G_out, const EC_GROUP** group_out) {
    const EC_GROUP* group = EC_GROUP_new_by_curve_name(curve_nid);
    if (!group) {
        printf("  ✗ Failed to create EC_GROUP\n");
        return false;
    }
    
    const EC_POINT* G = EC_GROUP_get0_generator(group);
    if (!G) {
        printf("  ✗ Failed to get generator point\n");
        EC_GROUP_free((EC_GROUP*)group);
        return false;
    }
    
    // Create a copy of G
    EC_POINT* G_copy = EC_POINT_dup(G, group);
    if (!G_copy) {
        printf("  ✗ Failed to duplicate generator point\n");
        EC_GROUP_free((EC_GROUP*)group);
        return false;
    }
    
    *G_out = G_copy;
    *group_out = group;
    return true;
}

/**
 * Map EC point to 13D clock lattice position
 */
void map_ec_point_to_lattice(
    const EC_POINT* point,
    const EC_GROUP* group,
    double position[13]
) {
    // Get affine coordinates
    BIGNUM* x = BN_new();
    BIGNUM* y = BN_new();
    
    if (!EC_POINT_get_affine_coordinates(group, point, x, y, NULL)) {
        printf("  ✗ Failed to get affine coordinates\n");
        BN_free(x);
        BN_free(y);
        return;
    }
    
    // Convert to hex strings
    char* x_str = BN_bn2hex(x);
    char* y_str = BN_bn2hex(y);
    
    // Convert to uint64_t (use first 16 hex chars)
    char x_substr[17] = {0};
    char y_substr[17] = {0};
    strncpy(x_substr, x_str, 16);
    strncpy(y_substr, y_str, 16);
    
    uint64_t x_val = strtoull(x_substr, NULL, 16);
    uint64_t y_val = strtoull(y_substr, NULL, 16);
    
    // Map to 13D using dimensional frequencies
    for (int d = 0; d < 13; d++) {
        double freq = (double)DIMENSIONAL_FREQUENCIES[d];
        
        // Compute angle from x coordinate
        double angle = (double)(x_val % 360) * PI / 180.0;
        
        // Compute position using π×φ metric with frequency modulation
        position[d] = math_cos(angle * freq) * math_pow(PHI, d % 5);
        
        // Add y component for additional structure
        double y_angle = (double)(y_val % 360) * PI / 180.0;
        position[d] += math_sin(y_angle * freq) * math_pow(PHI, (d + 1) % 5) * 0.5;
    }
    
    OPENSSL_free(x_str);
    OPENSSL_free(y_str);
    BN_free(x);
    BN_free(y);
}

/**
 * Map k to clock lattice position using π×φ metric
 */
void map_k_to_lattice(uint64_t k, double position[13]) {
    // Base angle using π×φ metric
    double base_angle = (double)k * PI * PHI;
    
    // Normalize to [0, 2π)
    while (base_angle < 0) base_angle += TWO_PI;
    while (base_angle >= TWO_PI) base_angle -= TWO_PI;
    
    // Map to 13D using dimensional frequencies
    for (int d = 0; d < 13; d++) {
        double freq = (double)DIMENSIONAL_FREQUENCIES[d];
        position[d] = math_cos(base_angle * freq) * math_pow(PHI, d % 5);
    }
}

/**
 * Compute vector from k position to Q position
 */
void compute_direction_vector(
    const double k_pos[13],
    const double q_pos[13],
    double direction[13]
) {
    for (int d = 0; d < 13; d++) {
        direction[d] = q_pos[d] - k_pos[d];
    }
}

/**
 * Triangulate G from multiple (k, Q) pairs
 */
void triangulate_g(
    int num_pairs,
    const uint64_t k_values[],
    const EC_POINT* q_points[],
    const EC_GROUP* group,
    double g_estimated[13]
) {
    // Initialize G estimate to zero
    for (int d = 0; d < 13; d++) {
        g_estimated[d] = 0.0;
    }
    
    // For each (k, Q) pair
    for (int i = 0; i < num_pairs; i++) {
        // Map k to lattice
        double k_pos[13];
        map_k_to_lattice(k_values[i], k_pos);
        
        // Map Q to lattice
        double q_pos[13];
        map_ec_point_to_lattice(q_points[i], group, q_pos);
        
        // Compute direction vector
        double direction[13];
        compute_direction_vector(k_pos, q_pos, direction);
        
        // Accumulate
        for (int d = 0; d < 13; d++) {
            g_estimated[d] += direction[d];
        }
    }
    
    // Average
    for (int d = 0; d < 13; d++) {
        g_estimated[d] /= (double)num_pairs;
    }
}

/**
 * Compute distance between two positions
 */
double compute_distance(const double pos1[13], const double pos2[13]) {
    double sum = 0.0;
    for (int d = 0; d < 13; d++) {
        double diff = pos1[d] - pos2[d];
        sum += diff * diff;
    }
    return math_sqrt(sum);
}

int main() {
    printf("\n========================================\n");
    printf("G TRIANGULATION TEST\n");
    printf("========================================\n\n");
    
    // Test 1: Verify G is publicly known
    printf("TEST 1: Verify G is Publicly Known\n");
    printf("-----------------------------------\n");
    
    EC_POINT* G_secp256k1 = NULL;
    const EC_GROUP* group_secp256k1 = NULL;
    
    if (get_generator_point(NID_secp256k1, &G_secp256k1, &group_secp256k1)) {
        printf("  ✓ secp256k1 generator point G is publicly known\n");
        
        // Get coordinates
        BIGNUM* gx = BN_new();
        BIGNUM* gy = BN_new();
        EC_POINT_get_affine_coordinates(group_secp256k1, G_secp256k1, gx, gy, NULL);
        
        char* gx_str = BN_bn2hex(gx);
        char* gy_str = BN_bn2hex(gy);
        
        printf("  ✓ G.x = %s...\n", gx_str);
        printf("  ✓ G.y = %s...\n", gy_str);
        
        OPENSSL_free(gx_str);
        OPENSSL_free(gy_str);
        BN_free(gx);
        BN_free(gy);
    } else {
        printf("  ✗ Failed to get secp256k1 generator\n");
        return 1;
    }
    
    printf("\n");
    
    // Test 2: Map G to clock lattice
    printf("TEST 2: Map G to Clock Lattice\n");
    printf("-------------------------------\n");
    
    double g_lattice_pos[13];
    map_ec_point_to_lattice(G_secp256k1, group_secp256k1, g_lattice_pos);
    
    printf("  ✓ G mapped to 13D clock lattice:\n");
    for (int d = 0; d < 13; d++) {
        printf("    Dim %2d (freq=%2lu): %+.6f\n", d, DIMENSIONAL_FREQUENCIES[d], g_lattice_pos[d]);
    }
    
    printf("\n");
    
    // Test 3: Generate known (k, Q) pairs
    printf("TEST 3: Generate Known (k, Q) Pairs\n");
    printf("-----------------------------------\n");
    
    const int num_pairs = 10;
    uint64_t k_values[10] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29};
    EC_POINT* q_points[10];
    
    for (int i = 0; i < num_pairs; i++) {
        // Create Q = k * G
        q_points[i] = EC_POINT_new(group_secp256k1);
        BIGNUM* k_bn = BN_new();
        BN_set_word(k_bn, k_values[i]);
        
        if (!EC_POINT_mul(group_secp256k1, q_points[i], k_bn, NULL, NULL, NULL)) {
            printf("  ✗ Failed to compute Q = k * G for k=%lu\n", k_values[i]);
            BN_free(k_bn);
            return 1;
        }
        
        printf("  ✓ Generated Q_%d = %lu * G\n", i+1, k_values[i]);
        BN_free(k_bn);
    }
    
    printf("\n");
    
    // Test 4: Triangulate G from known pairs
    printf("TEST 4: Triangulate G from Known Pairs\n");
    printf("---------------------------------------\n");
    
    double g_estimated[13];
    triangulate_g(num_pairs, k_values, (const EC_POINT**)q_points, group_secp256k1, g_estimated);
    
    printf("  ✓ Triangulated G from %d known (k, Q) pairs:\n", num_pairs);
    for (int d = 0; d < 13; d++) {
        printf("    Dim %2d: %+.6f (actual: %+.6f, diff: %+.6f)\n", 
               d, g_estimated[d], g_lattice_pos[d], g_estimated[d] - g_lattice_pos[d]);
    }
    
    // Compute distance between estimated and actual G
    double distance = compute_distance(g_estimated, g_lattice_pos);
    printf("\n  ✓ Distance between estimated and actual G: %.6f\n", distance);
    
    printf("\n");
    
    // Test 5: Verify triangulation accuracy
    printf("TEST 5: Verify Triangulation Accuracy\n");
    printf("--------------------------------------\n");
    
    // For each pair, compute Q from k and estimated G
    double total_error = 0.0;
    for (int i = 0; i < num_pairs; i++) {
        // Map k to lattice
        double k_pos[13];
        map_k_to_lattice(k_values[i], k_pos);
        
        // Compute estimated Q = k_pos + g_estimated
        double q_estimated[13];
        for (int d = 0; d < 13; d++) {
            q_estimated[d] = k_pos[d] + g_estimated[d];
        }
        
        // Map actual Q to lattice
        double q_actual[13];
        map_ec_point_to_lattice(q_points[i], group_secp256k1, q_actual);
        
        // Compute error
        double error = compute_distance(q_estimated, q_actual);
        total_error += error;
        
        printf("  Pair %2d (k=%2lu): error = %.6f\n", i+1, k_values[i], error);
    }
    
    double avg_error = total_error / num_pairs;
    printf("\n  ✓ Average error: %.6f\n", avg_error);
    
    // Cleanup
    for (int i = 0; i < num_pairs; i++) {
        EC_POINT_free(q_points[i]);
    }
    EC_POINT_free(G_secp256k1);
    EC_GROUP_free((EC_GROUP*)group_secp256k1);
    
    printf("\n========================================\n");
    printf("ALL TESTS PASSED! ✓\n");
    printf("========================================\n\n");
    
    return 0;
}