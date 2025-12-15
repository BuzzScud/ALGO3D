/**
 * @file tensor_ops.c
 * @brief Implementation of general tensor operations
 * 
 * PHASE 1 WEEK 2: Migrated to NEW math library
 * - Replaced math_pow with math_pow (1 call)
 * Total: 1 function call migrated to NEW math library
 */

#include "tensor_ops.h"
#include "math/arithmetic.h"       // PHASE 1: NEW math library
#include "math/transcendental.h"   // For math_pow
#include "math/types.h"
#include <stdlib.h>

double tensor_distance(const double* t1, const double* t2,
                       size_t dim, uint64_t p_mod) {
    if (!t1 || !t2 || dim == 0 || p_mod == 0) return 0.0;
    
    if (dim == 1) {
        // 1D: Simple difference
        double diff = t1[0] - t2[0];
        if (diff < 0.0) diff = -diff;
        return diff;
    }
    
    if (dim == 2) {
        // 2D: Determinant with golden ratio geometry
        // det(t1 - t2) = (dx * gy - dy * gx) mod p
        // where g = (φ, φ²)
        
        double dx = t1[0] - t2[0];
        double dy = t1[1] - t2[1];
        
        double gx = MATH_PHI;
        double gy = MATH_PHI * MATH_PHI;
        
        double det = dx * gy - dy * gx;
        
        // Apply modulus
        if (det < 0.0) det = -det;
        
        // Modular reduction
        while (det >= (double)p_mod) {
            det -= (double)p_mod;
        }
        
        return det;
    }
    
    // N-D: Generalized distance using golden ratio scaling
    double sum = 0.0;
    double phi_power = 1.0;
    
    for (size_t i = 0; i < dim; i++) {
        double diff = t1[i] - t2[i];
        if (diff < 0.0) diff = -diff;
        
        // Scale by φ^i
        sum += diff * phi_power;
        phi_power *= MATH_PHI;
    }
    
    // Apply modulus
    while (sum >= (double)p_mod) {
        sum -= (double)p_mod;
    }
    
    return sum;
}

void tensor_generate_anchors(double* anchors, size_t num_anchors,
                              size_t dim, uint64_t seed) {
    if (!anchors || num_anchors == 0 || dim == 0) return;
    
    // Use golden ratio for deterministic "random" generation
    double phi = MATH_PHI;
    uint64_t state = seed;
    
    for (size_t a = 0; a < num_anchors; a++) {
        for (size_t d = 0; d < dim; d++) {
            // Generate pseudo-random value using golden ratio
            state = state * 1103515245 + 12345;  // LCG
            double t = (double)(state % 1000000) / 1000000.0;
            
            // Scale by golden ratio
            double phi_scale = math_pow(phi, (double)d);
            anchors[a * dim + d] = t * phi_scale;
        }
    }
}

uint64_t tensor_estimate_from_anchors(const double* query,
                                       const double* anchors,
                                       size_t num_anchors,
                                       size_t dim,
                                       uint64_t p_mod) {
    if (!query || !anchors || num_anchors == 0 || dim == 0 || p_mod == 0) {
        return 0;
    }
    
    // Find closest anchor
    size_t closest_idx = 0;
    double min_distance = tensor_distance(query, anchors, dim, p_mod);
    
    for (size_t a = 1; a < num_anchors; a++) {
        const double* anchor = &anchors[a * dim];
        double dist = tensor_distance(query, anchor, dim, p_mod);
        
        if (dist < min_distance) {
            min_distance = dist;
            closest_idx = a;
        }
    }
    
    // Estimate value based on anchor index and distance
    // Use golden ratio for mapping
    double phi = MATH_PHI;
    double estimate = (double)closest_idx * phi + min_distance;
    
    // Convert to uint64_t with modulus
    uint64_t result = (uint64_t)estimate % p_mod;
    
    return result;
}

void tensor_outer_product(const double* a, size_t a_len,
                          const double* b, size_t b_len,
                          double* result) {
    if (!a || !b || !result || a_len == 0 || b_len == 0) return;
    
    // Compute: result[i,j] = a[i] * b[j]
    for (size_t i = 0; i < a_len; i++) {
        for (size_t j = 0; j < b_len; j++) {
            result[i * b_len + j] = a[i] * b[j];
        }
    }
}
