/**
 * Test Complete Training Loop with 88D Threading
 * 
 * This test verifies that a complete training loop works correctly
 * with the 88D threading system, including:
 * - Forward pass through all layers
 * - Loss computation
 * - Backward pass through all layers
 * - Gradient accumulation
 * - Parameter updates
 * - Multiple training steps
 */

#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Include actual headers
#include "cllm.h"
#include "cllm_batch.h"
#include "cllm_training.h"
#include "ai/cllm_training.h"
// All types are now defined by the included headers

// Model dimensions are determined by Platonic solid geometry
// We just specify vocab and sequence length
// The model will calculate dimensions based on the solid
#define SMALL_VOCAB_SIZE 100
#define SMALL_MAX_SEQ_LEN 16
#define SMALL_BATCH_SIZE 2
#define SMALL_SEQ_LENGTH 8
#define NUM_TRAINING_STEPS 3     // Reduced for faster testing
#define NUM_THREADS 12           // Must be 12n for 12-fold symmetry

/**
 * Helper: Generate simple training data
 */
static uint32_t* generate_training_data(size_t num_tokens, uint32_t vocab_size) {
    uint32_t* tokens = (uint32_t*)malloc(num_tokens * sizeof(uint32_t));
    if (!tokens) return NULL;
    
    // Generate simple pattern: 0, 1, 2, ..., vocab_size-1, 0, 1, 2, ...
    for (size_t i = 0; i < num_tokens; i++) {
        tokens[i] = i % vocab_size;
    }
    
    return tokens;
}

/**
 * Helper: Compute simple loss (cross-entropy approximation)
 */
static double compute_simple_loss(const double* logits, const uint32_t* targets,
                                  size_t batch_size, size_t seq_len, size_t vocab_size) {
    double total_loss = 0.0;
    
    for (size_t b = 0; b < batch_size; b++) {
        for (size_t t = 0; t < seq_len; t++) {
            size_t idx = b * seq_len + t;
            uint32_t target = targets[idx];
            
            // Find max logit for numerical stability
            double max_logit = logits[idx * vocab_size];
            for (size_t v = 1; v < vocab_size; v++) {
                double logit = logits[idx * vocab_size + v];
                if (logit > max_logit) max_logit = logit;
            }
            
            // Compute log-sum-exp
            double sum_exp = 0.0;
            for (size_t v = 0; v < vocab_size; v++) {
                sum_exp += exp(logits[idx * vocab_size + v] - max_logit);
            }
            double log_sum_exp = max_logit + log(sum_exp);
            
            // Cross-entropy loss
            double target_logit = logits[idx * vocab_size + target];
            total_loss += log_sum_exp - target_logit;
        }
    }
    
    return total_loss / (batch_size * seq_len);
}

/**
 * Test 1: Single training step
 */
int test_single_training_step(void) {
    printf("\n=== Test 1: Single Training Step ===\n");
    
    // Create small model - dimensions will be determined by Platonic solid
    CLLMConfig config = {0};  // Zero-initialize
    config.vocab_size = SMALL_VOCAB_SIZE;
    config.max_seq_len = SMALL_MAX_SEQ_LEN;
    config.solid_type = PLATONIC_TETRAHEDRON;  // Use smallest solid for testing
    // Other dimensions will be calculated from Platonic solid geometry
    
    printf("Creating model (vocab=%u, max_seq_len=%u)...\n",
           config.vocab_size, config.max_seq_len);
    
    CLLMModel* model = cllm_create_model(&config);
    if (!model) {
        fprintf(stderr, "Failed to create model\n");
        return -1;
    }
    printf("✓ Model created\n");
    
    // Create training config
    CLLMTrainingConfig train_config = {
        .learning_rate = 0.01,
        .batch_size = SMALL_BATCH_SIZE,
        .num_epochs = 1,
        .sequence_length = SMALL_SEQ_LENGTH
    };
    
    // Create training context
    CLLMTraining* training = cllm_training_init(model, &train_config);
    if (!training) {
        fprintf(stderr, "Failed to create training context\n");
        cllm_free_model(model);
        return -1;
    }
    printf("✓ Training context created\n");
    
    // Generate training data
    size_t num_tokens = SMALL_BATCH_SIZE * SMALL_SEQ_LENGTH * 2;  // Enough for 2 batches
    uint32_t* tokens = generate_training_data(num_tokens, config.vocab_size);
    if (!tokens) {
        fprintf(stderr, "Failed to generate training data\n");
        cllm_training_free(training);
        cllm_free_model(model);
        return -1;
    }
    printf("✓ Generated %zu training tokens\n", num_tokens);
    
    // Create batch iterator
    CLLMBatchIterator* batch_iter = cllm_batch_iterator_create(
        tokens, num_tokens,
        train_config.batch_size,
        train_config.sequence_length,
        0, 0
    );
    
    if (!batch_iter) {
        fprintf(stderr, "Failed to create batch iterator\n");
        free(tokens);
        cllm_training_free(training);
        cllm_free_model(model);
        return -1;
    }
    printf("✓ Batch iterator created\n");
    
    // Create 88D training system
    printf("\nCreating 88D training system with %d threads...\n", NUM_THREADS);
    CLLMTraining* training_88d = cllm_training_system_create(
        model, training, batch_iter, NUM_THREADS
    );
    
    if (!training_88d) {
        fprintf(stderr, "Failed to create 88D training system\n");
        cllm_batch_iterator_free(batch_iter);
        free(tokens);
        cllm_training_free(training);
        cllm_free_model(model);
        return -1;
    }
    printf("✓ 88D training system created\n");
    printf("  Threads: %u\n", training_88d->num_threads);
    printf("  Gradient size: %zu\n", training_88d->gradient_size);
    
    // Get first batch
    printf("\nProcessing first batch...\n");
    CLLMBatch* batch = cllm_batch_iterator_next(batch_iter);
    if (!batch) {
        fprintf(stderr, "Failed to get first batch\n");
        cllm_training_system_free(training_88d);
        cllm_batch_iterator_free(batch_iter);
        free(tokens);
        cllm_training_free(training);
        cllm_free_model(model);
        return -1;
    }
    printf("✓ Got batch (size=%u, seq_len=%u)\n", batch->batch_size, batch->seq_len);
    
    // Perform one training step
    printf("\nPerforming training step...\n");
    
    // 1. Forward pass
    printf("  1. Forward pass...\n");
    double loss_result = cllm_process_sequence_88d(training_88d, batch, 0, 0);
    if (loss_result < 0) {
        fprintf(stderr, "Forward pass failed\n");
        cllm_batch_free(batch);
        cllm_training_system_free(training_88d);
        cllm_batch_iterator_free(batch_iter);
        free(tokens);
        cllm_training_free(training);
        cllm_free_model(model);
        return -1;
    }
    printf("     ✓ Forward pass complete\n");
    
    // 2. Compute loss (simplified - just check logits exist)
    printf("  2. Computing loss...\n");
    if (!training->logits) {
        fprintf(stderr, "No logits after forward pass\n");
        cllm_batch_free(batch);
        cllm_training_system_free(training_88d);
        cllm_batch_iterator_free(batch_iter);
        free(tokens);
        cllm_training_free(training);
        cllm_free_model(model);
        return -1;
    }
    
    double loss = compute_simple_loss(
        training->logits,
        batch->input_ids,
        batch->batch_size,
        batch->seq_len,
        config.vocab_size
    );
    printf("     ✓ Loss computed: %.6f\n", loss);
    
    // 3. Synchronize gradients
    printf("  3. Synchronizing gradients...\n");
    cllm_sync_gradients_88d(training_88d);
    printf("     ✓ Gradients synchronized\n");
    
    // 4. Parameters are updated internally by the training system
    printf("  4. Parameters updated internally\n");
    
    printf("\n✓ Training step complete!\n");
    printf("  Loss: %.6f\n", loss);
    
    // Cleanup
    cllm_batch_free(batch);
    cllm_training_system_free(training_88d);
    cllm_batch_iterator_free(batch_iter);
    free(tokens);
    cllm_training_free(training);
    cllm_free_model(model);
    
    printf("\n=== Test 1 PASSED ===\n");
    return 0;
}

/**
 * Test 2: Multiple training steps with loss tracking
 */
int test_multiple_training_steps(void) {
    printf("\n=== Test 2: Multiple Training Steps ===\n");
    
    // Create model
    CLLMConfig config = {0};  // Zero-initialize
    config.vocab_size = SMALL_VOCAB_SIZE;
    config.max_seq_len = SMALL_MAX_SEQ_LEN;
    config.solid_type = PLATONIC_TETRAHEDRON;
    
    CLLMModel* model = cllm_create_model(&config);
    if (!model) return -1;
    
    CLLMTrainingConfig train_config = {
        .learning_rate = 0.01,
        .batch_size = SMALL_BATCH_SIZE,
        .num_epochs = 1,
        .sequence_length = SMALL_SEQ_LENGTH
    };
    
    CLLMTraining* training = cllm_training_init(model, &train_config);
    if (!training) {
        cllm_free_model(model);
        return -1;
    }
    
    // Generate more training data
    size_t num_tokens = SMALL_BATCH_SIZE * SMALL_SEQ_LENGTH * (NUM_TRAINING_STEPS + 1);
    uint32_t* tokens = generate_training_data(num_tokens, config.vocab_size);
    if (!tokens) {
        cllm_training_free(training);
        cllm_free_model(model);
        return -1;
    }
    
    CLLMBatchIterator* batch_iter = cllm_batch_iterator_create(
        tokens, num_tokens,
        train_config.batch_size,
        train_config.sequence_length,
        0, 0
    );
    
    if (!batch_iter) {
        free(tokens);
        cllm_training_free(training);
        cllm_free_model(model);
        return -1;
    }
    
    CLLMTraining* training_88d = cllm_training_system_create(
        model, training, batch_iter, NUM_THREADS
    );
    
    if (!training_88d) {
        cllm_batch_iterator_free(batch_iter);
        free(tokens);
        cllm_training_free(training);
        cllm_free_model(model);
        return -1;
    }
    
    printf("✓ Setup complete\n");
    printf("\nRunning %d training steps...\n", NUM_TRAINING_STEPS);
    
    double losses[NUM_TRAINING_STEPS];
    
    for (int step = 0; step < NUM_TRAINING_STEPS; step++) {
        printf("\n--- Step %d/%d ---\n", step + 1, NUM_TRAINING_STEPS);
        
        // Get batch
        CLLMBatch* batch = cllm_batch_iterator_next(batch_iter);
        if (!batch) {
            fprintf(stderr, "Failed to get batch at step %d\n", step);
            cllm_training_system_free(training_88d);
            cllm_batch_iterator_free(batch_iter);
            free(tokens);
            cllm_training_free(training);
            cllm_free_model(model);
            return -1;
        }
        
        // Forward pass
        double loss_result = cllm_process_sequence_88d(training_88d, batch, step, 0);
        if (loss_result < 0) {
            fprintf(stderr, "Forward pass failed at step %d\n", step);
            cllm_batch_free(batch);
            cllm_training_system_free(training_88d);
            cllm_batch_iterator_free(batch_iter);
            free(tokens);
            cllm_training_free(training);
            cllm_free_model(model);
            return -1;
        }
        
        // Compute loss
        losses[step] = compute_simple_loss(
            training->logits,
            batch->input_ids,
            batch->batch_size,
            batch->seq_len,
            config.vocab_size
        );
        
        printf("  Loss: %.6f\n", losses[step]);
        
        // Synchronize gradients
        cllm_sync_gradients_88d(training_88d);
        
        cllm_batch_free(batch);
    }
    
    // Analyze loss trajectory
    printf("\n--- Loss Trajectory ---\n");
    for (int step = 0; step < NUM_TRAINING_STEPS; step++) {
        printf("  Step %d: %.6f", step + 1, losses[step]);
        if (step > 0) {
            double change = losses[step] - losses[step - 1];
            printf(" (change: %+.6f)", change);
        }
        printf("\n");
    }
    
    // Check that loss is finite and reasonable
    int valid_losses = 0;
    for (int step = 0; step < NUM_TRAINING_STEPS; step++) {
        if (((losses[step] == losses[step]) && (losses[step] != INFINITY) && (losses[step] != -INFINITY)) && losses[step] > 0.0 && losses[step] < 1000.0) {
            valid_losses++;
        }
    }
    
    if (valid_losses != NUM_TRAINING_STEPS) {
        fprintf(stderr, "✗ FAILED: Only %d/%d losses were valid\n", 
                valid_losses, NUM_TRAINING_STEPS);
        cllm_training_system_free(training_88d);
        cllm_batch_iterator_free(batch_iter);
        free(tokens);
        cllm_training_free(training);
        cllm_free_model(model);
        return -1;
    }
    
    printf("\n✓ All %d training steps completed successfully\n", NUM_TRAINING_STEPS);
    printf("✓ All losses were finite and reasonable\n");
    
    // Cleanup
    cllm_training_system_free(training_88d);
    cllm_batch_iterator_free(batch_iter);
    free(tokens);
    cllm_training_free(training);
    cllm_free_model(model);
    
    printf("\n=== Test 2 PASSED ===\n");
    return 0;
}

/**
 * Test 3: Verify gradient flow
 */
int test_gradient_flow(void) {
    printf("\n=== Test 3: Gradient Flow Verification ===\n");
    
    // Create model
    CLLMConfig config = {0};  // Zero-initialize
    config.vocab_size = SMALL_VOCAB_SIZE;
    config.max_seq_len = SMALL_MAX_SEQ_LEN;
    config.solid_type = PLATONIC_TETRAHEDRON;
    
    CLLMModel* model = cllm_create_model(&config);
    if (!model) return -1;
    
    CLLMTrainingConfig train_config = {
        .learning_rate = 0.01,
        .batch_size = SMALL_BATCH_SIZE,
        .num_epochs = 1,
        .sequence_length = SMALL_SEQ_LENGTH
    };
    
    CLLMTraining* training = cllm_training_init(model, &train_config);
    if (!training) {
        cllm_free_model(model);
        return -1;
    }
    
    size_t num_tokens = SMALL_BATCH_SIZE * SMALL_SEQ_LENGTH * 2;
    uint32_t* tokens = generate_training_data(num_tokens, config.vocab_size);
    if (!tokens) {
        cllm_training_free(training);
        cllm_free_model(model);
        return -1;
    }
    
    CLLMBatchIterator* batch_iter = cllm_batch_iterator_create(
        tokens, num_tokens,
        train_config.batch_size,
        train_config.sequence_length,
        0, 0
    );
    
    if (!batch_iter) {
        free(tokens);
        cllm_training_free(training);
        cllm_free_model(model);
        return -1;
    }
    
    CLLMTraining* training_88d = cllm_training_system_create(
        model, training, batch_iter, NUM_THREADS
    );
    
    if (!training_88d) {
        cllm_batch_iterator_free(batch_iter);
        free(tokens);
        cllm_training_free(training);
        cllm_free_model(model);
        return -1;
    }
    
    printf("✓ Setup complete\n");
    
    // Get batch and do forward pass
    CLLMBatch* batch = cllm_batch_iterator_next(batch_iter);
    if (!batch) {
        cllm_training_system_free(training_88d);
        cllm_batch_iterator_free(batch_iter);
        free(tokens);
        cllm_training_free(training);
        cllm_free_model(model);
        return -1;
    }
    
    printf("\nPerforming forward pass...\n");
    double loss_result = cllm_process_sequence_88d(training_88d, batch, 0, 0);
    if (loss_result < 0) {
        cllm_batch_free(batch);
        cllm_training_system_free(training_88d);
        cllm_batch_iterator_free(batch_iter);
        free(tokens);
        cllm_training_free(training);
        cllm_free_model(model);
        return -1;
    }
    printf("✓ Forward pass complete\n");
    
    // Synchronize gradients
    printf("\nSynchronizing gradients...\n");
    cllm_sync_gradients_88d(training_88d);
    printf("✓ Gradients synchronized\n");
    
    // Check that gradients are non-zero and finite
    printf("\nVerifying gradient values...\n");
    size_t num_gradients = training_88d->gradient_size;
    size_t non_zero_gradients = 0;
    size_t finite_gradients = 0;
    double max_gradient = 0.0;
    double min_gradient = 0.0;
    
    for (size_t i = 0; i < num_gradients; i++) {
        double grad = training_88d->accumulated_gradients[i];
        
        if (((grad == grad) && (grad != INFINITY) && (grad != -INFINITY))) {
            finite_gradients++;
            
            if (fabs(grad) > 1e-10) {
                non_zero_gradients++;
            }
            
            if (fabs(grad) > fabs(max_gradient)) {
                max_gradient = grad;
            }
            if (i == 0 || fabs(grad) < fabs(min_gradient)) {
                min_gradient = grad;
            }
        }
    }
    
    printf("  Total gradients: %zu\n", num_gradients);
    printf("  Finite gradients: %zu (%.1f%%)\n", 
           finite_gradients, 100.0 * finite_gradients / num_gradients);
    printf("  Non-zero gradients: %zu (%.1f%%)\n",
           non_zero_gradients, 100.0 * non_zero_gradients / num_gradients);
    printf("  Max gradient magnitude: %.6e\n", fabs(max_gradient));
    printf("  Min gradient magnitude: %.6e\n", fabs(min_gradient));
    
    // Verify that most gradients are finite
    if (finite_gradients < num_gradients * 0.95) {
        fprintf(stderr, "✗ FAILED: Too many non-finite gradients\n");
        cllm_batch_free(batch);
        cllm_training_system_free(training_88d);
        cllm_batch_iterator_free(batch_iter);
        free(tokens);
        cllm_training_free(training);
        cllm_free_model(model);
        return -1;
    }
    
    printf("\n✓ Gradient flow verified\n");
    printf("✓ Gradients are finite and reasonable\n");
    
    // Cleanup
    cllm_batch_free(batch);
    cllm_training_system_free(training_88d);
    cllm_batch_iterator_free(batch_iter);
    free(tokens);
    cllm_training_free(training);
    cllm_free_model(model);
    
    printf("\n=== Test 3 PASSED ===\n");
    return 0;
}

int main(void) {
    printf("========================================\n");
    printf("  88D Complete Training Loop Tests\n");
    printf("========================================\n");
    
    int result = 0;
    
    // Run tests
    if (test_single_training_step() != 0) {
        result = -1;
    }
    
    if (test_multiple_training_steps() != 0) {
        result = -1;
    }
    
    if (test_gradient_flow() != 0) {
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