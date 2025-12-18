/**
 * test_search_recovery_v2.c - Test Enhanced Search-Based K Recovery
 * 
 * Tests all improvements:
 * 1. Quadrant-aware search
 * 2. 3-anchor averaging
 * 3. 3-layer recursive search
 * 4. Confidence scoring
 * 5. Q1 optimization
 */

#include "search_recovery_v2.h"
#include "search_recovery.h"
#include "clock_recovery.h"
#include "platonic_solids.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Test k values (same 50 primes)
static const uint64_t TEST_K_VALUES[] = {
    2, 3, 5, 7, 11, 13, 17, 19, 23, 29,
    31, 37, 41, 43, 47, 53, 59, 61, 67, 71,
    73, 79, 83, 89, 97, 101, 103, 107, 109, 113,
    127, 131, 137, 139, 149, 151, 157, 163, 167, 173,
    179, 181, 191, 193, 197, 199, 211, 223, 227, 229
};
#define NUM_TEST_K 50

void print_quadrant_name(Quadrant q) {
    switch (q) {
        case QUADRANT_Q1: printf("Q1 (TOP-RIGHT, 0-90°)"); break;
        case QUADRANT_Q2: printf("Q2 (TOP-LEFT, 90-180°)"); break;
        case QUADRANT_Q3: printf("Q3 (BOTTOM-LEFT, 180-270°)"); break;
        case QUADRANT_Q4: printf("Q4 (BOTTOM-RIGHT, 270-360°)"); break;
    }
}

int main() {
    printf("\n=== Enhanced Search Recovery Test ===\n\n");
    
    // Create Platonic overlay
    printf("Creating Platonic overlay...\n");
    PlatonicOverlay* overlay = create_platonic_overlay(0.5);
    if (!overlay) {
        printf("FAILED to create overlay\n");
        return 1;
    }
    printf("SUCCESS: %u shared vertices\n\n", overlay->num_shared_vertices);
    
    // Test 1: Baseline (v1 recursive torus search)
    printf("=== Test 1: Baseline (v1 recursive torus search) ===\n");
    SearchRecoveryContext* ctx_v1 = init_search_recovery(overlay, 0, 300);
    if (!ctx_v1) {
        printf("FAILED to create v1 context\n");
        free_platonic_overlay(overlay);
        return 1;
    }
    
    int success_v1 = 0;
    double total_error_v1 = 0.0;
    
    for (int i = 0; i < NUM_TEST_K; i++) {
        uint64_t k = TEST_K_VALUES[i];
        ClockPosition pos = map_k_to_clock_u64(k);
        
        double confidence = 0.0;
        uint64_t recovered_k = recursive_torus_search_k(pos, ctx_v1, 2, &confidence);
        
        int64_t k_error = (int64_t)recovered_k - (int64_t)k;
        double abs_error = fabs((double)k_error);
        total_error_v1 += abs_error;
        
        if (abs_error < 1.0) success_v1++;
    }
    
    double avg_error_v1 = total_error_v1 / NUM_TEST_K;
    double success_rate_v1 = 100.0 * success_v1 / NUM_TEST_K;
    
    printf("Results (baseline v1):\n");
    printf("  Success rate: %.1f%% (%d/%d)\n", success_rate_v1, success_v1, NUM_TEST_K);
    printf("  Average error: %.2f\n\n", avg_error_v1);
    
    // Test 2: Enhanced search (v2)
    printf("=== Test 2: Enhanced Search (v2) ===\n");
    SearchRecoveryV2Context* ctx_v2 = init_search_recovery_v2(overlay, 0, 300);
    if (!ctx_v2) {
        printf("FAILED to create v2 context\n");
        free_search_recovery(ctx_v1);
        free_platonic_overlay(overlay);
        return 1;
    }
    
    int success_v2 = 0;
    double total_error_v2 = 0.0;
    int quadrant_success[4] = {0};
    int quadrant_total[4] = {0};
    
    printf("\nDetailed results:\n");
    printf("%-4s %-8s %-12s %-10s %-8s %-10s %-10s\n",
           "k", "Angle", "Quadrant", "Recovered", "Error", "Confidence", "Status");
    printf("--------------------------------------------------------------------\n");
    
    for (int i = 0; i < NUM_TEST_K; i++) {
        uint64_t k = TEST_K_VALUES[i];
        ClockPosition pos = map_k_to_clock_u64(k);
        
        ConfidenceMetrics metrics;
        uint64_t recovered_k = enhanced_search_k(pos, ctx_v2, &metrics);
        
        int64_t k_error = (int64_t)recovered_k - (int64_t)k;
        double abs_error = fabs((double)k_error);
        total_error_v2 += abs_error;
        
        Quadrant quad = get_quadrant(pos.angle);
        quadrant_total[quad]++;
        
        bool success = (abs_error < 1.0);
        if (success) {
            success_v2++;
            quadrant_success[quad]++;
        }
        
        // Print first 20 results
        if (i < 20) {
            double angle_deg = pos.angle * 180.0 / M_PI;
            printf("%-4lu %-8.2f ", k, angle_deg);
            print_quadrant_name(quad);
            printf(" %-10lu %-8ld %-10.3f %s\n",
                   recovered_k, k_error, metrics.overall,
                   success ? "✓" : "✗");
        }
    }
    
    double avg_error_v2 = total_error_v2 / NUM_TEST_K;
    double success_rate_v2 = 100.0 * success_v2 / NUM_TEST_K;
    
    printf("\nResults (enhanced v2):\n");
    printf("  Success rate: %.1f%% (%d/%d)\n", success_rate_v2, success_v2, NUM_TEST_K);
    printf("  Average error: %.2f\n", avg_error_v2);
    printf("  Improvement: %.1f%% → %.1f%% (+%.1f%%)\n",
           success_rate_v1, success_rate_v2, success_rate_v2 - success_rate_v1);
    
    // Per-quadrant analysis
    printf("\nPer-quadrant performance:\n");
    for (int q = 0; q < 4; q++) {
        if (quadrant_total[q] > 0) {
            double q_rate = 100.0 * quadrant_success[q] / quadrant_total[q];
            printf("  ");
            print_quadrant_name((Quadrant)q);
            printf(": %.1f%% (%d/%d)\n", q_rate, quadrant_success[q], quadrant_total[q]);
        }
    }
    
    // Test 3: Q1 optimization
    printf("\n=== Test 3: Q1 Optimization ===\n");
    
    int q1_success = 0;
    int q1_total = 0;
    double q1_total_error = 0.0;
    
    printf("\nQ1 k values:\n");
    for (int i = 0; i < NUM_TEST_K; i++) {
        uint64_t k = TEST_K_VALUES[i];
        ClockPosition pos = map_k_to_clock_u64(k);
        
        Quadrant quad = get_quadrant(pos.angle);
        if (quad != QUADRANT_Q1) continue;
        
        q1_total++;
        
        ConfidenceMetrics metrics;
        uint64_t recovered_k = q1_optimized_search_k(pos, ctx_v2, &metrics);
        
        int64_t k_error = (int64_t)recovered_k - (int64_t)k;
        double abs_error = fabs((double)k_error);
        q1_total_error += abs_error;
        
        bool success = (abs_error < 1.0);
        if (success) q1_success++;
        
        double angle_deg = pos.angle * 180.0 / M_PI;
        printf("  k=%lu, angle=%.2f°, recovered=%lu, error=%ld, conf=%.3f %s\n",
               k, angle_deg, recovered_k, k_error, metrics.overall,
               success ? "✓" : "✗");
    }
    
    if (q1_total > 0) {
        double q1_rate = 100.0 * q1_success / q1_total;
        double q1_avg_error = q1_total_error / q1_total;
        printf("\nQ1 optimization results:\n");
        printf("  Success rate: %.1f%% (%d/%d)\n", q1_rate, q1_success, q1_total);
        printf("  Average error: %.2f\n", q1_avg_error);
    }
    
    // Test 4: Confidence analysis
    printf("\n=== Test 4: Confidence Analysis ===\n");
    
    int high_conf_success = 0;
    int high_conf_total = 0;
    int low_conf_success = 0;
    int low_conf_total = 0;
    
    for (int i = 0; i < NUM_TEST_K; i++) {
        uint64_t k = TEST_K_VALUES[i];
        ClockPosition pos = map_k_to_clock_u64(k);
        
        ConfidenceMetrics metrics;
        uint64_t recovered_k = enhanced_search_k(pos, ctx_v2, &metrics);
        
        int64_t k_error = (int64_t)recovered_k - (int64_t)k;
        double abs_error = fabs((double)k_error);
        bool success = (abs_error < 1.0);
        
        if (metrics.overall >= 0.8) {
            high_conf_total++;
            if (success) high_conf_success++;
        } else if (metrics.overall < 0.5) {
            low_conf_total++;
            if (success) low_conf_success++;
        }
    }
    
    printf("High confidence (≥0.8): %d/%d = %.1f%% success\n",
           high_conf_success, high_conf_total,
           high_conf_total > 0 ? 100.0 * high_conf_success / high_conf_total : 0.0);
    printf("Low confidence (<0.5): %d/%d = %.1f%% success\n",
           low_conf_success, low_conf_total,
           low_conf_total > 0 ? 100.0 * low_conf_success / low_conf_total : 0.0);
    
    // Summary
    printf("\n=== SUMMARY ===\n");
    printf("Baseline (v1):  %.1f%% (%d/%d)\n", success_rate_v1, success_v1, NUM_TEST_K);
    printf("Enhanced (v2):  %.1f%% (%d/%d)\n", success_rate_v2, success_v2, NUM_TEST_K);
    printf("Improvement:    +%.1f%% (%.1fx better)\n",
           success_rate_v2 - success_rate_v1,
           success_rate_v1 > 0 ? success_rate_v2 / success_rate_v1 : 0.0);
    
    if (success_rate_v2 >= 15.0) {
        printf("\n✓ TARGET ACHIEVED: ≥15%% success rate!\n");
    } else if (success_rate_v2 >= 12.0) {
        printf("\n✓ GOOD PROGRESS: ≥12%% success rate\n");
    } else if (success_rate_v2 > success_rate_v1) {
        printf("\n✓ IMPROVEMENT: Better than baseline\n");
    } else {
        printf("\n✗ NO IMPROVEMENT: Same or worse than baseline\n");
    }
    
    // Cleanup
    free_search_recovery_v2(ctx_v2);
    free_search_recovery(ctx_v1);
    free_platonic_overlay(overlay);
    
    return 0;
}