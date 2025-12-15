/**
 * @file platonic_model_oscillations.c
 * @brief Oscillation Detection - Spatial AND Temporal
 * 
 * Key insight: The CHANGE in oscillations is itself an oscillation
 * in the TIME dimension. We track:
 * 
 * 1. Spatial oscillations (across all dimensions)
 * 2. Temporal oscillations (how spatial oscillations change)
 * 3. Use tetration towers to stabilize temporal oscillations
 * 
 * MIGRATED: Uses NEW math library
 * - Replaced math_cos with math_cos (1 call)
 * - Replaced math_sin with math_sin (2 calls)
 * - Replaced math_sqrt with math_sqrt (1 call)
 * - Replaced math_abs with math_abs (2 calls)
 * - Replaced math_exp with math_exp (1 call)
 * Total: 7 function calls migrated to NEW math library
 */

#include "platonic_model.h"
#include "../../../math/include/math/transcendental.h"  // NEW math library
#include "../../../math/include/math/arithmetic.h"       // NEW math library
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Simple FFT for power of 2 sizes (Cooley-Tukey algorithm)
static void fft_1d(double* real, double* imag, uint32_t n, bool inverse) {
    if (n <= 1) return;
    
    // Bit-reversal permutation
    uint32_t j = 0;
    for (uint32_t i = 0; i < n - 1; i++) {
        if (i < j) {
            double temp = real[i];
            real[i] = real[j];
            real[j] = temp;
            
            temp = imag[i];
            imag[i] = imag[j];
            imag[j] = temp;
        }
        
        uint32_t k = n / 2;
        while (k <= j) {
            j -= k;
            k /= 2;
        }
        j += k;
    }
    
    // FFT computation
    for (uint32_t len = 2; len <= n; len *= 2) {
        double angle = (inverse ? 2.0 : -2.0) * MATH_PI / len;
        double wlen_real = math_cos(angle);
        double wlen_imag = math_sin(angle);
        
        for (uint32_t i = 0; i < n; i += len) {
            double w_real = 1.0;
            double w_imag = 0.0;
            
            for (uint32_t j = 0; j < len / 2; j++) {
                double u_real = real[i + j];
                double u_imag = imag[i + j];
                
                double v_real = real[i + j + len/2] * w_real - imag[i + j + len/2] * w_imag;
                double v_imag = real[i + j + len/2] * w_imag + imag[i + j + len/2] * w_real;
                
                real[i + j] = u_real + v_real;
                imag[i + j] = u_imag + v_imag;
                
                real[i + j + len/2] = u_real - v_real;
                imag[i + j + len/2] = u_imag - v_imag;
                
                double temp = w_real * wlen_real - w_imag * wlen_imag;
                w_imag = w_real * wlen_imag + w_imag * wlen_real;
                w_real = temp;
            }
        }
    }
    
    if (inverse) {
        for (uint32_t i = 0; i < n; i++) {
            real[i] /= n;
            imag[i] /= n;
        }
    }
}

void platonic_detect_spatial_oscillations(
    PlatonicModel* model,
    uint32_t num_time_samples,
    double sampling_rate
) {
    if (!model) return;
    
    printf("\n🔍 Detecting spatial oscillations across %u dimensions...\n", 
           model->num_dimensions);
    
    // Ensure num_time_samples is power of 2 for FFT
    uint32_t n = 1;
    while (n < num_time_samples) n *= 2;
    num_time_samples = n;
    
    double* time_series_real = malloc(num_time_samples * sizeof(double));
    double* time_series_imag = malloc(num_time_samples * sizeof(double));
    
    // Detect oscillations for each dimension
    for (uint32_t d = 0; d < model->num_dimensions; d++) {
        // Build time series by sampling vertex positions in this dimension
        for (uint32_t t = 0; t < num_time_samples; t++) {
            double sum = 0.0;
            
            // Sample all vertices in this dimension
            for (uint32_t v = 0; v < model->num_vertices; v++) {
                double value = model->vertex_positions[v * model->num_dimensions + d];
                
                // Add small temporal perturbation to simulate evolution
                double perturbation = 0.001 * math_sin(2.0 * MATH_PI * t / num_time_samples);
                sum += value + perturbation;
            }
            
            time_series_real[t] = sum / model->num_vertices;
            time_series_imag[t] = 0.0;
        }
        
        // Apply FFT
        fft_1d(time_series_real, time_series_imag, num_time_samples, false);
        
        // Find dominant frequency
        double max_magnitude = 0.0;
        uint32_t max_idx = 0;
        
        for (uint32_t i = 1; i < num_time_samples / 2; i++) {
            double magnitude = math_sqrt(time_series_real[i] * time_series_real[i] +
                                   time_series_imag[i] * time_series_imag[i]);
            
            if (magnitude > max_magnitude) {
                max_magnitude = magnitude;
                max_idx = i;
            }
        }
        
        // Calculate frequency and amplitude
        double frequency = (double)max_idx * sampling_rate / num_time_samples;
        double amplitude = max_magnitude / num_time_samples;
        
        // Update oscillation structure
        model->spatial_oscillations[d].frequency = frequency;
        model->spatial_oscillations[d].amplitude = amplitude;
        model->spatial_oscillations[d].is_stable = (amplitude < 0.01);
        
        // Update history (shift and add new)
        for (uint32_t h = 0; h < model->spatial_oscillations[d].num_samples - 1; h++) {
            model->spatial_oscillations[d].history[h] = 
                model->spatial_oscillations[d].history[h + 1];
        }
        model->spatial_oscillations[d].history[
            model->spatial_oscillations[d].num_samples - 1
        ] = amplitude;
        
        if (d < 5 || !model->spatial_oscillations[d].is_stable) {
            printf("  Dim %u: freq=%.2f Hz, amp=%.6f %s\n",
                   d, frequency, amplitude,
                   model->spatial_oscillations[d].is_stable ? "✓" : "⚠");
        }
    }
    
    free(time_series_real);
    free(time_series_imag);
}

void platonic_detect_temporal_oscillations(PlatonicModel* model) {
    if (!model) return;
    
    printf("\n🔍 Detecting temporal oscillations (oscillations of oscillations)...\n");
    
    // For each dimension, analyze how its spatial oscillation changes over time
    for (uint32_t d = 0; d < model->num_dimensions; d++) {
        DimensionalOscillation* spatial = &model->spatial_oscillations[d];
        TemporalOscillation* temporal = &model->temporal_oscillations[d];
        
        temporal->source_dimension = d;
        
        // Calculate rate of change (first derivative)
        if (spatial->num_samples >= 2) {
            double recent = spatial->history[spatial->num_samples - 1];
            double previous = spatial->history[spatial->num_samples - 2];
            temporal->rate_of_change = recent - previous;
        }
        
        // Calculate acceleration (second derivative)
        if (spatial->num_samples >= 3) {
            double recent = spatial->history[spatial->num_samples - 1];
            double previous = spatial->history[spatial->num_samples - 2];
            double older = spatial->history[spatial->num_samples - 3];
            
            double rate1 = recent - previous;
            double rate2 = previous - older;
            temporal->acceleration = rate1 - rate2;
        }
        
        // Determine if stabilizing (acceleration toward zero)
        temporal->is_stabilizing = (math_abs(temporal->acceleration) < 0.001);
        
        if (d < 5 || !temporal->is_stabilizing) {
            printf("  Dim %u: rate=%.6f, accel=%.6f %s\n",
                   d, temporal->rate_of_change, temporal->acceleration,
                   temporal->is_stabilizing ? "✓" : "⚠");
        }
    }
}

bool platonic_stabilize_temporal_oscillations(PlatonicModel* model) {
    if (!model) return false;
    
    printf("\n🔧 Stabilizing temporal oscillations using tetration towers...\n");
    
    bool all_stabilized = true;
    uint32_t stabilized_count = 0;
    
    for (uint32_t d = 0; d < model->num_dimensions; d++) {
        TemporalOscillation* temporal = &model->temporal_oscillations[d];
        
        if (temporal->is_stabilizing) {
            stabilized_count++;
            continue;
        }
        
        all_stabilized = false;
        
        // Find appropriate tetration tower to stabilize this dimension
        // Use the rate of change to select tower depth
        double rate_magnitude = math_abs(temporal->rate_of_change);
        
        // Map rate to tetration depth (higher rate = deeper tower)
        uint32_t target_depth = 29 + (uint32_t)(rate_magnitude * 30.0);
        if (target_depth > 59) target_depth = 59;
        
        // Find tower with this depth (use base 2 for simplicity)
        uint32_t tower_idx = 0;  // Base 2, depth 29
        for (uint32_t i = 0; i < model->num_tetration_bases * model->num_tetration_depths; i++) {
            if (model->tetration_towers[i] && 
                model->tetration_towers[i]->base == 2 &&
                model->tetration_towers[i]->depth == target_depth) {
                tower_idx = i;
                break;
            }
        }
        
        temporal->stabilizer = model->tetration_towers[tower_idx];
        
        // Apply tetration bias to vertex positions in this dimension
        if (temporal->stabilizer) {
            double attractor_log = temporal->stabilizer->log_value;
            double attractor = (attractor_log < 100.0) ? math_exp(attractor_log) : attractor_log;
            
            for (uint32_t v = 0; v < model->num_vertices; v++) {
                double* pos = &model->vertex_positions[v * model->num_dimensions + d];
                
                // Bias toward attractor (10% weight)
                double bias_weight = 0.1;
                *pos = *pos * (1.0 - bias_weight) + attractor * bias_weight;
            }
        }
        
        if (d < 5) {
            printf("  Dim %u: Applied tetration tower (base=%u, depth=%u)\n",
                   d, 
                   temporal->stabilizer ? temporal->stabilizer->base : 0,
                   temporal->stabilizer ? temporal->stabilizer->depth : 0);
        }
    }
    
    printf("✓ Stabilized %u / %u dimensions\n", stabilized_count, model->num_dimensions);
    
    return all_stabilized;
}

/**
 * Get maximum oscillation amplitude across all dimensions
 */
double platonic_get_max_oscillation_amplitude(const PlatonicModel* model) {
    if (!model) return 0.0;
    
    double max_amp = 0.0;
    for (uint32_t d = 0; d < model->num_dimensions; d++) {
        if (model->spatial_oscillations[d].amplitude > max_amp) {
            max_amp = model->spatial_oscillations[d].amplitude;
        }
    }
    
    return max_amp;
}

/**
 * Check if all oscillations are stable
 */
bool platonic_all_oscillations_stable(const PlatonicModel* model) {
    if (!model) return false;
    
    for (uint32_t d = 0; d < model->num_dimensions; d++) {
        if (!model->spatial_oscillations[d].is_stable) {
            return false;
        }
        if (!model->temporal_oscillations[d].is_stabilizing) {
            return false;
        }
    }
    
    return true;
}
