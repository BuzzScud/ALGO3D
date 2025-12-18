/**
 * @file test_phase3_ecdlp.c
 * @brief Test Phase 3: ECDLP Integration
 * 
 * Tests real elliptic curve operations with geometric recovery
 */

#include "../include/ecdlp_integration.h"
#include "../include/crystal_abacus.h"
#include "../include/tetration_attractors.h"
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
// TEST 1: ECDLP Instance Creation
// ============================================================================

bool test_ecdlp_instance_creation() {
    print_test_header("ECDLP Instance Creation");
    
    // Test secp192k1
    ECDLPInstance* instance = ecdlp_create_instance(NID_secp192k1);
    if (!instance) {
        printf("  Failed to create secp192k1 instance\n");
        return false;
    }
    
    printf("  Created secp192k1 instance\n");
    printf("  Curve: %s\n", instance->curve_name);
    printf("  Order bits: %d\n", BN_num_bits(instance->order));
    printf("  k bits: %d\n", BN_num_bits(instance->k));
    
    // Verify Q = k*G
    bool verified = ecdlp_verify_solution(instance, instance->k);
    printf("  Verification: %s\n", verified ? "PASS" : "FAIL");
    
    ecdlp_free_instance(instance);
    
    return verified;
}

// ============================================================================
// TEST 2: EC Point Coordinates
// ============================================================================

bool test_ec_point_coords() {
    print_test_header("EC Point Coordinates");
    
    ECDLPInstance* instance = ecdlp_create_instance(NID_secp192k1);
    if (!instance) return false;
    
    // Get Q coordinates
    ECPointCoords* coords = ec_get_point_coords(instance->group, instance->Q);
    if (!coords) {
        printf("  Failed to get coordinates\n");
        ecdlp_free_instance(instance);
        return false;
    }
    
    printf("  Q coordinates:\n");
    printf("    x: %s\n", BN_bn2hex(coords->x));
    printf("    y: %s\n", BN_bn2hex(coords->y));
    printf("    is_infinity: %s\n", coords->is_infinity ? "true" : "false");
    
    bool success = !coords->is_infinity && coords->x && coords->y;
    
    ec_free_point_coords(coords);
    ecdlp_free_instance(instance);
    
    return success;
}

// ============================================================================
// TEST 3: Lattice Embedding
// ============================================================================

bool test_lattice_embedding() {
    print_test_header("Lattice Embedding");
    
    ECDLPInstance* instance = ecdlp_create_instance(NID_secp192k1);
    if (!instance) return false;
    
    // Get Q coordinates
    ECPointCoords* coords = ec_get_point_coords(instance->group, instance->Q);
    if (!coords) {
        ecdlp_free_instance(instance);
        return false;
    }
    
    // Embed into lattice
    ECLatticeEmbedding embedding;
    bool success = lattice_embed_ec_point(coords, &embedding);
    
    if (success) {
        printf("  Lattice embedding successful\n");
        printf("  Magnitude: %.6f\n", embedding.magnitude);
        printf("  Angle: %.6f rad\n", embedding.angle);
        printf("  First 5 coordinates:\n");
        for (int i = 0; i < 5; i++) {
            printf("    [%d] p=%lu: %.6f\n", i, embedding.primes[i], embedding.coords[i]);
        }
    } else {
        printf("  Lattice embedding failed\n");
    }
    
    ec_free_point_coords(coords);
    ecdlp_free_instance(instance);
    
    return success;
}

// ============================================================================
// TEST 4: Lattice Distance
// ============================================================================

bool test_lattice_distance() {
    print_test_header("Lattice Distance");
    
    ECDLPInstance* instance = ecdlp_create_instance(NID_secp192k1);
    if (!instance) return false;
    
    // Get Q embedding
    ECPointCoords* Q_coords = ec_get_point_coords(instance->group, instance->Q);
    if (!Q_coords) {
        ecdlp_free_instance(instance);
        return false;
    }
    
    ECLatticeEmbedding Q_embedding;
    if (!lattice_embed_ec_point(Q_coords, &Q_embedding)) {
        ec_free_point_coords(Q_coords);
        ecdlp_free_instance(instance);
        return false;
    }
    ec_free_point_coords(Q_coords);
    
    // Get G embedding
    ECPointCoords* G_coords = ec_get_point_coords(instance->group, instance->G);
    if (!G_coords) {
        ecdlp_free_instance(instance);
        return false;
    }
    
    ECLatticeEmbedding G_embedding;
    if (!lattice_embed_ec_point(G_coords, &G_embedding)) {
        ec_free_point_coords(G_coords);
        ecdlp_free_instance(instance);
        return false;
    }
    ec_free_point_coords(G_coords);
    
    // Compute distance
    double distance = lattice_distance(&Q_embedding, &G_embedding);
    printf("  Distance(Q, G) = %.6f\n", distance);
    
    // Compute angle
    double angle = lattice_angle(&Q_embedding, &G_embedding);
    printf("  Angle(Q, G) = %.6f rad\n", angle);
    
    bool success = (distance > 0.0) && (angle >= 0.0);
    
    ecdlp_free_instance(instance);
    
    return success;
}

// ============================================================================
// TEST 5: K Candidate Generation
// ============================================================================

bool test_k_candidate_generation() {
    print_test_header("K Candidate Generation");
    
    ECDLPInstance* instance = ecdlp_create_instance(NID_secp192k1);
    if (!instance) return false;
    
    // Get Q embedding
    ECPointCoords* Q_coords = ec_get_point_coords(instance->group, instance->Q);
    if (!Q_coords) {
        ecdlp_free_instance(instance);
        return false;
    }
    
    ECLatticeEmbedding Q_embedding;
    if (!lattice_embed_ec_point(Q_coords, &Q_embedding)) {
        ec_free_point_coords(Q_coords);
        ecdlp_free_instance(instance);
        return false;
    }
    ec_free_point_coords(Q_coords);
    
    // Generate candidates
    BIGNUM* candidates[100];
    int num_candidates = generate_k_candidates_from_attractors(
        instance, &Q_embedding, candidates, 100
    );
    
    printf("  Generated %d candidates\n", num_candidates);
    
    if (num_candidates > 0) {
        printf("  First 5 candidates:\n");
        for (int i = 0; i < 5 && i < num_candidates; i++) {
            printf("    [%d] %s\n", i, BN_bn2hex(candidates[i]));
        }
    }
    
    // Free candidates
    for (int i = 0; i < num_candidates; i++) {
        BN_free(candidates[i]);
    }
    
    ecdlp_free_instance(instance);
    
    return num_candidates > 0;
}

// ============================================================================
// TEST 6: Small k Recovery
// ============================================================================

bool test_small_k_recovery() {
    print_test_header("Small k Recovery");
    
    // Create instance with small k
    BIGNUM* small_k = BN_new();
    BN_set_word(small_k, 42);  // k = 42
    
    ECDLPInstance* instance = ecdlp_create_instance_with_k(NID_secp192k1, small_k);
    BN_free(small_k);
    
    if (!instance) return false;
    
    printf("  Target k: %s\n", BN_bn2hex(instance->k));
    
    // Get Q embedding
    ECPointCoords* Q_coords = ec_get_point_coords(instance->group, instance->Q);
    if (!Q_coords) {
        ecdlp_free_instance(instance);
        return false;
    }
    
    ECLatticeEmbedding Q_embedding;
    if (!lattice_embed_ec_point(Q_coords, &Q_embedding)) {
        ec_free_point_coords(Q_coords);
        ecdlp_free_instance(instance);
        return false;
    }
    ec_free_point_coords(Q_coords);
    
    // Generate candidates
    BIGNUM* candidates[1000];
    int num_candidates = generate_k_candidates_from_attractors(
        instance, &Q_embedding, candidates, 1000
    );
    
    printf("  Generated %d candidates\n", num_candidates);
    
    // Test each candidate
    bool found = false;
    for (int i = 0; i < num_candidates; i++) {
        if (ecdlp_verify_solution(instance, candidates[i])) {
            printf("  %sFOUND k at candidate %d!%s\n", GREEN, i, RESET);
            printf("  Recovered k: %s\n", BN_bn2hex(candidates[i]));
            found = true;
            break;
        }
    }
    
    if (!found) {
        printf("  %sFailed to recover k%s\n", RED, RESET);
    }
    
    // Free candidates
    for (int i = 0; i < num_candidates; i++) {
        BN_free(candidates[i]);
    }
    
    ecdlp_free_instance(instance);
    
    return found;
}

// ============================================================================
// TEST 7: Multiple Instance Recovery
// ============================================================================

bool test_multiple_instance_recovery() {
    print_test_header("Multiple Instance Recovery");
    
    int num_tests = 10;
    int successes = test_ecdlp_recovery_toy(num_tests);
    
    double success_rate = 100.0 * successes / num_tests;
    printf("\n  Success rate: %.1f%% (%d/%d)\n", success_rate, successes, num_tests);
    
    return successes > 0;
}

// ============================================================================
// MAIN
// ============================================================================

int main() {
    printf("\n");
    printf("╔════════════════════════════════════════╗\n");
    printf("║  PHASE 3: ECDLP INTEGRATION TESTS     ║\n");
    printf("╔════════════════════════════════════════╗\n");
    printf("\n");
    
    int total_tests = 0;
    int passed_tests = 0;
    
    // Run tests
    bool result;
    
    result = test_ecdlp_instance_creation();
    print_test_result("ECDLP Instance Creation", result);
    total_tests++; if (result) passed_tests++;
    
    result = test_ec_point_coords();
    print_test_result("EC Point Coordinates", result);
    total_tests++; if (result) passed_tests++;
    
    result = test_lattice_embedding();
    print_test_result("Lattice Embedding", result);
    total_tests++; if (result) passed_tests++;
    
    result = test_lattice_distance();
    print_test_result("Lattice Distance", result);
    total_tests++; if (result) passed_tests++;
    
    result = test_k_candidate_generation();
    print_test_result("K Candidate Generation", result);
    total_tests++; if (result) passed_tests++;
    
    result = test_small_k_recovery();
    print_test_result("Small k Recovery", result);
    total_tests++; if (result) passed_tests++;
    
    result = test_multiple_instance_recovery();
    print_test_result("Multiple Instance Recovery", result);
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