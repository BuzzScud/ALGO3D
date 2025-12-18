#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/bn.h>
#include <openssl/ec.h>

#include "ecdlp_integration.h"
#include "recursive_search.h"

// Test results structure
typedef struct {
    uint32_t bit_length;
    BIGNUM* k_original;
    BIGNUM* k_recovered;
    bool recovery_success;
    bool q_match;
    double score;
    uint32_t iterations;
    double time_seconds;
} RecoveryTestResult;

/**
 * Generate random k of specified bit length
 */
BIGNUM* generate_random_k(uint32_t bit_length) {
    BIGNUM* k = BN_new();
    if (!k) return NULL;
    
    // Generate random number with specified bit length
    if (!BN_rand(k, bit_length, BN_RAND_TOP_ANY, BN_RAND_BOTTOM_ANY)) {
        BN_free(k);
        return NULL;
    }
    
    // Ensure it's actually the right bit length (set MSB)
    BN_set_bit(k, bit_length - 1);
    
    return k;
}

/**
 * Test recovery for a specific k value
 */
RecoveryTestResult test_single_recovery(int curve_nid, BIGNUM* k_original) {
    RecoveryTestResult result = {0};
    result.k_original = BN_dup(k_original);
    result.bit_length = BN_num_bits(k_original);
    
    // Create ECDLP instance with known k
    ECDLPInstance* instance = ecdlp_create_instance_with_k(curve_nid, k_original);
    if (!instance) {
        printf("  ❌ Failed to create ECDLP instance\n");
        return result;
    }
    
    // Store original Q
    EC_POINT* Q_original = EC_POINT_dup(instance->Q, instance->group);
    
    // Create search parameters
    SearchParameters params;
    init_default_search_parameters(&params);
    params.max_depth = 5;  // Increase depth for larger k values
    params.max_iterations = 10000;  // Increase iterations for larger k values
    params.strategy = STRATEGY_HYBRID;
    
    // Start timing
    clock_t start = clock();
    
    // Run recursive recovery (this is the proper high-level API)
    SearchResult* search_result = recover_k_recursive(instance, &params);
    
    // End timing
    clock_t end = clock();
    result.time_seconds = (double)(end - start) / CLOCKS_PER_SEC;
    
    // Check recovery success
    if (search_result && search_result->recovered_k) {
        result.k_recovered = BN_dup(search_result->recovered_k);
        result.recovery_success = (BN_cmp(result.k_recovered, k_original) == 0);
        result.score = search_result->final_score;
        result.iterations = search_result->total_iterations;
        
        // Verify Q regeneration: compute Q' = k_recovered * G
        EC_POINT* Q_regenerated = EC_POINT_new(instance->group);
        BN_CTX* ctx = BN_CTX_new();
        
        EC_POINT_mul(instance->group, Q_regenerated, result.k_recovered, NULL, NULL, ctx);
        
        // Check if Q_original == Q_regenerated
        result.q_match = (EC_POINT_cmp(instance->group, Q_original, Q_regenerated, ctx) == 0);
        
        BN_CTX_free(ctx);
        EC_POINT_free(Q_regenerated);
    } else {
        result.recovery_success = false;
        result.q_match = false;
        result.score = 0.0;
        result.iterations = 0;
    }
    
    // Cleanup
    if (search_result) {
        free_search_result(search_result);
    }
    ecdlp_free_instance(instance);
    EC_POINT_free(Q_original);
    
    return result;
}

/**
 * Print test result
 */
void print_test_result(const RecoveryTestResult* result) {
    char* k_orig_hex = BN_bn2hex(result->k_original);
    
    printf("\n  Bit Length: %u bits\n", result->bit_length);
    printf("  Original k: %s\n", k_orig_hex);
    
    if (result->k_recovered) {
        char* k_rec_hex = BN_bn2hex(result->k_recovered);
        printf("  Recovered k: %s\n", k_rec_hex);
        OPENSSL_free(k_rec_hex);
    } else {
        printf("  Recovered k: NULL\n");
    }
    
    printf("  Recovery: %s\n", result->recovery_success ? "✅ SUCCESS" : "❌ FAILED");
    printf("  Q Match: %s\n", result->q_match ? "✅ MATCH" : "❌ MISMATCH");
    printf("  Score: %.6f\n", result->score);
    printf("  Iterations: %u\n", result->iterations);
    printf("  Time: %.3f seconds\n", result->time_seconds);
    
    OPENSSL_free(k_orig_hex);
}

/**
 * Free test result
 */
void free_test_result(RecoveryTestResult* result) {
    if (result->k_original) {
        BN_free(result->k_original);
        result->k_original = NULL;
    }
    if (result->k_recovered) {
        BN_free(result->k_recovered);
        result->k_recovered = NULL;
    }
}

/**
 * Test recovery at specific bit length
 */
void test_bit_length(int curve_nid, uint32_t bit_length, uint32_t num_tests) {
    printf("\n========================================\n");
    printf("Testing %u-bit k values (%u tests)\n", bit_length, num_tests);
    printf("========================================\n");
    
    uint32_t successes = 0;
    uint32_t q_matches = 0;
    double total_time = 0.0;
    double total_score = 0.0;
    
    for (uint32_t i = 0; i < num_tests; i++) {
        printf("\nTest %u/%u:\n", i + 1, num_tests);
        
        // Generate random k
        BIGNUM* k = generate_random_k(bit_length);
        if (!k) {
            printf("  ❌ Failed to generate random k\n");
            continue;
        }
        
        // Test recovery
        RecoveryTestResult result = test_single_recovery(curve_nid, k);
        print_test_result(&result);
        
        // Update statistics
        if (result.recovery_success) successes++;
        if (result.q_match) q_matches++;
        total_time += result.time_seconds;
        total_score += result.score;
        
        // Cleanup
        free_test_result(&result);
        BN_free(k);
    }
    
    // Print summary
    printf("\n========================================\n");
    printf("Summary for %u-bit k:\n", bit_length);
    printf("========================================\n");
    printf("  Tests: %u\n", num_tests);
    printf("  Successes: %u / %u (%.1f%%)\n", 
           successes, num_tests, (100.0 * successes) / num_tests);
    printf("  Q Matches: %u / %u (%.1f%%)\n",
           q_matches, num_tests, (100.0 * q_matches) / num_tests);
    printf("  Avg Time: %.3f seconds\n", total_time / num_tests);
    printf("  Avg Score: %.6f\n", total_score / num_tests);
    printf("========================================\n");
}

/**
 * Main test runner
 */
int main() {
    printf("\n");
    printf("========================================\n");
    printf("COMPREHENSIVE ECDLP RECOVERY TEST\n");
    printf("========================================\n");
    printf("\nTesting recovery across multiple bit lengths\n");
    printf("Verifying Q regeneration for each recovery\n");
    printf("\n");
    
    // Seed random number generator
    srand(time(NULL));
    
    // Test configuration
    int curve_nid = NID_secp256k1;  // Use 256-bit curve
    
    // Test bit lengths with number of tests per length
    struct {
        uint32_t bit_length;
        uint32_t num_tests;
    } test_configs[] = {
        {8, 5},      // 8-bit: 5 tests
        {16, 5},     // 16-bit: 5 tests
        {32, 5},     // 32-bit: 5 tests
        {64, 5},     // 64-bit: 5 tests
        {128, 3},    // 128-bit: 3 tests
        {192, 3},    // 192-bit: 3 tests
        {256, 3},    // 256-bit: 3 tests (full curve size)
    };
    
    uint32_t num_configs = sizeof(test_configs) / sizeof(test_configs[0]);
    
    // Run tests for each bit length
    for (uint32_t i = 0; i < num_configs; i++) {
        test_bit_length(
            curve_nid,
            test_configs[i].bit_length,
            test_configs[i].num_tests
        );
        
        printf("\n");
    }
    
    printf("\n========================================\n");
    printf("COMPREHENSIVE TEST COMPLETE\n");
    printf("========================================\n");
    
    return 0;
}