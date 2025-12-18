/**
 * @file platonic_model_persistence.c
 * @brief Save/Load Platonic Models to Disk
 * 
 * This creates PERMANENT model files that can be:
 * - Saved after stabilization
 * - Loaded for continued recovery
 * - Analyzed for convergence patterns
 * - Shared and archived
 * 
 * File format: .platonic (binary format with header)
 */

#include "platonic_model.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// File format version
#define PLATONIC_FILE_VERSION 1

// File header
typedef struct {
    char magic[8];                // "PLATONIC"
    uint32_t version;             // File format version
    uint32_t base_type;           // PlatonicSolidType
    uint32_t num_dimensions;
    uint32_t num_vertices;
    uint32_t num_edges;
    uint32_t num_faces;
    uint32_t num_tetration_bases;
    uint32_t num_tetration_depths;
    bool is_converged;
    uint64_t total_iterations;
    double convergence_rate;
    double final_oscillation_amplitude;
    uint32_t num_dimension_scales;
    uint32_t num_vertex_scales;
    char model_id[64];
    uint64_t data_offset;         // Offset to vertex data
} PlatonicFileHeader;

bool platonic_model_save(
    const PlatonicModel* model,
    const char* file_path
) {
    if (!model || !file_path) return false;
    
    printf("\n💾 Saving model to: %s\n", file_path);
    
    FILE* file = fopen(file_path, "wb");
    if (!file) {
        printf("❌ Failed to open file for writing\n");
        return false;
    }
    
    // Write header
    PlatonicFileHeader header = {0};
    memcpy(header.magic, "PLATONIC", 8);
    header.version = PLATONIC_FILE_VERSION;
    header.base_type = model->base_type;
    header.num_dimensions = model->num_dimensions;
    header.num_vertices = model->num_vertices;
    header.num_edges = model->num_edges;
    header.num_faces = model->num_faces;
    header.num_tetration_bases = model->num_tetration_bases;
    header.num_tetration_depths = model->num_tetration_depths;
    header.is_converged = model->is_converged;
    header.total_iterations = model->total_iterations;
    header.convergence_rate = model->convergence_rate;
    header.final_oscillation_amplitude = model->final_oscillation_amplitude;
    header.num_dimension_scales = model->num_dimension_scales;
    header.num_vertex_scales = model->num_vertex_scales;
    strncpy(header.model_id, model->model_id, sizeof(header.model_id) - 1);
    header.data_offset = sizeof(PlatonicFileHeader);
    
    if (fwrite(&header, sizeof(header), 1, file) != 1) {
        printf("❌ Failed to write header\n");
        fclose(file);
        return false;
    }
    
    // Write vertex positions
    size_t vertex_data_size = model->num_vertices * model->num_dimensions * sizeof(double);
    if (fwrite(model->vertex_positions, 1, vertex_data_size, file) != vertex_data_size) {
        printf("❌ Failed to write vertex positions\n");
        fclose(file);
        return false;
    }
    
    // Write corruption mask
    if (fwrite(model->corruption_mask, sizeof(bool), model->num_vertices, file) != model->num_vertices) {
        printf("❌ Failed to write corruption mask\n");
        fclose(file);
        return false;
    }
    
    // Write confidence scores
    if (fwrite(model->confidence_scores, sizeof(double), model->num_vertices, file) != model->num_vertices) {
        printf("❌ Failed to write confidence scores\n");
        fclose(file);
        return false;
    }
    
    // Write scaling history
    if (model->num_dimension_scales > 0) {
        if (fwrite(model->dimension_scale_history, sizeof(uint32_t), 
                   model->num_dimension_scales, file) != model->num_dimension_scales) {
            printf("❌ Failed to write dimension scale history\n");
            fclose(file);
            return false;
        }
    }
    
    if (model->num_vertex_scales > 0) {
        if (fwrite(model->vertex_scale_history, sizeof(uint32_t), 
                   model->num_vertex_scales, file) != model->num_vertex_scales) {
            printf("❌ Failed to write vertex scale history\n");
            fclose(file);
            return false;
        }
    }
    
    fclose(file);
    
    printf("✓ Model saved successfully\n");
    printf("  File size: %zu bytes\n", 
           sizeof(header) + vertex_data_size + 
           model->num_vertices * (sizeof(bool) + sizeof(double)) +
           (model->num_dimension_scales + model->num_vertex_scales) * sizeof(uint32_t));
    
    return true;
}

PlatonicModel* platonic_model_load(const char* file_path) {
    if (!file_path) return NULL;
    
    printf("\n📂 Loading model from: %s\n", file_path);
    
    FILE* file = fopen(file_path, "rb");
    if (!file) {
        printf("❌ Failed to open file for reading\n");
        return NULL;
    }
    
    // Read header
    PlatonicFileHeader header;
    if (fread(&header, sizeof(header), 1, file) != 1) {
        printf("❌ Failed to read header\n");
        fclose(file);
        return NULL;
    }
    
    // Validate magic
    if (memcmp(header.magic, "PLATONIC", 8) != 0) {
        printf("❌ Invalid file format (bad magic)\n");
        fclose(file);
        return NULL;
    }
    
    // Validate version
    if (header.version != PLATONIC_FILE_VERSION) {
        printf("❌ Unsupported file version: %u\n", header.version);
        fclose(file);
        return NULL;
    }
    
    printf("✓ Valid Platonic model file\n");
    printf("  Model ID: %s\n", header.model_id);
    printf("  Dimensions: %u\n", header.num_dimensions);
    printf("  Vertices: %u\n", header.num_vertices);
    printf("  Converged: %s\n", header.is_converged ? "Yes" : "No");
    printf("  Iterations: %lu\n", header.total_iterations);
    
    // Create model structure
    PlatonicModel* model = calloc(1, sizeof(PlatonicModel));
    if (!model) {
        fclose(file);
        return NULL;
    }
    
    // Copy header data
    strncpy(model->model_id, header.model_id, sizeof(model->model_id) - 1);
    model->base_type = (PlatonicSolidType)header.base_type;
    model->num_dimensions = header.num_dimensions;
    model->num_vertices = header.num_vertices;
    model->num_edges = header.num_edges;
    model->num_faces = header.num_faces;
    model->num_tetration_bases = header.num_tetration_bases;
    model->num_tetration_depths = header.num_tetration_depths;
    model->is_converged = header.is_converged;
    model->total_iterations = header.total_iterations;
    model->convergence_rate = header.convergence_rate;
    model->final_oscillation_amplitude = header.final_oscillation_amplitude;
    model->num_dimension_scales = header.num_dimension_scales;
    model->num_vertex_scales = header.num_vertex_scales;
    
    // Set max values (allow further scaling)
    model->max_dimensions = model->num_dimensions * 4;
    model->max_vertices = model->num_vertices * 8;
    model->min_dimensions = 13;
    model->min_vertices = 2048;
    
    // Allocate and read vertex positions
    size_t vertex_data_size = model->num_vertices * model->num_dimensions * sizeof(double);
    model->vertex_positions = malloc(vertex_data_size);
    if (!model->vertex_positions || 
        fread(model->vertex_positions, 1, vertex_data_size, file) != vertex_data_size) {
        printf("❌ Failed to read vertex positions\n");
        platonic_model_free(model);
        fclose(file);
        return NULL;
    }
    
    // Allocate and read corruption mask
    model->corruption_mask = malloc(model->num_vertices * sizeof(bool));
    if (!model->corruption_mask ||
        fread(model->corruption_mask, sizeof(bool), model->num_vertices, file) != model->num_vertices) {
        printf("❌ Failed to read corruption mask\n");
        platonic_model_free(model);
        fclose(file);
        return NULL;
    }
    
    // Allocate and read confidence scores
    model->confidence_scores = malloc(model->num_vertices * sizeof(double));
    if (!model->confidence_scores ||
        fread(model->confidence_scores, sizeof(double), model->num_vertices, file) != model->num_vertices) {
        printf("❌ Failed to read confidence scores\n");
        platonic_model_free(model);
        fclose(file);
        return NULL;
    }
    
    // Allocate and read scaling history
    model->dimension_scale_history = calloc(10, sizeof(uint32_t));
    model->vertex_scale_history = calloc(10, sizeof(uint32_t));
    
    if (model->num_dimension_scales > 0) {
        if (fread(model->dimension_scale_history, sizeof(uint32_t), 
                  model->num_dimension_scales, file) != model->num_dimension_scales) {
            printf("⚠ Failed to read dimension scale history (non-fatal)\n");
        }
    }
    
    if (model->num_vertex_scales > 0) {
        if (fread(model->vertex_scale_history, sizeof(uint32_t), 
                  model->num_vertex_scales, file) != model->num_vertex_scales) {
            printf("⚠ Failed to read vertex scale history (non-fatal)\n");
        }
    }
    
    fclose(file);
    
    // Allocate oscillation tracking (not saved to file, will be recomputed)
    model->spatial_oscillations = calloc(model->num_dimensions, sizeof(DimensionalOscillation));
    model->temporal_oscillations = calloc(model->num_dimensions, sizeof(TemporalOscillation));
    
    for (uint32_t d = 0; d < model->num_dimensions; d++) {
        model->spatial_oscillations[d].dimension = d;
        model->spatial_oscillations[d].num_samples = 100;
        model->spatial_oscillations[d].history = calloc(100, sizeof(double));
    }
    
    // Allocate coprime matrix (will be recomputed)
    model->coprime_matrix = calloc(model->num_dimensions, sizeof(uint64_t*));
    for (uint32_t i = 0; i < model->num_dimensions; i++) {
        model->coprime_matrix[i] = calloc(model->num_dimensions, sizeof(uint64_t));
    }
    
    // Recreate tetration towers
    uint32_t bases[] = {2, 3, 5, 7, 11, 13};
    uint32_t total_towers = model->num_tetration_bases * model->num_tetration_depths;
    model->tetration_towers = calloc(total_towers, sizeof(TetrationTower*));
    
    uint32_t tower_idx = 0;
    for (uint32_t b = 0; b < model->num_tetration_bases; b++) {
        for (uint32_t depth = 29; depth < 29 + model->num_tetration_depths; depth++) {
            model->tetration_towers[tower_idx] = tetration_compute_real(bases[b], depth);
            tower_idx++;
        }
    }
    
    printf("✓ Model loaded successfully\n");
    
    return model;
}
