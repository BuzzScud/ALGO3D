/**
 * Comprehensive Test Suite for Lock-Free Queue
 * 
 * Tests the generic lock-free queue implementation.
 */

#include "../include/lock_free_queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>

// Test result tracking
static int tests_passed = 0;
static int tests_failed = 0;

// Color codes for output
#define COLOR_GREEN "\033[0;32m"
#define COLOR_RED "\033[0;31m"
#define COLOR_YELLOW "\033[0;33m"
#define COLOR_BLUE "\033[0;34m"
#define COLOR_RESET "\033[0m"

// Test macros
#define TEST_START(name) \
    printf(COLOR_BLUE "TEST: %s" COLOR_RESET "\n", name)

#define TEST_PASS(name) \
    do { \
        printf(COLOR_GREEN "✓ PASS: %s" COLOR_RESET "\n", name); \
        tests_passed++; \
    } while(0)

#define TEST_FAIL(name, reason) \
    do { \
        printf(COLOR_RED "✗ FAIL: %s - %s" COLOR_RESET "\n", name, reason); \
        tests_failed++; \
    } while(0)

#define ASSERT_TRUE(condition, message) \
    do { \
        if (!(condition)) { \
            TEST_FAIL(current_test, message); \
            return; \
        } \
    } while(0)

#define ASSERT_NOT_NULL(ptr, message) \
    ASSERT_TRUE((ptr) != NULL, message)

#define ASSERT_NULL(ptr, message) \
    ASSERT_TRUE((ptr) == NULL, message)

#define ASSERT_EQUAL(a, b, message) \
    ASSERT_TRUE((a) == (b), message)

// Thread test data
typedef struct {
    LockFreeQueue* queue;
    int thread_id;
    int num_iterations;
    int* results;
} ThreadTestData;

//=============================================================================
// Test 1: Basic Creation and Destruction
//=============================================================================
void test_basic_creation_destruction(void) {
    const char* current_test = "Basic Creation and Destruction";
    TEST_START(current_test);
    
    // Test unlimited queue
    LockFreeQueue* queue1 = lock_free_queue_create(0, false);
    ASSERT_NOT_NULL(queue1, "Failed to create unlimited queue");
    ASSERT_TRUE(lock_free_queue_is_empty(queue1), "New queue should be empty");
    lock_free_queue_free(queue1, NULL);
    
    // Test limited queue
    LockFreeQueue* queue2 = lock_free_queue_create(100, false);
    ASSERT_NOT_NULL(queue2, "Failed to create limited queue");
    ASSERT_EQUAL(queue2->max_queue_size, 100, "Max size not set correctly");
    lock_free_queue_free(queue2, NULL);
    
    // Test drop-on-full queue
    LockFreeQueue* queue3 = lock_free_queue_create(50, true);
    ASSERT_NOT_NULL(queue3, "Failed to create drop-on-full queue");
    ASSERT_TRUE(queue3->drop_on_full, "Drop-on-full not set");
    lock_free_queue_free(queue3, NULL);
    
    TEST_PASS(current_test);
}

//=============================================================================
// Test 2: Basic Enqueue and Dequeue
//=============================================================================
void test_basic_enqueue_dequeue(void) {
    const char* current_test = "Basic Enqueue and Dequeue";
    TEST_START(current_test);
    
    LockFreeQueue* queue = lock_free_queue_create(0, false);
    ASSERT_NOT_NULL(queue, "Failed to create queue");
    
    // Enqueue some items
    int data1 = 42;
    int data2 = 100;
    int data3 = 200;
    
    ASSERT_TRUE(lock_free_queue_enqueue(queue, &data1), "Failed to enqueue data1");
    ASSERT_TRUE(lock_free_queue_enqueue(queue, &data2), "Failed to enqueue data2");
    ASSERT_TRUE(lock_free_queue_enqueue(queue, &data3), "Failed to enqueue data3");
    
    // Check size
    ASSERT_EQUAL(lock_free_queue_size(queue), 3, "Queue size incorrect");
    ASSERT_TRUE(!lock_free_queue_is_empty(queue), "Queue should not be empty");
    
    // Dequeue and verify FIFO order
    int* result1 = (int*)lock_free_queue_dequeue(queue);
    ASSERT_NOT_NULL(result1, "Failed to dequeue");
    ASSERT_EQUAL(*result1, 42, "Wrong value dequeued");
    
    int* result2 = (int*)lock_free_queue_dequeue(queue);
    ASSERT_NOT_NULL(result2, "Failed to dequeue");
    ASSERT_EQUAL(*result2, 100, "Wrong value dequeued");
    
    int* result3 = (int*)lock_free_queue_dequeue(queue);
    ASSERT_NOT_NULL(result3, "Failed to dequeue");
    ASSERT_EQUAL(*result3, 200, "Wrong value dequeued");
    
    // Queue should be empty now
    ASSERT_TRUE(lock_free_queue_is_empty(queue), "Queue should be empty");
    ASSERT_NULL(lock_free_queue_dequeue(queue), "Dequeue from empty queue should return NULL");
    
    lock_free_queue_free(queue, NULL);
    TEST_PASS(current_test);
}

//=============================================================================
// Test 3: Peek Operation
//=============================================================================
void test_peek(void) {
    const char* current_test = "Peek Operation";
    TEST_START(current_test);
    
    LockFreeQueue* queue = lock_free_queue_create(0, false);
    ASSERT_NOT_NULL(queue, "Failed to create queue");
    
    // Peek at empty queue
    ASSERT_NULL(lock_free_queue_peek(queue), "Peek on empty queue should return NULL");
    
    // Enqueue item
    int data = 42;
    lock_free_queue_enqueue(queue, &data);
    
    // Peek should return the item without removing it
    int* peeked = (int*)lock_free_queue_peek(queue);
    ASSERT_NOT_NULL(peeked, "Peek failed");
    ASSERT_EQUAL(*peeked, 42, "Peeked wrong value");
    
    // Queue should still have the item
    ASSERT_EQUAL(lock_free_queue_size(queue), 1, "Peek should not remove item");
    
    // Dequeue should return the same item
    int* dequeued = (int*)lock_free_queue_dequeue(queue);
    ASSERT_NOT_NULL(dequeued, "Dequeue failed");
    ASSERT_EQUAL(*dequeued, 42, "Dequeued wrong value");
    
    lock_free_queue_free(queue, NULL);
    TEST_PASS(current_test);
}

//=============================================================================
// Test 4: Queue Size Limit
//=============================================================================
void test_size_limit(void) {
    const char* current_test = "Queue Size Limit";
    TEST_START(current_test);
    
    LockFreeQueue* queue = lock_free_queue_create(3, false);
    ASSERT_NOT_NULL(queue, "Failed to create queue");
    
    int data1 = 1, data2 = 2, data3 = 3, data4 = 4;
    
    // Fill queue to capacity
    ASSERT_TRUE(lock_free_queue_enqueue(queue, &data1), "Failed to enqueue 1");
    ASSERT_TRUE(lock_free_queue_enqueue(queue, &data2), "Failed to enqueue 2");
    ASSERT_TRUE(lock_free_queue_enqueue(queue, &data3), "Failed to enqueue 3");
    
    // Queue should be full
    ASSERT_TRUE(lock_free_queue_is_full(queue), "Queue should be full");
    
    // Next enqueue should fail
    ASSERT_TRUE(!lock_free_queue_enqueue(queue, &data4), "Enqueue should fail when full");
    
    // Dequeue one item
    lock_free_queue_dequeue(queue);
    
    // Now we should be able to enqueue again
    ASSERT_TRUE(lock_free_queue_enqueue(queue, &data4), "Should be able to enqueue after dequeue");
    
    lock_free_queue_free(queue, NULL);
    TEST_PASS(current_test);
}

//=============================================================================
// Test 5: Drop on Full
//=============================================================================
void test_drop_on_full(void) {
    const char* current_test = "Drop on Full";
    TEST_START(current_test);
    
    LockFreeQueue* queue = lock_free_queue_create(2, true);
    ASSERT_NOT_NULL(queue, "Failed to create queue");
    
    int data1 = 1, data2 = 2, data3 = 3;
    
    // Fill queue
    lock_free_queue_enqueue(queue, &data1);
    lock_free_queue_enqueue(queue, &data2);
    
    // Next enqueue should drop (return false)
    ASSERT_TRUE(!lock_free_queue_enqueue(queue, &data3), "Should drop when full");
    
    // Queue should still have only 2 items
    ASSERT_EQUAL(lock_free_queue_size(queue), 2, "Queue size should be 2");
    
    lock_free_queue_free(queue, NULL);
    TEST_PASS(current_test);
}

//=============================================================================
// Test 6: Statistics Tracking
//=============================================================================
void test_statistics(void) {
    const char* current_test = "Statistics Tracking";
    TEST_START(current_test);
    
    LockFreeQueue* queue = lock_free_queue_create(0, false);
    ASSERT_NOT_NULL(queue, "Failed to create queue");
    
    int data1 = 1, data2 = 2, data3 = 3;
    
    // Perform operations
    lock_free_queue_enqueue(queue, &data1);
    lock_free_queue_enqueue(queue, &data2);
    lock_free_queue_enqueue(queue, &data3);
    
    lock_free_queue_dequeue(queue);
    lock_free_queue_dequeue(queue);
    
    // Get statistics
    LockFreeQueueStatistics stats;
    lock_free_queue_get_statistics(queue, &stats);
    
    ASSERT_EQUAL(stats.total_enqueued, 3, "Enqueue count wrong");
    ASSERT_EQUAL(stats.total_dequeued, 2, "Dequeue count wrong");
    ASSERT_EQUAL(stats.current_size, 1, "Current size wrong");
    
    lock_free_queue_free(queue, NULL);
    TEST_PASS(current_test);
}

//=============================================================================
// Test 7: Clear Operation
//=============================================================================
void test_clear(void) {
    const char* current_test = "Clear Operation";
    TEST_START(current_test);
    
    LockFreeQueue* queue = lock_free_queue_create(0, false);
    ASSERT_NOT_NULL(queue, "Failed to create queue");
    
    // Add items
    int data1 = 1, data2 = 2, data3 = 3;
    lock_free_queue_enqueue(queue, &data1);
    lock_free_queue_enqueue(queue, &data2);
    lock_free_queue_enqueue(queue, &data3);
    
    ASSERT_EQUAL(lock_free_queue_size(queue), 3, "Queue should have 3 items");
    
    // Clear queue
    lock_free_queue_clear(queue, NULL);
    
    // Queue should be empty
    ASSERT_TRUE(lock_free_queue_is_empty(queue), "Queue should be empty after clear");
    ASSERT_EQUAL(lock_free_queue_size(queue), 0, "Queue size should be 0");
    
    lock_free_queue_free(queue, NULL);
    TEST_PASS(current_test);
}

//=============================================================================
// Test 8: Validation
//=============================================================================
void test_validation(void) {
    const char* current_test = "Validation";
    TEST_START(current_test);
    
    LockFreeQueue* queue = lock_free_queue_create(0, false);
    ASSERT_NOT_NULL(queue, "Failed to create queue");
    
    // Validate empty queue
    ASSERT_TRUE(lock_free_queue_validate(queue), "Empty queue should be valid");
    
    // Add items and validate
    int data1 = 1, data2 = 2;
    lock_free_queue_enqueue(queue, &data1);
    lock_free_queue_enqueue(queue, &data2);
    
    ASSERT_TRUE(lock_free_queue_validate(queue), "Queue with items should be valid");
    
    // Validate NULL
    ASSERT_TRUE(!lock_free_queue_validate(NULL), "NULL should be invalid");
    
    lock_free_queue_free(queue, NULL);
    TEST_PASS(current_test);
}

//=============================================================================
// Test 9: Multi-threaded Enqueue
//=============================================================================
void* producer_thread(void* arg) {
    ThreadTestData* data = (ThreadTestData*)arg;
    
    for (int i = 0; i < data->num_iterations; i++) {
        int* value = malloc(sizeof(int));
        *value = data->thread_id * 1000 + i;
        
        while (!lock_free_queue_enqueue(data->queue, value)) {
            // Retry if queue is full
            usleep(1);
        }
    }
    
    return NULL;
}

void test_multithreaded_enqueue(void) {
    const char* current_test = "Multi-threaded Enqueue";
    TEST_START(current_test);
    
    LockFreeQueue* queue = lock_free_queue_create(0, false);
    ASSERT_NOT_NULL(queue, "Failed to create queue");
    
    const int num_threads = 4;
    const int iterations = 100;
    pthread_t threads[num_threads];
    ThreadTestData thread_data[num_threads];
    
    // Create producer threads
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].queue = queue;
        thread_data[i].thread_id = i;
        thread_data[i].num_iterations = iterations;
        
        int result = pthread_create(&threads[i], NULL, producer_thread, &thread_data[i]);
        ASSERT_EQUAL(result, 0, "Failed to create thread");
    }
    
    // Wait for threads
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Verify all items were enqueued
    uint64_t expected_size = num_threads * iterations;
    ASSERT_EQUAL(lock_free_queue_size(queue), expected_size, "Not all items enqueued");
    
    // Cleanup
    lock_free_queue_clear(queue, free);
    lock_free_queue_free(queue, NULL);
    TEST_PASS(current_test);
}

//=============================================================================
// Test 10: Multi-threaded Dequeue
//=============================================================================
void* consumer_thread(void* arg) {
    ThreadTestData* data = (ThreadTestData*)arg;
    int consumed = 0;
    
    for (int i = 0; i < data->num_iterations; i++) {
        void* item = lock_free_queue_dequeue(data->queue);
        if (item != NULL) {
            free(item);
            consumed++;
        } else {
            usleep(1); // Wait a bit if queue is empty
            i--; // Retry
        }
    }
    
    data->results[data->thread_id] = consumed;
    return NULL;
}

void test_multithreaded_dequeue(void) {
    const char* current_test = "Multi-threaded Dequeue";
    TEST_START(current_test);
    
    LockFreeQueue* queue = lock_free_queue_create(0, false);
    ASSERT_NOT_NULL(queue, "Failed to create queue");
    
    // Pre-fill queue
    const int total_items = 400;
    for (int i = 0; i < total_items; i++) {
        int* value = malloc(sizeof(int));
        *value = i;
        lock_free_queue_enqueue(queue, value);
    }
    
    const int num_threads = 4;
    const int iterations = total_items / num_threads;
    pthread_t threads[num_threads];
    ThreadTestData thread_data[num_threads];
    int results[num_threads];
    
    // Create consumer threads
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].queue = queue;
        thread_data[i].thread_id = i;
        thread_data[i].num_iterations = iterations;
        thread_data[i].results = results;
        
        int result = pthread_create(&threads[i], NULL, consumer_thread, &thread_data[i]);
        ASSERT_EQUAL(result, 0, "Failed to create thread");
    }
    
    // Wait for threads
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Verify all items were consumed
    int total_consumed = 0;
    for (int i = 0; i < num_threads; i++) {
        total_consumed += results[i];
    }
    ASSERT_EQUAL(total_consumed, total_items, "Not all items consumed");
    ASSERT_TRUE(lock_free_queue_is_empty(queue), "Queue should be empty");
    
    lock_free_queue_free(queue, NULL);
    TEST_PASS(current_test);
}

//=============================================================================
// Test 11: Batch Operations
//=============================================================================
void test_batch_operations(void) {
    const char* current_test = "Batch Operations";
    TEST_START(current_test);
    
    LockFreeQueue* queue = lock_free_queue_create(0, false);
    ASSERT_NOT_NULL(queue, "Failed to create queue");
    
    // Prepare batch of items
    const int batch_size = 10;
    int data[batch_size];
    void* items[batch_size];
    
    for (int i = 0; i < batch_size; i++) {
        data[i] = i * 10;
        items[i] = &data[i];
    }
    
    // Batch enqueue
    uint64_t enqueued = lock_free_queue_enqueue_batch(queue, items, batch_size);
    ASSERT_EQUAL(enqueued, batch_size, "Not all items enqueued");
    ASSERT_EQUAL(lock_free_queue_size(queue), batch_size, "Queue size incorrect");
    
    // Batch dequeue
    void* dequeued_items[batch_size];
    uint64_t dequeued = lock_free_queue_dequeue_batch(queue, dequeued_items, batch_size);
    ASSERT_EQUAL(dequeued, batch_size, "Not all items dequeued");
    ASSERT_TRUE(lock_free_queue_is_empty(queue), "Queue should be empty");
    
    // Verify FIFO order
    for (int i = 0; i < batch_size; i++) {
        int* value = (int*)dequeued_items[i];
        ASSERT_EQUAL(*value, i * 10, "Wrong value in batch dequeue");
    }
    
    lock_free_queue_free(queue, NULL);
    TEST_PASS(current_test);
}

//=============================================================================
// Test 12: Memory Pool Operations
//=============================================================================
void test_memory_pool(void) {
    const char* current_test = "Memory Pool Operations";
    TEST_START(current_test);
    
    LockFreeQueue* queue = lock_free_queue_create(0, false);
    ASSERT_NOT_NULL(queue, "Failed to create queue");
    
    // Pre-allocate nodes
    uint64_t allocated = lock_free_queue_preallocate_nodes(queue, 100);
    ASSERT_EQUAL(allocated, 100, "Not all nodes pre-allocated");
    
    // Verify free count
    uint64_t free_count = atomic_load(&queue->free_count);
    ASSERT_EQUAL(free_count, 100, "Free count incorrect");
    
    // Trim free nodes
    uint64_t trimmed = lock_free_queue_trim_free_nodes(queue, 50);
    ASSERT_EQUAL(trimmed, 50, "Not all nodes trimmed");
    
    free_count = atomic_load(&queue->free_count);
    ASSERT_EQUAL(free_count, 50, "Free count after trim incorrect");
    
    lock_free_queue_free(queue, NULL);
    TEST_PASS(current_test);
}

//=============================================================================
// Main Test Runner
//=============================================================================
int main(void) {
    printf("\n");
    printf(COLOR_YELLOW "========================================\n" COLOR_RESET);
    printf(COLOR_YELLOW "  Lock-Free Queue Test Suite\n" COLOR_RESET);
    printf(COLOR_YELLOW "========================================\n" COLOR_RESET);
    printf("\n");
    
    // Run all tests
    test_basic_creation_destruction();
    test_basic_enqueue_dequeue();
    test_peek();
    test_size_limit();
    test_drop_on_full();
    test_statistics();
    test_clear();
    test_validation();
    test_multithreaded_enqueue();
    test_multithreaded_dequeue();
    test_batch_operations();
    test_memory_pool();
    
    // Print summary
    printf("\n");
    printf(COLOR_YELLOW "========================================\n" COLOR_RESET);
    printf(COLOR_YELLOW "  Test Summary\n" COLOR_RESET);
    printf(COLOR_YELLOW "========================================\n" COLOR_RESET);
    printf("Total Tests: %d\n", tests_passed + tests_failed);
    printf(COLOR_GREEN "Passed: %d\n" COLOR_RESET, tests_passed);
    printf(COLOR_RED "Failed: %d\n" COLOR_RESET, tests_failed);
    
    if (tests_failed == 0) {
        printf(COLOR_GREEN "\n✓ ALL TESTS PASSED!\n" COLOR_RESET);
        return 0;
    } else {
        printf(COLOR_RED "\n✗ SOME TESTS FAILED\n" COLOR_RESET);
        return 1;
    }
}
