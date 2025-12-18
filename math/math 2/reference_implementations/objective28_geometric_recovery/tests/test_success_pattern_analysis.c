/**
 * test_success_pattern_analysis.c - Analyze Successful K Values
 * 
 * Analyzes the 10 k values that v2 successfully recovered.
 * Goal: Find common patterns to replicate success.
 */

#include "search_recovery_v2.h"
#include "clock_recovery.h"
#include "platonic_solids.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define PI M_PI
#define PHI ((1.0 + sqrt(5.0)) / 2.0)

// All 50 test k values
static const uint64_t ALL_K_VALUES[] = {
    2, 3, 5, 7, 11, 13, 17, 19, 23, 29,
    31, 37, 41, 43, 47, 53, 59, 61, 67, 71,
    73, 79, 83, 89, 97, 101, 103, 107, 109, 113,
    127, 131, 137, 139, 149, 151, 157, 163, 167, 173,
    179, 181, 191, 193, 197, 199, 211, 223, 227, 229
};
#define NUM_K 50

int main() {
    printf("\n=== Success Pattern Analysis ===\n\n");
    
    // Create context
    PlatonicOverlay* overlay = create_platonic_overlay(0.5);
    if (!overlay) {
        printf("FAILED to create overlay\n");
        return 1;
    }
    
    SearchRecoveryV2Context* ctx = init_search_recovery_v2(overlay, 0, 300);
    if (!ctx) {
        printf("FAILED to create context\n");
        free_platonic_overlay(overlay);
        return 1;
    }
    
    // Test all k values and collect data
    typedef struct {
        uint64_t k;
        double angle;
        double angle_deg;
        int quadrant;
        uint64_t recovered_k;
        int64_t error;
        double confidence;
        bool success;
        double nearest_anchor_dist;
        int nearest_anchor_idx;
    } KAnalysis;
    
    KAnalysis* analyses = malloc(NUM_K * sizeof(KAnalysis));
    
    int total_success = 0;
    
    printf("Analyzing all %d k values...\n\n", NUM_K);
    
    for (int i = 0; i < NUM_K; i++) {
        uint64_t k = ALL_K_VALUES[i];
        ClockPosition pos = map_k_to_clock_u64(k);
        
        ConfidenceMetrics metrics;
        uint64_t recovered_k = enhanced_search_k(pos, ctx, &metrics);
        
        int64_t k_error = (int64_t)recovered_k - (int64_t)k;
        double abs_error = fabs((double)k_error);
        bool success = (abs_error < 1.0);
        
        if (success) total_success++;
        
        // Determine quadrant
        double angle_deg = pos.angle * 180.0 / PI;
        int quadrant = (int)(angle_deg / 90.0);
        if (quadrant >= 4) quadrant = 3;
        
        // Find nearest anchor (12 anchors at k = 0, 25, 50, ..., 275)
        int nearest_idx = 0;
        double min_dist = 1e9;
        for (int a = 0; a < 12; a++) {
            uint64_t anchor_k = (a * 300) / 12;
            double dist = fabs((double)k - (double)anchor_k);
            if (dist < min_dist) {
                min_dist = dist;
                nearest_idx = a;
            }
        }
        
        // Store analysis
        analyses[i].k = k;
        analyses[i].angle = pos.angle;
        analyses[i].angle_deg = angle_deg;
        analyses[i].quadrant = quadrant;
        analyses[i].recovered_k = recovered_k;
        analyses[i].error = k_error;
        analyses[i].confidence = metrics.overall;
        analyses[i].success = success;
        analyses[i].nearest_anchor_dist = min_dist;
        analyses[i].nearest_anchor_idx = nearest_idx;
    }
    
    // Print successful cases
    printf("=== SUCCESSFUL RECOVERIES (%d total) ===\n\n", total_success);
    printf("%-4s %-8s %-8s %-4s %-12s %-10s %-10s\n",
           "k", "Angle", "Quad", "Anch", "Anch_Dist", "Confidence", "Notes");
    printf("------------------------------------------------------------------------\n");
    
    for (int i = 0; i < NUM_K; i++) {
        if (analyses[i].success) {
            uint64_t anchor_k = (analyses[i].nearest_anchor_idx * 300) / 12;
            printf("%-4lu %-8.2f Q%-7d %-4d %-12.1f %-10.3f ",
                   analyses[i].k,
                   analyses[i].angle_deg,
                   analyses[i].quadrant + 1,
                   analyses[i].nearest_anchor_idx,
                   analyses[i].nearest_anchor_dist,
                   analyses[i].confidence);
            
            // Add notes
            if (analyses[i].nearest_anchor_dist < 15) {
                printf("NEAR ANCHOR (k=%lu)", anchor_k);
            } else if (analyses[i].quadrant == 3) {
                printf("Q4 ADVANTAGE");
            }
            printf("\n");
        }
    }
    
    // Statistical analysis of successes
    printf("\n=== SUCCESS PATTERN STATISTICS ===\n\n");
    
    // Quadrant distribution
    int quad_success[4] = {0};
    int quad_total[4] = {0};
    for (int i = 0; i < NUM_K; i++) {
        quad_total[analyses[i].quadrant]++;
        if (analyses[i].success) {
            quad_success[analyses[i].quadrant]++;
        }
    }
    
    printf("Quadrant distribution:\n");
    for (int q = 0; q < 4; q++) {
        double rate = quad_total[q] > 0 ? 100.0 * quad_success[q] / quad_total[q] : 0.0;
        printf("  Q%d: %.1f%% (%d/%d)\n", q+1, rate, quad_success[q], quad_total[q]);
    }
    
    // Distance to nearest anchor
    printf("\nDistance to nearest anchor:\n");
    double success_avg_dist = 0.0;
    double failure_avg_dist = 0.0;
    int success_count = 0;
    int failure_count = 0;
    
    for (int i = 0; i < NUM_K; i++) {
        if (analyses[i].success) {
            success_avg_dist += analyses[i].nearest_anchor_dist;
            success_count++;
        } else {
            failure_avg_dist += analyses[i].nearest_anchor_dist;
            failure_count++;
        }
    }
    
    success_avg_dist /= success_count;
    failure_avg_dist /= failure_count;
    
    printf("  Successes: avg %.2f distance to anchor\n", success_avg_dist);
    printf("  Failures:  avg %.2f distance to anchor\n", failure_avg_dist);
    printf("  Difference: %.2f (%.1f%% closer)\n", 
           failure_avg_dist - success_avg_dist,
           100.0 * (failure_avg_dist - success_avg_dist) / failure_avg_dist);
    
    // Confidence distribution
    printf("\nConfidence distribution:\n");
    double success_avg_conf = 0.0;
    double failure_avg_conf = 0.0;
    
    for (int i = 0; i < NUM_K; i++) {
        if (analyses[i].success) {
            success_avg_conf += analyses[i].confidence;
        } else {
            failure_avg_conf += analyses[i].confidence;
        }
    }
    
    success_avg_conf /= success_count;
    failure_avg_conf /= failure_count;
    
    printf("  Successes: avg %.3f confidence\n", success_avg_conf);
    printf("  Failures:  avg %.3f confidence\n", failure_avg_conf);
    
    // Anchor index distribution
    printf("\nAnchor index distribution (successes):\n");
    int anchor_success_count[12] = {0};
    for (int i = 0; i < NUM_K; i++) {
        if (analyses[i].success) {
            anchor_success_count[analyses[i].nearest_anchor_idx]++;
        }
    }
    
    for (int a = 0; a < 12; a++) {
        if (anchor_success_count[a] > 0) {
            uint64_t anchor_k = (a * 300) / 12;
            double anchor_angle = anchor_k * PI * PHI;
            double anchor_deg = fmod(anchor_angle, 2*PI) * 180.0 / PI;
            printf("  Anchor %d (k=%lu, %.0f°): %d successes\n",
                   a, anchor_k, anchor_deg, anchor_success_count[a]);
        }
    }
    
    // Recommendations
    printf("\n=== RECOMMENDATIONS ===\n\n");
    
    if (success_avg_dist < failure_avg_dist * 0.8) {
        printf("✓ Successes are significantly closer to anchors\n");
        printf("  → Add more anchors in success regions\n");
    }
    
    if (quad_success[3] > quad_success[0] + quad_success[1] + quad_success[2]) {
        printf("✓ Q4 dominates successes\n");
        printf("  → Optimize Q4 with tighter search\n");
    }
    
    printf("\nNext steps:\n");
    printf("1. Add anchors near successful k values\n");
    printf("2. Optimize Q4 quadrant (best performer)\n");
    printf("3. Use success patterns to guide search\n");
    
    // Cleanup
    free(analyses);
    free_search_recovery_v2(ctx);
    free_platonic_overlay(overlay);
    
    return 0;
}