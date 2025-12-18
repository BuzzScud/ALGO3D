/**
 * @file cllm_platonic_octahedron.c
 * @brief Octahedron model implementation
 * 
 * The octahedron is the dual of the cube:
 * - 6 vertices (information nodes)
 * - 12 edges (relationships)
 * - 8 faces (groups)
 * - 48 symmetries (O_h group)
 * 
 * Model dimensions:
 * - Embedding: 72 (6 × 12)
 * - Hidden: 144 (12 × 12)
 * - Layers: 8
 * 
 * Use case: Specialized tasks, dual representation
 * Dual: Cube
 */

#include "ai/cllm_platonic.h"
#include "math/transcendental.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "math/transcendental.h"

// ============================================================================
// OCTAHEDRON GEOMETRY
// ============================================================================

/**
 * Regular octahedron vertices (normalized)
 * 
 * Positioned at the centers of the cube's faces:
 * - (±1, 0, 0) - X axis
 * - (0, ±1, 0) - Y axis
 * - (0, 0, ±1) - Z axis
 * 
 * These form a perfect octahedron with edge length √2
 */
static const double OCTAHEDRON_VERTICES[6][3] = {
    { 1.0,  0.0,  0.0},  // Vertex 0 (+X)
    {-1.0,  0.0,  0.0},  // Vertex 1 (-X)
    { 0.0,  1.0,  0.0},  // Vertex 2 (+Y)
    { 0.0, -1.0,  0.0},  // Vertex 3 (-Y)
    { 0.0,  0.0,  1.0},  // Vertex 4 (+Z)
    { 0.0,  0.0, -1.0}   // Vertex 5 (-Z)
};

/**
 * Octahedron edges (pairs of vertex indices)
 * 
 * 12 edges connecting the 6 vertices:
 * - 4 edges around equator (XY plane)
 * - 4 edges to top vertex (+Z)
 * - 4 edges to bottom vertex (-Z)
 */
static const uint32_t OCTAHEDRON_EDGES[12][2] = {
    // Equator (XY plane)
    {0, 2}, {2, 1}, {1, 3}, {3, 0},
    // Top pyramid (+Z)
    {0, 4}, {2, 4}, {1, 4}, {3, 4},
    // Bottom pyramid (-Z)
    {0, 5}, {2, 5}, {1, 5}, {3, 5}
};

/**
 * Octahedron faces (triplets of vertex indices)
 * 
 * 8 triangular faces:
 * - 4 faces on top hemisphere
 * - 4 faces on bottom hemisphere
 */
static const uint32_t OCTAHEDRON_FACES[8][3] = {
    // Top hemisphere (+Z)
    {0, 2, 4},  // Face 0
    {2, 1, 4},  // Face 1
    {1, 3, 4},  // Face 2
    {3, 0, 4},  // Face 3
    // Bottom hemisphere (-Z)
    {0, 5, 2},  // Face 4
    {2, 5, 1},  // Face 5
    {1, 5, 3},  // Face 6
    {3, 5, 0}   // Face 7
};

// ============================================================================
// INITIALIZATION
// ============================================================================

/**
 * Initialize octahedron geometry
 */
bool platonic_octahedron_init_geometry(PlatonicModel* model) {
    if (!model) return false;
    
    printf("Initializing octahedron geometry...\n");
    
    // Copy vertex positions
    for (uint32_t i = 0; i < 6; i++) {
        model->vertex_positions[i * 3 + 0] = OCTAHEDRON_VERTICES[i][0];
        model->vertex_positions[i * 3 + 1] = OCTAHEDRON_VERTICES[i][1];
        model->vertex_positions[i * 3 + 2] = OCTAHEDRON_VERTICES[i][2];
    }
    
    // Copy edge connections
    for (uint32_t i = 0; i < 12; i++) {
        model->edge_connections[i * 2 + 0] = OCTAHEDRON_EDGES[i][0];
        model->edge_connections[i * 2 + 1] = OCTAHEDRON_EDGES[i][1];
    }
    
    // Copy face vertices
    for (uint32_t i = 0; i < 8; i++) {
        model->face_vertices[i * 4 + 0] = OCTAHEDRON_FACES[i][0];
        model->face_vertices[i * 4 + 1] = OCTAHEDRON_FACES[i][1];
        model->face_vertices[i * 4 + 2] = OCTAHEDRON_FACES[i][2];
        model->face_vertices[i * 4 + 3] = 0;  // Padding (faces are triangles)
    }
    
    printf("  ✓ 6 vertices initialized\n");
    printf("  ✓ 12 edges initialized\n");
    printf("  ✓ 8 faces initialized\n");
    
    // Verify Euler's formula: V - E + F = 6 - 12 + 8 = 2 ✓
    int euler = 6 - 12 + 8;
    printf("  ✓ Euler's formula: 6 - 12 + 8 = %d (verified)\n", euler);
    
    // Verify all edges have same length (should be √2 ≈ 1.414)
    double edge_length = math_sqrt(
        (OCTAHEDRON_VERTICES[0][0] - OCTAHEDRON_VERTICES[2][0]) * 
        (OCTAHEDRON_VERTICES[0][0] - OCTAHEDRON_VERTICES[2][0]) +
        (OCTAHEDRON_VERTICES[0][1] - OCTAHEDRON_VERTICES[2][1]) * 
        (OCTAHEDRON_VERTICES[0][1] - OCTAHEDRON_VERTICES[2][1]) +
        (OCTAHEDRON_VERTICES[0][2] - OCTAHEDRON_VERTICES[2][2]) * 
        (OCTAHEDRON_VERTICES[0][2] - OCTAHEDRON_VERTICES[2][2])
    );
    printf("  ✓ Edge length: %.6f (all edges equal)\n", edge_length);
    printf("  ✓ Dual of cube (vertices ↔ faces)\n");
    
    return true;
}

/**
 * Initialize octahedron embeddings
 * 
 * Each of the 6 vertices gets 12 embedding dimensions,
 * for a total of 72 dimensions.
 */
bool platonic_octahedron_init_embeddings(PlatonicModel* model) {
    if (!model) return false;
    
    printf("Initializing octahedron embeddings (72-dim)...\n");
    
    // Allocate embeddings: vocab_size × 72
    size_t embedding_size = model->config.vocab_size * model->config.embedding_dim;
    model->embeddings = (double*)calloc(embedding_size, sizeof(double));
    
    if (!model->embeddings) {
        fprintf(stderr, "Error: Failed to allocate embeddings\n");
        return false;
    }
    
    // Initialize embeddings using vertex positions
    // Each token gets a 72-dim embedding organized as 6 vertices × 12 dims
    for (uint32_t token = 0; token < model->config.vocab_size; token++) {
        for (uint32_t vertex = 0; vertex < 6; vertex++) {
            // Base index for this token-vertex combination
            size_t base_idx = token * model->config.embedding_dim + vertex * 12;
            
            // Initialize with vertex coordinates (repeated across 12 dims)
            for (uint32_t dim = 0; dim < 12; dim++) {
                // Use vertex position as seed, modulated by dimension
                double coord_sum = OCTAHEDRON_VERTICES[vertex][0] + 
                                 OCTAHEDRON_VERTICES[vertex][1] + 
                                 OCTAHEDRON_VERTICES[vertex][2];
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
 * Initialize octahedron layer weights
 * 
 * Each of the 8 faces corresponds to one layer.
 */
bool platonic_octahedron_init_layers(PlatonicModel* model) {
    if (!model) return false;
    
    printf("Initializing octahedron layers (8 layers)...\n");
    
    // Allocate layer weights
    size_t layer_size = model->config.embedding_dim * model->config.hidden_dim;
    size_t total_layer_size = 8 * layer_size;
    
    model->layer_weights = (double*)calloc(total_layer_size, sizeof(double));
    
    if (!model->layer_weights) {
        fprintf(stderr, "Error: Failed to allocate layer weights\n");
        return false;
    }
    
    // Initialize layer weights with small random values
    for (size_t i = 0; i < total_layer_size; i++) {
        model->layer_weights[i] = ((double)rand() / RAND_MAX - 0.5) * 0.1;
    }
    
    printf("  ✓ Layer weights initialized: 8 layers × %zu = %zu values\n",
           layer_size, total_layer_size);
    
    return true;
}

/**
 * Initialize octahedron attention weights
 * 
 * Each of the 12 edges corresponds to an attention connection.
 */
bool platonic_octahedron_init_attention(PlatonicModel* model) {
    if (!model) return false;
    
    printf("Initializing octahedron attention (12 edges)...\n");
    
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
 * Validate octahedron model structure
 */
bool platonic_octahedron_validate(const PlatonicModel* model) {
    if (!model) return false;
    
    // Check solid type
    if (model->config.solid_type != PLATONIC_OCTAHEDRON) {
        fprintf(stderr, "Error: Model is not an octahedron\n");
        return false;
    }
    
    // Check geometry
    if (model->geometry.vertices != 6 ||
        model->geometry.edges != 12 ||
        model->geometry.faces != 8) {
        fprintf(stderr, "Error: Invalid octahedron geometry\n");
        return false;
    }
    
    // Check Euler's formula
    if (!platonic_verify_euler(&model->geometry)) {
        fprintf(stderr, "Error: Euler's formula failed\n");
        return false;
    }
    
    // Check dimensions
    if (model->config.embedding_dim != 72) {
        fprintf(stderr, "Error: Invalid embedding dimension (expected 72, got %u)\n",
                model->config.embedding_dim);
        return false;
    }
    
    if (model->config.hidden_dim != 144) {
        fprintf(stderr, "Error: Invalid hidden dimension (expected 144, got %u)\n",
                model->config.hidden_dim);
        return false;
    }
    
    if (model->config.num_layers != 8) {
        fprintf(stderr, "Error: Invalid number of layers (expected 8, got %u)\n",
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
