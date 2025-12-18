/**
 * test_search_recovery_v4.c - Test Direct K-Based Anchor Selection
 * 
 * THE CRITICAL TEST: Does direct k-based anchor selection achieve 90-100%?
 * 
 * We test two modes:
 * 1. Direct mode: We know target k, select correct anchor
 * 2. Angle mode: We only know angle, must estimate k first
 */

#include "search_recovery_v4.h"
#include "search_recovery_v2.h"
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

int main() {
    printf("\n=== Direct K-Based Anchor Selection Test ===\n\n");
    
    // Test 1: Baseline v2 (angle-based anchors)
    printf("=== Test 1: Baseline v2 (angle-based anchors) ===\n");
    
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
    
    printf("Results (v2 - angle-based):\n");
    printf("  Success rate: %.1f%% (%d/%d)\n", success_rate_v2, success_v2, NUM_TEST_K);
    printf("  Average error: %.2f\n\n", avg_error_v2);
    
    // Test 2: v4 Direct Mode (we know target k)
    printf("=== Test 2: v4 Direct Mode (k-based anchors, known k) ===\n");
    
    SearchRecoveryV4Context* ctx_v4 = init_search_recovery_v4(0, 300);
    if (!ctx_v4) {
        printf("FAILED to create v4 context\n");
        free_search_recovery_v2(ctx_v2);
        free_platonic_overlay(overlay);
        return 1;
    }
    
    printf("v4 Configuration:\n");
    printf("  Anchors: %d (k = ", ctx_v4->num_anchors);
    for (int i = 0; i < ctx_v4->num_anchors; i++) {
        printf("%lu", ctx_v4->anchors[i]);
        if (i < ctx_v4->num_anchors - 1) printf(", ");
    }
    printf(")\n");
    printf("  Spacing: %lu\n", ctx_v4->anchor_spacing);
    printf("  Max distance to anchor: %.1f\n\n", ctx_v4->anchor_spacing / 2.0);
    
    int success_v4_direct = 0;
    double total_error_v4_direct = 0.0;
    
    printf("Detailed results (first 20):\n");
    printf("%-4s %-8s %-10s %-10s %-8s %-10s\n",
           "k", "Anchor", "Dist", "Recovered", "Error", "Status");
    printf("------------------------------------------------------------\n");
    
    for (int i = 0; i < NUM_TEST_K; i++) {
        uint64_t k = TEST_K_VALUES[i];
        
        double confidence = 0.0;
        uint64_t recovered_k = recover_k_direct_anchors(k, ctx_v4, &confidence);
        
        int64_t k_error = (int64_t)recovered_k - (int64_t)k;
        double abs_error = fabs((double)k_error);
        total_error_v4_direct += abs_error;
        
        bool success = (abs_error < 1.0);
        if (success) success_v4_direct++;
        
        if (i < 20) {
            uint64_t anchor = find_nearest_anchor_k(ctx_v4, k);
            int64_t dist = (int64_t)k - (int64_t)anchor;
            printf("%-4lu %-8lu %-10ld %-10lu %-8ld %s\n",
                   k, anchor, dist, recovered_k, k_error,
                   success ? "✓" : "✗");
        }
    }
    
    double avg_error_v4_direct = total_error_v4_direct / NUM_TEST_K;
    double success_rate_v4_direct = 100.0 * success_v4_direct / NUM_TEST_K;
    
    printf("\nResults (v4 direct - k-based):\n");
    printf("  Success rate: %.1f%% (%d/%d)\n", success_rate_v4_direct, success_v4_direct, NUM_TEST_K);
    printf("  Average error: %.2f\n", avg_error_v4_direct);
    printf("  Improvement: %.1f%% → %.1f%% (+%.1f%%)\n",
           success_rate_v2, success_rate_v4_direct, success_rate_v4_direct - success_rate_v2);
    
    // Test 3: v4 Angle Mode (production mode - only have angle)
    printf("\n=== Test 3: v4 Angle Mode (production mode - estimate k from angle) ===\n");
    
    int success_v4_angle = 0;
    double total_error_v4_angle = 0.0;
    
    for (int i = 0; i < NUM_TEST_K; i++) {
        uint64_t k = TEST_K_VALUES[i];
        ClockPosition pos = map_k_to_clock_u64(k);
        
        double confidence = 0.0;
        uint64_t recovered_k = recover_k_from_angle_v4(pos.angle, ctx_v4, &confidence);
        
        int64_t k_error = (int64_t)recovered_k - (int64_t)k;
        double abs_error = fabs((double)k_error);
        total_error_v4_angle += abs_error;
        
        if (abs_error < 1.0) success_v4_angle++;
    }
    
    double avg_error_v4_angle = total_error_v4_angle / NUM_TEST_K;
    double success_rate_v4_angle = 100.0 * success_v4_angle / NUM_TEST_K;
    
    printf("Results (v4 angle - production mode):\n");
    printf("  Success rate: %.1f%% (%d/%d)\n", success_rate_v4_angle, success_v4_angle, NUM_TEST_K);
    printf("  Average error: %.2f\n", avg_error_v4_angle);
    printf("  Improvement: %.1f%% → %.1f%% (+%.1f%%)\n",
           success_rate_v2, success_rate_v4_angle, success_rate_v4_angle - success_rate_v2);
    
    // Summary
    printf("\n=== SUMMARY ===\n");
    printf("Baseline (v2 angle-based):  %.1f%% (%d/%d)\n", 
           success_rate_v2, success_v2, NUM_TEST_K);
    printf("v4 Direct (k-based):        %.1f%% (%d/%d)\n", 
           success_rate_v4_direct, success_v4_direct, NUM_TEST_K);
    printf("v4 Angle (production):      %.1f%% (%d/%d)\n", 
           success_rate_v4_angle, success_v4_angle, NUM_TEST_K);
    
    printf("\nImprovement Analysis:\n");
    printf("  Direct mode:  %.1fx better (%.1f%% → %.1f%%)\n",
           success_rate_v2 > 0 ? success_rate_v4_direct / success_rate_v2 : 0.0,
           success_rate_v2, success_rate_v4_direct);
    printf("  Angle mode:   %.1fx better (%.1f%% → %.1f%%)\n",
           success_rate_v2 > 0 ? success_rate_v4_angle / success_rate_v2 : 0.0,
           success_rate_v2, success_rate_v4_angle);
    
    if (success_rate_v4_direct >= 90.0) {
        printf("\n🎉 BREAKTHROUGH: ≥90%% success rate in direct mode!\n");
    } else if (success_rate_v4_direct >= 50.0) {
        printf("\n✓ MAJOR PROGRESS: ≥50%% success rate in direct mode\n");
    } else if (success_rate_v4_direct > success_rate_v2) {
        printf("\n✓ IMPROVEMENT: Better than baseline\n");
    } else {
        printf("\n✗ NO IMPROVEMENT: Same or worse than baseline\n");
    }
    
    // Cleanup
    free_search_recovery_v4(ctx_v4);
    free_search_recovery_v2(ctx_v2);
    free_platonic_overlay(overlay);
    
    return 0;
}