/**
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * @file convergence_detection.c
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * @brief Convergence detection for iterative refinement
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * 
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * Detects when iterative refinement has converged based on
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * oscillation stability, structural consistency, and error metrics.
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 */
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"

#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
#include "blind_recovery.h"
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
#include <stdlib.h>

/**
 * Check if oscillations have converged
 */
bool is_converged(
    const OscillationMap* oscillations,
    double threshold
) {
    if (!oscillations) {
        return false;
    }
    
    // Check if already marked as converging
    if (!oscillations->is_converging) {
        return false;
    }
    
    // Check all dimensions for stability
    uint32_t stable_count = 0;
    
    for (uint32_t i = 0; i < oscillations->num_dimensions; i++) {
        const OscillationSignature* sig = &oscillations->signatures[i];
        
        // Check if oscillation is stable
        if (sig->is_stable) {
            stable_count++;
        }
        
        // Check convergence rate
        if (sig->convergence_rate < threshold) {
            return false;  // Still converging too slowly
        }
    }
    
    // Require at least 80% of dimensions to be stable
    double stability_ratio = (double)stable_count / oscillations->num_dimensions;
    if (stability_ratio < 0.8) {
        return false;
    }
    
    return true;
}

/**
 * Check if structure has converged
 */
bool is_structure_converged(
    const StructuralMap* structure,
    double threshold
) {
    if (!structure) {
        return false;
    }
    
    // Check Euler's formula
    int euler_result = (int)structure->num_vertices - 
                      (int)structure->num_edges + 
                      (int)structure->num_faces;
    
    if (euler_result != 2) {
        return false;  // Not a valid polyhedron
    }
    
    // Check corruption percentage
    if (structure->corruption_percentage > threshold) {
        return false;  // Still too much corruption
    }
    
    return true;
}

/**
 * Compute convergence metrics
 */
typedef struct {
    double oscillation_stability;
    double structural_consistency;
    double corruption_level;
    double overall_convergence;
    bool is_converged;
} ConvergenceMetrics;

ConvergenceMetrics compute_convergence_metrics(
    const OscillationMap* oscillations,
    const StructuralMap* structure,
    double threshold
) {
    ConvergenceMetrics metrics = {0};
    
    // Oscillation stability
    if (oscillations) {
        uint32_t stable_count = 0;
        double avg_convergence_rate = 0.0;
        
        for (uint32_t i = 0; i < oscillations->num_dimensions; i++) {
            if (oscillations->signatures[i].is_stable) {
                stable_count++;
            }
            avg_convergence_rate += oscillations->signatures[i].convergence_rate;
        }
        
        metrics.oscillation_stability = (double)stable_count / 
                                        oscillations->num_dimensions;
        avg_convergence_rate /= oscillations->num_dimensions;
        
        // Normalize convergence rate (0 to 1)
        metrics.oscillation_stability = (metrics.oscillation_stability + 
                                         avg_convergence_rate) / 2.0;
    }
    
    // Structural consistency
    if (structure) {
        int euler_result = (int)structure->num_vertices - 
                          (int)structure->num_edges + 
                          (int)structure->num_faces;
        int euler_delta = abs(euler_result - 2);
        
        // Perfect = 1.0, decreases with delta
        metrics.structural_consistency = 1.0 / (1.0 + euler_delta);
        
        // Corruption level (inverted - lower is better)
        metrics.corruption_level = 1.0 - structure->corruption_percentage;
    }
    
    // Overall convergence (weighted average)
    metrics.overall_convergence = 
        0.3 * metrics.oscillation_stability +
        0.4 * metrics.structural_consistency +
        0.3 * metrics.corruption_level;
    
    // Check if converged
    metrics.is_converged = (metrics.overall_convergence >= threshold);
    
    return metrics;
}

/**
 * Check convergence with history
 */
struct ConvergenceHistory {
    double* history;
    uint32_t history_size;
    uint32_t current_index;
    bool is_full;
};

ConvergenceHistory* create_convergence_history(uint32_t size) {
    ConvergenceHistory* history = (ConvergenceHistory*)calloc(
        1, sizeof(ConvergenceHistory)
    );
    
    if (!history) {
        return NULL;
    }
    
    history->history = (double*)calloc(size, sizeof(double));
    if (!history->history) {
        free(history);
        return NULL;
    }
    
    history->history_size = size;
    history->current_index = 0;
    history->is_full = false;
    
    return history;
}

void add_to_convergence_history(
    ConvergenceHistory* history,
    double value
) {
    if (!history) return;
    
    history->history[history->current_index] = value;
    history->current_index = (history->current_index + 1) % history->history_size;
    
    if (history->current_index == 0) {
        history->is_full = true;
    }
}

bool is_converged_with_history(
    const ConvergenceHistory* history,
    double threshold,
    double variance_threshold
) {
    if (!history || !history->is_full) {
        return false;
    }
    
    // Compute mean and variance
    double mean = 0.0;
    for (uint32_t i = 0; i < history->history_size; i++) {
        mean += history->history[i];
    }
    mean /= history->history_size;
    
    double variance = 0.0;
    for (uint32_t i = 0; i < history->history_size; i++) {
        double diff = history->history[i] - mean;
        variance += diff * diff;
    }
    variance /= history->history_size;
    
    // Check if mean is above threshold and variance is low
    if (mean >= threshold && variance < variance_threshold) {
        return true;
    }
    
    return false;
}

void free_convergence_history(ConvergenceHistory* history) {
    if (!history) return;
    
    free(history->history);
    free(history);
}

/**
 * Detect early convergence
 * 
 * Returns true if convergence is detected early (before max iterations)
 */
bool detect_early_convergence(
    const OscillationMap* oscillations,
    const StructuralMap* structure,
    ConvergenceHistory* history,
    double threshold
) {
    // Compute current metrics
    ConvergenceMetrics metrics = compute_convergence_metrics(
        oscillations,
        structure,
        threshold
    );
    
    // Add to history
    if (history) {
        add_to_convergence_history(history, metrics.overall_convergence);
        
        // Check history-based convergence
        if (is_converged_with_history(history, threshold, 0.01)) {
            return true;
        }
    }
    
    // Check immediate convergence
    return metrics.is_converged;
}
