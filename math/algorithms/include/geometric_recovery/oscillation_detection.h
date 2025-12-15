/**
 * @file oscillation_detection.h
 * @brief General-Purpose Oscillation Detection for Geometric Recovery
 * 
 * Provides FFT-based oscillation detection and stability analysis for
 * convergence monitoring in geometric recovery algorithms.
 * 
 * Key Features:
 * - FFT-based frequency analysis
 * - Stability detection
 * - Convergence rate computation
 * - Multi-dimensional oscillation tracking
 * - Cross-correlation analysis
 * 
 * Thesis Reference: Chapter 16.5 - Convergence Analysis
 * 
 * Usage:
 *   OscillationDetector* detector = oscillation_detector_create(num_dims, window_size);
 *   oscillation_detector_add_sample(detector, error_values);
 *   bool stable = oscillation_detector_is_stable(detector);
 *   oscillation_detector_free(detector);
 */

#ifndef OSCILLATION_DETECTION_H
#define OSCILLATION_DETECTION_H

#include <stdint.h>
#include <stdbool.h>

/* ============================================================================
 * TYPES
 * ============================================================================ */

/**
 * @brief Oscillation signature for a single dimension
 * 
 * Contains frequency-domain characteristics of oscillations.
 */
typedef struct {
    uint32_t dimension;          // Dimension index
    double frequency;            // Dominant frequency (Hz)
    double amplitude;            // Oscillation amplitude
    double phase;                // Phase offset (radians)
    uint64_t period;             // Period in samples
    bool is_stable;              // Stability flag
    double convergence_rate;     // Rate of convergence (positive = converging)
    double energy;               // Total energy in frequency domain
} OscillationSignature;

/**
 * @brief Oscillation detector state
 * 
 * Maintains history and analysis state for oscillation detection.
 */
typedef struct {
    uint32_t num_dimensions;     // Number of dimensions being tracked
    uint32_t window_size;        // Size of analysis window (must be power of 2)
    uint32_t num_samples;        // Current number of samples collected
    
    double* sample_buffer;       // Circular buffer: [dim0_samples, dim1_samples, ...]
    uint32_t buffer_position;    // Current position in circular buffer
    
    OscillationSignature* signatures; // Per-dimension signatures
    double* cross_correlations;  // Cross-correlation matrix (num_dimensions x num_dimensions)
    
    double sampling_rate;        // Sampling rate (Hz)
    bool is_converging;          // Global convergence flag
    uint32_t stable_iterations;  // Number of consecutive stable iterations
    
    // Thresholds
    double stability_threshold;  // Coefficient of variation threshold for stability
    double convergence_threshold; // Minimum convergence rate
    uint32_t min_stable_iterations; // Minimum stable iterations for convergence
} OscillationDetector;

/**
 * @brief Oscillation analysis result
 */
typedef struct {
    bool is_stable;              // Overall stability
    bool is_converging;          // Overall convergence
    double dominant_frequency;   // Dominant frequency across all dimensions
    double max_amplitude;        // Maximum amplitude across all dimensions
    double avg_convergence_rate; // Average convergence rate
    uint32_t unstable_dimensions; // Number of unstable dimensions
} OscillationAnalysis;

/* ============================================================================
 * API FUNCTIONS
 * ============================================================================ */

/**
 * @brief Create oscillation detector
 * 
 * @param num_dimensions Number of dimensions to track
 * @param window_size Analysis window size (must be power of 2, e.g., 64, 128, 256)
 * @return Detector instance or NULL on failure
 */
OscillationDetector* oscillation_detector_create(
    uint32_t num_dimensions,
    uint32_t window_size
);

/**
 * @brief Free oscillation detector
 * 
 * @param detector Detector to free
 */
void oscillation_detector_free(OscillationDetector* detector);

/**
 * @brief Add sample to detector
 * 
 * @param detector Detector instance
 * @param values Array of values (one per dimension)
 * @return 0 on success, -1 on failure
 */
int oscillation_detector_add_sample(
    OscillationDetector* detector,
    const double* values
);

/**
 * @brief Analyze oscillations
 * 
 * Performs FFT analysis on accumulated samples.
 * 
 * @param detector Detector instance
 * @return 0 on success, -1 on failure
 */
int oscillation_detector_analyze(OscillationDetector* detector);

/**
 * @brief Check if oscillations are stable
 * 
 * @param detector Detector instance
 * @return true if stable, false otherwise
 */
bool oscillation_detector_is_stable(const OscillationDetector* detector);

/**
 * @brief Check if system is converging
 * 
 * @param detector Detector instance
 * @return true if converging, false otherwise
 */
bool oscillation_detector_is_converging(const OscillationDetector* detector);

/**
 * @brief Get oscillation analysis
 * 
 * @param detector Detector instance
 * @param analysis Output analysis result
 * @return 0 on success, -1 on failure
 */
int oscillation_detector_get_analysis(
    const OscillationDetector* detector,
    OscillationAnalysis* analysis
);

/**
 * @brief Get signature for specific dimension
 * 
 * @param detector Detector instance
 * @param dimension Dimension index
 * @return Signature or NULL if invalid dimension
 */
const OscillationSignature* oscillation_detector_get_signature(
    const OscillationDetector* detector,
    uint32_t dimension
);

/**
 * @brief Get cross-correlation between two dimensions
 * 
 * @param detector Detector instance
 * @param dim1 First dimension
 * @param dim2 Second dimension
 * @return Correlation coefficient [-1, 1] or 0.0 on error
 */
double oscillation_detector_get_correlation(
    const OscillationDetector* detector,
    uint32_t dim1,
    uint32_t dim2
);

/**
 * @brief Reset detector state
 * 
 * Clears all samples and resets analysis.
 * 
 * @param detector Detector instance
 */
void oscillation_detector_reset(OscillationDetector* detector);

/**
 * @brief Set stability threshold
 * 
 * @param detector Detector instance
 * @param threshold Coefficient of variation threshold (default: 0.1)
 */
void oscillation_detector_set_stability_threshold(
    OscillationDetector* detector,
    double threshold
);

/**
 * @brief Set convergence threshold
 * 
 * @param detector Detector instance
 * @param threshold Minimum convergence rate (default: 0.01)
 */
void oscillation_detector_set_convergence_threshold(
    OscillationDetector* detector,
    double threshold
);

/**
 * @brief Set minimum stable iterations
 * 
 * @param detector Detector instance
 * @param iterations Minimum consecutive stable iterations (default: 5)
 */
void oscillation_detector_set_min_stable_iterations(
    OscillationDetector* detector,
    uint32_t iterations
);

/**
 * @brief Print detector statistics
 * 
 * @param detector Detector instance
 */
void oscillation_detector_print_stats(const OscillationDetector* detector);

#endif // OSCILLATION_DETECTION_H