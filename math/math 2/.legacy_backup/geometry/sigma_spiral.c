/**
 * @file sigma_spiral.c
 * @brief Implementation of sigma spiral using golden ratio iteration
 */

#include "sigma_spiral.h"
#include "math/types.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"
#include <stdlib.h>
#include <string.h>

double sigma_spiral(uint32_t iter, uint64_t p_mod) {
    if (p_mod == 0) {
        p_mod = P_MOD_32;  // Default: 2^32 + 13
    }
    
    // σ(iter) = (φ^iter) mod p_mod
    double value = math_pow(PHI, (double)iter);
    
    // Apply modular reduction
    while (value >= (double)p_mod) {
        value -= (double)p_mod;
    }
    
    return value;
}

void sigma_spiral_sequence(double* output, size_t len, uint64_t p_mod) {
    if (!output || len == 0) return;
    
    if (p_mod == 0) {
        p_mod = P_MOD_32;
    }
    
    for (size_t i = 0; i < len; i++) {
        output[i] = sigma_spiral((uint32_t)i, p_mod);
    }
}

void sigma_spiral_2d(uint32_t* x_coords, uint32_t* y_coords,
                     size_t len, uint32_t grid_size, uint64_t p_mod) {
    if (!x_coords || !y_coords || len == 0 || grid_size == 0) return;
    
    if (p_mod == 0) {
        p_mod = P_MOD_32;
    }
    
    for (size_t i = 0; i < len; i++) {
        double value = sigma_spiral((uint32_t)i, p_mod);
        
        // Map to 2D coordinates using modular arithmetic
        uint64_t int_value = (uint64_t)value;
        x_coords[i] = (uint32_t)(int_value % grid_size);
        y_coords[i] = (uint32_t)((int_value / grid_size) % grid_size);
    }
}

double sigma_spiral_custom_base(uint32_t iter, double base, uint64_t p_mod) {
    if (p_mod == 0) {
        p_mod = P_MOD_32;
    }
    
    // σ(iter, base) = (base^iter) mod p_mod
    double value = math_pow(base, (double)iter);
    
    // Apply modular reduction
    while (value >= (double)p_mod) {
        value -= (double)p_mod;
    }
    
    return value;
}

void sigma_spiral_analyze(const double* sequence, size_t len,
                          double* mean, double* variance, double* coverage) {
    if (!sequence || len == 0) return;
    
    // Calculate mean
    double sum = 0.0;
    for (size_t i = 0; i < len; i++) {
        sum += sequence[i];
    }
    if (mean) {
        *mean = sum / (double)len;
    }
    
    // Calculate variance
    if (variance) {
        double mean_val = sum / (double)len;
        double var_sum = 0.0;
        
        for (size_t i = 0; i < len; i++) {
            double diff = sequence[i] - mean_val;
            var_sum += diff * diff;
        }
        
        *variance = var_sum / (double)len;
    }
    
    // Calculate coverage (unique values / total)
    if (coverage) {
        // Simple approximation: count distinct values
        // For large sequences, this is expensive, so we sample
        size_t sample_size = (len > 1000) ? 1000 : len;
        size_t unique_count = 0;
        
        for (size_t i = 0; i < sample_size; i++) {
            bool is_unique = true;
            for (size_t j = 0; j < i; j++) {
                if (sequence[i] == sequence[j]) {
                    is_unique = false;
                    break;
                }
            }
            if (is_unique) {
                unique_count++;
            }
        }
        
        *coverage = (double)unique_count / (double)sample_size;
    }
}