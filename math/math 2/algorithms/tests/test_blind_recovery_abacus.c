/**
 * @file test_blind_recovery_abacus.c
 * @brief Test suite for Pure Abacus Blind Recovery
 */

#include "blind_recovery/blind_recovery_abacus.h"
#include "math/abacus.h"
#include "math/constants.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

// Test colors
#define COLOR_GREEN "\033[0;32m"
#define COLOR_RED "\033[0;31m"
#define COLOR_YELLOW "\033[0;33m"
#define COLOR_RESET "\033[0m"

// Test counters
static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST_START(name) \
    printf("\n" COLOR_YELLOW "TEST: %s" COLOR_RESET "\n", name); \
    tests_run++;

#define TEST_PASS() \
    printf(COLOR_GREEN "  ✓ PASS" COLOR_RESET "\n"); \
    tests_passed++;

#define TEST_FAIL(msg) \
    printf(COLOR_RED "  ✗ FAIL: %s" COLOR_RESET "\n", msg); \
    tests_failed++;

// ============================================================================
// TEST 1: Basic Structure Creation
// ============================================================================

void test_oscillation_map_creation() {
    TEST_START("Oscillation Map Creation");
    
    uint32_t base = 12;
    uint32_t num_dimensions = 3;
    uint32_t num_samples = 16;
    uint32_t precision = 20;
    
    // Create sample data
    CrystallineAbacus** data = (CrystallineAbacus**)malloc(num_dimensions * num_samples * sizeof(CrystallineAbacus*));
    for (uint32_t i = 0; i < num_dimensions * num_samples; i++) {
        data[i] = abacus_from_double(sin(i * 0.1), base, precision);
    }
    
    CrystallineAbacus* sampling_rate = abacus_from_double(1.0, base, precision);
    
    // Create oscillation map
    OscillationMapAbacus* map = detect_oscillations_abacus(
        (const CrystallineAbacus**)data,
        num_dimensions,
        num_samples,
        sampling_rate,
        base,
        precision
    );
    
    if (map && map->num_dimensions == num_dimensions) {
        TEST_PASS();
    } else {
        TEST_FAIL("Failed to create oscillation map");
    }
    
    // Cleanup
    free_oscillation_map_abacus(map);
    abacus_free(sampling_rate);
    for (uint32_t i = 0; i < num_dimensions * num_samples; i++) {
        abacus_free(data[i]);
    }
    free(data);
}

// ============================================================================
// TEST 2: Structural Mapping
// ============================================================================

void test_structural_mapping() {
    TEST_START("Structural Mapping");
    
    uint32_t base = 12;
    uint32_t num_vertices = 8;  // Cube
    uint32_t num_edges = 12;
    uint32_t num_faces = 6;
    
    // Create vertex positions (cube vertices)
    CrystallineAbacus*** positions = (CrystallineAbacus***)malloc(num_vertices * sizeof(CrystallineAbacus**));
    for (uint32_t i = 0; i < num_vertices; i++) {
        positions[i] = (CrystallineAbacus**)malloc(3 * sizeof(CrystallineAbacus*));
        positions[i][0] = abacus_from_double((i & 1) ? 1.0 : 0.0, base, 20);
        positions[i][1] = abacus_from_double((i & 2) ? 1.0 : 0.0, base, 20);
        positions[i][2] = abacus_from_double((i & 4) ? 1.0 : 0.0, base, 20);
    }
    
    // Create structural map
    StructuralMapAbacus* map = map_structure_abacus(
        num_vertices,
        num_edges,
        num_faces,
        (const CrystallineAbacus***)positions,
        base
    );
    
    if (map && map->num_vertices == num_vertices) {
        TEST_PASS();
    } else {
        TEST_FAIL("Failed to create structural map");
    }
    
    // Cleanup
    free_structural_map_abacus(map);
    for (uint32_t i = 0; i < num_vertices; i++) {
        for (uint32_t j = 0; j < 3; j++) {
            abacus_free(positions[i][j]);
        }
        free(positions[i]);
    }
    free(positions);
}

// ============================================================================
// TEST 3: Anchor System
// ============================================================================

void test_anchor_system() {
    TEST_START("Anchor System");
    
    uint32_t base = 12;
    uint32_t num_vertices = 8;
    uint32_t num_anchors = 4;
    
    // Create structural map
    CrystallineAbacus*** positions = (CrystallineAbacus***)malloc(num_vertices * sizeof(CrystallineAbacus**));
    for (uint32_t i = 0; i < num_vertices; i++) {
        positions[i] = (CrystallineAbacus**)malloc(3 * sizeof(CrystallineAbacus*));
        positions[i][0] = abacus_from_double((i & 1) ? 1.0 : 0.0, base, 20);
        positions[i][1] = abacus_from_double((i & 2) ? 1.0 : 0.0, base, 20);
        positions[i][2] = abacus_from_double((i & 4) ? 1.0 : 0.0, base, 20);
    }
    
    StructuralMapAbacus* structure = map_structure_abacus(num_vertices, 12, 6, (const CrystallineAbacus***)positions, base);
    
    // Create confidence scores
    CrystallineAbacus** confidence = (CrystallineAbacus**)malloc(num_vertices * sizeof(CrystallineAbacus*));
    for (uint32_t i = 0; i < num_vertices; i++) {
        confidence[i] = abacus_from_double(1.0, base, 20);
    }
    
    // Select anchors
    AnchorSystemAbacus* anchors = select_anchors_abacus(
        structure,
        (const CrystallineAbacus***)positions,
        (const CrystallineAbacus**)confidence,
        num_vertices,
        num_anchors,
        base
    );
    
    if (anchors && anchors->num_anchors == num_anchors) {
        TEST_PASS();
    } else {
        TEST_FAIL("Failed to create anchor system");
    }
    
    // Cleanup
    free_anchor_system_abacus(anchors);
    for (uint32_t i = 0; i < num_vertices; i++) {
        abacus_free(confidence[i]);
        for (uint32_t j = 0; j < 3; j++) {
            abacus_free(positions[i][j]);
        }
        free(positions[i]);
    }
    free(confidence);
    free(positions);
    free_structural_map_abacus(structure);
}

// ============================================================================
// TEST 4: Triangulation
// ============================================================================

void test_triangulation() {
    TEST_START("Triangulation");
    
    uint32_t base = 12;
    uint32_t precision = 20;
    
    // Create three anchor points
    AnchorPointAbacus anchor1, anchor2, anchor3;
    
    anchor1.base = base;
    anchor1.position = (CrystallineAbacus**)malloc(3 * sizeof(CrystallineAbacus*));
    anchor1.position[0] = abacus_from_double(0.0, base, precision);
    anchor1.position[1] = abacus_from_double(0.0, base, precision);
    anchor1.position[2] = abacus_from_double(0.0, base, precision);
    anchor1.confidence = abacus_from_double(1.0, base, precision);
    
    anchor2.base = base;
    anchor2.position = (CrystallineAbacus**)malloc(3 * sizeof(CrystallineAbacus*));
    anchor2.position[0] = abacus_from_double(1.0, base, precision);
    anchor2.position[1] = abacus_from_double(0.0, base, precision);
    anchor2.position[2] = abacus_from_double(0.0, base, precision);
    anchor2.confidence = abacus_from_double(1.0, base, precision);
    
    anchor3.base = base;
    anchor3.position = (CrystallineAbacus**)malloc(3 * sizeof(CrystallineAbacus*));
    anchor3.position[0] = abacus_from_double(0.0, base, precision);
    anchor3.position[1] = abacus_from_double(1.0, base, precision);
    anchor3.position[2] = abacus_from_double(0.0, base, precision);
    anchor3.confidence = abacus_from_double(1.0, base, precision);
    
    // Triangulate
    CrystallineAbacus** result = triangulate_position_abacus(
        &anchor1, &anchor2, &anchor3, base, precision
    );
    
    if (result) {
        // Expected result: average of three points = (1/3, 1/3, 0)
        double x, y, z;
        abacus_to_double(result[0], &x);
        abacus_to_double(result[1], &y);
        abacus_to_double(result[2], &z);
        
        printf("  Triangulated position: (%.6f, %.6f, %.6f)\n", x, y, z);
        printf("  Expected: (%.6f, %.6f, %.6f)\n", 1.0/3.0, 1.0/3.0, 0.0);
        
        double expected = 1.0 / 3.0;
        double tolerance = 0.01;
        
        if (fabs(x - expected) < tolerance && 
            fabs(y - expected) < tolerance && 
            fabs(z - 0.0) < tolerance) {
            TEST_PASS();
        } else {
            TEST_FAIL("Triangulation result incorrect");
        }
        
        for (int i = 0; i < 3; i++) {
            abacus_free(result[i]);
        }
        free(result);
    } else {
        TEST_FAIL("Triangulation failed");
    }
    
    // Cleanup
    for (int i = 0; i < 3; i++) {
        abacus_free(anchor1.position[i]);
        abacus_free(anchor2.position[i]);
        abacus_free(anchor3.position[i]);
    }
    free(anchor1.position);
    free(anchor2.position);
    free(anchor3.position);
    abacus_free(anchor1.confidence);
    abacus_free(anchor2.confidence);
    abacus_free(anchor3.confidence);
}

// ============================================================================
// TEST 5: Complete Recovery Pipeline
// ============================================================================

void test_complete_recovery() {
    TEST_START("Complete Recovery Pipeline");
    
    uint32_t base = 12;
    uint32_t num_vertices = 4;  // Tetrahedron
    uint32_t num_dimensions = 3;
    uint32_t precision = 20;
    
    // Create original data (tetrahedron vertices)
    CrystallineAbacus*** original = (CrystallineAbacus***)malloc(num_vertices * sizeof(CrystallineAbacus**));
    original[0] = (CrystallineAbacus**)malloc(3 * sizeof(CrystallineAbacus*));
    original[0][0] = abacus_from_double(0.0, base, precision);
    original[0][1] = abacus_from_double(0.0, base, precision);
    original[0][2] = abacus_from_double(0.0, base, precision);
    
    original[1] = (CrystallineAbacus**)malloc(3 * sizeof(CrystallineAbacus*));
    original[1][0] = abacus_from_double(1.0, base, precision);
    original[1][1] = abacus_from_double(0.0, base, precision);
    original[1][2] = abacus_from_double(0.0, base, precision);
    
    original[2] = (CrystallineAbacus**)malloc(3 * sizeof(CrystallineAbacus*));
    original[2][0] = abacus_from_double(0.5, base, precision);
    original[2][1] = abacus_from_double(0.866, base, precision);
    original[2][2] = abacus_from_double(0.0, base, precision);
    
    original[3] = (CrystallineAbacus**)malloc(3 * sizeof(CrystallineAbacus*));
    original[3][0] = abacus_from_double(0.5, base, precision);
    original[3][1] = abacus_from_double(0.289, base, precision);
    original[3][2] = abacus_from_double(0.816, base, precision);
    
    // Perform recovery (currently just copies data)
    CrystallineAbacus* corruption_pct = abacus_from_double(0.0, base, precision);
    CrystallineAbacus*** recovered = blind_recovery_abacus(
        (const CrystallineAbacus***)original,
        num_vertices,
        num_dimensions,
        corruption_pct,
        base,
        precision
    );
    
    if (recovered) {
        // Compute metrics
        RecoveryMetricsAbacus* metrics = compute_recovery_metrics_abacus(
            (const CrystallineAbacus***)original,
            (const CrystallineAbacus***)recovered,
            num_vertices,
            num_dimensions,
            base
        );
        
        if (metrics) {
            double recovery_rate;
            abacus_to_double(metrics->recovery_rate, &recovery_rate);
            
            printf("  Recovery rate: %.2f%%\n", recovery_rate * 100.0);
            printf("  Recovered vertices: %u/%u\n", metrics->recovered_vertices, metrics->total_vertices);
            
            if (recovery_rate > 0.9) {
                TEST_PASS();
            } else {
                TEST_FAIL("Recovery rate too low");
            }
            
            free_recovery_metrics_abacus(metrics);
        } else {
            TEST_FAIL("Failed to compute metrics");
        }
        
        // Cleanup recovered
        for (uint32_t i = 0; i < num_vertices; i++) {
            for (uint32_t j = 0; j < num_dimensions; j++) {
                abacus_free(recovered[i][j]);
            }
            free(recovered[i]);
        }
        free(recovered);
    } else {
        TEST_FAIL("Recovery failed");
    }
    
    // Cleanup
    abacus_free(corruption_pct);
    for (uint32_t i = 0; i < num_vertices; i++) {
        for (uint32_t j = 0; j < num_dimensions; j++) {
            abacus_free(original[i][j]);
        }
        free(original[i]);
    }
    free(original);
}

// ============================================================================
// TEST 6: Euler Formula Validation
// ============================================================================

void test_euler_validation() {
    TEST_START("Euler Formula Validation");
    
    uint32_t base = 12;
    
    // Test with cube: V=8, E=12, F=6 -> V-E+F=2
    CrystallineAbacus* v = abacus_from_uint64(8, base);
    CrystallineAbacus* e = abacus_from_uint64(12, base);
    CrystallineAbacus* f = abacus_from_uint64(6, base);
    
    bool valid = validate_euler_formula_abacus(v, e, f, base);
    
    if (valid) {
        TEST_PASS();
    } else {
        TEST_FAIL("Euler formula validation failed for cube");
    }
    
    abacus_free(v);
    abacus_free(e);
    abacus_free(f);
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main() {
    printf("\n");
    printf("========================================\n");
    printf("  Pure Abacus Blind Recovery Tests\n");
    printf("========================================\n");
    
    // Run all tests
    test_oscillation_map_creation();
    test_structural_mapping();
    test_anchor_system();
    test_triangulation();
    test_complete_recovery();
    test_euler_validation();
    
    // Print summary
    printf("\n");
    printf("========================================\n");
    printf("  Test Summary\n");
    printf("========================================\n");
    printf("  Total:  %d\n", tests_run);
    printf("  " COLOR_GREEN "Passed: %d" COLOR_RESET "\n", tests_passed);
    printf("  " COLOR_RED "Failed: %d" COLOR_RESET "\n", tests_failed);
    printf("========================================\n");
    printf("\n");
    
    return (tests_failed == 0) ? 0 : 1;
}