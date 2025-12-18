/**
 * test_per_sample_torus.c - Per-Sample Torus Analysis
 * 
 * This test:
 * 1. Tracks each of 20 training samples individually (not averaged)
 * 2. Computes tori for each sample separately
 * 3. Compares individual sample tori with averaged tori
 * 4. Measures reduction for individual vs averaged
 * 5. Identifies which samples have tightest bounds
 * 
 * Hypothesis: Individual samples have tighter bounds than averaged signal
 * Expected: Better reduction when analyzing samples separately
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
#include "../include/oscillation_decomposition.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"

#define MAX_ITERATIONS 2000
#define NUM_TRAINING 20
#define MAX_TORI 20

/**
 * Per-sample statistics
 */
typedef struct {
    int sample_id;
    uint64_t true_k;
    int num_tori;
    double intersection_k_min;
    double intersection_k_max;
    double intersection_size;
    double reduction_factor;
    bool contains_true_k;
    double tightest_torus_size;
    int tightest_torus_id;
} SampleStats;

/**
 * Analyze tori for a single sample
 */
SampleStats analyze_single_sample(
    int sample_id,
    uint64_t true_k,
    GTriangulationContext* ctx,
    const EC_GROUP* group,
    uint64_t max_k
) {
    SampleStats stats;
    stats.sample_id = sample_id;
    stats.true_k = true_k;
    
    // Create tracker for this sample
    MultiTorusTracker* tracker = create_multi_torus_tracker(MAX_TORI, MAX_ITERATIONS, max_k);
    
    // Collect k estimates for this sample only
    for (int iter = 0; iter < MAX_ITERATIONS; iter++) {
        perform_refinement_iteration(ctx);
        
        // Get k estimate for THIS sample only
        EC_POINT* Q = EC_POINT_new(group);
        BIGNUM* k_bn = BN_new();
        BN_set_word(k_bn, true_k);
        EC_POINT_mul(group, Q, k_bn, NULL, NULL, NULL);
        
        uint64_t k_estimated = estimate_k_from_q(ctx, Q);
        
        // Add to tracker
        multi_torus_add_sample(tracker, (double)k_estimated);
        
        EC_POINT_free(Q);
        BN_free(k_bn);
    }
    
    // Identify tori
    identify_tori(tracker);
    stats.num_tori = tracker->num_tori;
    
    // Compute intersection
    bool success = compute_torus_intersection(tracker);
    
    if (success) {
        stats.intersection_k_min = tracker->intersection_k_min;
        stats.intersection_k_max = tracker->intersection_k_max;
        stats.intersection_size = stats.intersection_k_max - stats.intersection_k_min;
        stats.reduction_factor = (double)max_k / stats.intersection_size;
        stats.contains_true_k = is_k_in_intersection(tracker, (double)true_k);
        
        // Find tightest torus
        stats.tightest_torus_size = 1e20;
        stats.tightest_torus_id = -1;
        
        for (int i = 0; i < tracker->num_tori; i++) {
            double size = tracker->tori[i].k_max - tracker->tori[i].k_min;
            if (size < stats.tightest_torus_size) {
                stats.tightest_torus_size = size;
                stats.tightest_torus_id = i;
            }
        }
    } else {
        stats.intersection_k_min = 0;
        stats.intersection_k_max = 0;
        stats.intersection_size = 0;
        stats.reduction_factor = 0;
        stats.contains_true_k = false;
        stats.tightest_torus_size = 0;
        stats.tightest_torus_id = -1;
    }
    
    free_multi_torus_tracker(tracker);
    
    return stats;
}

/**
 * Test per-sample analysis for a specific bit length
 */
void test_per_sample_analysis(int bit_length) {
    printf("\n========================================\n");
    printf("BIT LENGTH: %d-bit (Per-Sample Analysis)\n", bit_length);
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
    
    printf("Phase 1: Analyzing each of %d samples individually...\n\n", NUM_TRAINING);
    
    clock_t start = clock();
    
    // Analyze each sample
    SampleStats* all_stats = (SampleStats*)malloc(NUM_TRAINING * sizeof(SampleStats));
    
    for (int s = 0; s < NUM_TRAINING; s++) {
        printf("  Sample %2d (k=%lu): ", s+1, training_k[s]);
        fflush(stdout);
        
        all_stats[s] = analyze_single_sample(s, training_k[s], ctx, group, max_k);
        
        printf("tori=%d, reduction=%.2fx, contains_k=%s\n",
               all_stats[s].num_tori,
               all_stats[s].reduction_factor,
               all_stats[s].contains_true_k ? "YES" : "NO");
    }
    
    clock_t end = clock();
    double total_time = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("\n✓ Analyzed %d samples in %.2f seconds\n\n", NUM_TRAINING, total_time);
    
    // Phase 2: Compute statistics
    printf("Phase 2: Computing statistics...\n\n");
    
    double avg_reduction = 0.0;
    double best_reduction = 0.0;
    int best_sample = -1;
    int samples_with_reduction = 0;
    int samples_containing_true_k = 0;
    
    for (int s = 0; s < NUM_TRAINING; s++) {
        if (all_stats[s].reduction_factor > 1.0) {
            avg_reduction += all_stats[s].reduction_factor;
            samples_with_reduction++;
        }
        
        if (all_stats[s].reduction_factor > best_reduction) {
            best_reduction = all_stats[s].reduction_factor;
            best_sample = s;
        }
        
        if (all_stats[s].contains_true_k) {
            samples_containing_true_k++;
        }
    }
    
    if (samples_with_reduction > 0) {
        avg_reduction /= samples_with_reduction;
    }
    
    printf("Summary Statistics:\n");
    printf("  Samples with reduction > 1.0x: %d/%d (%.1f%%)\n",
           samples_with_reduction, NUM_TRAINING,
           100.0 * samples_with_reduction / NUM_TRAINING);
    printf("  Average reduction (for samples > 1.0x): %.2fx\n", avg_reduction);
    printf("  Best reduction: %.2fx (sample %d, k=%lu)\n",
           best_reduction, best_sample+1, training_k[best_sample]);
    printf("  Samples containing true k: %d/%d (%.1f%%)\n\n",
           samples_containing_true_k, NUM_TRAINING,
           100.0 * samples_containing_true_k / NUM_TRAINING);
    
    // Phase 3: Compare with averaged approach
    printf("Phase 3: Comparing with averaged approach...\n\n");
    
    // Compute averaged tori (current approach)
    MultiTorusTracker* avg_tracker = create_multi_torus_tracker(MAX_TORI, MAX_ITERATIONS, max_k);
    
    for (int iter = 0; iter < MAX_ITERATIONS; iter++) {
        perform_refinement_iteration(ctx);
        
        double total_k = 0.0;
        for (int i = 0; i < NUM_TRAINING; i++) {
            EC_POINT* Q = EC_POINT_new(group);
            BIGNUM* k_bn = BN_new();
            BN_set_word(k_bn, training_k[i]);
            EC_POINT_mul(group, Q, k_bn, NULL, NULL, NULL);
            
            uint64_t k_estimated = estimate_k_from_q(ctx, Q);
            total_k += (double)k_estimated;
            
            EC_POINT_free(Q);
            BN_free(k_bn);
        }
        
        multi_torus_add_sample(avg_tracker, total_k / NUM_TRAINING);
    }
    
    identify_tori(avg_tracker);
    compute_torus_intersection(avg_tracker);
    
    double avg_intersection_size = avg_tracker->intersection_k_max - avg_tracker->intersection_k_min;
    double avg_approach_reduction = (double)max_k / avg_intersection_size;
    
    printf("Averaged Approach:\n");
    printf("  Tori detected: %d\n", avg_tracker->num_tori);
    printf("  Intersection size: %.2f\n", avg_intersection_size);
    printf("  Reduction: %.2fx\n\n", avg_approach_reduction);
    
    printf("Per-Sample Approach:\n");
    printf("  Average reduction: %.2fx\n", avg_reduction);
    printf("  Best reduction: %.2fx\n\n", best_reduction);
    
    printf("Comparison:\n");
    printf("  Per-sample avg vs averaged: %.2fx improvement\n",
           avg_reduction / avg_approach_reduction);
    printf("  Per-sample best vs averaged: %.2fx improvement\n",
           best_reduction / avg_approach_reduction);
    
    // Export data
    char filename[256];
    snprintf(filename, sizeof(filename), "per_sample_%dbit.csv", bit_length);
    FILE* fp = fopen(filename, "w");
    if (fp) {
        fprintf(fp, "sample_id,true_k,num_tori,intersection_size,reduction_factor,contains_true_k,tightest_torus_size\n");
        for (int s = 0; s < NUM_TRAINING; s++) {
            fprintf(fp, "%d,%lu,%d,%.6f,%.6f,%d,%.6f\n",
                    all_stats[s].sample_id,
                    all_stats[s].true_k,
                    all_stats[s].num_tori,
                    all_stats[s].intersection_size,
                    all_stats[s].reduction_factor,
                    all_stats[s].contains_true_k ? 1 : 0,
                    all_stats[s].tightest_torus_size);
        }
        fclose(fp);
        printf("\n✓ Data exported to %s\n", filename);
    }
    
    // Cleanup
    free(all_stats);
    free_multi_torus_tracker(avg_tracker);
    free_g_triangulation_context(ctx);
    EC_GROUP_free((EC_GROUP*)group);
}

/**
 * Main test function
 */
int main(void) {
    printf("=================================================\n");
    printf("Per-Sample Torus Analysis\n");
    printf("=================================================\n");
    printf("Analyzing each training sample individually\n");
    printf("Comparing with averaged approach\n\n");
    
    clock_t start = clock();
    
    // Test multiple bit lengths
    int bit_lengths[] = {8, 16, 32};
    int num_bit_lengths = sizeof(bit_lengths) / sizeof(bit_lengths[0]);
    
    for (int i = 0; i < num_bit_lengths; i++) {
        test_per_sample_analysis(bit_lengths[i]);
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("\n=================================================\n");
    printf("Per-Sample Torus Analysis Complete!\n");
    printf("Total time: %.2f seconds\n", elapsed);
    printf("=================================================\n");
    
    return 0;
}