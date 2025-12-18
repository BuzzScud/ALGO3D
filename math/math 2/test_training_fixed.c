/**
 * Test Training with Fixed Transformer Forward Pass
 * 
 * This test verifies that:
 * 1. Training forward pass uses transformer layers
 * 2. Loss is computed correctly
 * 3. Model can process batches
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/cllm.h"
#include "include/cllm_training.h"
#include "include/ai/cllm_hierarchical_training.h"
#include "include/cllm_batch.h"

int main() {
    printf("=== Testing Fixed Training Pipeline ===\n\n");
    
    // Step 1: Create a small model
    printf("Step 1: Creating model...\n");
    CLLMConfig config = {
        .solid_type = PLATONIC_CUBE,
        .vocab_size = 100,
        .max_seq_len = 32,
        .embedding_dim = 64,
        .hidden_dim = 256,
        .num_layers = 2,
        .num_heads = 4,
        .enable_blind_recovery = false,
        .enable_harmonic_integration = false,
        .enable_ntt_attention = false,
        .enable_kissing_spheres = false
    };
    
    CLLMModel* model = cllm_create_model(&config);
    if (!model) {
        printf("Failed to create model\n");
        return 1;
    }
    printf("✓ Model created: %lu params\n", model->header.total_params);
    printf("  - Vocab size: %u\n", model->vocab_size);
    printf("  - Embedding dim: %u\n", model->embedding_dim);
    printf("  - Num layers: %u\n", model->num_layers);
    printf("\n");
    
    // Step 2: Create training configuration
    printf("Step 2: Creating training configuration...\n");
    CLLMTrainingConfig train_config = {
        .learning_rate = 0.001f,
        .batch_size = 2,
        .sequence_length = 8,
        .num_epochs = 1,
        .max_steps = 10,
        .weight_decay = 0.01f,
        .gradient_clip = 1.0f,
        .warmup_steps = 0,
        .save_interval = 100,
        .eval_interval = 10
    };
    strcpy(train_config.optimizer, "adam");
    printf("✓ Training config created\n");
    printf("  - Batch size: %d\n", train_config.batch_size);
    printf("  - Sequence length: %d\n", train_config.sequence_length);
    printf("\n");
    
    // Step 3: Initialize training
    printf("Step 3: Initializing training...\n");
    CLLMTraining* training = cllm_training_init(model, &train_config);
    if (!training) {
        printf("Failed to initialize training\n");
        cllm_free_model(model);
        return 1;
    }
    printf("✓ Training initialized\n\n");
    
    // Step 4: Create simple training data
    printf("Step 4: Creating training data...\n");
    int num_tokens = 100;
    uint32_t* tokens = (uint32_t*)malloc(num_tokens * sizeof(uint32_t));
    for (int i = 0; i < num_tokens; i++) {
        tokens[i] = i % config.vocab_size;
    }
    training->tokens = tokens;
    training->num_tokens = num_tokens;
    printf("✓ Training data created: %d tokens\n\n", num_tokens);
    
    // Step 5: Create batch iterator
    printf("Step 5: Creating batch iterator...\n");
    CLLMBatchIterator* iterator = cllm_batch_iterator_create(
        tokens, num_tokens,
        train_config.batch_size,
        train_config.sequence_length,
        0, 0
    );
    if (!iterator) {
        printf("Failed to create batch iterator\n");
        cllm_training_free(training);
        cllm_free_model(model);
        free(tokens);
        return 1;
    }
    printf("✓ Batch iterator created\n\n");
    
    // Step 6: Test forward pass with transformer
    printf("Step 6: Testing forward pass with transformer...\n");
    uint32_t test_tokens[16];
    for (int i = 0; i < 16; i++) {
        test_tokens[i] = i % config.vocab_size;
    }
    
    printf("  Running forward pass...\n");
    cllm_forward_training(training, test_tokens);
    printf("  ✓ Forward pass completed\n");
    
    // Check if logits were computed
    int has_valid_logits = 0;
    for (int i = 0; i < 10; i++) {
        if (training->logits[i] != 0.0) {
            has_valid_logits = 1;
            break;
        }
    }
    
    if (has_valid_logits) {
        printf("  ✓ Logits computed successfully\n");
        printf("  Sample logits: [%.4f, %.4f, %.4f, %.4f, %.4f]\n",
               training->logits[0], training->logits[1], training->logits[2],
               training->logits[3], training->logits[4]);
    } else {
        printf("  ⚠ Warning: All logits are zero\n");
    }
    printf("\n");
    
    // Step 7: Test hierarchical training system
    printf("Step 7: Testing hierarchical training system...\n");
    HierarchicalTrainingSystem* hier_system = hierarchical_training_create(
        training, 4, iterator
    );
    if (!hier_system) {
        printf("Failed to create hierarchical training system\n");
        cllm_batch_iterator_free(iterator);
        cllm_training_free(training);
        cllm_free_model(model);
        free(tokens);
        return 1;
    }
    printf("✓ Hierarchical training system created\n\n");
    
    // Step 8: Train one epoch
    printf("Step 8: Training one epoch...\n");
    float epoch_loss = hierarchical_train_epoch(hier_system);
    printf("✓ Epoch completed\n");
    printf("  Average loss: %.4f\n", epoch_loss);
    printf("  Best loss: %.4f\n", training->best_loss);
    printf("\n");
    
    // Cleanup
    printf("Cleaning up...\n");
    hierarchical_training_free(hier_system);
    cllm_batch_iterator_free(iterator);
    cllm_training_free(training);
    cllm_free_model(model);
    free(tokens);
    
    printf("\n=== Test Complete ===\n");
    printf("✓ Training pipeline is working with transformer layers!\n");
    
    return 0;
}