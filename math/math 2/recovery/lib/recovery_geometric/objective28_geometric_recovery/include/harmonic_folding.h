/**
 * harmonic_folding.h - Harmonic Folding for Signal Analysis
 * 
 * Applies harmonic folding using prime frequencies to reveal hidden
 * oscillation patterns in the k estimate signal.
 * 
 * Key principles:
 * - Uses prime frequencies: [5, 7, 11, 13, 17, 19, 23, 29, 31]
 * - Folds signal using sin(2πft) modulation
 * - Reveals structure beyond simple FFT analysis
 * - NO math.h - uses ONLY prime_* functions
 */

#ifndef HARMONIC_FOLDING_H
#define HARMONIC_FOLDING_H

#include <stdint.h>
#include <stdbool.h>

/**
 * Harmonic component structure
 */
typedef struct {
    uint32_t frequency;      // Prime frequency (5, 7, 11, etc.)
    double amplitude;        // Amplitude of this harmonic
    double phase;           // Phase offset in radians
    double energy;          // Energy contribution (amplitude^2)
    double* folded_signal;  // Folded signal for this frequency
    uint32_t signal_length; // Length of folded signal
} HarmonicComponent;

/**
 * Harmonic folding result
 */
typedef struct {
    HarmonicComponent* components;  // Array of harmonic components
    uint32_t num_components;        // Number of components (9 primes)
    double* original_signal;        // Original k estimates
    uint32_t signal_length;         // Length of original signal
    double total_energy;            // Total energy across all harmonics
    uint32_t dominant_frequency;    // Frequency with highest energy
    double dominant_energy;         // Energy of dominant frequency
} HarmonicFoldingResult;

/**
 * Create harmonic folding analyzer
 * 
 * @param signal Array of k estimates
 * @param signal_length Number of samples
 * @return Initialized result structure, or NULL on failure
 */
HarmonicFoldingResult* create_harmonic_folding_result(
    const double* signal,
    uint32_t signal_length
);

/**
 * Apply harmonic folding to signal
 * 
 * Folds the signal using each prime frequency:
 * folded[i] = signal[i] * sin(2π * freq * i / signal_length)
 * 
 * @param result Result structure to populate
 * @return true on success, false on failure
 */
bool apply_harmonic_folding(HarmonicFoldingResult* result);

/**
 * Compute energy for each harmonic component
 * 
 * Energy = sum(folded_signal[i]^2) / signal_length
 * 
 * @param result Result structure
 * @return true on success, false on failure
 */
bool compute_harmonic_energies(HarmonicFoldingResult* result);

/**
 * Identify dominant harmonic frequency
 * 
 * @param result Result structure
 * @return Dominant frequency (5-31), or 0 on failure
 */
uint32_t identify_dominant_harmonic(HarmonicFoldingResult* result);

/**
 * Get harmonic component by frequency
 * 
 * @param result Result structure
 * @param frequency Prime frequency (5, 7, 11, etc.)
 * @return Component structure, or NULL if not found
 */
HarmonicComponent* get_harmonic_component(
    HarmonicFoldingResult* result,
    uint32_t frequency
);

/**
 * Print harmonic folding analysis
 * 
 * @param result Result structure
 */
void print_harmonic_folding_analysis(const HarmonicFoldingResult* result);

/**
 * Export harmonic folding to CSV
 * 
 * Format: frequency,amplitude,phase,energy
 * 
 * @param result Result structure
 * @param filename Output filename
 * @return true on success, false on failure
 */
bool export_harmonic_folding_csv(
    const HarmonicFoldingResult* result,
    const char* filename
);

/**
 * Export folded signals to CSV
 * 
 * Format: iteration,freq5,freq7,freq11,...,freq31
 * 
 * @param result Result structure
 * @param filename Output filename
 * @return true on success, false on failure
 */
bool export_folded_signals_csv(
    const HarmonicFoldingResult* result,
    const char* filename
);

/**
 * Free harmonic folding result
 * 
 * @param result Result structure to free
 */
void free_harmonic_folding_result(HarmonicFoldingResult* result);

#endif // HARMONIC_FOLDING_H
