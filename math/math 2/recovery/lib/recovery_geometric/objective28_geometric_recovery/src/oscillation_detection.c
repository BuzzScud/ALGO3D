/**
 * @file oscillation_detection.c
 * @brief Oscillation Detection Implementation
 * 
 * Implements FFT-based oscillation detection using PURE crystalline math.
 * NO math.h - uses ONLY prime_* functions.
 */

#include "../include/oscillation_detection.h"
#include "../include/ecdlp_integration.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "math/arithmetic.h"
#include "math/transcendental.h"

// Mathematical constants (computed using crystalline math)
#define PI 3.14159265358979323846
#define TWO_PI (2.0 * PI)

// Inline helper for absolute value (since prime_fabs may not be exported)
static inline double inline_fabs(double x) {
    return (x < 0.0) ? -x : x;
}

// ============================================================================
// FFT OPERATIONS (PURE CRYSTALLINE MATH)
// ============================================================================

void dft_crystalline(const double* input, Complex* output, uint32_t n) {
    if (!input || !output || n == 0) return;
    
    for (uint32_t k = 0; k < n; k++) {
        output[k].real = 0.0;
        output[k].imag = 0.0;
        
        for (uint32_t t = 0; t < n; t++) {
            double angle = -TWO_PI * (double)k * (double)t / (double)n;
            
            // Use prime_cos and prime_sin (NO math.h)
            double cos_val = math_cos(angle);
            double sin_val = math_sin(angle);
            
            output[k].real += input[t] * cos_val;
            output[k].imag += input[t] * sin_val;
        }
    }
}

bool fft_crystalline(const double* input, Complex* output, uint32_t n) {
    if (!input || !output || n == 0) return false;
    
    // Check if n is power of 2
    if (!is_power_of_2(n)) return false;
    
    // Base case: n = 1
    if (n == 1) {
        output[0].real = input[0];
        output[0].imag = 0.0;
        return true;
    }
    
    // Allocate temporary arrays
    double* even_input = (double*)malloc((n/2) * sizeof(double));
    double* odd_input = (double*)malloc((n/2) * sizeof(double));
    Complex* even_output = (Complex*)malloc((n/2) * sizeof(Complex));
    Complex* odd_output = (Complex*)malloc((n/2) * sizeof(Complex));
    
    if (!even_input || !odd_input || !even_output || !odd_output) {
        if (even_input) free(even_input);
        if (odd_input) free(odd_input);
        if (even_output) free(even_output);
        if (odd_output) free(odd_output);
        return false;
    }
    
    // Split into even and odd
    for (uint32_t i = 0; i < n/2; i++) {
        even_input[i] = input[2*i];
        odd_input[i] = input[2*i + 1];
    }
    
    // Recursive FFT
    if (!fft_crystalline(even_input, even_output, n/2) ||
        !fft_crystalline(odd_input, odd_output, n/2)) {
        free(even_input);
        free(odd_input);
        free(even_output);
        free(odd_output);
        return false;
    }
    
    // Combine results
    for (uint32_t k = 0; k < n/2; k++) {
        double angle = -TWO_PI * (double)k / (double)n;
        
        // Twiddle factor using prime_cos and prime_sin
        double cos_val = math_cos(angle);
        double sin_val = math_sin(angle);
        
        // Complex multiplication: twiddle * odd_output[k]
        double t_real = cos_val * odd_output[k].real - sin_val * odd_output[k].imag;
        double t_imag = cos_val * odd_output[k].imag + sin_val * odd_output[k].real;
        
        // Butterfly operation
        output[k].real = even_output[k].real + t_real;
        output[k].imag = even_output[k].imag + t_imag;
        
        output[k + n/2].real = even_output[k].real - t_real;
        output[k + n/2].imag = even_output[k].imag - t_imag;
    }
    
    free(even_input);
    free(odd_input);
    free(even_output);
    free(odd_output);
    
    return true;
}

bool ifft_crystalline(const Complex* input, double* output, uint32_t n) {
    if (!input || !output || n == 0) return false;
    if (!is_power_of_2(n)) return false;
    
    // Conjugate input
    Complex* conjugated = (Complex*)malloc(n * sizeof(Complex));
    if (!conjugated) return false;
    
    for (uint32_t i = 0; i < n; i++) {
        conjugated[i].real = input[i].real;
        conjugated[i].imag = -input[i].imag;
    }
    
    // Forward FFT on conjugated input
    Complex* fft_output = (Complex*)malloc(n * sizeof(Complex));
    if (!fft_output) {
        free(conjugated);
        return false;
    }
    
    // Convert to double array for FFT
    double* real_input = (double*)malloc(n * sizeof(double));
    if (!real_input) {
        free(conjugated);
        free(fft_output);
        return false;
    }
    
    for (uint32_t i = 0; i < n; i++) {
        real_input[i] = conjugated[i].real;
    }
    
    bool success = fft_crystalline(real_input, fft_output, n);
    
    if (success) {
        // Conjugate and scale
        for (uint32_t i = 0; i < n; i++) {
            output[i] = fft_output[i].real / (double)n;
        }
    }
    
    free(conjugated);
    free(fft_output);
    free(real_input);
    
    return success;
}

void compute_power_spectrum(const Complex* fft_output, double* power_spectrum, uint32_t n) {
    if (!fft_output || !power_spectrum || n == 0) return;
    
    for (uint32_t i = 0; i < n; i++) {
        double real = fft_output[i].real;
        double imag = fft_output[i].imag;
        power_spectrum[i] = real * real + imag * imag;
    }
}

double find_dominant_frequency(const double* power_spectrum, uint32_t n, double sampling_rate) {
    if (!power_spectrum || n == 0) return 0.0;
    
    // Find index of maximum power (skip DC component at index 0)
    uint32_t max_idx = 1;
    double max_power = power_spectrum[1];
    
    for (uint32_t i = 2; i < n/2; i++) {  // Only check first half (Nyquist)
        if (power_spectrum[i] > max_power) {
            max_power = power_spectrum[i];
            max_idx = i;
        }
    }
    
    // Convert index to frequency
    double frequency = (double)max_idx * sampling_rate / (double)n;
    
    return frequency;
}

// ============================================================================
// TRAJECTORY SAMPLING
// ============================================================================

ECTrajectory* sample_ec_trajectory(
    const void* instance_ptr,
    uint64_t k_start,
    uint64_t k_step,
    uint32_t num_samples
) {
    if (!instance_ptr || num_samples == 0) return NULL;
    
    const ECDLPInstance* instance = (const ECDLPInstance*)instance_ptr;
    
    ECTrajectory* trajectory = (ECTrajectory*)malloc(sizeof(ECTrajectory));
    if (!trajectory) return NULL;
    
    trajectory->num_samples = num_samples;
    trajectory->sampling_rate = 1.0;  // 1 sample per unit
    trajectory->k_start = k_start;
    trajectory->k_step = k_step;
    
    trajectory->samples = (ECTrajectorySample*)malloc(num_samples * sizeof(ECTrajectorySample));
    if (!trajectory->samples) {
        free(trajectory);
        return NULL;
    }
    
    // Sample trajectory
    for (uint32_t i = 0; i < num_samples; i++) {
        uint64_t k = k_start + i * k_step;
        trajectory->samples[i].k = k;
        
        // Compute k*G
        BIGNUM* k_bn = BN_new();
        BN_set_word(k_bn, k);
        
        EC_POINT* point = EC_POINT_new(instance->group);
        if (!EC_POINT_mul(instance->group, point, k_bn, NULL, NULL, NULL)) {
            BN_free(k_bn);
            EC_POINT_free(point);
            free_ec_trajectory(trajectory);
            return NULL;
        }
        
        // Get coordinates
        ECPointCoords* coords = ec_get_point_coords(instance->group, point);
        if (!coords) {
            BN_free(k_bn);
            EC_POINT_free(point);
            free_ec_trajectory(trajectory);
            return NULL;
        }
        
        // Embed into lattice
        ECLatticeEmbedding embedding;
        if (!lattice_embed_ec_point(coords, &embedding)) {
            ec_free_point_coords(coords);
            BN_free(k_bn);
            EC_POINT_free(point);
            free_ec_trajectory(trajectory);
            return NULL;
        }
        
        // Copy to trajectory
        for (int j = 0; j < 15; j++) {
            trajectory->samples[i].coords[j] = embedding.coords[j];
        }
        trajectory->samples[i].magnitude = embedding.magnitude;
        trajectory->samples[i].angle = embedding.angle;
        
        ec_free_point_coords(coords);
        BN_free(k_bn);
        EC_POINT_free(point);
    }
    
    return trajectory;
}

void free_ec_trajectory(ECTrajectory* trajectory) {
    if (!trajectory) return;
    if (trajectory->samples) free(trajectory->samples);
    free(trajectory);
}

void extract_dimension(const ECTrajectory* trajectory, uint32_t dimension, double* output) {
    if (!trajectory || !output || dimension >= 15) return;
    
    for (uint32_t i = 0; i < trajectory->num_samples; i++) {
        output[i] = trajectory->samples[i].coords[dimension];
    }
}

// ============================================================================
// OSCILLATION DETECTION
// ============================================================================

OscillationMap* detect_oscillations(const ECTrajectory* trajectory) {
    if (!trajectory || trajectory->num_samples == 0) return NULL;
    
    OscillationMap* map = (OscillationMap*)malloc(sizeof(OscillationMap));
    if (!map) return NULL;
    
    map->num_dimensions = 15;
    map->signatures = (OscillationSignature*)malloc(15 * sizeof(OscillationSignature));
    map->cross_correlations = (double*)malloc(15 * 15 * sizeof(double));
    
    if (!map->signatures || !map->cross_correlations) {
        if (map->signatures) free(map->signatures);
        if (map->cross_correlations) free(map->cross_correlations);
        free(map);
        return NULL;
    }
    
    // Pad to next power of 2 for FFT
    uint32_t fft_size = next_power_of_2(trajectory->num_samples);
    double* padded_input = (double*)calloc(fft_size, sizeof(double));
    Complex* fft_output = (Complex*)malloc(fft_size * sizeof(Complex));
    double* power_spectrum = (double*)malloc(fft_size * sizeof(double));
    
    if (!padded_input || !fft_output || !power_spectrum) {
        if (padded_input) free(padded_input);
        if (fft_output) free(fft_output);
        if (power_spectrum) free(power_spectrum);
        free_oscillation_map(map);
        return NULL;
    }
    
    // Analyze each dimension
    map->global_amplitude = 0.0;
    int converging_count = 0;
    
    for (uint32_t d = 0; d < 15; d++) {
        // Extract dimension
        extract_dimension(trajectory, d, padded_input);
        
        // Compute FFT
        if (!fft_crystalline(padded_input, fft_output, fft_size)) {
            free(padded_input);
            free(fft_output);
            free(power_spectrum);
            free_oscillation_map(map);
            return NULL;
        }
        
        // Compute power spectrum
        compute_power_spectrum(fft_output, power_spectrum, fft_size);
        
        // Find dominant frequency
        double frequency = find_dominant_frequency(power_spectrum, fft_size, trajectory->sampling_rate);
        
        // Compute amplitude (from power spectrum peak)
        double amplitude = math_sqrt(power_spectrum[(uint32_t)(frequency * fft_size / trajectory->sampling_rate)]);
        
        // Compute phase (from FFT output)
        uint32_t freq_idx = (uint32_t)(frequency * fft_size / trajectory->sampling_rate);
        double phase = complex_phase(&fft_output[freq_idx]);
        
        // Fill signature
        map->signatures[d].dimension = d;
        map->signatures[d].frequency = frequency;
        map->signatures[d].amplitude = amplitude;
        map->signatures[d].phase = phase;
        map->signatures[d].period = (frequency > 0.0) ? (uint64_t)(1.0 / frequency) : 0;
        
        // Classify oscillation
        classify_oscillation(&map->signatures[d], trajectory, d);
        
        if (map->signatures[d].is_stable || map->signatures[d].convergence_rate > 0.0) {
            converging_count++;
        }
        
        map->global_amplitude += amplitude;
    }
    
    map->global_amplitude /= 15.0;
    
    // Compute cross-correlations
    for (uint32_t i = 0; i < 15; i++) {
        for (uint32_t j = 0; j < 15; j++) {
            map->cross_correlations[i * 15 + j] = compute_cross_correlation(trajectory, i, j);
        }
    }
    
    // Determine global convergence
    map->is_converging = (converging_count >= 10);  // At least 10/15 dimensions converging
    map->iterations_to_convergence = estimate_convergence(map);
    
    free(padded_input);
    free(fft_output);
    free(power_spectrum);
    
    return map;
}

void free_oscillation_map(OscillationMap* map) {
    if (!map) return;
    if (map->signatures) free(map->signatures);
    if (map->cross_correlations) free(map->cross_correlations);
    free(map);
}

double compute_cross_correlation(const ECTrajectory* trajectory, uint32_t dim1, uint32_t dim2) {
    if (!trajectory || dim1 >= 15 || dim2 >= 15) return 0.0;
    
    // Extract both dimensions
    double* series1 = (double*)malloc(trajectory->num_samples * sizeof(double));
    double* series2 = (double*)malloc(trajectory->num_samples * sizeof(double));
    
    if (!series1 || !series2) {
        if (series1) free(series1);
        if (series2) free(series2);
        return 0.0;
    }
    
    extract_dimension(trajectory, dim1, series1);
    extract_dimension(trajectory, dim2, series2);
    
    // Compute means
    double mean1 = 0.0, mean2 = 0.0;
    for (uint32_t i = 0; i < trajectory->num_samples; i++) {
        mean1 += series1[i];
        mean2 += series2[i];
    }
    mean1 /= trajectory->num_samples;
    mean2 /= trajectory->num_samples;
    
    // Compute correlation
    double numerator = 0.0;
    double denom1 = 0.0, denom2 = 0.0;
    
    for (uint32_t i = 0; i < trajectory->num_samples; i++) {
        double diff1 = series1[i] - mean1;
        double diff2 = series2[i] - mean2;
        numerator += diff1 * diff2;
        denom1 += diff1 * diff1;
        denom2 += diff2 * diff2;
    }
    
    double correlation = 0.0;
    if (denom1 > 0.0 && denom2 > 0.0) {
        correlation = numerator / math_sqrt(denom1 * denom2);
    }
    
    free(series1);
    free(series2);
    
    return correlation;
}

void classify_oscillation(OscillationSignature* signature, const ECTrajectory* trajectory, uint32_t dimension) {
    if (!signature || !trajectory) return;
    
    // Extract dimension for trend analysis
    double* series = (double*)malloc(trajectory->num_samples * sizeof(double));
    if (!series) return;
    
    extract_dimension(trajectory, dimension, series);
    
    // Compute trend (linear regression slope)
    double sum_x = 0.0, sum_y = 0.0, sum_xy = 0.0, sum_xx = 0.0;
    for (uint32_t i = 0; i < trajectory->num_samples; i++) {
        double x = (double)i;
        double y = series[i];
        sum_x += x;
        sum_y += y;
        sum_xy += x * y;
        sum_xx += x * x;
    }
    
    double n = (double)trajectory->num_samples;
    double slope = (n * sum_xy - sum_x * sum_y) / (n * sum_xx - sum_x * sum_x);
    
    // Classify based on amplitude and trend
    if (signature->amplitude < 0.01) {
        // Very small amplitude -> stable
        signature->is_stable = true;
        signature->convergence_rate = 0.0;
    } else if (inline_fabs(slope) < 0.001) {
        // Flat trend -> stable oscillation
        signature->is_stable = true;
        signature->convergence_rate = 0.0;
    } else if (slope < 0.0) {
        // Negative slope -> converging
        signature->is_stable = false;
        signature->convergence_rate = -slope;  // Positive convergence rate
    } else {
        // Positive slope -> diverging (unstable)
        signature->is_stable = false;
        signature->convergence_rate = -slope;  // Negative convergence rate
    }
    
    free(series);
}

uint64_t estimate_convergence(const OscillationMap* map) {
    if (!map || !map->is_converging) return 0;
    
    // Find slowest converging dimension
    double slowest_rate = 1e10;
    for (uint32_t i = 0; i < map->num_dimensions; i++) {
        if (map->signatures[i].convergence_rate > 0.0 && 
            map->signatures[i].convergence_rate < slowest_rate) {
            slowest_rate = map->signatures[i].convergence_rate;
        }
    }
    
    if (slowest_rate < 1e-10) return 0;
    
    // Estimate iterations: amplitude / convergence_rate
    uint64_t iterations = (uint64_t)(map->global_amplitude / slowest_rate);
    
    return iterations;
}

// ============================================================================
// OSCILLATION-GUIDED SEARCH
// ============================================================================

int generate_k_candidates_from_oscillations(
    const void* instance_ptr,
    const OscillationMap* map,
    const void* Q_embedding_ptr,
    void** candidates_ptr,
    int max_candidates
) {
    // TODO: Implement oscillation-guided candidate generation
    // This will use oscillation patterns to predict k values
    
    // For now, return 0 (not implemented)
    (void)instance_ptr;
    (void)map;
    (void)Q_embedding_ptr;
    (void)candidates_ptr;
    (void)max_candidates;
    
    return 0;
}

uint64_t predict_next_k(const OscillationMap* map, uint64_t current_k) {
    if (!map) return current_k + 1;
    
    // Use dominant frequency to predict next k
    // This is a simplified prediction
    double avg_frequency = 0.0;
    for (uint32_t i = 0; i < map->num_dimensions; i++) {
        avg_frequency += map->signatures[i].frequency;
    }
    avg_frequency /= map->num_dimensions;
    
    // Predict step size from frequency
    uint64_t step = (uint64_t)(1.0 / avg_frequency);
    if (step == 0) step = 1;
    
    return current_k + step;
}

double compute_oscillation_score(
    const void* instance_ptr,
    const void* k_candidate_ptr,
    const OscillationMap* map
) {
    // TODO: Implement oscillation scoring
    // This will score how well a k candidate fits the oscillation pattern
    
    (void)instance_ptr;
    (void)k_candidate_ptr;
    (void)map;
    
    return 0.5;  // Placeholder
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

bool is_power_of_2(uint32_t n) {
    return n > 0 && (n & (n - 1)) == 0;
}

uint32_t next_power_of_2(uint32_t n) {
    if (n == 0) return 1;
    
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;
    
    return n;
}

double complex_magnitude(const Complex* z) {
    if (!z) return 0.0;
    return math_sqrt(z->real * z->real + z->imag * z->imag);
}

double complex_phase(const Complex* z) {
    if (!z) return 0.0;
    return prime_atan2(z->imag, z->real);
}

void print_oscillation_map(const OscillationMap* map) {
    if (!map) return;
    
    printf("Oscillation Map:\n");
    printf("  Dimensions: %u\n", map->num_dimensions);
    printf("  Global amplitude: %.6f\n", map->global_amplitude);
    printf("  Converging: %s\n", map->is_converging ? "YES" : "NO");
    if (map->is_converging) {
        printf("  Iterations to convergence: %lu\n", map->iterations_to_convergence);
    }
    printf("\n");
    
    for (uint32_t i = 0; i < map->num_dimensions; i++) {
        print_oscillation_signature(&map->signatures[i]);
    }
}

void print_oscillation_signature(const OscillationSignature* signature) {
    if (!signature) return;
    
    printf("  Dimension %u:\n", signature->dimension);
    printf("    Frequency: %.6f Hz\n", signature->frequency);
    printf("    Amplitude: %.6f\n", signature->amplitude);
    printf("    Phase: %.6f rad\n", signature->phase);
    printf("    Period: %lu samples\n", signature->period);
    printf("    Stable: %s\n", signature->is_stable ? "YES" : "NO");
    if (signature->convergence_rate != 0.0) {
        printf("    Convergence rate: %.6f\n", signature->convergence_rate);
    }
}
