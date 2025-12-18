#include "../include/cllm.h"
#include "../include/cllm_training.h"
#include "../include/prime_float_math.h"
#include "../include/bigfixed_core.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdio.h>

// Initialize BigFixed weights with lattice-aware patterns
void cllm_lattice_aware_init(CLLMModel* model, float scale) {
    if (!model || !model->weights) return;
    
    size_t total_weights = model->num_weights;
    
    // Check if using BigFixed
    if (1) {
        // Initialize BigFixed weights with small random values
        for (size_t i = 0; i < total_weights; i++) {
            double random_val = ((double)rand() / RAND_MAX) * 2.0 - 1.0;
            double scaled_val = random_val * scale;
            model->weights[i] = scaled_val;
        }
    } else {
        // Legacy float initialization (deprecated)
        // This path should not be used in new code
        fprintf(stderr, "Warning: Using deprecated float weight initialization\n");
    }
}

// Initialize with crystalline structure patterns using BigFixed
void cllm_crystalline_init(CLLMModel* model, float base_scale) {
    if (!model || !model->weights) return;
    
    size_t vocab_size = model->vocab_size;
    size_t hidden_size = model->embedding_dim;
    
    if (1) {
        // Create periodic patterns in BigFixed weight initialization
        for (size_t i = 0; i < vocab_size; i++) {
            for (size_t j = 0; j < hidden_size; j++) {
                size_t idx = i * hidden_size + j;
                
                // Use sine wave patterns for crystalline structure
                double phase = (double)(i + j) / (double)(vocab_size + hidden_size);
                double pattern = prime_sinf(2.0 * 3.141592653589793 * phase);
                
                double random_val = ((double)rand() / RAND_MAX) * 2.0 - 1.0;
                double final_val = (pattern * 0.3 + random_val * 0.7) * base_scale;
                
                model->weights[idx] = final_val;
            }
        }
    } else {
        fprintf(stderr, "Warning: Using deprecated float weight initialization\n");
    }
}

// Initialize with symmetry-preserving patterns using BigFixed
void cllm_symmetric_init(CLLMModel* model, float scale) {
    if (!model || !model->weights) return;
    
    size_t vocab_size = model->vocab_size;
    size_t hidden_size = model->embedding_dim;
    
    if (1) {
        // Initialize first half randomly with BigFixed
        for (size_t i = 0; i < vocab_size / 2; i++) {
            for (size_t j = 0; j < hidden_size; j++) {
                size_t idx = i * hidden_size + j;
                double random_val = ((double)rand() / RAND_MAX) * 2.0 - 1.0;
                double scaled_val = random_val * scale;
                model->weights[idx] = scaled_val;
            }
        }
        
        // Mirror to second half for symmetry
        for (size_t i = 0; i < vocab_size / 2; i++) {
            for (size_t j = 0; j < hidden_size; j++) {
                size_t src_idx = i * hidden_size + j;
                size_t dst_idx = (vocab_size - 1 - i) * hidden_size + j;
                // Copy BigFixed value
                model->weights[dst_idx] = model->weights[src_idx];
            }
        }
    } else {
        fprintf(stderr, "Warning: Using deprecated float weight initialization\n");
    }
}

// Initialize with hierarchical lattice structure using BigFixed
void cllm_hierarchical_lattice_init(CLLMModel* model, int num_levels, float base_scale) {
    if (!model || !model->weights || num_levels <= 0) return;
    
    size_t vocab_size = model->vocab_size;
    size_t hidden_size = model->embedding_dim;
    
    if (1) {
        // Divide weights into hierarchical levels
        size_t level_size = vocab_size / num_levels;
        
        for (int level = 0; level < num_levels; level++) {
            double level_scale = base_scale / (double)(level + 1);
            
            size_t start_idx = level * level_size;
            size_t end_idx = (level + 1) * level_size;
            if (end_idx > vocab_size) end_idx = vocab_size;
            
            for (size_t i = start_idx; i < end_idx; i++) {
                for (size_t j = 0; j < hidden_size; j++) {
                    size_t idx = i * hidden_size + j;
                    double random_val = ((double)rand() / RAND_MAX) * 2.0 - 1.0;
                    double scaled_val = random_val * level_scale;
                    model->weights[idx] = scaled_val;
                }
            }
        }
    } else {
        fprintf(stderr, "Warning: Using deprecated float weight initialization\n");
    }
}