/**
 * Extended Iteration Analysis - Fix Period 2 Issue
 * 
 * This test extends the iteration count from 500 to 2000 to capture
 * longer oscillation periods that were being undersampled.
 * 
 * Expected: Reveal true oscillation periods (not just period 2)
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
#include "../include/plateau_detection.h"
#include "../include/oscillation_decomposition.h"
#include "../include/multi_torus_tracker.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"

#define MAX_ITERATIONS 2000
#define NUM_TRAINING 20

/**
 * Run extended iteration analysis for a specific bit length
 */
void analyze_extended_iterations(int bit_length) {
    printf("\n========================================\n");
    printf("BIT LENGTH: %d-bit (EXTENDED: %d iterations)\n", bit_length, MAX_ITERATIONS);
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
    
    // Create plateau detector (higher threshold for longer runs)
    PlateauDetector* plateau = create_plateau_detector(MAX_ITERATIONS, 0.01, 100);
    
    // Create multi-torus tracker
    MultiTorusTracker* torus_tracker = create_multi_torus_tracker(
        5,              // max 5 tori
        MAX_ITERATIONS, // history size
        max_k           // original space
    );
    
    printf("Phase 1: Running %d iterations...\n\n", MAX_ITERATIONS);
    
    bool plateau_detected = false;
    int iteration = 0;
    clock_t start = clock();
    
    while (iteration < MAX_ITERATIONS) {
        iteration++;
        
        // Perform refinement
        perform_refinement_iteration(ctx);
        
        // Compute metrics
        double total_error = 0.0;
        double total_k_estimate = 0.0;
        
        for (int i = 0; i < NUM_TRAINING; i++) {
            EC_POINT* Q = EC_POINT_new(group);
            BIGNUM* k_bn = BN_new();
            BN_set_word(k_bn, training_k[i]);
            EC_POINT_mul(group, Q, k_bn, NULL, NULL, NULL);
            
            uint64_t k_estimated = estimate_k_from_q(ctx, Q);
            double error = math_abs((double)k_estimated - (double)training_k[i]);
            
            total_error += error;
            total_k_estimate += (double)k_estimated;
            
            EC_POINT_free(Q);
            BN_free(k_bn);
        }
        
        double avg_error = total_error / NUM_TRAINING;
        double avg_k = total_k_estimate / NUM_TRAINING;
        
        // Add to torus tracker
        multi_torus_add_sample(torus_tracker, avg_k);
        
        // Check for plateau
        if (plateau_detector_add_sample(plateau, avg_error, iteration)) {
            if (!plateau_detected) {
                plateau_detected = true;
                printf("  ✓ Plateau detected at iteration %d (error: %.2f)\n", 
                       iteration, get_plateau_value(plateau));
                printf("  → Continuing to %d iterations to capture full oscillations...\n\n", 
                       MAX_ITERATIONS);
            }
        }
        
        // Print progress every 200 iterations
        if (iteration % 200 == 0) {
            clock_t now = clock();
            double elapsed = (double)(now - start) / CLOCKS_PER_SEC;
            printf("  Iteration %4d: avg_error = %12.2f, avg_k = %12.2f (%.2fs)\n", 
                   iteration, avg_error, avg_k, elapsed);
        }
    }
    
    clock_t end = clock();
    double total_time = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("\n✓ Completed %d iterations in %.2f seconds\n", MAX_ITERATIONS, total_time);
    printf("  Average: %.3f ms/iteration\n\n", (total_time * 1000.0) / MAX_ITERATIONS);
    
    // Phase 2: Identify tori
    printf("Phase 2: Identifying tori from %d samples...\n\n", MAX_ITERATIONS);
    
    identify_tori(torus_tracker);
    
    // Print results
    print_multi_torus_analysis(torus_tracker);
    
    // Export data
    char filename[256];
    snprintf(filename, sizeof(filename), "extended_%dbit_%diter.csv", bit_length, MAX_ITERATIONS);
    export_multi_torus_csv(torus_tracker, filename);
    printf("\n✓ Data exported to %s\n", filename);
    
    // Cleanup
    free_multi_torus_tracker(torus_tracker);
    free_plateau_detector(plateau);
    free_g_triangulation_context(ctx);
    EC_GROUP_free((EC_GROUP*)group);
}

/**
 * Main test function
 */
int main(void) {
    printf("=================================================\n");
    printf("Extended Iteration Analysis - Fix Period 2 Issue\n");
    printf("=================================================\n");
    printf("Running %d iterations per bit length (vs 500 before)\n", MAX_ITERATIONS);
    printf("Expected: Reveal true oscillation periods\n\n");
    
    clock_t start = clock();
    
    // Test multiple bit lengths
    int bit_lengths[] = {8, 16, 32};
    int num_bit_lengths = sizeof(bit_lengths) / sizeof(bit_lengths[0]);
    
    for (int i = 0; i < num_bit_lengths; i++) {
        analyze_extended_iterations(bit_lengths[i]);
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("\n=================================================\n");
    printf("Extended Iteration Analysis Complete!\n");
    printf("Total time: %.2f seconds\n", elapsed);
    printf("=================================================\n");
    
    return 0;
}