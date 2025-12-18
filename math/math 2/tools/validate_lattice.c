/**
 * Validate Kissing Spheres Tool
 * 
 * Validates that all lattice points have proper kissing spheres configuration
 * (12 neighbors, one from each symmetry group).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cllm.h"
#include "cllm_format.h"
#include "cllm_training.h"
#include "ai/cllm_lattice.h"

void print_usage(const char* program_name) {
    printf("Usage: %s <model.cllm>\n", program_name);
    printf("\n");
    printf("Validate kissing spheres configuration in a CLLM model.\n");
    printf("\n");
    printf("Checks:\n");
    printf("  - All points have 10-12 neighbors\n");
    printf("  - Neighbors are from different symmetry groups\n");
    printf("  - Symmetry group distribution is balanced\n");
    printf("\n");
    printf("Example:\n");
    printf("  %s model.cllm\n", program_name);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    const char* model_path = argv[1];
    
    printf("\n=== Kissing Spheres Validator ===\n\n");
    
    // Load model
    printf("Loading model from %s...\n", model_path);
    CLLMModel* model = cllm_read_model(model_path);
    if (!model) {
        fprintf(stderr, "ERROR: Failed to load model from %s\n", model_path);
        return 1;
    }
    
    printf("Model loaded successfully.\n");
    printf("  Vocabulary size: %lu\n", (unsigned long)model->vocab_size);
    printf("  Number of lattice points: %lu\n", (unsigned long)model->num_lattice_points);
    printf("\n");
    
    // Check if lattice points exist
    if (!model->lattice_points || model->num_lattice_points == 0) {
        printf("⚠ Warning: Model has no lattice points.\n");
        printf("Initializing kissing spheres...\n\n");
        
        // Allocate lattice points if needed
        if (!model->lattice_points && model->vocab_size > 0) {
            model->num_lattice_points = model->vocab_size;
            model->lattice_points = calloc(model->num_lattice_points, sizeof(CLLMLatticePoint));
            
            if (!model->lattice_points) {
                fprintf(stderr, "ERROR: Failed to allocate lattice points\n");
                free(model);
                return 1;
            }
            
            // Initialize point IDs
            for (uint32_t i = 0; i < model->num_lattice_points; i++) {
                model->lattice_points[i].point_id = i;
            }
        }
        
        // Initialize kissing spheres
        cllm_initialize_kissing_spheres(model);
    }
    
    // Display statistics
    cllm_kissing_spheres_stats(model);
    
    // Verify configuration
    int result = cllm_verify_kissing_spheres(model);
    
    printf("\n");
    if (result) {
        printf("✓ Kissing spheres configuration is VALID\n");
    } else {
        printf("✗ Kissing spheres configuration has ISSUES\n");
    }
    
    printf("\n");
    
    // Cleanup
    free(model);
    
    return result ? 0 : 1;
}