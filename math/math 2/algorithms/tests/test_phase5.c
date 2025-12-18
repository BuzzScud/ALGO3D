/**
 * OBJECTIVE 28 - Phase 5: Dynamic Model Expansion
 * Comprehensive Test Suite
 * 
 * Tests dynamic model expansion, Platonic solid transformations,
 * and self-similar structure generation.
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
    PlatonicSolidType type;
    int vertices;
    int edges;
    int faces;
} PlatonicSolidDef;

static const PlatonicSolidDef PLATONIC_SOLIDS[] = {
    {"Tetrahedron", PLATONIC_TETRAHEDRON, 4, 6, 4},
    {"Cube", PLATONIC_CUBE, 8, 12, 6},
    {"Octahedron", PLATONIC_OCTAHEDRON, 6, 12, 8},
    {"Dodecahedron", PLATONIC_DODECAHEDRON, 20, 30, 12},
    {"Icosahedron", PLATONIC_ICOSAHEDRON, 12, 30, 20}
};

static const int NUM_PLATONIC_SOLIDS = 5;

/**
 * Helper: Create test structure for Platonic solid
 */
static StructuralMap create_platonic_structure(const PlatonicSolidDef* solid) {
    StructuralMap structure = {
        .num_vertices = solid->vertices,
        .num_edges = solid->edges,
        .num_faces = solid->faces,
        .coprime_matrix = NULL,
        .dimensional_offsets = NULL,
        .corruption_mask = NULL,
        .corruption_percentage = 0.0
    };
    return structure;
}

/**
 * Test 1: Platonic solid expansion to dual
 */
static void test_platonic_dual_expansion(void) {
    TEST("Platonic solid expansion to dual");
    
    // Tetrahedron is self-dual
    StructuralMap tetra = create_platonic_structure(&PLATONIC_SOLIDS[0]);
    StructuralMap* expanded_tetra = expand_to_platonic_solid(&tetra, PLATONIC_TETRAHEDRON);
    ASSERT(expanded_tetra != NULL, "Tetrahedron → Tetrahedron (self-dual)");
    if (expanded_tetra) {
        ASSERT(is_expansion_valid(expanded_tetra), "Expansion maintains Euler's formula");
        free_expanded_model(expanded_tetra);
    }
    
    // Cube ↔ Octahedron (dual pair)
    StructuralMap cube = create_platonic_structure(&PLATONIC_SOLIDS[1]);
    StructuralMap* cube_to_oct = expand_to_platonic_solid(&cube, PLATONIC_OCTAHEDRON);
    ASSERT(cube_to_oct != NULL, "Cube → Octahedron (dual)");
    if (cube_to_oct) {
        ASSERT(is_expansion_valid(cube_to_oct), "Cube→Octahedron maintains Euler's formula");
        ASSERT(cube_to_oct->num_vertices == 6, "Octahedron has 6 vertices");
        ASSERT(cube_to_oct->num_edges == 12, "Octahedron has 12 edges");
        ASSERT(cube_to_oct->num_faces == 8, "Octahedron has 8 faces");
        free_expanded_model(cube_to_oct);
    }
    
    // Octahedron → Cube (reverse dual)
    StructuralMap oct = create_platonic_structure(&PLATONIC_SOLIDS[2]);
    StructuralMap* oct_to_cube = expand_to_platonic_solid(&oct, PLATONIC_CUBE);
    ASSERT(oct_to_cube != NULL, "Octahedron → Cube (dual)");
    if (oct_to_cube) {
        ASSERT(is_expansion_valid(oct_to_cube), "Octahedron→Cube maintains Euler's formula");
        ASSERT(oct_to_cube->num_vertices == 8, "Cube has 8 vertices");
        ASSERT(oct_to_cube->num_edges == 12, "Cube has 12 edges");
        ASSERT(oct_to_cube->num_faces == 6, "Cube has 6 faces");
        free_expanded_model(oct_to_cube);
    }
    
    // Dodecahedron ↔ Icosahedron (dual pair)
    StructuralMap dodeca = create_platonic_structure(&PLATONIC_SOLIDS[3]);
    StructuralMap* dodeca_to_icosa = expand_to_platonic_solid(&dodeca, PLATONIC_ICOSAHEDRON);
    ASSERT(dodeca_to_icosa != NULL, "Dodecahedron → Icosahedron (dual)");
    if (dodeca_to_icosa) {
        ASSERT(is_expansion_valid(dodeca_to_icosa), "Dodecahedron→Icosahedron maintains Euler's formula");
        ASSERT(dodeca_to_icosa->num_vertices == 12, "Icosahedron has 12 vertices");
        ASSERT(dodeca_to_icosa->num_edges == 30, "Icosahedron has 30 edges");
        ASSERT(dodeca_to_icosa->num_faces == 20, "Icosahedron has 20 faces");
        free_expanded_model(dodeca_to_icosa);
    }
}

/**
 * Test 2: Expansion by level
 */
static void test_expansion_by_level(void) {
    TEST("Expansion by level");
    
    StructuralMap cube = create_platonic_structure(&PLATONIC_SOLIDS[1]);
    
    // Level 1 expansion
    StructuralMap* level1 = expand_model_by_level(&cube, 1);
    ASSERT(level1 != NULL, "Level 1 expansion created");
    if (level1) {
        ASSERT(is_expansion_valid(level1), "Level 1 maintains Euler's formula");
        ASSERT(level1->num_vertices > cube.num_vertices, "Level 1 has more vertices");
        
        printf("  Cube: V=%u, E=%u, F=%u\n", cube.num_vertices, cube.num_edges, cube.num_faces);
        printf("  Level 1: V=%u, E=%u, F=%u\n", level1->num_vertices, level1->num_edges, level1->num_faces);
        
        free_expanded_model(level1);
    }
    
    // Level 2 expansion
    StructuralMap* level2 = expand_model_by_level(&cube, 2);
    ASSERT(level2 != NULL, "Level 2 expansion created");
    if (level2) {
        ASSERT(is_expansion_valid(level2), "Level 2 maintains Euler's formula");
        ASSERT(level2->num_vertices > cube.num_vertices, "Level 2 has more vertices");
        
        printf("  Level 2: V=%u, E=%u, F=%u\n", level2->num_vertices, level2->num_edges, level2->num_faces);
        
        free_expanded_model(level2);
    }
    
    // Level 3 expansion
    StructuralMap* level3 = expand_model_by_level(&cube, 3);
    ASSERT(level3 != NULL, "Level 3 expansion created");
    if (level3) {
        ASSERT(is_expansion_valid(level3), "Level 3 maintains Euler's formula");
        ASSERT(level3->num_vertices > cube.num_vertices, "Level 3 has more vertices");
        
        printf("  Level 3: V=%u, E=%u, F=%u\n", level3->num_vertices, level3->num_edges, level3->num_faces);
        
        free_expanded_model(level3);
    }
}

/**
 * Test 3: Expansion factors
 */
static void test_expansion_factors(void) {
    TEST("Expansion factors between Platonic solids");
    
    // Cube → Octahedron (dual)
    double factor_cube_oct = get_expansion_factor(PLATONIC_CUBE, PLATONIC_OCTAHEDRON);
    printf("  Cube → Octahedron: factor=%.3f\n", factor_cube_oct);
    ASSERT(factor_cube_oct > 1.0, "Cube→Octahedron has expansion factor");
    
    // Dodecahedron → Icosahedron (dual)
    double factor_dodeca_icosa = get_expansion_factor(PLATONIC_DODECAHEDRON, PLATONIC_ICOSAHEDRON);
    printf("  Dodecahedron → Icosahedron: factor=%.3f\n", factor_dodeca_icosa);
    ASSERT(factor_dodeca_icosa > 1.0, "Dodecahedron→Icosahedron has expansion factor");
    
    // Tetrahedron → Icosahedron (golden ratio)
    double factor_tetra_icosa = get_expansion_factor(PLATONIC_TETRAHEDRON, PLATONIC_ICOSAHEDRON);
    printf("  Tetrahedron → Icosahedron: factor=%.3f\n", factor_tetra_icosa);
    ASSERT(factor_tetra_icosa > 2.0, "Tetrahedron→Icosahedron has large expansion");
}

/**
 * Test 4: Available expansions
 */
static void test_available_expansions(void) {
    TEST("Available expansions for each Platonic solid");
    
    for (int i = 0; i < NUM_PLATONIC_SOLIDS; i++) {
        const PlatonicSolidDef* solid = &PLATONIC_SOLIDS[i];
        
        PlatonicSolidType targets[10];
        int num_expansions = get_available_expansions(solid->type, targets, 10);
        
        printf("  %s: %d available expansions\n", solid->name, num_expansions);
        
        for (int j = 0; j < num_expansions; j++) {
            const char* target_name = "Unknown";
            for (int k = 0; k < NUM_PLATONIC_SOLIDS; k++) {
                if (PLATONIC_SOLIDS[k].type == targets[j]) {
                    target_name = PLATONIC_SOLIDS[k].name;
                    break;
                }
            }
            printf("    → %s\n", target_name);
        }
        
        ASSERT(num_expansions >= 0, "Has valid expansion count");
    }
}

/**
 * Test 5: Expansion metrics
 */
static void test_expansion_metrics(void) {
    TEST("Expansion metrics computation");
    
    StructuralMap cube = create_platonic_structure(&PLATONIC_SOLIDS[1]);
    cube.corruption_percentage = 0.15;
    
    StructuralMap* expanded = expand_to_platonic_solid(&cube, PLATONIC_OCTAHEDRON);
    ASSERT(expanded != NULL, "Expansion created");
    
    if (expanded) {
        ExpansionMetrics metrics;
        compute_expansion_metrics(&cube, expanded, &metrics);
        
        printf("  Vertex ratio: %.3f\n", metrics.vertex_ratio);
        printf("  Edge ratio: %.3f\n", metrics.edge_ratio);
        printf("  Face ratio: %.3f\n", metrics.face_ratio);
        printf("  Corruption reduction: %.3f\n", metrics.corruption_reduction);
        printf("  Euler maintained: %s\n", metrics.euler_maintained ? "yes" : "no");
        printf("  Quality score: %.3f\n", metrics.quality_score);
        
        ASSERT(metrics.euler_maintained, "Euler's formula maintained");
        ASSERT(metrics.quality_score > 0.0, "Positive quality score");
        
        free_expanded_model(expanded);
    }
}

/**
 * Test 6: Self-similar hierarchy generation
 */
static void test_self_similar_hierarchy(void) {
    TEST("Self-similar hierarchy generation");
    
    StructuralMap cube = create_platonic_structure(&PLATONIC_SOLIDS[1]);
    
    // Generate 3-level hierarchy
    SelfSimilarHierarchy* hierarchy = generate_self_similar_hierarchy(&cube, 3);
    ASSERT(hierarchy != NULL, "Hierarchy created");
    
    if (hierarchy) {
        ASSERT(hierarchy->num_levels == 3, "Correct number of levels");
        
        // Check each level
        for (uint32_t i = 0; i < hierarchy->num_levels; i++) {
            const StructuralMap* level = get_hierarchy_level(hierarchy, i);
            ASSERT(level != NULL, "Level exists");
            
            if (level) {
                int euler = level->num_vertices - level->num_edges + level->num_faces;
                printf("  Level %u: V=%u, E=%u, F=%u, χ=%d\n",
                       i, level->num_vertices, level->num_edges, level->num_faces, euler);
                ASSERT(euler == 2, "Euler's formula holds");
            }
        }
        
        free_self_similar_hierarchy(hierarchy);
    }
}

/**
 * Test 7: Hierarchy validation
 */
static void test_hierarchy_validation(void) {
    TEST("Self-similar hierarchy validation");
    
    StructuralMap icosa = create_platonic_structure(&PLATONIC_SOLIDS[4]);
    
    SelfSimilarHierarchy* hierarchy = generate_self_similar_hierarchy(&icosa, 4);
    ASSERT(hierarchy != NULL, "Hierarchy created");
    
    if (hierarchy) {
        bool valid = validate_self_similar_hierarchy(hierarchy);
        printf("  Hierarchy valid: %s\n", valid ? "yes" : "no");
        ASSERT(valid, "Hierarchy is valid");
        
        // Check self-similarity score
        double score = compute_self_similarity_score(hierarchy);
        printf("  Self-similarity score: %.3f\n", score);
        ASSERT(score > 0.0, "Positive self-similarity score");
        ASSERT(score <= 1.0, "Score in valid range");
        
        free_self_similar_hierarchy(hierarchy);
    }
}

/**
 * Test 8: Hierarchy metrics
 */
static void test_hierarchy_metrics(void) {
    TEST("Hierarchy metrics computation");
    
    StructuralMap dodeca = create_platonic_structure(&PLATONIC_SOLIDS[3]);
    dodeca.corruption_percentage = 0.10;
    
    SelfSimilarHierarchy* hierarchy = generate_self_similar_hierarchy(&dodeca, 5);
    ASSERT(hierarchy != NULL, "Hierarchy created");
    
    if (hierarchy) {
        HierarchyMetrics metrics;
        compute_hierarchy_metrics(hierarchy, &metrics);
        
        printf("  Number of levels: %u\n", metrics.num_levels);
        printf("  Self-similarity score: %.3f\n", metrics.self_similarity_score);
        printf("  All levels valid: %s\n", metrics.all_levels_valid ? "yes" : "no");
        printf("  Average corruption: %.3f\n", metrics.average_corruption);
        printf("  Total vertices: %u\n", metrics.total_vertices);
        
        ASSERT(metrics.num_levels == 5, "Correct number of levels");
        ASSERT(metrics.all_levels_valid, "All levels valid");
        ASSERT(metrics.total_vertices > 0, "Has vertices");
        
        free_self_similar_hierarchy(hierarchy);
    }
}

/**
 * Test 9: Recovery using hierarchy
 */
static void test_recovery_using_hierarchy(void) {
    TEST("Recovery using self-similar hierarchy");
    
    StructuralMap oct = create_platonic_structure(&PLATONIC_SOLIDS[2]);
    oct.corruption_percentage = 0.20;
    
    SelfSimilarHierarchy* hierarchy = generate_self_similar_hierarchy(&oct, 4);
    ASSERT(hierarchy != NULL, "Hierarchy created");
    
    if (hierarchy) {
        // Try to recover level 2 using information from other levels
        bool recovered = recover_using_hierarchy(hierarchy, 2);
        printf("  Recovery successful: %s\n", recovered ? "yes" : "no");
        ASSERT(recovered, "Recovery succeeded");
        
        // Check if corruption was reduced
        const StructuralMap* level2 = get_hierarchy_level(hierarchy, 2);
        if (level2) {
            printf("  Level 2 corruption after recovery: %.3f\n", level2->corruption_percentage);
            ASSERT(level2->corruption_percentage < 0.20, "Corruption reduced");
        }
        
        free_self_similar_hierarchy(hierarchy);
    }
}

/**
 * Test 10: All Platonic solids with hierarchy
 */
static void test_all_solids_hierarchy(void) {
    TEST("Self-similar hierarchy for all Platonic solids");
    
    for (int i = 0; i < NUM_PLATONIC_SOLIDS; i++) {
        const PlatonicSolidDef* solid = &PLATONIC_SOLIDS[i];
        
        StructuralMap structure = create_platonic_structure(solid);
        SelfSimilarHierarchy* hierarchy = generate_self_similar_hierarchy(&structure, 3);
        
        char msg[256];
        snprintf(msg, sizeof(msg), "%s: hierarchy created", solid->name);
        ASSERT(hierarchy != NULL, msg);
        
        if (hierarchy) {
            bool valid = validate_self_similar_hierarchy(hierarchy);
            snprintf(msg, sizeof(msg), "%s: hierarchy valid", solid->name);
            ASSERT(valid, msg);
            
            printf("  %s: %u levels, valid=%s\n",
                   solid->name, hierarchy->num_levels, valid ? "yes" : "no");
            
            free_self_similar_hierarchy(hierarchy);
        }
    }
}

/**
 * Test 11: Expansion with corruption
 */
static void test_expansion_with_corruption(void) {
    TEST("Expansion with varying corruption levels");
    
    double corruption_levels[] = {0.05, 0.10, 0.15, 0.20};
    int num_levels = sizeof(corruption_levels) / sizeof(corruption_levels[0]);
    
    StructuralMap cube = create_platonic_structure(&PLATONIC_SOLIDS[1]);
    
    for (int i = 0; i < num_levels; i++) {
        cube.corruption_percentage = corruption_levels[i];
        
        StructuralMap* expanded = expand_to_platonic_solid(&cube, PLATONIC_OCTAHEDRON);
        ASSERT(expanded != NULL, "Expansion with corruption created");
        
        if (expanded) {
            printf("  Corruption %.0f%%: before=%.3f, after=%.3f\n",
                   corruption_levels[i] * 100,
                   cube.corruption_percentage,
                   expanded->corruption_percentage);
            
            ASSERT(expanded->corruption_percentage <= cube.corruption_percentage,
                   "Corruption not increased");
            
            free_expanded_model(expanded);
        }
    }
}

/**
 * Test 12: Large hierarchy
 */
static void test_large_hierarchy(void) {
    TEST("Large self-similar hierarchy (10 levels)");
    
    StructuralMap tetra = create_platonic_structure(&PLATONIC_SOLIDS[0]);
    
    SelfSimilarHierarchy* hierarchy = generate_self_similar_hierarchy(&tetra, 10);
    ASSERT(hierarchy != NULL, "Large hierarchy created");
    
    if (hierarchy) {
        ASSERT(hierarchy->num_levels == 10, "Correct number of levels");
        
        bool valid = validate_self_similar_hierarchy(hierarchy);
        printf("  10-level hierarchy valid: %s\n", valid ? "yes" : "no");
        ASSERT(valid, "Large hierarchy is valid");
        
        HierarchyMetrics metrics;
        compute_hierarchy_metrics(hierarchy, &metrics);
        printf("  Total vertices across all levels: %u\n", metrics.total_vertices);
        
        free_self_similar_hierarchy(hierarchy);
    }
}

/**
 * Main test runner
 */
int main(void) {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║  OBJECTIVE 28 - Phase 5: Dynamic Model Expansion          ║\n");
    printf("║  Comprehensive Test Suite                                  ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    // Run all tests
    test_platonic_dual_expansion();
    test_expansion_by_level();
    test_expansion_factors();
    test_available_expansions();
    test_expansion_metrics();
    test_self_similar_hierarchy();
    test_hierarchy_validation();
    test_hierarchy_metrics();
    test_recovery_using_hierarchy();
    test_all_solids_hierarchy();
    test_expansion_with_corruption();
    test_large_hierarchy();
    
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
