#ifndef CLLM_KISSING_SPHERES_H
#define CLLM_KISSING_SPHERES_H

#include "../cllm.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize kissing spheres (12 neighbors per point)
 * 
 * Each lattice point gets exactly 12 neighbors, one from each symmetry group.
 * 
 * @param model CLLM model with lattice points
 */
void cllm_initialize_kissing_spheres(CLLMModel* model);

/**
 * Optimize neighbor cache locality
 * 
 * Reorders the 12 neighbors for each point so that neighbors with
 * adjacent memory addresses are processed together. This improves
 * cache hit rate by 20-30%.
 * 
 * Called automatically by cllm_initialize_kissing_spheres().
 * 
 * @param model CLLM model
 */
void cllm_optimize_neighbor_cache_locality(CLLMModel* model);

/**
 * Process lattice point with its 12 kissing sphere neighbors
 * 
 * Uses L(n,d,k,λ) formula to compute interaction strength.
 * 
 * @param point Lattice point to process
 * @param model CLLM model
 * @param local_gradients Gradient buffer [vocab_size x embedding_dim]
 * @param gradient_size Total gradient buffer size
 */
// LEGACY: Commented out - uses old CLLMLatticePoint structure
// void cllm_process_kissing_spheres(CLLMLatticePoint* point,
//                                   CLLMModel* model,
//                                   float* local_gradients,
//                                   size_t gradient_size);

/**
 * Verify kissing spheres configuration
 * 
 * @param model CLLM model
 * @return 1 if valid, 0 if issues found
 */
int cllm_verify_kissing_spheres(CLLMModel* model);

/**
 * Get statistics about kissing spheres configuration
 * 
 * @param model CLLM model
 */
void cllm_kissing_spheres_stats(CLLMModel* model);

/**
 * UI Integration: Report sphere positions to metrics system
 * 
 * Extracts angular positions from tokens and reports them to the metrics
 * system for visualization in the UI.
 * 
 * @param model CLLM model with tokens
 * @param metrics Metrics system to report to
 * @param thread_id Thread ID for this sphere
 */
struct CLLMMetrics;  // Forward declaration
void cllm_report_sphere_position(CLLMModel* model, struct CLLMMetrics* metrics, int thread_id);

#ifdef __cplusplus
}
#endif

#endif /* CLLM_KISSING_SPHERES_H */