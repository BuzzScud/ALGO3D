#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "multi_scalar_analysis.h"
#include "ecdlp_integration.h"
#include "crystal_abacus.h"

// Test counter
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) \
    printf("\nTEST: %s\n", name); \
    printf("========================================\n");

#define ASSERT_TRUE(cond, msg) \
    if (!(cond)) { \
        printf("  ❌ FAILED: %s\n", msg); \
        tests_failed++; \
        return; \
    } else { \
        printf("  ✅ PASSED: %s\n", msg); \
    }

#define ASSERT_NOT_NULL(ptr, msg) \
    ASSERT_TRUE((ptr) != NULL, msg)

#define ASSERT_EQ(a, b, msg) \
    ASSERT_TRUE((a) == (b), msg)

#define TEST_PASSED() \
    printf("  ✅ TEST PASSED\n"); \
    tests_passed++;

/**
 * Helper function to create Q embedding
 */
ECLatticeEmbedding* create_q_embedding(ECDLPInstance* instance, EC_POINT* Q) {
    ECPointCoords* coords = ec_get_point_coords(instance->group, Q);
    if (!coords) return NULL;
    
    ECLatticeEmbedding* embedding = malloc(sizeof(ECLatticeEmbedding));
    if (!embedding) {
        free(coords);
        return NULL;
    }
    
    if (!lattice_embed_ec_point(coords, embedding)) {
        free(embedding);
        free(coords);
        return NULL;
    }
    
    free(coords);
    return embedding;
}

/**
 * Test 1: Multi-Scalar Analysis Creation
 */
void test_multiscalar_creation() {
    TEST("Multi-Scalar Analysis Creation");
    
    // Create ECDLP instance
    ECDLPInstance* instance = create_ecdlp_instance(NID_secp192k1);
    ASSERT_NOT_NULL(instance, "ECDLP instance created");
    
    // Generate k and Q
    BIGNUM* k = BN_new();
    BN_set_word(k, 42);
    
    EC_POINT* Q = EC_POINT_new(instance->group);
    EC_POINT_mul(instance->group, Q, k, NULL, NULL, instance->ctx);
    
    // Embed Q to 15D lattice
    ECLatticeEmbedding* Q_embedding = create_q_embedding(instance, Q);
    ASSERT_NOT_NULL(Q_embedding, "Q embedding created");
    
    // Create default scalars
    uint32_t num_scalars;
    double* scalars = create_default_scalars(&num_scalars);
    ASSERT_NOT_NULL(scalars, "Default scalars created");
    ASSERT_EQ(num_scalars, 7, "7 scalars created");
    
    // Create multi-scalar analysis
    MultiScalarAnalysis* analysis = create_multi_scalar_analysis(
        instance, Q_embedding, scalars, num_scalars
    );
    ASSERT_NOT_NULL(analysis, "Multi-scalar analysis created");
    ASSERT_EQ(analysis->num_scalars, 7, "Correct number of scalars");
    ASSERT_EQ(analysis->num_dimensions, 15, "Correct number of dimensions");
    
    // Cleanup
    free_multi_scalar_analysis(analysis);
    free(scalars);
    free(Q_embedding);
    EC_POINT_free(Q);
    BN_free(k);
    free_ecdlp_instance(instance);
    
    TEST_PASSED();
}

/**
 * Test 2: Single Scalar Test (1.0x)
 */
void test_single_scalar() {
    TEST("Single Scalar Test (1.0x)");
    
    // Create ECDLP instance
    ECDLPInstance* instance = create_ecdlp_instance(NID_secp192k1);
    ASSERT_NOT_NULL(instance, "ECDLP instance created");
    
    // Generate k and Q
    BIGNUM* k = BN_new();
    BN_set_word(k, 42);
    
    EC_POINT* Q = EC_POINT_new(instance->group);
    EC_POINT_mul(instance->group, Q, k, NULL, NULL, instance->ctx);
    
    // Embed Q to 15D lattice
    ECLatticeEmbedding* Q_embedding = create_q_embedding(instance, Q);
    ASSERT_NOT_NULL(Q_embedding, "Q embedding created");
    
    // Create single scalar (1.0x)
    double scalars[] = {1.0};
    
    // Create multi-scalar analysis
    MultiScalarAnalysis* analysis = create_multi_scalar_analysis(
        instance, Q_embedding, scalars, 1
    );
    ASSERT_NOT_NULL(analysis, "Multi-scalar analysis created");
    
    // Test at scalar 1.0x
    bool success = test_at_scalar(analysis, 0);
    ASSERT_TRUE(success, "Test at 1.0x completed");
    
    const ScalarTestResult* result = get_scalar_result(analysis, 0);
    ASSERT_NOT_NULL(result, "Result retrieved");
    ASSERT_TRUE(result->success, "Recovery successful at 1.0x");
    ASSERT_TRUE(result->final_score > 0.95, "Score > 0.95");
    
    printf("\n  Result at 1.0x:\n");
    printf("    Success: %s\n", result->success ? "YES" : "NO");
    printf("    Score: %.6f\n", result->final_score);
    printf("    Iterations: %u\n", result->iterations);
    printf("    Time: %.3f seconds\n", result->elapsed_time);
    
    // Cleanup
    free_multi_scalar_analysis(analysis);
    free(Q_embedding);
    EC_POINT_free(Q);
    BN_free(k);
    free_ecdlp_instance(instance);
    
    TEST_PASSED();
}

/**
 * Test 3: Multiple Scalar Test
 */
void test_multiple_scalars() {
    TEST("Multiple Scalar Test");
    
    // Create ECDLP instance
    ECDLPInstance* instance = create_ecdlp_instance(NID_secp192k1);
    ASSERT_NOT_NULL(instance, "ECDLP instance created");
    
    // Generate k and Q
    BIGNUM* k = BN_new();
    BN_set_word(k, 42);
    
    EC_POINT* Q = EC_POINT_new(instance->group);
    EC_POINT_mul(instance->group, Q, k, NULL, NULL, instance->ctx);
    
    // Embed Q to 15D lattice
    ECLatticeEmbedding* Q_embedding = create_q_embedding(instance, Q);
    ASSERT_NOT_NULL(Q_embedding, "Q embedding created");
    
    // Create multiple scalars
    double scalars[] = {0.1, 0.5, 1.0, 2.0, 10.0};
    uint32_t num_scalars = 5;
    
    // Create multi-scalar analysis
    MultiScalarAnalysis* analysis = create_multi_scalar_analysis(
        instance, Q_embedding, scalars, num_scalars
    );
    ASSERT_NOT_NULL(analysis, "Multi-scalar analysis created");
    
    // Test at all scalars
    bool success = analyze_all_scalars(analysis);
    ASSERT_TRUE(success, "All scalars tested");
    
    // Compute statistics
    compute_statistics(analysis);
    
    const ScalarStatistics* stats = get_statistics(analysis);
    ASSERT_NOT_NULL(stats, "Statistics computed");
    
    printf("\n  Statistics:\n");
    printf("    Success rate: %.1f%%\n", stats->success_rate * 100.0);
    printf("    Avg iterations: %.1f\n", stats->avg_iterations);
    printf("    Avg time: %.3f seconds\n", stats->avg_time);
    printf("    Avg score: %.6f\n", stats->avg_score);
    
    // Cleanup
    free_multi_scalar_analysis(analysis);
    free(Q_embedding);
    EC_POINT_free(Q);
    BN_free(k);
    free_ecdlp_instance(instance);
    
    TEST_PASSED();
}

/**
 * Test 4: Cross-Scalar Correlation
 */
void test_cross_scalar_correlation() {
    TEST("Cross-Scalar Correlation");
    
    // Create ECDLP instance
    ECDLPInstance* instance = create_ecdlp_instance(NID_secp192k1);
    ASSERT_NOT_NULL(instance, "ECDLP instance created");
    
    // Generate k and Q
    BIGNUM* k = BN_new();
    BN_set_word(k, 42);
    
    EC_POINT* Q = EC_POINT_new(instance->group);
    EC_POINT_mul(instance->group, Q, k, NULL, NULL, instance->ctx);
    
    // Embed Q to 15D lattice
    ECLatticeEmbedding* Q_embedding = create_q_embedding(instance, Q);
    ASSERT_NOT_NULL(Q_embedding, "Q embedding created");
    
    // Create scalars
    double scalars[] = {0.5, 1.0, 2.0};
    uint32_t num_scalars = 3;
    
    // Create multi-scalar analysis
    MultiScalarAnalysis* analysis = create_multi_scalar_analysis(
        instance, Q_embedding, scalars, num_scalars
    );
    ASSERT_NOT_NULL(analysis, "Multi-scalar analysis created");
    
    // Test at all scalars
    analyze_all_scalars(analysis);
    
    // Compute cross-scalar correlation
    bool success = compute_cross_scalar_correlation(analysis);
    ASSERT_TRUE(success, "Cross-scalar correlation computed");
    
    // Check correlation matrix
    printf("\n  Correlation Matrix:\n");
    for (uint32_t i = 0; i < num_scalars; i++) {
        printf("    ");
        for (uint32_t j = 0; j < num_scalars; j++) {
            printf("%.3f ", analysis->correlation_matrix[i][j]);
        }
        printf("\n");
    }
    
    // Diagonal should be 1.0
    for (uint32_t i = 0; i < num_scalars; i++) {
        ASSERT_TRUE(analysis->correlation_matrix[i][i] > 0.99,
                   "Diagonal correlation = 1.0");
    }
    
    // Cleanup
    free_multi_scalar_analysis(analysis);
    free(Q_embedding);
    EC_POINT_free(Q);
    BN_free(k);
    free_ecdlp_instance(instance);
    
    TEST_PASSED();
}

/**
 * Test 5: Variance Analysis
 */
void test_variance_analysis() {
    TEST("Variance Analysis");
    
    // Create ECDLP instance
    ECDLPInstance* instance = create_ecdlp_instance(NID_secp192k1);
    ASSERT_NOT_NULL(instance, "ECDLP instance created");
    
    // Generate k and Q
    BIGNUM* k = BN_new();
    BN_set_word(k, 42);
    
    EC_POINT* Q = EC_POINT_new(instance->group);
    EC_POINT_mul(instance->group, Q, k, NULL, NULL, instance->ctx);
    
    // Embed Q to 15D lattice
    ECLatticeEmbedding* Q_embedding = create_q_embedding(instance, Q);
    ASSERT_NOT_NULL(Q_embedding, "Q embedding created");
    
    // Create scalars
    double scalars[] = {0.5, 1.0, 2.0};
    uint32_t num_scalars = 3;
    
    // Create multi-scalar analysis
    MultiScalarAnalysis* analysis = create_multi_scalar_analysis(
        instance, Q_embedding, scalars, num_scalars
    );
    ASSERT_NOT_NULL(analysis, "Multi-scalar analysis created");
    
    // Test at all scalars
    analyze_all_scalars(analysis);
    
    // Compute variance analysis
    bool success = compute_variance_analysis(analysis);
    ASSERT_TRUE(success, "Variance analysis computed");
    
    printf("\n  Variance per scalar:\n");
    for (uint32_t i = 0; i < num_scalars; i++) {
        printf("    %.2fx: %.6f\n", scalars[i], analysis->variance_per_scalar[i]);
    }
    
    printf("\n  Variance per dimension (first 5):\n");
    for (uint32_t i = 0; i < 5; i++) {
        printf("    Dim %u: %.6f\n", i, analysis->variance_per_dimension[i]);
    }
    
    // Cleanup
    free_multi_scalar_analysis(analysis);
    free(Q_embedding);
    EC_POINT_free(Q);
    BN_free(k);
    free_ecdlp_instance(instance);
    
    TEST_PASSED();
}

/**
 * Test 6: Stability Verification
 */
void test_stability_verification() {
    TEST("Stability Verification");
    
    // Create ECDLP instance
    ECDLPInstance* instance = create_ecdlp_instance(NID_secp192k1);
    ASSERT_NOT_NULL(instance, "ECDLP instance created");
    
    // Generate k and Q
    BIGNUM* k = BN_new();
    BN_set_word(k, 42);
    
    EC_POINT* Q = EC_POINT_new(instance->group);
    EC_POINT_mul(instance->group, Q, k, NULL, NULL, instance->ctx);
    
    // Embed Q to 15D lattice
    ECLatticeEmbedding* Q_embedding = create_q_embedding(instance, Q);
    ASSERT_NOT_NULL(Q_embedding, "Q embedding created");
    
    // Create scalars
    double scalars[] = {0.5, 1.0, 2.0};
    uint32_t num_scalars = 3;
    
    // Create multi-scalar analysis
    MultiScalarAnalysis* analysis = create_multi_scalar_analysis(
        instance, Q_embedding, scalars, num_scalars
    );
    ASSERT_NOT_NULL(analysis, "Multi-scalar analysis created");
    
    // Test at all scalars
    analyze_all_scalars(analysis);
    
    // Compute statistics
    compute_statistics(analysis);
    
    // Verify stability
    bool stable = verify_stability(analysis);
    printf("\n  Stability: %s\n", stable ? "STABLE" : "UNSTABLE");
    printf("  Success rate: %.1f%%\n", analysis->statistics.success_rate * 100.0);
    printf("  Avg variance: %.6f\n", analysis->statistics.avg_variance);
    
    // Cleanup
    free_multi_scalar_analysis(analysis);
    free(Q_embedding);
    EC_POINT_free(Q);
    BN_free(k);
    free_ecdlp_instance(instance);
    
    TEST_PASSED();
}

/**
 * Test 7: Small k Recovery at Multiple Scalars
 */
void test_small_k_multiscalar() {
    TEST("Small k Recovery at Multiple Scalars (k=42)");
    
    // Create ECDLP instance
    ECDLPInstance* instance = create_ecdlp_instance(NID_secp192k1);
    ASSERT_NOT_NULL(instance, "ECDLP instance created");
    
    // Generate k and Q
    BIGNUM* k = BN_new();
    BN_set_word(k, 42);
    
    EC_POINT* Q = EC_POINT_new(instance->group);
    EC_POINT_mul(instance->group, Q, k, NULL, NULL, instance->ctx);
    
    // Embed Q to 15D lattice
    ECLatticeEmbedding* Q_embedding = create_q_embedding(instance, Q);
    ASSERT_NOT_NULL(Q_embedding, "Q embedding created");
    
    // Create default scalars
    uint32_t num_scalars;
    double* scalars = create_default_scalars(&num_scalars);
    ASSERT_NOT_NULL(scalars, "Default scalars created");
    
    // Create multi-scalar analysis
    MultiScalarAnalysis* analysis = create_multi_scalar_analysis(
        instance, Q_embedding, scalars, num_scalars
    );
    ASSERT_NOT_NULL(analysis, "Multi-scalar analysis created");
    
    // Test at all scalars
    bool success = analyze_all_scalars(analysis);
    ASSERT_TRUE(success, "All scalars tested");
    
    // Compute statistics
    compute_statistics(analysis);
    compute_cross_scalar_correlation(analysis);
    compute_variance_analysis(analysis);
    
    // Print results
    print_multi_scalar_results(analysis);
    
    // Check success rate
    const ScalarStatistics* stats = get_statistics(analysis);
    printf("\n  Final success rate: %.1f%%\n", stats->success_rate * 100.0);
    
    // Cleanup
    free_multi_scalar_analysis(analysis);
    free(scalars);
    free(Q_embedding);
    EC_POINT_free(Q);
    BN_free(k);
    free_ecdlp_instance(instance);
    
    TEST_PASSED();
}

/**
 * Main test runner
 */
int main() {
    printf("\n");
    printf("========================================\n");
    printf("PHASE 6: MULTI-SCALAR ANALYSIS TESTS\n");
    printf("========================================\n");
    
    // Run tests
    test_multiscalar_creation();
    test_single_scalar();
    test_multiple_scalars();
    test_cross_scalar_correlation();
    test_variance_analysis();
    test_stability_verification();
    test_small_k_multiscalar();
    
    // Print summary
    printf("\n");
    printf("========================================\n");
    printf("TEST SUMMARY\n");
    printf("========================================\n");
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_failed);
    printf("Total tests: %d\n", tests_passed + tests_failed);
    printf("Success rate: %.1f%%\n",
           (100.0 * tests_passed) / (tests_passed + tests_failed));
    printf("========================================\n");
    
    return (tests_failed == 0) ? 0 : 1;
}