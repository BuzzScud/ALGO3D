/**
 * test_torus_intersection.c - Test Torus Intersection Computation
 * 
 * This test:
 * 1. Detects 20 tori
 * 2. Computes intersection for different numbers of tori (5, 10, 15, 20)
 * 3. Measures search space reduction for each
 * 4. Compares results to find optimal number
 * 5. Validates that true k is in intersection
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <openssl/ec.h>
#include <openssl/bn.h>
#include <openssl/obj_mac.h>
#include "../include/g_triangulation.h"
#include "../include/multi_torus_tracker.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"

#define MAX_ITERATIONS 2000
#define NUM_TRAINING 20
#define MAX_TORI 20

/**
 * Test intersection for a specific bit length
 */
void test_intersection_for_bit_length(int bit_length) {
    printf("\n========================================\n");
    printf("BIT LENGTH: %d-bit (Intersection Analysis)\n", bit_length);
    printf("========================================\n\n");
    
    uint64_t max_k = (1ULL << bit_length) - 1;
    
    // Generate training k values
    uint64_t training_k[NUM_TRAINING];
    uint64_t primes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71};
    
    for (int i = 0; i < NUM_TRAINING; i++) {
        training_k[i] = (primes[i] * max_k) / 100;
        if (training_k[i] == 0) training_k[i] = primes[i];
        if (training_k[i] > max_k) training_k[i] = max_k;
    }
    
    // Compute average true k for validation
    double avg_true_k = 0.0;
    for (int i = 0; i < NUM_TRAINING; i++) {
        avg_true_k += (double)training_k[i];
    }
    avg_true_k /= NUM_TRAINING;
    
    // Create context
    GTriangulationContext* ctx = create_g_triangulation_context(
        NID_secp256k1,
        NUM_TRAINING,
        training_k,
        MAX_ITERATIONS
    );
    
    if (!ctx) {
        printf("  ✗ Failed to create context\n");
        return;
    }
    
    const EC_GROUP* group = EC_GROUP_new_by_curve_name(NID_secp256k1);
    
    // Create multi-torus tracker
    MultiTorusTracker* torus_tracker = create_multi_torus_tracker(
        MAX_TORI,
        MAX_ITERATIONS,
        max_k
    );
    
    printf("Phase 1: Collecting %d k estimates...\n", MAX_ITERATIONS);
    
    clock_t start = clock();
    
    for (int iteration = 0; iteration < MAX_ITERATIONS; iteration++) {
        perform_refinement_iteration(ctx);
        
        double total_k_estimate = 0.0;
        
        for (int i = 0; i < NUM_TRAINING; i++) {
            EC_POINT* Q = EC_POINT_new(group);
            BIGNUM* k_bn = BN_new();
            BN_set_word(k_bn, training_k[i]);
            EC_POINT_mul(group, Q, k_bn, NULL, NULL, NULL);
            
            uint64_t k_estimated = estimate_k_from_q(ctx, Q);
            total_k_estimate += (double)k_estimated;
            
            EC_POINT_free(Q);
            BN_free(k_bn);
        }
        
        double avg_k = total_k_estimate / NUM_TRAINING;
        multi_torus_add_sample(torus_tracker, avg_k);
    }
    
    clock_t end = clock();
    double total_time = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("✓ Collected %d samples in %.2f seconds\n\n", MAX_ITERATIONS, total_time);
    
    // Phase 2: Identify tori
    printf("Phase 2: Identifying tori...\n");
    identify_tori(torus_tracker);
    printf("✓ Detected %d tori\n\n", torus_tracker->num_tori);
    
    // Phase 3: Compute intersection using existing function
    printf("Phase 3: Computing intersection of all %d tori...\n\n", torus_tracker->num_tori);
    
    bool success = compute_torus_intersection(torus_tracker);
    
    if (success) {
        printf("✓ Intersection computed successfully\n\n");
        
        // Get intersection bounds
        double k_min = torus_tracker->intersection_k_min;
        double k_max = torus_tracker->intersection_k_max;
        double size = k_max - k_min;
        
        // Check validity
        bool is_valid = (k_max > k_min);
        
        printf("Intersection Analysis:\n");
        printf("  Valid: %s\n", is_valid ? "YES" : "NO");
        
        if (is_valid) {
            printf("  Bounds: [%.2f, %.2f]\n", k_min, k_max);
            printf("  Size: %.2f\n", size);
            
            // Compute reduction
            double reduction_factor = (double)max_k / size;
            double reduction_percent = (1.0 - (size / (double)max_k)) * 100.0;
            
            printf("  Reduction: %.2fx (%.2f%%)\n", reduction_factor, reduction_percent);
            
            // Check if true k is in intersection
            bool contains_true_k = is_k_in_intersection(torus_tracker, avg_true_k);
            printf("  Contains true k: %s\n", contains_true_k ? "YES" : "NO");
            
            // Export
            char filename[256];
            snprintf(filename, sizeof(filename), "intersection_%dbit.csv", bit_length);
            FILE* fp = fopen(filename, "w");
            if (fp) {
                fprintf(fp, "k_min,k_max,size,original_space,reduction_factor,reduction_percent\n");
                fprintf(fp, "%.6f,%.6f,%.6f,%lu,%.6f,%.2f\n",
                        k_min, k_max, size, max_k, reduction_factor, reduction_percent);
                fclose(fp);
                printf("\n✓ Data exported to %s\n", filename);
            }
        } else {
            printf("  Intersection is EMPTY - tori do not overlap!\n");
        }
    } else {
        printf("✗ Failed to compute intersection\n");
    }
    
    // Cleanup
    free_multi_torus_tracker(torus_tracker);
    free_g_triangulation_context(ctx);
    EC_GROUP_free((EC_GROUP*)group);
}

/**
 * Main test function
 */
int main(void) {
    printf("=================================================\n");
    printf("Torus Intersection Analysis\n");
    printf("=================================================\n");
    printf("Computing intersections for 5, 10, 15, 20 tori\n");
    printf("Measuring search space reduction\n\n");
    
    clock_t start = clock();
    
    // Test multiple bit lengths
    int bit_lengths[] = {8, 16, 32};
    int num_bit_lengths = sizeof(bit_lengths) / sizeof(bit_lengths[0]);
    
    for (int i = 0; i < num_bit_lengths; i++) {
        test_intersection_for_bit_length(bit_lengths[i]);
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("\n=================================================\n");
    printf("Torus Intersection Analysis Complete!\n");
    printf("Total time: %.2f seconds\n", elapsed);
    printf("=================================================\n");
    
    return 0;
}