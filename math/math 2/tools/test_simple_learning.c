#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cllm.h"
#include "cllm_training.h"
#include "cllm_utils.h"

int main() {
    printf("=== Simple Learning Test ===\n\n");
    
    // Create model
    printf("Creating model...\n");
    CLLMModel* model = cllm_create_small_model();
    if (!model) {
        fprintf(stderr, "Failed to create model\n");
        return 1;
    }
    printf("Model created\n\n");
    
    // Create synthetic dataset
    printf("Creating dataset...\n");
    size_t num_tokens = 16;
    uint32_t* tokens = (uint32_t*)malloc(num_tokens * sizeof(uint32_t));
    for (size_t i = 0; i < num_tokens; i++) {
        tokens[i] = (i % 4) + 1;
    }
    printf("Dataset created: [1,2,3,4] x 4\n\n");
    
    // Create training config
    CLLMTrainingConfig config = {
        .num_epochs = 1,
        .batch_size = 1,
        .sequence_length = 4,
        .learning_rate = 0.01,
        .gradient_accumulation_steps = 1,
        .save_every = 1000,
        .use_mixed_precision = false
    };
    
    printf("Creating training context...\n");
    CLLMTraining* training = cllm_training_init(model, &config);
    if (!training) {
        fprintf(stderr, "Failed to create training\n");
        free(tokens);
        cllm_free_model(model);
        return 1;
    }
    
    training->tokens = tokens;
    training->num_tokens = num_tokens;
    printf("Training context created\n\n");
    
    // Single training step
    printf("Running single training step...\n");
    uint32_t batch[4] = {1, 2, 3, 4};
    uint32_t target[4] = {2, 3, 4, 1};
    
    printf("  Forward pass...\n");
    fflush(stdout);
    cllm_forward_training(training, batch);
    
    printf("  Compute loss...\n");
    fflush(stdout);
    float loss1 = cllm_compute_loss(training, batch, target, 4);
    printf("  Loss before training: %.6f\n", loss1);
    
    printf("  Backward pass...\n");
    fflush(stdout);
    cllm_backward_training(training, target, NULL);
    
    printf("  Optimizer step...\n");
    fflush(stdout);
    cllm_optimizer_step(training);
    
    printf("  Done!\n\n");
    
    // Second training step to see if loss changed
    printf("Running second training step...\n");
    cllm_forward_training(training, batch);
    float loss2 = cllm_compute_loss(training, batch, target, 4);
    printf("  Loss after 1 step: %.6f\n", loss2);
    printf("  Change: %.6f\n\n", loss2 - loss1);
    
    if (loss2 < loss1) {
        printf("✅ SUCCESS: Loss decreased! Model is learning!\n");
    } else if (loss2 > loss1) {
        printf("⚠️  WARNING: Loss increased\n");
    } else {
        printf("❌ FAIL: Loss unchanged\n");
    }
    
    // Cleanup
    cllm_training_cleanup(training);  // This already frees training
    free(tokens);
    cllm_free_model(model);
    
    return 0;
}