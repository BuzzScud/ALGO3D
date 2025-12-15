/**
 * @file convergence_detection.h
 * @brief Universal Convergence Detection
 * 
 * UNIVERSAL GEOMETRIC MATHEMATICS - Works with ANY system!
 * 
 * Provides consistent, configurable convergence detection for all phases
 * of the geometric recovery pipeline. Implements 5 convergence criteria:
 * 
 * 1. Absolute Error: |error(t) - error(t-1)| < ε_abs
 * 2. Relative Error: |error(t) - error(t-1)| / |error(t-1)| < ε_rel
 * 3. Moving Average Variance: variance(error[t-w:t]) < ε_var
 * 4. Gradient: |d(error)/dt| < ε_grad
 * 5. Stability: error remains below threshold for n iterations
 * 
 * NO crypto-specific dependencies - works with raw uint64_t data.
 */

#ifndef CONVERGENCE_DETECTION_H
#define CONVERGENCE_DETECTION_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// STRUCTURES
// ============================================================================

/**
 * Convergence criteria configuration
 */
typedef struct {
    // Absolute error threshold
    double abs_threshold;
    
    // Relative error threshold (percentage)
    double rel_threshold;
    
    // Variance threshold for moving average
    double var_threshold;
    
    // Gradient threshold
    double grad_threshold;
    
    // Stability requirements
    uint32_t stability_window;      // Must be stable for this many iterations
    
    // Moving average window size
    uint32_t moving_avg_window;
    
    // Maximum iterations (safety limit)
    uint32_t max_iterations;
    
    // Minimum iterations (prevent premature convergence)
    uint32_t min_iterations;
    
} ConvergenceCriteria;

/**
 * Convergence detector state
 */
typedef struct ConvergenceDetector ConvergenceDetector;

/**
 * Convergence result
 */
typedef struct {
    bool converged;
    uint32_t iterations;
    double final_error;
    double convergence_rate;
    
    // Which criteria were met
    bool abs_criterion_met;
    bool rel_criterion_met;
    bool var_criterion_met;
    bool grad_criterion_met;
    bool stable_criterion_met;
    
    // Confidence in convergence
    double confidence;
    
} ConvergenceResult;

/**
 * Convergence diagnostics
 */
typedef struct {
    double convergence_rate;
    double oscillation_amplitude;
    double trend;                   // Positive = improving, negative = degrading
    bool is_oscillating;
    bool is_diverging;
    uint32_t estimated_iterations_remaining;
} ConvergenceDiagnostics;

// ============================================================================
// CORE API
// ============================================================================

/**
 * Create convergence detector with default criteria
 * 
 * Default criteria:
 * - abs_threshold: 1e-6
 * - rel_threshold: 1e-4
 * - var_threshold: 1e-8
 * - grad_threshold: 1e-5
 * - stability_window: 10
 * - moving_avg_window: 20
 * - min_iterations: 10
 * 
 * @param max_iterations Maximum iterations
 * @return New detector or NULL on error
 */
ConvergenceDetector* convergence_detector_create(
    uint32_t max_iterations
);

/**
 * Create convergence detector with custom criteria
 * 
 * @param criteria Custom convergence criteria
 * @return New detector or NULL on error
 */
ConvergenceDetector* convergence_detector_create_custom(
    const ConvergenceCriteria* criteria
);

/**
 * Update detector with new error value
 * 
 * @param detector Convergence detector
 * @param error Current error value
 * @return true if converged, false otherwise
 */
bool convergence_detector_update(
    ConvergenceDetector* detector,
    double error
);

/**
 * Check if converged (without updating)
 * 
 * @param detector Convergence detector
 * @return true if converged, false otherwise
 */
bool convergence_detector_check(
    const ConvergenceDetector* detector
);

/**
 * Get convergence result
 * 
 * @param detector Convergence detector
 * @return Convergence result structure
 */
ConvergenceResult convergence_detector_get_result(
    const ConvergenceDetector* detector
);

/**
 * Reset detector for new run
 * 
 * @param detector Convergence detector
 */
void convergence_detector_reset(
    ConvergenceDetector* detector
);

/**
 * Free detector
 * 
 * @param detector Convergence detector to free
 */
void convergence_detector_free(
    ConvergenceDetector* detector
);

/**
 * Get convergence confidence (0.0 to 1.0)
 * 
 * @param detector Convergence detector
 * @return Confidence score
 */
double convergence_detector_get_confidence(
    const ConvergenceDetector* detector
);

/**
 * Get error history for analysis
 * 
 * @param detector Convergence detector
 * @param size_out Pointer to store history size
 * @return Pointer to error history array
 */
const double* convergence_detector_get_history(
    const ConvergenceDetector* detector,
    uint32_t* size_out
);

// ============================================================================
// ADVANCED API
// ============================================================================

/**
 * Multi-metric convergence detection
 * 
 * Checks convergence across multiple metrics simultaneously.
 * All metrics must converge for overall convergence.
 * 
 * @param detector Convergence detector
 * @param metrics Array of metric values
 * @param num_metrics Number of metrics
 * @return true if all metrics converged, false otherwise
 */
bool convergence_detector_update_multi(
    ConvergenceDetector* detector,
    const double* metrics,
    uint32_t num_metrics
);

/**
 * Adaptive threshold adjustment
 * 
 * Automatically adjusts thresholds based on convergence behavior.
 * 
 * @param detector Convergence detector
 * @param adaptation_rate Rate of adaptation (0.0 to 1.0)
 */
void convergence_detector_adapt_thresholds(
    ConvergenceDetector* detector,
    double adaptation_rate
);

/**
 * Get convergence diagnostics
 * 
 * @param detector Convergence detector
 * @return Diagnostics structure
 */
ConvergenceDiagnostics convergence_detector_get_diagnostics(
    const ConvergenceDetector* detector
);

/**
 * Set custom convergence criteria
 * 
 * @param detector Convergence detector
 * @param criteria New criteria
 * @return true on success, false on error
 */
bool convergence_detector_set_criteria(
    ConvergenceDetector* detector,
    const ConvergenceCriteria* criteria
);

/**
 * Get current convergence criteria
 * 
 * @param detector Convergence detector
 * @param criteria_out Pointer to store criteria
 * @return true on success, false on error
 */
bool convergence_detector_get_criteria(
    const ConvergenceDetector* detector,
    ConvergenceCriteria* criteria_out
);

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * Create default convergence criteria
 * 
 * @return Default criteria structure
 */
ConvergenceCriteria convergence_criteria_default(void);

/**
 * Create strict convergence criteria
 * 
 * @return Strict criteria structure (tighter thresholds)
 */
ConvergenceCriteria convergence_criteria_strict(void);

/**
 * Create relaxed convergence criteria
 * 
 * @return Relaxed criteria structure (looser thresholds)
 */
ConvergenceCriteria convergence_criteria_relaxed(void);

/**
 * Print convergence result
 * 
 * @param result Convergence result to print
 */
void convergence_result_print(const ConvergenceResult* result);

/**
 * Print convergence diagnostics
 * 
 * @param diagnostics Diagnostics to print
 */
void convergence_diagnostics_print(const ConvergenceDiagnostics* diagnostics);

#ifdef __cplusplus
}
#endif

#endif // CONVERGENCE_DETECTION_H