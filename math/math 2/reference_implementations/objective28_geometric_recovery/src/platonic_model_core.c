/**
 * @file platonic_model_core.c
 * @brief Core Platonic Model Implementation with Dynamic Scaling
 * 
 * This creates a LITERAL geometric model that can:
 * - Scale dimensions dynamically (13 → 20 → 30 → ...)
 * - Scale vertices dynamically (2^11 → 2^12 → 2^13 → ...)
 * - Scale tetration depth dynamically (29 → 40 → 50 → 59)
 * - Track spatial AND temporal oscillations
 * - Save/load from disk as a persistent structure
 */

#include "platonic_model.h"
#include <stdlib.h>
#include <string.h>
#include "../../../include/prime_float_math.h"
#include <stdio.h>
#include <time.h>

// Golden ratio (for Icosahedron/Dodecahedron)
#define PHI 1.618033988749895

/**
 * Generate base Platonic solid vertices in 3D
 */
static void generate_base_vertices_3d(
    PlatonicSolidType type,
    double* vertices,
    uint32_t* out_num_vertices
) {
    switch (type) {
        case PLATONIC_TETRAHEDRON:
            *out_num_vertices = 4;
            // Tetrahedron vertices
            vertices[0] = 1.0; vertices[1] = 1.0; vertices[2] = 1.0;
            vertices[3] = 1.0; vertices[4] = -1.0; vertices[5] = -1.0;
            vertices[6] = -1.0; vertices[7] = 1.0; vertices[8] = -1.0;
            vertices[9] = -1.0; vertices[10] = -1.0; vertices[11] = 1.0;
            break;
            
        case PLATONIC_CUBE:
            *out_num_vertices = 8;
            // Cube vertices
            for (int i = 0; i < 8; i++) {
                vertices[i*3 + 0] = (i & 1) ? 1.0 : -1.0;
                vertices[i*3 + 1] = (i & 2) ? 1.0 : -1.0;
                vertices[i*3 + 2] = (i & 4) ? 1.0 : -1.0;
            }
            break;
            
        case PLATONIC_OCTAHEDRON:
            *out_num_vertices = 6;
            // Octahedron vertices (axis-aligned)
            vertices[0] = 1.0; vertices[1] = 0.0; vertices[2] = 0.0;
            vertices[3] = -1.0; vertices[4] = 0.0; vertices[5] = 0.0;
            vertices[6] = 0.0; vertices[7] = 1.0; vertices[8] = 0.0;
            vertices[9] = 0.0; vertices[10] = -1.0; vertices[11] = 0.0;
            vertices[12] = 0.0; vertices[13] = 0.0; vertices[14] = 1.0;
            vertices[15] = 0.0; vertices[16] = 0.0; vertices[17] = -1.0;
            break;
            
        case PLATONIC_DODECAHEDRON:
            *out_num_vertices = 20;
            // Dodecahedron vertices (using golden ratio)
            {
                double a = 1.0;
                double b = 1.0 / PHI;
                double c = PHI;
                
                int idx = 0;
                // 8 vertices of a cube
                for (int i = 0; i < 8; i++) {
                    vertices[idx++] = (i & 1) ? a : -a;
                    vertices[idx++] = (i & 2) ? a : -a;
                    vertices[idx++] = (i & 4) ? a : -a;
                }
                // 12 vertices on rectangular faces
                double coords[][3] = {
                    {0, b, c}, {0, -b, c}, {0, b, -c}, {0, -b, -c},
                    {c, 0, b}, {c, 0, -b}, {-c, 0, b}, {-c, 0, -b},
                    {b, c, 0}, {-b, c, 0}, {b, -c, 0}, {-b, -c, 0}
                };
                for (int i = 0; i < 12; i++) {
                    vertices[idx++] = coords[i][0];
                    vertices[idx++] = coords[i][1];
                    vertices[idx++] = coords[i][2];
                }
            }
            break;
            
        case PLATONIC_ICOSAHEDRON:
            *out_num_vertices = 12;
            // Icosahedron vertices (using golden ratio)
            {
                double a = 1.0;
                double b = PHI;
                
                int idx = 0;
                double coords[][3] = {
                    {0, a, b}, {0, -a, b}, {0, a, -b}, {0, -a, -b},
                    {b, 0, a}, {b, 0, -a}, {-b, 0, a}, {-b, 0, -a},
                    {a, b, 0}, {-a, b, 0}, {a, -b, 0}, {-a, -b, 0}
                };
                for (int i = 0; i < 12; i++) {
                    vertices[idx++] = coords[i][0];
                    vertices[idx++] = coords[i][1];
                    vertices[idx++] = coords[i][2];
                }
            }
            break;
    }
}

/**
 * Expand 3D vertices to high dimensions using prime-based projection
 */
static void expand_to_high_dimensions(
    const double* vertices_3d,
    uint32_t num_vertices_3d,
    double* vertices_hd,
    uint32_t num_dimensions
) {
    // Use prime numbers to project into higher dimensions
    uint32_t primes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71};
    
    for (uint32_t v = 0; v < num_vertices_3d; v++) {
        // Copy original 3D coordinates
        vertices_hd[v * num_dimensions + 0] = vertices_3d[v * 3 + 0];
        vertices_hd[v * num_dimensions + 1] = vertices_3d[v * 3 + 1];
        vertices_hd[v * num_dimensions + 2] = vertices_3d[v * 3 + 2];
        
        // Project into higher dimensions using prime-based formula
        for (uint32_t d = 3; d < num_dimensions; d++) {
            double sum = 0.0;
            for (uint32_t i = 0; i < 3; i++) {
                double coord = vertices_3d[v * 3 + i];
                uint32_t prime = primes[d % 20];
                sum += coord * math_cos(2.0 * M_PI * prime * (d - 3) / num_dimensions);
            }
            vertices_hd[v * num_dimensions + d] = sum / math_sqrt((double)num_dimensions);
        }
    }
}

/**
 * Replicate vertices to reach target count
 */
static void replicate_vertices(
    double* vertices,
    uint32_t base_num_vertices,
    uint32_t target_num_vertices,
    uint32_t num_dimensions
) {
    // Use self-similar replication with small perturbations
    for (uint32_t v = base_num_vertices; v < target_num_vertices; v++) {
        uint32_t source = v % base_num_vertices;
        
        for (uint32_t d = 0; d < num_dimensions; d++) {
            double base_value = vertices[source * num_dimensions + d];
            // Add small perturbation based on vertex index
            double perturbation = 0.01 * math_sin(2.0 * M_PI * v / target_num_vertices + d);
            vertices[v * num_dimensions + d] = base_value + perturbation;
        }
    }
}

PlatonicModel* platonic_model_create(
    PlatonicSolidType base_type,
    uint32_t initial_dimensions,
    uint32_t initial_vertices
) {
    // Enforce minimums
    if (initial_dimensions < 13) initial_dimensions = 13;
    if (initial_vertices < 2048) initial_vertices = 2048;  // 2^11
    
    PlatonicModel* model = calloc(1, sizeof(PlatonicModel));
    if (!model) return NULL;
    
    // Generate unique ID
    snprintf(model->model_id, sizeof(model->model_id), 
             "PLATONIC_%d_D%u_V%u_%ld",
             base_type, initial_dimensions, initial_vertices, time(NULL));
    
    model->base_type = base_type;
    model->num_dimensions = initial_dimensions;
    model->max_dimensions = initial_dimensions * 4;  // Can scale 4x
    model->min_dimensions = 13;
    
    model->num_vertices = initial_vertices;
    model->max_vertices = initial_vertices * 8;  // Can scale 8x
    model->min_vertices = 2048;
    
    // Allocate vertex positions
    model->vertex_positions = calloc(
        model->num_vertices * model->num_dimensions,
        sizeof(double)
    );
    if (!model->vertex_positions) {
        platonic_model_free(model);
        return NULL;
    }
    
    // Generate base 3D vertices
    double* vertices_3d = malloc(20 * 3 * sizeof(double));  // Max 20 vertices for dodecahedron
    uint32_t base_num_vertices;
    generate_base_vertices_3d(base_type, vertices_3d, &base_num_vertices);
    
    // Expand to high dimensions
    double* vertices_hd_base = malloc(base_num_vertices * initial_dimensions * sizeof(double));
    expand_to_high_dimensions(vertices_3d, base_num_vertices, vertices_hd_base, initial_dimensions);
    
    // Copy base vertices
    memcpy(model->vertex_positions, vertices_hd_base, 
           base_num_vertices * initial_dimensions * sizeof(double));
    
    // Replicate to reach target vertex count
    if (initial_vertices > base_num_vertices) {
        replicate_vertices(model->vertex_positions, base_num_vertices, 
                          initial_vertices, initial_dimensions);
    }
    
    free(vertices_3d);
    free(vertices_hd_base);
    
    // Allocate coprime matrix
    model->coprime_matrix = calloc(model->num_dimensions, sizeof(uint64_t*));
    for (uint32_t i = 0; i < model->num_dimensions; i++) {
        model->coprime_matrix[i] = calloc(model->num_dimensions, sizeof(uint64_t));
    }
    
    // Allocate corruption tracking
    model->corruption_mask = calloc(model->num_vertices, sizeof(bool));
    model->confidence_scores = calloc(model->num_vertices, sizeof(double));
    
    // Initialize confidence scores to 1.0 (all known good initially)
    for (uint32_t i = 0; i < model->num_vertices; i++) {
        model->confidence_scores[i] = 1.0;
    }
    
    // Allocate oscillation tracking
    model->spatial_oscillations = calloc(model->num_dimensions, sizeof(DimensionalOscillation));
    model->temporal_oscillations = calloc(model->num_dimensions, sizeof(TemporalOscillation));
    
    // Initialize oscillation history
    for (uint32_t d = 0; d < model->num_dimensions; d++) {
        model->spatial_oscillations[d].dimension = d;
        model->spatial_oscillations[d].num_samples = 100;  // Track last 100 samples
        model->spatial_oscillations[d].history = calloc(100, sizeof(double));
    }
    
    // Create initial tetration towers (bases 2, 3, 5, 7, 11, 13)
    uint32_t bases[] = {2, 3, 5, 7, 11, 13};
    model->num_tetration_bases = 6;
    model->num_tetration_depths = 31;  // Depths 29-59
    
    uint32_t total_towers = model->num_tetration_bases * model->num_tetration_depths;
    model->tetration_towers = calloc(total_towers, sizeof(TetrationTower*));
    
    uint32_t tower_idx = 0;
    for (uint32_t b = 0; b < model->num_tetration_bases; b++) {
        for (uint32_t depth = 29; depth <= 59; depth++) {
            model->tetration_towers[tower_idx] = tetration_compute_real(bases[b], depth);
            tower_idx++;
        }
    }
    
    // Initialize scaling history
    model->num_dimension_scales = 0;
    model->num_vertex_scales = 0;
    model->dimension_scale_history = calloc(10, sizeof(uint32_t));  // Track up to 10 scales
    model->vertex_scale_history = calloc(10, sizeof(uint32_t));
    
    printf("✓ Created Platonic Model: %s\n", model->model_id);
    printf("  Base: %d, Dimensions: %u, Vertices: %u\n", 
           base_type, initial_dimensions, initial_vertices);
    printf("  Tetration towers: %u bases × %u depths = %u towers\n",
           model->num_tetration_bases, model->num_tetration_depths, total_towers);
    
    return model;
}

void platonic_model_free(PlatonicModel* model) {
    if (!model) return;
    
    free(model->vertex_positions);
    free(model->edge_indices);
    free(model->face_indices);
    
    if (model->coprime_matrix) {
        for (uint32_t i = 0; i < model->num_dimensions; i++) {
            free(model->coprime_matrix[i]);
        }
        free(model->coprime_matrix);
    }
    
    free(model->corruption_mask);
    free(model->confidence_scores);
    
    if (model->spatial_oscillations) {
        for (uint32_t d = 0; d < model->num_dimensions; d++) {
            free(model->spatial_oscillations[d].history);
        }
        free(model->spatial_oscillations);
    }
    
    if (model->temporal_oscillations) {
        for (uint32_t d = 0; d < model->num_dimensions; d++) {
            if (model->temporal_oscillations[d].stabilizer) {
                tetration_tower_free(model->temporal_oscillations[d].stabilizer);
            }
        }
        free(model->temporal_oscillations);
    }
    
    if (model->tetration_towers) {
        uint32_t total_towers = model->num_tetration_bases * model->num_tetration_depths;
        for (uint32_t i = 0; i < total_towers; i++) {
            tetration_tower_free(model->tetration_towers[i]);
        }
        free(model->tetration_towers);
    }
    
    free(model->dimension_scale_history);
    free(model->vertex_scale_history);
    
    free(model);
}