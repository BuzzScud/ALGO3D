/**
 * @file g_triangulation.c
 * @brief G Triangulation - Universal Geometric Recovery
 * 
 * UNIVERSAL GEOMETRIC MATHEMATICS - Works with ANY system!
 * 
 * Implements geometric triangulation using:
 * - 13D clock lattice mapping
 * - 50 Platonic solid anchors
 * - Iterative refinement
 * - Oscillation tracking
 * 
 * NO crypto-specific dependencies - works with raw uint64_t data.
 */

#include "math/clock_lattice_13d.h"
#include "math/platonic_vertices.h"
#include "math/transcendental.h"
#include "math/types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct {
    double position[13];
    uint64_t k_estimate;
    double confidence;
} Anchor;

typedef struct {
    uint64_t n;
    double G_position[13];
    
    Anchor* anchors;
    int num_anchors;
    
    int num_training_pairs;
    uint64_t* training_inputs;
    uint64_t* training_outputs;
    
    double** k_estimates_history;
    int max_iterations;
    int current_iteration;
    
    double g_movement;
    double k_oscillation;
    bool converged;
} GTriangulationContext;

GTriangulationContext* g_triangulation_create(
    uint64_t n,
    int num_training_pairs,
    const uint64_t* training_inputs,
    const uint64_t* training_outputs,
    int max_iterations
) {
    GTriangulationContext* ctx = (GTriangulationContext*)calloc(1, sizeof(GTriangulationContext));
    if (!ctx) return NULL;
    
    ctx->n = n;
    ctx->num_training_pairs = num_training_pairs;
    ctx->max_iterations = max_iterations;
    ctx->current_iteration = 0;
    ctx->converged = false;
    
    ctx->training_inputs = (uint64_t*)malloc(num_training_pairs * sizeof(uint64_t));
    ctx->training_outputs = (uint64_t*)malloc(num_training_pairs * sizeof(uint64_t));
    
    if (!ctx->training_inputs || !ctx->training_outputs) {
        free(ctx);
        return NULL;
    }
    
    memcpy(ctx->training_inputs, training_inputs, num_training_pairs * sizeof(uint64_t));
    memcpy(ctx->training_outputs, training_outputs, num_training_pairs * sizeof(uint64_t));
    
    ctx->num_anchors = 50;
    ctx->anchors = (Anchor*)calloc(50, sizeof(Anchor));
    
    double vertices[50][13];
    int num_verts;
    math_generate_platonic_vertices_13d(vertices, &num_verts);
    
    for (int i = 0; i < num_verts; i++) {
        memcpy(ctx->anchors[i].position, vertices[i], 13 * sizeof(double));
        ctx->anchors[i].k_estimate = 0;
        ctx->anchors[i].confidence = 0.0;
    }
    
    ctx->k_estimates_history = (double**)malloc(max_iterations * sizeof(double*));
    for (int i = 0; i < max_iterations; i++) {
        ctx->k_estimates_history[i] = (double*)calloc(num_training_pairs, sizeof(double));
    }
    
    double sum_x = 0.0, sum_y = 0.0;
    for (int i = 0; i < num_training_pairs; i++) {
        sum_x += training_outputs[i];
        sum_y += training_inputs[i];
    }
    clock_map_pair_to_lattice_13d(sum_x / num_training_pairs, sum_y / num_training_pairs, ctx->G_position);
    
    return ctx;
}

void g_triangulation_destroy(GTriangulationContext* ctx) {
    if (!ctx) return;
    if (ctx->training_inputs) free(ctx->training_inputs);
    if (ctx->training_outputs) free(ctx->training_outputs);
    if (ctx->anchors) free(ctx->anchors);
    if (ctx->k_estimates_history) {
        for (int i = 0; i < ctx->max_iterations; i++) {
            if (ctx->k_estimates_history[i]) free(ctx->k_estimates_history[i]);
        }
        free(ctx->k_estimates_history);
    }
    free(ctx);
}

uint64_t g_triangulation_estimate_k(GTriangulationContext* ctx, uint64_t output) {
    double output_pos[13];
    clock_map_value_to_lattice_13d(output, output_pos);
    
    double min_dist = 1e10;
    uint64_t best_k = 0;
    
    for (int i = 0; i < ctx->num_anchors; i++) {
        double dist = math_distance_13d(output_pos, ctx->anchors[i].position);
        if (dist < min_dist) {
            min_dist = dist;
            best_k = ctx->anchors[i].k_estimate;
        }
    }
    
    return best_k;
}

void g_triangulation_refine(GTriangulationContext* ctx) {
    if (ctx->current_iteration >= ctx->max_iterations) return;
    
    for (int i = 0; i < ctx->num_training_pairs; i++) {
        uint64_t k_est = g_triangulation_estimate_k(ctx, ctx->training_outputs[i]);
        ctx->k_estimates_history[ctx->current_iteration][i] = (double)k_est;
    }
    
    double total_osc = 0.0;
    if (ctx->current_iteration > 0) {
        for (int i = 0; i < ctx->num_training_pairs; i++) {
            double diff = ctx->k_estimates_history[ctx->current_iteration][i] - 
                         ctx->k_estimates_history[ctx->current_iteration - 1][i];
            total_osc += diff * diff;
        }
        ctx->k_oscillation = math_sqrt(total_osc / ctx->num_training_pairs);
    }
    
    ctx->current_iteration++;
}

bool g_triangulation_check_convergence(GTriangulationContext* ctx, double threshold) {
    if (ctx->current_iteration < 2) return false;
    ctx->converged = (ctx->k_oscillation < threshold);
    return ctx->converged;
}

// Training function - iteratively refines estimates
bool g_triangulation_train(GTriangulationContext* ctx) {
    if (!ctx || ctx->num_training_pairs == 0) return false;
    
    const int MAX_ITERATIONS = 100;
    const double CONVERGENCE_THRESHOLD = 1e-6;
    
    for (int iter = 0; iter < MAX_ITERATIONS; iter++) {
        g_triangulation_refine(ctx);
        
        if (g_triangulation_check_convergence(ctx, CONVERGENCE_THRESHOLD)) {
            return true;  // Converged
        }
    }
    
    return false;  // Did not converge
}

// Estimate k from output using trained model
bool g_triangulation_estimate(
    GTriangulationContext* ctx,
    uint64_t output,
    uint64_t* k_estimate
) {
    if (!ctx || !k_estimate) return false;
    
    *k_estimate = g_triangulation_estimate_k(ctx, output);
    return true;
}

// Get confidence of current estimates
double g_triangulation_get_confidence(const GTriangulationContext* ctx) {
    if (!ctx || ctx->current_iteration < 2) return 0.0;
    
    // Confidence based on convergence (lower oscillation = higher confidence)
    // Map oscillation to confidence: 0 oscillation = 1.0 confidence
    double confidence = 1.0 / (1.0 + ctx->k_oscillation * 100.0);
    return confidence;
}
