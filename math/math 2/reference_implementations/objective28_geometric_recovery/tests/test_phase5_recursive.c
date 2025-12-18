/**
 * @file test_phase5_recursive.c
 * @brief Test Phase 5: Recursive Search
 * 
 * Tests oscillation-guided recursive search
 */

#include "../include/recursive_search.h"
#include "../include/ecdlp_integration.h"
#include "../include/oscillation_detection.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"
#include <stdio.h>
#include <stdlib.h>

// Test colors
#define GREEN "\033[0;32m"
#define RED "\033[0;31m"
#define YELLOW "\033[0;33m"
#define RESET "\033[0m"

void print_test_header(const char* test_name) {
    printf("\n");
    printf("========================================\n");
    printf("TEST: %s\n", test_name);
    printf("========================================\n");
}

void print_test_result(const char* test_name, bool passed) {
    if (passed) {
        printf("%s[PASS]%s %s\n", GREEN, RESET, test_name);
    } else {
        printf("%s[FAIL]%s %s\n", RED, RESET, test_name);
    }
}

// ============================================================================
// TEST 1: Search State Creation
// ============================================================================

bool test_search_state_creation() {
    print_test_header("Search State Creation");
    
    SearchParameters params;
    init_default_search_parameters(&params);
    
    SearchState* state = create_search_state(&params);
    
    if (!state) {
        printf("  Failed to create search state\n");
        return false;
    }
    
    printf("  Initial depth: %u\n", state->depth);
    printf("  Max depth: %u\n", state->max_depth);
    printf("  Max iterations: %lu\n", state->max_iterations);
    printf("  Convergence threshold: %.6f\n", state->convergence_threshold);
    
    bool success = (state->depth == params.initial_depth) &&
                   (state->max_depth == params.max_depth);
    
    free_search_state(state);
    
    return success;
}

// ============================================================================
// TEST 2: Default Parameters
// ============================================================================

bool test_default_parameters() {
    print_test_header("Default Parameters");
    
    SearchParameters params;
    init_default_search_parameters(&params);
    
    print_search_parameters(&params);
    
    bool success = (params.strategy == STRATEGY_HYBRID) &&
                   (params.max_depth > 0) &&
                   (params.max_iterations > 0);
    
    return success;
}

// ============================================================================
// TEST 3: Oscillation-Guided Candidate Generation
// ============================================================================

bool test_oscillation_guided_candidates() {
    print_test_header("Oscillation-Guided Candidate Generation");
    
    // Create ECDLP instance
    ECDLPInstance* instance = ecdlp_create_instance(NID_secp192k1);
    if (!instance) {
        printf("  Failed to create ECDLP instance\n");
        return false;
    }
    
    // Sample trajectory
    ECTrajectory* trajectory = sample_ec_trajectory(instance, 1, 1, 64);
    if (!trajectory) {
        printf("  Failed to sample trajectory\n");
        ecdlp_free_instance(instance);
        return false;
    }
    
    // Detect oscillations
    OscillationMap* osc_map = detect_oscillations(trajectory);
    free_ec_trajectory(trajectory);
    
    if (!osc_map) {
        printf("  Failed to detect oscillations\n");
        ecdlp_free_instance(instance);
        return false;
    }
    
    // Get Q embedding
    ECPointCoords* Q_coords = ec_get_point_coords(instance->group, instance->Q);
    if (!Q_coords) {
        free_oscillation_map(osc_map);
        ecdlp_free_instance(instance);
        return false;
    }
    
    ECLatticeEmbedding Q_embedding;
    if (!lattice_embed_ec_point(Q_coords, &Q_embedding)) {
        ec_free_point_coords(Q_coords);
        free_oscillation_map(osc_map);
        ecdlp_free_instance(instance);
        return false;
    }
    ec_free_point_coords(Q_coords);
    
    // Generate candidates
    BIGNUM* candidates[50];
    BIGNUM* current_k = BN_new();
    BN_set_word(current_k, 10);
    
    int num_candidates = generate_oscillation_guided_candidates(
        instance, osc_map, &Q_embedding, current_k,
        candidates, 50
    );
    
    printf("  Generated %d candidates\n", num_candidates);
    
    if (num_candidates > 0) {
        printf("  First 3 candidates:\n");
        for (int i = 0; i < 3 && i < num_candidates; i++) {
            printf("    [%d] %s\n", i, BN_bn2hex(candidates[i]));
        }
    }
    
    bool success = (num_candidates > 0);
    
    // Cleanup
    for (int i = 0; i < num_candidates; i++) {
        BN_free(candidates[i]);
    }
    BN_free(current_k);
    free_oscillation_map(osc_map);
    ecdlp_free_instance(instance);
    
    return success;
}

// ============================================================================
// TEST 4: Candidate Scoring
// ============================================================================

bool test_candidate_scoring() {
    print_test_header("Candidate Scoring");
    
    // Create ECDLP instance with known k
    BIGNUM* known_k = BN_new();
    BN_set_word(known_k, 42);
    
    ECDLPInstance* instance = ecdlp_create_instance_with_k(NID_secp192k1, known_k);
    if (!instance) {
        BN_free(known_k);
        return false;
    }
    
    // Get Q embedding
    ECPointCoords* Q_coords = ec_get_point_coords(instance->group, instance->Q);
    if (!Q_coords) {
        BN_free(known_k);
        ecdlp_free_instance(instance);
        return false;
    }
    
    ECLatticeEmbedding Q_embedding;
    if (!lattice_embed_ec_point(Q_coords, &Q_embedding)) {
        ec_free_point_coords(Q_coords);
        BN_free(known_k);
        ecdlp_free_instance(instance);
        return false;
    }
    ec_free_point_coords(Q_coords);
    
    // Score the correct k
    SearchParameters params;
    init_default_search_parameters(&params);
    
    double correct_score = score_by_lattice_distance(instance, known_k, &Q_embedding);
    printf("  Score for correct k (42): %.6f\n", correct_score);
    
    // Score a wrong k
    BIGNUM* wrong_k = BN_new();
    BN_set_word(wrong_k, 100);
    
    double wrong_score = score_by_lattice_distance(instance, wrong_k, &Q_embedding);
    printf("  Score for wrong k (100): %.6f\n", wrong_score);
    
    bool success = (correct_score > wrong_score);
    
    BN_free(wrong_k);
    BN_free(known_k);
    ecdlp_free_instance(instance);
    
    return success;
}

// ============================================================================
// TEST 5: Small k Recovery
// ============================================================================

bool test_small_k_recovery() {
    print_test_header("Small k Recovery with Recursive Search");
    
    // Create instance with small k
    BIGNUM* small_k = BN_new();
    BN_set_word(small_k, 42);
    
    ECDLPInstance* instance = ecdlp_create_instance_with_k(NID_secp192k1, small_k);
    if (!instance) {
        BN_free(small_k);
        return false;
    }
    
    printf("  Target k: %s\n", BN_bn2hex(instance->k));
    
    // Setup search parameters
    SearchParameters params;
    init_default_search_parameters(&params);
    params.max_depth = 3;
    params.max_iterations = 1000;
    params.strategy = STRATEGY_HYBRID;
    
    // Perform recovery
    SearchResult* result = recover_k_recursive(instance, &params);
    
    if (!result) {
        printf("  %sRecovery failed (NULL result)%s\n", RED, RESET);
        BN_free(small_k);
        ecdlp_free_instance(instance);
        return false;
    }
    
    print_search_result(result);
    
    bool success = result->success;
    
    free_search_result(result);
    BN_free(small_k);
    ecdlp_free_instance(instance);
    
    return success;
}

// ============================================================================
// MAIN
// ============================================================================

int main() {
    printf("\n");
    printf("╔════════════════════════════════════════╗\n");
    printf("║  PHASE 5: RECURSIVE SEARCH TESTS      ║\n");
    printf("╔════════════════════════════════════════╗\n");
    printf("\n");
    
    int total_tests = 0;
    int passed_tests = 0;
    
    // Run tests
    bool result;
    
    result = test_search_state_creation();
    print_test_result("Search State Creation", result);
    total_tests++; if (result) passed_tests++;
    
    result = test_default_parameters();
    print_test_result("Default Parameters", result);
    total_tests++; if (result) passed_tests++;
    
    result = test_oscillation_guided_candidates();
    print_test_result("Oscillation-Guided Candidates", result);
    total_tests++; if (result) passed_tests++;
    
    result = test_candidate_scoring();
    print_test_result("Candidate Scoring", result);
    total_tests++; if (result) passed_tests++;
    
    printf("\n%s=== LONG-RUNNING TEST ===%s\n", YELLOW, RESET);
    result = test_small_k_recovery();
    print_test_result("Small k Recovery", result);
    total_tests++; if (result) passed_tests++;
    
    // Summary
    printf("\n");
    printf("========================================\n");
    printf("SUMMARY\n");
    printf("========================================\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed: %s%d%s\n", GREEN, passed_tests, RESET);
    printf("Failed: %s%d%s\n", RED, total_tests - passed_tests, RESET);
    printf("Success rate: %.1f%%\n", 100.0 * passed_tests / total_tests);
    printf("\n");
    
    return (passed_tests == total_tests) ? 0 : 1;
}