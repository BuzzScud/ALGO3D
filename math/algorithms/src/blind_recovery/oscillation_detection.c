/**
 * @file oscillation_detection.c
 * @brief Oscillation detection using FFT (OBJECTIVE 28 - Layer 1)
 * 
 * Detects oscillations in multi-dimensional structures using Fast Fourier Transform.
 * 
 * KEY INSIGHT: Oscillations reveal structural information
 * - Stable oscillations → Valid geometric constraints
 * - Unstable oscillations → Corruption detected
 * - Oscillation patterns → Underlying structure revealed
 * 
 * PHASE 2: Migrated to NEW math library
 * - Replaced double complex with MathComplex
 * - Replaced cexp/cabs/carg with math_complex_* functions
 * - Removed <complex.h> dependency
 */

#include "blind_recovery/blind_recovery.h"
#include "math/complex.h"        // PHASE 2: NEW math library complex numbers
#include "math/types.h"           // For MATH_PI
#include "math/transcendental.h"  // For math_sqrt, math_exp, math_cos, math_sin
#include "math/arithmetic.h"      // For math_abs
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Simple FFT implementation (Cooley-Tukey algorithm)
// For production, would use FFTW library

/**
 * Compute FFT of real-valued signal
 * 
 * @param data Input/output data (in-place transform)
 * @param n Number of samples (must be power of 2)
 */
static void fft_real(MathComplex* data, uint32_t n) {
    if (n <= 1) return;
    
    // Divide
    MathComplex* even = malloc((n/2) * sizeof(MathComplex));
    MathComplex* odd = malloc((n/2) * sizeof(MathComplex));
    
    for (uint32_t i = 0; i < n/2; i++) {
        even[i] = data[i*2];
        odd[i] = data[i*2 + 1];
    }
    
    // Conquer
    fft_real(even, n/2);
    fft_real(odd, n/2);
    
    // Combine
    for (uint32_t k = 0; k < n/2; k++) {
        double angle = -2.0 * MATH_PI * k / n;
        MathComplex twiddle = math_complex_from_cartesian(math_cos(angle), math_sin(angle));
        MathComplex t = math_complex_mul(twiddle, odd[k]);
        data[k] = math_complex_add(even[k], t);
        data[k + n/2] = math_complex_sub(even[k], t);
    }
    
    free(even);
    free(odd);
}

/**
 * Find dominant frequency in FFT output
 * 
 * @param fft_data FFT output
 * @param n Number of samples
 * @param sampling_rate Sampling rate (Hz)
 * @param frequency Output: dominant frequency
 * @param amplitude Output: amplitude
 * @param phase Output: phase
 */
static void find_dominant_frequency(
    const MathComplex* fft_data,
    uint32_t n,
    double sampling_rate,
    double* frequency,
    double* amplitude,
    double* phase
) {
    double max_magnitude = 0.0;
    uint32_t max_index = 0;
    
    // Find peak in frequency domain (skip DC component)
    for (uint32_t i = 1; i < n/2; i++) {
        double magnitude = math_complex_magnitude(fft_data[i]);
        if (magnitude > max_magnitude) {
            max_magnitude = magnitude;
            max_index = i;
        }
    }
    
    // Convert to frequency
    *frequency = (double)max_index * sampling_rate / (double)n;
    *amplitude = max_magnitude / (double)n;  // Normalize
    *phase = math_complex_phase(fft_data[max_index]);
}

/**
 * Detect if oscillation is stable
 * 
 * @param amplitudes Array of amplitudes over time
 * @param num_samples Number of samples
 * @return true if stable, false if unstable
 */
static bool is_oscillation_stable(const double* amplitudes, uint32_t num_samples) {
    if (num_samples < 2) return true;
    
    // Compute variance of amplitudes
    double mean = 0.0;
    for (uint32_t i = 0; i < num_samples; i++) {
        mean += amplitudes[i];
    }
    mean /= num_samples;
    
    double variance = 0.0;
    for (uint32_t i = 0; i < num_samples; i++) {
        double diff = amplitudes[i] - mean;
        variance += diff * diff;
    }
    variance /= num_samples;
    
    // Stable if variance is low relative to mean
    double coefficient_of_variation = math_sqrt(variance) / (mean + 1e-10);
    return coefficient_of_variation < 0.1;  // 10% threshold
}

/**
 * Compute convergence rate
 * 
 * @param amplitudes Array of amplitudes over time
 * @param num_samples Number of samples
 * @return Convergence rate (negative if diverging)
 */
static double compute_convergence_rate(const double* amplitudes, uint32_t num_samples) {
    if (num_samples < 2) return 0.0;
    
    // Linear regression to find trend
    double sum_x = 0.0, sum_y = 0.0, sum_xy = 0.0, sum_xx = 0.0;
    
    for (uint32_t i = 0; i < num_samples; i++) {
        double x = (double)i;
        double y = amplitudes[i];
        sum_x += x;
        sum_y += y;
        sum_xy += x * y;
        sum_xx += x * x;
    }
    
    double n = (double)num_samples;
    double slope = (n * sum_xy - sum_x * sum_y) / (n * sum_xx - sum_x * sum_x);
    
    // Negative slope = converging, positive = diverging
    return -slope;
}

/**
 * Detect oscillations in structure using FFT
 */
OscillationMap* detect_oscillations(
    const double* structure_data,
    uint32_t num_dimensions,
    uint32_t num_samples,
    double sampling_rate
) {
    if (!structure_data || num_dimensions == 0 || num_samples == 0) {
        return NULL;
    }
    
    printf("🔬 Detecting oscillations...\n");
    printf("  Dimensions: %u\n", num_dimensions);
    printf("  Samples: %u\n", num_samples);
    printf("  Sampling rate: %.2f Hz\n", sampling_rate);
    
    // Allocate oscillation map
    OscillationMap* map = malloc(sizeof(OscillationMap));
    if (!map) return NULL;
    
    map->num_dimensions = num_dimensions;
    map->signatures = calloc(num_dimensions, sizeof(OscillationSignature));
    map->cross_correlations = calloc(num_dimensions * num_dimensions, sizeof(double));
    map->is_converging = true;
    map->iterations_to_convergence = 0;
    
    if (!map->signatures || !map->cross_correlations) {
        free_oscillation_map(map);
        return NULL;
    }
    
    // Analyze each dimension
    for (uint32_t d = 0; d < num_dimensions; d++) {
        // Extract samples for this dimension
        MathComplex* fft_data = malloc(num_samples * sizeof(MathComplex));
        if (!fft_data) continue;
        
        for (uint32_t i = 0; i < num_samples; i++) {
            fft_data[i] = math_complex_from_cartesian(structure_data[d * num_samples + i], 0.0);
        }
        
        // Compute FFT
        fft_real(fft_data, num_samples);
        
        // Find dominant frequency
        double frequency, amplitude, phase;
        find_dominant_frequency(fft_data, num_samples, sampling_rate,
                               &frequency, &amplitude, &phase);
        
        // Store signature
        map->signatures[d].dimension = d;
        map->signatures[d].frequency = frequency;
        map->signatures[d].amplitude = amplitude;
        map->signatures[d].phase = phase;
        map->signatures[d].period = (uint64_t)(sampling_rate / (frequency + 1e-10));
        
        // Check stability
        double* amplitudes = malloc(num_samples * sizeof(double));
        for (uint32_t i = 0; i < num_samples; i++) {
            amplitudes[i] = math_complex_magnitude(fft_data[i]);
        }
        map->signatures[d].is_stable = is_oscillation_stable(amplitudes, num_samples);
        map->signatures[d].convergence_rate = compute_convergence_rate(amplitudes, num_samples);
        
        free(amplitudes);
        free(fft_data);
        
        // Update global convergence
        if (map->signatures[d].convergence_rate < 0) {
            map->is_converging = false;
        }
        
        printf("  Dimension %u: freq=%.2f Hz, amp=%.4f, %s\n",
               d, frequency, amplitude,
               map->signatures[d].is_stable ? "stable" : "unstable");
    }
    
    // Compute cross-correlations
    for (uint32_t i = 0; i < num_dimensions; i++) {
        for (uint32_t j = 0; j < num_dimensions; j++) {
            if (i == j) {
                map->cross_correlations[i * num_dimensions + j] = 1.0;
            } else {
                // Simplified correlation based on frequency similarity
                double freq_diff = math_abs(map->signatures[i].frequency - 
                                       map->signatures[j].frequency);
                double correlation = math_exp(-freq_diff / 10.0);  // Decay with difference
                map->cross_correlations[i * num_dimensions + j] = correlation;
            }
        }
    }
    
    printf("  ✓ Oscillation detection complete\n");
    printf("  Converging: %s\n", map->is_converging ? "yes" : "no");
    
    return map;
}

/**
 * Free oscillation map
 */
void free_oscillation_map(OscillationMap* map) {
    if (!map) return;
    
    if (map->signatures) {
        free(map->signatures);
    }
    
    if (map->cross_correlations) {
        free(map->cross_correlations);
    }
    
    free(map);
}