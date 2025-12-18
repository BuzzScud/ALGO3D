/**
 * Unit Tests for Lock-Free Memory Access Patterns
 * 
 * Tests lock-free gradient accumulation, weight updates,
 * and atomic operations.
 */

#include "ai/cllm_lockfree_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
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

// ============================================================================
// TEST CASES - GRADIENT ACCUMULATOR
// ============================================================================

/**
 * Test: Create and destroy gradient accumulator
 */
int test_gradient_create_destroy() {
    LockFreeGradientAccumulator* acc = lockfree_gradient_accumulator_create(1000);
    assert(acc != NULL);
    assert(acc->size == 1000);
    assert(acc->gradients != NULL);
    
    lockfree_gradient_accumulator_destroy(acc);
    return 1;
}

/**
 * Test: Invalid size
 */
int test_gradient_invalid_size() {
    LockFreeGradientAccumulator* acc = lockfree_gradient_accumulator_create(0);
    assert(acc == NULL);
    return 1;
}

/**
 * Test: Single gradient accumulation
 */
int test_single_gradient_accumulation() {
    LockFreeGradientAccumulator* acc = lockfree_gradient_accumulator_create(10);
    assert(acc != NULL);
    
    // Accumulate gradients
    lockfree_accumulate_gradient(acc, 0, 1.5);
    lockfree_accumulate_gradient(acc, 0, 2.5);
    lockfree_accumulate_gradient(acc, 0, 3.0);
    
    // Check result
    double result = lockfree_get_gradient(acc, 0);
    assert(result == 7.0);
    
    lockfree_gradient_accumulator_destroy(acc);
    return 1;
}

/**
 * Test: Multiple gradient accumulation
 */
int test_multiple_gradient_accumulation() {
    LockFreeGradientAccumulator* acc = lockfree_gradient_accumulator_create(10);
    assert(acc != NULL);
    
    // Accumulate to different indices
    for (int i = 0; i < 10; i++) {
        lockfree_accumulate_gradient(acc, i, (double)i * 1.5);
    }
    
    // Verify results
    for (int i = 0; i < 10; i++) {
        double result = lockfree_get_gradient(acc, i);
        assert(result == (double)i * 1.5);
    }
    
    lockfree_gradient_accumulator_destroy(acc);
    return 1;
}

/**
 * Test: Gradient array accumulation
 */
int test_gradient_array_accumulation() {
    LockFreeGradientAccumulator* acc = lockfree_gradient_accumulator_create(100);
    assert(acc != NULL);
    
    // Create local gradients
    double local_gradients[100];
    for (int i = 0; i < 100; i++) {
        local_gradients[i] = (double)i * 0.1;
    }
    
    // Accumulate array
    size_t accumulated = lockfree_accumulate_gradients(acc, local_gradients, 100);
    assert(accumulated == 100);
    
    // Verify results
    for (int i = 0; i < 100; i++) {
        double result = lockfree_get_gradient(acc, i);
        assert(result == (double)i * 0.1);
    }
    
    lockfree_gradient_accumulator_destroy(acc);
    return 1;
}

/**
 * Test: Reset gradients
 */
int test_reset_gradients() {
    LockFreeGradientAccumulator* acc = lockfree_gradient_accumulator_create(10);
    assert(acc != NULL);
    
    // Accumulate some gradients
    for (int i = 0; i < 10; i++) {
        lockfree_accumulate_gradient(acc, i, 5.0);
    }
    
    // Reset
    lockfree_reset_gradients(acc);
    
    // Verify all zero
    for (int i = 0; i < 10; i++) {
        double result = lockfree_get_gradient(acc, i);
        assert(result == 0.0);
    }
    
    lockfree_gradient_accumulator_destroy(acc);
    return 1;
}

/**
 * Test: Gradient accumulator validation
 */
int test_gradient_validation() {
    LockFreeGradientAccumulator* acc = lockfree_gradient_accumulator_create(10);
    assert(acc != NULL);
    
    // Should be valid
    assert(lockfree_gradient_accumulator_validate(acc) == 1);
    
    // NULL should be invalid
    assert(lockfree_gradient_accumulator_validate(NULL) == 0);
    
    lockfree_gradient_accumulator_destroy(acc);
    return 1;
}

/**
 * Test: Gradient statistics
 */
int test_gradient_statistics() {
    LockFreeGradientAccumulator* acc = lockfree_gradient_accumulator_create(10);
    assert(acc != NULL);
    
    // Perform operations
    for (int i = 0; i < 100; i++) {
        lockfree_accumulate_gradient(acc, 0, 1.0);
    }
    
    // Check statistics
    uint64_t successful, failed, total, fallbacks;
    lockfree_gradient_accumulator_get_stats(acc, &successful, &failed, &total, &fallbacks);
    
    assert(total == 100);
    assert(successful > 0);  // Should have some successful CAS
    
    lockfree_gradient_accumulator_destroy(acc);
    return 1;
}

// ============================================================================
// TEST CASES - WEIGHT UPDATER
// ============================================================================

/**
 * Test: Create and destroy weight updater
 */
int test_weight_create_destroy() {
    LockFreeWeightUpdater* updater = lockfree_weight_updater_create(1000);
    assert(updater != NULL);
    assert(updater->size == 1000);
    assert(updater->weights != NULL);
    
    lockfree_weight_updater_destroy(updater);
    return 1;
}

/**
 * Test: Single weight update
 */
int test_single_weight_update() {
    LockFreeWeightUpdater* updater = lockfree_weight_updater_create(10);
    assert(updater != NULL);
    
    // Set initial weight
    lockfree_set_weight(updater, 0, 10.0);
    
    // Update weight
    lockfree_update_weight(updater, 0, -0.5);
    lockfree_update_weight(updater, 0, -0.3);
    
    // Check result
    double result = lockfree_get_weight(updater, 0);
    assert(result == 9.2);
    
    lockfree_weight_updater_destroy(updater);
    return 1;
}

/**
 * Test: Multiple weight updates
 */
int test_multiple_weight_updates() {
    LockFreeWeightUpdater* updater = lockfree_weight_updater_create(10);
    assert(updater != NULL);
    
    // Set initial weights
    for (int i = 0; i < 10; i++) {
        lockfree_set_weight(updater, i, (double)i * 10.0);
    }
    
    // Update weights
    for (int i = 0; i < 10; i++) {
        lockfree_update_weight(updater, i, -1.0);
    }
    
    // Verify results
    for (int i = 0; i < 10; i++) {
        double result = lockfree_get_weight(updater, i);
        assert(result == (double)i * 10.0 - 1.0);
    }
    
    lockfree_weight_updater_destroy(updater);
    return 1;
}

/**
 * Test: Weight array update
 */
int test_weight_array_update() {
    LockFreeWeightUpdater* updater = lockfree_weight_updater_create(100);
    assert(updater != NULL);
    
    // Set initial weights
    for (int i = 0; i < 100; i++) {
        lockfree_set_weight(updater, i, 100.0);
    }
    
    // Create deltas
    double deltas[100];
    for (int i = 0; i < 100; i++) {
        deltas[i] = -(double)i * 0.1;
    }
    
    // Update array
    size_t updated = lockfree_update_weights(updater, deltas, 100);
    assert(updated == 100);
    
    // Verify results
    for (int i = 0; i < 100; i++) {
        double result = lockfree_get_weight(updater, i);
        assert(result == 100.0 - (double)i * 0.1);
    }
    
    lockfree_weight_updater_destroy(updater);
    return 1;
}

/**
 * Test: Weight updater validation
 */
int test_weight_validation() {
    LockFreeWeightUpdater* updater = lockfree_weight_updater_create(10);
    assert(updater != NULL);
    
    // Should be valid
    assert(lockfree_weight_updater_validate(updater) == 1);
    
    // NULL should be invalid
    assert(lockfree_weight_updater_validate(NULL) == 0);
    
    lockfree_weight_updater_destroy(updater);
    return 1;
}

/**
 * Test: Weight statistics
 */
int test_weight_statistics() {
    LockFreeWeightUpdater* updater = lockfree_weight_updater_create(10);
    assert(updater != NULL);
    
    // Perform operations
    for (int i = 0; i < 100; i++) {
        lockfree_update_weight(updater, 0, 0.01);
    }
    
    // Check statistics
    uint64_t successful, failed, total, fallbacks;
    lockfree_weight_updater_get_stats(updater, &successful, &failed, &total, &fallbacks);
    
    assert(total == 100);
    assert(successful > 0);  // Should have some successful CAS
    
    lockfree_weight_updater_destroy(updater);
    return 1;
}

// ============================================================================
// CONCURRENT ACCESS TESTS
// ============================================================================

typedef struct {
    LockFreeGradientAccumulator* accumulator;
    int thread_id;
    int iterations;
} ThreadTestData;

void* gradient_accumulation_thread(void* arg) {
    ThreadTestData* data = (ThreadTestData*)arg;
    
    for (int i = 0; i < data->iterations; i++) {
        lockfree_accumulate_gradient(data->accumulator, 0, 1.0);
    }
    
    return NULL;
}

/**
 * Test: Concurrent gradient accumulation
 */
int test_concurrent_gradient_accumulation() {
    LockFreeGradientAccumulator* acc = lockfree_gradient_accumulator_create(10);
    assert(acc != NULL);
    
    // Create threads
    const int num_threads = 4;
    const int iterations = 250;
    pthread_t threads[num_threads];
    ThreadTestData thread_data[num_threads];
    
    // Start threads
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].accumulator = acc;
        thread_data[i].thread_id = i;
        thread_data[i].iterations = iterations;
        pthread_create(&threads[i], NULL, gradient_accumulation_thread, &thread_data[i]);
    }
    
    // Wait for threads
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Verify result
    double result = lockfree_get_gradient(acc, 0);
    assert(result == (double)(num_threads * iterations));
    
    lockfree_gradient_accumulator_destroy(acc);
    return 1;
}

typedef struct {
    LockFreeWeightUpdater* updater;
    int thread_id;
    int iterations;
} WeightThreadTestData;

void* weight_update_thread(void* arg) {
    WeightThreadTestData* data = (WeightThreadTestData*)arg;
    
    for (int i = 0; i < data->iterations; i++) {
        lockfree_update_weight(data->updater, 0, 0.01);
    }
    
    return NULL;
}

/**
 * Test: Concurrent weight updates
 */
int test_concurrent_weight_updates() {
    LockFreeWeightUpdater* updater = lockfree_weight_updater_create(10);
    assert(updater != NULL);
    
    // Set initial weight
    lockfree_set_weight(updater, 0, 0.0);
    
    // Create threads
    const int num_threads = 4;
    const int iterations = 250;
    pthread_t threads[num_threads];
    WeightThreadTestData thread_data[num_threads];
    
    // Start threads
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].updater = updater;
        thread_data[i].thread_id = i;
        thread_data[i].iterations = iterations;
        pthread_create(&threads[i], NULL, weight_update_thread, &thread_data[i]);
    }
    
    // Wait for threads
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Verify result (use epsilon for floating-point comparison)
    double result = lockfree_get_weight(updater, 0);
    double expected = (double)(num_threads * iterations) * 0.01;
    double epsilon = 0.0001;
    assert(fabs(result - expected) < epsilon);
    
    lockfree_weight_updater_destroy(updater);
    return 1;
}

/**
 * Test: Success rate calculation
 */
int test_success_rate() {
    LockFreeGradientAccumulator* acc = lockfree_gradient_accumulator_create(10);
    assert(acc != NULL);
    
    // Perform operations
    for (int i = 0; i < 100; i++) {
        lockfree_accumulate_gradient(acc, 0, 1.0);
    }
    
    // Calculate success rate
    double rate = lockfree_calculate_success_rate(&acc->stats);
    assert(rate >= 0.0 && rate <= 1.0);
    
    lockfree_gradient_accumulator_destroy(acc);
    return 1;
}

/**
 * Test: Large gradient array
 */
int test_large_gradient_array() {
    const size_t large_size = 10000;
    LockFreeGradientAccumulator* acc = lockfree_gradient_accumulator_create(large_size);
    assert(acc != NULL);
    
    // Create large gradient array
    double* gradients = malloc(large_size * sizeof(double));
    assert(gradients != NULL);
    
    for (size_t i = 0; i < large_size; i++) {
        gradients[i] = (double)i * 0.001;
    }
    
    // Accumulate
    size_t accumulated = lockfree_accumulate_gradients(acc, gradients, large_size);
    assert(accumulated == large_size);
    
    // Verify
    for (size_t i = 0; i < large_size; i++) {
        double result = lockfree_get_gradient(acc, i);
        assert(result == (double)i * 0.001);
    }
    
    free(gradients);
    lockfree_gradient_accumulator_destroy(acc);
    return 1;
}

/**
 * Test: Negative gradients
 */
int test_negative_gradients() {
    LockFreeGradientAccumulator* acc = lockfree_gradient_accumulator_create(10);
    assert(acc != NULL);
    
    // Accumulate positive and negative
    lockfree_accumulate_gradient(acc, 0, 10.0);
    lockfree_accumulate_gradient(acc, 0, -3.0);
    lockfree_accumulate_gradient(acc, 0, -2.0);
    
    double result = lockfree_get_gradient(acc, 0);
    assert(result == 5.0);
    
    lockfree_gradient_accumulator_destroy(acc);
    return 1;
}

/**
 * Test: Bounds checking
 */
int test_bounds_checking() {
    LockFreeGradientAccumulator* acc = lockfree_gradient_accumulator_create(10);
    assert(acc != NULL);
    
    // Try to access out of bounds
    int result = lockfree_accumulate_gradient(acc, 10, 1.0);
    assert(result == -1);
    
    double value = lockfree_get_gradient(acc, 10);
    assert(value == 0.0);
    
    lockfree_gradient_accumulator_destroy(acc);
    return 1;
}

// ============================================================================
// MAIN
// ============================================================================

int main() {
    printf("=== Lock-Free Memory Access Unit Tests ===\n\n");
    
    // Gradient accumulator tests
    TEST(gradient_create_destroy);
    TEST(gradient_invalid_size);
    TEST(single_gradient_accumulation);
    TEST(multiple_gradient_accumulation);
    TEST(gradient_array_accumulation);
    TEST(reset_gradients);
    TEST(gradient_validation);
    TEST(gradient_statistics);
    TEST(negative_gradients);
    TEST(bounds_checking);
    
    // Weight updater tests
    TEST(weight_create_destroy);
    TEST(single_weight_update);
    TEST(multiple_weight_updates);
    TEST(weight_array_update);
    TEST(weight_validation);
    TEST(weight_statistics);
    
    // Concurrent access tests
    TEST(concurrent_gradient_accumulation);
    TEST(concurrent_weight_updates);
    TEST(success_rate);
    TEST(large_gradient_array);
    
    printf("\n=== Test Summary ===\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Total:  %d\n", tests_passed + tests_failed);
    
    return tests_failed > 0 ? 1 : 0;
}