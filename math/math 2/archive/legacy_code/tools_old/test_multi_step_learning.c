#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/cllm.h"
#include "../include/cllm_training.h"
#include "../include/cllm_utils.h"

int main() {
    printf("=== Multi-Step Learning Test ===\n\n");
    
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
    printf("Dataset: [1,2,3,4] repeating\n\n");
    
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
    
    // Training loop
    printf("Training for 20 steps...\n");
    uint32_t batch[4] = {1, 2, 3, 4};
    uint32_t target[4] = {2, 3, 4, 1};
    
    float losses[20];
    
    for (int step = 0; step < 20; step++) {
        // Forward
        cllm_forward_training(training, batch);
        
        // Loss
        losses[step] = cllm_compute_loss(training, batch, target, 4);
        
        // Backward
        cllm_backward_training(training, target, NULL);
        
        // Update
        cllm_optimizer_step(training);
        
        if (step % 5 == 0 || step < 3) {
            printf("Step %2d: Loss = %.6f\n", step, losses[step]);
        }
    }
    
    printf("\n=== Results ===\n");
    printf("Initial loss (step 0):  %.6f\n", losses[0]);
    printf("Final loss (step 19):   %.6f\n", losses[19]);
    printf("Loss change:            %.6f (%.1f%%)\n", 
           losses[19] - losses[0],
           100.0 * (losses[19] - losses[0]) / losses[0]);
    
    // Check trend
    int decreasing = 0;
    for (int i = 1; i < 20; i++) {
        if (losses[i] < losses[i-1]) decreasing++;
    }
    
    printf("Steps with decreasing loss: %d/19 (%.1f%%)\n\n", 
           decreasing, 100.0 * decreasing / 19);
    
    if (losses[19] < losses[0] * 0.95) {
        printf("✅ SUCCESS: Loss decreased by >5%%! Model is learning!\n");
    } else if (losses[19] < losses[0]) {
        printf("⚠️  PARTIAL: Loss decreased slightly (<%5%%)\n");
    } else {
        printf("❌ FAIL: Loss did not decrease\n");
    }
    
    // Cleanup
    cllm_training_cleanup(training);
    free(tokens);
    cllm_free_model(model);
    
    return 0;
}
