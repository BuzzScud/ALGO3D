#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdlib.h>
#include <string.h>
#include "math/arithmetic.h"
#include "math/transcendental.h"
#include "../include/prime_hyperdim.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"

// Convert prime to hyperdimensional vector
HyperVector prime_to_hypervector(int prime) {
    HyperVector hv;
    hv.dim = 10; // Fixed dimension for simplicity
    hv.components = malloc(sizeof(double) * hv.dim);
    
    if (hv.components) {
        // Initialize components based on prime properties
        for (int i = 0; i < hv.dim; i++) {
            hv.components[i] = prime * (1.0 + 0.1 * i) * math_sin(prime + i);
        }
    }
    
    return hv;
}

// Free hypervector memory
void hypervector_free(HyperVector* hv) {
    if (hv && hv->components) {
        free(hv->components);
        hv->components = NULL;
        hv->dim = 0;
    }
}

// Calculate hypervector magnitude
double hypervector_magnitude(const HyperVector* hv) {
    if (!hv || hv->dim <= 0 || !hv->components) return 0.0;
    
    double sum = 0.0;
    for (int i = 0; i < hv->dim; i++) {
        sum += hv->components[i] * hv->components[i];
    }
    
    return math_sqrt(sum);
}

// Add two hypervectors
HyperVector hypervector_add(const HyperVector* a, const HyperVector* b) {
    HyperVector result;
    
    if (!a || !b || a->dim != b->dim || !a->components || !b->components) {
        result.dim = 0;
        result.components = NULL;
        return result;
    }
    
    result.dim = a->dim;
    result.components = malloc(sizeof(double) * result.dim);
    
    if (result.components) {
        for (int i = 0; i < result.dim; i++) {
            result.components[i] = a->components[i] + b->components[i];
        }
    }
    
    return result;
}

// Subtract two hypervectors
HyperVector hypervector_subtract(const HyperVector* a, const HyperVector* b) {
    HyperVector result;
    
    if (!a || !b || a->dim != b->dim || !a->components || !b->components) {
        result.dim = 0;
        result.components = NULL;
        return result;
    }
    
    result.dim = a->dim;
    result.components = malloc(sizeof(double) * result.dim);
    
    if (result.components) {
        for (int i = 0; i < result.dim; i++) {
            result.components[i] = a->components[i] - b->components[i];
        }
    }
    
    return result;
}

// Calculate dot product
double hypervector_dot(const HyperVector* a, const HyperVector* b) {
    if (!a || !b || a->dim != b->dim) return 0.0;
    
    double dot = 0.0;
    for (int i = 0; i < a->dim; i++) {
        dot += a->components[i] * b->components[i];
    }
    
    return dot;
}

// Hyperdimensional sine function
double hyper_sin(double x, int dimension) {
    if (dimension <= 0) dimension = 3; // Default to 3D
    
    // Base sine with dimensional modulation
    double result = math_sin(x);
    
    // Apply dimensional correction based on prime patterns
    for (int d = 2; d <= dimension && d <= 7; d++) {
        if (x > 0) {
            result += 0.1 * math_sin(x * d) / d;
        }
    }
    
    return result;
}

// Hyperdimensional cosine function
double hyper_cos(double x, int dimension) {
    if (dimension <= 0) dimension = 3; // Default to 3D
    
    // Base cosine with dimensional modulation
    double result = math_cos(x);
    
    // Apply dimensional correction
    for (int d = 2; d <= dimension && d <= 7; d++) {
        if (x > 0) {
            result += 0.1 * math_cos(x * d) / d;
        }
    }
    
    return result;
}

// Project hypervector to 2D
void hypervector_project_2d(const HyperVector* hv, double* x, double* y) {
    if (!hv || !x || !y) {
        if (x) *x = 0;
        if (y) *y = 0;
        return;
    }
    
    // Simple projection: use first two components with weighting
    *x = hv->components[0];
    *y = (hv->dim > 1) ? hv->components[1] : 0;
    
    // Add influence from higher dimensions
    for (int i = 2; i < hv->dim; i++) {
        double weight = 1.0 / (i + 1);
        *x += weight * hv->components[i] * math_cos(i);
        *y += weight * hv->components[i] * math_sin(i);
    }
}

// Calculate hypervector angle in 2D projection
double hypervector_angle_2d(const HyperVector* hv) {
    double x, y;
    hypervector_project_2d(hv, &x, &y);
    
    return prime_atan2(y, x);
}

// Normalize hypervector
HyperVector hypervector_normalize(const HyperVector* hv) {
    HyperVector result;
    
    if (!hv || hv->dim <= 0 || !hv->components) {
        result.dim = 0;
        result.components = NULL;
        return result;
    }
    
    result.dim = hv->dim;
    result.components = malloc(sizeof(double) * result.dim);
    
    if (result.components) {
        double mag = hypervector_magnitude(hv);
        
        if (mag > 0) {
            for (int i = 0; i < result.dim; i++) {
                result.components[i] = hv->components[i] / mag;
            }
        } else {
            // Zero vector remains zero
            for (int i = 0; i < result.dim; i++) {
                result.components[i] = 0.0;
            }
        }
    }
    
    return result;
}

// Scale hypervector
HyperVector hypervector_scale(const HyperVector* hv, double scale) {
    HyperVector result;
    
    if (!hv || hv->dim <= 0 || !hv->components) {
        result.dim = 0;
        result.components = NULL;
        return result;
    }
    
    result.dim = hv->dim;
    result.components = malloc(sizeof(double) * result.dim);
    
    if (result.components) {
        for (int i = 0; i < result.dim; i++) {
            result.components[i] = hv->components[i] * scale;
        }
    }
    
    return result;
}

// Calculate distance between two hypervectors
double hypervector_distance(const HyperVector* a, const HyperVector* b) {
    if (!a || !b || a->dim != b->dim) return -1.0;
    
    double sum = 0.0;
    for (int i = 0; i < a->dim; i++) {
        double diff = a->components[i] - b->components[i];
        sum += diff * diff;
    }
    
    return math_sqrt(sum);
}

// Check if two hypervectors are similar (within threshold)
int hypervector_similar(const HyperVector* a, const HyperVector* b, double threshold) {
    double dist = hypervector_distance(a, b);
    return (dist >= 0 && dist <= threshold);
}

// Create hypervector from components
HyperVector hypervector_create(double* components, int dim) {
    HyperVector hv;
    hv.dim = (dim > 0 && dim <= 50) ? dim : 10; // Limit max dimension
    hv.components = malloc(sizeof(double) * hv.dim);
    
    if (hv.components) {
        for (int i = 0; i < hv.dim; i++) {
            hv.components[i] = (components && i < dim) ? components[i] : 0.0;
        }
    }
    
    return hv;
}

// Cross product for 3D hypervectors
HyperVector hypervector_cross_3d(const HyperVector* a, const HyperVector* b) {
    HyperVector result;
    result.dim = 3;
    result.components = malloc(sizeof(double) * 3);
    
    if (!a || !b || a->dim < 3 || b->dim < 3 || !a->components || !b->components || !result.components) {
        if (result.components) {
            for (int i = 0; i < 3; i++) {
                result.components[i] = 0.0;
            }
        }
        return result;
    }
    
    // Standard 3D cross product
    result.components[0] = a->components[1] * b->components[2] - a->components[2] * b->components[1];
    result.components[1] = a->components[2] * b->components[0] - a->components[0] * b->components[2];
    result.components[2] = a->components[0] * b->components[1] - a->components[1] * b->components[0];
    
    return result;
}