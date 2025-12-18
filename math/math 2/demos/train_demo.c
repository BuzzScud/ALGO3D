#include "cllm_training.h"
#include "cllm_tokenizer.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    printf("CLLM Training Demo\n");
    printf("==================\n\n");
    
    // Create model config
    CLLMConfig config = {
        .vocab_size = 50000,
        .d_model = 512,
        .n_layers = 6,
        .n_heads = 8,
        .d_ff = 2048,
        .max_seq_len = 512,
        .dropout = 0.1f
    };
    
    // Create model
    printf("Creating model...\n");
    CLLM* model = cllm_create(&config);
    if (!model) {
        fprintf(stderr, "Failed to create model\n");
        return 1;
    }
    
    // Create tokenizer
    printf("Creating tokenizer...\n");
    CLLMTokenizer* tokenizer = cllm_create_tokenizer(config.vocab_size);
    
    // Create training context
    printf("Creating training context...\n");
    TrainingContext* ctx = training_context_create(model, 12);
    
    // Setup data files
    char* data_files[] = {"data/train.txt"};
    int num_files = 1;
    
    // Train
    printf("\nStarting training...\n");
    train_model(ctx, data_files, num_files, tokenizer, 
                10, 32, 512);
    
    // Save checkpoint
    printf("\nSaving checkpoint...\n");
    CheckpointConfig* ckpt_config = checkpoint_config_create("checkpoints", 5, 1);
    save_checkpoint_with_rotation(model, ckpt_config, 10, 0.5f);
    
    // Test inference
    printf("\nTesting inference...\n");
    InferenceConfig* inf_config = inference_config_create(model, tokenizer);
    char* output = generate_text(inf_config, "Hello world");
    printf("Generated: %s\n", output);
    free(output);
    
    // Cleanup
    inference_config_free(inf_config);
    checkpoint_config_free(ckpt_config);
    training_context_free(ctx);
    cllm_free_tokenizer(tokenizer);
    cllm_free(model);
    
    printf("\nDemo complete!\n");
    return 0;
}