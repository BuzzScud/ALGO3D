/**
 * @file convergence_detection.c
 * @brief Universal Convergence Detection Implementation
 * 
 * UNIVERSAL GEOMETRIC MATHEMATICS - Works with ANY system!
 */

#include "geometric_recovery/convergence_detection.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ============================================================================
// INTERNAL STRUCTURES
// ============================================================================

struct ConvergenceDetector {
    // Configuration
    ConvergenceCriteria criteria;
    
    // State
    uint32_t current_iteration;
    double current_error;
    double previous_error;
    
    // History
    double* error_history;
    uint32_t history_size;
    uint32_t history_capacity;
    
    // Convergence tracking
    bool has_converged;
    uint32_t convergence_iteration;
    uint32_t iterations_stable;
    
    // Multi-metric tracking
    bool abs_converged;
    bool rel_converged;
    bool var_converged;
    bool grad_converged;
    bool stable_converged;
    
    // Statistics
    double min_error;
    double max_error;
    double avg_error;
    double convergence_rate;
};

// ============================================================================
// INTERNAL FUNCTIONS
// ============================================================================

static bool check_absolute_convergence(ConvergenceDetector* detector) {
    if (detector->current_iteration < detector->criteria.min_iterations) {
        return false;
    }
    
    double abs_change = math_abs(detector->current_error - detector->previous_error);
    return abs_change < detector->criteria.abs_threshold;
}

static bool check_relative_convergence(ConvergenceDetector* detector) {
    if (detector->current_iteration < detector->criteria.min_iterations) {
        return false;
    }
    
    if (math_abs(detector->previous_error) < 1e-10) {
        return false;  // Avoid division by zero
    }
    
    double rel_change = math_abs(
        (detector->current_error - detector->previous_error) / 
        detector->previous_error
    );
    
    return rel_change < detector->criteria.rel_threshold;
}

static bool check_variance_convergence(ConvergenceDetector* detector) {
    uint32_t window = detector->criteria.moving_avg_window;
    
    if (detector->history_size < window) {
        return false;
    }
    
    // Compute variance over window
    double sum = 0.0;
    double sum_sq = 0.0;
    
    for (uint32_t i = detector->history_size - window; 
         i < detector->history_size; i++) {
        double val = detector->error_history[i];
        sum += val;
        sum_sq += val * val;
    }
    
    double mean = sum / window;
    double variance = (sum_sq / window) - (mean * mean);
    
    return variance < detector->criteria.var_threshold;
}

static bool check_gradient_convergence(ConvergenceDetector* detector) {
    if (detector->history_size < 2) {
        return false;
    }
    
    // Compute gradient (derivative)
    double gradient = detector->current_error - detector->previous_error;
    
    return math_abs(gradient) < detector->criteria.grad_threshold;
}

static bool check_stability_convergence(ConvergenceDetector* detector) {
    uint32_t window = detector->criteria.stability_window;
    
    if (detector->iterations_stable >= window) {
        return true;
    }
    
    // Check if error is below threshold
    if (detector->current_error < detector->criteria.abs_threshold) {
        detector->iterations_stable++;
    } else {
        detector->iterations_stable = 0;
    }
    
    return detector->iterations_stable >= window;
}

// ============================================================================
// CORE API IMPLEMENTATION
// ============================================================================

ConvergenceDetector* convergence_detector_create(uint32_t max_iterations) {
    ConvergenceCriteria criteria = convergence_criteria_default();
    criteria.max_iterations = max_iterations;
    
    return convergence_detector_create_custom(&criteria);
}

ConvergenceDetector* convergence_detector_create_custom(
    const ConvergenceCriteria* criteria
) {
    if (!criteria) {
        return NULL;
    }
    
    ConvergenceDetector* detector = (ConvergenceDetector*)calloc(
        1, sizeof(ConvergenceDetector)
    );
    if (!detector) {
        return NULL;
    }
    
    // Copy criteria
    detector->criteria = *criteria;
    
    // Allocate history
    detector->history_capacity = criteria->max_iterations;
    detector->error_history = (double*)calloc(
        detector->history_capacity, sizeof(double)
    );
    if (!detector->error_history) {
        free(detector);
        return NULL;
    }
    
    // Initialize state
    detector->current_iteration = 0;
    detector->current_error = INFINITY;
    detector->previous_error = INFINITY;
    detector->history_size = 0;
    
    detector->has_converged = false;
    detector->convergence_iteration = 0;
    detector->iterations_stable = 0;
    
    detector->abs_converged = false;
    detector->rel_converged = false;
    detector->var_converged = false;
    detector->grad_converged = false;
    detector->stable_converged = false;
    
    detector->min_error = INFINITY;
    detector->max_error = -INFINITY;
    detector->avg_error = 0.0;
    detector->convergence_rate = 0.0;
    
    return detector;
}

bool convergence_detector_update(
    ConvergenceDetector* detector,
    double error
) {
    if (!detector) {
        return false;
    }
    
    // Update state
    detector->previous_error = detector->current_error;
    detector->current_error = error;
    detector->current_iteration++;
    
    // Add to history
    if (detector->history_size < detector->history_capacity) {
        detector->error_history[detector->history_size++] = error;
    }
    
    // Update statistics
    if (error < detector->min_error) detector->min_error = error;
    if (error > detector->max_error) detector->max_error = error;
    
    // Update average
    double sum = 0.0;
    for (uint32_t i = 0; i < detector->history_size; i++) {
        sum += detector->error_history[i];
    }
    detector->avg_error = sum / detector->history_size;
    
    // Check maximum iterations
    if (detector->current_iteration >= detector->criteria.max_iterations) {
        detector->has_converged = false;
        return false;
    }
    
    // Check minimum iterations
    if (detector->current_iteration < detector->criteria.min_iterations) {
        return false;
    }
    
    // Check all convergence criteria
    detector->abs_converged = check_absolute_convergence(detector);
    detector->rel_converged = check_relative_convergence(detector);
    detector->var_converged = check_variance_convergence(detector);
    detector->grad_converged = check_gradient_convergence(detector);
    detector->stable_converged = check_stability_convergence(detector);
    
    // Combined convergence decision
    bool converged = (
        (detector->abs_converged && detector->rel_converged) ||
        (detector->var_converged && detector->stable_converged)
    );
    
    if (converged && !detector->has_converged) {
        detector->has_converged = true;
        detector->convergence_iteration = detector->current_iteration;
        
        // Compute convergence rate
        if (detector->history_size > 0) {
            detector->convergence_rate = 
                (detector->error_history[0] - detector->current_error) / 
                detector->current_iteration;
        }
    }
    
    return detector->has_converged;
}

bool convergence_detector_check(const ConvergenceDetector* detector) {
    if (!detector) {
        return false;
    }
    return detector->has_converged;
}

ConvergenceResult convergence_detector_get_result(
    const ConvergenceDetector* detector
) {
    ConvergenceResult result = {0};
    
    if (!detector) {
        return result;
    }
    
    result.converged = detector->has_converged;
    result.iterations = detector->current_iteration;
    result.final_error = detector->current_error;
    result.convergence_rate = detector->convergence_rate;
    
    result.abs_criterion_met = detector->abs_converged;
    result.rel_criterion_met = detector->rel_converged;
    result.var_criterion_met = detector->var_converged;
    result.grad_criterion_met = detector->grad_converged;
    result.stable_criterion_met = detector->stable_converged;
    
    // Compute confidence
    int criteria_met = 0;
    if (detector->abs_converged) criteria_met++;
    if (detector->rel_converged) criteria_met++;
    if (detector->var_converged) criteria_met++;
    if (detector->grad_converged) criteria_met++;
    if (detector->stable_converged) criteria_met++;
    
    result.confidence = (double)criteria_met / 5.0;
    
    return result;
}

void convergence_detector_reset(ConvergenceDetector* detector) {
    if (!detector) {
        return;
    }
    
    detector->current_iteration = 0;
    detector->current_error = INFINITY;
    detector->previous_error = INFINITY;
    detector->history_size = 0;
    
    detector->has_converged = false;
    detector->convergence_iteration = 0;
    detector->iterations_stable = 0;
    
    detector->abs_converged = false;
    detector->rel_converged = false;
    detector->var_converged = false;
    detector->grad_converged = false;
    detector->stable_converged = false;
    
    detector->min_error = INFINITY;
    detector->max_error = -INFINITY;
    detector->avg_error = 0.0;
    detector->convergence_rate = 0.0;
    
    memset(detector->error_history, 0, 
           detector->history_capacity * sizeof(double));
}

void convergence_detector_free(ConvergenceDetector* detector) {
    if (detector) {
        if (detector->error_history) {
            free(detector->error_history);
        }
        free(detector);
    }
}

double convergence_detector_get_confidence(
    const ConvergenceDetector* detector
) {
    if (!detector) {
        return 0.0;
    }
    
    ConvergenceResult result = convergence_detector_get_result(detector);
    return result.confidence;
}

const double* convergence_detector_get_history(
    const ConvergenceDetector* detector,
    uint32_t* size_out
) {
    if (!detector || !size_out) {
        return NULL;
    }
    
    *size_out = detector->history_size;
    return detector->error_history;
}

// ============================================================================
// ADVANCED API IMPLEMENTATION
// ============================================================================

bool convergence_detector_update_multi(
    ConvergenceDetector* detector,
    const double* metrics,
    uint32_t num_metrics
) {
    if (!detector || !metrics || num_metrics == 0) {
        return false;
    }
    
    // Compute combined error (average of all metrics)
    double combined_error = 0.0;
    for (uint32_t i = 0; i < num_metrics; i++) {
        combined_error += metrics[i];
    }
    combined_error /= num_metrics;
    
    // Update with combined error
    return convergence_detector_update(detector, combined_error);
}

void convergence_detector_adapt_thresholds(
    ConvergenceDetector* detector,
    double adaptation_rate
) {
    if (!detector || adaptation_rate <= 0.0 || adaptation_rate >= 1.0) {
        return;
    }
    
    // Adapt thresholds based on current error behavior
    if (detector->history_size > 10) {
        // Compute recent error trend
        double recent_avg = 0.0;
        uint32_t window = 10;
        for (uint32_t i = detector->history_size - window; 
             i < detector->history_size; i++) {
            recent_avg += detector->error_history[i];
        }
        recent_avg /= window;
        
        // If error is decreasing, tighten thresholds
        if (recent_avg < detector->avg_error) {
            detector->criteria.abs_threshold *= (1.0 - adaptation_rate);
            detector->criteria.rel_threshold *= (1.0 - adaptation_rate);
            detector->criteria.var_threshold *= (1.0 - adaptation_rate);
        }
        // If error is increasing, loosen thresholds
        else {
            detector->criteria.abs_threshold *= (1.0 + adaptation_rate);
            detector->criteria.rel_threshold *= (1.0 + adaptation_rate);
            detector->criteria.var_threshold *= (1.0 + adaptation_rate);
        }
    }
}

ConvergenceDiagnostics convergence_detector_get_diagnostics(
    const ConvergenceDetector* detector
) {
    ConvergenceDiagnostics diag = {0};
    
    if (!detector || detector->history_size < 2) {
        return diag;
    }
    
    // Convergence rate
    diag.convergence_rate = detector->convergence_rate;
    
    // Oscillation amplitude (max - min in recent window)
    uint32_t window = 10;
    if (detector->history_size >= window) {
        double window_min = INFINITY;
        double window_max = -INFINITY;
        
        for (uint32_t i = detector->history_size - window; 
             i < detector->history_size; i++) {
            double val = detector->error_history[i];
            if (val < window_min) window_min = val;
            if (val > window_max) window_max = val;
        }
        
        diag.oscillation_amplitude = window_max - window_min;
        diag.is_oscillating = (diag.oscillation_amplitude > 
                               detector->criteria.abs_threshold * 10.0);
    }
    
    // Trend (positive = improving, negative = degrading)
    if (detector->history_size >= 2) {
        diag.trend = detector->previous_error - detector->current_error;
    }
    
    // Diverging check
    diag.is_diverging = (diag.trend < 0 && 
                         math_abs(diag.trend) > detector->criteria.abs_threshold);
    
    // Estimate iterations remaining
    if (diag.convergence_rate > 0 && detector->current_error > 0) {
        diag.estimated_iterations_remaining = 
            (uint32_t)(detector->current_error / diag.convergence_rate);
    } else {
        diag.estimated_iterations_remaining = 
            detector->criteria.max_iterations - detector->current_iteration;
    }
    
    return diag;
}

bool convergence_detector_set_criteria(
    ConvergenceDetector* detector,
    const ConvergenceCriteria* criteria
) {
    if (!detector || !criteria) {
        return false;
    }
    
    detector->criteria = *criteria;
    return true;
}

bool convergence_detector_get_criteria(
    const ConvergenceDetector* detector,
    ConvergenceCriteria* criteria_out
) {
    if (!detector || !criteria_out) {
        return false;
    }
    
    *criteria_out = detector->criteria;
    return true;
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

ConvergenceCriteria convergence_criteria_default(void) {
    ConvergenceCriteria criteria = {
        .abs_threshold = 1e-6,
        .rel_threshold = 1e-4,
        .var_threshold = 1e-8,
        .grad_threshold = 1e-5,
        .stability_window = 10,
        .moving_avg_window = 20,
        .max_iterations = 1000,
        .min_iterations = 10
    };
    return criteria;
}

ConvergenceCriteria convergence_criteria_strict(void) {
    ConvergenceCriteria criteria = {
        .abs_threshold = 1e-8,
        .rel_threshold = 1e-6,
        .var_threshold = 1e-10,
        .grad_threshold = 1e-7,
        .stability_window = 20,
        .moving_avg_window = 30,
        .max_iterations = 5000,
        .min_iterations = 50
    };
    return criteria;
}

ConvergenceCriteria convergence_criteria_relaxed(void) {
    ConvergenceCriteria criteria = {
        .abs_threshold = 1e-4,
        .rel_threshold = 1e-2,
        .var_threshold = 1e-6,
        .grad_threshold = 1e-3,
        .stability_window = 5,
        .moving_avg_window = 10,
        .max_iterations = 500,
        .min_iterations = 5
    };
    return criteria;
}

void convergence_result_print(const ConvergenceResult* result) {
    if (!result) return;
    
    printf("\n╔══════════════════════════════════════════════════════════╗\n");
    printf("║  Convergence Result                                      ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n\n");
    
    printf("  Converged: %s\n", result->converged ? "YES" : "NO");
    printf("  Iterations: %u\n", result->iterations);
    printf("  Final Error: %.6e\n", result->final_error);
    printf("  Convergence Rate: %.6e\n", result->convergence_rate);
    printf("  Confidence: %.2f%%\n\n", result->confidence * 100.0);
    
    printf("  Criteria Met:\n");
    printf("    Absolute:  %s\n", result->abs_criterion_met ? "✓" : "✗");
    printf("    Relative:  %s\n", result->rel_criterion_met ? "✓" : "✗");
    printf("    Variance:  %s\n", result->var_criterion_met ? "✓" : "✗");
    printf("    Gradient:  %s\n", result->grad_criterion_met ? "✓" : "✗");
    printf("    Stability: %s\n\n", result->stable_criterion_met ? "✓" : "✗");
}

void convergence_diagnostics_print(const ConvergenceDiagnostics* diagnostics) {
    if (!diagnostics) return;
    
    printf("\n╔══════════════════════════════════════════════════════════╗\n");
    printf("║  Convergence Diagnostics                                 ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n\n");
    
    printf("  Convergence Rate: %.6e\n", diagnostics->convergence_rate);
    printf("  Oscillation Amplitude: %.6e\n", diagnostics->oscillation_amplitude);
    printf("  Trend: %.6e %s\n", diagnostics->trend,
           diagnostics->trend > 0 ? "(improving)" : "(degrading)");
    printf("  Oscillating: %s\n", diagnostics->is_oscillating ? "YES" : "NO");
    printf("  Diverging: %s\n", diagnostics->is_diverging ? "YES" : "NO");
    printf("  Estimated Iterations Remaining: %u\n\n", 
           diagnostics->estimated_iterations_remaining);
}