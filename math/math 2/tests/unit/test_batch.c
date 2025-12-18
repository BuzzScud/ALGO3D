#include "ai/cllm_batch.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

// Test counters
static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) \
    do { \
        printf("Running test: %s...", name); \
        tests_run++; \
    } while(0)

#define PASS() \
    do { \
        printf(" PASSED\n"); \
        tests_passed++; \
    } while(0)

#define FAIL(msg) \
    do { \
        printf(" FAILED: %s\n", msg); \
        return; \
    } while(0)

// ============================================================================
// TEST: Tensor Creation
// ============================================================================

void test_tensor_create(void) {
    TEST("tensor_create");
    
    uint32_t shape[3] = {2, 3, 4};
    Tensor* tensor = tensor_create(shape, 3);
    
    if (!tensor) FAIL("Failed to create tensor");
    
    if (tensor->ndim != 3) {
        tensor_free(tensor);
        FAIL("Incorrect number of dimensions");
    }
    
    if (tensor->total_size != 24) {
        tensor_free(tensor);
        FAIL("Incorrect total size");
    }
    
    if (!tensor->owns_data) {
        tensor_free(tensor);
        FAIL("Tensor should own data");
    }
    
    tensor_free(tensor);
    PASS();
}

// ============================================================================
// TEST: Tensor Operations
// ============================================================================

void test_tensor_operations(void) {
    TEST("tensor_operations");
    
    uint32_t shape[2] = {3, 4};
    Tensor* tensor = tensor_create(shape, 2);
    
    if (!tensor) FAIL("Failed to create tensor");
    
    // Fill with value
    tensor_fill(tensor, 5.0f);
    
    // Check value
    uint32_t indices[2] = {1, 2};
    float value = tensor_get(tensor, indices);
    
    if (value != 5.0f) {
        tensor_free(tensor);
        FAIL("Incorrect value after fill");
    }
    
    // Set value
    tensor_set(tensor, indices, 10.0f);
    value = tensor_get(tensor, indices);
    
    if (value != 10.0f) {
        tensor_free(tensor);
        FAIL("Incorrect value after set");
    }
    
    tensor_free(tensor);
    PASS();
}

// ============================================================================
// TEST: Tensor Copy
// ============================================================================

void test_tensor_copy(void) {
    TEST("tensor_copy");
    
    uint32_t shape[2] = {2, 3};
    Tensor* src = tensor_create(shape, 2);
    
    if (!src) FAIL("Failed to create source tensor");
    
    tensor_fill(src, 7.0f);
    
    Tensor* dst = tensor_copy(src);
    
    if (!dst) {
        tensor_free(src);
        FAIL("Failed to copy tensor");
    }
    
    // Check that data was copied
    uint32_t indices[2] = {0, 0};
    if (tensor_get(dst, indices) != 7.0f) {
        tensor_free(src);
        tensor_free(dst);
        FAIL("Data not copied correctly");
    }
    
    tensor_free(src);
    tensor_free(dst);
    PASS();
}

// ============================================================================
// TEST: Batch Creation
// ============================================================================

void test_batch_create(void) {
    TEST("batch_create");
    
    Batch* batch = batch_create(32, 128, 50000);
    
    if (!batch) FAIL("Failed to create batch");
    
    if (batch->batch_size != 32) {
        batch_free(batch);
        FAIL("Incorrect batch size");
    }
    
    if (batch->sequence_length != 128) {
        batch_free(batch);
        FAIL("Incorrect sequence length");
    }
    
    if (batch->vocab_size != 50000) {
        batch_free(batch);
        FAIL("Incorrect vocab size");
    }
    
    batch_free(batch);
    PASS();
}

// ============================================================================
// TEST: Batch Copy
// ============================================================================

void test_batch_copy(void) {
    TEST("batch_copy");
    
    Batch* src = batch_create(16, 64, 10000);
    
    if (!src) FAIL("Failed to create source batch");
    
    // Fill with test data
    tensor_fill(&src->input, 1.0f);
    tensor_fill(&src->target, 2.0f);
    
    Batch* dst = batch_copy(src);
    
    if (!dst) {
        batch_free(src);
        FAIL("Failed to copy batch");
    }
    
    // Check that data was copied
    if (dst->input.data[0] != 1.0f || dst->target.data[0] != 2.0f) {
        batch_free(src);
        batch_free(dst);
        FAIL("Data not copied correctly");
    }
    
    batch_free(src);
    batch_free(dst);
    PASS();
}

// ============================================================================
// TEST: Batch Split
// ============================================================================

void test_batch_split(void) {
    TEST("batch_split");
    
    Batch* batch = batch_create(32, 64, 10000);
    
    if (!batch) FAIL("Failed to create batch");
    
    // Fill with test data
    tensor_fill(&batch->input, 5.0f);
    
    // Split into 4 batches
    Batch* splits[4];
    if (!batch_split(batch, 4, splits)) {
        batch_free(batch);
        FAIL("Failed to split batch");
    }
    
    // Check split sizes
    for (int i = 0; i < 4; i++) {
        if (splits[i]->batch_size != 8) {
            batch_free(batch);
            for (int j = 0; j < 4; j++) batch_free(splits[j]);
            FAIL("Incorrect split size");
        }
    }
    
    // Check data
    if (splits[0]->input.data[0] != 5.0f) {
        batch_free(batch);
        for (int j = 0; j < 4; j++) batch_free(splits[j]);
        FAIL("Data not split correctly");
    }
    
    batch_free(batch);
    for (int i = 0; i < 4; i++) batch_free(splits[i]);
    PASS();
}

// ============================================================================
// TEST: Batch Merge
// ============================================================================

void test_batch_merge(void) {
    TEST("batch_merge");
    
    // Create 3 batches
    Batch* batches[3];
    for (int i = 0; i < 3; i++) {
        batches[i] = batch_create(8, 64, 10000);
        if (!batches[i]) {
            for (int j = 0; j < i; j++) batch_free(batches[j]);
            FAIL("Failed to create batch");
        }
        tensor_fill(&batches[i]->input, (float)(i + 1));
    }
    
    // Merge batches
    Batch* merged = batch_merge(batches, 3);
    
    if (!merged) {
        for (int i = 0; i < 3; i++) batch_free(batches[i]);
        FAIL("Failed to merge batches");
    }
    
    // Check merged size
    if (merged->batch_size != 24) {
        for (int i = 0; i < 3; i++) batch_free(batches[i]);
        batch_free(merged);
        FAIL("Incorrect merged size");
    }
    
    for (int i = 0; i < 3; i++) batch_free(batches[i]);
    batch_free(merged);
    PASS();
}

// ============================================================================
// TEST: Batch Reference Counting
// ============================================================================

void test_batch_refcount(void) {
    TEST("batch_refcount");
    
    Batch* batch = batch_create(16, 64, 10000);
    
    if (!batch) FAIL("Failed to create batch");
    
    // Initial ref count should be 1
    int initial_count = atomic_load(&batch->ref_count);
    if (initial_count != 1) {
        batch_free(batch);
        FAIL("Initial ref count should be 1");
    }
    
    // Retain
    batch_retain(batch);
    int after_retain = atomic_load(&batch->ref_count);
    if (after_retain != 2) {
        batch_free(batch);
        FAIL("Ref count should be 2 after retain");
    }
    
    // Release
    batch_release(batch);
    int after_release = atomic_load(&batch->ref_count);
    if (after_release != 1) {
        batch_free(batch);
        FAIL("Ref count should be 1 after release");
    }
    
    batch_free(batch);
    PASS();
}

// ============================================================================
// TEST: Batch Validation
// ============================================================================

void test_batch_validation(void) {
    TEST("batch_validation");
    
    Batch* batch = batch_create(16, 64, 10000);
    
    if (!batch) FAIL("Failed to create batch");
    
    // Fill with valid data
    tensor_fill(&batch->input, 1.0f);
    tensor_fill(&batch->target, 2.0f);
    
    if (!batch_validate(batch)) {
        batch_free(batch);
        FAIL("Valid batch should pass validation");
    }
    
    batch_free(batch);
    PASS();
}

// ============================================================================
// TEST: Batch Queue
// ============================================================================

void test_batch_queue(void) {
    TEST("batch_queue");
    
    BatchQueue* queue = batch_queue_create(10);
    
    if (!queue) FAIL("Failed to create batch queue");
    
    // Check initial state
    if (!batch_queue_is_empty(queue)) {
        batch_queue_free(queue);
        FAIL("Queue should be empty initially");
    }
    
    // Enqueue batches
    for (int i = 0; i < 5; i++) {
        Batch* batch = batch_create(16, 64, 10000);
        if (!batch_queue_try_enqueue(queue, batch)) {
            batch_free(batch);
            batch_queue_free(queue);
            FAIL("Failed to enqueue batch");
        }
    }
    
    // Check size
    if (batch_queue_size(queue) != 5) {
        batch_queue_free(queue);
        FAIL("Queue size should be 5");
    }
    
    // Dequeue batches
    for (int i = 0; i < 5; i++) {
        Batch* batch = batch_queue_try_dequeue(queue);
        if (!batch) {
            batch_queue_free(queue);
            FAIL("Failed to dequeue batch");
        }
        batch_free(batch);
    }
    
    // Check empty
    if (!batch_queue_is_empty(queue)) {
        batch_queue_free(queue);
        FAIL("Queue should be empty after dequeuing all");
    }
    
    batch_queue_free(queue);
    PASS();
}

// ============================================================================
// TEST: Batch Pool
// ============================================================================

void test_batch_pool(void) {
    TEST("batch_pool");
    
    BatchPool* pool = batch_pool_create(5, 16, 64, 10000);
    
    if (!pool) FAIL("Failed to create batch pool");
    
    // Allocate batches
    Batch* batches[5];
    for (int i = 0; i < 5; i++) {
        batches[i] = batch_pool_try_allocate(pool);
        if (!batches[i]) {
            batch_pool_free(pool);
            FAIL("Failed to allocate batch from pool");
        }
    }
    
    // Pool should be empty now
    Batch* extra = batch_pool_try_allocate(pool);
    if (extra) {
        batch_pool_free(pool);
        FAIL("Pool should be empty");
    }
    
    // Release batches
    for (int i = 0; i < 5; i++) {
        batch_pool_release(pool, batches[i]);
    }
    
    // Check statistics
    uint64_t allocations, releases, hits, misses;
    batch_pool_get_stats(pool, &allocations, &releases, &hits, &misses);
    
    if (allocations != 5 || releases != 5) {
        batch_pool_free(pool);
        FAIL("Incorrect pool statistics");
    }
    
    batch_pool_free(pool);
    PASS();
}

// ============================================================================
// TEST: Batch Distribution
// ============================================================================

void test_batch_distribution(void) {
    TEST("batch_distribution");
    
    Batch* batch = batch_create(32, 64, 10000);
    
    if (!batch) FAIL("Failed to create batch");
    
    // Distribute to 4 spheres
    Batch* sphere_batches[4];
    if (!batch_distribute_to_spheres(batch, 4, sphere_batches)) {
        batch_free(batch);
        FAIL("Failed to distribute batch");
    }
    
    // Check distribution
    for (int i = 0; i < 4; i++) {
        if (sphere_batches[i]->batch_size != 8) {
            batch_free(batch);
            for (int j = 0; j < 4; j++) batch_free(sphere_batches[j]);
            FAIL("Incorrect distribution size");
        }
    }
    
    batch_free(batch);
    for (int i = 0; i < 4; i++) batch_free(sphere_batches[i]);
    PASS();
}

// ============================================================================
// TEST: Batch Assignment
// ============================================================================

void test_batch_assignment(void) {
    TEST("batch_assignment");
    
    Batch* batch = batch_create(16, 64, 10000);
    
    if (!batch) FAIL("Failed to create batch");
    
    // Assign to symmetry group
    if (!batch_assign_to_group(batch, 5)) {
        batch_free(batch);
        FAIL("Failed to assign batch to group");
    }
    
    // Check assignment (stored in upper bits of batch_id)
    uint32_t group = (batch->batch_id >> 60) & 0xF;
    if (group != 5) {
        batch_free(batch);
        FAIL("Incorrect group assignment");
    }
    
    batch_free(batch);
    PASS();
}

// ============================================================================
// TEST: Batch Balance Distribution
// ============================================================================

void test_batch_balance(void) {
    TEST("batch_balance");
    
    // Create batches
    Batch* batches[10];
    for (int i = 0; i < 10; i++) {
        batches[i] = batch_create(16, 64, 10000);
        if (!batches[i]) {
            for (int j = 0; j < i; j++) batch_free(batches[j]);
            FAIL("Failed to create batch");
        }
    }
    
    // Balance distribution across 4 spheres
    uint32_t assignments[10];
    if (!batch_balance_distribution(batches, 10, 4, assignments)) {
        for (int i = 0; i < 10; i++) batch_free(batches[i]);
        FAIL("Failed to balance distribution");
    }
    
    // Check assignments (should be round-robin)
    for (int i = 0; i < 10; i++) {
        if (assignments[i] != (uint32_t)(i % 4)) {
            for (int j = 0; j < 10; j++) batch_free(batches[j]);
            FAIL("Incorrect assignment");
        }
    }
    
    for (int i = 0; i < 10; i++) batch_free(batches[i]);
    PASS();
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main(void) {
    printf("\n");
    printf("========================================\n");
    printf("  Batch Processing Tests\n");
    printf("========================================\n\n");
    
    // Run all tests
    test_tensor_create();
    test_tensor_operations();
    test_tensor_copy();
    test_batch_create();
    test_batch_copy();
    test_batch_split();
    test_batch_merge();
    test_batch_refcount();
    test_batch_validation();
    test_batch_queue();
    test_batch_pool();
    test_batch_distribution();
    test_batch_assignment();
    test_batch_balance();
    
    // Print summary
    printf("\n");
    printf("========================================\n");
    printf("  Test Summary\n");
    printf("========================================\n");
    printf("Tests run:    %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);
    printf("Success rate: %.1f%%\n", 
           tests_run > 0 ? (100.0 * tests_passed / tests_run) : 0.0);
    printf("========================================\n\n");
    
    return (tests_passed == tests_run) ? 0 : 1;
}