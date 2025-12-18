/**
 * @file cllm_unified_model.h
 * @brief Unified CLLM Model - Geometric Foundation
 * 
 * This header defines the unified model structure that consolidates
 * fragmented implementations around Platonic solid geometry.
 * 
 * Replaces:
 * - cllm_create.c
 * - cllm_init.c
 * - cllm_validate.c
 * 
 * Key Features:
 * - Platonic solid geometry as foundation
 * - Clock lattice coordinate system
 * - Blind recovery support
 * - Harmonic integration
 * - Unified threading model
 */

#ifndef CLLM_UNIFIED_MODEL_H
#define CLLM_UNIFIED_MODEL_H

#include <stdint.h>
#include <stdbool.h>
#include "cllm.h"
#include "ai/cllm_platonic.h"
#include "clock_lattice.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// UNIFIED MODEL CONFIGURATION
// ============================================================================

/**
 * Optimizer types
 */
typedef enum {
    OPTIMIZER_SGD = 0,
    OPTIMIZER_ADAM = 1,
    OPTIMIZER_RMSPROP = 2
} OptimizerType;

/**
 * Unified Model Configuration
 * 
 * Consolidates all model parameters with geometric foundation.
 * Dimensions are automatically calculated from Platonic solid geometry.
 */
typedef struct {
    // ========================================================================
    // GEOMETRIC FOUNDATION
    // ========================================================================
    
    /**
     * Which Platonic solid to use for model architecture
     * - PLATONIC_TETRAHEDRON: Small, fast (48-dim embeddings)
     * - PLATONIC_CUBE: Balanced (96-dim embeddings)
     * - PLATONIC_OCTAHEDRON: Dual of cube (72-dim embeddings)
     * - PLATONIC_DODECAHEDRON: Large, powerful (240-dim embeddings)
     * - PLATONIC_ICOSAHEDRON: Maximum symmetry (144-dim embeddings)
     */
    PlatonicSolidType solid_type;
    
    // ========================================================================
    // BASIC PARAMETERS
    // ========================================================================
    
    uint32_t vocab_size;        // Vocabulary size
    uint32_t max_seq_len;       // Maximum sequence length
    
    // ========================================================================
    // DERIVED DIMENSIONS (auto-calculated from geometry)
    // ========================================================================
    
    uint32_t embedding_dim;     // vertices × 12
    uint32_t hidden_dim;        // edges × 12
    uint32_t num_layers;        // faces (or other property)
    uint32_t num_heads;         // Always 12 (12-fold symmetry)
    
    // ========================================================================
    // FEATURE FLAGS
    // ========================================================================
    
    bool enable_blind_recovery;      // Enable corruption recovery
    bool enable_harmonic_integration; // Enable Fourier/cymatic/prime resonance
    bool enable_ntt_attention;       // Use O(n log n) NTT attention
    
    // ========================================================================
    // TRAINING PARAMETERS
    // ========================================================================
    
    OptimizerType optimizer_type;    // Which optimizer to use
    double learning_rate;            // Initial learning rate
    double beta1;                    // Adam beta1 (if using Adam)
    double beta2;                    // Adam beta2 (if using Adam)
    double epsilon;                  // Optimizer epsilon
    
} UnifiedModelConfig;

// ============================================================================
// UNIFIED MODEL STRUCTURE
// ============================================================================

/**
 * Unified CLLM Model
 * 
 * Single structure replacing fragmented CLLMModel implementations.
 * Based on Platonic solid geometry with clock lattice coordinates.
 */
typedef struct UnifiedCLLMModel {
    // ========================================================================
    // GEOMETRIC FOUNDATION
    // ========================================================================
    
    PlatonicModel* platonic;         // Underlying Platonic solid model
    PlatonicGeometry geometry;       // V, E, F, symmetries, golden ratio
    
    // Clock lattice mapping
    BabylonianClockPosition* token_positions;  // [vocab_size] - map tokens to clock
    double* angular_positions;       // [vocab_size] - θ(n,k,λ,ω,ψ)
    
    // ========================================================================
    // MODEL DIMENSIONS
    // ========================================================================
    
    uint32_t vocab_size;
    uint32_t embedding_dim;          // vertices × 12
    uint32_t hidden_dim;             // edges × 12
    uint32_t num_layers;             // faces
    uint32_t num_heads;              // Always 12
    uint32_t max_seq_len;
    
    // ========================================================================
    // MODEL PARAMETERS
    // ========================================================================
    
    // Embeddings
    double* embeddings;              // [vocab_size × embedding_dim]
    double* pos_encoding;            // [max_seq_len × embedding_dim]
    
    // Transformer layers
    struct {
        // Attention weights (12 heads)
        double* qkv_weights;         // [3 × embedding_dim × embedding_dim]
        double* output_weights;      // [embedding_dim × embedding_dim]
        
        // Feed-forward network
        double* ffn_w1;              // [embedding_dim × hidden_dim]
        double* ffn_w2;              // [hidden_dim × embedding_dim]
        double* ffn_b1;              // [hidden_dim]
        double* ffn_b2;              // [embedding_dim]
        
        // Layer normalization
        double* ln1_gamma;           // [embedding_dim]
        double* ln1_beta;            // [embedding_dim]
        double* ln2_gamma;           // [embedding_dim]
        double* ln2_beta;            // [embedding_dim]
    } *layers;                       // [num_layers]
    
    // Output projection
    double* output_weights;          // [embedding_dim × vocab_size]
    double* output_bias;             // [vocab_size]
    
    // ========================================================================
    // FEATURE FLAGS
    // ========================================================================
    
    bool blind_recovery_enabled;
    bool harmonic_enabled;
    bool ntt_attention_enabled;
    
    // ========================================================================
    // OPTIMIZER STATE
    // ========================================================================
    
    OptimizerType optimizer_type;
    double learning_rate;
    double beta1, beta2;
    double epsilon;
    
    // Optimizer buffers (for Adam/RMSprop)
    double* m;                       // First moment (momentum)
    double* v;                       // Second moment (velocity)
    uint64_t optimizer_step;         // Current step number
    
    // ========================================================================
    // TRAINING METRICS
    // ========================================================================
    
    uint64_t total_steps;
    double current_loss;
    double best_loss;
    uint64_t tokens_processed;
    
} UnifiedCLLMModel;

// ============================================================================
// API FUNCTIONS
// ============================================================================

/**
 * Create unified model from configuration
 * 
 * Automatically calculates dimensions from Platonic geometry:
 * - embedding_dim = vertices × 12
 * - hidden_dim = edges × 12
 * - num_layers = faces
 * - num_heads = 12 (always)
 * 
 * @param config Model configuration
 * @return Newly created model, or NULL on failure
 */
UnifiedCLLMModel* unified_model_create(const UnifiedModelConfig* config);

/**
 * Free unified model and all resources
 * 
 * @param model Model to free
 */
void unified_model_free(UnifiedCLLMModel* model);

/**
 * Validate model structure
 * 
 * Checks:
 * - Euler's formula: V - E + F = 2
 * - Dimension consistency
 * - Parameter allocation
 * - Clock lattice positions
 * 
 * @param model Model to validate
 * @return true if valid, false otherwise
 */
bool unified_model_validate(const UnifiedCLLMModel* model);

/**
 * Print model information
 * 
 * @param model Model to print
 */
void unified_model_print_info(const UnifiedCLLMModel* model);

// ============================================================================
// CONVERSION FUNCTIONS (for backward compatibility)
// ============================================================================

/**
 * Convert old CLLMModel to unified format
 * 
 * Infers appropriate Platonic solid from dimensions.
 * 
 * @param old_model Old model to convert
 * @return Unified model, or NULL on failure
 */
UnifiedCLLMModel* unified_model_from_cllm(CLLMModel* old_model);

/**
 * Convert unified model to old CLLMModel format
 * 
 * For backward compatibility with existing code.
 * 
 * @param unified Unified model to convert
 * @return Old-format model, or NULL on failure
 */
CLLMModel* unified_model_to_cllm(UnifiedCLLMModel* unified);

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * Infer Platonic solid type from dimensions
 * 
 * Used for migration from old models.
 * 
 * @param embedding_dim Embedding dimension
 * @param num_layers Number of layers
 * @return Best-matching Platonic solid type
 */
PlatonicSolidType unified_model_infer_solid(
    uint32_t embedding_dim,
    uint32_t num_layers
);

/**
 * Calculate total parameter count
 * 
 * @param model Model to count parameters for
 * @return Total number of parameters
 */
uint64_t unified_model_count_parameters(const UnifiedCLLMModel* model);

#ifdef __cplusplus
}
#endif

#endif // CLLM_UNIFIED_MODEL_H