/**
 * test_harmonic_folding.c - Test Harmonic Folding on Extended Data
 * 
 * This test:
 * 1. Runs G triangulation for 2000 iterations
 * 2. Applies harmonic folding to k estimates
 * 3. Identifies dominant harmonic frequencies
 * 4. Compares with raw FFT results
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
#include "../include/harmonic_folding.h"
#include "../include/multi_torus_tracker.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"

#define MAX_ITERATIONS 2000
#define NUM_TRAINING 20

/**
 * Run harmonic folding analysis for a specific bit length
 */
void analyze_harmonic_folding(int bit_length) {
    printf("\n========================================\n");
    printf("BIT LENGTH: %d-bit (Harmonic Folding)\n", bit_length);
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
    
    // Allocate array for k estimates
    double* k_estimates = (double*)malloc(MAX_ITERATIONS * sizeof(double));
    if (!k_estimates) {
        printf("  ✗ Failed to allocate k_estimates\n");
        free_g_triangulation_context(ctx);
        EC_GROUP_free((EC_GROUP*)group);
        return;
    }
    
    printf("Phase 1: Collecting %d k estimates...\n\n", MAX_ITERATIONS);
    
    clock_t start = clock();
    
    for (int iteration = 0; iteration < MAX_ITERATIONS; iteration++) {
        // Perform refinement
        perform_refinement_iteration(ctx);
        
        // Compute average k estimate
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
        
        k_estimates[iteration] = total_k_estimate / NUM_TRAINING;
        
        // Print progress every 400 iterations
        if ((iteration + 1) % 400 == 0) {
            printf("  Iteration %4d: avg_k = %.2f\n", iteration + 1, k_estimates[iteration]);
        }
    }
    
    clock_t end = clock();
    double total_time = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("\n✓ Collected %d samples in %.2f seconds\n\n", MAX_ITERATIONS, total_time);
    
    // Phase 2: Apply harmonic folding
    printf("Phase 2: Applying harmonic folding...\n\n");
    
    HarmonicFoldingResult* folding = create_harmonic_folding_result(k_estimates, MAX_ITERATIONS);
    if (!folding) {
        printf("  ✗ Failed to create harmonic folding result\n");
        free(k_estimates);
        free_g_triangulation_context(ctx);
        EC_GROUP_free((EC_GROUP*)group);
        return;
    }
    
    // Apply folding
    if (!apply_harmonic_folding(folding)) {
        printf("  ✗ Failed to apply harmonic folding\n");
        free_harmonic_folding_result(folding);
        free(k_estimates);
        free_g_triangulation_context(ctx);
        EC_GROUP_free((EC_GROUP*)group);
        return;
    }
    
    // Compute energies
    if (!compute_harmonic_energies(folding)) {
        printf("  ✗ Failed to compute harmonic energies\n");
        free_harmonic_folding_result(folding);
        free(k_estimates);
        free_g_triangulation_context(ctx);
        EC_GROUP_free((EC_GROUP*)group);
        return;
    }
    
    // Identify dominant harmonic
    uint32_t dominant = identify_dominant_harmonic(folding);
    printf("✓ Dominant harmonic: %u Hz\n\n", dominant);
    
    // Print analysis
    print_harmonic_folding_analysis(folding);
    
    // Export data
    char filename1[256], filename2[256];
    snprintf(filename1, sizeof(filename1), "harmonic_analysis_%dbit.csv", bit_length);
    snprintf(filename2, sizeof(filename2), "harmonic_signals_%dbit.csv", bit_length);
    
    export_harmonic_folding_csv(folding, filename1);
    export_folded_signals_csv(folding, filename2);
    
    printf("✓ Data exported to %s and %s\n", filename1, filename2);
    
    // Cleanup
    free_harmonic_folding_result(folding);
    free(k_estimates);
    free_g_triangulation_context(ctx);
    EC_GROUP_free((EC_GROUP*)group);
}

/**
 * Main test function
 */
int main(void) {
    printf("=================================================\n");
    printf("Harmonic Folding Analysis\n");
    printf("=================================================\n");
    printf("Applying prime frequency folding to k estimates\n");
    printf("Frequencies: [5, 7, 11, 13, 17, 19, 23, 29, 31]\n\n");
    
    clock_t start = clock();
    
    // Test multiple bit lengths
    int bit_lengths[] = {8, 16, 32};
    int num_bit_lengths = sizeof(bit_lengths) / sizeof(bit_lengths[0]);
    
    for (int i = 0; i < num_bit_lengths; i++) {
        analyze_harmonic_folding(bit_lengths[i]);
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("\n=================================================\n");
    printf("Harmonic Folding Analysis Complete!\n");
    printf("Total time: %.2f seconds\n", elapsed);
    printf("=================================================\n");
    
    return 0;
}