/**
 * Unit tests for Plimpton 322 integration with training system
 */

#include "ai/cllm_plimpton_integration.h"
#include "ai/cllm_plimpton_relationships.h"
#include "ai/cllm_work_distribution.h"
#include "ai/cllm_batch_splitting.h"
#include "cllm_batch.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "math/math.h"

#define MATH_EPSILON 1e-6

// Test counter
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) \
    printf("Testing %s...\n", #name); \
    if (test_##name()) { \
        printf("  ✓ PASSED\n"); \
        tests_passed++; \
    } else { \
        printf("  ✗ FAILED\n"); \
        tests_failed++; \
    }

/**
 * Test: Create and free integration context
 */
int test_create_free_context(void) {
    PlimptonIntegrationContext* ctx = plimpton_integration_create();
    if (!ctx) return 0;
    
    if (!plimpton_validate_context(ctx)) {
        plimpton_integration_free(ctx);
        return 0;
    }
    
    plimpton_integration_free(ctx);
    return 1;
}

/**
 * Test: Validate spawn with valid relationship
 */
int test_validate_spawn_valid(void) {
    PlimptonIntegrationContext* ctx = plimpton_integration_create();
    if (!ctx) return 0;
    
    // (3, 2) is a valid Plimpton pair
    int result = plimpton_validate_spawn(ctx, 3, 2);
    
    plimpton_integration_free(ctx);
    return result == 1;
}

/**
 * Test: Validate spawn with invalid relationship
 */
int test_validate_spawn_invalid(void) {
    PlimptonIntegrationContext* ctx = plimpton_integration_create();
    if (!ctx) return 0;
    
    // (4, 2) is invalid (not coprime)
    int result = plimpton_validate_spawn(ctx, 4, 2);
    
    plimpton_integration_free(ctx);
    return result == 0;
}

/**
 * Test: Generate children for parent
 */
int test_generate_children(void) {
    PlimptonIntegrationContext* ctx = plimpton_integration_create();
    if (!ctx) return 0;
    
    uint64_t child_ids[12];
    size_t count = plimpton_generate_children(ctx, 10, child_ids, 12);
    
    if (count == 0 || count > 12) {
        plimpton_integration_free(ctx);
        return 0;
    }
    
    // Verify all children are valid
    for (size_t i = 0; i < count; i++) {
        if (!validate_parent_child_relation(10, child_ids[i])) {
            plimpton_integration_free(ctx);
            return 0;
        }
    }
    
    plimpton_integration_free(ctx);
    return 1;
}

/**
 * Test: Calculate work distribution
 */
int test_calculate_distribution(void) {
    PlimptonIntegrationContext* ctx = plimpton_integration_create();
    if (!ctx) return 0;
    
    WorkDistribution dist = plimpton_calculate_distribution(ctx, 3, 2);
    
    // Verify ratios sum to 1.0
    double sum = dist.parent_keeps + dist.child_gets;
    if (fabs(sum - 1.0) > MATH_EPSILON) {
        plimpton_integration_free(ctx);
        return 0;
    }
    
    plimpton_integration_free(ctx);
    return 1;
}

/**
 * Test: Split batch by Plimpton ratios
 */
int test_split_batch(void) {
    PlimptonIntegrationContext* ctx = plimpton_integration_create();
    if (!ctx) return 0;
    
    // Create test batch
    CLLMBatch* batch = cllm_batch_create(100, 512);
    if (!batch) {
        plimpton_integration_free(ctx);
        return 0;
    }
    
    // Split batch
    SplitBatchResult result = plimpton_split_batch(ctx, batch, 3, 2);
    
    if (!result.is_valid) {
        cllm_batch_free(batch);
        plimpton_integration_free(ctx);
        return 0;
    }
    
    // Verify total samples preserved
    size_t total = result.parent_batch->batch_size + result.child_batch->batch_size;
    if (total != batch->batch_size) {
        split_batch_result_free(&result);
        cllm_batch_free(batch);
        plimpton_integration_free(ctx);
        return 0;
    }
    
    split_batch_result_free(&result);
    cllm_batch_free(batch);
    plimpton_integration_free(ctx);
    return 1;
}

/**
 * Test: Split batch among multiple children
 */
int test_split_batch_multi(void) {
    PlimptonIntegrationContext* ctx = plimpton_integration_create();
    if (!ctx) return 0;
    
    // Create test batch
    CLLMBatch* batch = cllm_batch_create(120, 512);
    if (!batch) {
        plimpton_integration_free(ctx);
        return 0;
    }
    
    // Use fixed children for simpler test
    uint64_t child_ids[3] = {1, 3, 7};
    size_t num_children = 3;
    
    // Split batch
    MultiChildSplitResult result = plimpton_split_batch_multi(
        ctx, batch, 10, child_ids, num_children
    );
    
    int success = 0;
    if (result.is_valid) {
        // Verify total samples preserved
        size_t total = result.parent_batch->batch_size;
        for (size_t i = 0; i < result.num_children; i++) {
            total += result.child_batches[i]->batch_size;
        }
        success = (total == batch->batch_size);
    }
    
    // Cleanup
    multi_child_split_result_free(&result);
    cllm_batch_free(batch);
    plimpton_integration_free(ctx);
    
    return success;
}

/**
 * Test: Create cache-aware distribution
 */
int test_cache_aware_distribution(void) {
    PlimptonIntegrationContext* ctx = plimpton_integration_create();
    if (!ctx) return 0;
    
    uint64_t child_ids[4] = {1, 3, 7, 9};
    uint64_t work_sizes[4] = {1000, 2000, 3000, 4000};
    
    CacheAwareDistribution dist = plimpton_create_cache_distribution(
        ctx, 10, child_ids, 4, work_sizes
    );
    
    int result = (dist.num_placements == 4);
    
    // Free the distribution before freeing context
    cache_aware_distribution_free(&dist);
    plimpton_integration_free(ctx);
    
    return result;
}

/**
 * Test: Record spawn success
 */
int test_record_spawn_success(void) {
    PlimptonIntegrationContext* ctx = plimpton_integration_create();
    if (!ctx) return 0;
    
    uint64_t child_ids[3] = {1, 3, 7};
    plimpton_record_spawn_success(ctx, 10, child_ids, 3);
    
    PlimptonIntegrationStats stats = plimpton_get_stats(ctx);
    
    if (stats.total_spawns != 1 || stats.successful_spawns != 1) {
        plimpton_integration_free(ctx);
        return 0;
    }
    
    plimpton_integration_free(ctx);
    return 1;
}

/**
 * Test: Record spawn failure
 */
int test_record_spawn_failure(void) {
    PlimptonIntegrationContext* ctx = plimpton_integration_create();
    if (!ctx) return 0;
    
    plimpton_record_spawn_failure(ctx, 10, 5);
    
    PlimptonIntegrationStats stats = plimpton_get_stats(ctx);
    
    if (stats.total_spawns != 1 || stats.failed_spawns != 1) {
        plimpton_integration_free(ctx);
        return 0;
    }
    
    plimpton_integration_free(ctx);
    return 1;
}

/**
 * Test: Statistics tracking
 */
int test_statistics_tracking(void) {
    PlimptonIntegrationContext* ctx = plimpton_integration_create();
    if (!ctx) return 0;
    
    // Record multiple operations
    uint64_t child_ids[2] = {1, 3};
    plimpton_record_spawn_success(ctx, 10, child_ids, 2);
    plimpton_record_spawn_failure(ctx, 10, 5);
    
    // Create and split batch
    CLLMBatch* batch = cllm_batch_create(100, 512);
    if (batch) {
        SplitBatchResult result = plimpton_split_batch(ctx, batch, 3, 2);
        if (result.is_valid) {
            split_batch_result_free(&result);
        }
        cllm_batch_free(batch);
    }
    
    PlimptonIntegrationStats stats = plimpton_get_stats(ctx);
    
    if (stats.total_spawns != 2) {
        plimpton_integration_free(ctx);
        return 0;
    }
    
    if (stats.successful_spawns != 1 || stats.failed_spawns != 1) {
        plimpton_integration_free(ctx);
        return 0;
    }
    
    plimpton_integration_free(ctx);
    return 1;
}

/**
 * Test: NULL context handling
 */
int test_null_context(void) {
    // All functions should handle NULL context gracefully
    int result = plimpton_validate_spawn(NULL, 3, 2);
    if (result != 0) return 0;
    
    size_t count = plimpton_generate_children(NULL, 10, NULL, 12);
    if (count != 0) return 0;
    
    plimpton_record_spawn_success(NULL, 10, NULL, 0);
    plimpton_record_spawn_failure(NULL, 10, 5);
    plimpton_print_stats(NULL);
    
    return 1;
}

/**
 * Test: Integration with known Plimpton pairs
 */
int test_known_plimpton_pairs(void) {
    PlimptonIntegrationContext* ctx = plimpton_integration_create();
    if (!ctx) return 0;
    
    // Test known Plimpton 322 pairs
    uint64_t pairs[][2] = {
        {3, 2}, {5, 4}, {5, 2}, {7, 4}, {9, 8}, {11, 10}
    };
    
    for (size_t i = 0; i < 6; i++) {
        if (!plimpton_validate_spawn(ctx, pairs[i][0], pairs[i][1])) {
            plimpton_integration_free(ctx);
            return 0;
        }
    }
    
    plimpton_integration_free(ctx);
    return 1;
}

/**
 * Test: End-to-end integration workflow
 */
int test_end_to_end_workflow(void) {
    PlimptonIntegrationContext* ctx = plimpton_integration_create();
    if (!ctx) return 0;
    
    // 1. Generate children for parent
    uint64_t parent_id = 10;
    uint64_t child_ids[12];
    size_t num_children = plimpton_generate_children(ctx, parent_id, child_ids, 12);
    
    if (num_children == 0) {
        plimpton_integration_free(ctx);
        return 0;
    }
    
    // 2. Create batch
    CLLMBatch* batch = cllm_batch_create(120, 512);
    if (!batch) {
        plimpton_integration_free(ctx);
        return 0;
    }
    
    // 3. Split batch among children
    MultiChildSplitResult result = plimpton_split_batch_multi(
        ctx, batch, parent_id, child_ids, num_children
    );
    
    if (!result.is_valid) {
        cllm_batch_free(batch);
        plimpton_integration_free(ctx);
        return 0;
    }
    
    // 4. Create cache-aware distribution
    uint64_t* work_sizes = (uint64_t*)malloc(num_children * sizeof(uint64_t));
    for (size_t i = 0; i < num_children; i++) {
        work_sizes[i] = result.child_batches[i]->batch_size * 512;
    }
    
    CacheAwareDistribution cache_dist = plimpton_create_cache_distribution(
        ctx, parent_id, child_ids, num_children, work_sizes
    );
    
    // 5. Record success
    plimpton_record_spawn_success(ctx, parent_id, child_ids, num_children);
    
    // 6. Verify statistics
    PlimptonIntegrationStats stats = plimpton_get_stats(ctx);
    if (stats.total_spawns != 1 || stats.successful_spawns != 1) {
        free(work_sizes);
        cache_aware_distribution_free(&cache_dist);
        multi_child_split_result_free(&result);
        cllm_batch_free(batch);
        plimpton_integration_free(ctx);
        return 0;
    }
    
    // Cleanup
    free(work_sizes);
    cache_aware_distribution_free(&cache_dist);
    multi_child_split_result_free(&result);
    cllm_batch_free(batch);
    plimpton_integration_free(ctx);
    return 1;
}

int main(void) {
    printf("=== Plimpton Integration Unit Tests ===\n\n");
    
    TEST(create_free_context);
    TEST(validate_spawn_valid);
    TEST(validate_spawn_invalid);
    TEST(generate_children);
    TEST(calculate_distribution);
    TEST(split_batch);
    TEST(split_batch_multi);
    TEST(cache_aware_distribution);
    TEST(record_spawn_success);
    TEST(record_spawn_failure);
    TEST(statistics_tracking);
    TEST(null_context);
    TEST(known_plimpton_pairs);
    TEST(end_to_end_workflow);
    
    printf("\n=== Test Summary ===\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Total:  %d\n", tests_passed + tests_failed);
    
    return tests_failed == 0 ? 0 : 1;
}