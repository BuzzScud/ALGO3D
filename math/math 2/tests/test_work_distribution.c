/**
 * Unit Tests for Plimpton 322 Work Distribution
 * 
 * Tests work distribution calculation, validation, and splitting
 * based on Plimpton 322 ratios.
 */

#include "ai/cllm_work_distribution.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "math/math.h"

// Test counter
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) \
    printf("Testing " #name "...\n"); \
    if (test_##name()) { \
        printf("  ✓ PASSED\n"); \
        tests_passed++; \
    } else { \
        printf("  ✗ FAILED\n"); \
        tests_failed++; \
    }

#define MATH_EPSILON 0.0001

// ============================================================================
// BASIC DISTRIBUTION TESTS
// ============================================================================

int test_calculate_distribution() {
    // Test with known Plimpton pair (3,2)
    WorkDistribution dist = calculate_work_distribution(3, 2);
    
    assert(dist.parent_id == 3);
    assert(dist.child_id == 2);
    assert(dist.is_valid == true);
    
    // Check ratios are in valid range
    assert(dist.parent_keeps >= 0.0 && dist.parent_keeps <= 1.0);
    assert(dist.child_gets >= 0.0 && dist.child_gets <= 1.0);
    
    // Check ratios sum to approximately 1.0
    double sum = dist.parent_keeps + dist.child_gets;
    assert(fabs(sum - 1.0) < MATH_EPSILON);
    
    return 1;
}

int test_validated_distribution() {
    WorkDistribution dist;
    
    // Valid relationship
    bool valid = calculate_work_distribution_validated(3, 2, &dist);
    assert(valid == true);
    assert(dist.is_valid == true);
    
    // Invalid relationship (both odd)
    valid = calculate_work_distribution_validated(5, 3, &dist);
    assert(valid == false);
    assert(dist.is_valid == false);
    
    return 1;
}

int test_known_plimpton_distributions() {
    // Test first 6 known Plimpton 322 pairs
    
    // (2,1)
    WorkDistribution dist = calculate_work_distribution(2, 1);
    assert(dist.is_valid == true);
    assert(fabs(get_ratio_sum(&dist) - 1.0) < MATH_EPSILON);
    
    // (3,2)
    dist = calculate_work_distribution(3, 2);
    assert(dist.is_valid == true);
    assert(fabs(get_ratio_sum(&dist) - 1.0) < MATH_EPSILON);
    
    // (4,1)
    dist = calculate_work_distribution(4, 1);
    assert(dist.is_valid == true);
    assert(fabs(get_ratio_sum(&dist) - 1.0) < MATH_EPSILON);
    
    // (4,3)
    dist = calculate_work_distribution(4, 3);
    assert(dist.is_valid == true);
    assert(fabs(get_ratio_sum(&dist) - 1.0) < MATH_EPSILON);
    
    // (5,2)
    dist = calculate_work_distribution(5, 2);
    assert(dist.is_valid == true);
    assert(fabs(get_ratio_sum(&dist) - 1.0) < MATH_EPSILON);
    
    // (5,4)
    dist = calculate_work_distribution(5, 4);
    assert(dist.is_valid == true);
    assert(fabs(get_ratio_sum(&dist) - 1.0) < MATH_EPSILON);
    
    return 1;
}

// ============================================================================
// VALIDATION TESTS
// ============================================================================

int test_ratio_sum_validation() {
    // Valid sum
    assert(ratios_sum_to_one(0.6, 0.4, MATH_EPSILON) == true);
    assert(ratios_sum_to_one(0.5, 0.5, MATH_EPSILON) == true);
    assert(ratios_sum_to_one(0.7, 0.3, MATH_EPSILON) == true);
    
    // Invalid sum
    assert(ratios_sum_to_one(0.6, 0.5, MATH_EPSILON) == false);
    assert(ratios_sum_to_one(0.3, 0.3, MATH_EPSILON) == false);
    
    // Edge cases
    assert(ratios_sum_to_one(1.0, 0.0, MATH_EPSILON) == true);
    assert(ratios_sum_to_one(0.0, 1.0, MATH_EPSILON) == true);
    
    return 1;
}

int test_distribution_validation() {
    WorkDistribution dist;
    
    // Valid distribution
    dist = calculate_work_distribution(3, 2);
    assert(validate_work_distribution(&dist, MATH_EPSILON) == true);
    
    // Invalid distribution (ratios don't sum to 1)
    dist.parent_keeps = 0.6;
    dist.child_gets = 0.5;
    dist.is_valid = false;
    assert(validate_work_distribution(&dist, MATH_EPSILON) == false);
    
    // Invalid distribution (negative ratio)
    dist.parent_keeps = -0.1;
    dist.child_gets = 1.1;
    assert(validate_work_distribution(&dist, MATH_EPSILON) == false);
    
    // Invalid distribution (ratio > 1)
    dist.parent_keeps = 1.5;
    dist.child_gets = -0.5;
    assert(validate_work_distribution(&dist, MATH_EPSILON) == false);
    
    return 1;
}

// ============================================================================
// WORK SPLITTING TESTS
// ============================================================================

int test_work_amount_calculation() {
    // Test various ratios
    assert(calculate_work_amount(100, 0.5) == 50);
    assert(calculate_work_amount(100, 0.25) == 25);
    assert(calculate_work_amount(100, 0.75) == 75);
    assert(calculate_work_amount(100, 1.0) == 100);
    assert(calculate_work_amount(100, 0.0) == 0);
    
    // Test with large numbers
    assert(calculate_work_amount(1000000, 0.5) == 500000);
    
    // Test edge cases
    assert(calculate_work_amount(100, -0.1) == 0);  // Negative clamped to 0
    assert(calculate_work_amount(100, 1.5) == 100); // >1 clamped to 1
    
    return 1;
}

int test_split_work() {
    WorkDistribution dist = calculate_work_distribution(3, 2);
    uint64_t parent_work, child_work;
    
    // Split 1000 units of work
    bool success = split_work(1000, &dist, &parent_work, &child_work);
    assert(success == true);
    
    // Check total is preserved
    assert(parent_work + child_work == 1000);
    
    // Check ratios are approximately correct
    double parent_ratio = (double)parent_work / 1000.0;
    double child_ratio = (double)child_work / 1000.0;
    assert(fabs(parent_ratio - dist.parent_keeps) < 0.01);
    assert(fabs(child_ratio - dist.child_gets) < 0.01);
    
    return 1;
}

int test_split_work_edge_cases() {
    WorkDistribution dist = calculate_work_distribution(3, 2);
    uint64_t parent_work, child_work;
    
    // Split 0 work
    bool success = split_work(0, &dist, &parent_work, &child_work);
    assert(success == true);
    assert(parent_work == 0);
    assert(child_work == 0);
    
    // Split 1 unit of work
    success = split_work(1, &dist, &parent_work, &child_work);
    assert(success == true);
    assert(parent_work + child_work == 1);
    
    // Split large amount
    success = split_work(1000000, &dist, &parent_work, &child_work);
    assert(success == true);
    assert(parent_work + child_work == 1000000);
    
    return 1;
}

// ============================================================================
// MULTI-CHILD DISTRIBUTION TESTS
// ============================================================================

int test_multi_child_distribution() {
    uint64_t child_ids[] = {1, 3, 7, 9};
    size_t num_children = 4;
    
    MultiChildDistribution dist = calculate_multi_child_distribution(
        10, child_ids, num_children
    );
    
    assert(dist.parent_id == 10);
    assert(dist.num_children == num_children);
    assert(dist.child_ids != NULL);
    assert(dist.child_ratios != NULL);
    
    // Check all ratios are in valid range
    assert(dist.parent_keeps >= 0.0 && dist.parent_keeps <= 1.0);
    for (size_t i = 0; i < num_children; i++) {
        assert(dist.child_ratios[i] >= 0.0 && dist.child_ratios[i] <= 1.0);
    }
    
    // Check ratios sum to approximately 1.0
    double sum = get_multi_child_ratio_sum(&dist);
    assert(fabs(sum - 1.0) < MATH_EPSILON);
    
    // Cleanup
    free(dist.child_ids);
    free(dist.child_ratios);
    
    return 1;
}

int test_multi_child_validation() {
    uint64_t child_ids[] = {1, 3, 7};
    MultiChildDistribution dist = calculate_multi_child_distribution(
        10, child_ids, 3
    );
    
    assert(validate_multi_child_distribution(&dist, MATH_EPSILON) == true);
    
    // Cleanup
    free(dist.child_ids);
    free(dist.child_ratios);
    
    return 1;
}

int test_split_work_multi_child() {
    uint64_t child_ids[] = {1, 3, 7, 9};
    size_t num_children = 4;
    
    MultiChildDistribution dist = calculate_multi_child_distribution(
        10, child_ids, num_children
    );
    
    uint64_t parent_work;
    uint64_t child_work[4];
    
    // Split 1000 units of work
    bool success = split_work_multi_child(
        1000, &dist, &parent_work, child_work
    );
    assert(success == true);
    
    // Check total is preserved
    uint64_t total = parent_work;
    for (size_t i = 0; i < num_children; i++) {
        total += child_work[i];
    }
    assert(total == 1000);
    
    // Cleanup
    free(dist.child_ids);
    free(dist.child_ratios);
    
    return 1;
}

int test_twelve_fold_distribution() {
    // Generate 12 children for parent 100
    uint64_t child_ids[12];
    for (size_t i = 0; i < 12; i++) {
        child_ids[i] = i + 1;
    }
    
    MultiChildDistribution dist = calculate_multi_child_distribution(
        100, child_ids, 12
    );
    
    assert(dist.num_children == 12);
    
    // Check ratios sum to approximately 1.0
    double sum = get_multi_child_ratio_sum(&dist);
    assert(fabs(sum - 1.0) < 0.1);  // Larger epsilon for 12 children
    
    // Cleanup
    free(dist.child_ids);
    free(dist.child_ratios);
    
    return 1;
}

// ============================================================================
// STATISTICS TESTS
// ============================================================================

int test_statistics_init() {
    WorkDistributionStats stats;
    work_distribution_stats_init(&stats);
    
    assert(stats.total_calculations == 0);
    assert(stats.valid_distributions == 0);
    assert(stats.invalid_distributions == 0);
    assert(stats.min_parent_ratio == 1.0);
    assert(stats.min_child_ratio == 1.0);
    
    return 1;
}

int test_statistics_update() {
    WorkDistributionStats stats;
    work_distribution_stats_init(&stats);
    
    // Add valid distribution
    WorkDistribution dist1 = calculate_work_distribution(3, 2);
    work_distribution_stats_update(&stats, &dist1);
    
    assert(stats.total_calculations == 1);
    assert(stats.valid_distributions == 1);
    
    // Add another valid distribution
    WorkDistribution dist2 = calculate_work_distribution(5, 2);
    work_distribution_stats_update(&stats, &dist2);
    
    assert(stats.total_calculations == 2);
    assert(stats.valid_distributions == 2);
    
    // Check averages are reasonable
    assert(stats.avg_parent_ratio > 0.0 && stats.avg_parent_ratio < 1.0);
    assert(stats.avg_child_ratio > 0.0 && stats.avg_child_ratio < 1.0);
    
    return 1;
}

int test_statistics_get() {
    WorkDistributionStats stats;
    work_distribution_stats_init(&stats);
    
    // Add some distributions
    for (int i = 0; i < 5; i++) {
        WorkDistribution dist = calculate_work_distribution(10, i + 1);
        work_distribution_stats_update(&stats, &dist);
    }
    
    uint64_t total, valid, invalid;
    double avg_parent, avg_child;
    
    work_distribution_stats_get(
        &stats,
        &total,
        &valid,
        &invalid,
        &avg_parent,
        &avg_child
    );
    
    assert(total == 5);
    assert(valid + invalid == total);
    
    return 1;
}

// ============================================================================
// UTILITY TESTS
// ============================================================================

int test_multi_child_create_destroy() {
    uint64_t child_ids[] = {1, 3, 7};
    
    MultiChildDistribution* dist = multi_child_distribution_create(
        10, child_ids, 3
    );
    
    assert(dist != NULL);
    assert(dist->is_valid == true);
    
    multi_child_distribution_destroy(dist);
    
    return 1;
}

int test_ratio_sum_getters() {
    // Single child
    WorkDistribution dist = calculate_work_distribution(3, 2);
    double sum = get_ratio_sum(&dist);
    assert(fabs(sum - 1.0) < MATH_EPSILON);
    
    // Multiple children
    uint64_t child_ids[] = {1, 3, 7};
    MultiChildDistribution multi_dist = calculate_multi_child_distribution(
        10, child_ids, 3
    );
    sum = get_multi_child_ratio_sum(&multi_dist);
    assert(fabs(sum - 1.0) < MATH_EPSILON);
    
    // Cleanup
    free(multi_dist.child_ids);
    free(multi_dist.child_ratios);
    
    return 1;
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main() {
    printf("=== Plimpton 322 Work Distribution Unit Tests ===\n\n");
    
    // Basic distribution tests
    TEST(calculate_distribution);
    TEST(validated_distribution);
    TEST(known_plimpton_distributions);
    
    // Validation tests
    TEST(ratio_sum_validation);
    TEST(distribution_validation);
    
    // Work splitting tests
    TEST(work_amount_calculation);
    TEST(split_work);
    TEST(split_work_edge_cases);
    
    // Multi-child distribution tests
    TEST(multi_child_distribution);
    TEST(multi_child_validation);
    TEST(split_work_multi_child);
    TEST(twelve_fold_distribution);
    
    // Statistics tests
    TEST(statistics_init);
    TEST(statistics_update);
    TEST(statistics_get);
    
    // Utility tests
    TEST(multi_child_create_destroy);
    TEST(ratio_sum_getters);
    
    printf("\n=== Test Summary ===\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Total:  %d\n", tests_passed + tests_failed);
    
    return tests_failed == 0 ? 0 : 1;
}