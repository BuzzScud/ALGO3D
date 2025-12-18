/**
 * test_torus_analysis.c - Test Torus Analysis with Plateau Detection
 * 
 * This test:
 * 1. Runs G triangulation until plateau detected
 * 2. Analyzes oscillations within plateau
 * 3. Identifies torus structure in 3D space
 * 4. Measures search space reduction
 * 5. Tests across multiple bit lengths
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <openssl/ec.h>
#include <openssl/bn.h>
#include <openssl/obj_mac.h>
#include "../include/g_triangulation.h"
#include "../include/plateau_detection.h"
#include "../include/torus_analysis.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"

int main() {
    printf("\n========================================\n");
    printf("TORUS ANALYSIS WITH PLATEAU DETECTION\n");
    printf("========================================\n\n");
    
    // Test with different bit lengths
    const int num_bit_lengths = 3;
    int bit_lengths[3] = {8, 16, 32};
    
    for (int bl = 0; bl < num_bit_lengths; bl++) {
        int bit_length = bit_lengths[bl];
        uint64_t max_k = (1ULL << bit_length) - 1;
        
        printf("\n========================================\n");
        printf("BIT LENGTH: %d-bit (max k = %lu)\n", bit_length, max_k);
        printf("========================================\n\n");
        
        // Generate training k values
        const int num_training = 20;
        uint64_t training_k[20];
        uint64_t primes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71};
        
        for (int i = 0; i < num_training; i++) {
            training_k[i] = (primes[i] * max_k) / 100;
            if (training_k[i] == 0) training_k[i] = primes[i];
            if (training_k[i] > max_k) training_k[i] = max_k;
        }
        
        // Create context
        GTriangulationContext* ctx = create_g_triangulation_context(
            NID_secp256k1,
            num_training,
            training_k,
            1000  // max iterations
        );
        
        if (!ctx) {
            printf("  ✗ Failed to create context\n");
            continue;
        }
        
        const EC_GROUP* group = EC_GROUP_new_by_curve_name(NID_secp256k1);
        
        // Create plateau detector
        PlateauDetector* plateau = create_plateau_detector(
            500,    // history size
            0.01,   // 1% threshold
            50      // window size
        );
        
        // Create torus analyzer
        TorusAnalyzer* torus = create_torus_analyzer(500, max_k);
        
        printf("Running iterations until plateau detected...\n\n");
        printf("%-10s | %-12s | %-12s | %-12s | %-10s\n",
               "Iteration", "Avg Error", "Min Error", "Max Error", "Plateau?");
        printf("-----------------------------------------------------------------------\n");
        
        bool plateau_detected = false;
        int iteration = 0;
        
        while (iteration < 1000 && !plateau_detected) {
            iteration++;
            
            // Perform refinement
            perform_refinement_iteration(ctx);
            
            // Compute metrics
            double total_error = 0.0;
            double min_error = 1e9;
            double max_error = 0.0;
            double total_k_estimate = 0.0;
            
            for (int i = 0; i < num_training; i++) {
                EC_POINT* Q = EC_POINT_new(group);
                BIGNUM* k_bn = BN_new();
                BN_set_word(k_bn, training_k[i]);
                EC_POINT_mul(group, Q, k_bn, NULL, NULL, NULL);
                
                uint64_t k_estimated = estimate_k_from_q(ctx, Q);
                double error = math_abs((double)k_estimated - (double)training_k[i]);
                
                total_error += error;
                total_k_estimate += k_estimated;
                
                if (error < min_error) min_error = error;
                if (error > max_error) max_error = error;
                
                BN_free(k_bn);
                EC_POINT_free(Q);
            }
            
            double avg_error = total_error / num_training;
            double avg_k_estimate = total_k_estimate / num_training;
            
            // Add to plateau detector
            plateau_detected = plateau_detector_add_sample(plateau, avg_error, iteration);
            
            // Add to torus analyzer
            torus_analyzer_add_sample(torus, avg_k_estimate, min_error, max_error);
            
            // Print every 10 iterations or when plateau detected
            if (iteration % 10 == 0 || plateau_detected) {
                printf("%-10d | %-12.2f | %-12.2f | %-12.2f | %-10s\n",
                       iteration, avg_error, min_error, max_error,
                       plateau_detected ? "YES" : "NO");
            }
        }
        
        printf("\n");
        
        // Print plateau results
        if (is_plateaued(plateau)) {
            printf("PLATEAU DETECTED:\n");
            printf("  Iteration: %d\n", get_plateau_iteration(plateau));
            printf("  Value: %.2f\n", get_plateau_value(plateau));
            printf("  Stopped at iteration: %d\n", iteration);
            printf("\n");
        } else {
            printf("NO PLATEAU DETECTED (reached max iterations)\n\n");
        }
        
        // Analyze torus
        printf("ANALYZING TORUS STRUCTURE...\n");
        bool torus_found = analyze_torus(torus);
        
        if (torus_found) {
            printf("  ✓ TORUS IDENTIFIED!\n\n");
        } else {
            printf("  ✗ No clear torus structure detected\n\n");
        }
        
        // Print full torus analysis
        print_torus_analysis(torus);
        
        // Export data
        char filename[256];
        snprintf(filename, sizeof(filename), "torus_data_%dbit.csv", bit_length);
        export_torus_data(torus, filename);
        printf("  ✓ Torus data exported to %s\n\n", filename);
        
        // Cleanup
        free_g_triangulation_context(ctx);
        free_plateau_detector(plateau);
        free_torus_analyzer(torus);
        EC_GROUP_free((EC_GROUP*)group);
    }
    
    printf("\n========================================\n");
    printf("ANALYSIS COMPLETE\n");
    printf("========================================\n\n");
    
    printf("SUMMARY:\n");
    printf("--------\n");
    printf("1. Plateau detection automatically stops iterations\n");
    printf("2. Torus analysis identifies oscillation patterns\n");
    printf("3. Search space reduction computed from torus parameters\n");
    printf("4. 3D oscillation space (k, error_min, error_max) analyzed\n");
    printf("\n");
    printf("Next steps:\n");
    printf("- Use torus parameters to guide search within reduced space\n");
    printf("- Identify additional tori (dual scalar oscillations)\n");
    printf("- Apply harmonic folding and entropy reduction\n");
    printf("\n");
    
    return 0;
}