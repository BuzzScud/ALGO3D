#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cllm.h"
#include "cllm_training.h"
#include "cllm_utils.h"

int main() {
    printf("=== Optimizer Debug Test ===\n\n");
    
    // Create model
    printf("Creating model...\n");
    CLLMModel* model = cllm_create_small_model();
    if (!model) {
        fprintf(stderr, "Failed to create model\n");
        return 1;
    }
    printf("Model created\n\n");
    
    // Create synthetic dataset
    size_t num_tokens = 16;
    uint32_t* tokens = (uint32_t*)malloc(num_tokens * sizeof(uint32_t));
    for (size_t i = 0; i < num_tokens; i++) {
        tokens[i] = (i % 4) + 1;
    }
    
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
    
    // Save initial embedding weight
    printf("Initial embedding[0][0] = %.10f\n", model->embeddings.embeddings[0]);
    printf("Initial embedding[1][0] = %.10f\n", model->embeddings.embeddings[128]);
    printf("\n");
    
    // Single training step
    uint32_t batch[4] = {1, 2, 3, 4};
    uint32_t target[4] = {2, 3, 4, 1};
    
    printf("Forward pass...\n");
    cllm_forward_training(training, batch);
    
    printf("Compute loss...\n");
    float loss1 = cllm_compute_loss(training, batch, target, 4);
    printf("Loss: %.6f\n\n", loss1);
    
    printf("Backward pass...\n");
    cllm_backward_training(training, target, NULL);
    
    // Check gradients before optimizer
    printf("Gradient[0] = %.10f\n", training->gradients[0]);
    printf("Gradient[128] = %.10f\n", training->gradients[128]);
    printf("accumulation_step before = %d\n", training->accumulation_step);
    printf("gradient_accumulation_steps = %d\n\n", training->config.gradient_accumulation_steps);
    
    printf("Optimizer step...\n");
    cllm_optimizer_step(training);
    
    printf("accumulation_step after = %d\n\n", training->accumulation_step);
    
    // Check if weights changed
    printf("After optimizer:\n");
    printf("Embedding[0][0] = %.10f\n", model->embeddings.embeddings[0]);
    printf("Embedding[1][0] = %.10f\n", model->embeddings.embeddings[128]);
    printf("Gradient[0] = %.10f\n", training->gradients[0]);
    printf("Gradient[128] = %.10f\n\n", training->gradients[128]);
    
    // Cleanup
    cllm_training_cleanup(training);
    free(tokens);
    cllm_free_model(model);
    
    return 0;
}