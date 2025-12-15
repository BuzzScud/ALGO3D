/**
 * @file oscillation_detection.c
 * @brief General-Purpose Oscillation Detection for Geometric Recovery
 * 
 * Implements FFT-based oscillation detection and stability analysis.
 * Extracted and adapted from blind_recovery/oscillation_detection.c
 * for general-purpose use in geometric recovery.
 * 
 * Key Features:
 * - Cooley-Tukey FFT algorithm
 * - Stability detection via coefficient of variation
 * - Convergence rate via linear regression
 * - Multi-dimensional tracking
 * - Cross-correlation analysis
 * 
 * Thesis Reference: Chapter 16.5 - Convergence Analysis
 */

#include "geometric_recovery/oscillation_detection.h"
#include "math/complex.h"
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * INTERNAL FUNCTIONS - FFT
 * ============================================================================ */

/**
 * @brief Compute FFT using Cooley-Tukey algorithm
 * 
 * In-place FFT computation. Input array is modified.
 * 
 * @param data Input/output complex data
 * @param n Number of samples (must be power of 2)
 */
static void fft_compute(MathComplex* data, uint32_t n) {
    if (n <= 1) return;
    
    // Divide
    MathComplex* even = malloc((n/2) * sizeof(MathComplex));
    MathComplex* odd = malloc((n/2) * sizeof(MathComplex));
    
    if (!even || !odd) {
        free(even);
        free(odd);
        return;
    }
    
    for (uint32_t i = 0; i < n/2; i++) {
        even[i] = data[i*2];
        odd[i] = data[i*2 + 1];
    }
    
    // Conquer
    fft_compute(even, n/2);
    fft_compute(odd, n/2);
    
    // Combine
    for (uint32_t k = 0; k < n/2; k++) {
        double angle = -2.0 * MATH_PI * k / n;
        MathComplex twiddle = math_complex_from_cartesian(
            math_cos(angle), 
            math_sin(angle)
        );
        MathComplex t = math_complex_mul(twiddle, odd[k]);
        data[k] = math_complex_add(even[k], t);
        data[k + n/2] = math_complex_sub(even[k], t);
    }
    
    free(even);
    free(odd);
}

/**
 * @brief Find dominant frequency in FFT output
 * 
 * @param fft_data FFT output
 * @param n Number of samples
 * @param sampling_rate Sampling rate (Hz)
 * @param frequency Output: dominant frequency
 * @param amplitude Output: amplitude
 * @param phase Output: phase
 * @param energy Output: total energy
 */
static void find_dominant_frequency(
    const MathComplex* fft_data,
    uint32_t n,
    double sampling_rate,
    double* frequency,
    double* amplitude,
    double* phase,
    double* energy
) {
    double max_magnitude = 0.0;
    uint32_t max_index = 0;
    double total_energy = 0.0;
    
    // Find peak in frequency domain (skip DC component)
    for (uint32_t i = 1; i < n/2; i++) {
        double magnitude = math_complex_magnitude(fft_data[i]);
        total_energy += magnitude * magnitude;
        
        if (magnitude > max_magnitude) {
            max_magnitude = magnitude;
            max_index = i;
        }
    }
    
    // Convert to frequency
    *frequency = (double)max_index * sampling_rate / (double)n;
    *amplitude = max_magnitude / (double)n;  // Normalize
    *phase = math_complex_phase(fft_data[max_index]);
    *energy = total_energy;
}

/* ============================================================================
 * INTERNAL FUNCTIONS - ANALYSIS
 * ============================================================================ */

/**
 * @brief Check if oscillation is stable
 * 
 * Uses coefficient of variation to determine stability.
 * 
 * @param values Array of values over time
 * @param num_samples Number of samples
 * @param threshold Stability threshold
 * @return true if stable, false otherwise
 */
static bool is_stable(
    const double* values, 
    uint32_t num_samples,
    double threshold
) {
    if (num_samples < 2) return true;
    
    // Compute mean
    double mean = 0.0;
    for (uint32_t i = 0; i < num_samples; i++) {
        mean += values[i];
    }
    mean /= num_samples;
    
    // Compute variance
    double variance = 0.0;
    for (uint32_t i = 0; i < num_samples; i++) {
        double diff = values[i] - mean;
        variance += diff * diff;
    }
    variance /= num_samples;
    
    // Coefficient of variation
    double cv = math_sqrt(variance) / (math_abs(mean) + 1e-10);
    return cv < threshold;
}

/**
 * @brief Compute convergence rate via linear regression
 * 
 * Positive rate = converging, negative = diverging
 * 
 * @param values Array of values over time
 * @param num_samples Number of samples
 * @return Convergence rate
 */
static double compute_convergence_rate(
    const double* values,
    uint32_t num_samples
) {
    if (num_samples < 2) return 0.0;
    
    // Linear regression: y = mx + b
    double sum_x = 0.0, sum_y = 0.0, sum_xy = 0.0, sum_xx = 0.0;
    
    for (uint32_t i = 0; i < num_samples; i++) {
        double x = (double)i;
        double y = values[i];
        sum_x += x;
        sum_y += y;
        sum_xy += x * y;
        sum_xx += x * x;
    }
    
    double n = (double)num_samples;
    double denominator = n * sum_xx - sum_x * sum_x;
    
    if (math_abs(denominator) < 1e-10) return 0.0;
    
    double slope = (n * sum_xy - sum_x * sum_y) / denominator;
    
    // Negative slope = converging (values decreasing)
    return -slope;
}

/* ============================================================================
 * PUBLIC API
 * ============================================================================ */

OscillationDetector* oscillation_detector_create(
    uint32_t num_dimensions,
    uint32_t window_size
) {
    if (num_dimensions == 0 || window_size == 0) return NULL;
    
    // Verify window_size is power of 2
    if ((window_size & (window_size - 1)) != 0) {
        fprintf(stderr, "Error: window_size must be power of 2\n");
        return NULL;
    }
    
    OscillationDetector* detector = calloc(1, sizeof(OscillationDetector));
    if (!detector) return NULL;
    
    detector->num_dimensions = num_dimensions;
    detector->window_size = window_size;
    detector->num_samples = 0;
    detector->buffer_position = 0;
    detector->sampling_rate = 1.0;  // Default: 1 Hz
    detector->is_converging = false;
    detector->stable_iterations = 0;
    
    // Default thresholds
    detector->stability_threshold = 0.1;      // 10% coefficient of variation
    detector->convergence_threshold = 0.01;   // Minimum convergence rate
    detector->min_stable_iterations = 5;      // 5 consecutive stable iterations
    
    // Allocate buffers
    detector->sample_buffer = calloc(num_dimensions * window_size, sizeof(double));
    detector->signatures = calloc(num_dimensions, sizeof(OscillationSignature));
    detector->cross_correlations = calloc(num_dimensions * num_dimensions, sizeof(double));
    
    if (!detector->sample_buffer || !detector->signatures || !detector->cross_correlations) {
        oscillation_detector_free(detector);
        return NULL;
    }
    
    // Initialize signatures
    for (uint32_t i = 0; i < num_dimensions; i++) {
        detector->signatures[i].dimension = i;
    }
    
    return detector;
}

void oscillation_detector_free(OscillationDetector* detector) {
    if (!detector) return;
    
    free(detector->sample_buffer);
    free(detector->signatures);
    free(detector->cross_correlations);
    free(detector);
}

int oscillation_detector_add_sample(
    OscillationDetector* detector,
    const double* values
) {
    if (!detector || !values) return -1;
    
    // Add samples to circular buffer
    for (uint32_t d = 0; d < detector->num_dimensions; d++) {
        uint32_t offset = d * detector->window_size;
        detector->sample_buffer[offset + detector->buffer_position] = values[d];
    }
    
    // Update position
    detector->buffer_position = (detector->buffer_position + 1) % detector->window_size;
    
    // Update sample count
    if (detector->num_samples < detector->window_size) {
        detector->num_samples++;
    }
    
    return 0;
}

int oscillation_detector_analyze(OscillationDetector* detector) {
    if (!detector) return -1;
    
    // Need full window for analysis
    if (detector->num_samples < detector->window_size) {
        return -1;
    }
    
    bool all_stable = true;
    double total_convergence_rate = 0.0;
    
    // Analyze each dimension
    for (uint32_t d = 0; d < detector->num_dimensions; d++) {
        uint32_t offset = d * detector->window_size;
        
        // Prepare FFT input (reorder from circular buffer)
        MathComplex* fft_data = malloc(detector->window_size * sizeof(MathComplex));
        if (!fft_data) continue;
        
        for (uint32_t i = 0; i < detector->window_size; i++) {
            uint32_t idx = (detector->buffer_position + i) % detector->window_size;
            double value = detector->sample_buffer[offset + idx];
            fft_data[i] = math_complex_from_cartesian(value, 0.0);
        }
        
        // Compute FFT
        fft_compute(fft_data, detector->window_size);
        
        // Find dominant frequency
        double frequency, amplitude, phase, energy;
        find_dominant_frequency(
            fft_data, 
            detector->window_size,
            detector->sampling_rate,
            &frequency, 
            &amplitude, 
            &phase,
            &energy
        );
        
        // Store signature
        detector->signatures[d].frequency = frequency;
        detector->signatures[d].amplitude = amplitude;
        detector->signatures[d].phase = phase;
        detector->signatures[d].energy = energy;
        detector->signatures[d].period = (uint64_t)(detector->sampling_rate / (frequency + 1e-10));
        
        // Check stability
        double* values = malloc(detector->window_size * sizeof(double));
        if (values) {
            for (uint32_t i = 0; i < detector->window_size; i++) {
                uint32_t idx = (detector->buffer_position + i) % detector->window_size;
                values[i] = detector->sample_buffer[offset + idx];
            }
            
            detector->signatures[d].is_stable = is_stable(
                values, 
                detector->window_size,
                detector->stability_threshold
            );
            
            detector->signatures[d].convergence_rate = compute_convergence_rate(
                values,
                detector->window_size
            );
            
            free(values);
        }
        
        free(fft_data);
        
        // Update global flags
        if (!detector->signatures[d].is_stable) {
            all_stable = false;
        }
        
        total_convergence_rate += detector->signatures[d].convergence_rate;
    }
    
    // Update global convergence
    double avg_convergence_rate = total_convergence_rate / detector->num_dimensions;
    detector->is_converging = (avg_convergence_rate > detector->convergence_threshold);
    
    // Update stable iterations counter
    if (all_stable) {
        detector->stable_iterations++;
    } else {
        detector->stable_iterations = 0;
    }
    
    // Compute cross-correlations
    for (uint32_t i = 0; i < detector->num_dimensions; i++) {
        for (uint32_t j = 0; j < detector->num_dimensions; j++) {
            if (i == j) {
                detector->cross_correlations[i * detector->num_dimensions + j] = 1.0;
            } else {
                // Correlation based on frequency similarity
                double freq_diff = math_abs(
                    detector->signatures[i].frequency - 
                    detector->signatures[j].frequency
                );
                double correlation = math_exp(-freq_diff / 10.0);
                detector->cross_correlations[i * detector->num_dimensions + j] = correlation;
            }
        }
    }
    
    return 0;
}

bool oscillation_detector_is_stable(const OscillationDetector* detector) {
    if (!detector) return false;
    
    return detector->stable_iterations >= detector->min_stable_iterations;
}

bool oscillation_detector_is_converging(const OscillationDetector* detector) {
    if (!detector) return false;
    
    return detector->is_converging;
}

int oscillation_detector_get_analysis(
    const OscillationDetector* detector,
    OscillationAnalysis* analysis
) {
    if (!detector || !analysis) return -1;
    
    memset(analysis, 0, sizeof(OscillationAnalysis));
    
    if (detector->num_samples < detector->window_size) {
        return -1;  // Not enough data
    }
    
    double max_amplitude = 0.0;
    double dominant_frequency = 0.0;
    double max_energy = 0.0;
    double total_convergence_rate = 0.0;
    uint32_t unstable_count = 0;
    
    for (uint32_t d = 0; d < detector->num_dimensions; d++) {
        const OscillationSignature* sig = &detector->signatures[d];
        
        if (sig->amplitude > max_amplitude) {
            max_amplitude = sig->amplitude;
        }
        
        if (sig->energy > max_energy) {
            max_energy = sig->energy;
            dominant_frequency = sig->frequency;
        }
        
        total_convergence_rate += sig->convergence_rate;
        
        if (!sig->is_stable) {
            unstable_count++;
        }
    }
    
    analysis->is_stable = oscillation_detector_is_stable(detector);
    analysis->is_converging = detector->is_converging;
    analysis->dominant_frequency = dominant_frequency;
    analysis->max_amplitude = max_amplitude;
    analysis->avg_convergence_rate = total_convergence_rate / detector->num_dimensions;
    analysis->unstable_dimensions = unstable_count;
    
    return 0;
}

const OscillationSignature* oscillation_detector_get_signature(
    const OscillationDetector* detector,
    uint32_t dimension
) {
    if (!detector || dimension >= detector->num_dimensions) {
        return NULL;
    }
    
    return &detector->signatures[dimension];
}

double oscillation_detector_get_correlation(
    const OscillationDetector* detector,
    uint32_t dim1,
    uint32_t dim2
) {
    if (!detector || dim1 >= detector->num_dimensions || dim2 >= detector->num_dimensions) {
        return 0.0;
    }
    
    return detector->cross_correlations[dim1 * detector->num_dimensions + dim2];
}

void oscillation_detector_reset(OscillationDetector* detector) {
    if (!detector) return;
    
    detector->num_samples = 0;
    detector->buffer_position = 0;
    detector->is_converging = false;
    detector->stable_iterations = 0;
    
    memset(detector->sample_buffer, 0, 
           detector->num_dimensions * detector->window_size * sizeof(double));
    memset(detector->signatures, 0, 
           detector->num_dimensions * sizeof(OscillationSignature));
    memset(detector->cross_correlations, 0,
           detector->num_dimensions * detector->num_dimensions * sizeof(double));
    
    // Reinitialize dimension indices
    for (uint32_t i = 0; i < detector->num_dimensions; i++) {
        detector->signatures[i].dimension = i;
    }
}

void oscillation_detector_set_stability_threshold(
    OscillationDetector* detector,
    double threshold
) {
    if (detector && threshold > 0.0) {
        detector->stability_threshold = threshold;
    }
}

void oscillation_detector_set_convergence_threshold(
    OscillationDetector* detector,
    double threshold
) {
    if (detector) {
        detector->convergence_threshold = threshold;
    }
}

void oscillation_detector_set_min_stable_iterations(
    OscillationDetector* detector,
    uint32_t iterations
) {
    if (detector) {
        detector->min_stable_iterations = iterations;
    }
}

void oscillation_detector_print_stats(const OscillationDetector* detector) {
    if (!detector) return;
    
    printf("=== Oscillation Detector Statistics ===\n");
    printf("Dimensions: %u\n", detector->num_dimensions);
    printf("Window size: %u\n", detector->window_size);
    printf("Samples collected: %u\n", detector->num_samples);
    printf("Sampling rate: %.2f Hz\n", detector->sampling_rate);
    printf("Stable iterations: %u / %u\n", 
           detector->stable_iterations, detector->min_stable_iterations);
    printf("Overall stable: %s\n", 
           oscillation_detector_is_stable(detector) ? "YES" : "NO");
    printf("Overall converging: %s\n", 
           detector->is_converging ? "YES" : "NO");
    
    if (detector->num_samples >= detector->window_size) {
        printf("\nPer-Dimension Analysis:\n");
        for (uint32_t d = 0; d < detector->num_dimensions; d++) {
            const OscillationSignature* sig = &detector->signatures[d];
            printf("  Dim %u: freq=%.3f Hz, amp=%.4f, rate=%.4f, %s\n",
                   d, sig->frequency, sig->amplitude, sig->convergence_rate,
                   sig->is_stable ? "stable" : "UNSTABLE");
        }
    }
    
    printf("========================================\n");
}