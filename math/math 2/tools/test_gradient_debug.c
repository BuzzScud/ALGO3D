#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "math/math.h"
#include "cllm.h"
#include "cllm_training.h"
#include "cllm_utils.h"
#include "cllm_data_loader.h"
#include "cllm_tokenizer.h"

// Helper to print gradient statistics
void print_gradient_stats(const char* name, double* grads, size_t size) {
    if (!grads || size == 0) {
        printf("  %s: NULL or empty\n", name);
        return;
    }
    
    double sum = 0.0, max_abs = 0.0, min_abs = 1e10;
    size_t non_zero = 0;
    
    for (size_t i = 0; i < size; i++) {
        double abs_val = fabs(grads[i]);
        sum += abs_val;
        if (abs_val > max_abs) max_abs = abs_val;
        if (abs_val < min_abs && abs_val > 1e-15) min_abs = abs_val;
        if (abs_val > 1e-10) non_zero++;
    }
    
    double avg = sum / size;
    printf("  %s: non_zero=%zu/%zu (%.1f%%), max=%.6e, avg=%.6e, min=%.6e\n", 
           name, non_zero, size, 100.0 * non_zero / size, max_abs, avg, min_abs);
}

int main() {
    printf("=== Gradient Debug Test ===\n\n");
    
    // Create a tiny model
    printf("Creating model...\n");
    CLLMModel* model = cllm_create_small_model();
    if (!model) {
        fprintf(stderr, "Failed to create model\n");
        return 1;
    }
    printf("Model created: vocab=%u, embed_dim=%u, layers=%u\n\n", 
           model->vocab_size, model->embedding_dim, model->num_layers);
    
    // Create simple synthetic dataset
    printf("Creating synthetic dataset...\n");
    size_t num_tokens = 16;
    uint32_t* tokens = (uint32_t*)malloc(num_tokens * sizeof(uint32_t));
    if (!tokens) {
        fprintf(stderr, "Failed to allocate tokens\n");
        cllm_free_model(model);
        return 1;
    }
    
    // Create a simple repeating pattern: [1, 2, 3, 4, 1, 2, 3, 4, ...]
    for (size_t i = 0; i < num_tokens; i++) {
        tokens[i] = (i % 4) + 1;  // Tokens 1, 2, 3, 4
    }
    printf("Dataset created: %zu tokens\n", num_tokens);
    printf("Pattern: ");
    for (size_t i = 0; i < num_tokens && i < 16; i++) {
        printf("%u ", tokens[i]);
    }
    printf("\n\n");
    
    // Create training config
    CLLMTrainingConfig config = {
        .num_epochs = 1,
        .batch_size = 1,
        .sequence_length = 4,
        .learning_rate = 0.001,
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
    
    // Set dataset tokens
    training->tokens = tokens;
    training->num_tokens = num_tokens;
    training->total_batches = (num_tokens / (config.batch_size * config.sequence_length));
    
    printf("Training context created\n\n");
    
    // Print initial gradient buffer status
    printf("=== Initial Gradient Buffer Status ===\n");
    printf("Embedding gradients: %p\n", (void*)training->gradients);
    printf("Attention gradients: %p\n", (void*)training->attention_grads);
    printf("FF gradients: %p\n", (void*)training->ff_grads);
    printf("LN gradients: %p\n", (void*)training->ln_grads);
    printf("\n");
    
    // Prepare a single batch
    printf("=== Running Single Training Step ===\n");
    uint32_t batch_tokens[4];
    uint32_t target_tokens[4];
    
    // Use first 4 tokens from dataset
    for (int i = 0; i < 4 && i < num_tokens; i++) {
        batch_tokens[i] = tokens[i];
        target_tokens[i] = tokens[(i + 1) % num_tokens];
    }
    
    printf("Input tokens: [%u, %u, %u, %u]\n", 
           batch_tokens[0], batch_tokens[1], batch_tokens[2], batch_tokens[3]);
    printf("Target tokens: [%u, %u, %u, %u]\n\n", 
           target_tokens[0], target_tokens[1], target_tokens[2], target_tokens[3]);
    
    // Forward pass
    printf("Running forward pass...\n");
    cllm_forward_training(training, batch_tokens);
    
    // Compute loss
    float loss = cllm_compute_loss(training, batch_tokens, target_tokens, 4);
    printf("Loss: %.6f\n\n", loss);
    
    // Check logits
    printf("Checking logits...\n");
    if (training->logits) {
        double logit_sum = 0.0, logit_max = -1e10, logit_min = 1e10;
        size_t logit_size = config.batch_size * config.sequence_length * model->vocab_size;
        for (size_t i = 0; i < logit_size; i++) {
            logit_sum += training->logits[i];
            if (training->logits[i] > logit_max) logit_max = training->logits[i];
            if (training->logits[i] < logit_min) logit_min = training->logits[i];
        }
        printf("Logits: sum=%.6f, max=%.6f, min=%.6f, avg=%.6f\n\n", 
               logit_sum, logit_max, logit_min, logit_sum / logit_size);
    } else {
        printf("Logits: NULL\n\n");
    }
    
    // Backward pass
    printf("Running backward pass...\n");
    cllm_backward_training(training, target_tokens, NULL);
    printf("Backward pass complete\n\n");
    
    // Check gradients
    printf("=== Gradient Statistics After Backward Pass ===\n\n");
    
    // Embedding gradients
    if (training->gradients) {
        size_t embed_size = model->vocab_size * model->embedding_dim;
        print_gradient_stats("Embedding gradients", training->gradients, embed_size);
    } else {
        printf("Embedding gradients: NULL\n");
    }
    printf("\n");
    
    // Attention gradients
    if (training->attention_grads) {
        for (uint32_t layer = 0; layer < model->num_layers; layer++) {
            printf("Layer %u Attention:\n", layer);
            size_t attn_size = model->embedding_dim * model->embedding_dim;
            print_gradient_stats("  Query", training->attention_grads[layer].query_lattice, attn_size);
            print_gradient_stats("  Key", training->attention_grads[layer].key_lattice, attn_size);
            print_gradient_stats("  Value", training->attention_grads[layer].value_lattice, attn_size);
        }
    } else {
        printf("Attention gradients: NULL\n");
    }
    printf("\n");
    
    // FF gradients
    if (training->ff_grads && model->ff_layers) {
        for (uint32_t layer = 0; layer < model->num_layers; layer++) {
            printf("Layer %u FeedForward:\n", layer);
            FeedForwardLayer* ff = &model->ff_layers[layer];
            print_gradient_stats("  W1", training->ff_grads[layer].w1_lattice, 
                               ff->input_dim * ff->hidden_dim);
            print_gradient_stats("  W2", training->ff_grads[layer].w2_lattice, 
                               ff->hidden_dim * ff->output_dim);
            print_gradient_stats("  Bias1", training->ff_grads[layer].bias1, ff->hidden_dim);
            print_gradient_stats("  Bias2", training->ff_grads[layer].bias2, ff->output_dim);
        }
    } else {
        printf("FF gradients: NULL\n");
    }
    printf("\n");
    
    // LN gradients
    if (training->ln_grads && model->layer_norms) {
        for (uint32_t layer = 0; layer < model->num_layers; layer++) {
            printf("Layer %u LayerNorm:\n", layer);
            print_gradient_stats("  Gamma", training->ln_grads[layer].gamma, model->embedding_dim);
            print_gradient_stats("  Beta", training->ln_grads[layer].beta, model->embedding_dim);
        }
    } else {
        printf("LN gradients: NULL\n");
    }
    
    printf("\n=== Test Complete ===\n");
    
    // Cleanup
    cllm_training_cleanup(training);
    free(training);
    free(tokens);
    cllm_free_model(model);
    
    return 0;
}