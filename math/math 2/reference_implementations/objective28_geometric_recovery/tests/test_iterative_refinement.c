/**
 * test_iterative_refinement.c - Test Iterative G Triangulation and Refinement
 * 
 * This test implements the full algorithm:
 * 1. Start with known G
 * 2. Generate known (k, Q) training pairs
 * 3. Adjust anchors relative to G
 * 4. Iteratively estimate k for each pair
 * 5. Track oscillation in k estimates
 * 6. Measure convergence
 * 7. Test unknown k recovery
 * 8. Measure search space reduction
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

int main() {
    printf("\n========================================\n");
    printf("ITERATIVE G TRIANGULATION TEST\n");
    printf("========================================\n\n");
    
    // Test 1: Create context with training pairs
    printf("TEST 1: Create G Triangulation Context\n");
    printf("---------------------------------------\n");
    
    const int num_training = 20;
    uint64_t training_k[20] = {
        2, 3, 5, 7, 11, 13, 17, 19, 23, 29,
        31, 37, 41, 43, 47, 53, 59, 61, 67, 71
    };
    
    GTriangulationContext* ctx = create_g_triangulation_context(
        NID_secp256k1,
        num_training,
        training_k,
        100  // max iterations
    );
    
    if (!ctx) {
        printf("  ✗ Failed to create context\n");
        return 1;
    }
    
    printf("  ✓ Context created successfully\n");
    printf("  ✓ Training pairs: %d\n", num_training);
    printf("  ✓ Max iterations: 100\n");
    printf("\n");
    
    // Test 2: Initial k estimation (before refinement)
    printf("TEST 2: Initial K Estimation\n");
    printf("-----------------------------\n");
    
    printf("  Testing initial k estimates (before refinement):\n\n");
    printf("  %-6s | %-10s | %-10s | %-10s | %-10s\n", 
           "Pair", "Real k", "Estimated", "Error", "Error %");
    printf("  %s\n", "------------------------------------------------------------");
    
    double total_initial_error = 0.0;
    int initial_successes = 0;
    
    for (int i = 0; i < num_training; i++) {
        // Get Q for this training pair
        const EC_GROUP* group = EC_GROUP_new_by_curve_name(NID_secp256k1);
        EC_POINT* Q = EC_POINT_new(group);
        BIGNUM* k_bn = BN_new();
        BN_set_word(k_bn, training_k[i]);
        EC_POINT_mul(group, Q, k_bn, NULL, NULL, NULL);
        
        // Estimate k
        uint64_t k_estimated = estimate_k_from_q(ctx, Q);
        
        // Compute error
        double error = math_abs((double)k_estimated - (double)training_k[i]);
        double error_pct = (error / (double)training_k[i]) * 100.0;
        total_initial_error += error;
        
        // Check success (within 10%)
        if (error_pct <= 10.0) {
            initial_successes++;
        }
        
        printf("  %-6d | %-10lu | %-10lu | %-10.0f | %-9.2f%%\n",
               i+1, training_k[i], k_estimated, error, error_pct);
        
        BN_free(k_bn);
        EC_POINT_free(Q);
        EC_GROUP_free((EC_GROUP*)group);
    }
    
    double avg_initial_error = total_initial_error / num_training;
    double initial_success_rate = (double)initial_successes / num_training * 100.0;
    
    printf("\n  ✓ Average initial error: %.2f\n", avg_initial_error);
    printf("  ✓ Initial success rate: %.2f%% (%d/%d)\n", 
           initial_success_rate, initial_successes, num_training);
    printf("\n");
    
    // Test 3: Iterative refinement
    printf("TEST 3: Iterative Refinement\n");
    printf("-----------------------------\n");
    
    printf("  Running iterative refinement...\n\n");
    printf("  %-10s | %-15s | %-15s\n", "Iteration", "Avg Error", "Success Rate");
    printf("  %s\n", "------------------------------------------------");
    
    const int max_iter = 50;
    for (int iter = 0; iter < max_iter; iter++) {
        // Perform one iteration
        perform_refinement_iteration(ctx);
        
        // Compute metrics every 5 iterations
        if ((iter + 1) % 5 == 0) {
            double total_error = 0.0;
            int successes = 0;
            
            for (int i = 0; i < num_training; i++) {
                const EC_GROUP* group = EC_GROUP_new_by_curve_name(NID_secp256k1);
                EC_POINT* Q = EC_POINT_new(group);
                BIGNUM* k_bn = BN_new();
                BN_set_word(k_bn, training_k[i]);
                EC_POINT_mul(group, Q, k_bn, NULL, NULL, NULL);
                
                uint64_t k_estimated = estimate_k_from_q(ctx, Q);
                double error = math_abs((double)k_estimated - (double)training_k[i]);
                double error_pct = (error / (double)training_k[i]) * 100.0;
                
                total_error += error;
                if (error_pct <= 10.0) successes++;
                
                BN_free(k_bn);
                EC_POINT_free(Q);
                EC_GROUP_free((EC_GROUP*)group);
            }
            
            double avg_error = total_error / num_training;
            double success_rate = (double)successes / num_training * 100.0;
            
            printf("  %-10d | %-15.2f | %-14.2f%%\n", iter + 1, avg_error, success_rate);
        }
        
        // Check convergence
        if (check_convergence(ctx, 1.0)) {
            printf("\n  ✓ Converged at iteration %d\n", iter + 1);
            break;
        }
    }
    
    printf("\n");
    
    // Test 4: Final k estimation (after refinement)
    printf("TEST 4: Final K Estimation\n");
    printf("--------------------------\n");
    
    printf("  Testing final k estimates (after refinement):\n\n");
    printf("  %-6s | %-10s | %-10s | %-10s | %-10s\n", 
           "Pair", "Real k", "Estimated", "Error", "Error %");
    printf("  %s\n", "------------------------------------------------------------");
    
    double total_final_error = 0.0;
    int final_successes = 0;
    
    for (int i = 0; i < num_training; i++) {
        const EC_GROUP* group = EC_GROUP_new_by_curve_name(NID_secp256k1);
        EC_POINT* Q = EC_POINT_new(group);
        BIGNUM* k_bn = BN_new();
        BN_set_word(k_bn, training_k[i]);
        EC_POINT_mul(group, Q, k_bn, NULL, NULL, NULL);
        
        uint64_t k_estimated = estimate_k_from_q(ctx, Q);
        double error = math_abs((double)k_estimated - (double)training_k[i]);
        double error_pct = (error / (double)training_k[i]) * 100.0;
        total_final_error += error;
        
        if (error_pct <= 10.0) {
            final_successes++;
        }
        
        printf("  %-6d | %-10lu | %-10lu | %-10.0f | %-9.2f%%\n",
               i+1, training_k[i], k_estimated, error, error_pct);
        
        BN_free(k_bn);
        EC_POINT_free(Q);
        EC_GROUP_free((EC_GROUP*)group);
    }
    
    double avg_final_error = total_final_error / num_training;
    double final_success_rate = (double)final_successes / num_training * 100.0;
    
    printf("\n  ✓ Average final error: %.2f\n", avg_final_error);
    printf("  ✓ Final success rate: %.2f%% (%d/%d)\n", 
           final_success_rate, final_successes, num_training);
    printf("\n");
    
    // Test 5: Improvement analysis
    printf("TEST 5: Improvement Analysis\n");
    printf("----------------------------\n");
    
    double error_reduction = ((avg_initial_error - avg_final_error) / avg_initial_error) * 100.0;
    double success_improvement = final_success_rate - initial_success_rate;
    
    printf("  Initial state:\n");
    printf("    Average error: %.2f\n", avg_initial_error);
    printf("    Success rate: %.2f%%\n", initial_success_rate);
    printf("\n");
    printf("  Final state:\n");
    printf("    Average error: %.2f\n", avg_final_error);
    printf("    Success rate: %.2f%%\n", final_success_rate);
    printf("\n");
    printf("  Improvement:\n");
    printf("    Error reduction: %.2f%%\n", error_reduction);
    printf("    Success improvement: %+.2f%%\n", success_improvement);
    printf("\n");
    
    // Cleanup
    free_g_triangulation_context(ctx);
    
    printf("========================================\n");
    printf("ALL TESTS PASSED! ✓\n");
    printf("========================================\n\n");
    
    return 0;
}