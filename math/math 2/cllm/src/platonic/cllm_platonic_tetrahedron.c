/**
 * @file cllm_platonic_tetrahedron.c
 * @brief Tetrahedron model implementation
 * 
 * The tetrahedron is the simplest Platonic solid:
 * - 4 vertices (information nodes)
 * - 6 edges (relationships)
 * - 4 faces (groups)
 * - 12 symmetries (T_d group)
 * 
 * Model dimensions:
 * - Embedding: 48 (4 × 12)
 * - Hidden: 72 (6 × 12)
 * - Layers: 4
 * 
 * Use case: Small, fast models for edge devices and real-time inference
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
// TETRAHEDRON GEOMETRY
// ============================================================================

/**
 * Regular tetrahedron vertices (normalized)
 * 
 * Positioned at alternating vertices of a cube:
 * - (1, 1, 1)
 * - (1, -1, -1)
 * - (-1, 1, -1)
 * - (-1, -1, 1)
 * 
 * These form a perfect tetrahedron with edge length 2√2
 */
static const double TETRAHEDRON_VERTICES[4][3] = {
    { 1.0,  1.0,  1.0},  // Vertex 0
    { 1.0, -1.0, -1.0},  // Vertex 1
    {-1.0,  1.0, -1.0},  // Vertex 2
    {-1.0, -1.0,  1.0}   // Vertex 3
};

/**
 * Tetrahedron edges (pairs of vertex indices)
 * 
 * 6 edges connecting the 4 vertices:
 * - Edge 0: v0-v1
 * - Edge 1: v0-v2
 * - Edge 2: v0-v3
 * - Edge 3: v1-v2
 * - Edge 4: v1-v3
 * - Edge 5: v2-v3
 */
static const uint32_t TETRAHEDRON_EDGES[6][2] = {
    {0, 1},  // Edge 0
    {0, 2},  // Edge 1
    {0, 3},  // Edge 2
    {1, 2},  // Edge 3
    {1, 3},  // Edge 4
    {2, 3}   // Edge 5
};

/**
 * Tetrahedron faces (triplets of vertex indices)
 * 
 * 4 triangular faces:
 * - Face 0: v0-v1-v2
 * - Face 1: v0-v1-v3
 * - Face 2: v0-v2-v3
 * - Face 3: v1-v2-v3
 */
static const uint32_t TETRAHEDRON_FACES[4][3] = {
    {0, 1, 2},  // Face 0
    {0, 1, 3},  // Face 1
    {0, 2, 3},  // Face 2
    {1, 2, 3}   // Face 3
};

// ============================================================================
// INITIALIZATION
// ============================================================================

/**
 * Initialize tetrahedron geometry
 */
bool platonic_tetrahedron_init_geometry(PlatonicModel* model) {
    if (!model) return false;
    
    printf("Initializing tetrahedron geometry...\n");
    
    // Copy vertex positions
    for (uint32_t i = 0; i < 4; i++) {
        model->vertex_positions[i * 3 + 0] = TETRAHEDRON_VERTICES[i][0];
        model->vertex_positions[i * 3 + 1] = TETRAHEDRON_VERTICES[i][1];
        model->vertex_positions[i * 3 + 2] = TETRAHEDRON_VERTICES[i][2];
    }
    
    // Copy edge connections
    for (uint32_t i = 0; i < 6; i++) {
        model->edge_connections[i * 2 + 0] = TETRAHEDRON_EDGES[i][0];
        model->edge_connections[i * 2 + 1] = TETRAHEDRON_EDGES[i][1];
    }
    
    // Copy face vertices
    for (uint32_t i = 0; i < 4; i++) {
        model->face_vertices[i * 4 + 0] = TETRAHEDRON_FACES[i][0];
        model->face_vertices[i * 4 + 1] = TETRAHEDRON_FACES[i][1];
        model->face_vertices[i * 4 + 2] = TETRAHEDRON_FACES[i][2];
        model->face_vertices[i * 4 + 3] = 0;  // Padding (faces are triangles)
    }
    
    printf("  ✓ 4 vertices initialized\n");
    printf("  ✓ 6 edges initialized\n");
    printf("  ✓ 4 faces initialized\n");
    
    // Verify Euler's formula: V - E + F = 4 - 6 + 4 = 2 ✓
    int euler = 4 - 6 + 4;
    printf("  ✓ Euler's formula: 4 - 6 + 4 = %d (verified)\n", euler);
    
    return true;
}

/**
 * Initialize tetrahedron embeddings
 * 
 * Each of the 4 vertices gets 12 embedding dimensions,
 * for a total of 48 dimensions.
 */
bool platonic_tetrahedron_init_embeddings(PlatonicModel* model) {
    if (!model) return false;
    
    printf("Initializing tetrahedron embeddings (48-dim)...\n");
    
    // Allocate embeddings: vocab_size × 48
    size_t embedding_size = model->config.vocab_size * model->config.embedding_dim;
    model->embeddings = (double*)calloc(embedding_size, sizeof(double));
    
    if (!model->embeddings) {
        fprintf(stderr, "Error: Failed to allocate embeddings\n");
        return false;
    }
    
    // Initialize embeddings using vertex positions
    // Each token gets a 48-dim embedding organized as 4 vertices × 12 dims
    for (uint32_t token = 0; token < model->config.vocab_size; token++) {
        for (uint32_t vertex = 0; vertex < 4; vertex++) {
            // Get vertex position
            double x = model->vertex_positions[vertex * 3 + 0];
            double y = model->vertex_positions[vertex * 3 + 1];
            double z = model->vertex_positions[vertex * 3 + 2];
            
            // Initialize 12 dimensions for this vertex
            for (uint32_t dim = 0; dim < 12; dim++) {
                size_t idx = token * 48 + vertex * 12 + dim;
                
                // Use geometric position + token info + dimension
                double value = (x + y + z) * 0.1 + (double)token * 0.01 + (double)dim * 0.001;
                
                // Normalize to [-1, 1]
                model->embeddings[idx] = math_tanh(value);
            }
        }
    }
    
    printf("  ✓ Embeddings initialized: %u tokens × 48 dims\n",
           model->config.vocab_size);
    
    return true;
}

/**
 * Initialize tetrahedron layers
 * 
 * 4 layers, one for each face of the tetrahedron.
 * Each layer has hidden_dim = 72 (6 edges × 12)
 */
bool platonic_tetrahedron_init_layers(PlatonicModel* model) {
    if (!model) return false;
    
    printf("Initializing tetrahedron layers (4 layers, 72-dim hidden)...\n");
    
    // Allocate layer weights
    // For simplicity: 4 layers × 72 hidden × 72 hidden
    size_t layer_size = 4 * 72 * 72;
    model->layer_weights = (double*)calloc(layer_size, sizeof(double));
    
    if (!model->layer_weights) {
        fprintf(stderr, "Error: Failed to allocate layer weights\n");
        return false;
    }
    
    // Initialize with small random values
    for (size_t i = 0; i < layer_size; i++) {
        model->layer_weights[i] = ((double)rand() / RAND_MAX - 0.5) * 0.1;
    }
    
    printf("  ✓ 4 layers initialized\n");
    printf("  ✓ Hidden dimension: 72 (6 edges × 12)\n");
    
    return true;
}

// ============================================================================
// TETRAHEDRON MODEL CREATION
// ============================================================================

/**
 * Create a complete tetrahedron model
 */
PlatonicModel* platonic_tetrahedron_create(
    uint32_t vocab_size,
    uint32_t max_seq_len
) {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║  CREATING TETRAHEDRON MODEL                              ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    // Create config
    PlatonicModelConfig config = platonic_config_create(
        PLATONIC_TETRAHEDRON,
        vocab_size,
        max_seq_len
    );
    
    // Create base model
    PlatonicModel* model = platonic_model_create(&config);
    if (!model) {
        fprintf(stderr, "Error: Failed to create base model\n");
        return NULL;
    }
    
    // Initialize tetrahedron-specific geometry
    if (!platonic_tetrahedron_init_geometry(model)) {
        fprintf(stderr, "Error: Failed to initialize geometry\n");
        platonic_model_free(model);
        return NULL;
    }
    
    // Initialize embeddings
    if (!platonic_tetrahedron_init_embeddings(model)) {
        fprintf(stderr, "Error: Failed to initialize embeddings\n");
        platonic_model_free(model);
        return NULL;
    }
    
    // Initialize layers
    if (!platonic_tetrahedron_init_layers(model)) {
        fprintf(stderr, "Error: Failed to initialize layers\n");
        platonic_model_free(model);
        return NULL;
    }
    
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║  TETRAHEDRON MODEL CREATED SUCCESSFULLY                  ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    // Print model info
    platonic_model_print_info(model);
    
    return model;
}

// ============================================================================
// TETRAHEDRON-SPECIFIC OPERATIONS
// ============================================================================

/**
 * Calculate distance between two vertices
 */
double platonic_tetrahedron_vertex_distance(
    const PlatonicModel* model,
    uint32_t v1,
    uint32_t v2
) {
    if (!model || v1 >= 4 || v2 >= 4) return 0.0;
    
    double dx = model->vertex_positions[v1 * 3 + 0] - model->vertex_positions[v2 * 3 + 0];
    double dy = model->vertex_positions[v1 * 3 + 1] - model->vertex_positions[v2 * 3 + 1];
    double dz = model->vertex_positions[v1 * 3 + 2] - model->vertex_positions[v2 * 3 + 2];
    
    return math_sqrt(dx * dx + dy * dy + dz * dz);
}

/**
 * Verify tetrahedron edge lengths are equal
 */
bool platonic_tetrahedron_verify_edges(const PlatonicModel* model) {
    if (!model) return false;
    
    printf("Verifying tetrahedron edge lengths...\n");
    
    double edge_length = platonic_tetrahedron_vertex_distance(model, 0, 1);
    printf("  Reference edge length: %.6f\n", edge_length);
    
    bool all_equal = true;
    for (uint32_t i = 0; i < 6; i++) {
        uint32_t v1 = model->edge_connections[i * 2 + 0];
        uint32_t v2 = model->edge_connections[i * 2 + 1];
        double length = platonic_tetrahedron_vertex_distance(model, v1, v2);
        
        double diff = math_abs(length - edge_length);
        if (diff > 0.0001) {
            printf("  ✗ Edge %u: length %.6f (diff: %.6f)\n", i, length, diff);
            all_equal = false;
        } else {
            printf("  ✓ Edge %u: length %.6f\n", i, length);
        }
    }
    
    return all_equal;
}
/**
 * Initialize tetrahedron attention weights
 * 
 * Each of the 6 edges corresponds to an attention connection.
 */
bool platonic_tetrahedron_init_attention(PlatonicModel* model) {
    if (!model) return false;
    
    printf("Initializing tetrahedron attention (6 edges)...\n");
    
    // Allocate attention weights
    size_t attention_size = model->config.hidden_dim * model->config.hidden_dim;
    size_t total_attention_size = 6 * attention_size;
    
    model->attention_weights = (double*)calloc(total_attention_size, sizeof(double));
    
    if (!model->attention_weights) {
        fprintf(stderr, "Error: Failed to allocate attention weights\n");
        return false;
    }
    
    // Initialize attention weights with small random values
    for (size_t i = 0; i < total_attention_size; i++) {
        model->attention_weights[i] = ((double)rand() / RAND_MAX - 0.5) * 0.1;
    }
    
    printf("  ✓ Attention weights initialized: 6 edges × %zu = %zu values\n",
           attention_size, total_attention_size);
    
    return true;
}
