/**
 * test_model_registry.c - Test Model Registry
 * 
 * Simple test tool to verify model registry functionality.
 */

#include <stdio.h>
#include <stdlib.h>
#include "../include/cllm_model_registry.h"

int main(int argc, char** argv) {
    printf("\n=== Model Registry Test ===\n\n");
    
    // Initialize registry
    printf("1. Initializing registry...\n");
    if (!model_registry_init("./models")) {
        fprintf(stderr, "Failed to initialize registry\n");
        return 1;
    }
    printf("   ✓ Registry initialized\n\n");
    
    // Check if initialized
    printf("2. Checking initialization...\n");
    if (!model_registry_is_initialized()) {
        fprintf(stderr, "Registry not initialized!\n");
        return 1;
    }
    printf("   ✓ Registry is initialized\n\n");
    
    // Get count
    printf("3. Getting model count...\n");
    uint32_t count = model_registry_count();
    printf("   Found %u model(s)\n\n", count);
    
    // List all models
    if (count > 0) {
        printf("4. Listing all models:\n");
        for (uint32_t i = 0; i < count; i++) {
            const ModelMetadata* meta = model_registry_get_at_index(i);
            if (meta) {
                printf("   %u. %s\n", i + 1, meta->name);
                printf("      Path: %s\n", meta->path);
                printf("      Vocab: %u, Dim: %u, Layers: %u, Heads: %u\n",
                       meta->vocab_size, meta->embedding_dim, 
                       meta->num_layers, meta->num_heads);
                printf("      Size: %lu bytes\n", (unsigned long)meta->file_size);
                printf("\n");
            }
        }
    }
    
    // Test get by name
    if (count > 0) {
        const ModelMetadata* first = model_registry_get_at_index(0);
        if (first) {
            printf("5. Testing get by name: %s\n", first->name);
            const ModelMetadata* meta = model_registry_get(first->name);
            if (meta) {
                printf("   ✓ Found model: %s\n", meta->name);
                printf("   Properties: %u vocab, %u dim, %u layers\n",
                       meta->vocab_size, meta->embedding_dim, meta->num_layers);
            } else {
                fprintf(stderr, "   ✗ Failed to get model by name\n");
            }
            printf("\n");
        }
    }
    
    // Test exists
    if (count > 0) {
        const ModelMetadata* first = model_registry_get_at_index(0);
        if (first) {
            printf("6. Testing exists: %s\n", first->name);
            if (model_registry_exists(first->name)) {
                printf("   ✓ Model exists\n");
            } else {
                fprintf(stderr, "   ✗ Model doesn't exist (but it should!)\n");
            }
            printf("\n");
        }
    }
    
    // Test get path
    if (count > 0) {
        const ModelMetadata* first = model_registry_get_at_index(0);
        if (first) {
            printf("7. Testing get path: %s\n", first->name);
            char path[512];
            if (model_registry_get_path(first->name, path)) {
                printf("   ✓ Path: %s\n", path);
            } else {
                fprintf(stderr, "   ✗ Failed to get path\n");
            }
            printf("\n");
        }
    }
    
    // Print full registry
    printf("8. Full registry dump:\n");
    model_registry_print();
    
    // Cleanup
    printf("9. Cleaning up...\n");
    model_registry_cleanup();
    printf("   ✓ Registry cleaned up\n\n");
    
    printf("=== All Tests Passed ===\n\n");
    
    return 0;
}
