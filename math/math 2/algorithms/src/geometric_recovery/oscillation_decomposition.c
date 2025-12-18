/**
 * oscillation_decomposition.c - FFT-based Oscillation Decomposition
 * 
 * Decomposes multi-frequency oscillations into individual components.
 * 
 * PHASE 2: Migrated to NEW math library
 * - Replaced <complex.h> with math/complex.h
 * - Replaced double complex with MathComplex
 * - Replaced creal/cimag/cabs/carg with MathComplex operations
 * - Replaced I with math_complex_from_cartesian(0, 1)
 */

#include "../include/oscillation_decomposition.h"
#include "math/constants.h"
#include "math/complex.h"        // PHASE 2: NEW math library complex numbers
#include "math/constants.h"
#include "math/transcendental.h" // PHASE 2: For math_cos, math_sin, math_sqrt
#include "math/constants.h"
#include "math/types.h"          // PHASE 2: For MATH_PI
#include "math/constants.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Simple FFT implementation (Cooley-Tukey radix-2)
 */
static void fft(MathComplex* x, int n, int stride) {
    if (n == 1) return;
    
    int half = n / 2;
    
    // Recursively compute FFT of even and odd elements
    fft(x, half, stride * 2);
    fft(x + stride, half, stride * 2);
    
    // Combine results
    for (int k = 0; k < half; k++) {
        MathComplex even = x[k * stride * 2];
        MathComplex odd = x[(k * stride * 2) + stride];
        
        double angle = -2.0 * MATH_PI * k / n;
        MathComplex twiddle = math_complex_from_cartesian(math_cos(angle), math_sin(angle));
        MathComplex t = math_complex_mul(twiddle, odd);
        
        x[k * stride] = math_complex_add(even, t);
        x[(k + half) * stride] = math_complex_sub(even, t);
    }
}

/**
 * Compute power spectrum from FFT
 */
static void compute_power_spectrum(
    const MathComplex* fft_result,
    double* power_spectrum,
    int n
) {
    for (int i = 0; i < n / 2; i++) {
        power_spectrum[i] = math_complex_magnitude(fft_result[i]);
    }
}

/**
 * Find next power of 2
 */
static int next_power_of_2(int n) {
    int power = 1;
    while (power < n) power *= 2;
    return power;
}

OscillationDecomposer* create_oscillation_decomposer(
    const double* signal,
    int signal_length,
    int max_components
) {
    OscillationDecomposer* decomposer = (OscillationDecomposer*)calloc(1, sizeof(OscillationDecomposer));
    if (!decomposer) return NULL;
    
    decomposer->original_signal = (double*)malloc(signal_length * sizeof(double));
    decomposer->residual = (double*)malloc(signal_length * sizeof(double));
    decomposer->components = (OscillationComponent*)calloc(max_components, sizeof(OscillationComponent));
    
    if (!decomposer->original_signal || !decomposer->residual || !decomposer->components) {
        free_oscillation_decomposer(decomposer);
        return NULL;
    }
    
    memcpy(decomposer->original_signal, signal, signal_length * sizeof(double));
    memcpy(decomposer->residual, signal, signal_length * sizeof(double));
    
    decomposer->signal_length = signal_length;
    decomposer->max_components = max_components;
    decomposer->num_components = 0;
    
    return decomposer;
}

void free_oscillation_decomposer(OscillationDecomposer* decomposer) {
    if (!decomposer) return;
    
    if (decomposer->original_signal) free(decomposer->original_signal);
    if (decomposer->residual) free(decomposer->residual);
    
    if (decomposer->components) {
        for (int i = 0; i < decomposer->num_components; i++) {
            if (decomposer->components[i].reconstructed) {
                free(decomposer->components[i].reconstructed);
            }
        }
        free(decomposer->components);
    }
    
    free(decomposer);
}

int decompose_oscillations(OscillationDecomposer* decomposer) {
    if (!decomposer) return 0;
    
    int n = decomposer->signal_length;
    int fft_size = next_power_of_2(n);
    
    // Allocate FFT buffers
    MathComplex* fft_input = (MathComplex*)calloc(fft_size, sizeof(MathComplex));
    double* power_spectrum = (double*)calloc(fft_size / 2, sizeof(double));
    
    if (!fft_input || !power_spectrum) {
        if (fft_input) free(fft_input);
        if (power_spectrum) free(power_spectrum);
        return 0;
    }
    
    // Copy residual to FFT input (zero-padded)
    for (int i = 0; i < n; i++) {
        fft_input[i] = math_complex_from_cartesian(decomposer->residual[i], 0.0);
    }
    
    // Compute FFT
    fft(fft_input, fft_size, 1);
    
    // Compute power spectrum
    compute_power_spectrum(fft_input, power_spectrum, fft_size);
    
    // Find dominant frequencies
    for (int comp = 0; comp < decomposer->max_components; comp++) {
        // Find peak in power spectrum (skip DC component at index 0)
        int peak_idx = 1;
        double peak_power = power_spectrum[1];
        
        for (int i = 2; i < fft_size / 2; i++) {
            if (power_spectrum[i] > peak_power) {
                peak_power = power_spectrum[i];
                peak_idx = i;
            }
        }
        
        // Stop if peak is too small (less than 1% of original signal energy)
        double signal_energy = 0.0;
        for (int i = 0; i < n; i++) {
            signal_energy += decomposer->original_signal[i] * decomposer->original_signal[i];
        }
        
        if (peak_power * peak_power < 0.01 * signal_energy) {
            break;
        }
        
        // Extract component
        OscillationComponent* component = &decomposer->components[comp];
        
        component->frequency = (double)peak_idx / fft_size;
        component->period = (int)(fft_size / peak_idx);
        component->amplitude = 2.0 * math_complex_magnitude(fft_input[peak_idx]) / n;
        component->phase = math_complex_phase(fft_input[peak_idx]);
        
        // Reconstruct this component
        component->signal_length = n;
        component->reconstructed = (double*)malloc(n * sizeof(double));
        
        for (int i = 0; i < n; i++) {
            double angle = 2.0 * MATH_PI * component->frequency * i + component->phase;
            component->reconstructed[i] = component->amplitude * math_cos(angle);
        }
        
        // Remove this component from residual
        for (int i = 0; i < n; i++) {
            decomposer->residual[i] -= component->reconstructed[i];
        }
        
        decomposer->num_components++;
        
        // Zero out this peak in power spectrum for next iteration
        power_spectrum[peak_idx] = 0.0;
        
        // Also zero nearby bins to avoid harmonics
        for (int i = peak_idx - 2; i <= peak_idx + 2; i++) {
            if (i >= 0 && i < fft_size / 2) {
                power_spectrum[i] = 0.0;
            }
        }
    }
    
    // Compute residual energy
    decomposer->residual_energy = 0.0;
    for (int i = 0; i < n; i++) {
        decomposer->residual_energy += decomposer->residual[i] * decomposer->residual[i];
    }
    
    free(fft_input);
    free(power_spectrum);
    
    return decomposer->num_components;
}

const OscillationComponent* get_oscillation_component(
    const OscillationDecomposer* decomposer,
    int index
) {
    if (!decomposer || index < 0 || index >= decomposer->num_components) {
        return NULL;
    }
    return &decomposer->components[index];
}

void reconstruct_component_signal(
    const OscillationComponent* component,
    double* output,
    int length
) {
    if (!component || !output) return;
    
    for (int i = 0; i < length && i < component->signal_length; i++) {
        output[i] = component->reconstructed[i];
    }
}

void print_oscillation_decomposition(const OscillationDecomposer* decomposer) {
    if (!decomposer) return;
    
    printf("\n=== Oscillation Decomposition Results ===\n\n");
    printf("Signal length: %d\n", decomposer->signal_length);
    printf("Components found: %d\n", decomposer->num_components);
    printf("Residual energy: %.6f\n\n", decomposer->residual_energy);
    
    for (int i = 0; i < decomposer->num_components; i++) {
        const OscillationComponent* comp = &decomposer->components[i];
        printf("Component %d:\n", i + 1);
        printf("  Frequency: %.6f Hz\n", comp->frequency);
        printf("  Period: %d iterations\n", comp->period);
        printf("  Amplitude: %.6f\n", comp->amplitude);
        printf("  Phase: %.6f radians\n", comp->phase);
        printf("\n");
    }
}

void export_decomposition_csv(
    const OscillationDecomposer* decomposer,
    const char* filename
) {
    if (!decomposer || !filename) return;
    
    FILE* f = fopen(filename, "w");
    if (!f) return;
    
    // Header
    fprintf(f, "iteration,original,residual");
    for (int i = 0; i < decomposer->num_components; i++) {
        fprintf(f, ",component_%d", i + 1);
    }
    fprintf(f, "\n");
    
    // Data
    for (int t = 0; t < decomposer->signal_length; t++) {
        fprintf(f, "%d,%.6f,%.6f", t, decomposer->original_signal[t], decomposer->residual[t]);
        
        for (int i = 0; i < decomposer->num_components; i++) {
            fprintf(f, ",%.6f", decomposer->components[i].reconstructed[t]);
        }
        
        fprintf(f, "\n");
    }
    
    fclose(f);
}