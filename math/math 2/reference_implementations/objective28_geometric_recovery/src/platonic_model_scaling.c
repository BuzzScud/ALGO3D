/**
 * @file platonic_model_scaling.c
 * @brief Dynamic Scaling for Platonic Models
 * 
 * When oscillations don't stabilize, we INCREASE complexity:
 * - Add more dimensions
 * - Add more vertices
 * - Increase tetration depth
 * 
 * This is the key insight: if the model isn't complex enough to capture
 * the structure, make it MORE complex until it can.
 */

#include "platonic_model.h"
#include <stdlib.h>
#include <string.h>
#include "../../../include/prime_float_math.h"
#include <stdio.h>

bool platonic_scale_dimensions(
    PlatonicModel* model,
    uint32_t additional_dimensions
) {
    if (!model) return false;
    
    uint32_t new_dimensions = model->num_dimensions + additional_dimensions;
    
    // Check if we can scale
    if (new_dimensions > model->max_dimensions) {
        printf("⚠ Cannot scale dimensions: would exceed max (%u)\n", model->max_dimensions);
        return false;
    }
    
    printf("🔼 Scaling dimensions: %u → %u (+%u)\n", 
           model->num_dimensions, new_dimensions, additional_dimensions);
    
    // Allocate new vertex positions
    double* new_positions = calloc(
        model->num_vertices * new_dimensions,
        sizeof(double)
    );
    if (!new_positions) return false;
    
    // Copy existing positions
    for (uint32_t v = 0; v < model->num_vertices; v++) {
        // Copy old dimensions
        memcpy(&new_positions[v * new_dimensions],
               &model->vertex_positions[v * model->num_dimensions],
               model->num_dimensions * sizeof(double));
        
        // Initialize new dimensions using prime-based projection
        uint32_t primes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47};
        for (uint32_t d = model->num_dimensions; d < new_dimensions; d++) {
            double sum = 0.0;
            for (uint32_t i = 0; i < model->num_dimensions; i++) {
                double coord = model->vertex_positions[v * model->num_dimensions + i];
                uint32_t prime = primes[d % 15];
                sum += coord * math_cos(2.0 * M_PI * prime * d / new_dimensions);
            }
            new_positions[v * new_dimensions + d] = sum / math_sqrt((double)new_dimensions);
        }
    }
    
    // Replace old positions
    free(model->vertex_positions);
    model->vertex_positions = new_positions;
    
    // Reallocate coprime matrix
    uint64_t** new_coprime = calloc(new_dimensions, sizeof(uint64_t*));
    for (uint32_t i = 0; i < new_dimensions; i++) {
        new_coprime[i] = calloc(new_dimensions, sizeof(uint64_t));
        
        // Copy old values
        if (i < model->num_dimensions) {
            memcpy(new_coprime[i], model->coprime_matrix[i],
                   model->num_dimensions * sizeof(uint64_t));
        }
    }
    
    // Free old coprime matrix
    for (uint32_t i = 0; i < model->num_dimensions; i++) {
        free(model->coprime_matrix[i]);
    }
    free(model->coprime_matrix);
    model->coprime_matrix = new_coprime;
    
    // Reallocate oscillation tracking
    DimensionalOscillation* new_spatial = calloc(new_dimensions, sizeof(DimensionalOscillation));
    memcpy(new_spatial, model->spatial_oscillations,
           model->num_dimensions * sizeof(DimensionalOscillation));
    
    // Initialize new dimensions
    for (uint32_t d = model->num_dimensions; d < new_dimensions; d++) {
        new_spatial[d].dimension = d;
        new_spatial[d].num_samples = 100;
        new_spatial[d].history = calloc(100, sizeof(double));
    }
    
    free(model->spatial_oscillations);
    model->spatial_oscillations = new_spatial;
    
    // Reallocate temporal oscillations
    TemporalOscillation* new_temporal = calloc(new_dimensions, sizeof(TemporalOscillation));
    memcpy(new_temporal, model->temporal_oscillations,
           model->num_dimensions * sizeof(TemporalOscillation));
    
    free(model->temporal_oscillations);
    model->temporal_oscillations = new_temporal;
    
    // Update dimension count
    uint32_t old_dimensions = model->num_dimensions;
    model->num_dimensions = new_dimensions;
    
    // Record scaling
    if (model->num_dimension_scales < 10) {
        model->dimension_scale_history[model->num_dimension_scales] = new_dimensions;
        model->num_dimension_scales++;
    }
    
    printf("✓ Dimensions scaled: %u → %u\n", old_dimensions, new_dimensions);
    printf("  New oscillation tracking: %u dimensions\n", new_dimensions);
    
    return true;
}

bool platonic_scale_vertices(
    PlatonicModel* model,
    uint32_t scale_factor
) {
    if (!model || scale_factor < 2) return false;
    
    uint32_t new_vertices = model->num_vertices * scale_factor;
    
    // Check if we can scale
    if (new_vertices > model->max_vertices) {
        printf("⚠ Cannot scale vertices: would exceed max (%u)\n", model->max_vertices);
        return false;
    }
    
    printf("🔼 Scaling vertices: %u → %u (×%u)\n", 
           model->num_vertices, new_vertices, scale_factor);
    
    // Allocate new vertex positions
    double* new_positions = calloc(
        new_vertices * model->num_dimensions,
        sizeof(double)
    );
    if (!new_positions) return false;
    
    // Copy existing vertices
    memcpy(new_positions, model->vertex_positions,
           model->num_vertices * model->num_dimensions * sizeof(double));
    
    // Generate new vertices using self-similar replication
    for (uint32_t v = model->num_vertices; v < new_vertices; v++) {
        uint32_t source = v % model->num_vertices;
        
        for (uint32_t d = 0; d < model->num_dimensions; d++) {
            double base_value = model->vertex_positions[source * model->num_dimensions + d];
            
            // Add perturbation based on vertex index and dimension
            double perturbation = 0.01 * math_sin(2.0 * M_PI * v / new_vertices + d);
            
            new_positions[v * model->num_dimensions + d] = base_value + perturbation;
        }
    }
    
    // Replace old positions
    free(model->vertex_positions);
    model->vertex_positions = new_positions;
    
    // Reallocate corruption tracking
    bool* new_corruption = calloc(new_vertices, sizeof(bool));
    memcpy(new_corruption, model->corruption_mask, model->num_vertices * sizeof(bool));
    free(model->corruption_mask);
    model->corruption_mask = new_corruption;
    
    // Reallocate confidence scores
    double* new_confidence = calloc(new_vertices, sizeof(double));
    memcpy(new_confidence, model->confidence_scores, model->num_vertices * sizeof(double));
    
    // Initialize new vertices with confidence 1.0
    for (uint32_t v = model->num_vertices; v < new_vertices; v++) {
        new_confidence[v] = 1.0;
    }
    
    free(model->confidence_scores);
    model->confidence_scores = new_confidence;
    
    // Update vertex count
    uint32_t old_vertices = model->num_vertices;
    model->num_vertices = new_vertices;
    
    // Record scaling
    if (model->num_vertex_scales < 10) {
        model->vertex_scale_history[model->num_vertex_scales] = new_vertices;
        model->num_vertex_scales++;
    }
    
    printf("✓ Vertices scaled: %u → %u\n", old_vertices, new_vertices);
    
    return true;
}

bool platonic_scale_tetration_depth(
    PlatonicModel* model,
    uint32_t additional_depth
) {
    if (!model || additional_depth == 0) return false;
    
    uint32_t current_max_depth = 29 + model->num_tetration_depths - 1;
    uint32_t new_max_depth = current_max_depth + additional_depth;
    
    // Cap at reasonable maximum (100)
    if (new_max_depth > 100) {
        printf("⚠ Cannot scale tetration depth: would exceed 100\n");
        return false;
    }
    
    printf("🔼 Scaling tetration depth: %u → %u (+%u)\n", 
           current_max_depth, new_max_depth, additional_depth);
    
    uint32_t new_num_depths = new_max_depth - 29 + 1;
    uint32_t new_total_towers = model->num_tetration_bases * new_num_depths;
    
    // Allocate new tower array
    TetrationTower** new_towers = calloc(new_total_towers, sizeof(TetrationTower*));
    if (!new_towers) return false;
    
    // Copy existing towers
    uint32_t old_total = model->num_tetration_bases * model->num_tetration_depths;
    memcpy(new_towers, model->tetration_towers, old_total * sizeof(TetrationTower*));
    
    // Compute new towers for additional depths
    uint32_t bases[] = {2, 3, 5, 7, 11, 13};
    uint32_t tower_idx = old_total;
    
    for (uint32_t b = 0; b < model->num_tetration_bases; b++) {
        for (uint32_t depth = current_max_depth + 1; depth <= new_max_depth; depth++) {
            new_towers[tower_idx] = tetration_compute_real(bases[b], depth);
            if (new_towers[tower_idx]) {
                tower_idx++;
            }
        }
    }
    
    // Replace old towers
    free(model->tetration_towers);
    model->tetration_towers = new_towers;
    model->num_tetration_depths = new_num_depths;
    
    printf("✓ Tetration depth scaled: %u → %u\n", current_max_depth, new_max_depth);
    printf("  Total towers: %u\n", new_total_towers);
    
    return true;
}

/**
 * Decide if we need to scale and what to scale
 */
bool platonic_auto_scale(PlatonicModel* model, double max_oscillation_amplitude) {
    if (!model) return false;
    
    // If oscillations are very large, we need more complexity
    if (max_oscillation_amplitude > 0.5) {
        printf("\n🔍 Large oscillations detected (%.4f), scaling complexity...\n", 
               max_oscillation_amplitude);
        
        // Scale dimensions first (most impactful)
        if (model->num_dimensions < model->max_dimensions) {
            uint32_t add_dims = (uint32_t)(model->num_dimensions * 0.5);  // Add 50%
            if (add_dims < 5) add_dims = 5;
            if (platonic_scale_dimensions(model, add_dims)) {
                return true;
            }
        }
        
        // Then scale vertices
        if (model->num_vertices < model->max_vertices) {
            if (platonic_scale_vertices(model, 2)) {  // Double vertices
                return true;
            }
        }
        
        // Finally scale tetration depth
        if (platonic_scale_tetration_depth(model, 10)) {  // Add 10 depths
            return true;
        }
    }
    
    // If oscillations are moderate, scale tetration depth
    if (max_oscillation_amplitude > 0.1) {
        printf("\n🔍 Moderate oscillations detected (%.4f), increasing tetration depth...\n",
               max_oscillation_amplitude);
        
        if (platonic_scale_tetration_depth(model, 5)) {
            return true;
        }
    }
    
    return false;
}