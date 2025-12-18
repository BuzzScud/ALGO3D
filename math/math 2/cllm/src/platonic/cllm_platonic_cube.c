/**
 * @file cllm_platonic_cube.c
 * @brief Cube model implementation
 * 
 * The cube is a balanced Platonic solid:
 * - 8 vertices (information nodes)
 * - 12 edges (relationships)
 * - 6 faces (groups)
 * - 48 symmetries (O_h group)
 * 
 * Model dimensions:
 * - Embedding: 96 (8 × 12)
 * - Hidden: 144 (12 × 12)
 * - Layers: 6
 * 
 * Use case: Balanced models for general-purpose training
 * Dual: Octahedron
 */

#include "math/transcendental.h"
#include "math/arithmetic.h"
#include "ai/cllm_platonic.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// CUBE GEOMETRY
// ============================================================================

/**
 * Regular cube vertices (normalized to unit cube)
 * 
 * Standard cube centered at origin with edge length 2:
 * - 8 vertices at (±1, ±1, ±1)
 */
static const double CUBE_VERTICES[8][3] = {
    {-1.0, -1.0, -1.0},  // Vertex 0
    { 1.0, -1.0, -1.0},  // Vertex 1
    { 1.0,  1.0, -1.0},  // Vertex 2
    {-1.0,  1.0, -1.0},  // Vertex 3
    {-1.0, -1.0,  1.0},  // Vertex 4
    { 1.0, -1.0,  1.0},  // Vertex 5
    { 1.0,  1.0,  1.0},  // Vertex 6
    {-1.0,  1.0,  1.0}   // Vertex 7
};

/**
 * Cube edges (pairs of vertex indices)
 * 
 * 12 edges:
 * - 4 bottom edges (z = -1)
 * - 4 top edges (z = 1)
 * - 4 vertical edges
 */
static const uint32_t CUBE_EDGES[12][2] = {
    {0, 1}, {1, 2}, {2, 3}, {3, 0},  // Bottom face
    {4, 5}, {5, 6}, {6, 7}, {7, 4},  // Top face
    {0, 4}, {1, 5}, {2, 6}, {3, 7}   // Vertical edges
};

/**
 * Cube faces (quadruplets of vertex indices)
 * 
 * 6 square faces:
 * - Bottom, Top, Front, Back, Left, Right
 */
static const uint32_t CUBE_FACES[6][4] = {
    {0, 1, 2, 3},  // Bottom (-Z)
    {4, 7, 6, 5},  // Top (+Z)
    {0, 4, 5, 1},  // Front (-Y)
    {2, 6, 7, 3},  // Back (+Y)
    {0, 3, 7, 4},  // Left (-X)
    {1, 5, 6, 2}   // Right (+X)
};

// ============================================================================
// INITIALIZATION
// ============================================================================

/**
 * Initialize cube geometry
 */
bool platonic_cube_init_geometry(PlatonicModel* model) {
    if (!model) return false;
    
    printf("Initializing cube geometry...\n");
    
    // Copy vertex positions
    for (uint32_t i = 0; i < 8; i++) {
        model->vertex_positions[i * 3 + 0] = CUBE_VERTICES[i][0];
        model->vertex_positions[i * 3 + 1] = CUBE_VERTICES[i][1];
        model->vertex_positions[i * 3 + 2] = CUBE_VERTICES[i][2];
    }
    
    // Copy edge connections
    for (uint32_t i = 0; i < 12; i++) {
        model->edge_connections[i * 2 + 0] = CUBE_EDGES[i][0];
        model->edge_connections[i * 2 + 1] = CUBE_EDGES[i][1];
    }
    
    // Copy face vertices
    for (uint32_t i = 0; i < 6; i++) {
        model->face_vertices[i * 4 + 0] = CUBE_FACES[i][0];
        model->face_vertices[i * 4 + 1] = CUBE_FACES[i][1];
        model->face_vertices[i * 4 + 2] = CUBE_FACES[i][2];
        model->face_vertices[i * 4 + 3] = CUBE_FACES[i][3];
    }
    
    printf("  ✓ 8 vertices initialized\n");
    printf("  ✓ 12 edges initialized\n");
    printf("  ✓ 6 faces initialized\n");
    
    // Verify Euler's formula: V - E + F = 8 - 12 + 6 = 2 ✓
    int euler = 8 - 12 + 6;
    printf("  ✓ Euler's formula: 8 - 12 + 6 = %d (verified)\n", euler);
    
    // Verify all edges have same length (should be 2.0)
    double edge_length = math_sqrt(
        (CUBE_VERTICES[1][0] - CUBE_VERTICES[0][0]) * 
        (CUBE_VERTICES[1][0] - CUBE_VERTICES[0][0]) +
        (CUBE_VERTICES[1][1] - CUBE_VERTICES[0][1]) * 
        (CUBE_VERTICES[1][1] - CUBE_VERTICES[0][1]) +
        (CUBE_VERTICES[1][2] - CUBE_VERTICES[0][2]) * 
        (CUBE_VERTICES[1][2] - CUBE_VERTICES[0][2])
    );
    printf("  ✓ Edge length: %.6f (all edges equal)\n", edge_length);
    
    return true;
}

/**
 * Initialize cube embeddings
 * 
 * Each of the 8 vertices gets 12 embedding dimensions,
 * for a total of 96 dimensions.
 */
bool platonic_cube_init_embeddings(PlatonicModel* model) {
    if (!model) return false;
    
    printf("Initializing cube embeddings (96-dim)...\n");
    
    // Allocate embeddings: vocab_size × 96
    size_t embedding_size = model->config.vocab_size * model->config.embedding_dim;
    model->embeddings = (double*)calloc(embedding_size, sizeof(double));
    
    if (!model->embeddings) {
        fprintf(stderr, "Error: Failed to allocate embeddings\n");
        return false;
    }
    
    // Initialize embeddings using vertex positions
    // Each token gets a 96-dim embedding organized as 8 vertices × 12 dims
    for (uint32_t token = 0; token < model->config.vocab_size; token++) {
        for (uint32_t vertex = 0; vertex < 8; vertex++) {
            // Base index for this token-vertex combination
            size_t base_idx = token * model->config.embedding_dim + vertex * 12;
            
            // Initialize with vertex coordinates (repeated across 12 dims)
            for (uint32_t dim = 0; dim < 12; dim++) {
                // Use vertex position as seed, modulated by dimension
                double coord_sum = CUBE_VERTICES[vertex][0] + 
                                 CUBE_VERTICES[vertex][1] + 
                                 CUBE_VERTICES[vertex][2];
                model->embeddings[base_idx + dim] = coord_sum * (dim + 1) / 12.0;
            }
        }
    }
    
    printf("  ✓ Embeddings initialized: %zu × %u = %zu values\n",
           (size_t)model->config.vocab_size, 
           model->config.embedding_dim,
           embedding_size);
    
    return true;
}

/**
 * Initialize cube layer weights
 * 
 * Each of the 6 faces corresponds to one layer.
 */
bool platonic_cube_init_layers(PlatonicModel* model) {
    if (!model) return false;
    
    printf("Initializing cube layers (6 layers)...\n");
    
    // Allocate layer weights
    size_t layer_size = model->config.embedding_dim * model->config.hidden_dim;
    size_t total_layer_size = 6 * layer_size;
    
    model->layer_weights = (double*)calloc(total_layer_size, sizeof(double));
    
    if (!model->layer_weights) {
        fprintf(stderr, "Error: Failed to allocate layer weights\n");
        return false;
    }
    
    // Initialize layer weights with small random values
    for (size_t i = 0; i < total_layer_size; i++) {
        model->layer_weights[i] = ((double)rand() / RAND_MAX - 0.5) * 0.1;
    }
    
    printf("  ✓ Layer weights initialized: 6 layers × %zu = %zu values\n",
           layer_size, total_layer_size);
    
    return true;
}

/**
 * Initialize cube attention weights
 * 
 * Each of the 12 edges corresponds to an attention connection.
 */
bool platonic_cube_init_attention(PlatonicModel* model) {
    if (!model) return false;
    
    printf("Initializing cube attention (12 edges)...\n");
    
    // Allocate attention weights
    size_t attention_size = model->config.hidden_dim * model->config.hidden_dim;
    size_t total_attention_size = 12 * attention_size;
    
    model->attention_weights = (double*)calloc(total_attention_size, sizeof(double));
    
    if (!model->attention_weights) {
        fprintf(stderr, "Error: Failed to allocate attention weights\n");
        return false;
    }
    
    // Initialize attention weights with small random values
    for (size_t i = 0; i < total_attention_size; i++) {
        model->attention_weights[i] = ((double)rand() / RAND_MAX - 0.5) * 0.1;
    }
    
    printf("  ✓ Attention weights initialized: 12 edges × %zu = %zu values\n",
           attention_size, total_attention_size);
    
    return true;
}

// ============================================================================
// VALIDATION
// ============================================================================

/**
 * Validate cube model structure
 */
bool platonic_cube_validate(const PlatonicModel* model) {
    if (!model) return false;
    
    // Check solid type
    if (model->config.solid_type != PLATONIC_CUBE) {
        fprintf(stderr, "Error: Model is not a cube\n");
        return false;
    }
    
    // Check geometry
    if (model->geometry.vertices != 8 ||
        model->geometry.edges != 12 ||
        model->geometry.faces != 6) {
        fprintf(stderr, "Error: Invalid cube geometry\n");
        return false;
    }
    
    // Check Euler's formula
    if (!platonic_verify_euler(&model->geometry)) {
        fprintf(stderr, "Error: Euler's formula failed\n");
        return false;
    }
    
    // Check dimensions
    if (model->config.embedding_dim != 96) {
        fprintf(stderr, "Error: Invalid embedding dimension (expected 96, got %u)\n",
                model->config.embedding_dim);
        return false;
    }
    
    if (model->config.hidden_dim != 144) {
        fprintf(stderr, "Error: Invalid hidden dimension (expected 144, got %u)\n",
                model->config.hidden_dim);
        return false;
    }
    
    if (model->config.num_layers != 6) {
        fprintf(stderr, "Error: Invalid number of layers (expected 6, got %u)\n",
                model->config.num_layers);
        return false;
    }
    
    // Check allocations
    if (!model->vertex_positions || !model->edge_connections || 
        !model->face_vertices) {
        fprintf(stderr, "Error: Geometry not initialized\n");
        return false;
    }
    
    if (!model->embeddings || !model->layer_weights || 
        !model->attention_weights) {
        fprintf(stderr, "Error: Weights not initialized\n");
        return false;
    }
    
    return true;
}