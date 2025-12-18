#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include "../include/search_recovery_v2.h"
#include "../include/platonic_solids.h"

// Test k values (same as v2)
#define NUM_TEST_K 50
static const uint64_t TEST_K_VALUES[NUM_TEST_K] = {
    2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71,
    73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151,
    157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229
};

typedef struct {
    uint64_t k;
    double angle_deg;
    double radius;
    int quadrant;
    uint64_t nearest_anchor;
    double anchor_distance;
    uint64_t recovered_k;
    int64_t error;
    bool success;
    double confidence;
} DetailedResult;

const char* quadrant_name(int q) {
    switch(q) {
        case 0: return "Q1";
        case 1: return "Q2";
        case 2: return "Q3";
        case 3: return "Q4";
        default: return "??";
    }
}

uint64_t find_nearest_anchor_k(uint64_t k) {
    // 12 anchors at k = 0, 25, 50, 75, 100, 125, 150, 175, 200, 225, 250, 275
    uint64_t anchor_spacing = 25;
    uint64_t anchor_idx = (k + anchor_spacing/2) / anchor_spacing;
    if (anchor_idx > 11) anchor_idx = 11;
    return anchor_idx * anchor_spacing;
}

int main() {
    printf("=== DEEP SUCCESS PATTERN ANALYSIS ===\n\n");
    
    // Initialize Platonic overlay
    PlatonicOverlay* overlay = create_platonic_overlay(1e-6);
    if (!overlay) {
        fprintf(stderr, "Failed to create overlay\n");
        return 1;
    }
    
    // Initialize context
    SearchRecoveryV2Context* ctx = init_search_recovery_v2(overlay, 0, 300);
    if (!ctx) {
        fprintf(stderr, "Failed to create context\n");
        free_platonic_overlay(overlay);
        return 1;
    }
    
    // Collect detailed results
    DetailedResult results[NUM_TEST_K];
    int num_success = 0;
    int num_failure = 0;
    
    printf("Collecting data for %d test cases...\n\n", NUM_TEST_K);
    
    for (int i = 0; i < NUM_TEST_K; i++) {
        uint64_t k = TEST_K_VALUES[i];
        ClockPosition pos = map_k_to_clock_u64(k);
        
        ConfidenceMetrics metrics;
        uint64_t recovered_k = enhanced_search_k(pos, ctx, &metrics);
        
        int64_t error = (int64_t)recovered_k - (int64_t)k;
        bool success = (llabs(error) < 1);
        
        results[i].k = k;
        results[i].angle_deg = pos.angle * 180.0 / M_PI;
        results[i].radius = pos.radius;
        results[i].quadrant = get_quadrant(pos.angle);
        results[i].nearest_anchor = find_nearest_anchor_k(k);
        results[i].anchor_distance = fabs((double)k - (double)results[i].nearest_anchor);
        results[i].recovered_k = recovered_k;
        results[i].error = error;
        results[i].success = success;
        results[i].confidence = metrics.overall;
        
        if (success) num_success++;
        else num_failure++;
    }
    
    printf("Results: %d successes, %d failures\n\n", num_success, num_failure);
    
    // Analysis 1: Success vs Failure Comparison
    printf("=== ANALYSIS 1: SUCCESS vs FAILURE PATTERNS ===\n\n");
    
    printf("SUCCESSFUL RECOVERIES (%d):\n", num_success);
    printf("%-6s %-10s %-8s %-6s %-10s %-10s %-12s %-10s\n",
           "k", "Angle", "Radius", "Quad", "Anchor", "Dist", "Confidence", "Recovered");
    printf("--------------------------------------------------------------------------------\n");
    
    double success_avg_angle = 0, success_avg_radius = 0, success_avg_dist = 0, success_avg_conf = 0;
    for (int i = 0; i < NUM_TEST_K; i++) {
        if (results[i].success) {
            printf("%-6lu %-10.2f %-8.4f %-6s %-10lu %-10.1f %-12.3f %-10lu\n",
                   results[i].k, results[i].angle_deg, results[i].radius,
                   quadrant_name(results[i].quadrant), results[i].nearest_anchor,
                   results[i].anchor_distance, results[i].confidence, results[i].recovered_k);
            success_avg_angle += results[i].angle_deg;
            success_avg_radius += results[i].radius;
            success_avg_dist += results[i].anchor_distance;
            success_avg_conf += results[i].confidence;
        }
    }
    
    if (num_success > 0) {
        success_avg_angle /= num_success;
        success_avg_radius /= num_success;
        success_avg_dist /= num_success;
        success_avg_conf /= num_success;
    }
    
    printf("\nFAILED RECOVERIES (first 20 of %d):\n", num_failure);
    printf("%-6s %-10s %-8s %-6s %-10s %-10s %-12s %-10s %-10s\n",
           "k", "Angle", "Radius", "Quad", "Anchor", "Dist", "Confidence", "Recovered", "Error");
    printf("--------------------------------------------------------------------------------------------\n");
    
    double failure_avg_angle = 0, failure_avg_radius = 0, failure_avg_dist = 0, failure_avg_conf = 0;
    int failure_count = 0;
    for (int i = 0; i < NUM_TEST_K && failure_count < 20; i++) {
        if (!results[i].success) {
            printf("%-6lu %-10.2f %-8.4f %-6s %-10lu %-10.1f %-12.3f %-10lu %-10ld\n",
                   results[i].k, results[i].angle_deg, results[i].radius,
                   quadrant_name(results[i].quadrant), results[i].nearest_anchor,
                   results[i].anchor_distance, results[i].confidence, 
                   results[i].recovered_k, results[i].error);
            failure_avg_angle += results[i].angle_deg;
            failure_avg_radius += results[i].radius;
            failure_avg_dist += results[i].anchor_distance;
            failure_avg_conf += results[i].confidence;
            failure_count++;
        }
    }
    
    if (num_failure > 0) {
        failure_avg_angle /= num_failure;
        failure_avg_radius /= num_failure;
        failure_avg_dist /= num_failure;
        failure_avg_conf /= num_failure;
    }
    
    printf("\n=== STATISTICAL COMPARISON ===\n\n");
    printf("Metric              Success Avg    Failure Avg    Difference\n");
    printf("----------------------------------------------------------------\n");
    printf("Angle (degrees)     %-14.2f %-14.2f %-10.2f\n", 
           success_avg_angle, failure_avg_angle, success_avg_angle - failure_avg_angle);
    printf("Radius              %-14.4f %-14.4f %-10.4f\n",
           success_avg_radius, failure_avg_radius, success_avg_radius - failure_avg_radius);
    printf("Anchor Distance     %-14.2f %-14.2f %-10.2f\n",
           success_avg_dist, failure_avg_dist, success_avg_dist - failure_avg_dist);
    printf("Confidence          %-14.3f %-14.3f %-10.3f\n",
           success_avg_conf, failure_avg_conf, success_avg_conf - failure_avg_conf);
    
    // Analysis 2: Radius Distribution
    printf("\n=== ANALYSIS 2: RADIUS DISTRIBUTION ===\n\n");
    
    // Find min/max radius
    double min_radius = results[0].radius, max_radius = results[0].radius;
    for (int i = 1; i < NUM_TEST_K; i++) {
        if (results[i].radius < min_radius) min_radius = results[i].radius;
        if (results[i].radius > max_radius) max_radius = results[i].radius;
    }
    
    printf("Radius range: %.4f to %.4f\n\n", min_radius, max_radius);
    
    // Create radius bins
    #define NUM_BINS 10
    double bin_width = (max_radius - min_radius) / NUM_BINS;
    int success_bins[NUM_BINS] = {0};
    int total_bins[NUM_BINS] = {0};
    
    for (int i = 0; i < NUM_TEST_K; i++) {
        int bin = (int)((results[i].radius - min_radius) / bin_width);
        if (bin >= NUM_BINS) bin = NUM_BINS - 1;
        total_bins[bin]++;
        if (results[i].success) success_bins[bin]++;
    }
    
    printf("Radius Bin         Count    Success    Rate\n");
    printf("----------------------------------------------\n");
    for (int i = 0; i < NUM_BINS; i++) {
        double bin_start = min_radius + i * bin_width;
        double bin_end = bin_start + bin_width;
        double rate = total_bins[i] > 0 ? (100.0 * success_bins[i] / total_bins[i]) : 0.0;
        printf("%.4f - %.4f    %-8d %-10d %.1f%%\n",
               bin_start, bin_end, total_bins[i], success_bins[i], rate);
    }
    
    // Analysis 3: Angle Distribution
    printf("\n=== ANALYSIS 3: ANGLE DISTRIBUTION ===\n\n");
    
    int angle_success_bins[12] = {0};  // 30-degree bins
    int angle_total_bins[12] = {0};
    
    for (int i = 0; i < NUM_TEST_K; i++) {
        int bin = (int)(results[i].angle_deg / 30.0);
        if (bin >= 12) bin = 11;
        angle_total_bins[bin]++;
        if (results[i].success) angle_success_bins[bin]++;
    }
    
    printf("Angle Range        Count    Success    Rate\n");
    printf("----------------------------------------------\n");
    for (int i = 0; i < 12; i++) {
        int start = i * 30;
        int end = start + 30;
        double rate = angle_total_bins[i] > 0 ? (100.0 * angle_success_bins[i] / angle_total_bins[i]) : 0.0;
        printf("%3d° - %3d°        %-8d %-10d %.1f%%\n",
               start, end, angle_total_bins[i], angle_success_bins[i], rate);
    }
    
    // Analysis 4: Anchor Distance Distribution
    printf("\n=== ANALYSIS 4: ANCHOR DISTANCE DISTRIBUTION ===\n\n");
    
    int dist_success_bins[13] = {0};  // 0-12 distance bins
    int dist_total_bins[13] = {0};
    
    for (int i = 0; i < NUM_TEST_K; i++) {
        int bin = (int)results[i].anchor_distance;
        if (bin > 12) bin = 12;
        dist_total_bins[bin]++;
        if (results[i].success) dist_success_bins[bin]++;
    }
    
    printf("Distance    Count    Success    Rate\n");
    printf("----------------------------------------\n");
    for (int i = 0; i <= 12; i++) {
        double rate = dist_total_bins[i] > 0 ? (100.0 * dist_success_bins[i] / dist_total_bins[i]) : 0.0;
        printf("%-11d %-8d %-10d %.1f%%\n",
               i, dist_total_bins[i], dist_success_bins[i], rate);
    }
    
    // Analysis 5: Key Insights
    printf("\n=== KEY INSIGHTS ===\n\n");
    
    printf("1. RADIUS CORRELATION:\n");
    if (fabs(success_avg_radius - failure_avg_radius) > 0.01) {
        printf("   ✓ Significant difference: %.4f (success) vs %.4f (failure)\n",
               success_avg_radius, failure_avg_radius);
        printf("   → Radius could be a discriminating factor!\n");
    } else {
        printf("   ✗ No significant difference in radius\n");
    }
    
    printf("\n2. ANCHOR DISTANCE CORRELATION:\n");
    if (success_avg_dist < failure_avg_dist) {
        printf("   ✓ Successes closer to anchors: %.2f vs %.2f\n",
               success_avg_dist, failure_avg_dist);
        printf("   → Confirms anchor proximity hypothesis\n");
    } else {
        printf("   ✗ No clear anchor distance pattern\n");
    }
    
    printf("\n3. CONFIDENCE CORRELATION:\n");
    if (success_avg_conf > failure_avg_conf) {
        printf("   ✓ Higher confidence for successes: %.3f vs %.3f\n",
               success_avg_conf, failure_avg_conf);
        printf("   → Confidence metric is meaningful\n");
    } else {
        printf("   ✗ Confidence doesn't correlate with success\n");
    }
    
    printf("\n4. QUADRANT PERFORMANCE:\n");
    int quad_success[4] = {0}, quad_total[4] = {0};
    for (int i = 0; i < NUM_TEST_K; i++) {
        quad_total[results[i].quadrant]++;
        if (results[i].success) quad_success[results[i].quadrant]++;
    }
    for (int q = 0; q < 4; q++) {
        double rate = quad_total[q] > 0 ? (100.0 * quad_success[q] / quad_total[q]) : 0.0;
        printf("   %s: %.1f%% (%d/%d)\n", quadrant_name(q), rate, quad_success[q], quad_total[q]);
    }
    
    free_search_recovery_v2(ctx);
    free_platonic_overlay(overlay);
    
    printf("\n=== ANALYSIS COMPLETE ===\n");
    return 0;
}
