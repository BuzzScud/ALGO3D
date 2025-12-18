/**
 * Profile Kissing Spheres Initialization
 * 
 * Measures the actual bottleneck in initialization
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "cllm.h"
#include "cllm_utils.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s <vocab_size>\n", argv[0]);
        return 1;
    }
    
    uint32_t vocab_size = atoi(argv[1]);
    
    printf("\n=== Profiling Kissing Spheres Initialization ===\n");
    printf("Vocabulary size: %u\n", vocab_size);
    printf("Expected operations: %u × 12 × %u = %lu\n", 
           vocab_size, vocab_size, (unsigned long)(vocab_size * 12ULL * vocab_size));
    printf("\n");
    
    // Create minimal config
    CLLMConfig config = {
        .vocab_size = vocab_size,
        .embedding_dim = 64,  // Small for testing
        .num_layers = 1,
        .num_heads = 4,
        .ff_dim = 256,
        .max_seq_len = 128,
        .dropout = 0.1f
    };
    
    printf("Creating model...\n");
    clock_t start = clock();
    
    CLLMModel* model = cllm_create_model(&config);
    
    clock_t end = clock();
    double total_time = (double)(end - start) / CLOCKS_PER_SEC;
    
    if (!model) {
        printf("Failed to create model\n");
        return 1;
    }
    
    printf("\n=== Results ===\n");
    printf("Total time: %.2f seconds\n", total_time);
    printf("Time per point: %.2f ms\n", (total_time * 1000.0) / vocab_size);
    
    if (model->lattice_points) {
        uint32_t total_neighbors = 0;
        for (uint32_t i = 0; i < model->num_lattice_points; i++) {
            total_neighbors += model->lattice_points[i].num_neighbors;
        }
        printf("Total neighbors found: %u\n", total_neighbors);
        printf("Average neighbors per point: %.2f\n", 
               (double)total_neighbors / model->num_lattice_points);
    }
    
    cllm_free_model(model);
    
    return 0;
}