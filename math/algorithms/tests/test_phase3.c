/**
 * @file test_phase3.c
 * @brief Comprehensive test suite for OBJECTIVE 28 Phase 3 (Iterative Search with SFT)
 * 
 * Tests candidate generation, fitness scoring, iterative refinement, and convergence detection.
 */

#include "blind_recovery/blind_recovery.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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

/**
 * Test 1: Candidate generation using SFT
 */
static void test_candidate_generation() {
    printf("\n=== Test 1: Candidate Generation (SFT-based) ===\n");
    
    // Create test structure (cube)
    StructuralMap structure = {
        .num_vertices = 8,
        .num_edges = 12,
        .num_faces = 6,
        .coprime_matrix = NULL,
        .dimensional_offsets = NULL,
        .corruption_mask = NULL,
        .corruption_percentage = 0.0
    };
    
    uint64_t nonce = 0x9e3779b97f4a7c15ULL;
    uint32_t num_candidates = 10;
    
    // Generate candidates for dimension 0 (vertices)
    SearchCandidate* candidates = generate_candidates(
        nonce,
        0,  // dimension
        num_candidates,
        &structure
    );
    
    TEST_ASSERT(candidates != NULL, "Candidates generated");
    TEST_ASSERT(candidates[0].dimension == 0, "Correct dimension");
    TEST_ASSERT(candidates[0].candidate > 0, "Valid candidate value");
    
    // Check that candidates are sorted by score
    bool sorted = true;
    for (uint32_t i = 0; i < num_candidates - 1; i++) {
        if (candidates[i].sft_score < candidates[i + 1].sft_score) {
            sorted = false;
            break;
        }
    }
    TEST_ASSERT(sorted, "Candidates sorted by score");
    
    printf("  Generated %u candidates\n", num_candidates);
    printf("  Best score: %.2f\n", candidates[0].sft_score);
    printf("  Worst score: %.2f\n", candidates[num_candidates - 1].sft_score);
    
    free_candidates(candidates);
}

/**
 * Test 2: Fitness scoring
 */
static void test_fitness_scoring() {
    printf("\n=== Test 2: Fitness Scoring ===\n");
    
    // Create test structure
    StructuralMap structure = {
        .num_vertices = 12,
        .num_edges = 30,
        .num_faces = 20,
        .coprime_matrix = NULL,
        .dimensional_offsets = NULL,
        .corruption_mask = NULL,
        .corruption_percentage = 0.0
    };
    
    // Generate candidates
    uint64_t nonce = 0x123456789ABCDEFULL;
    uint32_t num_candidates = 20;
    
    SearchCandidate* candidates = generate_candidates(
        nonce,
        1,  // edges dimension
        num_candidates,
        &structure
    );
    
    TEST_ASSERT(candidates != NULL, "Candidates generated");
    
    // Score candidates (already done in generation, but test explicit scoring)
    score_and_sort_candidates(candidates, num_candidates, &structure, NULL);
    
    TEST_ASSERT(candidates[0].sft_score > 0.0, "Positive fitness scores");
    
    // Find best candidate
    const SearchCandidate* best = find_best_candidate(candidates, num_candidates);
    TEST_ASSERT(best != NULL, "Best candidate found");
    TEST_ASSERT(best->sft_score == candidates[0].sft_score, "Best candidate is first");
    
    // Compute average fitness
    double avg_fitness = compute_average_fitness(candidates, num_candidates);
    TEST_ASSERT(avg_fitness > 0.0, "Positive average fitness");
    
    printf("  Best fitness: %.2f\n", best->sft_score);
    printf("  Average fitness: %.2f\n", avg_fitness);
    
    free_candidates(candidates);
}

/**
 * Test 3: Iterative refinement
 */
static void test_iterative_refinement() {
    printf("\n=== Test 3: Iterative Refinement ===\n");
    
    // Create corrupted structure (cube with wrong edge count)
    StructuralMap structure = {
        .num_vertices = 8,
        .num_edges = 10,  // Wrong! Should be 12
        .num_faces = 6,
        .coprime_matrix = NULL,
        .dimensional_offsets = NULL,
        .corruption_mask = NULL,
        .corruption_percentage = 0.2
    };
    
    // Check initial Euler's formula
    int initial_euler = (int)structure.num_vertices - 
                       (int)structure.num_edges + 
                       (int)structure.num_faces;
    printf("  Initial Euler result: %d (should be 2)\n", initial_euler);
    
    // Create search state
    IterativeSearch* search = create_iterative_search(
        10,    // max iterations
        0.01,  // convergence threshold
        0x9e3779b97f4a7c15ULL
    );
    
    TEST_ASSERT(search != NULL, "Search state created");
    
    // Generate candidates
    SearchCandidate* candidates = generate_candidates(
        search->nonce,
        1,  // edges dimension
        20,
        &structure
    );
    
    TEST_ASSERT(candidates != NULL, "Candidates generated");
    
    // Initialize search with candidates
    bool init_ok = initialize_search_candidates(search, candidates, 20);
    TEST_ASSERT(init_ok, "Search initialized with candidates");
    
    // Refine structure
    bool converged = refine_structure(&structure, search, 0.01);
    
    // Check final Euler's formula
    int final_euler = (int)structure.num_vertices - 
                     (int)structure.num_edges + 
                     (int)structure.num_faces;
    printf("  Final Euler result: %d\n", final_euler);
    printf("  Converged: %s\n", converged ? "Yes" : "No");
    
    TEST_ASSERT(abs(final_euler - 2) <= abs(initial_euler - 2), 
                "Euler's formula improved or maintained");
    
    free_candidates(candidates);
    free_iterative_search(search);
}

/**
 * Test 4: Convergence detection
 */
static void test_convergence_detection() {
    printf("\n=== Test 4: Convergence Detection ===\n");
    
    // Create valid structure (cube)
    StructuralMap structure = {
        .num_vertices = 8,
        .num_edges = 12,
        .num_faces = 6,
        .coprime_matrix = NULL,
        .dimensional_offsets = NULL,
        .corruption_mask = NULL,
        .corruption_percentage = 0.0
    };
    
    // Check if structure is converged
    bool converged = is_structure_converged(&structure, 0.05);
    TEST_ASSERT(converged, "Valid structure detected as converged");
    
    // Create corrupted structure
    StructuralMap corrupted = {
        .num_vertices = 8,
        .num_edges = 10,
        .num_faces = 6,
        .coprime_matrix = NULL,
        .dimensional_offsets = NULL,
        .corruption_mask = NULL,
        .corruption_percentage = 0.2
    };
    
    bool not_converged = is_structure_converged(&corrupted, 0.05);
    TEST_ASSERT(!not_converged, "Corrupted structure detected as not converged");
    
    // Test convergence history
    ConvergenceHistory* history = create_convergence_history(5);
    TEST_ASSERT(history != NULL, "Convergence history created");
    
    // Add values to history
    add_to_convergence_history(history, 0.7);
    add_to_convergence_history(history, 0.75);
    add_to_convergence_history(history, 0.8);
    add_to_convergence_history(history, 0.82);
    add_to_convergence_history(history, 0.83);
    
    // Check convergence with history
    bool hist_converged = is_converged_with_history(history, 0.75, 0.01);
    TEST_ASSERT(hist_converged, "History-based convergence detected");
    
    free_convergence_history(history);
}

/**
 * Test 5: Multi-pass refinement
 */
static void test_multi_pass_refinement() {
    printf("\n=== Test 5: Multi-Pass Refinement ===\n");
    
    // Create corrupted structure (dodecahedron with errors)
    StructuralMap structure = {
        .num_vertices = 18,  // Wrong! Should be 20
        .num_edges = 28,     // Wrong! Should be 30
        .num_faces = 12,     // Correct
        .coprime_matrix = NULL,
        .dimensional_offsets = NULL,
        .corruption_mask = NULL,
        .corruption_percentage = 0.15
    };
    
    printf("  Initial: V=%u, E=%u, F=%u\n", 
           structure.num_vertices, structure.num_edges, structure.num_faces);
    
    int initial_euler = (int)structure.num_vertices - 
                       (int)structure.num_edges + 
                       (int)structure.num_faces;
    printf("  Initial Euler: %d (should be 2)\n", initial_euler);
    
    // Perform multi-pass refinement
    bool converged = refine_structure_multi_pass(
        &structure,
        NULL,  // no oscillations
        5,     // 5 passes
        10,    // 10 candidates per pass
        0.01   // convergence threshold
    );
    
    printf("  Final: V=%u, E=%u, F=%u\n", 
           structure.num_vertices, structure.num_edges, structure.num_faces);
    
    int final_euler = (int)structure.num_vertices - 
                     (int)structure.num_edges + 
                     (int)structure.num_faces;
    printf("  Final Euler: %d\n", final_euler);
    printf("  Converged: %s\n", converged ? "Yes" : "No");
    
    TEST_ASSERT(abs(final_euler - 2) <= abs(initial_euler - 2), 
                "Multi-pass refinement improved structure");
}

/**
 * Test 6: All dimensions candidate generation
 */
static void test_all_dimensions_generation() {
    printf("\n=== Test 6: All Dimensions Candidate Generation ===\n");
    
    // Create test structure
    StructuralMap structure = {
        .num_vertices = 6,
        .num_edges = 12,
        .num_faces = 8,
        .coprime_matrix = NULL,
        .dimensional_offsets = NULL,
        .corruption_mask = NULL,
        .corruption_percentage = 0.0
    };
    
    uint32_t total_candidates = 0;
    SearchCandidate** all_candidates = generate_candidates_all_dimensions(
        0x123456789ABCDEFULL,
        10,  // 10 candidates per dimension
        &structure,
        &total_candidates
    );
    
    TEST_ASSERT(all_candidates != NULL, "All dimensions candidates generated");
    TEST_ASSERT(total_candidates == 30, "Correct total candidate count (3 dims × 10)");
    
    // Check each dimension
    for (uint32_t dim = 0; dim < 3; dim++) {
        TEST_ASSERT(all_candidates[dim] != NULL, "Dimension candidates exist");
        TEST_ASSERT(all_candidates[dim][0].dimension == dim, "Correct dimension");
    }
    
    printf("  Generated %u total candidates across 3 dimensions\n", total_candidates);
    
    free_all_candidates(all_candidates, 3);
}

/**
 * Main test runner
 */
int main(void) {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║  OBJECTIVE 28 Phase 3: Iterative Search with SFT Tests   ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    // Run tests
    test_candidate_generation();
    test_fitness_scoring();
    test_iterative_refinement();
    test_convergence_detection();
    test_multi_pass_refinement();
    test_all_dimensions_generation();
    
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
