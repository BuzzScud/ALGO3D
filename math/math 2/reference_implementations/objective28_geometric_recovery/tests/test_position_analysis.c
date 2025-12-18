#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/obj_mac.h>
#include "geometric_recovery.h"

int main() {
    printf("=== POSITION MAPPING ANALYSIS ===\n\n");
    
    EC_GROUP* curve = EC_GROUP_new_by_curve_name(NID_secp128r1);
    BIGNUM* order = BN_new();
    EC_GROUP_get_order(curve, order, NULL);
    
    // Generate 10 random k values and map them
    printf("Mapping 10 random k values to 13D positions:\n\n");
    
    for (int i = 0; i < 10; i++) {
        BIGNUM* k = BN_new();
        BN_rand_range(k, order);
        
        double position[13];
        map_k_to_position(k, position, 13);
        
        char* k_hex = BN_bn2hex(k);
        printf("k[%d]: %s\n", i, k_hex);
        printf("  Position: [");
        for (int d = 0; d < 13; d++) {
            printf("%.4f", position[d]);
            if (d < 12) printf(", ");
        }
        printf("]\n");
        
        // Check range
        double min_pos = position[0], max_pos = position[0];
        for (int d = 1; d < 13; d++) {
            if (position[d] < min_pos) min_pos = position[d];
            if (position[d] > max_pos) max_pos = position[d];
        }
        printf("  Range: [%.4f, %.4f]\n\n", min_pos, max_pos);
        
        OPENSSL_free(k_hex);
        BN_free(k);
    }
    
    // Test distance between two random positions
    printf("Testing distances between random k values:\n\n");
    
    BIGNUM* k1 = BN_new();
    BIGNUM* k2 = BN_new();
    BN_rand_range(k1, order);
    BN_rand_range(k2, order);
    
    double pos1[13], pos2[13];
    map_k_to_position(k1, pos1, 13);
    map_k_to_position(k2, pos2, 13);
    
    double dist = 0.0;
    for (int d = 0; d < 13; d++) {
        double diff = pos1[d] - pos2[d];
        dist += diff * diff;
    }
    dist = sqrt(dist);
    
    printf("Distance: %.6f\n", dist);
    printf("Expected range: [0, sqrt(13*4)] = [0, %.6f]\n", sqrt(13.0 * 4.0));
    
    BN_free(k1);
    BN_free(k2);
    BN_free(order);
    EC_GROUP_free(curve);
    
    return 0;
}
