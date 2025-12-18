/**
 * @file cllm_platonic.h
 * @brief Platonic Solid Model Architecture
 * 
 * Revolutionary neural network architecture based on Platonic solids.
 * Each model size corresponds to a perfect geometric form, enabling:
 * - Blind recovery from corruption (Euler's formula: V - E + F = 2)
 * - Maximum sphere packing (optimal information density)
 * - Harmonic relationships (Fourier/cymatic/prime resonance)
 * - Consistent architecture (all dimensions multiples of 12)
 * - Natural interpretability (visible geometric structure)
 * 
 * The Five Platonic Models:
 * 1. Tetrahedron (4,6,4) - Small, fast (48-dim embeddings)
 * 2. Cube (8,12,6) - Balanced (96-dim embeddings)
 * 3. Octahedron (6,12,8) - Dual of cube (72-dim embeddings)
 * 4. Dodecahedron (20,30,12) - Large, powerful (240-dim embeddings)
 * 5. Icosahedron (12,30,20) - Maximum symmetry (144-dim embeddings)
 */

#ifndef CLLM_PLATONIC_H
#define CLLM_PLATONIC_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "cllm.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// PLATONIC SOLID TYPES
// ============================================================================

// Note: PlatonicSolidType and PlatonicGeometry are now defined in cllm.h
// to avoid circular dependencies. We just use them here.

// ============================================================================
// MODEL CONFIGURATION
// ============================================================================

/**
 * Configuration for Platonic model
 */
typedef struct {
    PlatonicSolidType solid_type;  // Which Platonic solid to use
    
    // Dimensions (automatically calculated from solid_type)
    uint32_t embedding_dim;    // vertices × 12
    uint32_t hidden_dim;       // edges × 12
    uint32_t num_layers;       // faces (or other solid property)
    
    // Standard CLLM config
    uint32_t vocab_size;
    uint32_t num_heads;        // Always 12 (12-fold symmetry)
    uint32_t max_seq_len;
    
    // Geometric options
    bool enable_blind_recovery;     // Enable recovery mechanisms
    bool enable_fourier_transform;  // Use Platonic Fourier
    bool enable_cymatic_modulation; // Use cymatic frequencies
    bool enable_tetration_optimizer;// Use tetration attractors
    
    // Recovery options
    double corruption_tolerance;    // Max corruption % for recovery
    uint32_t max_recovery_iterations;
    
} PlatonicModelConfig;

/**
 * Platonic model structure
 */
typedef struct {
    PlatonicModelConfig config;
    PlatonicGeometry geometry;
    
    // Geometric structure
    double* vertex_positions;   // 3D coordinates of vertices
    uint32_t* edge_connections; // Pairs of vertex indices
    uint32_t* face_vertices;    // Vertex indices for each face
    
    // Symmetry operations (rotation matrices)
    double* symmetry_operations;
    uint32_t num_symmetries;
    
    // Model weights (organized geometrically)
    double* embeddings;         // Mapped to vertices
    double* layer_weights;      // Mapped to faces
    double* attention_weights;  // Mapped to edges
    
    // Recovery state
    bool is_corrupted;
    double corruption_level;
    
    // Underlying CLLM model
    CLLMModel* cllm_model;
    
} PlatonicModel;

// ============================================================================
// CORE API
// ============================================================================

/**
 * Get geometric properties for a Platonic solid
 */
PlatonicGeometry platonic_get_geometry(PlatonicSolidType solid_type);

/**
 * Create a Platonic model configuration with default values
 */
PlatonicModelConfig platonic_config_create(
    PlatonicSolidType solid_type,
    uint32_t vocab_size,
    uint32_t max_seq_len
);

/**
 * Create a Platonic model
 */
PlatonicModel* platonic_model_create(const PlatonicModelConfig* config);

/**
 * Free a Platonic model
 */
void platonic_model_free(PlatonicModel* model);

/**
 * Train a Platonic model
 */
bool platonic_model_train(
    PlatonicModel* model,
    const uint32_t* tokens,
    size_t num_tokens,
    uint32_t epochs,
    double learning_rate
);

/**
 * Run inference with a Platonic model
 */
uint32_t platonic_model_infer(
    PlatonicModel* model,
    const uint32_t* input_tokens,
    size_t num_input_tokens,
    uint32_t* output_tokens,
    size_t max_output_tokens
);

/**
 * Save a Platonic model
 */
bool platonic_model_save(const PlatonicModel* model, const char* path);

/**
 * Load a Platonic model
 */
PlatonicModel* platonic_model_load(const char* path);

// ============================================================================
// GEOMETRIC OPERATIONS
// ============================================================================

/**
 * Project data onto Platonic manifold
 */
void platonic_project_to_manifold(
    PlatonicModel* model,
    double* data,
    size_t data_size
);

/**
 * Apply symmetry operation to data
 */
void platonic_apply_symmetry(
    PlatonicModel* model,
    double* data,
    size_t data_size,
    uint32_t symmetry_index
);

/**
 * Verify Euler's formula: V - E + F = 2
 */
bool platonic_verify_euler(const PlatonicGeometry* geometry);

// ============================================================================
// INTERNAL GEOMETRY INITIALIZATION (for recovery)
// ============================================================================

/**
 * Initialize geometry for each solid type (used by recovery mechanisms)
 */
bool platonic_tetrahedron_init_geometry(PlatonicModel* model);
bool platonic_cube_init_geometry(PlatonicModel* model);
bool platonic_octahedron_init_geometry(PlatonicModel* model);
bool platonic_dodecahedron_init_geometry(PlatonicModel* model);
bool platonic_icosahedron_init_geometry(PlatonicModel* model);

// ============================================================================
// BLIND RECOVERY API
// ============================================================================

/**
 * Recovery result structure
 */
typedef struct {
    bool success;
    double recovery_time_ms;
    double final_corruption_level;
    uint32_t iterations_used;
    char method_used[64];  // Which recovery method succeeded
} RecoveryResult;

/**
 * Corrupt a model (for testing)
 */
void platonic_corrupt_model(
    PlatonicModel* model,
    double corruption_percentage
);

/**
 * Recover model from corruption using structural redundancy
 */
RecoveryResult platonic_recover_structural(PlatonicModel* model);

/**
 * Recover model using symmetry operations
 */
RecoveryResult platonic_recover_symmetry(PlatonicModel* model);

/**
 * Recover model using prime validation
 */
RecoveryResult platonic_recover_prime(PlatonicModel* model);

/**
 * Recover model using tetration attractors
 */
RecoveryResult platonic_recover_tetration(PlatonicModel* model);

/**
 * Automatic recovery (tries all methods)
 */
RecoveryResult platonic_recover_auto(PlatonicModel* model);

// ============================================================================
// CONVERSION API
// ============================================================================

/**
 * Convert between Platonic models
 */
PlatonicModel* platonic_convert(
    const PlatonicModel* source,
    PlatonicSolidType target_type
);

/**
 * Convert to dual solid (cube ↔ octahedron, dodecahedron ↔ icosahedron)
 */
PlatonicModel* platonic_convert_to_dual(const PlatonicModel* model);

// ============================================================================
// HARMONIC INTEGRATION API
// ============================================================================

/**
 * Apply Platonic Fourier transform
 */
void platonic_fourier_transform(
    PlatonicModel* model,
    const double* signal,
    double* coefficients,
    size_t signal_size
);

/**
 * Apply cymatic frequency modulation to gradients
 */
void platonic_cymatic_modulate(
    PlatonicModel* model,
    double* gradients,
    size_t gradient_size,
    double frequency_hz
);

/**
 * Apply prime resonance alignment to attention
 */
void platonic_align_attention_resonance(
    PlatonicModel* model,
    double* attention_weights,
    size_t attention_size
);

/**
 * Map clock lattice rings to frequency bands
 */
void platonic_map_clock_to_frequencies(
    PlatonicModel* model,
    double* frequency_bands,
    size_t num_bands
);

/**
 * Apply tetration-based learning rate
 */
double platonic_tetration_learning_rate(
    PlatonicModel* model,
    uint32_t epoch,
    uint32_t base
);

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * Get human-readable name for solid type
 */
const char* platonic_solid_name(PlatonicSolidType solid_type);

/**
 * Print model information
 */
void platonic_model_print_info(const PlatonicModel* model);

/**
 * Validate model geometric structure
 */
bool platonic_model_validate(const PlatonicModel* model);

/**
 * Calculate sphere packing efficiency
 */
double platonic_sphere_packing_efficiency(PlatonicSolidType solid_type);

#ifdef __cplusplus
}
#endif

#endif // CLLM_PLATONIC_H