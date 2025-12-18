#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "math/math.h"
#include "cllm.h"
#include "cllm_training.h"
#include "cllm_utils.h"

int main() {
    printf("=== Learning Verification Test ===\n\n");
    
    // Create a tiny model
    printf("Creating model...\n");
    CLLMModel* model = cllm_create_small_model();
    if (!model) {
        fprintf(stderr, "Failed to create model\n");
        return 1;
    }
    printf("Model created: vocab=%u, embed_dim=%u, layers=%u\n\n", 
           model->vocab_size, model->embedding_dim, model->num_layers);
    
    // Create simple synthetic dataset with clear pattern
    // Pattern: [1, 2, 3, 4] repeating
    printf("Creating synthetic dataset...\n");
    size_t num_tokens = 32;
    uint32_t* tokens = (uint32_t*)malloc(num_tokens * sizeof(uint32_t));
    if (!tokens) {
        fprintf(stderr, "Failed to allocate tokens\n");
        cllm_free_model(model);
        return 1;
    }
    
    for (size_t i = 0; i < num_tokens; i++) {
        tokens[i] = (i % 4) + 1;  // Tokens 1, 2, 3, 4
    }
    printf("Dataset: %zu tokens with pattern [1, 2, 3, 4] repeating\n\n", num_tokens);
    
    // Create training config
    CLLMTrainingConfig config = {
        .num_epochs = 1,
        .batch_size = 1,
        .sequence_length = 4,
        .learning_rate = 0.01,  // Higher learning rate for faster learning
        .gradient_accumulation_steps = 1,
        .save_every = 1000,
        .use_mixed_precision = false
    };
    
    printf("Creating training context...\n");
    fflush(stdout);
    CLLMTraining* training = cllm_training_init(model, &config);
    printf("After cllm_training_init, training=%p\n", (void*)training);
    fflush(stdout);
    
    if (!training) {
        fprintf(stderr, "Failed to create training\n");
        free(tokens);
        cllm_free_model(model);
        return 1;
    }
    
    printf("Setting tokens...\n");
    fflush(stdout);
    training->tokens = tokens;
    training->num_tokens = num_tokens;
    training->total_batches = (num_tokens / (config.batch_size * config.sequence_length));
    printf("Training context created\n\n");
    fflush(stdout);
    
    // Save initial weights for comparison
    printf("Saving initial embedding weights...\n");
    fflush(stdout);
    double* initial_embeddings = NULL;
    
    printf("Checking embeddings pointer: %p\n", (void*)model->embeddings.embeddings);
    fflush(stdout);
    
    if (model->embeddings.embeddings) {
        printf("Allocating initial_embeddings buffer...\n");
        fflush(stdout);
        initial_embeddings = (double*)malloc(5 * model->embedding_dim * sizeof(double));
        printf("Allocated: %p\n", (void*)initial_embeddings);
        fflush(stdout);
        
        if (initial_embeddings) {
            printf("Copying weights...\n");
            fflush(stdout);
            for (int i = 0; i < 5 && i < model->vocab_size; i++) {
                printf("Copying token %d...\n", i);
                fflush(stdout);
                memcpy(&initial_embeddings[i * model->embedding_dim],
                       &model->embeddings.embeddings[i * model->embedding_dim],
                       model->embedding_dim * sizeof(double));
            }
            printf("Initial weights saved\n");
            fflush(stdout);
        }
    } else {
        printf("Warning: Embeddings not initialized yet (lazy init)\n");
        fflush(stdout);
    }
    
    // Training loop
    printf("=== Training for 50 steps ===\n");
    fflush(stdout);
    int num_steps = 50;
    printf("Allocating losses array...\n");
    fflush(stdout);
    double losses[50];
    printf("Losses array allocated\n");
    fflush(stdout);
    
    for (int step = 0; step < num_steps; step++) {
        if (step == 0) printf("Starting step 0...\n");
        
        // Prepare batch
        uint32_t batch_tokens[4];
        uint32_t target_tokens[4];
        
        int start_idx = (step * 4) % num_tokens;
        for (int i = 0; i < 4; i++) {
            batch_tokens[i] = tokens[(start_idx + i) % num_tokens];
            target_tokens[i] = tokens[(start_idx + i + 1) % num_tokens];
        }
        
        if (step == 0) printf("Running forward pass...\n");
        // Forward pass
        cllm_forward_training(training, batch_tokens);
        
        if (step == 0) printf("Computing loss...\n");
        // Compute loss
        float loss = cllm_compute_loss(training, batch_tokens, target_tokens, 4);
        losses[step] = loss;
        
        if (step == 0) printf("Running backward pass...\n");
        // Backward pass
        cllm_backward_training(training, target_tokens, NULL);
        
        if (step == 0) printf("Running optimizer step...\n");
        // Optimizer step
        if (training->gradients) {
            cllm_optimizer_step(training);
        } else {
            printf("Warning: No gradients at step %d\n", step);
        }
        
        if (step % 10 == 0 || step < 5) {
            printf("Step %3d: Loss = %.6f\n", step, loss);
        }
    }
    
    printf("\n=== Training Complete ===\n\n");
    
    // Analyze loss trajectory
    printf("Loss Analysis:\n");
    printf("  Initial loss (step 0):  %.6f\n", losses[0]);
    printf("  Final loss (step 49):   %.6f\n", losses[49]);
    printf("  Loss change:            %.6f (%.1f%%)\n", 
           losses[49] - losses[0],
           100.0 * (losses[49] - losses[0]) / losses[0]);
    
    // Check if loss decreased
    if (losses[49] < losses[0]) {
        printf("  ✅ Loss DECREASED - Model is learning!\n");
    } else {
        printf("  ❌ Loss did not decrease - Model may not be learning\n");
    }
    printf("\n");
    
    // Check weight changes
    printf("Weight Change Analysis:\n");
    if (initial_embeddings) {
        double total_change = 0.0;
        double max_change = 0.0;
        int changed_weights = 0;
        
        for (int i = 0; i < 5 && i < model->vocab_size; i++) {
            for (int d = 0; d < model->embedding_dim; d++) {
                double initial = initial_embeddings[i * model->embedding_dim + d];
                double current = model->embeddings.embeddings[i * model->embedding_dim + d];
                double change = fabs(current - initial);
                
                total_change += change;
                if (change > max_change) max_change = change;
                if (change > 1e-6) changed_weights++;
            }
        }
        
        int total_weights = 5 * model->embedding_dim;
        printf("  Weights checked: %d\n", total_weights);
        printf("  Weights changed: %d (%.1f%%)\n", 
               changed_weights, 100.0 * changed_weights / total_weights);
        printf("  Max weight change: %.6e\n", max_change);
        printf("  Avg weight change: %.6e\n", total_change / total_weights);
        
        if (changed_weights > 0) {
            printf("  ✅ Weights CHANGED - Optimizer is working!\n");
        } else {
            printf("  ❌ Weights did not change - Optimizer may not be working\n");
        }
        
        free(initial_embeddings);
    }
    printf("\n");
    
    // Test inference on the learned pattern
    printf("=== Testing Inference ===\n");
    printf("Input sequence: [1, 2, 3] -> Expected: 4\n");
    
    uint32_t test_input[3] = {1, 2, 3};
    uint32_t generated[1];
    
    cllm_generate(model, test_input, 3, generated, 1, 1.0f);
    
    printf("Generated token: %u\n", generated[0]);
    if (generated[0] == 4) {
        printf("  ✅ CORRECT! Model learned the pattern!\n");
    } else {
        printf("  ⚠️  Incorrect (expected 4, got %u)\n", generated[0]);
        printf("  Note: May need more training steps\n");
    }
    
    printf("\n=== Test Complete ===\n");
    
    // Cleanup
    cllm_training_cleanup(training);
    free(training);
    free(tokens);
    cllm_free_model(model);
    
    return 0;
}