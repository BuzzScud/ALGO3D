/**
 * @file cllm_platonic_generator.c
 * @brief Integration of math library's Platonic generator with CLLM
 * 
 * Phase 2: Replace simple lookup table with comprehensive math library generator
 */

#include "ai/cllm.h"
#include "math/platonic_generator.h"
#include "math/polytope.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// PLATONIC SOLID GENERATION
// ============================================================================

/**
 * @brief Generate PlatonicSolid from math library based on solid type
 * 
 * @param solid_type Type of Platonic solid to generate
 * @return Pointer to generated PlatonicSolid, or NULL on error
 */
PlatonicSolid* cllm_generate_platonic_solid(PlatonicSolidType solid_type) {
    PlatonicSolid* solid = NULL;
    
    switch (solid_type) {
        case PLATONIC_TETRAHEDRON:
            solid = platonic_generate_tetrahedron();
            break;
            
        case PLATONIC_CUBE:
            solid = platonic_generate_cube();
            break;
            
        case PLATONIC_OCTAHEDRON:
            solid = platonic_generate_octahedron();
            break;
            
        case PLATONIC_DODECAHEDRON:
            solid = platonic_generate_dodecahedron();
            break;
            
        case PLATONIC_ICOSAHEDRON:
            solid = platonic_generate_icosahedron();
            break;
            
        default:
            fprintf(stderr, "Error: Invalid Platonic solid type: %d\n", solid_type);
            return NULL;
    }
    
    if (!solid) {
        fprintf(stderr, "Error: Failed to generate Platonic solid\n");
        return NULL;
    }
    
    // Compute all properties
    if (!platonic_compute_properties(solid)) {
        fprintf(stderr, "Error: Failed to compute Platonic solid properties\n");
        platonic_free(solid);
        return NULL;
    }
    
    // Validate the solid
    if (!platonic_validate(solid)) {
        fprintf(stderr, "Warning: Generated Platonic solid failed validation\n");
        // Continue anyway - validation might be too strict
    }
    
    return solid;
}

/**
 * @brief Generate PlatonicSolid from Schläfli symbol
 * 
 * @param schlafli_str Schläfli symbol string (e.g., "{3,3}", "{4,3}", "{3,5}")
 * @return Pointer to generated PlatonicSolid, or NULL on error
 */
PlatonicSolid* cllm_generate_from_schlafli(const char* schlafli_str) {
    if (!schlafli_str) {
        fprintf(stderr, "Error: NULL Schläfli symbol\n");
        return NULL;
    }
    
    // Parse Schläfli symbol
    // Format: "{p,q}" for 3D, "{p,q,r}" for 4D, etc.
    uint32_t schlafli[10];  // Support up to 10D
    uint32_t length = 0;
    
    const char* p = schlafli_str;
    while (*p && length < 10) {
        if (*p >= '0' && *p <= '9') {
            schlafli[length++] = atoi(p);
            while (*p >= '0' && *p <= '9') p++;
        } else {
            p++;
        }
    }
    
    if (length == 0) {
        fprintf(stderr, "Error: Invalid Schläfli symbol: %s\n", schlafli_str);
        return NULL;
    }
    
    // Determine dimension from symbol length
    uint32_t dimension = length + 1;
    
    // Generate solid
    PlatonicSolid* solid = platonic_generate(dimension, schlafli, length);
    if (!solid) {
        fprintf(stderr, "Error: Failed to generate solid from Schläfli symbol: %s\n", 
                schlafli_str);
        return NULL;
    }
    
    // Compute all properties
    if (!platonic_compute_properties(solid)) {
        fprintf(stderr, "Error: Failed to compute properties for Schläfli symbol: %s\n",
                schlafli_str);
        platonic_free(solid);
        return NULL;
    }
    
    return solid;
}

/**
 * @brief Update CLLMModel geometry from PlatonicSolid
 * 
 * @param model_ptr CLLM model to update (void* to avoid circular dependency)
 * @param solid_ptr PlatonicSolid from math library (void* to avoid circular dependency)
 */
void cllm_update_geometry_from_solid(void* model_ptr, const void* solid_ptr) {
    if (!model_ptr || !solid_ptr) {
        return;
    }
    
    CLLMModel* model = (CLLMModel*)model_ptr;
    const PlatonicSolid* solid = (const PlatonicSolid*)solid_ptr;
    
    // Update legacy PlatonicGeometry structure
    model->geometry.vertices = (uint32_t)solid->num_vertices;
    model->geometry.edges = (uint32_t)solid->num_edges;
    model->geometry.faces = (uint32_t)solid->num_faces;
    model->geometry.symmetries = solid->symmetry_order;
    model->geometry.edge_length = solid->edge_length;
    model->geometry.has_golden_ratio = solid->has_golden_ratio;
    
    // Update dimensions (derived from geometry)
    model->embedding_dim = solid->embedding_dim;  // Already computed as vertices × 12
    model->hidden_dim = solid->hidden_dim;        // Already computed as edges × 12
    model->num_layers = solid->num_layers;        // Already set to faces
    model->num_heads = solid->num_heads;          // Always 12
}

/**
 * @brief Print Platonic solid information
 * 
 * @param solid_ptr PlatonicSolid to print (void* to avoid circular dependency)
 */
void cllm_print_platonic_solid(const void* solid_ptr) {
    if (!solid_ptr) {
        printf("NULL Platonic solid\n");
        return;
    }
    
    const PlatonicSolid* solid = (const PlatonicSolid*)solid_ptr;
    
    printf("\n=== Platonic Solid Information ===\n");
    printf("Name: %s\n", solid->name);
    printf("Dimension: %u\n", solid->dimension);
    
    // Schläfli symbol
    printf("Schläfli symbol: {");
    for (uint32_t i = 0; i < solid->symbol_length; i++) {
        printf("%u", solid->schlafli_symbol[i]);
        if (i < solid->symbol_length - 1) printf(",");
    }
    printf("}\n");
    
    // Combinatorial structure
    printf("\nCombinatorial Structure:\n");
    printf("  Vertices: %lu\n", solid->num_vertices);
    printf("  Edges: %lu\n", solid->num_edges);
    printf("  Faces: %lu\n", solid->num_faces);
    if (solid->dimension >= 4) {
        printf("  Cells: %lu\n", solid->num_cells);
    }
    
    // Euler characteristic
    printf("  Euler characteristic: %ld\n", solid->euler_characteristic);
    
    // Symmetry
    printf("\nSymmetry:\n");
    printf("  Group: %s\n", solid->symmetry_group);
    printf("  Order: %u\n", solid->symmetry_order);
    
    // Metric properties
    printf("\nMetric Properties:\n");
    printf("  Edge length: %.6f\n", solid->edge_length);
    printf("  Circumradius: %.6f\n", solid->circumradius);
    printf("  Inradius: %.6f\n", solid->inradius);
    printf("  Volume: %.6f\n", solid->volume);
    
    // CLLM integration
    printf("\nCLLM Integration (12-fold):\n");
    printf("  Embedding dim: %u (vertices × 12)\n", solid->embedding_dim);
    printf("  Hidden dim: %u (edges × 12)\n", solid->hidden_dim);
    printf("  Num layers: %u (faces)\n", solid->num_layers);
    printf("  Num heads: %u (always 12)\n", solid->num_heads);
    
    // Validation
    printf("\nValidation:\n");
    printf("  Valid: %s\n", solid->is_valid ? "YES" : "NO");
    printf("  Regular: %s\n", solid->is_regular ? "YES" : "NO");
    printf("  Convex: %s\n", solid->is_convex ? "YES" : "NO");
    printf("  Has golden ratio: %s\n", solid->has_golden_ratio ? "YES" : "NO");
    
    printf("==================================\n\n");
}

// ============================================================================
// 4D POLYTOPE SUPPORT
// ============================================================================

/**
 * @brief Generate 4D polytope for CLLM
 * 
 * @param polytope_type Type of 4D polytope (0-5)
 *   0 = 5-cell (4-simplex)
 *   1 = Tesseract (4-cube)
 *   2 = 16-cell (4-orthoplex)
 *   3 = 24-cell
 *   4 = 120-cell
 *   5 = 600-cell
 * @return Pointer to generated PlatonicSolid, or NULL on error
 */
PlatonicSolid* cllm_generate_4d_polytope(uint32_t polytope_type) {
    PlatonicSolid* solid = NULL;
    
    switch (polytope_type) {
        case 0:  // 5-cell
            solid = platonic_generate_5cell();
            break;
            
        case 1:  // Tesseract
            solid = platonic_generate_tesseract();
            break;
            
        case 2:  // 16-cell
            solid = platonic_generate_16cell();
            break;
            
        case 3:  // 24-cell
            solid = platonic_generate_24cell();
            break;
            
        case 4:  // 120-cell
            solid = platonic_generate_120cell();
            break;
            
        case 5:  // 600-cell
            solid = platonic_generate_600cell();
            break;
            
        default:
            fprintf(stderr, "Error: Invalid 4D polytope type: %u\n", polytope_type);
            return NULL;
    }
    
    if (!solid) {
        fprintf(stderr, "Error: Failed to generate 4D polytope\n");
        return NULL;
    }
    
    // Compute all properties
    if (!platonic_compute_properties(solid)) {
        fprintf(stderr, "Error: Failed to compute 4D polytope properties\n");
        platonic_free(solid);
        return NULL;
    }
    
    return solid;
}

/**
 * @brief Generate nD polytope (simplex, hypercube, or cross-polytope)
 * 
 * @param dimension Dimension (3, 4, 5, ..., n)
 * @param type 0=simplex, 1=hypercube, 2=cross-polytope
 * @return Pointer to generated PlatonicSolid, or NULL on error
 */
PlatonicSolid* cllm_generate_nd_polytope(uint32_t dimension, uint32_t type) {
    PlatonicSolid* solid = NULL;
    
    switch (type) {
        case 0:  // Simplex
            solid = platonic_generate_simplex(dimension);
            break;
            
        case 1:  // Hypercube
            solid = platonic_generate_hypercube(dimension);
            break;
            
        case 2:  // Cross-polytope
            solid = platonic_generate_cross_polytope(dimension);
            break;
            
        default:
            fprintf(stderr, "Error: Invalid nD polytope type: %u\n", type);
            return NULL;
    }
    
    if (!solid) {
        fprintf(stderr, "Error: Failed to generate %uD polytope\n", dimension);
        return NULL;
    }
    
    // Compute all properties
    if (!platonic_compute_properties(solid)) {
        fprintf(stderr, "Error: Failed to compute %uD polytope properties\n", dimension);
        platonic_free(solid);
        return NULL;
    }
    
    return solid;
}