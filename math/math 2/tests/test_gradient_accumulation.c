/**
 * Test Gradient Accumulation Across Threads
 * 
 * This test verifies that gradients are correctly accumulated across
 * multiple threads using the 88D shared memory system.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cllm.h"
#include "math/transcendental.h"
#include "cllm_batch.h"
#include "cllm_training.h"
#include "ai/cllm_training_system.h"

#define TEST_GRADIENT_SIZE 1000
#define TEST_NUM_THREADS 12
#define TEST_VALUE 1.0

/**
 * Test 1: Basic gradient accumulation
 * Each thread writes a known value, verify sum is correct
 */
int test_basic_accumulation(void) {
    printf("\n=== Test 1: Basic Gradient Accumulation ===\n");
    
    // Create a minimal model for testing
    CLLMConfig config = {
        .vocab_size = 100,
        .embedding_dim = 64,
        .num_layers = 2,
        .num_heads = 4,
        .hidden_dim = 256,
        .max_seq_len = 32,
        
    };
    
    CLLMModel* model = cllm_create_model(&config);
    if (!model) {
        fprintf(stderr, "Failed to create model\n");
        return -1;
    }
    
    // Create training config
    CLLMTrainingConfig train_config = {
        .learning_rate = 0.001,
        .batch_size = 4,
        .num_epochs = 1,
        .sequence_length = 32
    };
    
    // Create minimal training context
    CLLMTraining* training = cllm_training_init(model, &train_config);
    if (!training) {
        fprintf(stderr, "Failed to create training context\n");
        cllm_free_model(model);
        return -1;
    }
    
    // Create dummy batch iterator
    uint32_t dummy_tokens[128];
    for (int i = 0; i < 128; i++) {
        dummy_tokens[i] = i % config.vocab_size;
    }
    
    CLLMBatchIterator* batch_iter = cllm_batch_iterator_create(
        dummy_tokens, 128,
        train_config.batch_size,
        train_config.sequence_length,
        0, 0
    );
    
    if (!batch_iter) {
        fprintf(stderr, "Failed to create batch iterator\n");
        cllm_training_free(training);
        cllm_free_model(model);
        return -1;
    }
    
    // Create 88D training system
    CLLMTrainingSystem* training_88d = cllm_training_system_create(
        model,
        training,
        batch_iter,
        TEST_NUM_THREADS
    );
    
    if (!training_88d) {
        fprintf(stderr, "Failed to create 88D training system\n");
        cllm_batch_iterator_free(batch_iter);
        cllm_training_free(training);
        cllm_free_model(model);
        return -1;
    }
    
    printf("✓ Created 88D training system with %u threads\n", training_88d->num_threads);
    printf("  Gradient size: %zu doubles\n", training_88d->gradient_size);
    
    // Test: Write known values to each thread's gradient buffer
    printf("\nWriting test values to gradient buffers...\n");
    for (uint32_t t = 0; t < training_88d->num_threads; t++) {
        double* gradient_buffer = (double*)training_88d->gradient_memory->base.data + 
                                  (t * training_88d->gradient_size);
        
        // Write thread ID as the value (so we can verify each thread wrote)
        for (size_t i = 0; i < training_88d->gradient_size && i < TEST_GRADIENT_SIZE; i++) {
            gradient_buffer[i] = (double)(t + 1);  // Thread ID + 1
        }
    }
    printf("✓ Wrote test values\n");
    
    // Synchronize gradients (should sum all thread contributions)
    printf("\nSynchronizing gradients...\n");
    cllm_system_sync_gradients(training_88d);
    printf("✓ Gradients synchronized\n");
    
    // Verify accumulated gradients
    printf("\nVerifying accumulated gradients...\n");
    double expected_sum = 0.0;
    for (uint32_t t = 0; t < training_88d->num_threads; t++) {
        expected_sum += (double)(t + 1);
    }
    double expected_avg = expected_sum / training_88d->num_threads;
    
    printf("  Expected average per gradient: %.2f\n", expected_avg);
    
    int errors = 0;
    for (size_t i = 0; i < TEST_GRADIENT_SIZE && i < training_88d->gradient_size; i++) {
        double actual = training_88d->accumulated_gradients[i];
        double diff = fabs(actual - expected_avg);
        
        if (diff > 1e-6) {
            if (errors < 5) {  // Only print first 5 errors
                fprintf(stderr, "  ERROR at index %zu: expected %.2f, got %.2f (diff: %.6f)\n",
                       i, expected_avg, actual, diff);
            }
            errors++;
        }
    }
    
    if (errors > 0) {
        fprintf(stderr, "✗ FAILED: %d gradient values incorrect\n", errors);
        cllm_training_system_free(training_88d);
        cllm_batch_iterator_free(batch_iter);
        cllm_training_free(training);
        cllm_free_model(model);
        return -1;
    }
    
    printf("✓ All gradient values correct!\n");
    
    // Cleanup
    cllm_training_system_free(training_88d);
    cllm_batch_iterator_free(batch_iter);
    cllm_training_free(training);
    cllm_free_model(model);
    
    printf("\n=== Test 1 PASSED ===\n");
    return 0;
}

/**
 * Test 2: Gradient accumulation with different values per thread
 */
int test_varied_accumulation(void) {
    printf("\n=== Test 2: Varied Gradient Accumulation ===\n");
    
    // Similar setup as Test 1
    CLLMConfig config = {
        .vocab_size = 100,
        .embedding_dim = 64,
        .num_layers = 2,
        .num_heads = 4,
        .hidden_dim = 256,
        .max_seq_len = 32,
        
    };
    
    CLLMModel* model = cllm_create_model(&config);
    if (!model) return -1;
    
    CLLMTrainingConfig train_config = {
        .learning_rate = 0.001,
        .batch_size = 4,
        .num_epochs = 1,
        .sequence_length = 32
    };
    
    CLLMTraining* training = cllm_training_init(model, &train_config);
    if (!training) {
        cllm_free_model(model);
        return -1;
    }
    
    uint32_t dummy_tokens[128];
    for (int i = 0; i < 128; i++) {
        dummy_tokens[i] = i % config.vocab_size;
    }
    
    CLLMBatchIterator* batch_iter = cllm_batch_iterator_create(
        dummy_tokens, 128,
        train_config.batch_size,
        train_config.sequence_length,
        0, 0
    );
    
    if (!batch_iter) {
        cllm_training_free(training);
        cllm_free_model(model);
        return -1;
    }
    
    CLLMTrainingSystem* training_88d = cllm_training_system_create(
        model, training, batch_iter, TEST_NUM_THREADS
    );
    
    if (!training_88d) {
        cllm_batch_iterator_free(batch_iter);
        cllm_training_free(training);
        cllm_free_model(model);
        return -1;
    }
    
    printf("✓ Created 88D training system\n");
    
    // Test: Each thread writes different values based on gradient index
    printf("\nWriting varied test values...\n");
    for (uint32_t t = 0; t < training_88d->num_threads; t++) {
        double* gradient_buffer = (double*)training_88d->gradient_memory->base.data + 
                                  (t * training_88d->gradient_size);
        
        for (size_t i = 0; i < training_88d->gradient_size && i < TEST_GRADIENT_SIZE; i++) {
            // Each thread writes: (thread_id + 1) * (gradient_index + 1)
            gradient_buffer[i] = (double)((t + 1) * (i + 1));
        }
    }
    printf("✓ Wrote varied values\n");
    
    // Synchronize
    printf("\nSynchronizing gradients...\n");
    cllm_system_sync_gradients(training_88d);
    printf("✓ Synchronized\n");
    
    // Verify
    printf("\nVerifying accumulated gradients...\n");
    int errors = 0;
    for (size_t i = 0; i < TEST_GRADIENT_SIZE && i < training_88d->gradient_size; i++) {
        // Expected: average of all thread contributions
        double expected_sum = 0.0;
        for (uint32_t t = 0; t < training_88d->num_threads; t++) {
            expected_sum += (double)((t + 1) * (i + 1));
        }
        double expected_avg = expected_sum / training_88d->num_threads;
        
        double actual = training_88d->accumulated_gradients[i];
        double diff = fabs(actual - expected_avg);
        
        if (diff > 1e-6) {
            if (errors < 5) {
                fprintf(stderr, "  ERROR at index %zu: expected %.2f, got %.2f\n",
                       i, expected_avg, actual);
            }
            errors++;
        }
    }
    
    if (errors > 0) {
        fprintf(stderr, "✗ FAILED: %d gradient values incorrect\n", errors);
        cllm_training_system_free(training_88d);
        cllm_batch_iterator_free(batch_iter);
        cllm_training_free(training);
        cllm_free_model(model);
        return -1;
    }
    
    printf("✓ All gradient values correct!\n");
    
    // Cleanup
    cllm_training_system_free(training_88d);
    cllm_batch_iterator_free(batch_iter);
    cllm_training_free(training);
    cllm_free_model(model);
    
    printf("\n=== Test 2 PASSED ===\n");
    return 0;
}

int main(void) {
    printf("========================================\n");
    printf("  88D Gradient Accumulation Tests\n");
    printf("========================================\n");
    
    int result = 0;
    
    // Run tests
    if (test_basic_accumulation() != 0) {
        result = -1;
    }
    
    if (test_varied_accumulation() != 0) {
        result = -1;
    }
    
    if (result == 0) {
        printf("\n========================================\n");
        printf("  ✓ ALL TESTS PASSED\n");
        printf("========================================\n");
    } else {
        printf("\n========================================\n");
        printf("  ✗ SOME TESTS FAILED\n");
        printf("========================================\n");
    }
    
    return result;
}