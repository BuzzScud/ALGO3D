/**
 * test_deep_recovery_analysis.c - Deep Analysis of Recovery Algorithm
 * 
 * This test analyzes:
 * 1. What does the 62% improvement actually mean?
 * 2. Are we recovering the correct k values?
 * 3. How do the 4 correction steps contribute?
 * 4. What is the relationship between anchors and accuracy?
 * 5. Is there additional oscillation in the correction steps?
 * 6. What polytopic structures exist in the correction process?
 */

#include "clock_inverse_mapping.h"
#include "clock_recovery.h"
#include "platonic_solids.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

// Test k values (known primes for validation)
static const uint64_t TEST_K_VALUES[] = {
    2, 3, 5, 7, 11, 13, 17, 19, 23, 29,
    31, 37, 41, 43, 47, 53, 59, 61, 67, 71,
    73, 79, 83, 89, 97, 101, 103, 107, 109, 113,
    127, 131, 137, 139, 149, 151, 157, 163, 167, 173,
    179, 181, 191, 193, 197, 199, 211, 223, 227, 229
};
#define NUM_TEST_K 50

// Structure to track correction steps
typedef struct {
    double step1_basic;      // Basic inverse: θ / (π·φ)
    double step2_ring;       // After ring correction
    double step3_position;   // After position correction
    double step4_anchor;     // After anchor correction
    double actual_k;         // The actual k value
    double error_step1;      // Error after step 1
    double error_step2;      // Error after step 2
    double error_step3;      // Error after step 3
    double error_step4;      // Error after step 4
    ClockPosition position;  // Clock position for this k
} RecoverySteps;

// Forward declarations
void analyze_single_k(uint64_t k, PlatonicOverlay* overlay, RecoverySteps* steps);
void analyze_correction_oscillation(RecoverySteps* all_steps, int num_steps);
void analyze_polytopic_structure(RecoverySteps* all_steps, int num_steps);
void analyze_anchor_relationships(RecoverySteps* all_steps, int num_steps, PlatonicOverlay* overlay);
double compute_step_by_step_k(ClockPosition pos, ClockPosition* anchors, int num_anchors, int stop_at_step);

int main() {
    printf("\n=== Deep Recovery Analysis ===\n\n");
    
    // Create Platonic overlay
    printf("Creating Platonic overlay...\n");
    PlatonicOverlay* overlay = create_platonic_overlay(0.5);
    if (!overlay) {
        printf("FAILED to create overlay\n");
        return 1;
    }
    printf("SUCCESS: %u shared vertices, 5 Platonic solids\n\n", overlay->num_shared_vertices);
    
    // Allocate array for all recovery steps
    RecoverySteps* all_steps = calloc(NUM_TEST_K, sizeof(RecoverySteps));
    if (!all_steps) {
        printf("FAILED to allocate memory\n");
        free_platonic_overlay(overlay);
        return 1;
    }
    
    // Analyze each test k value
    printf("Analyzing %d test k values...\n\n", NUM_TEST_K);
    for (int i = 0; i < NUM_TEST_K; i++) {
        analyze_single_k(TEST_K_VALUES[i], overlay, &all_steps[i]);
    }
    
    // Print summary statistics
    printf("\n=== Summary Statistics ===\n\n");
    
    double avg_error_step1 = 0, avg_error_step2 = 0, avg_error_step3 = 0, avg_error_step4 = 0;
    double min_error_step4 = 1e9, max_error_step4 = 0;
    int perfect_recoveries = 0;
    
    for (int i = 0; i < NUM_TEST_K; i++) {
        avg_error_step1 += all_steps[i].error_step1;
        avg_error_step2 += all_steps[i].error_step2;
        avg_error_step3 += all_steps[i].error_step3;
        avg_error_step4 += all_steps[i].error_step4;
        
        if (all_steps[i].error_step4 < min_error_step4) min_error_step4 = all_steps[i].error_step4;
        if (all_steps[i].error_step4 > max_error_step4) max_error_step4 = all_steps[i].error_step4;
        
        if (fabs(all_steps[i].error_step4) < 1.0) perfect_recoveries++;
    }
    
    avg_error_step1 /= NUM_TEST_K;
    avg_error_step2 /= NUM_TEST_K;
    avg_error_step3 /= NUM_TEST_K;
    avg_error_step4 /= NUM_TEST_K;
    
    printf("Average Errors:\n");
    printf("  Step 1 (Basic):    %.2f\n", avg_error_step1);
    printf("  Step 2 (Ring):     %.2f\n", avg_error_step2);
    printf("  Step 3 (Position): %.2f\n", avg_error_step3);
    printf("  Step 4 (Anchor):   %.2f\n", avg_error_step4);
    printf("\n");
    
    printf("Error Reduction:\n");
    printf("  Step 1 → 2: %.2f%% reduction\n", 100.0 * (avg_error_step1 - avg_error_step2) / avg_error_step1);
    printf("  Step 2 → 3: %.2f%% reduction\n", 100.0 * (avg_error_step2 - avg_error_step3) / avg_error_step2);
    printf("  Step 3 → 4: %.2f%% reduction\n", 100.0 * (avg_error_step3 - avg_error_step4) / avg_error_step3);
    printf("  Overall:    %.2f%% reduction\n", 100.0 * (avg_error_step1 - avg_error_step4) / avg_error_step1);
    printf("\n");
    
    printf("Final Error Range:\n");
    printf("  Minimum: %.2f\n", min_error_step4);
    printf("  Maximum: %.2f\n", max_error_step4);
    printf("  Perfect recoveries (error < 1.0): %d / %d (%.1f%%)\n", 
           perfect_recoveries, NUM_TEST_K, 100.0 * perfect_recoveries / NUM_TEST_K);
    printf("\n");
    
    // Analyze oscillation patterns
    printf("\n=== Oscillation Analysis ===\n\n");
    analyze_correction_oscillation(all_steps, NUM_TEST_K);
    
    // Analyze polytopic structure
    printf("\n=== Polytopic Structure Analysis ===\n\n");
    analyze_polytopic_structure(all_steps, NUM_TEST_K);
    
    // Analyze anchor relationships
    printf("\n=== Anchor Relationship Analysis ===\n\n");
    analyze_anchor_relationships(all_steps, NUM_TEST_K, overlay);
    
    // Cleanup
    free(all_steps);
    free_platonic_overlay(overlay);
    
    printf("\n=== Analysis Complete ===\n\n");
    
    return 0;
}

void analyze_single_k(uint64_t k, PlatonicOverlay* overlay, RecoverySteps* steps) {
    // Store actual k
    steps->actual_k = (double)k;
    
    // Map k to clock position
    steps->position = map_k_to_clock_u64(k);
    
    // Get Icosahedron anchors
    PlatonicSolid* icosa = &overlay->solids[SOLID_ICOSAHEDRON];
    
    // Compute k at each step
    steps->step1_basic = compute_step_by_step_k(steps->position, icosa->clock_positions, icosa->num_vertices, 1);
    steps->step2_ring = compute_step_by_step_k(steps->position, icosa->clock_positions, icosa->num_vertices, 2);
    steps->step3_position = compute_step_by_step_k(steps->position, icosa->clock_positions, icosa->num_vertices, 3);
    steps->step4_anchor = compute_step_by_step_k(steps->position, icosa->clock_positions, icosa->num_vertices, 4);
    
    // Compute errors
    steps->error_step1 = fabs(steps->step1_basic - steps->actual_k);
    steps->error_step2 = fabs(steps->step2_ring - steps->actual_k);
    steps->error_step3 = fabs(steps->step3_position - steps->actual_k);
    steps->error_step4 = fabs(steps->step4_anchor - steps->actual_k);
    
    // Print detailed results for first 10 k values
    static int count = 0;
    if (count < 10) {
        printf("k = %lu:\n", k);
        printf("  Clock: ring=%d, pos=%d, angle=%.4f\n", 
               steps->position.ring, steps->position.position, steps->position.angle);
        printf("  Step 1 (Basic):    k ≈ %.2f, error = %.2f\n", steps->step1_basic, steps->error_step1);
        printf("  Step 2 (Ring):     k ≈ %.2f, error = %.2f\n", steps->step2_ring, steps->error_step2);
        printf("  Step 3 (Position): k ≈ %.2f, error = %.2f\n", steps->step3_position, steps->error_step3);
        printf("  Step 4 (Anchor):   k ≈ %.2f, error = %.2f\n", steps->step4_anchor, steps->error_step4);
        printf("\n");
    }
    count++;
}

double compute_step_by_step_k(ClockPosition pos, ClockPosition* anchors, int num_anchors, int stop_at_step) {
    const double PHI = (1.0 + sqrt(5.0)) / 2.0;
    const double PI = M_PI;
    
    // Step 1: Basic inverse
    double k = pos.angle / (PI * PHI);
    if (stop_at_step == 1) return k;
    
    // Step 2: Ring correction
    double ring_offsets[] = {0, 12, 72, 132, 232};
    if (pos.ring < 5) {
        k += ring_offsets[pos.ring];
    }
    if (stop_at_step == 2) return k;
    
    // Step 3: Position correction
    k += pos.position;
    if (stop_at_step == 3) return k;
    
    // Step 4: Anchor correction
    if (anchors && num_anchors > 0) {
        // Find 3 nearest anchors
        double distances[12];
        int indices[12];
        
        for (int i = 0; i < num_anchors && i < 12; i++) {
            double dx = pos.angle - anchors[i].angle;
            double dy = pos.radius - anchors[i].radius;
            distances[i] = sqrt(dx*dx + dy*dy);
            indices[i] = i;
        }
        
        // Simple bubble sort to find 3 nearest
        for (int i = 0; i < num_anchors - 1 && i < 11; i++) {
            for (int j = 0; j < num_anchors - i - 1 && j < 11; j++) {
                if (distances[j] > distances[j+1]) {
                    double tmp_d = distances[j];
                    distances[j] = distances[j+1];
                    distances[j+1] = tmp_d;
                    
                    int tmp_i = indices[j];
                    indices[j] = indices[j+1];
                    indices[j+1] = tmp_i;
                }
            }
        }
        
        // Weighted correction using 3 nearest anchors
        double total_weight = 0;
        double correction = 0;
        
        for (int i = 0; i < 3 && i < num_anchors; i++) {
            double weight = 1.0 / (distances[i] + 1e-10);
            total_weight += weight;
            
            // Compute k for this anchor
            double anchor_k = anchors[indices[i]].angle / (PI * PHI);
            anchor_k += ring_offsets[anchors[indices[i]].ring];
            anchor_k += anchors[indices[i]].position;
            
            correction += weight * (anchor_k - k);
        }
        
        if (total_weight > 0) {
            k += correction / total_weight;
        }
    }
    
    return k;
}

void analyze_correction_oscillation(RecoverySteps* all_steps, int num_steps) {
    printf("Analyzing oscillation patterns in correction steps...\n\n");
    
    // Compute step-to-step changes
    double step1_to_2[50], step2_to_3[50], step3_to_4[50];
    
    for (int i = 0; i < num_steps; i++) {
        step1_to_2[i] = all_steps[i].step2_ring - all_steps[i].step1_basic;
        step2_to_3[i] = all_steps[i].step3_position - all_steps[i].step2_ring;
        step3_to_4[i] = all_steps[i].step4_anchor - all_steps[i].step3_position;
    }
    
    // Analyze patterns
    printf("Step 1 → 2 (Ring Correction):\n");
    double avg_1_2 = 0, min_1_2 = 1e9, max_1_2 = -1e9;
    for (int i = 0; i < num_steps; i++) {
        avg_1_2 += step1_to_2[i];
        if (step1_to_2[i] < min_1_2) min_1_2 = step1_to_2[i];
        if (step1_to_2[i] > max_1_2) max_1_2 = step1_to_2[i];
    }
    avg_1_2 /= num_steps;
    printf("  Average: %.2f, Range: [%.2f, %.2f]\n", avg_1_2, min_1_2, max_1_2);
    
    printf("\nStep 2 → 3 (Position Correction):\n");
    double avg_2_3 = 0, min_2_3 = 1e9, max_2_3 = -1e9;
    for (int i = 0; i < num_steps; i++) {
        avg_2_3 += step2_to_3[i];
        if (step2_to_3[i] < min_2_3) min_2_3 = step2_to_3[i];
        if (step2_to_3[i] > max_2_3) max_2_3 = step2_to_3[i];
    }
    avg_2_3 /= num_steps;
    printf("  Average: %.2f, Range: [%.2f, %.2f]\n", avg_2_3, min_2_3, max_2_3);
    
    printf("\nStep 3 → 4 (Anchor Correction):\n");
    double avg_3_4 = 0, min_3_4 = 1e9, max_3_4 = -1e9;
    for (int i = 0; i < num_steps; i++) {
        avg_3_4 += step3_to_4[i];
        if (step3_to_4[i] < min_3_4) min_3_4 = step3_to_4[i];
        if (step3_to_4[i] > max_3_4) max_3_4 = step3_to_4[i];
    }
    avg_3_4 /= num_steps;
    printf("  Average: %.2f, Range: [%.2f, %.2f]\n", avg_3_4, min_3_4, max_3_4);
    
    // Check for oscillation patterns
    printf("\nOscillation Detection:\n");
    int oscillations_1_2 = 0, oscillations_2_3 = 0, oscillations_3_4 = 0;
    for (int i = 1; i < num_steps; i++) {
        if ((step1_to_2[i] > 0 && step1_to_2[i-1] < 0) || (step1_to_2[i] < 0 && step1_to_2[i-1] > 0)) {
            oscillations_1_2++;
        }
        if ((step2_to_3[i] > 0 && step2_to_3[i-1] < 0) || (step2_to_3[i] < 0 && step2_to_3[i-1] > 0)) {
            oscillations_2_3++;
        }
        if ((step3_to_4[i] > 0 && step3_to_4[i-1] < 0) || (step3_to_4[i] < 0 && step3_to_4[i-1] > 0)) {
            oscillations_3_4++;
        }
    }
    printf("  Step 1→2: %d sign changes (%.1f%%)\n", oscillations_1_2, 100.0 * oscillations_1_2 / (num_steps - 1));
    printf("  Step 2→3: %d sign changes (%.1f%%)\n", oscillations_2_3, 100.0 * oscillations_2_3 / (num_steps - 1));
    printf("  Step 3→4: %d sign changes (%.1f%%)\n", oscillations_3_4, 100.0 * oscillations_3_4 / (num_steps - 1));
}

void analyze_polytopic_structure(RecoverySteps* all_steps, int num_steps) {
    printf("Analyzing polytopic structure in correction process...\n\n");
    
    // Group k values by ring
    int ring_counts[5] = {0};
    double ring_errors[5] = {0};
    
    for (int i = 0; i < num_steps; i++) {
        int ring = all_steps[i].position.ring;
        if (ring < 5) {
            ring_counts[ring]++;
            ring_errors[ring] += all_steps[i].error_step4;
        }
    }
    
    printf("Distribution by Ring:\n");
    for (int r = 0; r < 5; r++) {
        if (ring_counts[r] > 0) {
            printf("  Ring %d: %d k values, avg error = %.2f\n", 
                   r, ring_counts[r], ring_errors[r] / ring_counts[r]);
        }
    }
    
    // Analyze position patterns
    printf("\nPosition Patterns:\n");
    int position_histogram[12] = {0};
    for (int i = 0; i < num_steps; i++) {
        int pos = all_steps[i].position.position % 12;
        position_histogram[pos]++;
    }
    
    printf("  Position distribution (mod 12):\n");
    for (int p = 0; p < 12; p++) {
        printf("    %2d: %d k values\n", p, position_histogram[p]);
    }
    
    // Check for 12-fold symmetry
    double avg_per_position = 0;
    for (int p = 0; p < 12; p++) {
        avg_per_position += position_histogram[p];
    }
    avg_per_position /= 12.0;
    
    double variance = 0;
    for (int p = 0; p < 12; p++) {
        double diff = position_histogram[p] - avg_per_position;
        variance += diff * diff;
    }
    variance /= 12.0;
    
    printf("\n  12-fold symmetry analysis:\n");
    printf("    Average per position: %.2f\n", avg_per_position);
    printf("    Variance: %.2f\n", variance);
    printf("    Symmetry quality: %.1f%% (lower variance = better)\n", 
           100.0 * (1.0 - variance / (avg_per_position * avg_per_position)));
}

void analyze_anchor_relationships(RecoverySteps* all_steps, int num_steps, PlatonicOverlay* overlay) {
    printf("Analyzing anchor relationships...\n\n");
    
    // Get Icosahedron anchors
    PlatonicSolid* icosa = &overlay->solids[SOLID_ICOSAHEDRON];
    
    printf("Icosahedron Properties:\n");
    printf("  Vertices: %d\n", icosa->num_vertices);
    printf("  12-fold symmetry: YES\n");
    printf("  Golden ratio: φ = %.6f\n", (1.0 + sqrt(5.0)) / 2.0);
    printf("\n");
    
    // Analyze which anchors are most frequently used
    int anchor_usage[12] = {0};
    
    for (int i = 0; i < num_steps; i++) {
        ClockPosition pos = all_steps[i].position;
        
        // Find 3 nearest anchors
        double distances[12];
        int indices[12];
        
        for (uint32_t j = 0; j < icosa->num_vertices; j++) {
            double dx = pos.angle - icosa->clock_positions[j].angle;
            double dy = pos.radius - icosa->clock_positions[j].radius;
            distances[j] = sqrt(dx*dx + dy*dy);
            indices[j] = j;
        }
        
        // Sort to find 3 nearest
        for (int a = 0; a < 11; a++) {
            for (int b = 0; b < 11 - a; b++) {
                if (distances[b] > distances[b+1]) {
                    double tmp_d = distances[b];
                    distances[b] = distances[b+1];
                    distances[b+1] = tmp_d;
                    
                    int tmp_i = indices[b];
                    indices[b] = indices[b+1];
                    indices[b+1] = tmp_i;
                }
            }
        }
        
        // Count usage of top 3
        for (int j = 0; j < 3; j++) {
            anchor_usage[indices[j]]++;
        }
    }
    
    printf("Anchor Usage Frequency:\n");
    for (int i = 0; i < 12; i++) {
        printf("  Anchor %2d: used %d times (%.1f%%)\n", 
               i, anchor_usage[i], 100.0 * anchor_usage[i] / (num_steps * 3));
    }
    
    // Check for uniform usage (12-fold symmetry)
    double avg_usage = 0;
    for (int i = 0; i < 12; i++) {
        avg_usage += anchor_usage[i];
    }
    avg_usage /= 12.0;
    
    double usage_variance = 0;
    for (int i = 0; i < 12; i++) {
        double diff = anchor_usage[i] - avg_usage;
        usage_variance += diff * diff;
    }
    usage_variance /= 12.0;
    
    printf("\nAnchor Usage Symmetry:\n");
    printf("  Average usage: %.2f\n", avg_usage);
    printf("  Variance: %.2f\n", usage_variance);
    printf("  Symmetry quality: %.1f%%\n", 
           100.0 * (1.0 - usage_variance / (avg_usage * avg_usage)));
}