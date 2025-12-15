/**
 * @file g_triangulation_abstracted.h
 * @brief G Triangulation - Abstracted for Universal Use
 * 
 * UNIVERSAL GEOMETRIC MATHEMATICS - Works with ANY system!
 * 
 * This module implements geometric triangulation using:
 * - 13D clock lattice mapping
 * - 50 Platonic solid anchors
 * - Iterative refinement
 * - Oscillation tracking
 * 
 * NO crypto-specific dependencies - works with raw uint64_t data.
 * Can be used for:
 * - ECDLP: (k, Q) where Q = k·G
 * - RSA: (d, e) where e·d ≡ 1 (mod φ(n))
 * - Embeddings: (index, embedding_value)
 * - Any geometric mapping
 */

#ifndef G_TRIANGULATION_ABSTRACTED_H
#define G_TRIANGULATION_ABSTRACTED_H

#include <stdint.h>
#include <stdbool.h>

// Forward declaration
typedef struct GTriangulationContext GTriangulationContext;

/**
 * Create a new G triangulation context
 * 
 * @param n System size (e.g., curve order for ECDLP, modulus for RSA)
 * @param num_samples Number of training samples
 * @param inputs Array of input values (e.g., k values)
 * @param outputs Array of output values (e.g., Q values)
 * @param max_iterations Maximum iterations for refinement
 * @return New context or NULL on error
 */
GTriangulationContext* g_triangulation_create(
    uint64_t n,
    uint32_t num_samples,
    const uint64_t* inputs,
    const uint64_t* outputs,
    uint32_t max_iterations
);

/**
 * Train the triangulation system
 * 
 * Performs iterative refinement to find optimal G position
 * and anchor positions in 13D clock lattice.
 * 
 * @param ctx Context
 * @return true if training converged, false otherwise
 */
bool g_triangulation_train(GTriangulationContext* ctx);

/**
 * Estimate input value from output value
 * 
 * @param ctx Context
 * @param output Output value to recover input for
 * @param estimate_out Pointer to store estimated input
 * @return true on success, false on error
 */
bool g_triangulation_estimate(
    GTriangulationContext* ctx,
    uint64_t output,
    uint64_t* estimate_out
);

/**
 * Get confidence score for current triangulation
 * 
 * @param ctx Context
 * @return Confidence score [0.0, 1.0]
 */
double g_triangulation_get_confidence(const GTriangulationContext* ctx);

/**
 * Get G position in 13D clock lattice
 * 
 * @param ctx Context
 * @param position_out Array of 13 doubles to store position
 */
void g_triangulation_get_g_position(
    const GTriangulationContext* ctx,
    double position_out[13]
);

/**
 * Get number of anchors
 * 
 * @param ctx Context
 * @return Number of anchors
 */
int g_triangulation_get_num_anchors(const GTriangulationContext* ctx);

/**
 * Check if training has converged
 * 
 * @param ctx Context
 * @return true if converged, false otherwise
 */
bool g_triangulation_has_converged(const GTriangulationContext* ctx);

/**
 * Get current iteration number
 * 
 * @param ctx Context
 * @return Current iteration
 */
int g_triangulation_get_iteration(const GTriangulationContext* ctx);

/**
 * Destroy triangulation context
 * 
 * @param ctx Context to destroy
 */
void g_triangulation_destroy(GTriangulationContext* ctx);

#endif // G_TRIANGULATION_ABSTRACTED_H