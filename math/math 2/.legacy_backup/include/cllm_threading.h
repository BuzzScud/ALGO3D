#ifndef CLLM_THREADING_H
#define CLLM_THREADING_H

/**
 * CLLM Threading Interface
 * 
 * CLLM-specific wrapper around the generic sphere threading model
 * from the algorithms layer. This provides CLLM-specific functionality
 * while using the generic threading infrastructure.
 * 
 * The generic sphere threading model (algorithms/sphere_threading.h) handles:
 * - Neighbor assignment using clock geometry
 * - Cache optimization
 * - Work distribution
 * - Statistics and validation
 * 
 * This wrapper adds CLLM-specific functionality:
 * - Integration with CLLMModel structure
 * - Lattice point processing
 * - Gradient accumulation
 * - UI metrics reporting
 */

#include "cllm.h"
#include "cllm_metrics.h"
#include "../algorithms/include/sphere_threading.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// CLLM THREADING MODEL
// ============================================================================

/**
 * CLLM Threading Context
 * 
 * Wraps the generic sphere threading model with CLLM-specific context.
 */
typedef struct {
    SphereThreadingModel* base_model;  // Generic threading model
    CLLMModel* cllm_model;             // CLLM model
    
    // CLLM-specific state
    float* gradient_buffer;            // Shared gradient buffer
    size_t gradient_size;              // Size of gradient buffer
    
} CLLMThreading;

// ============================================================================
// INITIALIZATION
// ============================================================================

/**
 * Initialize CLLM threading
 * 
 * Creates a threading model for the CLLM model using the generic
 * sphere threading infrastructure from the algorithms layer.
 * 
 * This function:
 * 1. Creates a generic sphere threading model
 * 2. Initializes neighbors using clock geometry
 * 3. Optimizes cache locality
 * 4. Sets up CLLM-specific context
 * 
 * @param model CLLM model
 * @return CLLM threading context, or NULL on error
 */
CLLMThreading* cllm_threading_init(CLLMModel* model);

/**
 * Free CLLM threading
 * 
 * @param threading Threading context to free
 */
void cllm_threading_free(CLLMThreading* threading);

// ============================================================================
// LATTICE POINT OPERATIONS
// ============================================================================

/**
 * Initialize lattice points with sphere threading
 * 
 * Sets up the lattice points in the CLLM model to use sphere threading.
 * This assigns 12 neighbors to each lattice point using clock geometry.
 * 
 * @param model CLLM model
 * @return 0 on success, -1 on error
 */
int cllm_threading_init_lattice_points(CLLMModel* model);

/**
 * Optimize cache locality for lattice points
 * 
 * Reorders neighbors for optimal cache performance.
 * 
 * @param model CLLM model
 * @return 0 on success, -1 on error
 */
int cllm_threading_optimize_cache(CLLMModel* model);

// ============================================================================
// PROCESSING
// ============================================================================

/**
 * Process lattice point with neighbors
 * 
 * Processes a lattice point using its 12 neighbors.
 * Uses L(n,d,k,λ) formula for interaction strength.
 * 
 * @param point Lattice point to process
 * @param model CLLM model
 * @param local_gradients Gradient buffer
 * @param gradient_size Size of gradient buffer
 */
void cllm_threading_process_point(
    CLLMLatticePoint* point,
    CLLMModel* model,
    float* local_gradients,
    size_t gradient_size
);

// ============================================================================
// VALIDATION & STATISTICS
// ============================================================================

/**
 * Verify threading configuration
 * 
 * @param model CLLM model
 * @return 1 if valid, 0 if issues found
 */
int cllm_threading_verify(CLLMModel* model);

/**
 * Get threading statistics
 * 
 * @param model CLLM model
 */
void cllm_threading_stats(CLLMModel* model);

// ============================================================================
// UI INTEGRATION
// ============================================================================

/**
 * Report sphere position to metrics system
 * 
 * @param model CLLM model
 * @param metrics Metrics system
 * @param thread_id Thread ID
 */
void cllm_threading_report_position(
    CLLMModel* model,
    CLLMMetrics* metrics,
    int thread_id
);

// ============================================================================
// BACKWARD COMPATIBILITY
// ============================================================================

/**
 * Legacy function names for backward compatibility
 * These map to the new threading functions.
 */

static inline void cllm_initialize_kissing_spheres(CLLMModel* model) {
    cllm_threading_init_lattice_points(model);
}

static inline void cllm_optimize_neighbor_cache_locality(CLLMModel* model) {
    cllm_threading_optimize_cache(model);
}

static inline void cllm_process_kissing_spheres(
    CLLMLatticePoint* point,
    CLLMModel* model,
    float* local_gradients,
    size_t gradient_size
) {
    cllm_threading_process_point(point, model, local_gradients, gradient_size);
}

static inline int cllm_verify_kissing_spheres(CLLMModel* model) {
    return cllm_threading_verify(model);
}

static inline void cllm_kissing_spheres_stats(CLLMModel* model) {
    cllm_threading_stats(model);
}

static inline void cllm_report_sphere_position(
    CLLMModel* model,
    CLLMMetrics* metrics,
    int thread_id
) {
    cllm_threading_report_position(model, metrics, thread_id);
}

#ifdef __cplusplus
}
#endif

#endif /* CLLM_THREADING_H */