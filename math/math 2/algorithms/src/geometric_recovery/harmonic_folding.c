/**
 * @file harmonic_folding.c
 * @brief Harmonic Folding for Data Compression and Recovery
 * 
 * Implements harmonic folding operations for geometric recovery.
 * Uses NEW math library exclusively - NO legacy dependencies.
 * 
 * Thesis Reference: Chapter 12 - Blind Recovery
 * 
 * Algorithm:
 * - Fold data using harmonic patterns
 * - Compression via harmonic analysis
 * - Integration with cymatic modulation
 */

#include "math/types.h"
#include "math/constants.h"
#include "math/arithmetic.h"
#include "math/constants.h"
#include "math/transcendental.h"
#include "math/constants.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * CONSTANTS
 * ============================================================================ */

#define MAX_HARMONICS 12
#define DEFAULT_FOLD_DEPTH 3

/* ============================================================================
 * TYPES
 * ============================================================================ */

/**
 * @brief Harmonic component
 */
typedef struct {
    double frequency;           // Harmonic frequency
    double amplitude;           // Amplitude
    double phase;               // Phase offset
} HarmonicComponent;

/**
 * @brief Harmonic fold structure
 */
typedef struct {
    HarmonicComponent* harmonics; // Array of harmonic components
    uint32_t num_harmonics;     // Number of harmonics
    uint32_t fold_depth;        // Folding depth
    double compression_ratio;   // Achieved compression ratio
} HarmonicFold;

/**
 * @brief Folded data structure
 */
typedef struct {
    double* folded_data;        // Folded data
    uint32_t folded_size;       // Size of folded data
    uint32_t original_size;     // Original data size
    HarmonicFold* fold;         // Fold parameters
} FoldedData;

/* ============================================================================
 * INTERNAL FUNCTIONS
 * ============================================================================ */

/**
 * @brief Compute harmonic basis function
 * 
 * h(t) = A * sin(2π * f * t + φ)
 */
static double compute_harmonic_basis(
    double t,
    double frequency,
    double amplitude,
    double phase
) {
    return amplitude * math_sin(2.0 * MATH_PI * frequency * t + phase);
}

/**
 * @brief Decompose signal into harmonic components
 * 
 * Uses simplified Fourier-like decomposition
 */
static HarmonicFold* decompose_harmonics(
    const double* data,
    uint32_t size,
    uint32_t num_harmonics
) {
    if (!data || size == 0 || num_harmonics == 0) {
        return NULL;
    }
    
    HarmonicFold* fold = (HarmonicFold*)calloc(1, sizeof(HarmonicFold));
    if (!fold) return NULL;
    
    fold->harmonics = (HarmonicComponent*)calloc(num_harmonics, sizeof(HarmonicComponent));
    if (!fold->harmonics) {
        free(fold);
        return NULL;
    }
    
    fold->num_harmonics = num_harmonics;
    fold->fold_depth = DEFAULT_FOLD_DEPTH;
    
    // Compute harmonic components
    for (uint32_t h = 0; h < num_harmonics; h++) {
        double frequency = (double)(h + 1); // Fundamental and harmonics
        double amplitude = 0.0;
        double phase = 0.0;
        
        // Compute amplitude via correlation
        double sum_cos = 0.0;
        double sum_sin = 0.0;
        
        for (uint32_t i = 0; i < size; i++) {
            double t = (double)i / size;
            double angle = 2.0 * MATH_PI * frequency * t;
            sum_cos += data[i] * math_cos(angle);
            sum_sin += data[i] * math_sin(angle);
        }
        
        amplitude = math_sqrt(sum_cos * sum_cos + sum_sin * sum_sin) / size;
        phase = math_atan2(sum_sin, sum_cos);
        
        fold->harmonics[h].frequency = frequency;
        fold->harmonics[h].amplitude = amplitude;
        fold->harmonics[h].phase = phase;
    }
    
    return fold;
}

/**
 * @brief Fold data using harmonic components
 */
static double* fold_data_harmonically(
    const double* data,
    uint32_t size,
    const HarmonicFold* fold,
    uint32_t* folded_size_out
) {
    if (!data || !fold || size == 0) {
        return NULL;
    }
    
    // Folded size is determined by fold depth
    uint32_t folded_size = size / (1 << fold->fold_depth);
    if (folded_size < fold->num_harmonics) {
        folded_size = fold->num_harmonics;
    }
    
    double* folded = (double*)calloc(folded_size, sizeof(double));
    if (!folded) return NULL;
    
    // Fold data by summing harmonics
    for (uint32_t i = 0; i < folded_size; i++) {
        double t = (double)i / folded_size;
        double value = 0.0;
        
        for (uint32_t h = 0; h < fold->num_harmonics; h++) {
            value += compute_harmonic_basis(
                t,
                fold->harmonics[h].frequency,
                fold->harmonics[h].amplitude,
                fold->harmonics[h].phase
            );
        }
        
        folded[i] = value;
    }
    
    *folded_size_out = folded_size;
    return folded;
}

/**
 * @brief Unfold data using harmonic components
 */
static double* unfold_data_harmonically(
    const double* folded_data,
    uint32_t folded_size,
    const HarmonicFold* fold,
    uint32_t target_size
) {
    if (!folded_data || !fold || folded_size == 0) {
        return NULL;
    }
    
    double* unfolded = (double*)calloc(target_size, sizeof(double));
    if (!unfolded) return NULL;
    
    // Reconstruct data using harmonic components
    for (uint32_t i = 0; i < target_size; i++) {
        double t = (double)i / target_size;
        double value = 0.0;
        
        for (uint32_t h = 0; h < fold->num_harmonics; h++) {
            value += compute_harmonic_basis(
                t,
                fold->harmonics[h].frequency,
                fold->harmonics[h].amplitude,
                fold->harmonics[h].phase
            );
        }
        
        unfolded[i] = value;
    }
    
    return unfolded;
}

/* ============================================================================
 * PUBLIC API
 * ============================================================================ */

/**
 * @brief Fold data using harmonic folding
 * 
 * @param data Input data
 * @param size Data size
 * @param num_harmonics Number of harmonic components (1-12)
 * @param fold_depth Folding depth (1-5)
 * @return Folded data structure, or NULL on error
 */
FoldedData* harmonic_fold_data(
    const double* data,
    uint32_t size,
    uint32_t num_harmonics,
    uint32_t fold_depth
) {
    if (!data || size == 0) {
        return NULL;
    }
    
    // Clamp parameters
    if (num_harmonics == 0) num_harmonics = MAX_HARMONICS;
    if (num_harmonics > MAX_HARMONICS) num_harmonics = MAX_HARMONICS;
    if (fold_depth == 0) fold_depth = DEFAULT_FOLD_DEPTH;
    if (fold_depth > 5) fold_depth = 5;
    
    FoldedData* folded_data = (FoldedData*)calloc(1, sizeof(FoldedData));
    if (!folded_data) return NULL;
    
    // Decompose into harmonics
    folded_data->fold = decompose_harmonics(data, size, num_harmonics);
    if (!folded_data->fold) {
        free(folded_data);
        return NULL;
    }
    
    folded_data->fold->fold_depth = fold_depth;
    
    // Fold data
    uint32_t folded_size;
    folded_data->folded_data = fold_data_harmonically(
        data, size, folded_data->fold, &folded_size
    );
    
    if (!folded_data->folded_data) {
        free(folded_data->fold->harmonics);
        free(folded_data->fold);
        free(folded_data);
        return NULL;
    }
    
    folded_data->folded_size = folded_size;
    folded_data->original_size = size;
    folded_data->fold->compression_ratio = (double)size / folded_size;
    
    return folded_data;
}

/**
 * @brief Unfold data using harmonic folding
 * 
 * @param folded Folded data structure
 * @param unfolded_data Output buffer (must be allocated)
 * @param target_size Target size (should match original size)
 * @return 0 on success, -1 on error
 */
int harmonic_unfold_data(
    const FoldedData* folded,
    double* unfolded_data,
    uint32_t target_size
) {
    if (!folded || !unfolded_data || target_size == 0) {
        return -1;
    }
    
    double* temp = unfold_data_harmonically(
        folded->folded_data,
        folded->folded_size,
        folded->fold,
        target_size
    );
    
    if (!temp) return -1;
    
    memcpy(unfolded_data, temp, target_size * sizeof(double));
    free(temp);
    
    return 0;
}

/**
 * @brief Free folded data
 */
void harmonic_free_folded_data(FoldedData* folded) {
    if (!folded) return;
    
    if (folded->folded_data) free(folded->folded_data);
    
    if (folded->fold) {
        if (folded->fold->harmonics) free(folded->fold->harmonics);
        free(folded->fold);
    }
    
    free(folded);
}

/**
 * @brief Get compression ratio
 */
double harmonic_get_compression_ratio(const FoldedData* folded) {
    if (!folded || !folded->fold) return 1.0;
    return folded->fold->compression_ratio;
}

/**
 * @brief Compute reconstruction error
 * 
 * Returns RMS error between original and reconstructed data
 */
double harmonic_compute_error(
    const double* original,
    const double* reconstructed,
    uint32_t size
) {
    if (!original || !reconstructed || size == 0) {
        return -1.0;
    }
    
    double sum_sq_error = 0.0;
    
    for (uint32_t i = 0; i < size; i++) {
        double error = original[i] - reconstructed[i];
        sum_sq_error += error * error;
    }
    
    return math_sqrt(sum_sq_error / size);
}

/**
 * @brief Print harmonic fold statistics
 */
void harmonic_print_stats(const FoldedData* folded) {
    if (!folded || !folded->fold) return;
    
    printf("=== Harmonic Fold Statistics ===\n");
    printf("Original size: %u\n", folded->original_size);
    printf("Folded size: %u\n", folded->folded_size);
    printf("Compression ratio: %.2fx\n", folded->fold->compression_ratio);
    printf("Number of harmonics: %u\n", folded->fold->num_harmonics);
    printf("Fold depth: %u\n", folded->fold->fold_depth);
    
    printf("\nHarmonic components:\n");
    for (uint32_t i = 0; i < folded->fold->num_harmonics; i++) {
        const HarmonicComponent* h = &folded->fold->harmonics[i];
        printf("  H%u: f=%.2f, A=%.4f, φ=%.4f\n",
               i + 1, h->frequency, h->amplitude, h->phase);
    }
    
    printf("================================\n");
}