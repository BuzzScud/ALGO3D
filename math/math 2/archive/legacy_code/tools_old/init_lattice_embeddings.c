/**
 * Initialize Lattice Embeddings Tool
 * 
 * Loads a CLLM model and initializes embeddings using the L(n,d,k,λ) formula
 * instead of random initialization.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/cllm.h"
#include "../include/cllm_format.h"
#include "../include/cllm_training.h"
#include "../include/cllm_inference.h"

void print_usage(const char* program_name) {
    printf("Usage: %s <input_model.cllm> <output_model.cllm>\n", program_name);
    printf("\n");
    printf("Initialize model embeddings using L(n,d,k,λ) lattice formula.\n");
    printf("\n");
    printf("Arguments:\n");
    printf("  input_model.cllm   - Input model file\n");
    printf("  output_model.cllm  - Output model file with lattice embeddings\n");
    printf("\n");
    printf("Example:\n");
    printf("  %s model.cllm model_lattice.cllm\n", program_name);
}

int main(int argc, char** argv) {
    if (argc < 3) {
        print_usage(argv[0]);
        return 1;
    }
    
    const char* input_path = argv[1];
    const char* output_path = argv[2];
    
    printf("\n=== Lattice Embedding Initializer ===\n\n");
    
    // Load model
    printf("Loading model from %s...\n", input_path);
    CLLMModel* model = cllm_read_model(input_path);
    if (!model) {
        fprintf(stderr, "ERROR: Failed to load model from %s\n", input_path);
        return 1;
    }
    
    printf("Model loaded successfully.\n");
    printf("  Vocabulary size: %lu\n", (unsigned long)model->vocab_size);
    printf("  Embedding dimension: %u\n", model->embedding_dim);
    printf("  Number of layers: %u\n", model->num_layers);
    printf("\n");
    
    // Initialize embeddings with lattice formula
    printf("Initializing embeddings with L(n,d,k,λ) formula...\n");
    cllm_init_embeddings_with_lattice(model);
    
    // Save model
    printf("Saving model to %s...\n", output_path);
    if (!cllm_write_model(model, output_path)) {
        fprintf(stderr, "ERROR: Failed to save model to %s\n", output_path);
        free(model);
        return 1;
    }
    
    printf("Model saved successfully.\n");
    printf("\n✓ Lattice embedding initialization complete!\n\n");
    
    // Cleanup
    free(model);
    
    return 0;
}
