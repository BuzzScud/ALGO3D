#ifndef OSCILLATION_DECOMPOSITION_H
#define OSCILLATION_DECOMPOSITION_H

#include <stdbool.h>
#include <stdint.h>
#include <complex.h>

/**
 * Oscillation Decomposition Library
 * 
 * Decomposes multi-frequency oscillations into individual components
 * using FFT and frequency domain analysis.
 */

typedef struct {
    double frequency;          // Frequency in Hz (cycles per iteration)
    double amplitude;          // Amplitude of this component
    double phase;              // Phase offset
    int period;                // Period in iterations
    double* reconstructed;     // Reconstructed signal for this component
    int signal_length;         // Length of reconstructed signal
} OscillationComponent;

typedef struct {
    OscillationComponent* components;  // Array of oscillation components
    int num_components;                // Number of components found
    int max_components;                // Maximum components to extract
    
    double* original_signal;           // Original signal
    int signal_length;                 // Length of signal
    
    double* residual;                  // Residual after removing components
    double residual_energy;            // Energy remaining in residual
} OscillationDecomposer;

/**
 * Create oscillation decomposer
 * 
 * @param signal Original signal to decompose
 * @param signal_length Length of signal
 * @param max_components Maximum number of components to extract
 * @return Decomposer or NULL on error
 */
OscillationDecomposer* create_oscillation_decomposer(
    const double* signal,
    int signal_length,
    int max_components
);

/**
 * Free oscillation decomposer
 */
void free_oscillation_decomposer(OscillationDecomposer* decomposer);

/**
 * Decompose signal into oscillation components
 * 
 * Uses FFT to identify dominant frequencies and extract them
 * 
 * @param decomposer Oscillation decomposer
 * @return Number of components extracted
 */
int decompose_oscillations(OscillationDecomposer* decomposer);

/**
 * Get specific oscillation component
 * 
 * @param decomposer Oscillation decomposer
 * @param index Component index (0 = strongest, 1 = second strongest, etc.)
 * @return Component or NULL if index out of range
 */
const OscillationComponent* get_oscillation_component(
    const OscillationDecomposer* decomposer,
    int index
);

/**
 * Reconstruct signal from component
 * 
 * @param component Oscillation component
 * @param output Output buffer (must be allocated)
 * @param length Length of output buffer
 */
void reconstruct_component_signal(
    const OscillationComponent* component,
    double* output,
    int length
);

/**
 * Print decomposition results
 */
void print_oscillation_decomposition(const OscillationDecomposer* decomposer);

/**
 * Export decomposition to CSV
 */
void export_decomposition_csv(
    const OscillationDecomposer* decomposer,
    const char* filename
);

#endif // OSCILLATION_DECOMPOSITION_H
