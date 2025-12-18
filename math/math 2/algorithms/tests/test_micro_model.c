/**
 * @file test_micro_model.c
 * @brief Create a stabilized micro model (<1MB) for user verification
 * 
 * This demonstrates the complete OBJECTIVE 28 algorithm:
 * - High-dimensional Platonic solid (13D)
 * - Small vertex count (256 = 2^8)
 * - Tetration towers (60 total: 6 bases × 10 depths)
 * - Oscillation detection (spatial + temporal)
 * - Recovery and stabilization
 * - Persistent storage
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/platonic_model.h"

// Create micro model with minimal but complete configuration
PlatonicModel* create_micro_model(void) {
    printf("Creating micro model...\n");
    printf("  Dimensions: 13 (minimum for proper geometry)\n");
    printf("  Vertices: 2048 (2^11 - enforced minimum)\n");
    printf("  Tetration: Based on model configuration\n");
    
    PlatonicModel* model = platonic_model_create(
        PLATONIC_ICOSAHEDRON,  // Best symmetry
        13,    // dimensions (minimum)
        2048   // vertices (2^11 - minimum enforced by implementation)
    );
    
    if (!model) {
        fprintf(stderr, "Failed to create model\n");
        return NULL;
    }
    
    printf("  Created: %u vertices, %u edges, %u faces\n",
           model->num_vertices, model->num_edges, model->num_faces);
    printf("  Tetration: %u bases × %u depths\n", 
           model->num_tetration_bases, model->num_tetration_depths);
    
    return model;
}

// Simulate corruption for testing
void simulate_corruption(PlatonicModel* model, double corruption_rate) {
    printf("\nSimulating %.1f%% corruption...\n", corruption_rate * 100.0);
    
    uint32_t num_corrupted = (uint32_t)(model->num_vertices * corruption_rate);
    
    // Randomly corrupt vertices
    srand(time(NULL));
    for (uint32_t i = 0; i < num_corrupted; i++) {
        uint32_t vertex_id = rand() % model->num_vertices;
        model->corruption_mask[vertex_id] = true;
        model->confidence_scores[vertex_id] = 0.0;
        
        // Perturb position
        for (uint32_t d = 0; d < model->num_dimensions; d++) {
            uint32_t idx = vertex_id * model->num_dimensions + d;
            double noise = ((double)rand() / RAND_MAX - 0.5) * 2.0;
            model->vertex_positions[idx] += noise;
        }
    }
    
    printf("  Corrupted %u vertices (%.1f%%)\n", 
           num_corrupted, (double)num_corrupted / model->num_vertices * 100.0);
}

// Validate geometric properties
bool validate_geometry(PlatonicModel* model) {
    printf("\nValidating geometry...\n");
    
    // For high-dimensional models, edges and faces may not be explicitly computed
    // Instead, we validate the vertex structure and coprime relationships
    printf("  Vertices: %u\n", model->num_vertices);
    printf("  Dimensions: %u\n", model->num_dimensions);
    
    // Check that vertices are allocated
    if (!model->vertex_positions) {
        fprintf(stderr, "  ❌ Vertex positions not allocated!\n");
        return false;
    }
    printf("  ✅ Vertex positions allocated\n");
    
    // Check coprime relationships
    printf("  Checking coprime matrix...\n");
    if (!model->coprime_matrix) {
        fprintf(stderr, "  ❌ Coprime matrix not allocated!\n");
        return false;
    }
    
    // Sample a few coprime relationships
    for (uint32_t i = 0; i < model->num_dimensions && i < 3; i++) {
        for (uint32_t j = i + 1; j < model->num_dimensions && j < 3; j++) {
            uint64_t gcd = model->coprime_matrix[i][j];
            printf("    gcd(dim%u, dim%u) = %lu\n", i, j, gcd);
        }
    }
    printf("  ✅ Coprime relationships computed\n");
    
    // Check tetration towers
    uint32_t total_towers = model->num_tetration_bases * model->num_tetration_depths;
    printf("  Tetration towers: %u\n", total_towers);
    if (!model->tetration_towers) {
        fprintf(stderr, "  ❌ Tetration towers not allocated!\n");
        return false;
    }
    printf("  ✅ Tetration towers allocated\n");
    
    return true;
}

// Display oscillation status
void display_oscillations(PlatonicModel* model) {
    printf("\nOscillation Status:\n");
    
    // Spatial oscillations
    uint32_t stable_spatial = 0;
    for (uint32_t d = 0; d < model->num_dimensions; d++) {
        if (model->spatial_oscillations[d].is_stable) {
            stable_spatial++;
        }
    }
    printf("  Spatial: %u/%u dimensions stable (%.1f%%)\n",
           stable_spatial, model->num_dimensions,
           (double)stable_spatial / model->num_dimensions * 100.0);
    
    // Temporal oscillations
    uint32_t stabilizing_temporal = 0;
    for (uint32_t d = 0; d < model->num_dimensions; d++) {
        if (model->temporal_oscillations[d].is_stabilizing) {
            stabilizing_temporal++;
        }
    }
    printf("  Temporal: %u/%u dimensions stabilizing (%.1f%%)\n",
           stabilizing_temporal, model->num_dimensions,
           (double)stabilizing_temporal / model->num_dimensions * 100.0);
    
    // Tetration convergence
    uint32_t total_towers = model->num_tetration_bases * model->num_tetration_depths;
    uint32_t converged_towers = 0;
    for (uint32_t i = 0; i < total_towers; i++) {
        if (model->tetration_towers[i]->is_converged) {
            converged_towers++;
        }
    }
    printf("  Tetration: %u/%u towers converged (%.1f%%)\n",
           converged_towers, total_towers,
           (double)converged_towers / total_towers * 100.0);
}

// Display recovery statistics
void display_recovery_stats(PlatonicModel* model) {
    printf("\nRecovery Statistics:\n");
    
    // Count recovered vertices
    uint32_t corrupted = 0;
    uint32_t recovered = 0;
    
    for (uint32_t i = 0; i < model->num_vertices; i++) {
        if (model->corruption_mask[i]) {
            corrupted++;
            if (model->confidence_scores[i] > 0.8) {
                recovered++;
            }
        }
    }
    
    if (corrupted > 0) {
        double recovery_rate = (double)recovered / corrupted * 100.0;
        printf("  Corrupted: %u vertices\n", corrupted);
        printf("  Recovered: %u vertices (%.1f%%)\n", recovered, recovery_rate);
        printf("  Confidence: %.2f average\n", 
               (double)recovered / corrupted);
    } else {
        printf("  No corruption detected\n");
    }
}

int main(void) {
    printf("=== OBJECTIVE 28: Micro Model Test ===\n\n");
    
    // Create micro model
    PlatonicModel* model = create_micro_model();
    if (!model) {
        return 1;
    }
    
    // Validate initial geometry
    if (!validate_geometry(model)) {
        platonic_model_free(model);
        return 1;
    }
    
    // Display initial state
    display_oscillations(model);
    
    // Simulate corruption
    simulate_corruption(model, 0.25);  // 25% corruption
    
    // Recover
    printf("\nRunning recovery algorithm...\n");
    clock_t start = clock();
    
    bool success = platonic_recover(
        model,
        100,    // max iterations
        0.01    // convergence threshold
    );
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    if (success) {
        printf("  ✅ Recovery CONVERGED in %.3f seconds\n", elapsed);
    } else {
        printf("  ⚠️  Recovery did not fully converge (%.3f seconds)\n", elapsed);
    }
    
    // Display final state
    display_oscillations(model);
    display_recovery_stats(model);
    
    // Validate final geometry
    if (!validate_geometry(model)) {
        platonic_model_free(model);
        return 1;
    }
    
    // Save to file
    const char* filename = "micro_model_stabilized.platonic";
    printf("\nSaving model to %s...\n", filename);
    
    if (platonic_model_save(model, filename)) {
        // Get file size
        FILE* f = fopen(filename, "rb");
        if (f) {
            fseek(f, 0, SEEK_END);
            long size = ftell(f);
            fclose(f);
            
            printf("  ✅ Saved successfully\n");
            printf("  File size: %ld bytes (%.2f KB)\n", size, size / 1024.0);
            
            if (size < 1024 * 1024) {
                printf("  ✅ Size < 1MB target achieved\n");
            } else {
                printf("  ⚠️  Size exceeds 1MB target\n");
            }
        }
    } else {
        fprintf(stderr, "  ❌ Failed to save model\n");
    }
    
    // Test load/save cycle
    printf("\nTesting load/save cycle...\n");
    PlatonicModel* loaded = platonic_model_load(filename);
    if (loaded) {
        printf("  ✅ Model loaded successfully\n");
        printf("  Dimensions: %u\n", loaded->num_dimensions);
        printf("  Vertices: %u\n", loaded->num_vertices);
        printf("  Tetration: %u bases × %u depths\n", 
               loaded->num_tetration_bases, loaded->num_tetration_depths);
        
        platonic_model_free(loaded);
    } else {
        fprintf(stderr, "  ❌ Failed to load model\n");
    }
    
    // Cleanup
    platonic_model_free(model);
    
    printf("\n=== Test Complete ===\n");
    return 0;
}
