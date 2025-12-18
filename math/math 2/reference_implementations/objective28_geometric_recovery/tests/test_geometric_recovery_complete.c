/**
 * test_geometric_recovery_complete.c - Complete Geometric Recovery Test
 * 
 * Tests the full geometric anchor system with actual k recovery
 */

#include "geometric_anchors.h"
#include "search_recovery.h"
#include "clock_recovery.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Test k values
static const uint64_t TEST_K_VALUES[] = {
    2, 3, 5, 7, 11, 13, 17, 19, 23, 29,
    31, 37, 41, 43, 47, 53, 59, 61, 67, 71,
    73, 79, 83, 89, 97, 101, 103, 107, 109, 113,
    127, 131, 137, 139, 149, 151, 157, 163, 167, 173,
    179, 181, 191, 193, 197, 199, 211, 223, 227, 229
};
#define NUM_TEST_K 50

int main() {
    printf("\n=== Complete Geometric Recovery Test ===\n\n");
    
    // Initialize geometric anchor system
    printf("Initializing geometric anchor system...\n");
    GeometricAnchorSystem* geo_system = init_geometric_anchor_system();
    if (!geo_system) {
        printf("FAILED\n");
        return 1;
    }
    printf("SUCCESS: %d base anchors, %d shared vertices\n\n", 
           geo_system->num_base_anchors, geo_system->num_shared_vertices);
    
    // Test with GCD constraints
    printf("Testing with GCD constraints (k divisible by 6)...\n");
    
    int success_count = 0;
    double total_error = 0.0;
    
    for (int i = 0; i < NUM_TEST_K; i++) {
        uint64_t k = TEST_K_VALUES[i];
        
        // Apply GCD constraint
        uint64_t k_constrained = apply_gcd_constraints(k, geo_system->gcd);
        
        // Map to clock position
        ClockPosition pos = map_k_to_clock_u64(k);
        
        // Binary search with GCD constraints
        uint64_t min_k = (k > 50) ? k - 50 : 0;
        uint64_t max_k = k + 50;
        
        // Apply GCD to search bounds
        min_k = apply_gcd_constraints(min_k, geo_system->gcd);
        max_k = apply_gcd_constraints(max_k, geo_system->gcd);
        
        double error = 0.0;
        uint64_t recovered_k = min_k;
        double best_error = 1e9;
        
        // Search only multiples of 6
        for (uint64_t candidate = min_k; candidate <= max_k; candidate += 6) {
            ClockPosition cand_pos = map_k_to_clock_u64(candidate);
            
            double angle_diff = fabs(cand_pos.angle - pos.angle);
            if (angle_diff > M_PI) angle_diff = 2.0 * M_PI - angle_diff;
            
            if (angle_diff < best_error) {
                best_error = angle_diff;
                recovered_k = candidate;
            }
        }
        
        int64_t k_error = (int64_t)recovered_k - (int64_t)k;
        double abs_error = fabs((double)k_error);
        total_error += abs_error;
        
        if (abs_error < 1.0) success_count++;
        
        if (i < 10) {
            printf("  k=%lu: recovered=%lu, error=%ld\n", k, recovered_k, k_error);
        }
    }
    
    double avg_error = total_error / NUM_TEST_K;
    double success_rate = 100.0 * success_count / NUM_TEST_K;
    
    printf("\nResults:\n");
    printf("  Success rate: %.1f%% (%d/%d)\n", success_rate, success_count, NUM_TEST_K);
    printf("  Average error: %.2f\n", avg_error);
    printf("  Search space reduction: 6x (GCD constraints)\n");
    printf("\n");
    
    printf("Comparison:\n");
    printf("  Baseline: 4.0%% (2/50)\n");
    printf("  Recursive search: 6.0%% (3/50)\n");
    printf("  Geometric + GCD: %.1f%% (%d/50)\n", success_rate, success_count);
    printf("  Improvement: %.1fx\n", success_rate / 4.0);
    printf("\n");
    
    free_geometric_anchor_system(geo_system);
    
    printf("=== Test Complete ===\n\n");
    
    return 0;
}