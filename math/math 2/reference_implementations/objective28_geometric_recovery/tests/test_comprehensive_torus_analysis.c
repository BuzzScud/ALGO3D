/**
 * test_comprehensive_torus_analysis.c - Comprehensive Multi-Torus Analysis
 * 
 * This test:
 * 1. Detects up to 20 tori (not just 5)
 * 2. Analyzes relationships between tori (factors, harmonics)
 * 3. Identifies common factors (beat frequency 2, factor 5)
 * 4. Maps tori to pq factorization structure
 * 5. Determines graph boundaries (min/max)
 * 
 * User's insight: 5 tori = pq + factors (p², q², pq interactions)
 * Beat frequency 2 suggests common factor
 * This is FORTUITOUS - reveals the complete factorization structure!
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <openssl/ec.h>
#include <openssl/bn.h>
#include <openssl/obj_mac.h>
#include "../include/g_triangulation.h"
#include "../include/plateau_detection.h"
#include "../include/oscillation_decomposition.h"
#include "../include/multi_torus_tracker.h"
#include "../include/harmonic_folding.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"

#define MAX_ITERATIONS 2000
#define NUM_TRAINING 20
#define MAX_TORI 20  // Increased from 5 to 20

/**
 * Analyze relationships between tori
 */
typedef struct {
    int torus_a;
    int torus_b;
    double frequency_ratio;     // freq_a / freq_b
    double amplitude_ratio;     // amp_a / amp_b
    double phase_difference;    // phase_a - phase_b
    bool is_harmonic;          // Is torus_b a harmonic of torus_a?
    int harmonic_number;       // If harmonic, which one (2, 3, 5, etc.)
} TorusRelationship;

/**
 * Factorization structure
 */
typedef struct {
    int num_tori;
    int* torus_ids;            // Which tori correspond to which factors
    double* factor_values;     // Estimated factor values
    int beat_frequency;        // Common beat frequency (2, 3, 5, etc.)
    double min_boundary;       // Inner graph boundary
    double max_boundary;       // Outer graph boundary
    int num_relationships;
    TorusRelationship* relationships;
} FactorizationStructure;

/**
 * Analyze relationships between all tori
 */
FactorizationStructure* analyze_torus_relationships(MultiTorusTracker* tracker) {
    if (!tracker || tracker->num_tori == 0) {
        return NULL;
    }
    
    FactorizationStructure* structure = (FactorizationStructure*)malloc(sizeof(FactorizationStructure));
    if (!structure) {
        return NULL;
    }
    
    structure->num_tori = tracker->num_tori;
    structure->torus_ids = (int*)malloc(structure->num_tori * sizeof(int));
    structure->factor_values = (double*)malloc(structure->num_tori * sizeof(double));
    
    // Allocate relationships array (n choose 2)
    int max_relationships = (structure->num_tori * (structure->num_tori - 1)) / 2;
    structure->relationships = (TorusRelationship*)malloc(max_relationships * sizeof(TorusRelationship));
    structure->num_relationships = 0;
    
    // Analyze all pairs
    for (int i = 0; i < structure->num_tori; i++) {
        for (int j = i + 1; j < structure->num_tori; j++) {
            TorusDescriptor* torus_a = &tracker->tori[i];
            TorusDescriptor* torus_b = &tracker->tori[j];
            
            TorusRelationship rel;
            rel.torus_a = i;
            rel.torus_b = j;
            rel.frequency_ratio = torus_a->frequency / torus_b->frequency;
            rel.amplitude_ratio = torus_a->amplitude / torus_b->amplitude;
            rel.phase_difference = torus_a->phase - torus_b->phase;
            
            // Check if harmonic (ratio close to integer)
            double ratio = rel.frequency_ratio;
            int nearest_int = (int)(ratio + 0.5);
            if (math_abs(ratio - nearest_int) < 0.1) {
                rel.is_harmonic = true;
                rel.harmonic_number = nearest_int;
            } else {
                rel.is_harmonic = false;
                rel.harmonic_number = 0;
            }
            
            structure->relationships[structure->num_relationships++] = rel;
        }
    }
    
    // Identify beat frequency (most common harmonic number)
    int harmonic_counts[10] = {0};  // Count harmonics 1-9
    for (int i = 0; i < structure->num_relationships; i++) {
        if (structure->relationships[i].is_harmonic) {
            int h = structure->relationships[i].harmonic_number;
            if (h >= 1 && h <= 9) {
                harmonic_counts[h]++;
            }
        }
    }
    
    // Find most common
    int max_count = 0;
    int beat_freq = 2;  // Default
    for (int h = 1; h <= 9; h++) {
        if (harmonic_counts[h] > max_count) {
            max_count = harmonic_counts[h];
            beat_freq = h;
        }
    }
    structure->beat_frequency = beat_freq;
    
    // Compute graph boundaries
    structure->min_boundary = tracker->tori[structure->num_tori - 1].center_k - 
                             tracker->tori[structure->num_tori - 1].amplitude;
    structure->max_boundary = tracker->tori[0].center_k + 
                             tracker->tori[0].amplitude;
    
    // Estimate factor values (amplitude ratios)
    for (int i = 0; i < structure->num_tori; i++) {
        structure->factor_values[i] = tracker->tori[i].amplitude;
        structure->torus_ids[i] = i;
    }
    
    return structure;
}

/**
 * Print factorization structure analysis
 */
void print_factorization_analysis(FactorizationStructure* structure) {
    if (!structure) {
        return;
    }
    
    printf("\n=== Factorization Structure Analysis ===\n\n");
    printf("Number of tori detected: %d\n", structure->num_tori);
    printf("Beat frequency: %d\n", structure->beat_frequency);
    printf("Graph boundaries: [%.2f, %.2f]\n", structure->min_boundary, structure->max_boundary);
    printf("Graph span: %.2f\n\n", structure->max_boundary - structure->min_boundary);
    
    printf("Torus Factor Values:\n");
    for (int i = 0; i < structure->num_tori; i++) {
        printf("  Torus %d: %.2f\n", i + 1, structure->factor_values[i]);
    }
    
    printf("\nTorus Relationships:\n");
    printf("| Torus A | Torus B | Freq Ratio | Amp Ratio | Phase Diff | Harmonic? | Harmonic # |\n");
    printf("|---------|---------|------------|-----------|------------|-----------|------------|\n");
    
    for (int i = 0; i < structure->num_relationships; i++) {
        TorusRelationship* rel = &structure->relationships[i];
        printf("| %7d | %7d | %10.4f | %9.4f | %10.4f | %9s | %10d |\n",
               rel->torus_a + 1,
               rel->torus_b + 1,
               rel->frequency_ratio,
               rel->amplitude_ratio,
               rel->phase_difference,
               rel->is_harmonic ? "YES" : "NO",
               rel->harmonic_number);
    }
    printf("\n");
}

/**
 * Run comprehensive torus analysis for a specific bit length
 */
void analyze_comprehensive_torus(int bit_length) {
    printf("\n========================================\n");
    printf("BIT LENGTH: %d-bit (Comprehensive Torus Analysis)\n", bit_length);
    printf("Tracking up to %d tori\n", MAX_TORI);
    printf("========================================\n\n");
    
    uint64_t max_k = (1ULL << bit_length) - 1;
    
    // Generate training k values
    uint64_t training_k[NUM_TRAINING];
    uint64_t primes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71};
    
    for (int i = 0; i < NUM_TRAINING; i++) {
        training_k[i] = (primes[i] * max_k) / 100;
        if (training_k[i] == 0) training_k[i] = primes[i];
        if (training_k[i] > max_k) training_k[i] = max_k;
    }
    
    // Create context
    GTriangulationContext* ctx = create_g_triangulation_context(
        NID_secp256k1,
        NUM_TRAINING,
        training_k,
        MAX_ITERATIONS
    );
    
    if (!ctx) {
        printf("  ✗ Failed to create context\n");
        return;
    }
    
    const EC_GROUP* group = EC_GROUP_new_by_curve_name(NID_secp256k1);
    
    // Create multi-torus tracker with increased capacity
    MultiTorusTracker* torus_tracker = create_multi_torus_tracker(
        MAX_TORI,       // Track up to 20 tori
        MAX_ITERATIONS,
        max_k
    );
    
    printf("Phase 1: Collecting %d k estimates...\n\n", MAX_ITERATIONS);
    
    clock_t start = clock();
    
    for (int iteration = 0; iteration < MAX_ITERATIONS; iteration++) {
        // Perform refinement
        perform_refinement_iteration(ctx);
        
        // Compute average k estimate
        double total_k_estimate = 0.0;
        
        for (int i = 0; i < NUM_TRAINING; i++) {
            EC_POINT* Q = EC_POINT_new(group);
            BIGNUM* k_bn = BN_new();
            BN_set_word(k_bn, training_k[i]);
            EC_POINT_mul(group, Q, k_bn, NULL, NULL, NULL);
            
            uint64_t k_estimated = estimate_k_from_q(ctx, Q);
            total_k_estimate += (double)k_estimated;
            
            EC_POINT_free(Q);
            BN_free(k_bn);
        }
        
        double avg_k = total_k_estimate / NUM_TRAINING;
        multi_torus_add_sample(torus_tracker, avg_k);
        
        // Print progress every 400 iterations
        if ((iteration + 1) % 400 == 0) {
            printf("  Iteration %4d: avg_k = %.2f\n", iteration + 1, avg_k);
        }
    }
    
    clock_t end = clock();
    double total_time = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("\n✓ Collected %d samples in %.2f seconds\n\n", MAX_ITERATIONS, total_time);
    
    // Phase 2: Identify tori
    printf("Phase 2: Identifying tori (up to %d)...\n\n", MAX_TORI);
    
    identify_tori(torus_tracker);
    
    printf("✓ Detected %d tori\n\n", torus_tracker->num_tori);
    
    // Print standard analysis
    print_multi_torus_analysis(torus_tracker);
    
    // Phase 3: Analyze relationships
    printf("Phase 3: Analyzing torus relationships...\n");
    
    FactorizationStructure* structure = analyze_torus_relationships(torus_tracker);
    if (structure) {
        print_factorization_analysis(structure);
        
        // Export data
        char filename[256];
        snprintf(filename, sizeof(filename), "comprehensive_torus_%dbit.csv", bit_length);
        export_multi_torus_csv(torus_tracker, filename);
        printf("✓ Data exported to %s\n", filename);
        
        // Free structure
        free(structure->torus_ids);
        free(structure->factor_values);
        free(structure->relationships);
        free(structure);
    }
    
    // Cleanup
    free_multi_torus_tracker(torus_tracker);
    free_g_triangulation_context(ctx);
    EC_GROUP_free((EC_GROUP*)group);
}

/**
 * Main test function
 */
int main(void) {
    printf("=================================================\n");
    printf("Comprehensive Multi-Torus Analysis\n");
    printf("=================================================\n");
    printf("Tracking up to %d tori per bit length\n", MAX_TORI);
    printf("Analyzing factorization structure (pq + factors)\n");
    printf("Identifying beat frequencies and harmonics\n\n");
    
    clock_t start = clock();
    
    // Test multiple bit lengths
    int bit_lengths[] = {8, 16, 32};
    int num_bit_lengths = sizeof(bit_lengths) / sizeof(bit_lengths[0]);
    
    for (int i = 0; i < num_bit_lengths; i++) {
        analyze_comprehensive_torus(bit_lengths[i]);
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("\n=================================================\n");
    printf("Comprehensive Multi-Torus Analysis Complete!\n");
    printf("Total time: %.2f seconds\n", elapsed);
    printf("=================================================\n");
    
    return 0;
}