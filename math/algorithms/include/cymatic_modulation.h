#ifndef CYMATIC_MODULATION_H
#define CYMATIC_MODULATION_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * General Cymatic Frequency Modulation Algorithm
 * 
 * This is a GENERAL algorithm that can be used by any training system.
 * It applies cymatic frequency resonance to modulate gradients for smoother convergence.
 * 
 * Cymatic frequencies: 432 Hz, 528 Hz, 639 Hz, 741 Hz, 852 Hz, 963 Hz
 */

/**
 * Apply cymatic resonance modulation to gradients
 * 
 * @param gradients Gradient buffer [num_elements x element_dim]
 * @param symmetry_groups Symmetry group for each element [num_elements]
 * @param num_elements Number of elements (e.g., vocab size)
 * @param element_dim Dimension of each element (e.g., embedding dim)
 * @param training_step Current training step (for phase calculation)
 * @param cymatic_freqs Array of cymatic frequencies (Hz)
 * @param num_freqs Number of cymatic frequencies
 * @param dimensional_freqs Array of dimensional frequencies φᵢ
 * @param modulation_strength Modulation strength (0.0 to 1.0, typically 0.1)
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
);

/**
 * Compute harmonic series with golden ratio damping
 * 
 * Generates: f, 2f, 3f, ... × φ^(-i)
 * 
 * @param base_freq Base frequency (Hz)
 * @param num_harmonics Number of harmonics to generate
 * @param harmonics Output harmonic frequencies [num_harmonics]
 * @param phi Golden ratio for damping
 */
void compute_cymatic_harmonics(
    double base_freq,
    uint32_t num_harmonics,
    double* harmonics,
    double phi
);

/**
 * Analyze gradient spectrum (simple DFT)
 * 
 * @param gradients Gradient buffer [size]
 * @param size Gradient buffer size
 * @param spectrum Output power spectrum [size/2]
 * @param num_bins Number of frequency bins (size/2)
 */
void analyze_gradient_spectrum(
    const float* gradients,
    uint32_t size,
    double* spectrum,
    uint32_t num_bins
);

/* ============================================================================
 * Extended Cymatic Functions for Signal Processing
 * ============================================================================ */

/**
 * @brief Simulate cymatic wave pattern
 * 
 * Generates wave pattern: Σ [sin(p[i] * x) + cos(p[i+1] * x)]
 * 
 * @param output Output signal (pre-allocated)
 * @param len Signal length
 * @param primes Prime frequencies
 * @param num_primes Number of primes
 * @param shape_symmetry Geometric symmetry (12=dodeca, 20=icosa, 6=cube)
 */
void cymatic_simulate_wave(double* output, size_t len,
                           const uint64_t* primes, size_t num_primes,
                           uint32_t shape_symmetry);

/**
 * @brief Detect harmonic alignment
 * 
 * Checks if signal has frequency peaks aligned with target primes.
 * 
 * @param signal Input signal
 * @param len Signal length
 * @param target_primes Target prime frequencies
 * @param num_primes Number of target primes
 * @return true if >50% of primes show alignment
 */
bool cymatic_detect_alignment(const double* signal, size_t len,
                               const uint64_t* target_primes, size_t num_primes);

/**
 * @brief Calculate resonance score
 * 
 * Measures signal strength at prime frequencies.
 * 
 * @param signal Input signal
 * @param len Signal length
 * @param primes Prime frequencies
 * @param num_primes Number of primes
 * @return Resonance score (0.0 to 1.0)
 */
double cymatic_resonance_score(const double* signal, size_t len,
                                const uint64_t* primes, size_t num_primes);

/**
 * @brief Find frequency peaks
 * 
 * Detects local maxima in signal.
 * 
 * @param signal Input signal
 * @param len Signal length
 * @param peak_indices Output peak indices (pre-allocated)
 * @param max_peaks Maximum peaks to find
 * @param threshold Minimum peak height
 * @return Number of peaks found
 */
size_t cymatic_find_peaks(const double* signal, size_t len,
                          size_t* peak_indices, size_t max_peaks,
                          double threshold);

/**
 * @brief Apply geometric modulation
 * 
 * Modulates signal based on geometric symmetry.
 * 
 * @param signal Signal to modulate (modified in place)
 * @param len Signal length
 * @param shape_symmetry Symmetry order (12, 20, 6, etc.)
 */
void cymatic_geometric_modulation(double* signal, size_t len, uint32_t shape_symmetry);

/* ============================================================================
 * Harmonic FFT Analysis
 * ============================================================================ */

/**
 * @brief Detect harmonic alignment with target primes
 * 
 * Performs FFT analysis and checks if peaks align with target prime frequencies.
 * This is useful for validating that a signal contains expected harmonic content.
 * 
 * @param signal Input signal
 * @param len Signal length
 * @param target_primes Array of target prime frequencies
 * @param num_primes Number of target primes
 * @param alignment_threshold Threshold for considering alignment (default: 5)
 * @return true if harmonically aligned (>50% of primes show peaks)
 */
bool cymatic_harmonic_alignment(const double* signal, size_t len,
                                const uint64_t* target_primes, size_t num_primes,
                                uint32_t alignment_threshold);

/**
 * @brief Find top N frequency peaks in FFT result
 * 
 * Sorts FFT magnitude spectrum and returns indices of top N peaks.
 * 
 * @param fft_result FFT magnitude spectrum
 * @param len Spectrum length
 * @param peak_indices Output array for peak indices (pre-allocated, size N)
 * @param N Number of peaks to find
 * @return Number of peaks found
 */
size_t cymatic_find_top_peaks(const double* fft_result, size_t len,
                              size_t* peak_indices, size_t N);

#ifdef __cplusplus
}
#endif

#endif /* CYMATIC_MODULATION_H */
