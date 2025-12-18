/**
 * Cymatic Frequency Modulation Algorithm (General Purpose)
 * 
 * This is a GENERAL algorithm that applies cymatic frequency resonance
 * to modulate gradients. It can be used by ANY training system.
 * 
 * Cymatic frequencies create smoother convergence by aligning training
 * dynamics with natural frequency patterns.
 * 
 * MIGRATED: Uses NEW math library
 * - Replaced math_cos with math_cos (7 calls)
 * - Replaced math_sin with math_sin (6 calls)
 * - Replaced math_sqrt with math_sqrt (1 call)
 * - Replaced math_pow with math_pow (1 call)
 * - Replaced MATH_PI with MATH_PI
 * - Replaced MATH_PHI with MATH_PHI
 * Total: 15 function calls migrated to NEW math library
 * 
 * PHASE 2: Consolidated constants
 * - Removed local MATH_TWO_PI definition
 * - Using MATH_TWO_PI from math/types.h
 */

#include <stdlib.h>
#include <string.h>
#include "../include/cymatic_modulation.h"
#include "math/constants.h"
#include "math/transcendental.h"  // NEW math library
#include "math/constants.h"
#include "math/arithmetic.h"       // NEW math library
#include "math/constants.h"
#include "math/types.h"            // NEW math library constants
#include "math/constants.h"

// MATH_PI is already defined in math/types.h - no need to redefine

/**
 * Apply cymatic resonance modulation to gradients
 */
void apply_cymatic_modulation(
    float* gradients,
    const uint32_t* symmetry_groups,
    uint32_t num_elements,
    uint32_t element_dim,
    uint32_t training_step,
    const double* cymatic_freqs,
    uint32_t num_freqs,
    const uint64_t* dimensional_freqs,
    double modulation_strength
) {
    if (!gradients || !symmetry_groups || !cymatic_freqs || !dimensional_freqs) return;
    if (num_elements == 0 || element_dim == 0 || num_freqs == 0) return;
    
    // Compute global phase based on training step
    // This creates a slowly varying modulation over training
    double global_phase = 2.0 * MATH_PI * (double)training_step / 1000.0;
    
    // Apply to each element
    for (uint32_t elem_id = 0; elem_id < num_elements; elem_id++) {
        uint32_t symmetry_group = symmetry_groups[elem_id];
        
        // Compute resonance for this element
        double resonance = 0.0;
        for (uint32_t f = 0; f < num_freqs; f++) {
            // Phase for this frequency (normalized to first frequency as base)
            double freq_phase = global_phase * cymatic_freqs[f] / cymatic_freqs[0];
            
            // Add harmonic contribution
            resonance += math_cos(freq_phase) / (double)num_freqs;
        }
        
        // Get dimensional frequency for element's symmetry group
        uint64_t phi_i = dimensional_freqs[symmetry_group % 12];
        
        // Modulate resonance with φᵢ
        double modulation = math_cos(2.0 * MATH_PI * (double)phi_i * resonance / 100.0);
        
        // Apply to gradients with specified modulation strength
        double scale = 1.0 + modulation_strength * modulation;
        
        for (uint32_t dim = 0; dim < element_dim; dim++) {
            gradients[elem_id * element_dim + dim] *= (float)scale;
        }
    }
}

/**
 * Compute harmonic series with golden ratio damping
 */
void compute_cymatic_harmonics(
    double base_freq,
    uint32_t num_harmonics,
    double* harmonics,
    double phi
) {
    if (!harmonics || num_harmonics == 0) return;
    
    for (uint32_t i = 0; i < num_harmonics; i++) {
        // Harmonic series: f, 2f, 3f, 4f, ...
        harmonics[i] = base_freq * (double)(i + 1);
        
        // Apply golden ratio damping: φ^(-i)
        // This makes higher harmonics progressively weaker
        double damping = math_pow(phi, -(double)i);
        harmonics[i] *= damping;
    }
}

/**
 * Analyze gradient spectrum (simple DFT)
 */
void analyze_gradient_spectrum(
    const float* gradients,
    uint32_t size,
    double* spectrum,
    uint32_t num_bins
) {
    if (!gradients || !spectrum || size == 0 || num_bins == 0) return;
    
    // Simple DFT for frequency analysis
    // spectrum[k] = |Σ gradients[n] * e^(-2πikn/N)|²
    
    for (uint32_t k = 0; k < num_bins; k++) {
        double real = 0.0;
        double imag = 0.0;
        
        for (uint32_t n = 0; n < size; n++) {
            double angle = -2.0 * MATH_PI * (double)k * (double)n / (double)size;
            real += (double)gradients[n] * math_cos(angle);
            imag += (double)gradients[n] * math_sin(angle);
        }
        
        // Power spectrum: |X[k]|²
        spectrum[k] = real * real + imag * imag;
    }
}

/* ============================================================================
 * Extended Cymatic Functions for Signal Processing
 * ============================================================================ */

void cymatic_simulate_wave(double* output, size_t len,
                           const uint64_t* primes, size_t num_primes,
                           uint32_t shape_symmetry) {
    if (!output || !primes || len == 0 || num_primes == 0) return;
    
    // Initialize output
    for (size_t i = 0; i < len; i++) {
        output[i] = 0.0;
    }
    
    // Generate wave pattern: Σ [sin(p[i] * x) + cos(p[i+1] * x)]
    for (size_t i = 0; i < num_primes; i++) {
        double freq = (double)primes[i];
        
        for (size_t x = 0; x < len; x++) {
            double t = (double)x / (double)len;
            
            // Add sine component
            output[x] += math_sin(freq * t * 2.0 * MATH_PI);
            
            // Add cosine component from next prime
            if (i + 1 < num_primes) {
                double freq2 = (double)primes[i + 1];
                output[x] += math_cos(freq2 * t * 2.0 * MATH_PI);
            }
        }
    }
    
    // Apply geometric symmetry modulation
    if (shape_symmetry > 0) {
        cymatic_geometric_modulation(output, len, shape_symmetry);
    }
    
    // Normalize
    double max_val = 0.0;
    for (size_t i = 0; i < len; i++) {
        double abs_val = output[i];
        if (abs_val < 0.0) abs_val = -abs_val;
        if (abs_val > max_val) max_val = abs_val;
    }
    
    if (max_val > 0.0) {
        for (size_t i = 0; i < len; i++) {
            output[i] /= max_val;
        }
    }
}

bool cymatic_detect_alignment(const double* signal, size_t len,
                               const uint64_t* target_primes, size_t num_primes) {
    if (!signal || !target_primes || len == 0 || num_primes == 0) {
        return false;
    }
    
    size_t aligned_count = 0;
    
    for (size_t p = 0; p < num_primes; p++) {
        double freq = (double)target_primes[p];
        
        // Check for peak at this frequency using simple correlation
        double correlation = 0.0;
        for (size_t i = 0; i < len; i++) {
            double t = (double)i / (double)len;
            double expected = math_sin(freq * t * 2.0 * MATH_PI);
            correlation += signal[i] * expected;
        }
        
        // Normalize correlation
        correlation /= (double)len;
        
        // Consider aligned if correlation > 0.3
        if (correlation > 0.3 || correlation < -0.3) {
            aligned_count++;
        }
    }
    
    // Return true if >50% of primes show alignment
    return (aligned_count * 2 > num_primes);
}

double cymatic_resonance_score(const double* signal, size_t len,
                                const uint64_t* primes, size_t num_primes) {
    if (!signal || !primes || len == 0 || num_primes == 0) {
        return 0.0;
    }
    
    double total_resonance = 0.0;
    
    for (size_t p = 0; p < num_primes; p++) {
        double freq = (double)primes[p];
        
        // Measure signal strength at this frequency
        double real = 0.0;
        double imag = 0.0;
        
        for (size_t i = 0; i < len; i++) {
            double t = (double)i / (double)len;
            double angle = freq * t * 2.0 * MATH_PI;
            real += signal[i] * math_cos(angle);
            imag += signal[i] * math_sin(angle);
        }
        
        // Power at this frequency
        double power = (real * real + imag * imag) / (double)(len * len);
        total_resonance += power;
    }
    
    // Normalize by number of primes
    return total_resonance / (double)num_primes;
}

size_t cymatic_find_peaks(const double* signal, size_t len,
                          size_t* peak_indices, size_t max_peaks,
                          double threshold) {
    if (!signal || !peak_indices || len < 3 || max_peaks == 0) {
        return 0;
    }
    
    size_t num_peaks = 0;
    
    // Find local maxima
    for (size_t i = 1; i < len - 1 && num_peaks < max_peaks; i++) {
        // Check if this is a local maximum
        if (signal[i] > signal[i-1] && signal[i] > signal[i+1]) {
            // Check if it exceeds threshold
            if (signal[i] >= threshold) {
                peak_indices[num_peaks] = i;
                num_peaks++;
            }
        }
    }
    
    return num_peaks;
}

void cymatic_geometric_modulation(double* signal, size_t len, uint32_t shape_symmetry) {
    if (!signal || len == 0 || shape_symmetry == 0) return;
    
    // Apply modulation based on geometric symmetry
    for (size_t i = 0; i < len; i++) {
        double t = (double)i / (double)len;
        
        // Calculate symmetry angle
        double angle = t * (double)shape_symmetry * MATH_TWO_PI;
        
        // Apply golden ratio scaling
        double scale = 1.0 + 0.1 * math_cos(angle / MATH_PHI);
        
        signal[i] *= scale;
    }
}

/* ============================================================================
 * Harmonic FFT Analysis (from Python prototype)
 * ============================================================================ */

bool cymatic_harmonic_alignment(const double* signal, size_t len,
                                const uint64_t* target_primes, size_t num_primes,
                                uint32_t alignment_threshold) {
    if (!signal || !target_primes || len == 0 || num_primes == 0) {
        return false;
    }
    
    // Perform FFT analysis (using DFT for now)
    double* fft_magnitudes = (double*)malloc(len * sizeof(double));
    if (!fft_magnitudes) {
        return false;
    }
    
    // Calculate FFT magnitudes
    for (size_t k = 0; k < len; k++) {
        double real = 0.0;
        double imag = 0.0;
        
        for (size_t n = 0; n < len; n++) {
            double angle = -MATH_TWO_PI * (double)k * (double)n / (double)len;
            real += signal[n] * math_cos(angle);
            imag += signal[n] * math_sin(angle);
        }
        
        fft_magnitudes[k] = math_sqrt(real * real + imag * imag);
    }
    
    // Find top N peaks
    size_t* peak_indices = (size_t*)malloc(num_primes * sizeof(size_t));
    if (!peak_indices) {
        free(fft_magnitudes);
        return false;
    }
    
    size_t num_peaks = cymatic_find_top_peaks(fft_magnitudes, len, 
                                               peak_indices, num_primes);
    
    // Check alignment with target primes
    size_t aligned_count = 0;
    
    for (size_t i = 0; i < num_peaks; i++) {
        size_t peak_idx = peak_indices[i];
        
        for (size_t p = 0; p < num_primes; p++) {
            // Check if peak index aligns with target prime frequency
            size_t expected_idx = target_primes[p] % len;
            
            if (peak_idx >= expected_idx - alignment_threshold &&
                peak_idx <= expected_idx + alignment_threshold) {
                aligned_count++;
                break;
            }
        }
    }
    
    free(fft_magnitudes);
    free(peak_indices);
    
    // Return true if >50% of primes show alignment
    return (aligned_count * 2 > num_primes);
}

size_t cymatic_find_top_peaks(const double* fft_result, size_t len,
                              size_t* peak_indices, size_t N) {
    if (!fft_result || !peak_indices || len == 0 || N == 0) {
        return 0;
    }
    
    // Create array of (index, magnitude) pairs
    typedef struct {
        size_t index;
        double magnitude;
    } Peak;
    
    Peak* peaks = (Peak*)malloc(len * sizeof(Peak));
    if (!peaks) {
        return 0;
    }
    
    for (size_t i = 0; i < len; i++) {
        peaks[i].index = i;
        peaks[i].magnitude = fft_result[i];
    }
    
    // Sort by magnitude (descending) using simple selection sort
    for (size_t i = 0; i < N && i < len; i++) {
        size_t max_idx = i;
        double max_mag = peaks[i].magnitude;
        
        for (size_t j = i + 1; j < len; j++) {
            if (peaks[j].magnitude > max_mag) {
                max_mag = peaks[j].magnitude;
                max_idx = j;
            }
        }
        
        // Swap
        if (max_idx != i) {
            Peak temp = peaks[i];
            peaks[i] = peaks[max_idx];
            peaks[max_idx] = temp;
        }
        
        peak_indices[i] = peaks[i].index;
    }
    
    size_t found = (N < len) ? N : len;
    free(peaks);
    
    return found;
}
