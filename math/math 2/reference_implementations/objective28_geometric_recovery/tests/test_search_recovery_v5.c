#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include "../include/search_recovery_v5.h"
#include "../include/search_recovery_v2.h"
#include "../include/platonic_solids.h"

// Test k values (same as v2)
#define NUM_TEST_K 50
static const uint64_t TEST_K_VALUES[NUM_TEST_K] = {
    2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71,
    73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151,
    157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229
};

const char* quadrant_name(int q) {
    switch(q) {
        case 0: return "Q1";
        case 1: return "Q2";
        case 2: return "Q3";
        case 3: return "Q4";
        default: return "??";
    }
}

int main() {
    printf("=== RADIUS-AWARE SEARCH TEST (V5) ===\n\n");
    
    // Initialize context
    PlatonicOverlay* overlay = create_platonic_overlay(1e-6);
    if (!overlay) {
        fprintf(stderr, "Failed to create overlay\n");
        return 1;
    }
    
    SearchRecoveryV2Context* ctx = init_search_recovery_v2(overlay, 0, 300);
    if (!ctx) {
        fprintf(stderr, "Failed to create context\n");
        free_platonic_overlay(overlay);
        return 1;
    }
    
    printf("Testing with %d k values...\n\n", NUM_TEST_K);
    
    // Test v2 (baseline - angle only)
    int success_v2 = 0;
    double total_error_v2 = 0;
    int quadrant_success_v2[4] = {0};
    int quadrant_total[4] = {0};
    
    printf("=== Test 1: Baseline (v2 - angle only) ===\n");
    for (int i = 0; i < NUM_TEST_K; i++) {
        uint64_t k = TEST_K_VALUES[i];
        ClockPosition pos = map_k_to_clock_u64(k);
        
        ConfidenceMetrics metrics;
        uint64_t recovered_k = enhanced_search_k(pos, ctx, &metrics);
        
        int64_t error = (int64_t)recovered_k - (int64_t)k;
        double abs_error = fabs((double)error);
        total_error_v2 += abs_error;
        
        Quadrant quad = get_quadrant(pos.angle);
        quadrant_total[quad]++;
        
        if (abs_error < 1.0) {
            success_v2++;
            quadrant_success_v2[quad]++;
        }
    }
    
    double avg_error_v2 = total_error_v2 / NUM_TEST_K;
    double success_rate_v2 = 100.0 * success_v2 / NUM_TEST_K;
    
    printf("Results (v2 baseline):\n");
    printf("  Success rate: %.1f%% (%d/%d)\n", success_rate_v2, success_v2, NUM_TEST_K);
    printf("  Average error: %.2f\n\n", avg_error_v2);
    
    // Test v5 (radius-aware)
    int success_v5 = 0;
    double total_error_v5 = 0;
    int quadrant_success_v5[4] = {0};
    
    printf("=== Test 2: Radius-Aware (v5) ===\n");
    
    printf("\nDetailed results (first 20):\n");
    printf("%-4s %-8s %-8s %-6s %-10s %-10s %-8s %-12s %-10s\n",
           "k", "Angle", "Radius", "Quad", "Recovered", "Error", "Status", "Confidence", "Weights");
    printf("-------------------------------------------------------------------------------------\n");
    
    for (int i = 0; i < NUM_TEST_K; i++) {
        uint64_t k = TEST_K_VALUES[i];
        ClockPosition pos = map_k_to_clock_u64(k);
        
        RadiusConfidenceMetrics metrics;
        uint64_t recovered_k = radius_aware_search_k(pos, ctx, &metrics);
        
        int64_t error = (int64_t)recovered_k - (int64_t)k;
        double abs_error = fabs((double)error);
        total_error_v5 += abs_error;
        
        Quadrant quad = get_quadrant(pos.angle);
        
        bool success = (abs_error < 1.0);
        if (success) {
            success_v5++;
            quadrant_success_v5[quad]++;
        }
        
        // Print first 20 results
        if (i < 20) {
            double angle_deg = pos.angle * 180.0 / M_PI;
            ErrorWeights weights = compute_adaptive_weights(pos.radius);
            
            printf("%-4lu %-8.2f %-8.4f %-6s %-10lu %-10ld %s %-12.3f %.1f/%.1f\n",
                   k, angle_deg, pos.radius, quadrant_name(quad),
                   recovered_k, error, success ? "✓" : "✗",
                   metrics.overall_confidence,
                   weights.angle_weight, weights.radius_weight);
        }
    }
    
    double avg_error_v5 = total_error_v5 / NUM_TEST_K;
    double success_rate_v5 = 100.0 * success_v5 / NUM_TEST_K;
    
    printf("\nResults (v5 radius-aware):\n");
    printf("  Success rate: %.1f%% (%d/%d)\n", success_rate_v5, success_v5, NUM_TEST_K);
    printf("  Average error: %.2f\n", avg_error_v5);
    printf("  Improvement: %.1f%% → %.1f%% (+%.1f%%)\n\n",
           success_rate_v2, success_rate_v5, success_rate_v5 - success_rate_v2);
    
    // Per-quadrant analysis
    printf("=== Per-Quadrant Performance ===\n\n");
    printf("Quadrant    Total    v2 Success    v5 Success    Improvement\n");
    printf("---------------------------------------------------------------\n");
    
    for (int q = 0; q < 4; q++) {
        if (quadrant_total[q] > 0) {
            double rate_v2 = 100.0 * quadrant_success_v2[q] / quadrant_total[q];
            double rate_v5 = 100.0 * quadrant_success_v5[q] / quadrant_total[q];
            double improvement = rate_v5 - rate_v2;
            
            printf("%-11s %-8d %-13.1f%% %-13.1f%% %+.1f%%\n",
                   quadrant_name(q), quadrant_total[q],
                   rate_v2, rate_v5, improvement);
        }
    }
    
    // Summary
    printf("\n=== SUMMARY ===\n");
    printf("Baseline (v2 angle-only):  %.1f%% (%d/%d)\n", 
           success_rate_v2, success_v2, NUM_TEST_K);
    printf("Radius-aware (v5):         %.1f%% (%d/%d)\n",
           success_rate_v5, success_v5, NUM_TEST_K);
    printf("\nImprovement: %.1fx better (%.1f%% → %.1f%%)\n",
           success_rate_v5 / success_rate_v2,
           success_rate_v2, success_rate_v5);
    
    if (success_v5 > success_v2) {
        printf("\n✓ SUCCESS: Radius-aware search improves performance!\n");
    } else if (success_v5 == success_v2) {
        printf("\n⚠ NEUTRAL: No improvement, but no regression\n");
    } else {
        printf("\n✗ REGRESSION: Performance decreased\n");
    }
    
    free_search_recovery_v2(ctx);
    free_platonic_overlay(overlay);
    
    return 0;
}