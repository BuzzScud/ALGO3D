/**
 * harmonic_folding.c - Harmonic Folding Implementation
 * 
 * Uses ONLY prime_* functions (NO math.h)
 */

#include "../include/harmonic_folding.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "math/arithmetic.h"
#include "math/transcendental.h"

// Prime frequencies for harmonic folding
static const uint32_t PRIME_FREQUENCIES[] = {5, 7, 11, 13, 17, 19, 23, 29, 31};
static const uint32_t NUM_PRIME_FREQUENCIES = 9;

/**
 * Create harmonic folding analyzer
 */
HarmonicFoldingResult* create_harmonic_folding_result(
    const double* signal,
    uint32_t signal_length
) {
    if (!signal || signal_length == 0) {
        return NULL;
    }
    
    HarmonicFoldingResult* result = (HarmonicFoldingResult*)malloc(sizeof(HarmonicFoldingResult));
    if (!result) {
        return NULL;
    }
    
    // Allocate components array
    result->components = (HarmonicComponent*)calloc(NUM_PRIME_FREQUENCIES, sizeof(HarmonicComponent));
    if (!result->components) {
        free(result);
        return NULL;
    }
    
    result->num_components = NUM_PRIME_FREQUENCIES;
    result->signal_length = signal_length;
    
    // Copy original signal
    result->original_signal = (double*)malloc(signal_length * sizeof(double));
    if (!result->original_signal) {
        free(result->components);
        free(result);
        return NULL;
    }
    memcpy(result->original_signal, signal, signal_length * sizeof(double));
    
    // Initialize components
    for (uint32_t i = 0; i < NUM_PRIME_FREQUENCIES; i++) {
        result->components[i].frequency = PRIME_FREQUENCIES[i];
        result->components[i].amplitude = 0.0;
        result->components[i].phase = 0.0;
        result->components[i].energy = 0.0;
        result->components[i].signal_length = signal_length;
        
        // Allocate folded signal
        result->components[i].folded_signal = (double*)calloc(signal_length, sizeof(double));
        if (!result->components[i].folded_signal) {
            // Cleanup on failure
            for (uint32_t j = 0; j < i; j++) {
                free(result->components[j].folded_signal);
            }
            free(result->original_signal);
            free(result->components);
            free(result);
            return NULL;
        }
    }
    
    result->total_energy = 0.0;
    result->dominant_frequency = 0;
    result->dominant_energy = 0.0;
    
    return result;
}

/**
 * Apply harmonic folding to signal
 */
bool apply_harmonic_folding(HarmonicFoldingResult* result) {
    if (!result) {
        return false;
    }
    
    const double TWO_PI = 6.283185307179586;
    
    // For each prime frequency
    for (uint32_t c = 0; c < result->num_components; c++) {
        HarmonicComponent* comp = &result->components[c];
        uint32_t freq = comp->frequency;
        
        // Fold the signal: folded[i] = signal[i] * sin(2π * freq * i / N)
        for (uint32_t i = 0; i < result->signal_length; i++) {
            // Compute phase: 2π * freq * i / N
            double phase = TWO_PI * (double)freq * (double)i / (double)result->signal_length;
            
            // Compute sin(phase)
            double sin_val = math_sin(phase);
            
            // Fold: signal[i] * sin(phase)
            comp->folded_signal[i] = result->original_signal[i] * sin_val;
        }
        
        // Compute amplitude (max absolute value of folded signal)
        double max_abs = 0.0;
        for (uint32_t i = 0; i < result->signal_length; i++) {
            double abs_val = math_abs(comp->folded_signal[i]);
            if (abs_val > max_abs) {
                max_abs = abs_val;
            }
        }
        comp->amplitude = max_abs;
        
        // Compute phase (angle of first peak)
        for (uint32_t i = 0; i < result->signal_length; i++) {
            if (math_abs(comp->folded_signal[i]) > max_abs * 0.9) {
                // Found peak - compute phase
                double phase = TWO_PI * (double)freq * (double)i / (double)result->signal_length;
                comp->phase = phase;
                break;
            }
        }
    }
    
    return true;
}

/**
 * Compute energy for each harmonic component
 */
bool compute_harmonic_energies(HarmonicFoldingResult* result) {
    if (!result) {
        return false;
    }
    
    result->total_energy = 0.0;
    
    // For each component
    for (uint32_t c = 0; c < result->num_components; c++) {
        HarmonicComponent* comp = &result->components[c];
        
        // Energy = sum(folded_signal[i]^2) / N
        double sum_squares = 0.0;
        for (uint32_t i = 0; i < result->signal_length; i++) {
            double val = comp->folded_signal[i];
            sum_squares += val * val;
        }
        
        comp->energy = sum_squares / (double)result->signal_length;
        result->total_energy += comp->energy;
    }
    
    return true;
}

/**
 * Identify dominant harmonic frequency
 */
uint32_t identify_dominant_harmonic(HarmonicFoldingResult* result) {
    if (!result || result->num_components == 0) {
        return 0;
    }
    
    double max_energy = 0.0;
    uint32_t dominant_freq = 0;
    
    for (uint32_t c = 0; c < result->num_components; c++) {
        if (result->components[c].energy > max_energy) {
            max_energy = result->components[c].energy;
            dominant_freq = result->components[c].frequency;
        }
    }
    
    result->dominant_frequency = dominant_freq;
    result->dominant_energy = max_energy;
    
    return dominant_freq;
}

/**
 * Get harmonic component by frequency
 */
HarmonicComponent* get_harmonic_component(
    HarmonicFoldingResult* result,
    uint32_t frequency
) {
    if (!result) {
        return NULL;
    }
    
    for (uint32_t c = 0; c < result->num_components; c++) {
        if (result->components[c].frequency == frequency) {
            return &result->components[c];
        }
    }
    
    return NULL;
}

/**
 * Print harmonic folding analysis
 */
void print_harmonic_folding_analysis(const HarmonicFoldingResult* result) {
    if (!result) {
        return;
    }
    
    printf("\n=== Harmonic Folding Analysis ===\n\n");
    printf("Signal length: %u samples\n", result->signal_length);
    printf("Number of harmonics: %u\n", result->num_components);
    printf("Total energy: %.6f\n", result->total_energy);
    printf("Dominant frequency: %u Hz (energy: %.6f)\n\n", 
           result->dominant_frequency, result->dominant_energy);
    
    printf("Harmonic Components:\n\n");
    printf("| Freq | Amplitude | Phase (rad) | Energy    | %% of Total |\n");
    printf("|------|-----------|-------------|-----------|------------|\n");
    
    for (uint32_t c = 0; c < result->num_components; c++) {
        const HarmonicComponent* comp = &result->components[c];
        double percent = (result->total_energy > 0.0) ? 
                        (comp->energy / result->total_energy * 100.0) : 0.0;
        
        printf("| %4u | %9.2f | %11.6f | %9.6f | %9.2f%% |\n",
               comp->frequency,
               comp->amplitude,
               comp->phase,
               comp->energy,
               percent);
    }
    printf("\n");
}

/**
 * Export harmonic folding to CSV
 */
bool export_harmonic_folding_csv(
    const HarmonicFoldingResult* result,
    const char* filename
) {
    if (!result || !filename) {
        return false;
    }
    
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        return false;
    }
    
    // Header
    fprintf(fp, "frequency,amplitude,phase,energy,percent_total\n");
    
    // Data
    for (uint32_t c = 0; c < result->num_components; c++) {
        const HarmonicComponent* comp = &result->components[c];
        double percent = (result->total_energy > 0.0) ? 
                        (comp->energy / result->total_energy * 100.0) : 0.0;
        
        fprintf(fp, "%u,%.6f,%.6f,%.6f,%.2f\n",
                comp->frequency,
                comp->amplitude,
                comp->phase,
                comp->energy,
                percent);
    }
    
    fclose(fp);
    return true;
}

/**
 * Export folded signals to CSV
 */
bool export_folded_signals_csv(
    const HarmonicFoldingResult* result,
    const char* filename
) {
    if (!result || !filename) {
        return false;
    }
    
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        return false;
    }
    
    // Header
    fprintf(fp, "iteration,original");
    for (uint32_t c = 0; c < result->num_components; c++) {
        fprintf(fp, ",freq%u", result->components[c].frequency);
    }
    fprintf(fp, "\n");
    
    // Data
    for (uint32_t i = 0; i < result->signal_length; i++) {
        fprintf(fp, "%u,%.6f", i, result->original_signal[i]);
        
        for (uint32_t c = 0; c < result->num_components; c++) {
            fprintf(fp, ",%.6f", result->components[c].folded_signal[i]);
        }
        
        fprintf(fp, "\n");
    }
    
    fclose(fp);
    return true;
}

/**
 * Free harmonic folding result
 */
void free_harmonic_folding_result(HarmonicFoldingResult* result) {
    if (!result) {
        return;
    }
    
    if (result->components) {
        for (uint32_t c = 0; c < result->num_components; c++) {
            if (result->components[c].folded_signal) {
                free(result->components[c].folded_signal);
            }
        }
        free(result->components);
    }
    
    if (result->original_signal) {
        free(result->original_signal);
    }
    
    free(result);
}
