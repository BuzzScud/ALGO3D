/**
 * @file oscillation_detection.h
 * @brief Oscillation Detection for ECDLP Recovery
 * 
 * Phase 4: Detect oscillations in EC point trajectories using FFT.
 * Uses PURE crystalline mathematics (prime_sin, prime_cos, prime_sqrt).
 * 
 * CRITICAL: NO math.h - only prime_* functions from crystalline library.
 */

#ifndef OSCILLATION_DETECTION_H
#define OSCILLATION_DETECTION_H

#include <stdint.h>
#include <stdbool.h>
#include <openssl/ec.h>
#include <openssl/bn.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Complex number (for FFT)
 * 
 * Represents a + bi
 */
typedef struct {
    double real;
    double imag;
} Complex;

/**
 * Oscillation signature
 * 
 * Describes oscillation in a single dimension
 */
typedef struct {
    uint32_t dimension;         // Which dimension (0-14 for 15D lattice)
    double frequency;           // Dominant frequency (Hz)
    double amplitude;           // Oscillation amplitude
    double phase;               // Phase offset (radians)
    uint64_t period;            // Period in samples
    bool is_stable;             // Stable vs unstable
    double convergence_rate;    // Rate of convergence (if converging)
} OscillationSignature;

/**
 * Oscillation map
 * 
 * Complete oscillation analysis across all dimensions
 */
typedef struct {
    uint32_t num_dimensions;                    // Number of dimensions (15 for lattice)
    OscillationSignature* signatures;           // [num_dimensions]
    double* cross_correlations;                 // [num_dimensions × num_dimensions]
    bool is_converging;                         // Global convergence
    uint64_t iterations_to_convergence;         // Estimated iterations
    double global_amplitude;                    // Overall oscillation amplitude
} OscillationMap;

/**
 * EC trajectory sample
 * 
 * Single sample in EC point trajectory
 */
typedef struct {
    uint64_t k;                 // Scalar value
    double coords[15];          // 15D lattice coordinates
    double magnitude;           // Vector magnitude
    double angle;               // Angle in lattice space
} ECTrajectorySample;

/**
 * EC trajectory
 * 
 * Sequence of EC point samples
 */
typedef struct {
    uint32_t num_samples;               // Number of samples
    ECTrajectorySample* samples;        // [num_samples]
    double sampling_rate;               // Samples per unit
    uint64_t k_start;                   // Starting k value
    uint64_t k_step;                    // Step size
} ECTrajectory;

// ============================================================================
// FFT OPERATIONS (PURE CRYSTALLINE MATH)
// ============================================================================

/**
 * Discrete Fourier Transform (DFT)
 * 
 * Computes DFT using PURE crystalline math (prime_sin, prime_cos)
 * 
 * Complexity: O(n²)
 * 
 * @param input Input signal [n]
 * @param output Output spectrum [n] (complex)
 * @param n Number of samples
 */
void dft_crystalline(const double* input, Complex* output, uint32_t n);

/**
 * Fast Fourier Transform (FFT)
 * 
 * Computes FFT using PURE crystalline math
 * Cooley-Tukey radix-2 algorithm
 * 
 * Complexity: O(n log n)
 * 
 * REQUIREMENT: n must be power of 2
 * 
 * @param input Input signal [n]
 * @param output Output spectrum [n] (complex)
 * @param n Number of samples (must be power of 2)
 * @return true on success, false if n not power of 2
 */
bool fft_crystalline(const double* input, Complex* output, uint32_t n);

/**
 * Inverse FFT
 * 
 * Computes inverse FFT using PURE crystalline math
 * 
 * @param input Input spectrum [n] (complex)
 * @param output Output signal [n]
 * @param n Number of samples (must be power of 2)
 * @return true on success, false if n not power of 2
 */
bool ifft_crystalline(const Complex* input, double* output, uint32_t n);

/**
 * Power spectrum
 * 
 * Computes power spectrum from FFT output
 * Power[i] = |FFT[i]|² = real² + imag²
 * 
 * @param fft_output FFT output [n] (complex)
 * @param power_spectrum Output power spectrum [n]
 * @param n Number of samples
 */
void compute_power_spectrum(const Complex* fft_output, double* power_spectrum, uint32_t n);

/**
 * Find dominant frequency
 * 
 * Finds the frequency with maximum power
 * 
 * @param power_spectrum Power spectrum [n]
 * @param n Number of samples
 * @param sampling_rate Sampling rate (samples per unit)
 * @return Dominant frequency (Hz)
 */
double find_dominant_frequency(const double* power_spectrum, uint32_t n, double sampling_rate);

// ============================================================================
// TRAJECTORY SAMPLING
// ============================================================================

/**
 * Sample EC trajectory
 * 
 * Samples EC points k*G for k = k_start, k_start+k_step, ..., k_start+n*k_step
 * and embeds each into 15D lattice
 * 
 * @param instance ECDLP instance
 * @param k_start Starting k value
 * @param k_step Step size
 * @param num_samples Number of samples
 * @return EC trajectory, or NULL on failure
 */
ECTrajectory* sample_ec_trajectory(
    const void* instance,  // ECDLPInstance*
    uint64_t k_start,
    uint64_t k_step,
    uint32_t num_samples
);

/**
 * Free EC trajectory
 * 
 * @param trajectory Trajectory to free
 */
void free_ec_trajectory(ECTrajectory* trajectory);

/**
 * Extract dimension from trajectory
 * 
 * Extracts time series for a single dimension
 * 
 * @param trajectory EC trajectory
 * @param dimension Dimension to extract (0-14)
 * @param output Output time series [num_samples]
 */
void extract_dimension(const ECTrajectory* trajectory, uint32_t dimension, double* output);

// ============================================================================
// OSCILLATION DETECTION
// ============================================================================

/**
 * Detect oscillations in trajectory
 * 
 * Analyzes EC trajectory and detects oscillations in each dimension
 * 
 * Algorithm:
 * 1. For each dimension:
 *    - Extract time series
 *    - Compute FFT
 *    - Find dominant frequency
 *    - Compute amplitude and phase
 *    - Classify as stable/unstable/converging
 * 2. Compute cross-correlations between dimensions
 * 3. Determine global convergence
 * 
 * @param trajectory EC trajectory
 * @return Oscillation map, or NULL on failure
 */
OscillationMap* detect_oscillations(const ECTrajectory* trajectory);

/**
 * Free oscillation map
 * 
 * @param map Oscillation map to free
 */
void free_oscillation_map(OscillationMap* map);

/**
 * Compute cross-correlation
 * 
 * Computes cross-correlation between two dimensions
 * 
 * @param trajectory EC trajectory
 * @param dim1 First dimension
 * @param dim2 Second dimension
 * @return Cross-correlation coefficient [-1, 1]
 */
double compute_cross_correlation(const ECTrajectory* trajectory, uint32_t dim1, uint32_t dim2);

/**
 * Classify oscillation
 * 
 * Classifies oscillation as stable, unstable, or converging
 * 
 * @param signature Oscillation signature
 * @param trajectory EC trajectory (for trend analysis)
 * @param dimension Dimension being analyzed
 */
void classify_oscillation(OscillationSignature* signature, const ECTrajectory* trajectory, uint32_t dimension);

/**
 * Estimate convergence
 * 
 * Estimates number of iterations until convergence
 * 
 * @param map Oscillation map
 * @return Estimated iterations, or 0 if not converging
 */
uint64_t estimate_convergence(const OscillationMap* map);

// ============================================================================
// OSCILLATION-GUIDED SEARCH
// ============================================================================

/**
 * Generate k candidates from oscillations
 * 
 * Uses oscillation patterns to generate k candidates
 * 
 * Algorithm:
 * 1. Identify dimensions with strong oscillations
 * 2. Use frequency/phase to predict next k values
 * 3. Use amplitude to estimate search radius
 * 4. Generate candidates around predicted values
 * 5. Score candidates by oscillation fit
 * 
 * @param instance ECDLP instance
 * @param map Oscillation map
 * @param Q_embedding Target lattice embedding
 * @param candidates Output array for k candidates
 * @param max_candidates Maximum number of candidates
 * @return Number of candidates generated
 */
int generate_k_candidates_from_oscillations(
    const void* instance,  // ECDLPInstance*
    const OscillationMap* map,
    const void* Q_embedding,  // ECLatticeEmbedding*
    void** candidates,  // BIGNUM**
    int max_candidates
);

/**
 * Predict next k value
 * 
 * Predicts next k value based on oscillation pattern
 * 
 * @param map Oscillation map
 * @param current_k Current k value
 * @return Predicted next k value
 */
uint64_t predict_next_k(const OscillationMap* map, uint64_t current_k);

/**
 * Compute oscillation score
 * 
 * Scores how well a k candidate fits the oscillation pattern
 * 
 * @param instance ECDLP instance
 * @param k_candidate Candidate k value
 * @param map Oscillation map
 * @return Score [0, 1] (higher is better)
 */
double compute_oscillation_score(
    const void* instance,  // ECDLPInstance*
    const void* k_candidate,  // BIGNUM*
    const OscillationMap* map
);

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * Check if n is power of 2
 * 
 * @param n Number to check
 * @return true if power of 2, false otherwise
 */
bool is_power_of_2(uint32_t n);

/**
 * Next power of 2
 * 
 * Finds smallest power of 2 >= n
 * 
 * @param n Input number
 * @return Next power of 2
 */
uint32_t next_power_of_2(uint32_t n);

/**
 * Complex magnitude
 * 
 * Computes |z| = sqrt(real² + imag²) using prime_sqrt
 * 
 * @param z Complex number
 * @return Magnitude
 */
double complex_magnitude(const Complex* z);

/**
 * Complex phase
 * 
 * Computes arg(z) = atan2(imag, real) using prime_atan2
 * 
 * @param z Complex number
 * @return Phase (radians)
 */
double complex_phase(const Complex* z);

/**
 * Print oscillation map
 * 
 * @param map Oscillation map
 */
void print_oscillation_map(const OscillationMap* map);

/**
 * Print oscillation signature
 * 
 * @param signature Oscillation signature
 */
void print_oscillation_signature(const OscillationSignature* signature);

#ifdef __cplusplus
}
#endif

#endif // OSCILLATION_DETECTION_H
