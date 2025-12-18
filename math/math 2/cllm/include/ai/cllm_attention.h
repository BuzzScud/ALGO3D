#ifndef CLLM_CRYSTALLINE_ATTENTION_H
#define CLLM_CRYSTALLINE_ATTENTION_H

#include "cllm.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Crystalline Attention - Advanced Implementation
 * 
 * This module implements the complete crystalline lattice attention mechanism
 * with all advanced mathematical features:
 * 
 * 1. Q→K Reversal: Transform query to key space through lattice operations
 * 2. Hyperdimensional Resonance: Multi-dimensional similarity computation
 * 3. Lattice-Based Attention: Use crystalline coordinates for attention weights
 * 4. Symmetry Operations: Apply rotations, reflections, and Möbius transforms
 * 5. Fourier Dampening: Signal processing for smooth attention
 * 6. Plimpton Ratios: base-60 Pythagorean triple corrections
 * 7. Einstein Lambda: Cosmological constant for gradient stability
 * 8. Cymatic Resonance: Harmonic frequency modulation
 * 9. Schumann Resonance: Earth frequency dampening
 * 10. Gamma Burst: Neural oscillation enhancement
 */

/**
 * Crystalline Attention Forward Pass
 * 
 * Complete implementation with all advanced features.
 * 
 * The attention mechanism works as follows:
 * 1. Project input to Q, K, V using lattice weights
 * 2. Apply Q→K reversal to transform queries into key space
 * 3. Compute hyperdimensional resonance between queries and keys
 * 4. Apply crystalline transformations:
 *    - Möbius transform (conformal mapping)
 *    - Plimpton ratio correction (geometric relationships)
 *    - Cymatic resonance (harmonic frequencies)
 *    - Schumann dampening (stability)
 *    - Gamma burst (neural enhancement)
 * 5. Softmax normalization
 * 6. Apply attention to values
 * 
 * @param layer Attention layer with Q, K, V projection weights
 * @param input Input sequence [seq_len x embedding_dim]
 * @param output Output sequence [seq_len x embedding_dim]
 * @param lattice_coords Lattice coordinates for each token [seq_len x 3]
 *                       Can be NULL for standard attention
 * @param token_primes Prime numbers associated with each token [seq_len]
 *                     Can be NULL, defaults to 2
 * @param seq_len Sequence length
 */
// LEGACY: Commented out - uses old AttentionLayer structure
// void cllm_crystalline_attention_forward(AttentionLayer* layer,
//                                        const float* input,
//                                        float* output,
//                                        const float* lattice_coords,
//                                        const uint64_t* token_primes,
//                                        int seq_len);

/**
 * Apply Einstein Lambda correction to gradients
 * 
 * Corrects gradients using Einstein's cosmological constant (Λ = 3/144000).
 * This provides stability and prevents gradient explosion during training.
 * 
 * Formula: g' = g * (1 - Λ)
 * 
 * @param gradients Gradient array to correct
 * @param size Array size
 */
void cllm_apply_einstein_correction(float* gradients, size_t size);

/**
 * Compute lattice coordinates for a token
 * 
 * Maps a token to 3D crystalline lattice coordinates based on:
 * - Prime factorization
 * - Ulam spiral positioning
 * - Golden angle packing
 * 
 * @param token_id Token ID
 * @param prime Associated prime number
 * @param coords Output: 3D coordinates [x, y, z]
 */
void cllm_compute_token_lattice_coords(uint32_t token_id, uint64_t prime, float* coords);

/**
 * Get prime number for token
 * 
 * Maps token ID to a prime number for crystalline operations.
 * Uses various strategies:
 * - Direct prime for small token IDs
 * - Prime factorization for composite IDs
 * - Coprime relationships for semantic similarity
 * 
 * @param token_id Token ID
 * @return Associated prime number
 */
uint64_t cllm_get_token_prime(uint32_t token_id);

/**
 * Compute semantic similarity using prime factorization
 * 
 * Two tokens are similar if their primes are:
 * - Coprime (gcd = 1): Maximally different
 * - Share factors: Semantically related
 * - One divides other: Hierarchical relationship
 * 
 * @param prime1 First token's prime
 * @param prime2 Second token's prime
 * @return Similarity score [0, 1]
 */
double cllm_compute_prime_similarity(uint64_t prime1, uint64_t prime2);

/**
 * Apply symmetry operation to attention weights
 * 
 * Applies crystalline symmetry operations:
 * - Rotations (12-fold symmetry)
 * - Reflections (mirror planes)
 * - Inversions (point symmetry)
 * 
 * @param weights Attention weights [seq_len]
 * @param seq_len Sequence length
 * @param symmetry_type Type of symmetry (0-11 for rotations, 12-23 for reflections)
 */
void cllm_apply_symmetry_operation(float* weights, int seq_len, int symmetry_type);

/**
 * Compute Fourier transform of attention pattern
 * 
 * Analyzes attention in frequency domain for:
 * - Pattern detection
 * - Noise filtering
 * - Resonance identification
 * 
 * @param attention_weights Attention weights [seq_len]
 * @param fourier_output Output: Fourier coefficients [seq_len]
 * @param seq_len Sequence length
 */
void cllm_compute_attention_fourier(const float* attention_weights,
                                    float* fourier_output,
                                    int seq_len);

/**
 * Apply Fourier-based dampening to attention
 * 
 * Dampens high-frequency components in attention for stability.
 * Uses low-pass filter based on Schumann resonance.
 * 
 * @param attention_weights Attention weights [seq_len]
 * @param seq_len Sequence length
 * @param cutoff_freq Cutoff frequency (default: 7.83 Hz)
 */
void cllm_apply_fourier_dampening(float* attention_weights,
                                  int seq_len,
                                  double cutoff_freq);

/**
 * Compute hyperdimensional distance
 * 
 * Distance in crystalline lattice space incorporating:
 * - Euclidean distance
 * - Prime factorization
 * - Lattice symmetries
 * - Fourier phase
 * 
 * @param coords1 First point [3]
 * @param coords2 Second point [3]
 * @param prime1 First point's prime
 * @param prime2 Second point's prime
 * @return Hyperdimensional distance
 */
double cllm_compute_hyperdimensional_distance(const double* coords1,
                                             const double* coords2,
                                             uint64_t prime1,
                                             uint64_t prime2);

/**
 * Root word modeling: Extract linguistic root from token
 * 
 * Uses prime factorization to identify root words:
 * - Prime tokens are roots
 * - Composite tokens are variations (tense, plurality, etc.)
 * - Factorization reveals morphological structure
 * 
 * @param token_id Token ID
 * @param prime Token's prime number
 * @return Root token ID (prime factor)
 */
uint32_t cllm_extract_root_word(uint32_t token_id, uint64_t prime);

/**
 * Compute morphological relationship between tokens
 * 
 * Determines relationship based on prime factorization:
 * - Same prime: Same root
 * - One divides other: Derivation
 * - Coprime: Unrelated
 * - Share factors: Related concepts
 * 
 * @param token1_prime First token's prime
 * @param token2_prime Second token's prime
 * @return Relationship type (0=unrelated, 1=related, 2=derived, 3=same)
 */
int cllm_compute_morphological_relationship(uint64_t token1_prime,
                                           uint64_t token2_prime);

// ============================================================================
// TRAINING MODE FUNCTIONS (Week 7 Integration)
// ============================================================================

/**
 * Enable training mode and allocate caches for backward pass
 * 
 * Allocates memory to store intermediate activations (Q, K, V, attention weights)
 * needed for computing gradients during backward pass.
 * 
 * @param model CLLM model
 * @param max_batch_size Maximum batch size to support
 * @param max_seq_len Maximum sequence length to support
 * @return 0 on success, -1 on failure
 */
int cllm_enable_training_mode(CLLMModel* model, uint32_t max_batch_size, uint32_t max_seq_len);

/**
 * Disable training mode and free caches
 * 
 * Frees all memory allocated for training mode.
 * 
 * @param model CLLM model
 */
void cllm_disable_training_mode(CLLMModel* model);

/**
 * Check if model is in training mode
 * 
 * @param model CLLM model
 * @return true if in training mode, false otherwise
 */
bool cllm_is_training(const CLLMModel* model);

/**
 * Attention forward pass (with optional caching for training)
 * 
 * Performs multi-head attention with automatic NTT switching.
 * If in training mode, caches intermediate values for backward pass.
 * 
 * @param model CLLM model
 * @param layer_idx Layer index
 * @param input Input sequence [batch_size × seq_len × embedding_dim]
 * @param output Output sequence [batch_size × seq_len × embedding_dim]
 * @param batch_size Batch size
 * @param seq_len Sequence length
 */
void cllm_attention_forward(
    CLLMModel* model,
    uint32_t layer_idx,
    const double* input,
    double* output,
    uint32_t batch_size,
    uint32_t seq_len
);

/**
 * Attention backward pass
 * 
 * Computes gradients for attention layer using cached intermediate values.
 * Must be called in training mode after forward pass.
 * 
 * @param model CLLM model
 * @param layer_idx Layer index
 * @param grad_output Gradient from next layer [batch_size × seq_len × embedding_dim]
 * @param input Original input [batch_size × seq_len × embedding_dim]
 * @param batch_size Batch size
 * @param seq_len Sequence length
 */
void cllm_attention_backward(
    CLLMModel* model,
    uint32_t layer_idx,
    const double* grad_output,
    const double* input,
    uint32_t batch_size,
    uint32_t seq_len
);

/**
 * Print attention statistics
 * 
 * @param model CLLM model
 */
void cllm_attention_print_stats(const CLLMModel* model);

#ifdef __cplusplus
}
#endif

#endif // CLLM_CRYSTALLINE_ATTENTION_H