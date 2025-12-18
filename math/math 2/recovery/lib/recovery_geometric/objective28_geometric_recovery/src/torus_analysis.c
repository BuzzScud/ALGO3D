/**
 * torus_analysis.c - Torus Analysis and Multi-Dimensional Oscillation Tracking
 */

#include "../include/torus_analysis.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "math/arithmetic.h"
#include "math/transcendental.h"

TorusAnalyzer* create_torus_analyzer(int history_size, uint64_t original_space_size) {
    TorusAnalyzer* analyzer = (TorusAnalyzer*)calloc(1, sizeof(TorusAnalyzer));
    if (!analyzer) return NULL;
    
    analyzer->estimated_k_history = (double*)calloc(history_size, sizeof(double));
    analyzer->error_min_history = (double*)calloc(history_size, sizeof(double));
    analyzer->error_max_history = (double*)calloc(history_size, sizeof(double));
    
    if (!analyzer->estimated_k_history || !analyzer->error_min_history || !analyzer->error_max_history) {
        free_torus_analyzer(analyzer);
        return NULL;
    }
    
    analyzer->history_size = history_size;
    analyzer->current_index = 0;
    analyzer->samples_collected = 0;
    analyzer->torus_identified = false;
    analyzer->original_space_size = original_space_size;
    analyzer->reduced_space_size = original_space_size;
    analyzer->reduction_factor = 1.0;
    
    return analyzer;
}

void free_torus_analyzer(TorusAnalyzer* analyzer) {
    if (!analyzer) return;
    if (analyzer->estimated_k_history) free(analyzer->estimated_k_history);
    if (analyzer->error_min_history) free(analyzer->error_min_history);
    if (analyzer->error_max_history) free(analyzer->error_max_history);
    free(analyzer);
}

void torus_analyzer_add_sample(
    TorusAnalyzer* analyzer,
    double estimated_k,
    double error_min,
    double error_max
) {
    if (!analyzer) return;
    
    int idx = analyzer->current_index;
    analyzer->estimated_k_history[idx] = estimated_k;
    analyzer->error_min_history[idx] = error_min;
    analyzer->error_max_history[idx] = error_max;
    
    analyzer->current_index = (analyzer->current_index + 1) % analyzer->history_size;
    analyzer->samples_collected++;
}

OscillationMetrics compute_oscillation_metrics(
    const double* history,
    int size
) {
    OscillationMetrics metrics = {0};
    
    if (size < 2) return metrics;
    
    // Find min and max
    metrics.min_value = history[0];
    metrics.max_value = history[0];
    
    for (int i = 1; i < size; i++) {
        if (history[i] < metrics.min_value) metrics.min_value = history[i];
        if (history[i] > metrics.max_value) metrics.max_value = history[i];
    }
    
    // Compute amplitude
    metrics.amplitude = (metrics.max_value - metrics.min_value) / 2.0;
    
    // Estimate frequency by counting zero crossings
    double mean = (metrics.min_value + metrics.max_value) / 2.0;
    int zero_crossings = 0;
    
    for (int i = 1; i < size; i++) {
        if ((history[i-1] < mean && history[i] >= mean) ||
            (history[i-1] >= mean && history[i] < mean)) {
            zero_crossings++;
        }
    }
    
    // Frequency = zero_crossings / (2 * size)
    if (zero_crossings > 0) {
        metrics.period = (2 * size) / zero_crossings;
        metrics.frequency = 1.0 / metrics.period;
    }
    
    // Estimate phase (simplified - angle of first sample)
    if (metrics.amplitude > 0) {
        double normalized = (history[0] - mean) / metrics.amplitude;
        // Clamp to [-1, 1]
        if (normalized > 1.0) normalized = 1.0;
        if (normalized < -1.0) normalized = -1.0;
        metrics.phase = math_acos(normalized);
    }
    
    return metrics;
}

bool analyze_torus(TorusAnalyzer* analyzer) {
    if (!analyzer) return false;
    
    // Need at least 50 samples to analyze
    int samples = (analyzer->samples_collected < analyzer->history_size) ? 
                  analyzer->samples_collected : analyzer->history_size;
    
    if (samples < 50) return false;
    
    // Compute oscillation metrics for each dimension
    analyzer->k_oscillation = compute_oscillation_metrics(
        analyzer->estimated_k_history, samples);
    
    analyzer->error_min_oscillation = compute_oscillation_metrics(
        analyzer->error_min_history, samples);
    
    analyzer->error_max_oscillation = compute_oscillation_metrics(
        analyzer->error_max_history, samples);
    
    // Identify torus if oscillations are detected in at least 2 dimensions
    int oscillating_dims = 0;
    
    if (analyzer->k_oscillation.amplitude > 0.01) oscillating_dims++;
    if (analyzer->error_min_oscillation.amplitude > 0.01) oscillating_dims++;
    if (analyzer->error_max_oscillation.amplitude > 0.01) oscillating_dims++;
    
    if (oscillating_dims >= 2) {
        analyzer->torus_identified = true;
        
        // Compute torus parameters
        // Major radius = average of k oscillation amplitude
        analyzer->major_radius = analyzer->k_oscillation.amplitude;
        
        // Minor radius = average of error oscillation amplitudes
        analyzer->minor_radius = (analyzer->error_min_oscillation.amplitude + 
                                 analyzer->error_max_oscillation.amplitude) / 2.0;
        
        // Compute search space reduction
        // Original space: full range
        // Reduced space: torus volume ≈ 2π² * R * r
        // where R = major radius, r = minor radius
        
        double torus_volume_factor = 2.0 * 3.14159 * 3.14159 * 
                                    analyzer->major_radius * analyzer->minor_radius;
        
        // Normalize by original space
        double original_volume = (double)analyzer->original_space_size;
        
        if (original_volume > 0) {
            analyzer->reduced_space_size = (uint64_t)(torus_volume_factor);
            analyzer->reduction_factor = original_volume / torus_volume_factor;
        }
        
        return true;
    }
    
    return false;
}

double get_reduction_factor(const TorusAnalyzer* analyzer) {
    return analyzer ? analyzer->reduction_factor : 1.0;
}

void print_torus_analysis(const TorusAnalyzer* analyzer) {
    if (!analyzer) return;
    
    printf("\n=== Torus Analysis Results ===\n\n");
    
    printf("Samples collected: %d\n", analyzer->samples_collected);
    printf("Torus identified: %s\n\n", analyzer->torus_identified ? "YES" : "NO");
    
    if (analyzer->torus_identified) {
        printf("Torus Parameters:\n");
        printf("  Major radius (R): %.6f\n", analyzer->major_radius);
        printf("  Minor radius (r): %.6f\n", analyzer->minor_radius);
        printf("\n");
    }
    
    printf("Dimension 1: Estimated k Oscillation\n");
    printf("  Min: %.6f\n", analyzer->k_oscillation.min_value);
    printf("  Max: %.6f\n", analyzer->k_oscillation.max_value);
    printf("  Amplitude: %.6f\n", analyzer->k_oscillation.amplitude);
    printf("  Period: %d iterations\n", analyzer->k_oscillation.period);
    printf("  Frequency: %.6f\n", analyzer->k_oscillation.frequency);
    printf("\n");
    
    printf("Dimension 2: Error Min Oscillation\n");
    printf("  Min: %.6f\n", analyzer->error_min_oscillation.min_value);
    printf("  Max: %.6f\n", analyzer->error_min_oscillation.max_value);
    printf("  Amplitude: %.6f\n", analyzer->error_min_oscillation.amplitude);
    printf("  Period: %d iterations\n", analyzer->error_min_oscillation.period);
    printf("  Frequency: %.6f\n", analyzer->error_min_oscillation.frequency);
    printf("\n");
    
    printf("Dimension 3: Error Max Oscillation\n");
    printf("  Min: %.6f\n", analyzer->error_max_oscillation.min_value);
    printf("  Max: %.6f\n", analyzer->error_max_oscillation.max_value);
    printf("  Amplitude: %.6f\n", analyzer->error_max_oscillation.amplitude);
    printf("  Period: %d iterations\n", analyzer->error_max_oscillation.period);
    printf("  Frequency: %.6f\n", analyzer->error_max_oscillation.frequency);
    printf("\n");
    
    printf("Search Space Reduction:\n");
    printf("  Original space: %lu\n", analyzer->original_space_size);
    printf("  Reduced space: %lu\n", analyzer->reduced_space_size);
    printf("  Reduction factor: %.2fx\n", analyzer->reduction_factor);
    printf("\n");
}

void export_torus_data(const TorusAnalyzer* analyzer, const char* filename) {
    if (!analyzer || !filename) return;
    
    FILE* f = fopen(filename, "w");
    if (!f) return;
    
    fprintf(f, "iteration,estimated_k,error_min,error_max\n");
    
    int samples = (analyzer->samples_collected < analyzer->history_size) ? 
                  analyzer->samples_collected : analyzer->history_size;
    
    for (int i = 0; i < samples; i++) {
        fprintf(f, "%d,%.6f,%.6f,%.6f\n",
                i,
                analyzer->estimated_k_history[i],
                analyzer->error_min_history[i],
                analyzer->error_max_history[i]);
    }
    
    fclose(f);
}
