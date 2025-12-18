/**
 * @file test_lattice_entropy.c
 * @brief Unit tests for lattice entropy calculation Γ(n,d)
 */

#include "ai/cllm_lattice_entropy.h"
#include <stdio.h>
#include <stdlib.h>
#include "math/math.h"
#include <assert.h>

#define TEST_EPSILON 1e-6

/**
 * @brief Test helper: Check if two doubles are approximately equal
 */
static int doubles_equal(double a, double b, double epsilon) {
    return fabs(a - b) < epsilon;
}

/**
 * @brief Test: Entropy context initialization
 */
static void test_entropy_context_init(void) {
    printf("Testing entropy context initialization...\n");
    
    EntropyContext ctx;
    assert(entropy_context_init(&ctx));
    
    // Verify cache is initialized
    assert(ctx.cache_hits == 0);
    assert(ctx.cache_misses == 0);
    assert(ctx.total_calculations == 0);
    
    // Verify all cache entries are invalid
    for (int i = 0; i < ENTROPY_CACHE_SIZE; i++) {
        assert(!ctx.cache[i].valid);
    }
    
    entropy_context_destroy(&ctx);
    
    printf("✓ Entropy context initialization test passed\n");
}

/**
 * @brief Test: NULL context handling
 */
static void test_null_context(void) {
    printf("Testing NULL context handling...\n");
    
    assert(!entropy_context_init(NULL));
    entropy_context_destroy(NULL);
    
    printf("✓ NULL context handling test passed\n");
}

/**
 * @brief Test: Count primes in dimension 1
 */
static void test_count_primes_dimension_1(void) {
    printf("Testing prime counting in dimension 1...\n");
    
    // Test small values (exact counting)
    uint64_t count_10 = count_primes_in_dimension(10, 1);
    assert(count_10 == 4); // 2, 3, 5, 7
    
    uint64_t count_20 = count_primes_in_dimension(20, 1);
    assert(count_20 == 8); // 2, 3, 5, 7, 11, 13, 17, 19
    
    uint64_t count_30 = count_primes_in_dimension(30, 1);
    assert(count_30 == 10); // up to 29
    
    // Test edge cases
    assert(count_primes_in_dimension(0, 1) == 0);
    assert(count_primes_in_dimension(1, 1) == 0);
    assert(count_primes_in_dimension(2, 1) == 1);
    
    // Test larger values (approximation)
    uint64_t count_1000 = count_primes_in_dimension(1000, 1);
    assert(count_1000 > 100 && count_1000 < 300); // π(1000) ≈ 168, but approximation may vary
    
    printf("✓ Prime counting dimension 1 test passed\n");
}

/**
 * @brief Test: Count primes in higher dimensions
 */
static void test_count_primes_higher_dimensions(void) {
    printf("Testing prime counting in higher dimensions...\n");
    
    // Dimension 2
    uint64_t count_d2 = count_primes_in_dimension(100, 2);
    assert(count_d2 > 0);
    assert(count_d2 < count_primes_in_dimension(100, 1)); // Fewer primes in higher dims
    
    // Dimension 3
    uint64_t count_d3 = count_primes_in_dimension(1000, 3);
    assert(count_d3 > 0);
    assert(count_d3 < count_primes_in_dimension(1000, 2));
    
    // Dimension 12 (maximum) - may be 0 for small n
    count_primes_in_dimension(10000, 12); // Just verify it doesn't crash
    
    // Invalid dimension
    assert(count_primes_in_dimension(100, 0) == 0);
    assert(count_primes_in_dimension(100, 13) == 0);
    
    printf("✓ Prime counting higher dimensions test passed\n");
}

/**
 * @brief Test: Point entropy calculation
 */
static void test_point_entropy(void) {
    printf("Testing point entropy calculation...\n");
    
    // Test basic properties
    double entropy_10_1 = calculate_point_entropy(10, 1);
    assert(entropy_10_1 >= 0.0);
    assert(entropy_10_1 <= 1.0); // Binary entropy max is 1.0
    
    double entropy_100_1 = calculate_point_entropy(100, 1);
    assert(entropy_100_1 >= 0.0);
    assert(entropy_100_1 <= 1.0);
    
    // Higher dimensions
    double entropy_100_2 = calculate_point_entropy(100, 2);
    assert(entropy_100_2 >= 0.0);
    
    double entropy_100_12 = calculate_point_entropy(100, 12);
    assert(entropy_100_12 >= 0.0);
    
    // Edge cases
    assert(calculate_point_entropy(0, 1) == 0.0);
    assert(calculate_point_entropy(10, 0) == 0.0);
    assert(calculate_point_entropy(10, 13) == 0.0);
    
    printf("✓ Point entropy calculation test passed\n");
}

/**
 * @brief Test: Lattice entropy calculation
 */
static void test_lattice_entropy(void) {
    printf("Testing lattice entropy calculation Γ(n,d)...\n");
    
    // Test without context (no caching)
    double entropy_10_1 = calculate_lattice_entropy(NULL, 10, 1);
    assert(entropy_10_1 >= 0.0);
    
    double entropy_100_1 = calculate_lattice_entropy(NULL, 100, 1);
    assert(entropy_100_1 >= 0.0);
    
    // Entropy should generally increase with position
    double entropy_1000_1 = calculate_lattice_entropy(NULL, 1000, 1);
    assert(entropy_1000_1 >= entropy_100_1);
    
    // Test different dimensions
    double entropy_100_2 = calculate_lattice_entropy(NULL, 100, 2);
    assert(entropy_100_2 >= 0.0);
    
    double entropy_100_6 = calculate_lattice_entropy(NULL, 100, 6);
    assert(entropy_100_6 >= 0.0);
    
    double entropy_100_12 = calculate_lattice_entropy(NULL, 100, 12);
    assert(entropy_100_12 >= 0.0);
    
    // Test that all dimensions produce valid entropy values
    for (uint32_t d = 1; d <= 12; d++) {
        double entropy_d = calculate_lattice_entropy(NULL, 1000, d);
        assert(entropy_d >= 0.0);
        assert(!isnan(entropy_d));
        assert(!isinf(entropy_d));
    }
    
    // Edge cases
    assert(calculate_lattice_entropy(NULL, 0, 1) == 0.0);
    assert(calculate_lattice_entropy(NULL, 10, 0) == 0.0);
    assert(calculate_lattice_entropy(NULL, 10, 13) == 0.0);
    
    printf("✓ Lattice entropy calculation test passed\n");
}

/**
 * @brief Test: Entropy caching
 */
static void test_entropy_caching(void) {
    printf("Testing entropy caching...\n");
    
    EntropyContext ctx;
    assert(entropy_context_init(&ctx));
    
    // First call - cache miss
    double entropy1 = calculate_lattice_entropy_cached(&ctx, 100, 1);
    assert(ctx.cache_misses == 1);
    assert(ctx.cache_hits == 0);
    assert(ctx.total_calculations == 1);
    
    // Second call with same parameters - cache hit
    double entropy2 = calculate_lattice_entropy_cached(&ctx, 100, 1);
    assert(ctx.cache_hits == 1);
    assert(ctx.cache_misses == 1);
    assert(doubles_equal(entropy1, entropy2, TEST_EPSILON));
    
    // Different parameters - cache miss
    double entropy3 = calculate_lattice_entropy_cached(&ctx, 200, 1);
    assert(ctx.cache_misses == 2);
    assert(ctx.total_calculations == 2);
    
    // Different dimension - cache miss
    double entropy4 = calculate_lattice_entropy_cached(&ctx, 100, 2);
    assert(ctx.cache_misses == 3);
    assert(ctx.total_calculations == 3);
    
    // Repeat first call - cache hit
    double entropy5 = calculate_lattice_entropy_cached(&ctx, 100, 1);
    assert(ctx.cache_hits == 2);
    assert(doubles_equal(entropy1, entropy5, TEST_EPSILON));
    
    entropy_context_destroy(&ctx);
    
    printf("✓ Entropy caching test passed\n");
}

/**
 * @brief Test: Cache statistics
 */
static void test_cache_statistics(void) {
    printf("Testing cache statistics...\n");
    
    EntropyContext ctx;
    assert(entropy_context_init(&ctx));
    
    // Perform some calculations
    for (int i = 0; i < 10; i++) {
        calculate_lattice_entropy_cached(&ctx, 100 + i, 1);
    }
    
    // Repeat some calculations (cache hits)
    for (int i = 0; i < 5; i++) {
        calculate_lattice_entropy_cached(&ctx, 100 + i, 1);
    }
    
    // Get statistics
    double hit_rate;
    uint64_t total_calcs;
    entropy_get_cache_stats(&ctx, &hit_rate, &total_calcs);
    
    assert(total_calcs == 10); // Only 10 unique calculations
    assert(hit_rate > 0.0 && hit_rate < 1.0); // Some hits, some misses
    assert(ctx.cache_hits == 5);
    assert(ctx.cache_misses == 10);
    
    // Test NULL context
    entropy_get_cache_stats(NULL, &hit_rate, &total_calcs);
    assert(hit_rate == 0.0);
    assert(total_calcs == 0);
    
    entropy_context_destroy(&ctx);
    
    printf("✓ Cache statistics test passed\n");
}

/**
 * @brief Test: Cache clearing
 */
static void test_cache_clearing(void) {
    printf("Testing cache clearing...\n");
    
    EntropyContext ctx;
    assert(entropy_context_init(&ctx));
    
    // Fill cache
    for (int i = 0; i < 10; i++) {
        calculate_lattice_entropy_cached(&ctx, 100 + i, 1);
    }
    
    assert(ctx.cache_misses == 10);
    
    // Clear cache
    entropy_cache_clear(&ctx);
    
    assert(ctx.cache_hits == 0);
    assert(ctx.cache_misses == 0);
    
    // Verify all entries are invalid
    for (int i = 0; i < ENTROPY_CACHE_SIZE; i++) {
        assert(!ctx.cache[i].valid);
    }
    
    // Recalculate - should be cache misses again
    calculate_lattice_entropy_cached(&ctx, 100, 1);
    assert(ctx.cache_misses == 1);
    
    entropy_context_destroy(&ctx);
    
    printf("✓ Cache clearing test passed\n");
}

/**
 * @brief Test: Normalized entropy
 */
static void test_normalized_entropy(void) {
    printf("Testing normalized entropy...\n");
    
    EntropyContext ctx;
    assert(entropy_context_init(&ctx));
    
    // Test normalization to [0, 1] range
    for (uint32_t d = 1; d <= 12; d++) {
        for (uint64_t n = 10; n <= 1000; n += 100) {
            double norm_entropy = calculate_normalized_entropy(&ctx, n, d);
            assert(norm_entropy >= 0.0);
            assert(norm_entropy <= 1.0);
        }
    }
    
    // Edge cases
    assert(calculate_normalized_entropy(&ctx, 0, 1) == 0.0);
    assert(calculate_normalized_entropy(&ctx, 10, 0) == 0.0);
    assert(calculate_normalized_entropy(&ctx, 10, 13) == 0.0);
    
    // Test without context
    double norm_entropy = calculate_normalized_entropy(NULL, 100, 1);
    assert(norm_entropy >= 0.0 && norm_entropy <= 1.0);
    
    entropy_context_destroy(&ctx);
    
    printf("✓ Normalized entropy test passed\n");
}

/**
 * @brief Test: Entropy consistency
 */
static void test_entropy_consistency(void) {
    printf("Testing entropy consistency...\n");
    
    EntropyContext ctx;
    assert(entropy_context_init(&ctx));
    
    // Same parameters should give same results
    double entropy1 = calculate_lattice_entropy(&ctx, 100, 1);
    double entropy2 = calculate_lattice_entropy(&ctx, 100, 1);
    assert(doubles_equal(entropy1, entropy2, TEST_EPSILON));
    
    // Cached and non-cached should match
    double entropy_cached = calculate_lattice_entropy_cached(&ctx, 200, 2);
    double entropy_direct = calculate_lattice_entropy(NULL, 200, 2);
    assert(doubles_equal(entropy_cached, entropy_direct, TEST_EPSILON));
    
    entropy_context_destroy(&ctx);
    
    printf("✓ Entropy consistency test passed\n");
}

/**
 * @brief Main test runner
 */
int main(void) {
    printf("\n=== Lattice Entropy Tests ===\n\n");
    
    test_entropy_context_init();
    test_null_context();
    test_count_primes_dimension_1();
    test_count_primes_higher_dimensions();
    test_point_entropy();
    test_lattice_entropy();
    test_entropy_caching();
    test_cache_statistics();
    test_cache_clearing();
    test_normalized_entropy();
    test_entropy_consistency();
    
    printf("\n=== All Lattice Entropy Tests Passed ===\n\n");
    
    return 0;
}