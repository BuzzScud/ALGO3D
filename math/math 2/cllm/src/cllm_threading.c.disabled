/**
 * CLLM Threading Implementation
 * 
 * CLLM-specific wrapper around generic sphere threading from algorithms layer.
 * Provides CLLM-specific functionality while using generic threading infrastructure.
 */

#include "../../include/cllm_threading.h"
#include "../../include/cllm_angular_position.h"
#include "../../include/cllm_mathematical_constants.h"
#include "../../include/prime_lattice_core.h"
#include "../../include/prime_float_math.h"
#include "../../include/cllm_metrics.h"
#include "../../include/clock_lattice.h"
#include "../../algorithms/include/sphere_threading.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// INITIALIZATION
// ============================================================================

CLLMThreading* cllm_threading_init(CLLMModel* model) {
    if (!model) {
        fprintf(stderr, "ERROR: NULL model\n");
        return NULL;
    }
    
    // Allocate threading context
    CLLMThreading* threading = calloc(1, sizeof(CLLMThreading));
    if (!threading) {
        fprintf(stderr, "ERROR: Failed to allocate threading context\n");
        return NULL;
    }
    
    // Create generic sphere threading model
    threading->base_model = sphere_threading_init(model->num_lattice_points);
    if (!threading->base_model) {
        fprintf(stderr, "ERROR: Failed to initialize sphere threading\n");
        free(threading);
        return NULL;
    }
    
    // Store CLLM model reference
    threading->cllm_model = model;
    
    // Allocate gradient buffer
    threading->gradient_size = model->vocab_size * model->embedding_dim;
    threading->gradient_buffer = calloc(threading->gradient_size, sizeof(float));
    if (!threading->gradient_buffer) {
        fprintf(stderr, "ERROR: Failed to allocate gradient buffer\n");
        sphere_threading_free(threading->base_model);
        free(threading);
        return NULL;
    }
    
    // Optimize cache locality
    sphere_optimize_cache_locality(
        threading->base_model,
        model->embedding_dim * sizeof(float)
    );
    
    return threading;
}

void cllm_threading_free(CLLMThreading* threading) {
    if (!threading) return;
    
    if (threading->base_model) {
        sphere_threading_free(threading->base_model);
    }
    
    if (threading->gradient_buffer) {
        free(threading->gradient_buffer);
    }
    
    free(threading);
}

// ============================================================================
// LATTICE POINT OPERATIONS
// ============================================================================

int cllm_threading_init_lattice_points(CLLMModel* model) {
    if (!model || !model->lattice_points) {
        fprintf(stderr, "ERROR: Invalid model or no lattice points\n");
        return -1;
    }
    
    printf("\n=== Initializing Sphere Threading (Geometric Pattern) ===\n");
    printf("Number of lattice points: %lu\n", (unsigned long)model->num_lattice_points);
    printf("Using deterministic clock geometry - INSTANT initialization\n");
    
    // Use generic sphere threading to assign neighbors
    // O(n) initialization using geometric pattern
    for (uint32_t point_id = 0; point_id < model->num_lattice_points; point_id++) {
        CLLMLatticePoint* point = &model->lattice_points[point_id];
        
        // Each point gets exactly 12 neighbors (one per symmetry group)
        // Neighbors are determined by clock geometry
        for (uint32_t group = 0; group < 12; group++) {
            uint32_t neighbor_id = sphere_find_neighbor_by_geometry(
                model->num_lattice_points,
                point_id,
                group
            );
            point->neighbors[group] = neighbor_id;
        }
        
        point->num_neighbors = 12;
    }
    
    printf("✓ All %lu points initialized with 12 neighbors\n", 
           (unsigned long)model->num_lattice_points);
    printf("✓ Initialization complete (O(n) complexity)\n");
    
    // Optimize cache locality
    printf("\n=== Optimizing Neighbor Cache Locality ===\n");
    cllm_threading_optimize_cache(model);
    printf("✓ Neighbors reordered for optimal cache performance\n");
    printf("==========================================\n\n");
    
    return 0;
}

int cllm_threading_optimize_cache(CLLMModel* model) {
    if (!model || !model->lattice_points) return -1;
    
    uint32_t embed_dim = model->embedding_dim;
    
    // For each lattice point, sort its neighbors by memory address
    for (uint32_t i = 0; i < model->num_lattice_points; i++) {
        CLLMLatticePoint* point = &model->lattice_points[i];
        
        // Simple bubble sort (only 12 elements, very fast)
        for (uint32_t pass = 0; pass < point->num_neighbors - 1; pass++) {
            for (uint32_t j = 0; j < point->num_neighbors - pass - 1; j++) {
                uint32_t id1 = point->neighbors[j];
                uint32_t id2 = point->neighbors[j + 1];
                
                // Compare memory addresses (embedding offsets)
                size_t addr1 = id1 * embed_dim;
                size_t addr2 = id2 * embed_dim;
                
                // Sort by address (ascending)
                if (addr1 > addr2) {
                    // Swap
                    point->neighbors[j] = id2;
                    point->neighbors[j + 1] = id1;
                }
            }
        }
    }
    
    return 0;
}

// ============================================================================
// PROCESSING
// ============================================================================

void cllm_threading_process_point(
    CLLMLatticePoint* point,
    CLLMModel* model,
    float* local_gradients,
    size_t gradient_size
) {
    if (!point || !model || !local_gradients) return;
    
    // This is a placeholder for the actual processing logic
    // In a full implementation, this would:
    // 1. Process the point using its 12 neighbors
    // 2. Use L(n,d,k,λ) formula for interaction strength
    // 3. Accumulate gradients
    
    (void)gradient_size; // Unused for now
}

// ============================================================================
// VALIDATION & STATISTICS
// ============================================================================

int cllm_threading_verify(CLLMModel* model) {
    if (!model || !model->lattice_points) return 0;
    
    int errors = 0;
    
    // Verify each lattice point
    for (uint32_t i = 0; i < model->num_lattice_points; i++) {
        CLLMLatticePoint* point = &model->lattice_points[i];
        
        // Check neighbor count
        if (point->num_neighbors != 12) {
            fprintf(stderr, "ERROR: Point %u has %u neighbors (expected 12)\n",
                    i, point->num_neighbors);
            errors++;
        }
        
        // Check neighbors
        for (uint32_t j = 0; j < point->num_neighbors && j < 12; j++) {
            uint32_t neighbor_id = point->neighbors[j];
            
            // Check bounds
            if (neighbor_id >= model->num_lattice_points) {
                fprintf(stderr, "ERROR: Point %u has invalid neighbor %u\n",
                        i, neighbor_id);
                errors++;
            }
        }
    }
    
    if (errors == 0) {
        printf("✓ Threading verification passed\n");
    } else {
        fprintf(stderr, "✗ Found %d errors in threading configuration\n", errors);
    }
    
    return (errors == 0) ? 1 : 0;
}

void cllm_threading_stats(CLLMModel* model) {
    if (!model || !model->lattice_points) return;
    
    printf("\n=== Threading Statistics ===\n");
    printf("Total lattice points: %lu\n", (unsigned long)model->num_lattice_points);
    printf("Neighbors per point: 12\n");
    printf("Total connections: %lu\n", (unsigned long)(model->num_lattice_points * 12));
    
    // Count points in each symmetry group
    int counts[12] = {0};
    for (uint32_t i = 0; i < model->num_lattice_points; i++) {
        int group = i % 12;
        counts[group]++;
    }
    
    printf("\nSymmetry distribution:\n");
    for (int i = 0; i < 12; i++) {
        printf("  Group %2d: %d points (%.1f%%)\n", 
               i, counts[i], 100.0 * counts[i] / model->num_lattice_points);
    }
    
    printf("============================\n\n");
}

// ============================================================================
// UI INTEGRATION
// ============================================================================

void cllm_threading_report_position(
    CLLMModel* model,
    CLLMMetrics* metrics,
    int thread_id
) {
    if (!model || !metrics || thread_id < 0) return;
    
    // Select a token to represent this thread
    uint32_t token_id = thread_id % model->vocab_size;
    if (token_id >= model->vocab_size) return;
    
    CLLMToken* token = &model->tokens[token_id];
    
    // Calculate angular position
    AngularPosition pos;
    double wavelength = get_phonetic_wavelength(token->token_str[0]);
    angular_position_calculate(token->prime_encoding, token_id, 0, wavelength, &pos);
    
    // Convert to 3D position for metrics
    double position[3];
    position[0] = (double)pos.theta;
    position[1] = (double)pos.distance_to_144000;
    position[2] = (double)token->symmetry_group;
    
    // Report position to metrics system
    cllm_metrics_update_thread(metrics, thread_id, THREAD_STATE_IDLE, 0.0, position);
}