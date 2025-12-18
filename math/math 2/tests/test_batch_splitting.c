/**
 * Unit Tests for Plimpton 322 Batch Splitting
 * 
 * Tests batch splitting based on work distribution ratios.
 */

#include "ai/cllm_batch_splitting.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

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

// Helper function to create a test batch
static CLLMBatch* create_test_batch(uint32_t batch_size, uint32_t seq_len) {
    CLLMBatch* batch = cllm_batch_create(batch_size, seq_len);
    if (!batch) return NULL;
    
    // Fill with test data
    for (uint32_t i = 0; i < batch_size * seq_len; i++) {
        batch->input_ids[i] = i;
        batch->target_ids[i] = i + 1;
        batch->attention_mask[i] = 1.0f;
    }
    
    return batch;
}

// ============================================================================
// SPLIT INDICES TESTS
// ============================================================================

int test_calculate_split_indices() {
    uint32_t parent_samples, child_samples;
    
    // Test 50/50 split
    calculate_split_indices(100, 0.5, &parent_samples, &child_samples);
    assert(parent_samples == 50);
    assert(child_samples == 50);
    
    // Test 30/70 split
    calculate_split_indices(100, 0.3, &parent_samples, &child_samples);
    assert(parent_samples == 30);
    assert(child_samples == 70);
    
    // Test edge cases
    calculate_split_indices(100, 0.0, &parent_samples, &child_samples);
    assert(parent_samples >= 0);
    assert(child_samples >= 0);
    assert(parent_samples + child_samples == 100);
    
    calculate_split_indices(100, 1.0, &parent_samples, &child_samples);
    assert(parent_samples >= 0);
    assert(child_samples >= 0);
    assert(parent_samples + child_samples == 100);
    
    return 1;
}

int test_calculate_multi_child_split_indices() {
    MultiChildDistribution dist;
    dist.num_children = 3;
    dist.parent_keeps = 0.25;
    
    double child_ratios[] = {0.25, 0.25, 0.25};
    dist.child_ratios = child_ratios;
    
    uint32_t parent_samples;
    uint32_t child_samples[3];
    
    calculate_multi_child_split_indices(100, &dist, &parent_samples, child_samples);
    
    // Check total
    uint32_t total = parent_samples;
    for (int i = 0; i < 3; i++) {
        total += child_samples[i];
    }
    assert(total == 100);
    
    return 1;
}

// ============================================================================
// BATCH COPY TESTS
// ============================================================================

int test_copy_batch_samples() {
    CLLMBatch* source = create_test_batch(10, 5);
    CLLMBatch* dest = create_test_batch(5, 5);
    
    assert(source != NULL);
    assert(dest != NULL);
    
    // Copy first 5 samples
    bool success = copy_batch_samples(dest, source, 0, 5);
    assert(success == true);
    assert(dest->batch_size == 5);
    
    // Verify data was copied
    for (uint32_t i = 0; i < 5 * 5; i++) {
        assert(dest->input_ids[i] == source->input_ids[i]);
    }
    
    cllm_batch_free(source);
    cllm_batch_free(dest);
    
    return 1;
}

int test_copy_batch_samples_range() {
    CLLMBatch* source = create_test_batch(10, 5);
    CLLMBatch* dest = create_test_batch(5, 5);
    
    assert(source != NULL);
    assert(dest != NULL);
    
    // Copy samples 5-9
    bool success = copy_batch_samples(dest, source, 5, 5);
    assert(success == true);
    
    // Verify data was copied from correct range
    for (uint32_t i = 0; i < 5 * 5; i++) {
        assert(dest->input_ids[i] == source->input_ids[5 * 5 + i]);
    }
    
    cllm_batch_free(source);
    cllm_batch_free(dest);
    
    return 1;
}

// ============================================================================
// BATCH SPLITTING TESTS
// ============================================================================

int test_split_batch_by_count() {
    CLLMBatch* source = create_test_batch(10, 5);
    assert(source != NULL);
    
    SplitBatchResult result = split_batch_by_count(source, 6, 4);
    
    assert(result.is_valid == true);
    assert(result.parent_samples == 6);
    assert(result.child_samples == 4);
    assert(result.parent_batch != NULL);
    assert(result.child_batch != NULL);
    assert(result.parent_batch->batch_size == 6);
    assert(result.child_batch->batch_size == 4);
    
    split_batch_result_free(&result);
    cllm_batch_free(source);
    
    return 1;
}

int test_split_batch_by_ratios() {
    CLLMBatch* source = create_test_batch(100, 10);
    assert(source != NULL);
    
    // Create work distribution (30/70 split)
    WorkDistribution dist;
    dist.parent_keeps = 0.3;
    dist.child_gets = 0.7;
    dist.is_valid = true;
    
    SplitBatchResult result = split_batch_by_ratios(source, &dist);
    
    assert(result.is_valid == true);
    assert(result.parent_samples == 30);
    assert(result.child_samples == 70);
    
    // Verify total samples preserved
    assert(get_split_result_total_samples(&result) == 100);
    
    split_batch_result_free(&result);
    cllm_batch_free(source);
    
    return 1;
}

int test_split_batch_multi_child() {
    CLLMBatch* source = create_test_batch(100, 10);
    assert(source != NULL);
    
    // Create multi-child distribution
    MultiChildDistribution dist;
    dist.num_children = 3;
    dist.parent_keeps = 0.25;
    
    double child_ratios[] = {0.25, 0.25, 0.25};
    dist.child_ratios = child_ratios;
    
    uint64_t child_ids[] = {1, 2, 3};
    dist.child_ids = child_ids;
    dist.is_valid = true;
    
    MultiChildSplitResult result = split_batch_multi_child(source, &dist);
    
    assert(result.is_valid == true);
    assert(result.num_children == 3);
    assert(result.parent_batch != NULL);
    assert(result.child_batches != NULL);
    
    // Verify total samples preserved
    assert(get_multi_child_split_result_total_samples(&result) == 100);
    
    multi_child_split_result_free(&result);
    cllm_batch_free(source);
    
    return 1;
}

// ============================================================================
// VALIDATION TESTS
// ============================================================================

int test_validate_split_result() {
    CLLMBatch* source = create_test_batch(10, 5);
    assert(source != NULL);
    
    SplitBatchResult result = split_batch_by_count(source, 6, 4);
    
    assert(validate_split_result(&result, source) == true);
    
    split_batch_result_free(&result);
    cllm_batch_free(source);
    
    return 1;
}

int test_validate_multi_child_split_result() {
    CLLMBatch* source = create_test_batch(100, 10);
    assert(source != NULL);
    
    MultiChildDistribution dist;
    dist.num_children = 2;
    dist.parent_keeps = 0.5;
    
    double child_ratios[] = {0.25, 0.25};
    dist.child_ratios = child_ratios;
    
    uint64_t child_ids[] = {1, 2};
    dist.child_ids = child_ids;
    dist.is_valid = true;
    
    MultiChildSplitResult result = split_batch_multi_child(source, &dist);
    
    assert(validate_multi_child_split_result(&result, source) == true);
    
    multi_child_split_result_free(&result);
    cllm_batch_free(source);
    
    return 1;
}

// ============================================================================
// EDGE CASE TESTS
// ============================================================================

int test_split_small_batch() {
    CLLMBatch* source = create_test_batch(2, 5);
    assert(source != NULL);
    
    WorkDistribution dist;
    dist.parent_keeps = 0.5;
    dist.child_gets = 0.5;
    dist.is_valid = true;
    
    SplitBatchResult result = split_batch_by_ratios(source, &dist);
    
    assert(result.is_valid == true);
    assert(result.parent_samples + result.child_samples == 2);
    
    split_batch_result_free(&result);
    cllm_batch_free(source);
    
    return 1;
}

int test_split_large_batch() {
    CLLMBatch* source = create_test_batch(1000, 10);
    assert(source != NULL);
    
    WorkDistribution dist;
    dist.parent_keeps = 0.3;
    dist.child_gets = 0.7;
    dist.is_valid = true;
    
    SplitBatchResult result = split_batch_by_ratios(source, &dist);
    
    assert(result.is_valid == true);
    assert(get_split_result_total_samples(&result) == 1000);
    
    split_batch_result_free(&result);
    cllm_batch_free(source);
    
    return 1;
}

// ============================================================================
// STATISTICS TESTS
// ============================================================================

int test_statistics_init() {
    BatchSplittingStats stats;
    batch_splitting_stats_init(&stats);
    
    assert(stats.total_splits == 0);
    assert(stats.successful_splits == 0);
    assert(stats.failed_splits == 0);
    
    return 1;
}

int test_statistics_update() {
    BatchSplittingStats stats;
    batch_splitting_stats_init(&stats);
    
    CLLMBatch* source = create_test_batch(100, 10);
    WorkDistribution dist;
    dist.parent_keeps = 0.3;
    dist.child_gets = 0.7;
    dist.is_valid = true;
    
    SplitBatchResult result = split_batch_by_ratios(source, &dist);
    
    batch_splitting_stats_update(&stats, &result, &dist);
    
    assert(stats.total_splits == 1);
    assert(stats.successful_splits == 1);
    assert(stats.total_samples_split == 100);
    
    split_batch_result_free(&result);
    cllm_batch_free(source);
    
    return 1;
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main() {
    printf("=== Plimpton 322 Batch Splitting Unit Tests ===\n\n");
    
    // Split indices tests
    TEST(calculate_split_indices);
    TEST(calculate_multi_child_split_indices);
    
    // Batch copy tests
    TEST(copy_batch_samples);
    TEST(copy_batch_samples_range);
    
    // Batch splitting tests
    TEST(split_batch_by_count);
    TEST(split_batch_by_ratios);
    TEST(split_batch_multi_child);
    
    // Validation tests
    TEST(validate_split_result);
    TEST(validate_multi_child_split_result);
    
    // Edge case tests
    TEST(split_small_batch);
    TEST(split_large_batch);
    
    // Statistics tests
    TEST(statistics_init);
    TEST(statistics_update);
    
    printf("\n=== Test Summary ===\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Total:  %d\n", tests_passed + tests_failed);
    
    return tests_failed == 0 ? 0 : 1;
}