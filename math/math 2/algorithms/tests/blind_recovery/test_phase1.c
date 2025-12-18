/**
 * @file test_phase1.c
 * @brief Test suite for OBJECTIVE 28 Phase 1 (Core Detection)
 */

#include "blind_recovery/blind_recovery.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "math/arithmetic.h"
#include "math/transcendental.h"

#define TEST_PASSED "\033[32m✓ PASSED\033[0m"
#define TEST_FAILED "\033[31m✗ FAILED\033[0m"

static int tests_passed = 0;
static int tests_failed = 0;

void test_result(const char* test_name, int passed) {
    if (passed) {
        printf("%s: %s\n\n", test_name, TEST_PASSED);
        tests_passed++;
    } else {
        printf("%s: %s\n\n", test_name, TEST_FAILED);
        tests_failed++;
    }
}

/**
 * Test 1: Oscillation Detection
 */
void test_oscillation_detection() {
    printf("=== Test 1: Oscillation Detection ===\n");
    
    // Create synthetic oscillating data
    uint32_t num_dimensions = 3;
    uint32_t num_samples = 64;  // Power of 2 for FFT
    double sampling_rate = 100.0;  // Hz
    
    double* data = malloc(num_dimensions * num_samples * sizeof(double));
    
    // Generate sine waves at different frequencies
    for (uint32_t d = 0; d < num_dimensions; d++) {
        double freq = 5.0 * (d + 1);  // 5 Hz, 10 Hz, 15 Hz
        for (uint32_t i = 0; i < num_samples; i++) {
            double t = (double)i / sampling_rate;
            data[d * num_samples + i] = math_sin(2.0 * MATH_PI * freq * t);
        }
    }
    
    // Detect oscillations
    OscillationMap* map = detect_oscillations(data, num_dimensions, num_samples, sampling_rate);
    
    int passed = (map != NULL && map->num_dimensions == num_dimensions);
    
    if (map) {
        printf("  Detected %u oscillations\n", map->num_dimensions);
        for (uint32_t d = 0; d < map->num_dimensions; d++) {
            printf("    Dim %u: %.2f Hz (expected ~%.2f Hz)\n",
                   d, map->signatures[d].frequency, 5.0 * (d + 1));
        }
        free_oscillation_map(map);
    }
    
    free(data);
    test_result("Oscillation detection", passed);
}

/**
 * Test 2: Structural Mapping
 */
void test_structural_mapping() {
    printf("=== Test 2: Structural Mapping ===\n");
    
    // Cube: V=8, E=12, F=6
    uint32_t num_vertices = 8;
    uint32_t num_edges = 12;
    uint32_t num_faces = 6;
    
    // Create vertex positions (unit cube)
    double* positions = malloc(num_vertices * 3 * sizeof(double));
    double coords[][3] = {
        {0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0},
        {0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 1, 1}
    };
    for (int i = 0; i < 8; i++) {
        positions[i*3 + 0] = coords[i][0];
        positions[i*3 + 1] = coords[i][1];
        positions[i*3 + 2] = coords[i][2];
    }
    
    // Map structure
    StructuralMap* map = map_structure(num_vertices, num_edges, num_faces, positions);
    
    int passed = (map != NULL && 
                  map->num_vertices == num_vertices &&
                  map->num_edges == num_edges &&
                  map->num_faces == num_faces);
    
    if (map) {
        printf("  Euler check: V(%u) - E(%u) + F(%u) = %d\n",
               map->num_vertices, map->num_edges, map->num_faces,
               (int)map->num_vertices - (int)map->num_edges + (int)map->num_faces);
        free_structural_map(map);
    }
    
    free(positions);
    test_result("Structural mapping", passed);
}

/**
 * Test 3: Coprime Analysis
 */
void test_coprime_analysis() {
    printf("=== Test 3: Coprime Analysis ===\n");
    
    // Test with cube dimensions: 8, 12, 6
    uint32_t dimensions[] = {8, 12, 6};
    uint32_t num_dimensions = 3;
    
    uint64_t** matrix = compute_coprime_matrix(dimensions, num_dimensions);
    
    int passed = (matrix != NULL);
    
    if (matrix) {
        // Verify GCD(8, 12) = 4
        passed = passed && (matrix[0][1] == 4);
        // Verify GCD(8, 6) = 2
        passed = passed && (matrix[0][2] == 2);
        // Verify GCD(12, 6) = 6
        passed = passed && (matrix[1][2] == 6);
        
        free_coprime_matrix(matrix, num_dimensions);
    }
    
    test_result("Coprime analysis", passed);
}

/**
 * Test 4: Corruption Detection
 */
void test_corruption_detection() {
    printf("=== Test 4: Corruption Detection ===\n");
    
    uint32_t num_elements = 100;
    double* data = malloc(num_elements * sizeof(double));
    
    // Create mostly good data with some corruption
    for (uint32_t i = 0; i < num_elements; i++) {
        if (i % 10 == 0) {
            data[i] = NAN;  // 10% corruption
        } else {
            data[i] = (double)i * 0.1;
        }
    }
    
    bool* mask = detect_corruption(data, num_elements, 100.0);
    
    int passed = (mask != NULL);
    
    if (mask) {
        // Count corrupted elements
        uint32_t corrupted = 0;
        for (uint32_t i = 0; i < num_elements; i++) {
            if (mask[i]) corrupted++;
        }
        
        printf("  Detected %u/%u corrupted elements\n", corrupted, num_elements);
        passed = passed && (corrupted == 10);  // Should detect 10% corruption
        
        free(mask);
    }
    
    free(data);
    test_result("Corruption detection", passed);
}

/**
 * Test 5: Integrated Phase 1
 */
void test_integrated_phase1() {
    printf("=== Test 5: Integrated Phase 1 ===\n");
    
    // Create a corrupted cube structure
    uint32_t num_vertices = 8;
    uint32_t num_edges = 12;
    uint32_t num_faces = 6;
    
    // Vertex positions with some corruption
    double* positions = malloc(num_vertices * 3 * sizeof(double));
    double coords[][3] = {
        {0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0},
        {0, 0, 1}, {1, 0, 1}, {NAN, NAN, NAN}, {0, 1, 1}  // Vertex 6 corrupted
    };
    for (int i = 0; i < 8; i++) {
        positions[i*3 + 0] = coords[i][0];
        positions[i*3 + 1] = coords[i][1];
        positions[i*3 + 2] = coords[i][2];
    }
    
    // Map structure
    StructuralMap* map = map_structure(num_vertices, num_edges, num_faces, positions);
    
    int passed = (map != NULL && map->corruption_percentage > 0.0);
    
    if (map) {
        printf("  Corruption detected: %.1f%%\n", map->corruption_percentage * 100.0);
        
        // Compute coprime matrix
        uint32_t dims[] = {num_vertices, num_edges, num_faces};
        map->coprime_matrix = compute_coprime_matrix(dims, 3);
        
        passed = passed && (map->coprime_matrix != NULL);
        
        free_structural_map(map);
    }
    
    free(positions);
    test_result("Integrated Phase 1", passed);
}

int main() {
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║  OBJECTIVE 28 - PHASE 1 TEST SUITE                        ║\n");
    printf("║  Core Detection: Oscillation, Structure, Coprime          ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    // Run all tests
    test_oscillation_detection();
    test_structural_mapping();
    test_coprime_analysis();
    test_corruption_detection();
    test_integrated_phase1();
    
    // Print summary
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                    TEST SUMMARY                            ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║  Tests Passed: %-43d ║\n", tests_passed);
    printf("║  Tests Failed: %-43d ║\n", tests_failed);
    printf("║  Total Tests:  %-43d ║\n", tests_passed + tests_failed);
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    if (tests_failed == 0) {
        printf("🎉 ALL TESTS PASSED! 🎉\n");
        printf("✅ OBJECTIVE 28 Phase 1 (Core Detection) is working!\n\n");
        return 0;
    } else {
        printf("❌ SOME TESTS FAILED ❌\n\n");
        return 1;
    }
}
