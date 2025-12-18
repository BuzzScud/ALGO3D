/**
 * @file cllm_create_from_schlafli.c
 * @brief Create CLLM model from Schläfli symbol
 * 
 * Phase 2: Enable dynamic model creation from geometric notation
 */

#include "ai/cllm.h"
#include "math/platonic_generator.h"
#include "math/polytope.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// External functions
extern CLLMModel* cllm_create_model(const CLLMConfig* config);
extern void cllm_update_geometry_from_solid(void* model, const void* solid);
extern void cllm_print_platonic_solid(const void* solid);
extern PlatonicSolid* cllm_generate_from_schlafli(const char* schlafli_str);

/**
 * @brief Create CLLM model from Schläfli symbol
 * 
 * This enables dynamic model creation from geometric notation:
 * - 3D: {3,3} = Tetrahedron, {4,3} = Cube, {3,4} = Octahedron, 
 *       {5,3} = Dodecahedron, {3,5} = Icosahedron
 * - 4D: {3,3,3} = 5-cell, {4,3,3} = Tesseract, {3,3,4} = 16-cell,
 *       {3,4,3} = 24-cell, {5,3,3} = 120-cell, {3,3,5} = 600-cell
 * - nD: {3,3,...,3} = Simplex, {4,3,...,3} = Hypercube, {3,3,...,4} = Cross-polytope
 * 
 * @param vocab_size Vocabulary size
 * @param schlafli_symbol Schläfli symbol string (e.g., "{3,5}" for icosahedron)
 * @param max_seq_len Maximum sequence length
 * @return Pointer to created model, or NULL on error
 * 
 * Example usage:
 *   CLLMModel* model = cllm_create_from_schlafli(10000, "{3,5}", 512);  // Icosahedron
 *   CLLMModel* model = cllm_create_from_schlafli(10000, "{4,3,3}", 512); // Tesseract (4D)
 */
CLLMModel* cllm_create_from_schlafli(uint32_t vocab_size, 
                                     const char* schlafli_symbol,
                                     uint32_t max_seq_len) {
    if (!schlafli_symbol) {
        fprintf(stderr, "Error: NULL Schläfli symbol\n");
        return NULL;
    }
    
    printf("🔷 Creating CLLM model from Schläfli symbol: %s\n", schlafli_symbol);
    
    // Generate Platonic solid from Schläfli symbol
    PlatonicSolid* solid = cllm_generate_from_schlafli(schlafli_symbol);
    if (!solid) {
        fprintf(stderr, "Error: Failed to generate solid from Schläfli symbol: %s\n",
                schlafli_symbol);
        return NULL;
    }
    
    // Print solid information
    printf("  ✓ Generated solid from Schläfli symbol:\n");
    cllm_print_platonic_solid(solid);
    
    // Create model configuration
    CLLMConfig config = {0};
    config.vocab_size = vocab_size;
    config.max_seq_len = max_seq_len;
    
    // Dimensions will be derived from the solid
    config.embedding_dim = 0;  // Auto-derive
    config.hidden_dim = 0;     // Auto-derive
    config.num_layers = 0;     // Auto-derive
    
    // Determine solid type (for 3D solids only)
    // For 4D and higher, we'll use PLATONIC_CUBE as a placeholder
    config.solid_type = PLATONIC_CUBE;  // Default
    
    // Try to match Schläfli symbol to known 3D solids
    if (strcmp(schlafli_symbol, "{3,3}") == 0) {
        config.solid_type = PLATONIC_TETRAHEDRON;
    } else if (strcmp(schlafli_symbol, "{4,3}") == 0) {
        config.solid_type = PLATONIC_CUBE;
    } else if (strcmp(schlafli_symbol, "{3,4}") == 0) {
        config.solid_type = PLATONIC_OCTAHEDRON;
    } else if (strcmp(schlafli_symbol, "{5,3}") == 0) {
        config.solid_type = PLATONIC_DODECAHEDRON;
    } else if (strcmp(schlafli_symbol, "{3,5}") == 0) {
        config.solid_type = PLATONIC_ICOSAHEDRON;
    }
    
    // Create model using standard creation function
    CLLMModel* model = cllm_create_model(&config);
    if (!model) {
        fprintf(stderr, "Error: Failed to create CLLM model\n");
        platonic_free(solid);
        return NULL;
    }
    
    // Replace the auto-generated solid with our custom one
    if (model->platonic_solid) {
        platonic_free((PlatonicSolid*)model->platonic_solid);
    }
    model->platonic_solid = solid;
    
    // Update geometry from the solid
    cllm_update_geometry_from_solid(model, solid);
    
    printf("  ✓ Model created successfully from Schläfli symbol\n");
    printf("  ✓ Dimensions: embedding=%u, hidden=%u, layers=%u, heads=%u\n",
           model->embedding_dim, model->hidden_dim, model->num_layers, model->num_heads);
    
    return model;
}

/**
 * @brief Create CLLM model from 4D polytope
 * 
 * @param vocab_size Vocabulary size
 * @param polytope_type Type of 4D polytope (0-5)
 *   0 = 5-cell, 1 = Tesseract, 2 = 16-cell, 3 = 24-cell, 4 = 120-cell, 5 = 600-cell
 * @param max_seq_len Maximum sequence length
 * @return Pointer to created model, or NULL on error
 */
CLLMModel* cllm_create_from_4d_polytope(uint32_t vocab_size,
                                        uint32_t polytope_type,
                                        uint32_t max_seq_len) {
    const char* polytope_names[] = {
        "5-cell", "Tesseract", "16-cell", "24-cell", "120-cell", "600-cell"
    };
    
    const char* schlafli_symbols[] = {
        "{3,3,3}", "{4,3,3}", "{3,3,4}", "{3,4,3}", "{5,3,3}", "{3,3,5}"
    };
    
    if (polytope_type > 5) {
        fprintf(stderr, "Error: Invalid 4D polytope type: %u (must be 0-5)\n", polytope_type);
        return NULL;
    }
    
    printf("🔷 Creating CLLM model from 4D polytope: %s\n", polytope_names[polytope_type]);
    
    return cllm_create_from_schlafli(vocab_size, schlafli_symbols[polytope_type], max_seq_len);
}

/**
 * @brief Create CLLM model from nD polytope
 * 
 * @param vocab_size Vocabulary size
 * @param dimension Dimension (3, 4, 5, ..., n)
 * @param polytope_type Type: 0=simplex, 1=hypercube, 2=cross-polytope
 * @param max_seq_len Maximum sequence length
 * @return Pointer to created model, or NULL on error
 */
CLLMModel* cllm_create_from_nd_polytope(uint32_t vocab_size,
                                        uint32_t dimension,
                                        uint32_t polytope_type,
                                        uint32_t max_seq_len) {
    const char* type_names[] = {"Simplex", "Hypercube", "Cross-polytope"};
    
    if (polytope_type > 2) {
        fprintf(stderr, "Error: Invalid nD polytope type: %u (must be 0-2)\n", polytope_type);
        return NULL;
    }
    
    if (dimension < 3) {
        fprintf(stderr, "Error: Invalid dimension: %u (must be >= 3)\n", dimension);
        return NULL;
    }
    
    printf("🔷 Creating CLLM model from %uD %s\n", dimension, type_names[polytope_type]);
    
    // Generate Schläfli symbol
    char schlafli[256];
    schlafli[0] = '{';
    schlafli[1] = '\0';
    
    if (polytope_type == 0) {
        // Simplex: {3,3,...,3} (dimension-1 threes)
        for (uint32_t i = 0; i < dimension - 1; i++) {
            if (i > 0) strcat(schlafli, ",");
            strcat(schlafli, "3");
        }
    } else if (polytope_type == 1) {
        // Hypercube: {4,3,...,3} (dimension-2 threes)
        strcat(schlafli, "4");
        for (uint32_t i = 1; i < dimension - 1; i++) {
            strcat(schlafli, ",3");
        }
    } else {
        // Cross-polytope: {3,3,...,4} (dimension-2 threes, then 4)
        for (uint32_t i = 0; i < dimension - 2; i++) {
            if (i > 0) strcat(schlafli, ",");
            strcat(schlafli, "3");
        }
        strcat(schlafli, ",4");
    }
    
    strcat(schlafli, "}");
    
    printf("  → Schläfli symbol: %s\n", schlafli);
    
    return cllm_create_from_schlafli(vocab_size, schlafli, max_seq_len);
}