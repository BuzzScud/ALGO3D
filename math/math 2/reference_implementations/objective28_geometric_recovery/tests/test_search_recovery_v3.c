/**
 * test_search_recovery_v3.c - Test 24-Anchor Grid Enhancement
 * 
 * Tests the improvement from 12 anchors (v2) to 24 anchors (v3).
 * Expected: 20% → 30% success rate
 */

#include "search_recovery_v3.h"
#include "search_recovery_v2.h"
#include "clock_recovery.h"
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

int main() {
    printf("\n=== 24-Anchor Grid Enhancement Test ===\n\n");
    
    // Test 1: Baseline v2 (12 anchors)
    printf("=== Test 1: Baseline v2 (12 anchors) ===\n");
    
    PlatonicOverlay* overlay = create_platonic_overlay(0.5);
    if (!overlay) {
        printf("FAILED to create overlay\n");
        return 1;
    }
    
    SearchRecoveryV2Context* ctx_v2 = init_search_recovery_v2(overlay, 0, 300);
    if (!ctx_v2) {
        printf("FAILED to create v2 context\n");
        free_platonic_overlay(overlay);
        return 1;
    }
    
    int success_v2 = 0;
    double total_error_v2 = 0.0;
    
    for (int i = 0; i < NUM_TEST_K; i++) {
        uint64_t k = TEST_K_VALUES[i];
        ClockPosition pos = map_k_to_clock_u64(k);
        
        ConfidenceMetrics metrics;
        uint64_t recovered_k = enhanced_search_k(pos, ctx_v2, &metrics);
        
        int64_t k_error = (int64_t)recovered_k - (int64_t)k;
        double abs_error = fabs((double)k_error);
        total_error_v2 += abs_error;
        
        if (abs_error < 1.0) success_v2++;
    }
    
    double avg_error_v2 = total_error_v2 / NUM_TEST_K;
    double success_rate_v2 = 100.0 * success_v2 / NUM_TEST_K;
    
    printf("Results (v2 - 12 anchors):\n");
    printf("  Success rate: %.1f%% (%d/%d)\n", success_rate_v2, success_v2, NUM_TEST_K);
    printf("  Average error: %.2f\n\n", avg_error_v2);
    
    // Test 2: Enhanced v3 (24 anchors)
    printf("=== Test 2: Enhanced v3 (24 anchors) ===\n");
    
    SearchRecoveryV3Context* ctx_v3 = init_search_recovery_v3(0, 300);
    if (!ctx_v3) {
        printf("FAILED to create v3 context\n");
        free_search_recovery_v2(ctx_v2);
        free_platonic_overlay(overlay);
        return 1;
    }
    
    // Print anchor grid
    printf("\nAnchor grid configuration:\n");
    print_anchor_grid_24(ctx_v3->anchor_grid);
    printf("\n");
    
    int success_v3 = 0;
    double total_error_v3 = 0.0;
    
    printf("\nDetailed results (first 20):\n");
    printf("%-4s %-8s %-12s %-8s %-10s\n", "k", "Angle", "Recovered", "Error", "Status");
    printf("--------------------------------------------------------\n");
    
    for (int i = 0; i < NUM_TEST_K; i++) {
        uint64_t k = TEST_K_VALUES[i];
        ClockPosition pos = map_k_to_clock_u64(k);
        
        double confidence = 0.0;
        uint64_t recovered_k = search_with_24_anchors(pos.angle, ctx_v3, &confidence);
        
        int64_t k_error = (int64_t)recovered_k - (int64_t)k;
        double abs_error = fabs((double)k_error);
        total_error_v3 += abs_error;
        
        bool success = (abs_error < 1.0);
        if (success) success_v3++;
        
        if (i < 20) {
            double angle_deg = pos.angle * 180.0 / M_PI;
            printf("%-4lu %-8.2f %-12lu %-8ld %s\n",
                   k, angle_deg, recovered_k, k_error,
                   success ? "✓" : "✗");
        }
    }
    
    double avg_error_v3 = total_error_v3 / NUM_TEST_K;
    double success_rate_v3 = 100.0 * success_v3 / NUM_TEST_K;
    
    printf("\nResults (v3 - 24 anchors):\n");
    printf("  Success rate: %.1f%% (%d/%d)\n", success_rate_v3, success_v3, NUM_TEST_K);
    printf("  Average error: %.2f\n", avg_error_v3);
    printf("  Improvement: %.1f%% → %.1f%% (+%.1f%%)\n",
           success_rate_v2, success_rate_v3, success_rate_v3 - success_rate_v2);
    
    // Analysis
    printf("\n=== Analysis ===\n");
    printf("Anchor count: 12 → 24 (2x increase)\n");
    printf("Success rate: %.1f%% → %.1f%% (%.1fx improvement)\n",
           success_rate_v2, success_rate_v3,
           success_rate_v2 > 0 ? success_rate_v3 / success_rate_v2 : 0.0);
    printf("Average error: %.2f → %.2f (%.1f%% reduction)\n",
           avg_error_v2, avg_error_v3,
           avg_error_v2 > 0 ? 100.0 * (avg_error_v2 - avg_error_v3) / avg_error_v2 : 0.0);
    
    // Summary
    printf("\n=== SUMMARY ===\n");
    printf("Baseline (v2):  %.1f%% (%d/%d)\n", success_rate_v2, success_v2, NUM_TEST_K);
    printf("Enhanced (v3):  %.1f%% (%d/%d)\n", success_rate_v3, success_v3, NUM_TEST_K);
    
    if (success_rate_v3 >= 30.0) {
        printf("\n✓ TARGET ACHIEVED: ≥30%% success rate!\n");
    } else if (success_rate_v3 >= 25.0) {
        printf("\n✓ GOOD PROGRESS: ≥25%% success rate\n");
    } else if (success_rate_v3 > success_rate_v2) {
        printf("\n✓ IMPROVEMENT: Better than v2\n");
    } else {
        printf("\n✗ NO IMPROVEMENT: Same or worse than v2\n");
    }
    
    // Cleanup
    free_search_recovery_v3(ctx_v3);
    free_search_recovery_v2(ctx_v2);
    free_platonic_overlay(overlay);
    
    return 0;
}