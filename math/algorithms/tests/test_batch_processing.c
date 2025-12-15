#include "batch_processing.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>

// Test counter
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) \
    do { \
        printf("Testing %s... ", name); \
        fflush(stdout); \
    } while(0)

#define PASS() \
    do { \
        printf("✓ PASS\n"); \
        tests_passed++; \
    } while(0)

#define FAIL(msg) \
    do { \
        printf("✗ FAIL: %s\n", msg); \
        tests_failed++; \
    } while(0)

// ============================================================================
// BATCH TESTS
// ============================================================================

void test_batch_creation() {
    TEST("Batch Creation");
    
    int* data = malloc(sizeof(int) * 10);
    for (int i = 0; i < 10; i++) data[i] = i;
    
    GenericBatch* batch = batch_create(1, 0, 10, data, sizeof(int) * 10, free);
    
    if (!batch) {
        FAIL("Failed to create batch");
        return;
    }
    
    if (batch->batch_id != 1 || batch->batch_size != 10) {
        FAIL("Incorrect batch properties");
        batch_free(batch);
        return;
    }
    
    if (batch_get_ref_count(batch) != 1) {
        FAIL("Incorrect reference count");
        batch_free(batch);
        return;
    }
    
    batch_free(batch);
    PASS();
}

void test_batch_reference_counting() {
    TEST("Batch Reference Counting");
    
    GenericBatch* batch = batch_create(1, 0, 10, NULL, 0, NULL);
    
    if (batch_get_ref_count(batch) != 1) {
        FAIL("Initial ref count incorrect");
        batch_free(batch);
        return;
    }
    
    batch_retain(batch);
    if (batch_get_ref_count(batch) != 2) {
        FAIL("Ref count after retain incorrect");
        batch_free(batch);
        return;
    }
    
    batch_release(batch);
    if (batch_get_ref_count(batch) != 1) {
        FAIL("Ref count after release incorrect");
        batch_free(batch);
        return;
    }
    
    batch_release(batch);  // Should free the batch
    PASS();
}

void test_batch_processing_state() {
    TEST("Batch Processing State");
    
    GenericBatch* batch = batch_create(1, 0, 10, NULL, 0, NULL);
    
    if (batch_is_processed(batch)) {
        FAIL("New batch should not be processed");
        batch_free(batch);
        return;
    }
    
    batch_mark_processed(batch, 1.5);
    
    if (!batch_is_processed(batch)) {
        FAIL("Batch should be marked as processed");
        batch_free(batch);
        return;
    }
    
    if (batch->processing_time != 1.5) {
        FAIL("Processing time incorrect");
        batch_free(batch);
        return;
    }
    
    batch_free(batch);
    PASS();
}

// ============================================================================
// BATCH QUEUE TESTS
// ============================================================================

void test_queue_creation() {
    TEST("Queue Creation");
    
    BatchQueue* queue = batch_queue_create(10);
    
    if (!queue) {
        FAIL("Failed to create queue");
        return;
    }
    
    if (!batch_queue_is_empty(queue)) {
        FAIL("New queue should be empty");
        batch_queue_free(queue);
        return;
    }
    
    if (batch_queue_size(queue) != 0) {
        FAIL("New queue size should be 0");
        batch_queue_free(queue);
        return;
    }
    
    batch_queue_free(queue);
    PASS();
}

void test_queue_enqueue_dequeue() {
    TEST("Queue Enqueue/Dequeue");
    
    BatchQueue* queue = batch_queue_create(10);
    
    // Enqueue batches
    for (int i = 0; i < 5; i++) {
        GenericBatch* batch = batch_create(i, 0, 10, NULL, 0, NULL);
        if (!batch_queue_try_enqueue(queue, batch)) {
            FAIL("Failed to enqueue");
            batch_queue_free(queue);
            return;
        }
    }
    
    if (batch_queue_size(queue) != 5) {
        FAIL("Incorrect queue size");
        batch_queue_free(queue);
        return;
    }
    
    // Dequeue batches
    for (int i = 0; i < 5; i++) {
        GenericBatch* batch = batch_queue_try_dequeue(queue);
        if (!batch) {
            FAIL("Failed to dequeue");
            batch_queue_free(queue);
            return;
        }
        
        if (batch->batch_id != (uint64_t)i) {
            FAIL("Incorrect batch order");
            batch_release(batch);
            batch_queue_free(queue);
            return;
        }
        
        batch_release(batch);
    }
    
    if (!batch_queue_is_empty(queue)) {
        FAIL("Queue should be empty");
        batch_queue_free(queue);
        return;
    }
    
    batch_queue_free(queue);
    PASS();
}

void test_queue_peek() {
    TEST("Queue Peek");
    
    BatchQueue* queue = batch_queue_create(10);
    
    GenericBatch* batch = batch_create(42, 0, 10, NULL, 0, NULL);
    batch_queue_try_enqueue(queue, batch);
    
    // Peek at batch
    GenericBatch* peeked = batch_queue_peek(queue);
    if (!peeked || peeked->batch_id != 42) {
        FAIL("Incorrect peeked batch");
        batch_queue_free(queue);
        return;
    }
    
    // Queue should still have the batch
    if (batch_queue_size(queue) != 1) {
        FAIL("Peek should not remove batch");
        batch_queue_free(queue);
        return;
    }
    
    batch_queue_free(queue);
    PASS();
}

void test_queue_full() {
    TEST("Queue Full");
    
    BatchQueue* queue = batch_queue_create(3);
    
    // Fill queue
    for (int i = 0; i < 3; i++) {
        GenericBatch* batch = batch_create(i, 0, 10, NULL, 0, NULL);
        batch_queue_try_enqueue(queue, batch);
    }
    
    if (!batch_queue_is_full(queue)) {
        FAIL("Queue should be full");
        batch_queue_free(queue);
        return;
    }
    
    // Try to enqueue when full
    GenericBatch* batch = batch_create(99, 0, 10, NULL, 0, NULL);
    if (batch_queue_try_enqueue(queue, batch)) {
        FAIL("Should not be able to enqueue when full");
        batch_release(batch);
        batch_queue_free(queue);
        return;
    }
    
    batch_release(batch);
    batch_queue_free(queue);
    PASS();
}

void test_queue_close() {
    TEST("Queue Close");
    
    BatchQueue* queue = batch_queue_create(10);
    
    batch_queue_close(queue);
    
    if (!batch_queue_is_closed(queue)) {
        FAIL("Queue should be closed");
        batch_queue_free(queue);
        return;
    }
    
    // Try to enqueue after close
    GenericBatch* batch = batch_create(1, 0, 10, NULL, 0, NULL);
    if (batch_queue_try_enqueue(queue, batch)) {
        FAIL("Should not be able to enqueue after close");
        batch_release(batch);
        batch_queue_free(queue);
        return;
    }
    
    batch_release(batch);
    batch_queue_free(queue);
    PASS();
}

// Thread function for concurrent queue test
static BatchQueue* test_queue = NULL;
static atomic_int enqueue_count = ATOMIC_VAR_INIT(0);
static atomic_int dequeue_count = ATOMIC_VAR_INIT(0);

void* producer_thread(void* arg) {
    int id = (int)(intptr_t)arg;
    
    for (int i = 0; i < 10; i++) {
        GenericBatch* batch = batch_create(id * 100 + i, 0, 10, NULL, 0, NULL);
        batch_queue_enqueue(test_queue, batch);
        atomic_fetch_add(&enqueue_count, 1);
    }
    
    return NULL;
}

void* consumer_thread(void* arg) {
    (void)arg;
    
    for (int i = 0; i < 10; i++) {
        GenericBatch* batch = batch_queue_dequeue(test_queue);
        if (batch) {
            atomic_fetch_add(&dequeue_count, 1);
            batch_release(batch);
        }
    }
    
    return NULL;
}

void test_queue_concurrent() {
    TEST("Queue Concurrent Access");
    
    test_queue = batch_queue_create(50);
    atomic_store(&enqueue_count, 0);
    atomic_store(&dequeue_count, 0);
    
    const int num_producers = 3;
    const int num_consumers = 3;
    
    pthread_t producers[num_producers];
    pthread_t consumers[num_consumers];
    
    // Start consumers first
    for (int i = 0; i < num_consumers; i++) {
        pthread_create(&consumers[i], NULL, consumer_thread, (void*)(intptr_t)i);
    }
    
    // Start producers
    for (int i = 0; i < num_producers; i++) {
        pthread_create(&producers[i], NULL, producer_thread, (void*)(intptr_t)i);
    }
    
    // Wait for all threads
    for (int i = 0; i < num_producers; i++) {
        pthread_join(producers[i], NULL);
    }
    for (int i = 0; i < num_consumers; i++) {
        pthread_join(consumers[i], NULL);
    }
    
    int enqueued = atomic_load(&enqueue_count);
    int dequeued = atomic_load(&dequeue_count);
    
    if (enqueued != 30 || dequeued != 30) {
        FAIL("Incorrect concurrent operation counts");
        batch_queue_free(test_queue);
        return;
    }
    
    batch_queue_free(test_queue);
    PASS();
}

// ============================================================================
// BATCH POOL TESTS
// ============================================================================

void test_pool_creation() {
    TEST("Pool Creation");
    
    BatchPool* pool = batch_pool_create(5, 10, 100);
    
    if (!pool) {
        FAIL("Failed to create pool");
        return;
    }
    
    if (pool->pool_size != 5) {
        FAIL("Incorrect pool size");
        batch_pool_free(pool);
        return;
    }
    
    batch_pool_free(pool);
    PASS();
}

void test_pool_allocate_release() {
    TEST("Pool Allocate/Release");
    
    BatchPool* pool = batch_pool_create(3, 10, 100);
    
    // Allocate batches
    GenericBatch* batch1 = batch_pool_try_allocate(pool);
    GenericBatch* batch2 = batch_pool_try_allocate(pool);
    GenericBatch* batch3 = batch_pool_try_allocate(pool);
    
    if (!batch1 || !batch2 || !batch3) {
        FAIL("Failed to allocate from pool");
        batch_pool_free(pool);
        return;
    }
    
    // Pool should be exhausted
    GenericBatch* batch4 = batch_pool_try_allocate(pool);
    if (batch4) {
        FAIL("Should not be able to allocate from exhausted pool");
        batch_pool_free(pool);
        return;
    }
    
    // Release one batch
    batch_pool_release(pool, batch1);
    
    // Should be able to allocate again
    batch4 = batch_pool_try_allocate(pool);
    if (!batch4) {
        FAIL("Should be able to allocate after release");
        batch_pool_free(pool);
        return;
    }
    
    batch_pool_free(pool);
    PASS();
}

void test_pool_statistics() {
    TEST("Pool Statistics");
    
    BatchPool* pool = batch_pool_create(3, 10, 100);
    
    // Allocate and release
    GenericBatch* batch1 = batch_pool_try_allocate(pool);
    GenericBatch* batch2 = batch_pool_try_allocate(pool);
    
    batch_pool_release(pool, batch1);
    batch_pool_release(pool, batch2);
    
    // Try to allocate when pool is exhausted (miss)
    batch_pool_try_allocate(pool);
    batch_pool_try_allocate(pool);
    batch_pool_try_allocate(pool);
    batch_pool_try_allocate(pool);  // This should be a miss
    
    uint64_t allocations, releases, hits, misses;
    batch_pool_get_stats(pool, &allocations, &releases, &hits, &misses);
    
    // Total: 2 initial + 4 more = 6 allocations
    if (allocations != 6) {
        FAIL("Incorrect allocation count");
        batch_pool_free(pool);
        return;
    }
    
    if (releases != 2) {
        FAIL("Incorrect release count");
        batch_pool_free(pool);
        return;
    }
    
    // Hits: 2 initial + 3 after release = 5
    if (hits != 5) {
        FAIL("Incorrect hit count");
        batch_pool_free(pool);
        return;
    }
    
    // Misses: 1 (the 4th allocation after exhausting pool)
    if (misses != 1) {
        FAIL("Incorrect miss count");
        batch_pool_free(pool);
        return;
    }
    
    batch_pool_free(pool);
    PASS();
}

void test_pool_efficiency() {
    TEST("Pool Efficiency");
    
    BatchPool* pool = batch_pool_create(5, 10, 100);
    
    // All allocations should hit
    for (int i = 0; i < 5; i++) {
        GenericBatch* batch = batch_pool_try_allocate(pool);
        if (batch) batch_pool_release(pool, batch);
    }
    
    double efficiency = batch_pool_get_efficiency(pool);
    if (efficiency != 1.0) {
        FAIL("Efficiency should be 100%");
        batch_pool_free(pool);
        return;
    }
    
    batch_pool_free(pool);
    PASS();
}

// ============================================================================
// BATCH SPLITTING AND MERGING TESTS
// ============================================================================

void test_batch_split() {
    TEST("Batch Split");
    
    GenericBatch* batch = batch_create(1, 0, 10, NULL, 0, NULL);
    
    GenericBatch* splits[2];
    if (!batch_split(batch, 2, splits)) {
        FAIL("Failed to split batch");
        batch_free(batch);
        return;
    }
    
    if (splits[0]->batch_size != 5 || splits[1]->batch_size != 5) {
        FAIL("Incorrect split sizes");
        batch_free(splits[0]);
        batch_free(splits[1]);
        batch_free(batch);
        return;
    }
    
    batch_free(splits[0]);
    batch_free(splits[1]);
    batch_free(batch);
    PASS();
}

void test_batch_merge() {
    TEST("Batch Merge");
    
    GenericBatch* batch1 = batch_create(1, 0, 5, NULL, 0, NULL);
    GenericBatch* batch2 = batch_create(2, 0, 5, NULL, 0, NULL);
    
    GenericBatch* batches[] = {batch1, batch2};
    GenericBatch* merged = batch_merge(batches, 2);
    
    if (!merged) {
        FAIL("Failed to merge batches");
        batch_free(batch1);
        batch_free(batch2);
        return;
    }
    
    if (merged->batch_size != 10) {
        FAIL("Incorrect merged size");
        batch_free(merged);
        batch_free(batch1);
        batch_free(batch2);
        return;
    }
    
    batch_free(merged);
    batch_free(batch1);
    batch_free(batch2);
    PASS();
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main() {
    printf("=== Batch Processing Tests ===\n\n");
    
    // Batch tests
    test_batch_creation();
    test_batch_reference_counting();
    test_batch_processing_state();
    
    // Queue tests
    test_queue_creation();
    test_queue_enqueue_dequeue();
    test_queue_peek();
    test_queue_full();
    test_queue_close();
    test_queue_concurrent();
    
    // Pool tests
    test_pool_creation();
    test_pool_allocate_release();
    test_pool_statistics();
    test_pool_efficiency();
    
    // Split/merge tests
    test_batch_split();
    test_batch_merge();
    
    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Total:  %d\n", tests_passed + tests_failed);
    
    if (tests_failed == 0) {
        printf("\n✓ All tests passed!\n");
        return 0;
    } else {
        printf("\n✗ Some tests failed!\n");
        return 1;
    }
}
