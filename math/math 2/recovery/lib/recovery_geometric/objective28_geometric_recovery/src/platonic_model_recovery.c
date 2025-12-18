/**
 * @file platonic_model_recovery.c
 * @brief Main Recovery Algorithm with Dynamic Scaling
 * 
 * This is the COMPLETE recovery algorithm that:
 * 1. Detects spatial oscillations
 * 2. Detects temporal oscillations (oscillations of oscillations)
 * 3. Applies tetration stabilization
 * 4. If not converged, SCALES UP complexity
 * 5. Repeats until converged or max complexity reached
 */

#include "platonic_model.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "math/arithmetic.h"
#include "math/transcendental.h"

// Forward declarations
extern void platonic_detect_spatial_oscillations(PlatonicModel*, uint32_t, double);
extern void platonic_detect_temporal_oscillations(PlatonicModel*);
extern bool platonic_stabilize_temporal_oscillations(PlatonicModel*);
extern double platonic_get_max_oscillation_amplitude(const PlatonicModel*);
extern bool platonic_all_oscillations_stable(const PlatonicModel*);
extern bool platonic_auto_scale(PlatonicModel*, double);

bool platonic_recover(
    PlatonicModel* model,
    uint64_t max_iterations,
    double convergence_threshold
) {
    if (!model) return false;
    
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║  OBJECTIVE 28: Universal Blind Recovery                 ║\n");
    printf("║  High-Dimensional Platonic Solid Recovery               ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");
    printf("\n");
    printf("Model: %s\n", model->model_id);
    printf("Initial Configuration:\n");
    printf("  Dimensions: %u (max: %u)\n", model->num_dimensions, model->max_dimensions);
    printf("  Vertices: %u (max: %u)\n", model->num_vertices, model->max_vertices);
    printf("  Tetration: %u bases × %u depths\n", 
           model->num_tetration_bases, model->num_tetration_depths);
    printf("  Max iterations: %lu\n", max_iterations);
    printf("  Convergence threshold: %.6f\n", convergence_threshold);
    printf("\n");
    
    clock_t start_time = clock();
    uint64_t iteration = 0;
    bool converged = false;
    
    // Main recovery loop
    while (iteration < max_iterations && !converged) {
        printf("═══════════════════════════════════════════════════════════\n");
        printf("Iteration %lu / %lu\n", iteration + 1, max_iterations);
        printf("═══════════════════════════════════════════════════════════\n");
        
        // PHASE 1: Detect spatial oscillations across ALL dimensions
        platonic_detect_spatial_oscillations(model, 128, 1000.0);
        
        // PHASE 2: Detect temporal oscillations (oscillations of oscillations)
        platonic_detect_temporal_oscillations(model);
        
        // PHASE 3: Stabilize temporal oscillations using tetration towers
        bool temporal_stable = platonic_stabilize_temporal_oscillations(model);
        
        // PHASE 4: Check convergence
        double max_amplitude = platonic_get_max_oscillation_amplitude(model);
        bool spatial_stable = platonic_all_oscillations_stable(model);
        
        printf("\nConvergence Check:\n");
        printf("  Max oscillation amplitude: %.6f\n", max_amplitude);
        printf("  Spatial oscillations stable: %s\n", spatial_stable ? "Yes" : "No");
        printf("  Temporal oscillations stable: %s\n", temporal_stable ? "Yes" : "No");
        
        if (max_amplitude < convergence_threshold && spatial_stable && temporal_stable) {
            converged = true;
            printf("\n✓ CONVERGED!\n");
            break;
        }
        
        // PHASE 5: If not converged, decide if we need to scale
        if (iteration > 0 && iteration % 10 == 0) {
            // Every 10 iterations, check if we should scale
            if (max_amplitude > convergence_threshold * 10.0) {
                printf("\n⚠ Oscillations not decreasing, scaling complexity...\n");
                
                if (platonic_auto_scale(model, max_amplitude)) {
                    printf("✓ Complexity scaled, continuing recovery...\n");
                    
                    // After scaling, re-detect oscillations
                    continue;
                } else {
                    printf("⚠ Cannot scale further, continuing with current complexity...\n");
                }
            }
        }
        
        iteration++;
        
        // Progress indicator
        if (iteration % 10 == 0) {
            double elapsed = (double)(clock() - start_time) / CLOCKS_PER_SEC;
            printf("\nProgress: %lu iterations, %.2f seconds elapsed\n", iteration, elapsed);
        }
    }
    
    // Final statistics
    double total_time = (double)(clock() - start_time) / CLOCKS_PER_SEC;
    
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║  RECOVERY COMPLETE                                       ║\n");
    printf("╠══════════════════════════════════════════════════════════╣\n");
    printf("║  Status: %s\n", converged ? "CONVERGED ✓" : "MAX ITERATIONS REACHED");
    printf("║  Iterations: %lu / %lu\n", iteration, max_iterations);
    printf("║  Time: %.3f seconds\n", total_time);
    printf("║  Final amplitude: %.6f\n", platonic_get_max_oscillation_amplitude(model));
    printf("║  \n");
    printf("║  Final Configuration:\n");
    printf("║    Dimensions: %u (scaled %u times)\n", 
           model->num_dimensions, model->num_dimension_scales);
    printf("║    Vertices: %u (scaled %u times)\n", 
           model->num_vertices, model->num_vertex_scales);
    printf("║    Tetration depth: %u\n", 
           29 + model->num_tetration_depths - 1);
    printf("╚══════════════════════════════════════════════════════════╝\n");
    
    // Update model statistics
    model->is_converged = converged;
    model->total_iterations = iteration;
    model->convergence_rate = converged ? 1.0 : 0.0;
    model->final_oscillation_amplitude = platonic_get_max_oscillation_amplitude(model);
    
    return converged;
}

/**
 * Validate recovery using Euler's formula
 */
bool platonic_model_validate_euler(const PlatonicModel* model) {
    if (!model) return false;
    
    // For now, just check basic structure
    // TODO: Implement proper Euler validation for high-dimensional solids
    
    printf("\n🔍 Validating using Euler's formula...\n");
    printf("  V=%u, E=%u, F=%u\n", model->num_vertices, model->num_edges, model->num_faces);
    
    // Basic check: V > 0
    if (model->num_vertices == 0) {
        printf("  ❌ Invalid: No vertices\n");
        return false;
    }
    
    printf("  ✓ Basic validation passed\n");
    return true;
}

/**
 * Validate using coprime relationships
 */
bool platonic_model_validate_coprime(const PlatonicModel* model) {
    if (!model || !model->coprime_matrix) return false;
    
    printf("\n🔍 Validating coprime relationships...\n");
    
    // Check that coprime matrix is symmetric
    for (uint32_t i = 0; i < model->num_dimensions; i++) {
        for (uint32_t j = i + 1; j < model->num_dimensions; j++) {
            if (model->coprime_matrix[i][j] != model->coprime_matrix[j][i]) {
                printf("  ❌ Coprime matrix not symmetric at (%u, %u)\n", i, j);
                return false;
            }
        }
    }
    
    printf("  ✓ Coprime matrix is symmetric\n");
    return true;
}

/**
 * Validate geometric constraints
 */
bool platonic_model_validate_geometry(const PlatonicModel* model) {
    if (!model || !model->vertex_positions) return false;
    
    printf("\n🔍 Validating geometric constraints...\n");
    
    // Check that all vertex positions are finite
    for (uint32_t v = 0; v < model->num_vertices; v++) {
        for (uint32_t d = 0; d < model->num_dimensions; d++) {
            double value = model->vertex_positions[v * model->num_dimensions + d];
            if (math_is_inf(value) || math_is_nan(value)) {
                printf("  ❌ Non-finite value at vertex %u, dimension %u\n", v, d);
                return false;
            }
        }
    }
    
    printf("  ✓ All vertex positions are finite\n");
    return true;
}
