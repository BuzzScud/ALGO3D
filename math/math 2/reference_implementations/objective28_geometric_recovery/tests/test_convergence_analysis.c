/**
 * test_convergence_analysis.c - Analyze Convergence and Plateau Behavior
 * 
 * This test runs many iterations to determine:
 * 1. Does the algorithm solve G (converge to 0 error)?
 * 2. Does it plateau at some error level?
 * 3. Does it continue to oscillate after plateau?
 * 4. How does bit length affect convergence?
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <openssl/ec.h>
#include <openssl/bn.h>
#include <openssl/obj_mac.h>
#include "../include/g_triangulation.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"

typedef struct {
    int iteration;
    double avg_error;
    double min_error;
    double max_error;
    double std_dev;
    int successes;
    double success_rate;
} IterationMetrics;

void compute_metrics(
    GTriangulationContext* ctx,
    const EC_GROUP* group,
    int num_pairs,
    const uint64_t* training_k,
    IterationMetrics* metrics,
    int iteration
) {
    double total_error = 0.0;
    double min_error = 1e9;
    double max_error = 0.0;
    int successes = 0;
    double errors[100];
    
    for (int i = 0; i < num_pairs; i++) {
        // Compute Q = k * G
        EC_POINT* Q = EC_POINT_new(group);
        BIGNUM* k_bn = BN_new();
        BN_set_word(k_bn, training_k[i]);
        EC_POINT_mul(group, Q, k_bn, NULL, NULL, NULL);
        
        // Estimate k
        uint64_t k_estimated = estimate_k_from_q(ctx, Q);
        
        // Compute error
        double error = math_abs((double)k_estimated - (double)training_k[i]);
        double error_pct = (error / (double)training_k[i]) * 100.0;
        
        errors[i] = error;
        total_error += error;
        
        if (error < min_error) min_error = error;
        if (error > max_error) max_error = error;
        
        if (error_pct <= 10.0) successes++;
        
        BN_free(k_bn);
        EC_POINT_free(Q);
    }
    
    // Compute average
    double avg_error = total_error / num_pairs;
    
    // Compute standard deviation
    double variance = 0.0;
    for (int i = 0; i < num_pairs; i++) {
        double diff = errors[i] - avg_error;
        variance += diff * diff;
    }
    double std_dev = math_sqrt(variance / num_pairs);
    
    // Fill metrics
    metrics->iteration = iteration;
    metrics->avg_error = avg_error;
    metrics->min_error = min_error;
    metrics->max_error = max_error;
    metrics->std_dev = std_dev;
    metrics->successes = successes;
    metrics->success_rate = (double)successes / num_pairs * 100.0;
}

int main() {
    printf("\n========================================\n");
    printf("CONVERGENCE ANALYSIS TEST\n");
    printf("========================================\n\n");
    
    // Test with different bit lengths
    const int num_bit_lengths = 4;
    int bit_lengths[4] = {8, 16, 32, 64};
    
    for (int bl = 0; bl < num_bit_lengths; bl++) {
        int bit_length = bit_lengths[bl];
        uint64_t max_k = (1ULL << bit_length) - 1;
        
        printf("\n========================================\n");
        printf("BIT LENGTH: %d-bit (max k = %lu)\n", bit_length, max_k);
        printf("========================================\n\n");
        
        // Generate training k values in this bit range
        const int num_training = 20;
        uint64_t training_k[20];
        
        // Use primes distributed across the range
        uint64_t primes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71};
        for (int i = 0; i < num_training; i++) {
            // Scale primes to fit in bit range
            training_k[i] = (primes[i] * max_k) / 100;
            if (training_k[i] == 0) training_k[i] = primes[i];
            if (training_k[i] > max_k) training_k[i] = max_k;
        }
        
        printf("Training k values:\n");
        for (int i = 0; i < num_training; i++) {
            printf("  k[%2d] = %lu\n", i, training_k[i]);
        }
        printf("\n");
        
        // Create context
        GTriangulationContext* ctx = create_g_triangulation_context(
            NID_secp256k1,
            num_training,
            training_k,
            500  // max iterations
        );
        
        if (!ctx) {
            printf("  ✗ Failed to create context\n");
            continue;
        }
        
        const EC_GROUP* group = EC_GROUP_new_by_curve_name(NID_secp256k1);
        
        // Run iterations and track metrics
        printf("Running 500 iterations...\n\n");
        printf("%-10s | %-12s | %-12s | %-12s | %-12s | %-10s\n",
               "Iteration", "Avg Error", "Min Error", "Max Error", "Std Dev", "Success %");
        printf("---------------------------------------------------------------------------------\n");
        
        IterationMetrics metrics_history[500];
        
        for (int iter = 0; iter < 500; iter++) {
            // Perform refinement
            perform_refinement_iteration(ctx);
            
            // Compute metrics
            compute_metrics(ctx, group, num_training, training_k, &metrics_history[iter], iter + 1);
            
            // Print every 10 iterations
            if ((iter + 1) % 10 == 0 || iter < 10) {
                IterationMetrics* m = &metrics_history[iter];
                printf("%-10d | %-12.2f | %-12.2f | %-12.2f | %-12.2f | %-9.2f%%\n",
                       m->iteration, m->avg_error, m->min_error, m->max_error, 
                       m->std_dev, m->success_rate);
            }
        }
        
        printf("\n");
        
        // Analyze convergence pattern
        printf("CONVERGENCE ANALYSIS:\n");
        printf("---------------------\n");
        
        // Find best iteration
        int best_iter = 0;
        double best_error = metrics_history[0].avg_error;
        for (int i = 1; i < 500; i++) {
            if (metrics_history[i].avg_error < best_error) {
                best_error = metrics_history[i].avg_error;
                best_iter = i;
            }
        }
        
        printf("  Best iteration: %d\n", best_iter + 1);
        printf("  Best avg error: %.2f\n", best_error);
        printf("  Best success rate: %.2f%%\n", metrics_history[best_iter].success_rate);
        printf("\n");
        
        // Check for plateau
        // Plateau = error changes less than 1% over last 50 iterations
        bool plateaued = true;
        double plateau_start_error = metrics_history[450].avg_error;
        for (int i = 451; i < 500; i++) {
            double change = math_abs(metrics_history[i].avg_error - plateau_start_error);
            double change_pct = (change / plateau_start_error) * 100.0;
            if (change_pct > 1.0) {
                plateaued = false;
                break;
            }
        }
        
        if (plateaued) {
            printf("  ✓ PLATEAUED at iteration ~450\n");
            printf("  Plateau error: %.2f\n", metrics_history[499].avg_error);
        } else {
            printf("  ✗ NOT PLATEAUED - still improving\n");
        }
        printf("\n");
        
        // Check for oscillation
        // Oscillation = error goes up and down repeatedly
        int oscillation_count = 0;
        for (int i = 1; i < 499; i++) {
            double prev = metrics_history[i-1].avg_error;
            double curr = metrics_history[i].avg_error;
            double next = metrics_history[i+1].avg_error;
            
            // Check if current is a local min or max
            if ((curr < prev && curr < next) || (curr > prev && curr > next)) {
                oscillation_count++;
            }
        }
        
        printf("  Oscillation count: %d\n", oscillation_count);
        if (oscillation_count > 50) {
            printf("  ✓ OSCILLATING - may indicate additional oscillation to track\n");
        } else {
            printf("  ✗ NOT OSCILLATING - smooth convergence\n");
        }
        printf("\n");
        
        // Compute improvement
        double initial_error = metrics_history[0].avg_error;
        double final_error = metrics_history[499].avg_error;
        double improvement = ((initial_error - final_error) / initial_error) * 100.0;
        
        printf("  Initial error: %.2f\n", initial_error);
        printf("  Final error: %.2f\n", final_error);
        printf("  Improvement: %.2f%%\n", improvement);
        printf("\n");
        
        // Cleanup
        free_g_triangulation_context(ctx);
        EC_GROUP_free((EC_GROUP*)group);
    }
    
    printf("\n========================================\n");
    printf("ANALYSIS COMPLETE\n");
    printf("========================================\n\n");
    
    return 0;
}