/**
 * @file iterative_search.c
 * @brief Implementation of iterative search with entropy reduction
 * 
 * PHASE 1 WEEK 2: Migrated to NEW math library
 * - Replaced math_pow with math_pow (1 call)
 * - Replaced math_sqrt with math_sqrt (1 call)
 * Total: 2 function calls migrated to NEW math library
 */

#include "iterative_search.h"
#include "statistics.h"
#include "math/types.h"
#include "math/transcendental.h"  // PHASE 1: NEW math library
#include "math/arithmetic.h"       // PHASE 1: NEW math library
#include <stdlib.h>
#include <string.h>

#ifndef LARGE_VALUE
#define LARGE_VALUE 1e308
#endif

bool iterative_search(const void* query, const SearchConfig* config, SearchResult* result) {
    if (!query || !config || !result) {
        return false;
    }
    
    if (!config->distance_fn || !config->anchor_gen_fn) {
        return false;
    }
    
    // Initialize result
    result->best_candidate = 0;
    result->min_distance = LARGE_VALUE;
    result->iterations = 0;
    result->reassessments = 0;
    result->converged = false;
    
    double current_entropy = config->initial_entropy_bits;
    
    // Allocate anchor array
    uint64_t* anchors = (uint64_t*)malloc(config->num_anchors * sizeof(uint64_t));
    if (!anchors) {
        return false;
    }
    
    // Main reassessment loop
    for (uint32_t reassess = 0; reassess < config->max_reassessments; reassess++) {
        result->reassessments = reassess;
        
        // Generate new anchors
        config->anchor_gen_fn(anchors, config->num_anchors, config->user_data);
        
        // Estimate starting point
        uint64_t estimated = iterative_estimate_from_anchors(query, anchors,
                                                              config->num_anchors,
                                                              config->distance_fn,
                                                              config->user_data);
        
        // Iterative search loop
        for (uint32_t iter = 0; iter < config->max_iterations; iter++) {
            result->iterations++;
            
            // Calculate reduced search space
            uint64_t search_size = iterative_reduced_space_size(current_entropy, iter, 0.18, 0.45);
            
            // Re-estimate from anchors
            estimated = iterative_estimate_from_anchors(query, anchors,
                                                        config->num_anchors,
                                                        config->distance_fn,
                                                        config->user_data);
            
            // Search around estimate
            uint64_t search_start = (estimated > search_size / 2) ? 
                                    (estimated - search_size / 2) : 0;
            uint64_t search_end = (estimated + search_size / 2 < config->search_space_size) ?
                                  (estimated + search_size / 2) : config->search_space_size;
            
            // Find best in range
            for (uint64_t candidate = search_start; candidate < search_end; candidate++) {
                double dist = config->distance_fn(query, candidate, config->user_data);
                
                if (dist < result->min_distance) {
                    result->min_distance = dist;
                    result->best_candidate = candidate;
                }
                
                // Check for convergence
                if (dist < config->convergence_threshold) {
                    result->converged = true;
                    free(anchors);
                    return true;
                }
            }
            
            // Check for oscillation
            bool oscillating = false;
            if (config->oscillation_fn) {
                oscillating = config->oscillation_fn(anchors, config->num_anchors, config->user_data);
            }
            
            if (oscillating || result->min_distance > MATH_EPSILON) {
                // Regenerate anchors and reduce entropy
                config->anchor_gen_fn(anchors, config->num_anchors, config->user_data);
                current_entropy = stats_entropy_reduction(current_entropy, 1, 0.18, 0.45);
            }
            
            // Update estimate
            estimated = result->best_candidate;
            
            // Check if converged
            if (result->min_distance < config->convergence_threshold) {
                result->converged = true;
                free(anchors);
                return true;
            }
        }
        
        // Trigger reassessment if not converged
        if (!result->converged) {
            current_entropy = stats_entropy_reduction(current_entropy, 5, 0.18, 0.45);
        }
    }
    
    free(anchors);
    return result->converged;
}

uint64_t iterative_estimate_from_anchors(const void* query,
                                          const uint64_t* anchors,
                                          size_t num_anchors,
                                          DistanceFunction distance_fn,
                                          void* user_data) {
    if (!query || !anchors || num_anchors == 0 || !distance_fn) {
        return 0;
    }
    
    // Calculate distances to all anchors
    double total_distance = 0.0;
    double* distances = (double*)malloc(num_anchors * sizeof(double));
    if (!distances) {
        return 0;
    }
    
    for (size_t i = 0; i < num_anchors; i++) {
        distances[i] = distance_fn(query, anchors[i], user_data);
        total_distance += distances[i];
    }
    
    // Weighted average based on inverse distances
    double weighted_sum = 0.0;
    double weight_sum = 0.0;
    
    for (size_t i = 0; i < num_anchors; i++) {
        double weight = 1.0 / (distances[i] + MATH_EPSILON);
        weighted_sum += (double)anchors[i] * weight;
        weight_sum += weight;
    }
    
    free(distances);
    
    if (weight_sum > 0.0) {
        return (uint64_t)(weighted_sum / weight_sum);
    }
    
    // Fallback: simple average
    uint64_t sum = 0;
    for (size_t i = 0; i < num_anchors; i++) {
        sum += anchors[i];
    }
    return sum / num_anchors;
}

uint64_t iterative_reduced_space_size(double current_entropy,
                                       uint32_t steps,
                                       double cut_min,
                                       double cut_max) {
    double reduced = stats_entropy_reduction(current_entropy, steps, cut_min, cut_max);
    
    // Convert bits to size: 2^bits
    double size = math_pow(2.0, reduced);
    
    // Ensure minimum size of 1
    if (size < 1.0) {
        size = 1.0;
    }
    
    return (uint64_t)size;
}

void iterative_default_anchor_generator(uint64_t* anchors,
                                         size_t num_anchors,
                                         void* user_data) {
    if (!anchors || num_anchors == 0 || !user_data) {
        return;
    }
    
    uint64_t search_space_size = *(uint64_t*)user_data;
    
    // Generate anchors using golden ratio spacing
    for (size_t i = 0; i < num_anchors; i++) {
        // Use golden ratio for quasi-random distribution
        double t = (double)i * MATH_PHI;
        t = t - (uint64_t)t;  // Fractional part
        
        anchors[i] = (uint64_t)(t * (double)search_space_size);
    }
}

bool iterative_default_oscillation_detector(const uint64_t* anchors,
                                             size_t num_anchors,
                                             void* user_data) {
    (void)user_data;  // Unused
    
    if (!anchors || num_anchors < 2) {
        return false;
    }
    
    // Calculate mean
    double mean = 0.0;
    for (size_t i = 0; i < num_anchors; i++) {
        mean += (double)anchors[i];
    }
    mean /= (double)num_anchors;
    
    // Calculate variance
    double variance = 0.0;
    for (size_t i = 0; i < num_anchors; i++) {
        double diff = (double)anchors[i] - mean;
        variance += diff * diff;
    }
    variance /= (double)num_anchors;
    
    // Calculate standard deviation
    double std_dev = math_sqrt(variance);
    
    // Oscillation detected if std_dev is too high relative to mean
    double coefficient_of_variation = std_dev / (mean + MATH_EPSILON);
    
    return coefficient_of_variation > 0.5;  // 50% threshold
}

bool iterative_fft_oscillation_detector(const uint64_t* anchors,
                                         size_t num_anchors,
                                         void* user_data) {
    (void)user_data;  // Unused
    
    if (!anchors || num_anchors < 4) {
        return false;  // Need at least 4 points for FFT
    }
    
    // Convert anchors to signal (normalize to [0, 1])
    double* signal = (double*)malloc(num_anchors * sizeof(double));
    if (!signal) {
        return false;  // Fallback to no oscillation
    }
    
    // Find min and max for normalization
    uint64_t min_val = anchors[0];
    uint64_t max_val = anchors[0];
    for (size_t i = 1; i < num_anchors; i++) {
        if (anchors[i] < min_val) min_val = anchors[i];
        if (anchors[i] > max_val) max_val = anchors[i];
    }
    
    // Normalize signal
    double range = (double)(max_val - min_val);
    if (range < MATH_EPSILON) {
        free(signal);
        return false;  // All values the same, no oscillation
    }
    
    for (size_t i = 0; i < num_anchors; i++) {
        signal[i] = ((double)(anchors[i] - min_val)) / range;
    }
    
    // Perform FFT using cymatic functions
    // Note: This is a simplified approach - full FFT would use cymatic_simulate_wave
    // For now, we detect oscillation by looking for repeated patterns
    
    // Count peaks in the signal
    size_t peak_count = 0;
    for (size_t i = 1; i < num_anchors - 1; i++) {
        if (signal[i] > signal[i-1] && signal[i] > signal[i+1]) {
            peak_count++;
        }
    }
    
    // Count unique peak heights (quantized to 0.1 intervals)
    bool peak_heights[10] = {false};  // 10 bins for [0, 1] range
    for (size_t i = 1; i < num_anchors - 1; i++) {
        if (signal[i] > signal[i-1] && signal[i] > signal[i+1]) {
            int bin = (int)(signal[i] * 10.0);
            if (bin >= 10) bin = 9;
            if (bin < 0) bin = 0;
            peak_heights[bin] = true;
        }
    }
    
    size_t unique_peaks = 0;
    for (int i = 0; i < 10; i++) {
        if (peak_heights[i]) unique_peaks++;
    }
    
    free(signal);
    
    // Oscillation detected if we have repeated peak patterns
    // (fewer unique peaks than total peaks indicates repetition)
    if (peak_count >= 2 && unique_peaks < peak_count / 2) {
        return true;
    }
    
    return false;
}
