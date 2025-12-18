/**
 * Simple CLLM Inference Example
 * 
 * Demonstrates basic usage of the CLLM for text generation
 */

#include "../include/cllm.h"
#include "../include/cllm_inference.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    printf("=== CLLM Simple Inference Example ===\n\n");
    
    // Print version information
    printf("Version: %s\n", cllm_get_version());
    printf("Build: %s\n\n", cllm_get_build_info());
    
    // Create configuration
    printf("Creating model configuration...\n");
    CLLMConfig* config = cllm_create_config(
        10000,  // vocab_size
        512,    // embedding_dim
        6,      // num_layers
        8,      // num_heads
        2048    // ff_dim
    );
    
    if (!config) {
        fprintf(stderr, "Failed to create configuration\n");
        return 1;
    }
    
    // Validate configuration
    if (!cllm_validate_config(config)) {
        fprintf(stderr, "Invalid configuration\n");
        cllm_free_config(config);
        return 1;
    }
    
    // Print configuration
    cllm_print_config(config);
    printf("\n");
    
    // Note: Actual model creation would require more setup
    // This is a simplified example showing the API usage
    
    printf("Configuration validated successfully!\n");
    printf("\nNote: Full model creation and inference requires:\n");
    printf("  1. Model weight initialization\n");
    printf("  2. Loading pre-trained weights (if available)\n");
    printf("  3. Setting up inference context\n");
    printf("  4. Tokenizing input text\n");
    printf("  5. Running forward pass\n");
    printf("  6. Sampling from output distribution\n");
    printf("  7. Detokenizing generated tokens\n");
    
    // Cleanup
    cllm_free_config(config);
    
    printf("\n=== Example Complete ===\n");
    return 0;
}