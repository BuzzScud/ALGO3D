/**
 * test_multi_sample_intersection.c - Multi-Sample Intersection Analysis
 * 
 * This test:
 * 1. Loads per-sample torus data from CSV files
 * 2. Selects top 5 samples with best reduction factors
 * 3. Computes intersection of all 5 samples
 * 4. Measures combined reduction factor
 * 5. Validates true k is within intersection
 * 
 * Expected: 10-20x reduction (90-95% elimination)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../include/multi_torus_tracker.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"

#define MAX_SAMPLES 20
#define TOP_N 5

/**
 * Sample data from CSV
 */
typedef struct {
    int sample_id;
    uint64_t true_k;
    int num_tori;
    double intersection_size;
    double reduction_factor;
    bool contains_true_k;
    double k_min;
    double k_max;
} SampleData;

/**
 * Load sample data from CSV
 */
int load_sample_data(const char* filename, SampleData* samples, int max_samples) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        printf("ERROR: Could not open %s\n", filename);
        return 0;
    }
    
    // Skip header
    char line[1024];
    fgets(line, sizeof(line), f);
    
    int count = 0;
    while (fgets(line, sizeof(line), f) && count < max_samples) {
        SampleData* s = &samples[count];
        int contains_int;
        
        // Parse CSV line
        int parsed = sscanf(line, "%d,%lu,%d,%lf,%lf,%d,%lf",
            &s->sample_id,
            &s->true_k,
            &s->num_tori,
            &s->intersection_size,
            &s->reduction_factor,
            &contains_int,
            &s->k_min
        );
        
        if (parsed >= 6) {
            s->contains_true_k = (contains_int != 0);
            // k_max = k_min + intersection_size
            s->k_max = s->k_min + s->intersection_size;
            count++;
        }
    }
    
    fclose(f);
    return count;
}

/**
 * Compare samples by reduction factor (descending)
 */
int compare_samples(const void* a, const void* b) {
    const SampleData* sa = (const SampleData*)a;
    const SampleData* sb = (const SampleData*)b;
    
    if (sa->reduction_factor > sb->reduction_factor) return -1;
    if (sa->reduction_factor < sb->reduction_factor) return 1;
    return 0;
}

/**
 * Create mock tracker from sample data
 */
MultiTorusTracker* create_tracker_from_sample(const SampleData* sample, uint64_t max_k) {
    MultiTorusTracker* tracker = malloc(sizeof(MultiTorusTracker));
    if (!tracker) return NULL;
    
    tracker->max_tori = sample->num_tori;
    tracker->num_tori = sample->num_tori;
    tracker->original_space = max_k;
    
    tracker->intersection_valid = true;
    tracker->intersection_k_min = sample->k_min;
    tracker->intersection_k_max = sample->k_max;
    tracker->reduced_space = (uint64_t)sample->intersection_size;
    tracker->reduction_factor = sample->reduction_factor;
    
    // Allocate arrays (not used in this test)
    tracker->k_history = NULL;
    tracker->history_size = 0;
    tracker->history_index = 0;
    tracker->tori = NULL;
    
    return tracker;
}

/**
 * Test multi-sample intersection for a specific bit length
 */
void test_bit_length(const char* csv_file, int bit_length, uint64_t max_k) {
    printf("\n========================================\n");
    printf("Testing %d-bit samples\n", bit_length);
    printf("========================================\n\n");
    
    // Load sample data
    SampleData samples[MAX_SAMPLES];
    int num_samples = load_sample_data(csv_file, samples, MAX_SAMPLES);
    
    if (num_samples == 0) {
        printf("ERROR: No samples loaded from %s\n", csv_file);
        return;
    }
    
    printf("Loaded %d samples from %s\n\n", num_samples, csv_file);
    
    // Sort by reduction factor (best first)
    qsort(samples, num_samples, sizeof(SampleData), compare_samples);
    
    // Print top 5 samples
    printf("Top %d samples by reduction factor:\n", TOP_N);
    printf("%-10s %-12s %-15s %-15s %-10s\n", 
           "Sample ID", "True k", "Reduction", "Intersection", "Contains k");
    printf("%-10s %-12s %-15s %-15s %-10s\n",
           "---------", "------", "---------", "------------", "----------");
    
    for (int i = 0; i < TOP_N && i < num_samples; i++) {
        printf("%-10d %-12lu %-15.2fx %-15.0f %-10s\n",
               samples[i].sample_id,
               samples[i].true_k,
               samples[i].reduction_factor,
               samples[i].intersection_size,
               samples[i].contains_true_k ? "YES" : "NO");
    }
    printf("\n");
    
    // Create trackers for top N samples
    MultiTorusTracker* trackers[TOP_N];
    for (int i = 0; i < TOP_N && i < num_samples; i++) {
        trackers[i] = create_tracker_from_sample(&samples[i], max_k);
    }
    
    // Compute multi-sample intersection
    printf("Computing intersection of top %d samples...\n", TOP_N);
    
    MultiSampleIntersection* intersection = compute_multi_sample_intersection(
        trackers, TOP_N, samples[0].true_k, max_k
    );
    
    if (!intersection) {
        printf("ERROR: Failed to compute intersection\n");
        return;
    }
    
    // Print results
    printf("\nMulti-Sample Intersection Results:\n");
    printf("  Number of samples: %u\n", intersection->num_samples);
    printf("  Original space: %.0f\n", intersection->original_space);
    printf("  Intersection k range: [%.2f, %.2f]\n", 
           intersection->intersection_k_min, intersection->intersection_k_max);
    printf("  Intersection size: %.2f\n", intersection->intersection_size);
    printf("  Reduction factor: %.2fx\n", intersection->reduction_factor);
    printf("  Contains true k: %s\n", intersection->contains_true_k ? "YES ✓" : "NO ✗");
    
    // Calculate percentage eliminated
    double percent_eliminated = (1.0 - (1.0 / intersection->reduction_factor)) * 100.0;
    printf("  Search space eliminated: %.2f%%\n", percent_eliminated);
    
    // Compare with individual samples
    printf("\nComparison with Individual Samples:\n");
    printf("  Best individual: %.2fx (%.2f%% eliminated)\n",
           samples[0].reduction_factor,
           (1.0 - (1.0 / samples[0].reduction_factor)) * 100.0);
    printf("  Multi-sample: %.2fx (%.2f%% eliminated)\n",
           intersection->reduction_factor,
           percent_eliminated);
    
    double improvement = intersection->reduction_factor / samples[0].reduction_factor;
    printf("  Improvement: %.2fx\n", improvement);
    
    // Cleanup
    free_multi_sample_intersection(intersection);
    for (int i = 0; i < TOP_N && i < num_samples; i++) {
        free(trackers[i]);
    }
}

int main() {
    printf("===========================================\n");
    printf("MULTI-SAMPLE INTERSECTION ANALYSIS\n");
    printf("===========================================\n");
    printf("\nGoal: Combine top 5 samples to achieve 10-20x reduction\n");
    printf("Expected: 90-95%% search space elimination\n");
    
    // Test 8-bit
    test_bit_length("per_sample_8bit.csv", 8, 255);
    
    // Test 16-bit
    test_bit_length("per_sample_16bit.csv", 16, 65535);
    
    // Test 32-bit
    test_bit_length("per_sample_32bit.csv", 32, 4294967295UL);
    
    printf("\n===========================================\n");
    printf("ANALYSIS COMPLETE\n");
    printf("===========================================\n");
    
    return 0;
}