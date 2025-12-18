/**
 * @file g_triangulation.h
 * @brief G Triangulation - Abstracted for Universal Use
 * 
 * UNIVERSAL GEOMETRIC MATHEMATICS - Works with ANY system!
 * 
 * NO crypto-specific dependencies - works with raw uint64_t data.
 */

#ifndef ALGORITHMS_G_TRIANGULATION_H
#define ALGORITHMS_G_TRIANGULATION_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct GTriangulationContext GTriangulationContext;

/**
 * Create G triangulation context
 * 
 * @param n System size
 * @param num_training_pairs Number of training samples
 * @param training_inputs Input values (k, index, etc.)
 * @param training_outputs Output values (Q, embedding, etc.)
 * @param max_iterations Maximum refinement iterations
 * @return Context or NULL on error
 */
GTriangulationContext* g_triangulation_create(
    uint64_t n,
    int num_training_pairs,
    const uint64_t* training_inputs,
    const uint64_t* training_outputs,
    int max_iterations
);

/**
 * Free G triangulation context
 */
void g_triangulation_free(GTriangulationContext* ctx);

/**
 * Estimate input from output using geometric triangulation
 */
uint64_t g_triangulation_estimate_k(GTriangulationContext* ctx, uint64_t output);

/**
 * Perform one refinement iteration
 */
void g_triangulation_refine(GTriangulationContext* ctx);

/**
 * Check for convergence
 */
bool g_triangulation_check_convergence(GTriangulationContext* ctx, double threshold);

#ifdef __cplusplus
}
#endif

#endif /* ALGORITHMS_G_TRIANGULATION_H */