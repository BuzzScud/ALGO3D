/**
 * test_spherical_recovery.c - Test Spherical Coordinate Recovery
 * 
 * This tests the spherical recovery system with the 50 known k values
 * and compares performance against the linear system.
 */

#include "spherical_recovery.h"
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
    printf("\n=== Spherical Recovery System Test ===\n\n");
    
    // Create Platonic overlay
    printf("Creating Platonic overlay...\n");
    PlatonicOverlay* overlay = create_platonic_overlay(0.5);
    if (!overlay) {
        printf("FAILED to create overlay\n");
        return 1;
    }
    printf("SUCCESS: %u shared vertices\n\n", overlay->num_shared_vertices);
    
    // Test 1: No quadrant filter
    printf("Test 1: Spherical recovery (no filter)\n");
    SphericalRecoveryContext* ctx1 = init_spherical_recovery(overlay, false, QUADRANT_TOP_RIGHT);
    if (!ctx1) {
        printf("FAILED to create context\n");
        free_platonic_overlay(overlay);
        return 1;
    }
    
    double total_error1 = 0.0;
    int success_count1 = 0;
    
    for (int i = 0; i < NUM_TEST_K; i++) {
        uint64_t k = TEST_K_VALUES[i];
        ClockPosition pos = map_k_to_clock_u64(k);
        
        double confidence = 0.0;
        double recovered_k = recover_k_spherical(pos, ctx1, &confidence);
        double error = fabs(recovered_k - (double)k);
        
        total_error1 += error;
        if (error < 1.0) success_count1++;
        
        if (i < 10) {
            printf("  k=%lu: recovered=%.2f, error=%.2f, confidence=%.2f\n",
                   k, recovered_k, error, confidence);
        }
    }
    
    double avg_error1 = total_error1 / NUM_TEST_K;
    double success_rate1 = 100.0 * success_count1 / NUM_TEST_K;
    
    printf("\nResults (no filter):\n");
    printf("  Average error: %.2f\n", avg_error1);
    printf("  Success rate: %.1f%% (%d/%d)\n", success_rate1, success_count1, NUM_TEST_K);
    printf("  Polarity flips: %u\n", ctx1->polarity_flips);
    printf("\n");
    
    // Test 2: TOP-RIGHT quadrant filter (optimal zone)
    printf("Test 2: Spherical recovery (TOP-RIGHT filter)\n");
    SphericalRecoveryContext* ctx2 = init_spherical_recovery(overlay, true, QUADRANT_TOP_RIGHT);
    if (!ctx2) {
        printf("FAILED to create context\n");
        free_spherical_recovery(ctx1);
        free_platonic_overlay(overlay);
        return 1;
    }
    
    double total_error2 = 0.0;
    int success_count2 = 0;
    int filtered_count = 0;
    
    for (int i = 0; i < NUM_TEST_K; i++) {
        uint64_t k = TEST_K_VALUES[i];
        ClockPosition pos = map_k_to_clock_u64(k);
        
        // Check if in TOP-RIGHT quadrant
        Quadrant q = get_quadrant_from_position(pos);
        if (q != QUADRANT_TOP_RIGHT) {
            filtered_count++;
            continue;
        }
        
        double confidence = 0.0;
        double recovered_k = recover_k_spherical(pos, ctx2, &confidence);
        double error = fabs(recovered_k - (double)k);
        
        total_error2 += error;
        if (error < 1.0) success_count2++;
        
        printf("  k=%lu: recovered=%.2f, error=%.2f, confidence=%.2f\n",
               k, recovered_k, error, confidence);
    }
    
    int tested_count2 = NUM_TEST_K - filtered_count;
    double avg_error2 = (tested_count2 > 0) ? total_error2 / tested_count2 : 0.0;
    double success_rate2 = (tested_count2 > 0) ? 100.0 * success_count2 / tested_count2 : 0.0;
    
    printf("\nResults (TOP-RIGHT filter):\n");
    printf("  Tested: %d/%d (filtered %d)\n", tested_count2, NUM_TEST_K, filtered_count);
    printf("  Average error: %.2f\n", avg_error2);
    printf("  Success rate: %.1f%% (%d/%d)\n", success_rate2, success_count2, tested_count2);
    printf("\n");
    
    // Test 3: Optimal zone filter (near 12 o'clock)
    printf("Test 3: Spherical recovery (optimal zone filter)\n");
    SphericalRecoveryContext* ctx3 = init_spherical_recovery(overlay, false, QUADRANT_TOP_RIGHT);
    if (!ctx3) {
        printf("FAILED to create context\n");
        free_spherical_recovery(ctx1);
        free_spherical_recovery(ctx2);
        free_platonic_overlay(overlay);
        return 1;
    }
    
    double total_error3 = 0.0;
    int success_count3 = 0;
    int optimal_count = 0;
    
    for (int i = 0; i < NUM_TEST_K; i++) {
        uint64_t k = TEST_K_VALUES[i];
        ClockPosition pos = map_k_to_clock_u64(k);
        
        // Check if in optimal zone
        if (!is_in_optimal_zone(pos)) {
            continue;
        }
        
        optimal_count++;
        
        double confidence = 0.0;
        double recovered_k = recover_k_spherical(pos, ctx3, &confidence);
        double error = fabs(recovered_k - (double)k);
        
        total_error3 += error;
        if (error < 1.0) success_count3++;
        
        printf("  k=%lu: recovered=%.2f, error=%.2f, confidence=%.2f, optimal=YES\n",
               k, recovered_k, error, confidence);
    }
    
    double avg_error3 = (optimal_count > 0) ? total_error3 / optimal_count : 0.0;
    double success_rate3 = (optimal_count > 0) ? 100.0 * success_count3 / optimal_count : 0.0;
    
    printf("\nResults (optimal zone):\n");
    printf("  In optimal zone: %d/%d\n", optimal_count, NUM_TEST_K);
    printf("  Average error: %.2f\n", avg_error3);
    printf("  Success rate: %.1f%% (%d/%d)\n", success_rate3, success_count3, optimal_count);
    printf("\n");
    
    // Comparison with baseline (4% success rate)
    printf("=== Comparison with Baseline ===\n\n");
    printf("Baseline (linear corrections):\n");
    printf("  Success rate: 4.0%% (2/50)\n");
    printf("  Average error: 77.77\n");
    printf("\n");
    
    printf("Spherical (no filter):\n");
    printf("  Success rate: %.1f%% (%d/50)\n", success_rate1, success_count1);
    printf("  Average error: %.2f\n", avg_error1);
    printf("  Improvement: %.1fx\n", success_rate1 / 4.0);
    printf("\n");
    
    printf("Spherical (TOP-RIGHT filter):\n");
    printf("  Success rate: %.1f%% (%d/%d)\n", success_rate2, success_count2, tested_count2);
    printf("  Average error: %.2f\n", avg_error2);
    if (tested_count2 > 0) {
        printf("  Improvement: %.1fx\n", success_rate2 / 4.0);
    }
    printf("\n");
    
    printf("Spherical (optimal zone):\n");
    printf("  Success rate: %.1f%% (%d/%d)\n", success_rate3, success_count3, optimal_count);
    printf("  Average error: %.2f\n", avg_error3);
    if (optimal_count > 0) {
        printf("  Improvement: %.1fx\n", success_rate3 / 4.0);
    }
    printf("\n");
    
    // Cleanup
    free_spherical_recovery(ctx1);
    free_spherical_recovery(ctx2);
    free_spherical_recovery(ctx3);
    free_platonic_overlay(overlay);
    
    printf("=== Test Complete ===\n\n");
    
    return 0;
}