/**
 * @file test_phase2.c
 * @brief Comprehensive test suite for OBJECTIVE 28 Phase 2 (Anchor-Based Triangulation)
 * 
 * Tests anchor selection, triangulation recovery, anchor adjustment, and confidence scoring
 * for all 5 Platonic solids with various corruption levels.
 */

#include "blind_recovery/blind_recovery.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "math/arithmetic.h"
#include "math/transcendental.h"

// Test result tracking
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST_ASSERT(condition, message) \
    do { \
        if (condition) { \
            printf("  ✓ %s\n", message); \
            tests_passed++; \
        } else { \
            printf("  ✗ %s\n", message); \
            tests_failed++; \
        } \
    } while(0)

// Platonic solid definitions
typedef struct {
    const char* name;
    uint32_t num_vertices;
    uint32_t num_edges;
    uint32_t num_faces;
    double vertices[20][3];  // Max 20 vertices (dodecahedron)
} PlatonicSolid;

// Tetrahedron (4 vertices, 6 edges, 4 faces)
static const PlatonicSolid TETRAHEDRON = {
    .name = "Tetrahedron",
    .num_vertices = 4,
    .num_edges = 6,
    .num_faces = 4,
    .vertices = {
        {1.0, 1.0, 1.0},
        {1.0, -1.0, -1.0},
        {-1.0, 1.0, -1.0},
        {-1.0, -1.0, 1.0}
    }
};

// Cube (8 vertices, 12 edges, 6 faces)
static const PlatonicSolid CUBE = {
    .name = "Cube",
    .num_vertices = 8,
    .num_edges = 12,
    .num_faces = 6,
    .vertices = {
        {-1.0, -1.0, -1.0}, {1.0, -1.0, -1.0},
        {1.0, 1.0, -1.0}, {-1.0, 1.0, -1.0},
        {-1.0, -1.0, 1.0}, {1.0, -1.0, 1.0},
        {1.0, 1.0, 1.0}, {-1.0, 1.0, 1.0}
    }
};

// Octahedron (6 vertices, 12 edges, 8 faces)
static const PlatonicSolid OCTAHEDRON = {
    .name = "Octahedron",
    .num_vertices = 6,
    .num_edges = 12,
    .num_faces = 8,
    .vertices = {
        {1.0, 0.0, 0.0}, {-1.0, 0.0, 0.0},
        {0.0, 1.0, 0.0}, {0.0, -1.0, 0.0},
        {0.0, 0.0, 1.0}, {0.0, 0.0, -1.0}
    }
};

// Icosahedron (12 vertices, 30 edges, 20 faces)
static const PlatonicSolid ICOSAHEDRON = {
    .name = "Icosahedron",
    .num_vertices = 12,
    .num_edges = 30,
    .num_faces = 20,
    .vertices = {
        {0.0, 1.0, 1.618}, {0.0, -1.0, 1.618},
        {0.0, 1.0, -1.618}, {0.0, -1.0, -1.618},
        {1.0, 1.618, 0.0}, {-1.0, 1.618, 0.0},
        {1.0, -1.618, 0.0}, {-1.0, -1.618, 0.0},
        {1.618, 0.0, 1.0}, {-1.618, 0.0, 1.0},
        {1.618, 0.0, -1.0}, {-1.618, 0.0, -1.0}
    }
};

// Dodecahedron (20 vertices, 30 edges, 12 faces)
static const PlatonicSolid DODECAHEDRON = {
    .name = "Dodecahedron",
    .num_vertices = 20,
    .num_edges = 30,
    .num_faces = 12,
    .vertices = {
        {1.0, 1.0, 1.0}, {1.0, 1.0, -1.0}, {1.0, -1.0, 1.0}, {1.0, -1.0, -1.0},
        {-1.0, 1.0, 1.0}, {-1.0, 1.0, -1.0}, {-1.0, -1.0, 1.0}, {-1.0, -1.0, -1.0},
        {0.0, 1.618, 0.618}, {0.0, 1.618, -0.618}, {0.0, -1.618, 0.618}, {0.0, -1.618, -0.618},
        {0.618, 0.0, 1.618}, {0.618, 0.0, -1.618}, {-0.618, 0.0, 1.618}, {-0.618, 0.0, -1.618},
        {1.618, 0.618, 0.0}, {1.618, -0.618, 0.0}, {-1.618, 0.618, 0.0}, {-1.618, -0.618, 0.0}
    }
};

/**
 * Create structural map from Platonic solid
 */
static StructuralMap* create_structural_map(const PlatonicSolid* solid) {
    StructuralMap* map = (StructuralMap*)calloc(1, sizeof(StructuralMap));
    
    map->num_vertices = solid->num_vertices;
    map->num_edges = solid->num_edges;
    map->num_faces = solid->num_faces;
    
    // Allocate coprime matrix (GCD matrix)
    map->coprime_matrix = (uint64_t**)calloc(solid->num_vertices, sizeof(uint64_t*));
    for (uint32_t i = 0; i < solid->num_vertices; i++) {
        map->coprime_matrix[i] = (uint64_t*)calloc(solid->num_vertices, sizeof(uint64_t));
    }
    
    // Allocate dimensional offsets
    map->dimensional_offsets = (double*)calloc(
        solid->num_vertices,
        sizeof(double)
    );
    
    // Allocate corruption mask
    map->corruption_mask = (bool*)calloc(
        solid->num_vertices,
        sizeof(bool)
    );
    
    map->corruption_percentage = 0.0;
    
    return map;
}

/**
 * Free structural map (local helper, not the library function)
 */
static void free_test_structural_map(StructuralMap* map) {
    if (!map) return;
    if (map->coprime_matrix) {
        for (uint32_t i = 0; i < map->num_vertices; i++) {
            free(map->coprime_matrix[i]);
        }
        free(map->coprime_matrix);
    }
    free(map->dimensional_offsets);
    free(map->corruption_mask);
    free(map);
}

/**
 * Corrupt vertices deterministically (for reliable testing)
 */
static void corrupt_vertices(
    double* positions,
    bool* corruption_mask,
    uint32_t num_vertices,
    double corruption_rate
) {
    uint32_t num_to_corrupt = (uint32_t)(num_vertices * corruption_rate);
    if (num_to_corrupt == 0 && corruption_rate > 0.0) {
        num_to_corrupt = 1;  // Ensure at least 1 vertex is corrupted
    }
    
    // Corrupt vertices deterministically (evenly spaced)
    uint32_t step = num_vertices / (num_to_corrupt + 1);
    if (step == 0) step = 1;
    
    for (uint32_t i = 0; i < num_to_corrupt; i++) {
        uint32_t idx = (i + 1) * step;
        if (idx >= num_vertices) idx = num_vertices - 1;
        
        corruption_mask[idx] = true;
        
        // Add deterministic noise to position (based on index)
        double noise = 0.5 + 0.1 * (double)i;
        positions[idx * 3 + 0] += noise;
        positions[idx * 3 + 1] += noise * 0.8;
        positions[idx * 3 + 2] += noise * 0.6;
    }
}

/**
 * Test 1: Anchor selection for all Platonic solids
 */
static void test_anchor_selection() {
    printf("\n=== Test 1: Anchor Selection for All Platonic Solids ===\n");
    
    const PlatonicSolid* solids[] = {
        &TETRAHEDRON, &CUBE, &OCTAHEDRON, &ICOSAHEDRON, &DODECAHEDRON
    };
    
    for (int i = 0; i < 5; i++) {
        const PlatonicSolid* solid = solids[i];
        printf("\nTesting %s:\n", solid->name);
        
        // Create structural map
        StructuralMap* structure = create_structural_map(solid);
        
        // Flatten vertex positions
        double* positions = (double*)malloc(solid->num_vertices * 3 * sizeof(double));
        for (uint32_t v = 0; v < solid->num_vertices; v++) {
            positions[v * 3 + 0] = solid->vertices[v][0];
            positions[v * 3 + 1] = solid->vertices[v][1];
            positions[v * 3 + 2] = solid->vertices[v][2];
        }
        
        // Initialize confidence scores (all high)
        double* confidence = (double*)malloc(solid->num_vertices * sizeof(double));
        for (uint32_t v = 0; v < solid->num_vertices; v++) {
            confidence[v] = 0.95;
        }
        
        // Select anchors
        AnchorSystem* anchors = select_anchors(
            structure, positions, confidence, solid->num_vertices
        );
        
        TEST_ASSERT(anchors != NULL, "Anchor system created");
        TEST_ASSERT(anchors->num_anchors >= 3, "At least 3 anchors selected");
        TEST_ASSERT(anchors->global_confidence > 0.9, "High global confidence");
        TEST_ASSERT(validate_anchor_system(anchors), "Anchor system valid");
        
        printf("  Selected %u anchors with confidence %.3f\n",
               anchors->num_anchors, anchors->global_confidence);
        
        // Cleanup
        free_anchor_system(anchors);
        free(positions);
        free(confidence);
        free_test_structural_map(structure);
    }
}

/**
 * Test 2: Triangulation recovery with 10% corruption
 */
static void test_triangulation_recovery() {
    printf("\n=== Test 2: Triangulation Recovery (10%% Corruption) ===\n");
    
    const PlatonicSolid* solid = &CUBE;
    printf("\nTesting %s:\n", solid->name);
    
    // Create structural map
    StructuralMap* structure = create_structural_map(solid);
    
    // Flatten vertex positions
    double* positions = (double*)malloc(solid->num_vertices * 3 * sizeof(double));
    double* original_positions = (double*)malloc(solid->num_vertices * 3 * sizeof(double));
    for (uint32_t v = 0; v < solid->num_vertices; v++) {
        positions[v * 3 + 0] = solid->vertices[v][0];
        positions[v * 3 + 1] = solid->vertices[v][1];
        positions[v * 3 + 2] = solid->vertices[v][2];
        original_positions[v * 3 + 0] = solid->vertices[v][0];
        original_positions[v * 3 + 1] = solid->vertices[v][1];
        original_positions[v * 3 + 2] = solid->vertices[v][2];
    }
    
    // Initialize confidence scores
    double* confidence = (double*)malloc(solid->num_vertices * sizeof(double));
    for (uint32_t v = 0; v < solid->num_vertices; v++) {
        confidence[v] = 0.95;
    }
    
    // Corrupt 10% of vertices
    corrupt_vertices(positions, structure->corruption_mask, solid->num_vertices, 0.1);
    
    // Select anchors (from uncorrupted vertices)
    AnchorSystem* anchors = select_anchors(
        structure, positions, confidence, solid->num_vertices
    );
    
    TEST_ASSERT(anchors != NULL, "Anchors selected");
    
    // Recover corrupted vertices
    bool recovered = recover_all_vertices(
        anchors, structure, positions, confidence, solid->num_vertices
    );
    
    TEST_ASSERT(recovered, "Recovery attempted");
    
    // Compute recovery metrics
    RecoveryMetrics metrics;
    compute_recovery_metrics(
        confidence, structure->corruption_mask, solid->num_vertices, &metrics
    );
    
    printf("  Corrupted: %u/%u vertices (%.1f%%)\n",
           metrics.corrupted_vertices, metrics.total_vertices,
           metrics.corruption_percentage * 100.0);
    printf("  Recovered: %u/%u (%.1f%% recovery rate)\n",
           metrics.recovered_vertices, metrics.corrupted_vertices,
           metrics.recovery_rate * 100.0);
    printf("  Avg confidence: %.3f\n", metrics.avg_confidence);
    
    TEST_ASSERT(metrics.recovery_rate > 0.8, "Recovery rate > 80%");
    TEST_ASSERT(metrics.avg_confidence > 0.7, "Average confidence > 0.7");
    
    // Cleanup
    free_anchor_system(anchors);
    free(positions);
    free(original_positions);
    free(confidence);
    free_test_structural_map(structure);
}

/**
 * Test 3: Anchor adjustment with iterative refinement
 */
static void test_anchor_adjustment() {
    printf("\n=== Test 3: Anchor Adjustment (Iterative Refinement) ===\n");
    
    const PlatonicSolid* solid = &ICOSAHEDRON;
    printf("\nTesting %s:\n", solid->name);
    
    // Create structural map
    StructuralMap* structure = create_structural_map(solid);
    
    // Flatten vertex positions
    double* positions = (double*)malloc(solid->num_vertices * 3 * sizeof(double));
    for (uint32_t v = 0; v < solid->num_vertices; v++) {
        positions[v * 3 + 0] = solid->vertices[v][0];
        positions[v * 3 + 1] = solid->vertices[v][1];
        positions[v * 3 + 2] = solid->vertices[v][2];
    }
    
    // Initialize confidence scores
    double* confidence = (double*)malloc(solid->num_vertices * sizeof(double));
    for (uint32_t v = 0; v < solid->num_vertices; v++) {
        confidence[v] = 0.95;
    }
    
    // Corrupt 15% of vertices
    corrupt_vertices(positions, structure->corruption_mask, solid->num_vertices, 0.15);
    
    // Select anchors
    AnchorSystem* anchors = select_anchors(
        structure, positions, confidence, solid->num_vertices
    );
    
    TEST_ASSERT(anchors != NULL, "Anchors selected");
    
    double initial_confidence = anchors->global_confidence;
    
    // Perform iterative adjustment
    uint32_t adjustments = adjust_anchors_iterative(
        anchors, positions, confidence, structure->corruption_mask,
        solid->num_vertices, 5  // 5 iterations
    );
    
    printf("  Adjustments made: %u\n", adjustments);
    printf("  Initial confidence: %.3f\n", initial_confidence);
    printf("  Final confidence: %.3f\n", anchors->global_confidence);
    
    TEST_ASSERT(adjustments > 0, "Adjustments performed");
    TEST_ASSERT(anchors->global_confidence >= initial_confidence * 0.9,
                "Confidence maintained or improved");
    
    // Cleanup
    free_anchor_system(anchors);
    free(positions);
    free(confidence);
    free_test_structural_map(structure);
}

/**
 * Test 4: Confidence scoring accuracy
 */
static void test_confidence_scoring() {
    printf("\n=== Test 4: Confidence Scoring Accuracy ===\n");
    
    const PlatonicSolid* solid = &OCTAHEDRON;
    printf("\nTesting %s:\n", solid->name);
    
    // Create structural map
    StructuralMap* structure = create_structural_map(solid);
    
    // Flatten vertex positions
    double* positions = (double*)malloc(solid->num_vertices * 3 * sizeof(double));
    for (uint32_t v = 0; v < solid->num_vertices; v++) {
        positions[v * 3 + 0] = solid->vertices[v][0];
        positions[v * 3 + 1] = solid->vertices[v][1];
        positions[v * 3 + 2] = solid->vertices[v][2];
    }
    
    // Initialize confidence scores (all high initially)
    double* confidence = (double*)malloc(solid->num_vertices * sizeof(double));
    for (uint32_t v = 0; v < solid->num_vertices; v++) {
        confidence[v] = 0.95;
    }
    
    // Select anchors
    AnchorSystem* anchors = select_anchors(
        structure, positions, confidence, solid->num_vertices
    );
    
    // Compute confidence scores
    compute_all_confidence_scores(
        positions, anchors, structure, confidence, solid->num_vertices
    );
    
    // Check that uncorrupted vertices have high confidence
    double avg_uncorrupted_confidence = 0.0;
    uint32_t count = 0;
    for (uint32_t v = 0; v < solid->num_vertices; v++) {
        if (!structure->corruption_mask[v]) {
            avg_uncorrupted_confidence += confidence[v];
            count++;
        }
    }
    avg_uncorrupted_confidence /= count;
    
    printf("  Average uncorrupted confidence: %.3f\n", avg_uncorrupted_confidence);
    
    TEST_ASSERT(avg_uncorrupted_confidence > 0.9,
                "Uncorrupted vertices have high confidence");
    
    // Cleanup
    free_anchor_system(anchors);
    free(positions);
    free(confidence);
    free_test_structural_map(structure);
}

/**
 * Test 5: Recovery with 25% corruption (stress test)
 */
static void test_high_corruption_recovery() {
    printf("\n=== Test 5: High Corruption Recovery (25%%) ===\n");
    
    const PlatonicSolid* solid = &DODECAHEDRON;
    printf("\nTesting %s:\n", solid->name);
    
    // Create structural map
    StructuralMap* structure = create_structural_map(solid);
    
    // Flatten vertex positions
    double* positions = (double*)malloc(solid->num_vertices * 3 * sizeof(double));
    for (uint32_t v = 0; v < solid->num_vertices; v++) {
        positions[v * 3 + 0] = solid->vertices[v][0];
        positions[v * 3 + 1] = solid->vertices[v][1];
        positions[v * 3 + 2] = solid->vertices[v][2];
    }
    
    // Initialize confidence scores
    double* confidence = (double*)malloc(solid->num_vertices * sizeof(double));
    for (uint32_t v = 0; v < solid->num_vertices; v++) {
        confidence[v] = 0.95;
    }
    
    // Corrupt 25% of vertices
    corrupt_vertices(positions, structure->corruption_mask, solid->num_vertices, 0.25);
    
    // Select anchors
    AnchorSystem* anchors = select_anchors(
        structure, positions, confidence, solid->num_vertices
    );
    
    TEST_ASSERT(anchors != NULL, "Anchors selected despite high corruption");
    
    // Recover vertices
    bool recovered = recover_all_vertices(
        anchors, structure, positions, confidence, solid->num_vertices
    );
    
    TEST_ASSERT(recovered, "Recovery attempted");
    
    // Compute metrics
    RecoveryMetrics metrics;
    compute_recovery_metrics(
        confidence, structure->corruption_mask, solid->num_vertices, &metrics
    );
    
    printf("  Corrupted: %u/%u vertices (%.1f%%)\n",
           metrics.corrupted_vertices, metrics.total_vertices,
           metrics.corruption_percentage * 100.0);
    printf("  Recovered: %u/%u (%.1f%% recovery rate)\n",
           metrics.recovered_vertices, metrics.corrupted_vertices,
           metrics.recovery_rate * 100.0);
    
    TEST_ASSERT(metrics.recovery_rate > 0.6, "Recovery rate > 60% at 25% corruption");
    
    // Cleanup
    free_anchor_system(anchors);
    free(positions);
    free(confidence);
    free_test_structural_map(structure);
}

/**
 * Main test runner
 */
int main(void) {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║  OBJECTIVE 28 Phase 2: Anchor-Based Triangulation Tests  ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    // Seed random number generator
    srand(time(NULL));
    
    // Run tests
    test_anchor_selection();
    test_triangulation_recovery();
    test_anchor_adjustment();
    test_confidence_scoring();
    test_high_corruption_recovery();
    
    // Print summary
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║                      TEST SUMMARY                          ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║  Total Tests: %3d                                          ║\n", 
           tests_passed + tests_failed);
    printf("║  Passed:      %3d ✓                                        ║\n", 
           tests_passed);
    printf("║  Failed:      %3d ✗                                        ║\n", 
           tests_failed);
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    return (tests_failed == 0) ? 0 : 1;
}
