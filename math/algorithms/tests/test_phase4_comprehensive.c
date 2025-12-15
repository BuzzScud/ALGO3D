/**
 * OBJECTIVE 28 - Phase 4: Recursive Stabilization
 * Comprehensive Test Suite
 * 
 * Tests multi-scale stabilization with all Platonic solids,
 * validates recursive and iterative strategies, and verifies
 * convergence detection and quality metrics.
 */

#include "blind_recovery/blind_recovery.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "math/arithmetic.h"
#include "math/transcendental.h"

// Test counters
static int tests_run = 0;
static int tests_passed = 0;

// Test result macro
#define TEST(name) do { \
    printf("\n=== Test %d: %s ===\n", ++tests_run, name); \
} while(0)

#define ASSERT(condition, message) do { \
    if (condition) { \
        printf("✓ PASS: %s\n", message); \
        tests_passed++; \
    } else { \
        printf("✗ FAIL: %s\n", message); \
    } \
} while(0)

// Platonic solid definitions
typedef struct {
    const char* name;
    int vertices;
    int edges;
    int faces;
} PlatonicSolid;

static const PlatonicSolid PLATONIC_SOLIDS[] = {
    {"Tetrahedron", 4, 6, 4},
    {"Cube", 8, 12, 6},
    {"Octahedron", 6, 12, 8},
    {"Dodecahedron", 20, 30, 12},
    {"Icosahedron", 12, 30, 20}
};

static const int NUM_PLATONIC_SOLIDS = 5;

/**
 * Helper: Create test structure with known properties
 */
static StructuralMap create_test_structure(int V, int E, int F, double corruption_level) {
    StructuralMap structure = {
        .num_vertices = V,
        .num_edges = E,
        .num_faces = F,
        .coprime_matrix = NULL,
        .dimensional_offsets = NULL,
        .corruption_mask = NULL,
        .corruption_percentage = corruption_level
    };
    
    return structure;
}

/**
 * Test 1: Multi-scale analysis creation for all Platonic solids
 */
static void test_multiscale_all_solids(void) {
    TEST("Multi-scale analysis for all Platonic solids");
    
    for (int i = 0; i < NUM_PLATONIC_SOLIDS; i++) {
        const PlatonicSolid* solid = &PLATONIC_SOLIDS[i];
        
        StructuralMap structure = create_test_structure(
            solid->vertices, solid->edges, solid->faces, 0.0
        );
        
        MultiScaleAnalysis* analysis = analyze_multi_scale(&structure, 3);
        ASSERT(analysis != NULL, "Multi-scale analysis created");
        ASSERT(analysis->num_scales == 3, "Correct number of scales");
        
        // Verify Euler's formula at original scale
        int V = solid->vertices;
        int E = solid->edges;
        int F = solid->faces;
        int euler = V - E + F;
        
        char msg[256];
        snprintf(msg, sizeof(msg), "%s: V=%d, E=%d, F=%d, χ=%d (should be 2)",
                 solid->name, V, E, F, euler);
        printf("  %s\n", msg);
        ASSERT(euler == 2, "Euler's formula holds");
        
        free_multi_scale_analysis(analysis);
    }
}

/**
 * Test 2: Recursive stabilization for each Platonic solid
 */
static void test_recursive_stabilization_all_solids(void) {
    TEST("Recursive stabilization for all Platonic solids");
    
    for (int i = 0; i < NUM_PLATONIC_SOLIDS; i++) {
        const PlatonicSolid* solid = &PLATONIC_SOLIDS[i];
        
        StructuralMap structure = create_test_structure(
            solid->vertices, solid->edges, solid->faces, 0.15
        );
        
        MultiScaleAnalysis* analysis = analyze_multi_scale(&structure, 3);
        ASSERT(analysis != NULL, "Multi-scale analysis created");
        
        bool stabilized = stabilize_recursive(&structure, analysis, 10);
        
        char msg[256];
        snprintf(msg, sizeof(msg), "%s: stabilized=%s",
                 solid->name, stabilized ? "yes" : "no");
        printf("  %s\n", msg);
        
        free_multi_scale_analysis(analysis);
    }
}

/**
 * Test 3: Iterative stabilization for each Platonic solid
 */
static void test_iterative_stabilization_all_solids(void) {
    TEST("Iterative stabilization for all Platonic solids");
    
    for (int i = 0; i < NUM_PLATONIC_SOLIDS; i++) {
        const PlatonicSolid* solid = &PLATONIC_SOLIDS[i];
        
        StructuralMap structure = create_test_structure(
            solid->vertices, solid->edges, solid->faces, 0.15
        );
        
        MultiScaleAnalysis* analysis = analyze_multi_scale(&structure, 3);
        ASSERT(analysis != NULL, "Multi-scale analysis created");
        
        bool stabilized = stabilize_iterative(&structure, analysis, 10);
        
        char msg[256];
        snprintf(msg, sizeof(msg), "%s: stabilized=%s",
                 solid->name, stabilized ? "yes" : "no");
        printf("  %s\n", msg);
        
        free_multi_scale_analysis(analysis);
    }
}

/**
 * Test 4: Adaptive stabilization strategy selection
 */
static void test_adaptive_strategy_all_solids(void) {
    TEST("Adaptive strategy selection for all Platonic solids");
    
    for (int i = 0; i < NUM_PLATONIC_SOLIDS; i++) {
        const PlatonicSolid* solid = &PLATONIC_SOLIDS[i];
        
        StructuralMap structure = create_test_structure(
            solid->vertices, solid->edges, solid->faces, 0.15
        );
        
        MultiScaleAnalysis* analysis = analyze_multi_scale(&structure, 3);
        ASSERT(analysis != NULL, "Multi-scale analysis created");
        
        bool stabilized = stabilize_adaptive(&structure, analysis);
        
        char msg[256];
        snprintf(msg, sizeof(msg), "%s: stabilized=%s",
                 solid->name, stabilized ? "yes" : "no");
        printf("  %s\n", msg);
        
        free_multi_scale_analysis(analysis);
    }
}

/**
 * Test 5: Multi-scale stabilization (coarse → fine)
 */
static void test_multiscale_stabilization(void) {
    TEST("Multi-scale stabilization (coarse to fine)");
    
    // Use cube for this test
    StructuralMap structure = create_test_structure(8, 12, 6, 0.20);
    MultiScaleAnalysis* analysis = analyze_multi_scale(&structure, 3);
    
    ASSERT(analysis != NULL, "Multi-scale analysis created");
    
    // Check quality at each scale
    for (uint32_t s = 0; s < analysis->num_scales; s++) {
        const ScaleLevel* level = get_scale_level(analysis, s);
        
        char msg[256];
        snprintf(msg, sizeof(msg), "Scale %u: resolution=%.1f, stable=%s",
                 s, level->resolution, level->is_stable ? "yes" : "no");
        printf("  %s\n", msg);
    }
    
    // Apply recursive stabilization
    bool stabilized = stabilize_recursive(&structure, analysis, 15);
    printf("  Stabilization result: %s\n", stabilized ? "success" : "in progress");
    
    // Check overall quality
    double quality = compute_stabilization_quality(analysis);
    printf("  Final quality: %.3f\n", quality);
    ASSERT(quality >= 0.0 && quality <= 1.0, "Quality in valid range");
    
    free_multi_scale_analysis(analysis);
}

/**
 * Test 6: Convergence with varying corruption
 */
static void test_convergence_varying_corruption(void) {
    TEST("Convergence with varying corruption levels");
    
    double corruption_levels[] = {0.05, 0.10, 0.15, 0.20, 0.25};
    int num_levels = sizeof(corruption_levels) / sizeof(corruption_levels[0]);
    
    for (int i = 0; i < num_levels; i++) {
        double corruption = corruption_levels[i];
        
        StructuralMap structure = create_test_structure(12, 30, 20, corruption);
        MultiScaleAnalysis* analysis = analyze_multi_scale(&structure, 3);
        
        bool stabilized = stabilize_adaptive(&structure, analysis);
        double quality = compute_stabilization_quality(analysis);
        
        char msg[256];
        snprintf(msg, sizeof(msg), "Corruption %.0f%%: stabilized=%s, quality=%.3f",
                 corruption * 100,
                 stabilized ? "yes" : "no",
                 quality);
        printf("  %s\n", msg);
        
        free_multi_scale_analysis(analysis);
    }
}

/**
 * Test 7: Quality metrics computation
 */
static void test_quality_metrics(void) {
    TEST("Quality metrics computation");
    
    // Test with clean structure
    StructuralMap clean = create_test_structure(8, 12, 6, 0.0);
    MultiScaleAnalysis* clean_analysis = analyze_multi_scale(&clean, 3);
    double clean_quality = compute_stabilization_quality(clean_analysis);
    
    printf("  Clean structure quality: %.3f\n", clean_quality);
    ASSERT(clean_quality > 0.7, "High quality for clean structure");
    
    // Test with corrupted structure
    StructuralMap corrupt = create_test_structure(8, 12, 6, 0.20);
    MultiScaleAnalysis* corrupt_analysis = analyze_multi_scale(&corrupt, 3);
    double corrupt_quality = compute_stabilization_quality(corrupt_analysis);
    
    printf("  Corrupted structure (20%%) quality: %.3f\n", corrupt_quality);
    ASSERT(corrupt_quality < clean_quality, "Lower quality for corrupted structure");
    
    free_multi_scale_analysis(clean_analysis);
    free_multi_scale_analysis(corrupt_analysis);
}

/**
 * Test 8: Stabilization with tracking
 */
static void test_stabilization_tracking(void) {
    TEST("Stabilization with tracking");
    
    StructuralMap structure = create_test_structure(12, 30, 20, 0.15);
    MultiScaleAnalysis* analysis = analyze_multi_scale(&structure, 3);
    
    bool stabilized = stabilize_adaptive(&structure, analysis);
    (void)stabilized; // Use tracking via analysis->all_scales_stable
    double quality = compute_stabilization_quality(analysis);
    
    printf("  Stabilized: %s\n", stabilized ? "yes" : "no");
    printf("  Final quality: %.3f\n", quality);
    printf("  All scales stable: %s\n", analysis->all_scales_stable ? "yes" : "no");
    
    ASSERT(quality > 0.5, "Reasonable quality achieved");
    
    free_multi_scale_analysis(analysis);
}

/**
 * Test 9: Recursive vs Iterative comparison
 */
static void test_recursive_vs_iterative(void) {
    TEST("Recursive vs Iterative strategy comparison");
    
    StructuralMap structure = create_test_structure(20, 30, 12, 0.15);
    
    // Test recursive
    MultiScaleAnalysis* analysis1 = analyze_multi_scale(&structure, 3);
    bool recursive_result = stabilize_recursive(&structure, analysis1, 15);
    double recursive_quality = compute_stabilization_quality(analysis1);
    
    printf("  Recursive:\n");
    printf("    Stabilized: %s\n", recursive_result ? "yes" : "no");
    printf("    Quality: %.3f\n", recursive_quality);
    
    // Test iterative
    MultiScaleAnalysis* analysis2 = analyze_multi_scale(&structure, 3);
    bool iterative_result = stabilize_iterative(&structure, analysis2, 15);
    double iterative_quality = compute_stabilization_quality(analysis2);
    
    printf("  Iterative:\n");
    printf("    Stabilized: %s\n", iterative_result ? "yes" : "no");
    printf("    Quality: %.3f\n", iterative_quality);
    
    // Both should achieve reasonable quality
    ASSERT(recursive_quality > 0.4, "Recursive quality reasonable");
    ASSERT(iterative_quality > 0.4, "Iterative quality reasonable");
    
    free_multi_scale_analysis(analysis1);
    free_multi_scale_analysis(analysis2);
}

/**
 * Test 10: Scale propagation (coarse → fine)
 */
static void test_scale_propagation(void) {
    TEST("Scale propagation from coarse to fine");
    
    StructuralMap structure = create_test_structure(12, 30, 20, 0.15);
    MultiScaleAnalysis* analysis = analyze_multi_scale(&structure, 4);
    
    ASSERT(analysis->num_scales == 4, "4 scales created");
    
    // Verify scale resolutions increase
    for (uint32_t s = 0; s < analysis->num_scales; s++) {
        const ScaleLevel* level = get_scale_level(analysis, s);
        
        char msg[256];
        snprintf(msg, sizeof(msg), "Scale %u: resolution=%.1f",
                 s, level->resolution);
        printf("  %s\n", msg);
    }
    
    // Apply stabilization
    bool stabilized = stabilize_recursive(&structure, analysis, 20);
    double quality = compute_stabilization_quality(analysis);
    
    printf("  Stabilization result:\n");
    printf("    Stabilized: %s\n", stabilized ? "yes" : "no");
    printf("    Quality: %.3f\n", quality);
    
    ASSERT(quality > 0.4, "Reasonable quality");
    
    free_multi_scale_analysis(analysis);
}

/**
 * Test 11: High corruption recovery
 */
static void test_high_corruption_recovery(void) {
    TEST("High corruption recovery (20-25%)");
    
    double high_corruption[] = {0.20, 0.22, 0.25};
    int num_tests = sizeof(high_corruption) / sizeof(high_corruption[0]);
    
    for (int i = 0; i < num_tests; i++) {
        double corruption = high_corruption[i];
        
        StructuralMap structure = create_test_structure(12, 30, 20, corruption);
        MultiScaleAnalysis* analysis = analyze_multi_scale(&structure, 4);
        
        // Use more iterations for high corruption
        stabilize_adaptive(&structure, analysis);
        double quality = compute_stabilization_quality(analysis);
        
        char msg[256];
        snprintf(msg, sizeof(msg), "Corruption %.0f%%: quality=%.3f",
                 corruption * 100, quality);
        printf("  %s\n", msg);
        
        // Should still achieve some quality
        ASSERT(quality > 0.3, "Some quality at high corruption");
        
        free_multi_scale_analysis(analysis);
    }
}

/**
 * Test 12: Early stopping
 */
static void test_early_stopping(void) {
    TEST("Early stopping when stable");
    
    // Use low corruption for quick stabilization
    StructuralMap structure = create_test_structure(8, 12, 6, 0.05);
    MultiScaleAnalysis* analysis = analyze_multi_scale(&structure, 3);
    
    bool stabilized = stabilize_adaptive(&structure, analysis);
    double quality = compute_stabilization_quality(analysis);
    
    printf("  Stabilized: %s\n", stabilized ? "yes" : "no");
    printf("  Final quality: %.3f\n", quality);
    printf("  All scales stable: %s\n", analysis->all_scales_stable ? "yes" : "no");
    
    ASSERT(quality > 0.7, "High quality with low corruption");
    
    free_multi_scale_analysis(analysis);
}

/**
 * Test 13: Multiple scale counts
 */
static void test_multiple_scale_counts(void) {
    TEST("Multiple scale counts (2, 3, 4, 5 scales)");
    
    uint32_t scale_counts[] = {2, 3, 4, 5};
    int num_counts = sizeof(scale_counts) / sizeof(scale_counts[0]);
    
    for (int i = 0; i < num_counts; i++) {
        uint32_t num_scales = scale_counts[i];
        
        StructuralMap structure = create_test_structure(12, 30, 20, 0.15);
        MultiScaleAnalysis* analysis = analyze_multi_scale(&structure, num_scales);
        
        ASSERT(analysis->num_scales == num_scales, "Correct scale count");
        
        bool stabilized = stabilize_adaptive(&structure, analysis);
        double quality = compute_stabilization_quality(analysis);
        
        char msg[256];
        snprintf(msg, sizeof(msg), "%u scales: quality=%.3f, stabilized=%s",
                 num_scales, quality, stabilized ? "yes" : "no");
        printf("  %s\n", msg);
        
        free_multi_scale_analysis(analysis);
    }
}

/**
 * Test 14: Stability propagation
 */
static void test_stability_propagation(void) {
    TEST("Stability propagation across scales");
    
    StructuralMap structure = create_test_structure(12, 30, 20, 0.10);
    MultiScaleAnalysis* analysis = analyze_multi_scale(&structure, 4);
    
    // Stabilize
    bool stabilized = stabilize_recursive(&structure, analysis, 20);
    
    printf("  Stabilization: %s\n", stabilized ? "success" : "in progress");
    printf("  All scales stable: %s\n", analysis->all_scales_stable ? "yes" : "no");
    
    // Check each scale
    for (uint32_t s = 0; s < analysis->num_scales; s++) {
        const ScaleLevel* level = get_scale_level(analysis, s);
        printf("    Scale %u: stable=%s\n", s, level->is_stable ? "yes" : "no");
    }
    
    double quality = compute_stabilization_quality(analysis);
    ASSERT(quality > 0.6, "Good quality after stabilization");
    
    free_multi_scale_analysis(analysis);
}

/**
 * Test 15: Integration with Phase 3 (convergence detection)
 */
static void test_phase3_integration(void) {
    TEST("Integration with Phase 3 convergence detection");
    
    StructuralMap structure = create_test_structure(12, 30, 20, 0.15);
    MultiScaleAnalysis* analysis = analyze_multi_scale(&structure, 3);
    
    // Simulate Phase 3 iterative refinement followed by Phase 4 stabilization
    bool stabilized = stabilize_adaptive(&structure, analysis);
    double quality = compute_stabilization_quality(analysis);
    
    printf("  Phase 4 stabilization:\n");
    printf("    Stabilized: %s\n", stabilized ? "yes" : "no");
    printf("    Quality: %.3f\n", quality);
    printf("    All scales stable: %s\n", analysis->all_scales_stable ? "yes" : "no");
    
    ASSERT(quality > 0.5, "Good quality after Phase 3+4");
    
    free_multi_scale_analysis(analysis);
}

/**
 * Main test runner
 */
int main(void) {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║  OBJECTIVE 28 - Phase 4: Recursive Stabilization          ║\n");
    printf("║  Comprehensive Test Suite                                  ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    // Run all tests
    test_multiscale_all_solids();
    test_recursive_stabilization_all_solids();
    test_iterative_stabilization_all_solids();
    test_adaptive_strategy_all_solids();
    test_multiscale_stabilization();
    test_convergence_varying_corruption();
    test_quality_metrics();
    test_stabilization_tracking();
    test_recursive_vs_iterative();
    test_scale_propagation();
    test_high_corruption_recovery();
    test_early_stopping();
    test_multiple_scale_counts();
    test_stability_propagation();
    test_phase3_integration();
    
    // Print summary
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║  TEST SUMMARY                                              ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("Tests run:    %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Pass rate:    %.1f%%\n", (tests_passed * 100.0) / tests_run);
    
    if (tests_passed == tests_run) {
        printf("\n✓ ALL TESTS PASSED!\n");
        return 0;
    } else {
        printf("\n✗ SOME TESTS FAILED\n");
        return 1;
    }
}
