#ifndef FULL_PIPELINE_H
#define FULL_PIPELINE_H

#include <stdint.h>
#include <stdbool.h>
#include <openssl/ec.h>
#include "g_triangulation.h"
#include "multi_torus_tracker.h"
#include "micro_model.h"

/**
 * Full Pipeline Context
 * 
 * Manages the complete geometric recovery pipeline integrating
 * all phases from G triangulation through final recovery.
 */
typedef struct {
    // Curve parameters
    int curve_nid;
    uint64_t n;
    
    // Training data
    int num_training_samples;
    uint64_t* training_k;
    EC_POINT** training_Q;
    
    // Phase components
    GTriangulationContext* g_triangulation;
    MultiTorusTracker* torus_tracker;
    MicroModel* micro_model;
    
    // Extracted factors
    uint64_t p;
    uint64_t q;
    int p_index;
    int q_index;
    
    // Status flags
    bool g_refined;
    bool pipeline_complete;
} FullPipelineContext;

/**
 * Pipeline Statistics
 */
typedef struct {
    bool pipeline_complete;
    bool g_refined;
    uint64_t p;
    uint64_t q;
    int num_tori_tracked;
    int num_training_samples;
    int g_iterations;
    bool g_converged;
    bool micro_model_trained;
    double training_error;
} PipelineStatistics;

/**
 * Create full pipeline context
 */
FullPipelineContext* create_full_pipeline_context(
    int curve_nid,
    uint64_t n,
    int num_training_samples
);

/**
 * Free full pipeline context
 */
void free_full_pipeline_context(FullPipelineContext* ctx);

/**
 * Execute the complete geometric recovery pipeline
 */
int geometric_recovery_full_pipeline(
    FullPipelineContext* ctx,
    const EC_POINT* target_Q,
    uint64_t* k_min_out,
    uint64_t* k_max_out,
    double* reduction_factor_out
);

/**
 * Run pipeline with training data
 */
int geometric_recovery_pipeline_with_training(
    FullPipelineContext* ctx,
    const uint64_t* training_k,
    const EC_POINT** training_Q,
    int num_training,
    const EC_POINT* target_Q,
    uint64_t* k_min_out,
    uint64_t* k_max_out,
    double* reduction_factor_out
);

/**
 * Get pipeline statistics
 */
void get_pipeline_statistics(
    const FullPipelineContext* ctx,
    PipelineStatistics* stats
);

/**
 * Print pipeline statistics
 */
void print_pipeline_statistics(const PipelineStatistics* stats);

#endif // FULL_PIPELINE_H
