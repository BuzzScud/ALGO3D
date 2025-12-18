#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cllm.h"


int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <vocab_size> <output_file>\n", argv[0]);
        return 1;
    }
    
    uint32_t vocab_size = atoi(argv[1]);
    const char* output_file = argv[2];
    
    printf("Creating test model with vocab_size=%u\n", vocab_size);
    
    // Create model configuration
    CLLMConfig config = {
        .vocab_size = vocab_size,
        .embedding_dim = 128,
        .num_layers = 2,
        .num_heads = 4,
        .ff_dim = 512,
        .max_seq_len = 256,
        .dropout = 0.1f
    };
    
    printf("Creating model...\n");
    CLLMModel* model = cllm_create_model(&config);
    if (!model) {
        fprintf(stderr, "Failed to create model\n");
        return 1;
    }
    
    printf("Model created successfully\n");
    printf("  Vocab size: %lu\n", (unsigned long)model->vocab_size);
    printf("  Embedding dim: %lu\n", (unsigned long)model->embedding_dim);
    printf("  Num layers: %u\n", model->num_layers);
    
    printf("Saving model to %s...\n", output_file);
    if (cllm_write_model(model, output_file) != 0) {
        fprintf(stderr, "Failed to save model\n");
        cllm_free_model(model);
        return 1;
    }
    
    printf("Model saved successfully\n");
    
    cllm_free_model(model);
    printf("Test model creation complete\n");
    
    return 0;
}
