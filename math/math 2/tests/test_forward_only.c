#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ai/cllm.h"
#include "ai/cllm_training.h"

int main() {
    printf("=== Testing Forward Pass Only (No Free) ===\n\n");
    fflush(stdout);
    
    // Create model
    printf("1. Creating model...\n");
    fflush(stdout);
    CLLMConfig config = cllm_default_config(PLATONIC_CUBE, 100);
    CLLMModel* model = cllm_create_model(&config);
    
    if (!model) {
        printf("FAIL: Model creation failed\n");
        return 1;
    }
    printf("   ✓ Model created\n\n");
    
    // Create training context
    printf("2. Creating training context...\n");
    fflush(stdout);
    
    CLLMTrainingConfig train_config = {
        .batch_size = 4,
        .sequence_length = 8,
        .learning_rate = 0.001,
        .num_epochs = 1,
        .weight_decay = 0.01,
        .gradient_clip = 1.0
    };
    strcpy(train_config.optimizer, "adam");
    
    CLLMTraining* training = (CLLMTraining*)calloc(1, sizeof(CLLMTraining));
    if (!training) {
        printf("FAIL: Training allocation failed\n");
        return 1;
    }
    
    training->model = model;
    training->config = train_config;
    printf("   ✓ Training context created\n\n");
    
    // Create input tokens
    printf("3. Creating input tokens...\n");
    fflush(stdout);
    int num_tokens = train_config.batch_size * train_config.sequence_length;
    uint32_t* input_tokens = (uint32_t*)malloc(num_tokens * sizeof(uint32_t));
    
    for (int i = 0; i < num_tokens; i++) {
        input_tokens[i] = i % model->vocab_size;
    }
    printf("   ✓ Created %d input tokens\n\n", num_tokens);
    
    // Run forward pass
    printf("4. Running forward pass...\n");
    fflush(stdout);
    
    fprintf(stderr, "DEBUG: About to call cllm_forward_training\n");
    
    double loss = cllm_forward_training(training, input_tokens);
    
    fprintf(stderr, "DEBUG: cllm_forward_training returned: %f\n", loss);
    
    printf("   ✓ Forward pass completed\n");
    printf("   Loss: %f\n\n", loss);
    
    // Cleanup (skip model free to avoid hang)
    free(input_tokens);
    free(training);
    
    printf("=== TEST PASSED (model not freed to avoid hang) ===\n");
    
    return 0;
}