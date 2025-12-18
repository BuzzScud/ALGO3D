#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cllm.h"
#include "cllm_training.h"
#include "cllm_data_loader.h"

int main(int argc, char** argv) {
    printf("\n=== CLLM Simple Training and Inference Test ===\n\n");
    
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <data_directory>\n", argv[0]);
        return 1;
    }
    
    const char* data_dir = argv[1];
    
    // Step 1: Load data
    printf("Step 1: Loading data from %s\n", data_dir);
    CLLMDataLoader* loader = cllm_data_loader_create(data_dir, 1000, 16);
    if (!loader) {
        fprintf(stderr, "Failed to create data loader\n");
        return 1;
    }
    
    printf("  Loaded %d documents\n", loader->num_documents);
    printf("  Vocabulary size: %d\n", loader->vocab_size);
    
    // Step 2: Create model
    printf("\nStep 2: Creating model\n");
    CLLMConfig config = {
        .vocab_size = loader->vocab_size,
        .embedding_dim = 64,
        .num_layers = 2,
        .num_heads = 2,
        .seq_length = 16,
        .dropout_rate = 0.1
    };
    
    CLLMModel* model = cllm_create(&config);
    if (!model) {
        fprintf(stderr, "Failed to create model\n");
        cllm_data_loader_free(loader);
        return 1;
    }
    
    printf("  Model created successfully\n");
    printf("  Parameters: %zu\n", cllm_count_parameters(model));
    
    // Step 3: Train for a few steps
    printf("\nStep 3: Training for 10 batches\n");
    
    CLLMTrainingConfig train_config = {
        .learning_rate = 0.001,
        .batch_size = 4,
        .num_epochs = 1,
        .max_steps = 10,
        .gradient_clip = 1.0,
        .weight_decay = 0.01
    };
    
    for (int step = 0; step < 10; step++) {
        // Get a batch
        CLLMBatch* batch = cllm_data_loader_get_batch(loader, 4, 16);
        if (!batch) {
            fprintf(stderr, "Failed to get batch\n");
            break;
        }
        
        // Forward pass
        float loss = cllm_forward(model, batch->input_ids, batch->batch_size, batch->seq_length);
        
        // Backward pass
        cllm_backward(model);
        
        // Update weights
        cllm_optimizer_step(model, train_config.learning_rate);
        
        printf("  Step %d: loss = %.4f\n", step + 1, loss);
        
        cllm_batch_free(batch);
    }
    
    // Step 4: Test inference
    printf("\nStep 4: Testing inference\n");
    
    const char* prompt = "int main";
    printf("  Prompt: &quot;%s&quot;\n", prompt);
    printf("  Generated: &quot;");
    
    // Tokenize prompt
    uint32_t tokens[32];
    int num_tokens = 0;
    
    // Simple tokenization (just use first few vocab items for demo)
    for (int i = 0; i < 4 && prompt[i] != '\0'; i++) {
        tokens[num_tokens++] = (uint32_t)prompt[i] % loader->vocab_size;
    }
    
    // Generate 20 tokens
    for (int i = 0; i < 20; i++) {
        // Forward pass with current tokens
        float* logits = cllm_generate_logits(model, tokens, num_tokens);
        if (!logits) break;
        
        // Get most likely next token (greedy sampling)
        int next_token = 0;
        float max_logit = logits[0];
        for (int j = 1; j < loader->vocab_size; j++) {
            if (logits[j] > max_logit) {
                max_logit = logits[j];
                next_token = j;
            }
        }
        
        // Add to sequence
        if (num_tokens < 32) {
            tokens[num_tokens++] = next_token;
        }
        
        // Print token (simplified - just print index)
        printf("%d ", next_token);
        
        free(logits);
    }
    
    printf("&quot;\n");
    
    // Cleanup
    printf("\nStep 5: Cleanup\n");
    cllm_free(model);
    cllm_data_loader_free(loader);
    
    printf("\n=== Test Complete ===\n\n");
    
    return 0;
}