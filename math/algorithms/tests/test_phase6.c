/**
 * OBJECTIVE 28 - Phase 6: Hyper-Dimensional Analysis
 * Comprehensive Test Suite
 * 
 * Tests hyper-dimensional structure analysis (4D+), multi-scalar analysis,
 * variance analysis, and cross-dimensional correlations.
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

/**
 * Helper: Create test data for hyper-dimensional structure
 */
static double* create_test_data(uint64_t num_elements) {
    double* data = (double*)calloc(num_elements, sizeof(double));
    if (!data) return NULL;
    
    // Initialize with simple pattern
    for (uint64_t i = 0; i < num_elements; i++) {
        data[i] = math_sin((double)i * 0.1) + math_cos((double)i * 0.05);
    }
    
    return data;
}

/**
 * Test 1: 4D structure analysis (tesseract)
 */
static void test_4d_tesseract(void) {
    TEST("4D structure analysis (tesseract)");
    
    // Tesseract: 4D hypercube
    uint32_t dimensions[] = {2, 2, 2, 2};  // 2×2×2×2 = 16 vertices
    uint64_t num_elements = 16;
    
    double* data = create_test_data(num_elements);
    ASSERT(data != NULL, "Test data created");
    
    HyperDimensionalStructure* structure = analyze_hyperdimensional(
        4, dimensions, data
    );
    ASSERT(structure != NULL, "4D structure created");
    
    if (structure) {
        ASSERT(structure->num_dimensions == 4, "4 dimensions");
        ASSERT(is_hyperdimensional_valid(structure), "Structure is valid");
        
        uint64_t total = get_total_elements(structure);
        printf("  Total elements: %lu\n", total);
        ASSERT(total == 16, "16 vertices in tesseract");
        
        // Check dimension sizes
        for (uint32_t i = 0; i < 4; i++) {
            uint32_t size = get_dimension_size(structure, i);
            printf("  Dimension %u: size=%u\n", i, size);
            ASSERT(size == 2, "Dimension size is 2");
        }
        
        free_hyperdimensional_structure(structure);
    }
    
    free(data);
}

/**
 * Test 2: 5D structure analysis
 */
static void test_5d_structure(void) {
    TEST("5D structure analysis");
    
    uint32_t dimensions[] = {3, 3, 3, 3, 3};  // 3^5 = 243 elements
    uint64_t num_elements = 243;
    
    double* data = create_test_data(num_elements);
    ASSERT(data != NULL, "Test data created");
    
    HyperDimensionalStructure* structure = analyze_hyperdimensional(
        5, dimensions, data
    );
    ASSERT(structure != NULL, "5D structure created");
    
    if (structure) {
        ASSERT(structure->num_dimensions == 5, "5 dimensions");
        ASSERT(is_hyperdimensional_valid(structure), "Structure is valid");
        
        uint64_t total = get_total_elements(structure);
        printf("  Total elements: %lu\n", total);
        ASSERT(total == 243, "243 elements in 5D structure");
        
        free_hyperdimensional_structure(structure);
    }
    
    free(data);
}

/**
 * Test 3: 6D structure analysis
 */
static void test_6d_structure(void) {
    TEST("6D structure analysis");
    
    uint32_t dimensions[] = {2, 2, 2, 2, 2, 2};  // 2^6 = 64 elements
    uint64_t num_elements = 64;
    
    double* data = create_test_data(num_elements);
    ASSERT(data != NULL, "Test data created");
    
    HyperDimensionalStructure* structure = analyze_hyperdimensional(
        6, dimensions, data
    );
    ASSERT(structure != NULL, "6D structure created");
    
    if (structure) {
        ASSERT(structure->num_dimensions == 6, "6 dimensions");
        ASSERT(is_hyperdimensional_valid(structure), "Structure is valid");
        
        uint64_t total = get_total_elements(structure);
        printf("  Total elements: %lu\n", total);
        ASSERT(total == 64, "64 elements in 6D structure");
        
        free_hyperdimensional_structure(structure);
    }
    
    free(data);
}

/**
 * Test 4: Dimension stability
 */
static void test_dimension_stability(void) {
    TEST("Dimension stability analysis");
    
    uint32_t dimensions[] = {4, 4, 4, 4};
    uint64_t num_elements = 256;
    
    double* data = create_test_data(num_elements);
    HyperDimensionalStructure* structure = analyze_hyperdimensional(
        4, dimensions, data
    );
    
    ASSERT(structure != NULL, "Structure created");
    
    if (structure) {
        bool all_stable = are_all_dimensions_stable(structure);
        printf("  All dimensions stable: %s\n", all_stable ? "yes" : "no");
        
        for (uint32_t i = 0; i < 4; i++) {
            bool stable = is_dimension_stable(structure, i);
            printf("  Dimension %u: %s\n", i, stable ? "stable" : "unstable");
            ASSERT(stable, "Dimension is stable");
        }
        
        ASSERT(all_stable, "All dimensions stable");
        
        free_hyperdimensional_structure(structure);
    }
    
    free(data);
}

/**
 * Test 5: GCD relationships between dimensions
 */
static void test_dimension_gcd(void) {
    TEST("GCD relationships between dimensions");
    
    uint32_t dimensions[] = {12, 18, 24, 30};  // All divisible by 6
    uint64_t num_elements = 12 * 18 * 24 * 30;
    
    double* data = create_test_data(num_elements);
    HyperDimensionalStructure* structure = analyze_hyperdimensional(
        4, dimensions, data
    );
    
    ASSERT(structure != NULL, "Structure created");
    
    if (structure) {
        // Check GCD(12, 18) = 6
        uint64_t gcd_12_18 = get_dimension_gcd(structure, 0, 1);
        printf("  GCD(12, 18) = %lu\n", gcd_12_18);
        ASSERT(gcd_12_18 == 6, "GCD(12,18) = 6");
        
        // Check GCD(12, 24) = 12
        uint64_t gcd_12_24 = get_dimension_gcd(structure, 0, 2);
        printf("  GCD(12, 24) = %lu\n", gcd_12_24);
        ASSERT(gcd_12_24 == 12, "GCD(12,24) = 12");
        
        // Check GCD(18, 24) = 6
        uint64_t gcd_18_24 = get_dimension_gcd(structure, 1, 2);
        printf("  GCD(18, 24) = %lu\n", gcd_18_24);
        ASSERT(gcd_18_24 == 6, "GCD(18,24) = 6");
        
        free_hyperdimensional_structure(structure);
    }
    
    free(data);
}

/**
 * Test 6: Multi-scalar analysis
 */
static void test_multi_scalar_analysis(void) {
    TEST("Multi-scalar analysis");
    
    uint32_t dimensions[] = {4, 4, 4, 4};
    uint64_t num_elements = 256;
    
    double* data = create_test_data(num_elements);
    HyperDimensionalStructure* structure = analyze_hyperdimensional(
        4, dimensions, data
    );
    
    ASSERT(structure != NULL, "Structure created");
    
    if (structure) {
        // Test at 5 different scales
        double scalars[] = {0.5, 0.75, 1.0, 1.25, 1.5};
        MultiScalarAnalysis* analysis = analyze_multi_scalar(structure, scalars, 5);
        
        ASSERT(analysis != NULL, "Multi-scalar analysis created");
        
        if (analysis) {
            ASSERT(analysis->num_scalars == 5, "5 scalars analyzed");
            
            // Check each scalar
            for (uint32_t i = 0; i < 5; i++) {
                const ScalarAnalysis* scalar = get_scalar_analysis(analysis, i);
                ASSERT(scalar != NULL, "Scalar analysis exists");
                
                if (scalar) {
                    printf("  Scalar %.2f: stable=%s\n",
                           scalar->scalar_value,
                           scalar->is_stable ? "yes" : "no");
                }
            }
            
            // Check if all stable
            bool all_stable = are_all_scalars_stable(analysis);
            printf("  All scalars stable: %s\n", all_stable ? "yes" : "no");
            
            free_multi_scalar_analysis(analysis);
        }
        
        free_hyperdimensional_structure(structure);
    }
    
    free(data);
}

/**
 * Test 7: Cross-scalar correlations
 */
static void test_cross_scalar_correlations(void) {
    TEST("Cross-scalar correlations");
    
    uint32_t dimensions[] = {3, 3, 3, 3};
    uint64_t num_elements = 81;
    
    double* data = create_test_data(num_elements);
    HyperDimensionalStructure* structure = analyze_hyperdimensional(
        4, dimensions, data
    );
    
    if (structure) {
        double scalars[] = {0.8, 0.9, 1.0, 1.1, 1.2};
        MultiScalarAnalysis* analysis = analyze_multi_scalar(structure, scalars, 5);
        
        if (analysis) {
            // Check correlations between adjacent scalars
            for (uint32_t i = 0; i < 4; i++) {
                double corr = get_cross_scalar_correlation(analysis, i, i + 1);
                printf("  Correlation(%.1f, %.1f) = %.3f\n",
                       scalars[i], scalars[i+1], corr);
                ASSERT(corr > 0.5, "Adjacent scalars are correlated");
            }
            
            // Check consistency
            double consistency = compute_multi_scalar_consistency(analysis);
            printf("  Multi-scalar consistency: %.3f\n", consistency);
            ASSERT(consistency > 0.5, "Good consistency across scalars");
            
            free_multi_scalar_analysis(analysis);
        }
        
        free_hyperdimensional_structure(structure);
    }
    
    free(data);
}

/**
 * Test 8: Variance analysis
 */
static void test_variance_analysis(void) {
    TEST("Variance analysis");
    
    uint32_t dimensions[] = {5, 5, 5, 5};
    uint64_t num_elements = 625;
    
    double* data = create_test_data(num_elements);
    HyperDimensionalStructure* structure = analyze_hyperdimensional(
        4, dimensions, data
    );
    
    if (structure) {
        double* variance_per_axis = (double*)calloc(4, sizeof(double));
        double* variance_per_vertex = (double*)calloc(num_elements, sizeof(double));
        
        compute_variance(structure, data, variance_per_axis, variance_per_vertex);
        
        printf("  Variance per axis:\n");
        for (uint32_t i = 0; i < 4; i++) {
            printf("    Axis %u: %.6f\n", i, variance_per_axis[i]);
            ASSERT(variance_per_axis[i] >= 0.0, "Non-negative variance");
        }
        
        // Check some vertex variances
        printf("  Sample vertex variances:\n");
        for (int i = 0; i < 5 && i < (int)num_elements; i++) {
            printf("    Vertex %d: %.6f\n", i, variance_per_vertex[i]);
            ASSERT(variance_per_vertex[i] >= 0.0, "Non-negative variance");
        }
        
        free(variance_per_axis);
        free(variance_per_vertex);
        free_hyperdimensional_structure(structure);
    }
    
    free(data);
}

/**
 * Test 9: Cross-dimensional correlations
 */
static void test_cross_dimensional_correlations(void) {
    TEST("Cross-dimensional correlations");
    
    uint32_t dimensions[] = {6, 12, 18, 24};  // All multiples of 6
    uint64_t num_elements = 6 * 12 * 18 * 24;
    
    double* data = create_test_data(num_elements);
    HyperDimensionalStructure* structure = analyze_hyperdimensional(
        4, dimensions, data
    );
    
    if (structure) {
        double* correlations = compute_cross_correlation_matrix(structure, data);
        ASSERT(correlations != NULL, "Correlation matrix computed");
        
        if (correlations) {
            // Check diagonal (self-correlations should be 1.0)
            for (uint32_t i = 0; i < 4; i++) {
                double self_corr = get_dimension_correlation(correlations, 4, i, i);
                printf("  Self-correlation dim %u: %.3f\n", i, self_corr);
                ASSERT(math_abs(self_corr - 1.0) < 0.01, "Self-correlation is 1.0");
            }
            
            // Check some cross-correlations
            double corr_01 = get_dimension_correlation(correlations, 4, 0, 1);
            printf("  Correlation(6, 12): %.3f\n", corr_01);
            ASSERT(corr_01 > 0.0, "Positive correlation");
            
            // Validate consistency
            bool consistent = validate_cross_dimensional_consistency(correlations, 4);
            printf("  Cross-dimensional consistency: %s\n", consistent ? "yes" : "no");
            ASSERT(consistent, "Correlations are consistent");
            
            // Compute average correlation
            double avg_corr = compute_average_correlation(correlations, 4);
            printf("  Average correlation: %.3f\n", avg_corr);
            ASSERT(avg_corr > 0.0, "Positive average correlation");
            
            free(correlations);
        }
        
        free_hyperdimensional_structure(structure);
    }
    
    free(data);
}

/**
 * Test 10: Varying dimensions (4D, 5D, 6D, 7D)
 */
static void test_varying_dimensions(void) {
    TEST("Varying dimensions (4D, 5D, 6D, 7D)");
    
    for (uint32_t num_dims = 4; num_dims <= 7; num_dims++) {
        uint32_t* dimensions = (uint32_t*)calloc(num_dims, sizeof(uint32_t));
        
        // Each dimension has size 2
        uint64_t num_elements = 1;
        for (uint32_t i = 0; i < num_dims; i++) {
            dimensions[i] = 2;
            num_elements *= 2;
        }
        
        double* data = create_test_data(num_elements);
        HyperDimensionalStructure* structure = analyze_hyperdimensional(
            num_dims, dimensions, data
        );
        
        char msg[256];
        snprintf(msg, sizeof(msg), "%uD structure created", num_dims);
        ASSERT(structure != NULL, msg);
        
        if (structure) {
            snprintf(msg, sizeof(msg), "%uD structure valid", num_dims);
            ASSERT(is_hyperdimensional_valid(structure), msg);
            
            uint64_t total = get_total_elements(structure);
            printf("  %uD: %lu elements\n", num_dims, total);
            
            free_hyperdimensional_structure(structure);
        }
        
        free(data);
        free(dimensions);
    }
}

/**
 * Test 11: Dimension stability with corruption
 */
static void test_stability_with_corruption(void) {
    TEST("Dimension stability with corruption");
    
    uint32_t dimensions[] = {4, 4, 4, 4};
    uint64_t num_elements = 256;
    
    double* data = create_test_data(num_elements);
    
    // Add corruption to some elements
    for (int i = 0; i < 25; i++) {  // 10% corruption
        data[i * 10] += 10.0;  // Large corruption
    }
    
    HyperDimensionalStructure* structure = analyze_hyperdimensional(
        4, dimensions, data
    );
    
    if (structure) {
        bool all_stable = are_all_dimensions_stable(structure);
        printf("  All dimensions stable: %s\n", all_stable ? "yes" : "no");
        
        // Individual dimension stability
        for (uint32_t i = 0; i < 4; i++) {
            bool stable = is_dimension_stable(structure, i);
            printf("  Dimension %u: %s\n", i, stable ? "stable" : "unstable");
        }
        
        free_hyperdimensional_structure(structure);
    }
    
    free(data);
}

/**
 * Test 12: Multi-scalar consistency
 */
static void test_multi_scalar_consistency(void) {
    TEST("Multi-scalar consistency");
    
    uint32_t dimensions[] = {3, 3, 3, 3};
    uint64_t num_elements = 81;
    
    double* data = create_test_data(num_elements);
    HyperDimensionalStructure* structure = analyze_hyperdimensional(
        4, dimensions, data
    );
    
    if (structure) {
        double close_consistency = 0.0;
        
        // Test with closely spaced scalars (should be consistent)
        double close_scalars[] = {0.9, 0.95, 1.0, 1.05, 1.1};
        MultiScalarAnalysis* close_analysis = analyze_multi_scalar(
            structure, close_scalars, 5
        );
        
        if (close_analysis) {
            close_consistency = compute_multi_scalar_consistency(close_analysis);
            printf("  Close scalars consistency: %.3f\n", close_consistency);
            ASSERT(close_consistency > 0.7, "High consistency for close scalars");
            
            free_multi_scalar_analysis(close_analysis);
        }
        
        // Test with widely spaced scalars (should be less consistent)
        double wide_scalars[] = {0.5, 1.0, 1.5, 2.0, 2.5};
        MultiScalarAnalysis* wide_analysis = analyze_multi_scalar(
            structure, wide_scalars, 5
        );
        
        if (wide_analysis) {
            double wide_consistency = compute_multi_scalar_consistency(wide_analysis);
            printf("  Wide scalars consistency: %.3f\n", wide_consistency);
            ASSERT(wide_consistency < close_consistency, "Lower consistency for wide scalars");
            
            free_multi_scalar_analysis(wide_analysis);
        }
        
        free_hyperdimensional_structure(structure);
    }
    
    free(data);
}

/**
 * Test 13: Most stable scalar detection
 */
static void test_most_stable_scalar(void) {
    TEST("Most stable scalar detection");
    
    uint32_t dimensions[] = {4, 4, 4, 4};
    uint64_t num_elements = 256;
    
    double* data = create_test_data(num_elements);
    HyperDimensionalStructure* structure = analyze_hyperdimensional(
        4, dimensions, data
    );
    
    if (structure) {
        double scalars[] = {0.5, 0.8, 1.0, 1.3, 2.0};
        MultiScalarAnalysis* analysis = analyze_multi_scalar(structure, scalars, 5);
        
        if (analysis) {
            uint32_t most_stable = find_most_stable_scalar(analysis);
            printf("  Most stable scalar index: %u\n", most_stable);
            printf("  Most stable scalar value: %.1f\n", scalars[most_stable]);
            
            // Should be scalar closest to 1.0 (index 2)
            ASSERT(most_stable == 2, "Scalar 1.0 is most stable");
            
            free_multi_scalar_analysis(analysis);
        }
        
        free_hyperdimensional_structure(structure);
    }
    
    free(data);
}

/**
 * Test 14: Correlation strength
 */
static void test_correlation_strength(void) {
    TEST("Correlation strength computation");
    
    uint32_t dimensions[] = {8, 12, 16, 20};
    uint64_t num_elements = 8 * 12 * 16 * 20;
    
    double* data = create_test_data(num_elements);
    HyperDimensionalStructure* structure = analyze_hyperdimensional(
        4, dimensions, data
    );
    
    if (structure) {
        double* correlations = compute_cross_correlation_matrix(structure, data);
        
        if (correlations) {
            double strength = compute_correlation_strength(correlations, 4);
            printf("  Correlation strength: %.3f\n", strength);
            ASSERT(strength > 0.0, "Positive correlation strength");
            ASSERT(strength <= 1.0, "Correlation strength in valid range");
            
            free(correlations);
        }
        
        free_hyperdimensional_structure(structure);
    }
    
    free(data);
}

/**
 * Test 15: Corruption detection by variance
 */
static void test_corruption_detection_variance(void) {
    TEST("Corruption detection by variance");
    
    uint32_t dimensions[] = {4, 4, 4, 4};
    uint64_t num_elements = 256;
    
    double* data = create_test_data(num_elements);
    
    // Add corruption
    for (int i = 0; i < 25; i++) {  // 10% corruption
        data[i * 10] += 5.0;
    }
    
    HyperDimensionalStructure* structure = analyze_hyperdimensional(
        4, dimensions, data
    );
    
    if (structure) {
        uint64_t num_corrupted = 0;
        bool detected = detect_corruption_by_variance(
            structure, data, 1.0, &num_corrupted
        );
        
        printf("  Corruption detected: %s\n", detected ? "yes" : "no");
        printf("  Number of corrupted elements: %lu\n", num_corrupted);
        
        ASSERT(detected, "Corruption detected");
        ASSERT(num_corrupted > 0, "Corrupted elements identified");
        
        free_hyperdimensional_structure(structure);
    }
    
    free(data);
}

/**
 * Main test runner
 */
int main(void) {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║  OBJECTIVE 28 - Phase 6: Hyper-Dimensional Analysis       ║\n");
    printf("║  Comprehensive Test Suite                                  ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    // Run all tests
    test_4d_tesseract();
    test_5d_structure();
    test_6d_structure();
    test_dimension_stability();
    test_dimension_gcd();
    test_multi_scalar_analysis();
    test_cross_scalar_correlations();
    test_variance_analysis();
    test_cross_dimensional_correlations();
    test_varying_dimensions();
    test_stability_with_corruption();
    test_multi_scalar_consistency();
    test_most_stable_scalar();
    test_correlation_strength();
    test_corruption_detection_variance();
    
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
