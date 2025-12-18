/**
 * test_search_recovery.c - Test Search-Based K Recovery
 * 
 * This tests the CORRECT approach: binary search with forward mapping
 * instead of trying to invert the formula.
 */

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

int main() {
    printf("\n=== Search-Based Recovery Test ===\n\n");
    
    // Create Platonic overlay
    printf("Creating Platonic overlay...\n");
    PlatonicOverlay* overlay = create_platonic_overlay(0.5);
    if (!overlay) {
        printf("FAILED to create overlay\n");
        return 1;
    }
    printf("SUCCESS: %u shared vertices\n\n", overlay->num_shared_vertices);
    
    // Test 1: Binary search (no anchors)
    printf("Test 1: Binary search (no anchors)\n");
    SearchRecoveryContext* ctx1 = init_search_recovery(NULL, 0, 300);
    if (!ctx1) {
        printf("FAILED to create context\n");
        free_platonic_overlay(overlay);
        return 1;
    }
    
    int success_count1 = 0;
    double total_error1 = 0.0;
    
    for (int i = 0; i < NUM_TEST_K; i++) {
        uint64_t k = TEST_K_VALUES[i];
        ClockPosition pos = map_k_to_clock_u64(k);
        
        double error = 0.0;
        uint64_t recovered_k = binary_search_k(pos, 0, 300, 100, &error);
        
        int64_t k_error = (int64_t)recovered_k - (int64_t)k;
        double abs_error = fabs((double)k_error);
        total_error1 += abs_error;
        
        if (abs_error < 1.0) success_count1++;
        
        if (i < 10) {
            printf("  k=%lu: recovered=%lu, error=%ld, angle_error=%.4f\n",
                   k, recovered_k, k_error, error);
        }
    }
    
    double avg_error1 = total_error1 / NUM_TEST_K;
    double success_rate1 = 100.0 * success_count1 / NUM_TEST_K;
    
    printf("\nResults (binary search):\n");
    printf("  Average error: %.2f\n", avg_error1);
    printf("  Success rate: %.1f%% (%d/%d)\n", success_rate1, success_count1, NUM_TEST_K);
    printf("\n");
    
    // Test 2: Anchor-guided search
    printf("Test 2: Anchor-guided search\n");
    SearchRecoveryContext* ctx2 = init_search_recovery(overlay, 0, 300);
    if (!ctx2) {
        printf("FAILED to create context\n");
        free_search_recovery(ctx1);
        free_platonic_overlay(overlay);
        return 1;
    }
    
    int success_count2 = 0;
    double total_error2 = 0.0;
    
    for (int i = 0; i < NUM_TEST_K; i++) {
        uint64_t k = TEST_K_VALUES[i];
        ClockPosition pos = map_k_to_clock_u64(k);
        
        double confidence = 0.0;
        uint64_t recovered_k = anchor_guided_search_k(pos, ctx2, &confidence);
        
        int64_t k_error = (int64_t)recovered_k - (int64_t)k;
        double abs_error = fabs((double)k_error);
        total_error2 += abs_error;
        
        if (abs_error < 1.0) success_count2++;
        
        if (i < 10) {
            printf("  k=%lu: recovered=%lu, error=%ld, confidence=%.2f\n",
                   k, recovered_k, k_error, confidence);
        }
    }
    
    double avg_error2 = total_error2 / NUM_TEST_K;
    double success_rate2 = 100.0 * success_count2 / NUM_TEST_K;
    
    printf("\nResults (anchor-guided):\n");
    printf("  Average error: %.2f\n", avg_error2);
    printf("  Success rate: %.1f%% (%d/%d)\n", success_rate2, success_count2, NUM_TEST_K);
    printf("\n");
    
    // Test 3: Recursive torus search
    printf("Test 3: Recursive torus search\n");
    SearchRecoveryContext* ctx3 = init_search_recovery(overlay, 0, 300);
    if (!ctx3) {
        printf("FAILED to create context\n");
        free_search_recovery(ctx1);
        free_search_recovery(ctx2);
        free_platonic_overlay(overlay);
        return 1;
    }
    
    int success_count3 = 0;
    double total_error3 = 0.0;
    
    for (int i = 0; i < NUM_TEST_K; i++) {
        uint64_t k = TEST_K_VALUES[i];
        ClockPosition pos = map_k_to_clock_u64(k);
        
        double confidence = 0.0;
        uint64_t recovered_k = recursive_torus_search_k(pos, ctx3, 0, &confidence);
        
        int64_t k_error = (int64_t)recovered_k - (int64_t)k;
        double abs_error = fabs((double)k_error);
        total_error3 += abs_error;
        
        if (abs_error < 1.0) success_count3++;
        
        if (i < 10) {
            printf("  k=%lu: recovered=%lu, error=%ld, confidence=%.2f\n",
                   k, recovered_k, k_error, confidence);
        }
    }
    
    double avg_error3 = total_error3 / NUM_TEST_K;
    double success_rate3 = 100.0 * success_count3 / NUM_TEST_K;
    
    printf("\nResults (recursive torus):\n");
    printf("  Average error: %.2f\n", avg_error3);
    printf("  Success rate: %.1f%% (%d/%d)\n", success_rate3, success_count3, NUM_TEST_K);
    printf("\n");
    
    // Comparison
    printf("=== Comparison ===\n\n");
    printf("Baseline (linear corrections):\n");
    printf("  Success rate: 4.0%% (2/50)\n");
    printf("  Average error: 77.77\n");
    printf("\n");
    
    printf("Binary search:\n");
    printf("  Success rate: %.1f%% (%d/50)\n", success_rate1, success_count1);
    printf("  Average error: %.2f\n", avg_error1);
    printf("  Improvement: %.1fx\n", success_rate1 / 4.0);
    printf("\n");
    
    printf("Anchor-guided search:\n");
    printf("  Success rate: %.1f%% (%d/50)\n", success_rate2, success_count2);
    printf("  Average error: %.2f\n", avg_error2);
    printf("  Improvement: %.1fx\n", success_rate2 / 4.0);
    printf("\n");
    
    printf("Recursive torus search:\n");
    printf("  Success rate: %.1f%% (%d/50)\n", success_rate3, success_count3);
    printf("  Average error: %.2f\n", avg_error3);
    printf("  Improvement: %.1fx\n", success_rate3 / 4.0);
    printf("\n");
    
    // Cleanup
    free_search_recovery(ctx1);
    free_search_recovery(ctx2);
    free_search_recovery(ctx3);
    free_platonic_overlay(overlay);
    
    printf("=== Test Complete ===\n\n");
    
    return 0;
}