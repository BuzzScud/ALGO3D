#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdlib.h>
#include <string.h>
#include "math/arithmetic.h"
#include "math/transcendental.h"
#include "../include/prime_coords.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"
#include "../include/crystal_abacus.h"

// Ulam spiral coordinates
void ulam_spiral_coords(int n, double* x, double* y) {
    if (!x || !y) {
        if (x) *x = 0;
        if (y) *y = 0;
        return;
    }
    
    if (n <= 1) {
        *x = 0;
        *y = 0;
        return;
    }
    
    // Find the layer (square ring) containing n
    int layer = (int)math_ceil((math_sqrt(n) - 1) / 2);
    int max_in_layer = (2 * layer + 1) * (2 * layer + 1);
    int side_length = 2 * layer;
    
    // Position in the current layer
    int pos_in_layer = max_in_layer - n;
    
    // Calculate coordinates based on side
    int side = pos_in_layer / side_length;
    int offset = pos_in_layer % side_length;
    
    switch (side) {
        case 0: // Right side (moving down)
            *x = layer;
            *y = layer - offset;
            break;
        case 1: // Bottom side (moving left)
            *x = layer - offset;
            *y = -layer;
            break;
        case 2: // Left side (moving up)
            *x = -layer;
            *y = -layer + offset;
            break;
        case 3: // Top side (moving right)
            *x = -layer + offset;
            *y = layer;
            break;
        default:
            *x = 0;
            *y = 0;
    }
}

// Golden spiral coordinates
void golden_spiral_coords(int n, double* x, double* y) {
    if (!x || !y) {
        if (x) *x = 0;
        if (y) *y = 0;
        return;
    }
    
    if (n <= 0) {
        *x = 0;
        *y = 0;
        return;
    }
    
    // Golden spiral: r = a * e^(b * θ)
    double theta = n * 0.5; // Angular increment
    double r = math_pow(PHI, n / 10.0); // Radial distance using golden ratio
    
    *x = r * math_cos(theta);
    *y = r * math_sin(theta);
}

// Archimedes spiral coordinates
void archimedes_spiral_coords(double angle, double spacing, double* x, double* y) {
    if (!x || !y) {
        if (x) *x = 0;
        if (y) *y = 0;
        return;
    }
    
    // Archimedes spiral: r = a * θ
    double r = spacing * angle;
    
    *x = r * math_cos(angle);
    *y = r * math_sin(angle);
}

// Logarithmic spiral coordinates
void log_spiral_coords(double a, double b, double angle, double* x, double* y) {
    if (!x || !y) {
        if (x) *x = 0;
        if (y) *y = 0;
        return;
    }
    
    // Logarithmic spiral: r = a * e^(b * θ)
    double r = a * math_exp(b * angle);
    
    *x = r * math_cos(angle);
    *y = r * math_sin(angle);
}

// Cartesian to polar conversion
void cartesian_to_polar(double x, double y, double* r, double* theta) {
    if (!r || !theta) {
        if (r) *r = 0;
        if (theta) *theta = 0;
        return;
    }
    
    *r = math_sqrt(x * x + y * y);
    *theta = prime_atan2(y, x);
    
    // Normalize theta to [0, 2π]
    if (*theta < 0) {
        *theta += 2 * MATH_PI;
    }
}

// Polar to cartesian conversion
void polar_to_cartesian(double r, double theta, double* x, double* y) {
    if (!x || !y) {
        if (x) *x = 0;
        if (y) *y = 0;
        return;
    }
    
    *x = r * math_cos(theta);
    *y = r * math_sin(theta);
}

// 3D Cartesian to spherical conversion
void cartesian_to_spherical(double x, double y, double z, double* r, double* theta, double* phi) {
    if (!r || !theta || !phi) {
        if (r) *r = 0;
        if (theta) *theta = 0;
        if (phi) *phi = 0;
        return;
    }
    
    *r = math_sqrt(x * x + y * y + z * z);
    *theta = prime_atan2(y, x);
    *phi = math_acos(z / (*r));
}

// Map number to clock position
void map_to_clock_position(int number, int max_numbers, double* angle, double* radius) {
    if (!angle || !radius || max_numbers <= 0) {
        if (angle) *angle = 0;
        if (radius) *radius = 0;
        return;
    }
    
    // Clock angles start from 12 o'clock (π/2) and go clockwise
    double clock_angle = MATH_PI / 2 - (2 * MATH_PI * number / max_numbers);
    
    // Normalize to [0, 2π]
    while (clock_angle < 0) clock_angle += 2 * MATH_PI;
    while (clock_angle >= 2 * MATH_PI) clock_angle -= 2 * MATH_PI;
    
    *angle = clock_angle;
    *radius = 1.0 + 0.1 * (number % 12); // Vary radius slightly
}

// Quadratic mirror fold transformation
void quadratic_mirror_fold(double x, double y, double fold_amount, double* new_x, double* new_y) {
    if (!new_x || !new_y) {
        if (new_x) *new_x = 0;
        if (new_y) *new_y = 0;
        return;
    }
    
    // Apply quadratic folding
    double quad_x = x * x * fold_amount;
    double quad_y = y * y * fold_amount;
    
    // Mirror folding with symmetry
    *new_x = (x > 0) ? (x - quad_x) : (x + quad_x);
    *new_y = (y > 0) ? (y - quad_y) : (y + quad_y);
    
    // Additional prime-based modulation
    int prime_mod = 2;
    while (prime_mod * prime_mod <= math_abs(x) + math_abs(y)) {
        if (math_fmod(math_abs(x) + math_abs(y), prime_mod) < 1.0) {
            *new_x *= (1.0 + 0.01 / prime_mod);
            *new_y *= (1.0 + 0.01 / prime_mod);
        }
        prime_mod++;
        if (prime_mod > 50) break;
    }
}

// Spiral collapse transformation
void spiral_collapse(double x, double y, double collapse_rate, double* new_x, double* new_y) {
    if (!new_x || !new_y) {
        if (new_x) *new_x = 0;
        if (new_y) *new_y = 0;
        return;
    }
    
    // Convert to polar
    double r, theta;
    cartesian_to_polar(x, y, &r, &theta);
    
    // Apply spiral collapse
    double new_r = r * math_exp(-collapse_rate);
    double new_theta = theta + collapse_rate * 2; // Spiral inward
    
    // Convert back to cartesian
    polar_to_cartesian(new_r, new_theta, new_x, new_y);
    
    // Add prime-based perturbation
    int prime_factor = 2;
    while (prime_factor < 20) {
        if (math_fmod(r, prime_factor) < collapse_rate) {
            *new_x += 0.01 * math_sin(prime_factor * theta);
            *new_y += 0.01 * math_cos(prime_factor * theta);
        }
        prime_factor++;
    }
}

// BigInt Ulam spiral coordinates
void big_ulam_spiral_coords(BigInt* n, double* x, double* y) {
    if (!n || !x || !y) {
        if (x) *x = 0;
        if (y) *y = 0;
        return;
    }
    
    // Convert BigInt to long for calculation
    char* str = big_to_string(n);
    if (!str) {
        *x = 0;
        *y = 0;
        return;
    }
    
    long n_val = atol(str);
    free(str);
    
    if (n_val <= 1000000) { // Handle reasonably sized numbers
        ulam_spiral_coords((int)n_val, x, y);
    } else {
        // For very large numbers, use modular arithmetic
        int reduced_n = (int)(n_val % 10000);
        ulam_spiral_coords(reduced_n, x, y);
        
        // Scale up for large numbers
        double scale = math_log(n_val) / 10.0;
        *x *= scale;
        *y *= scale;
    }
}

// Coordinate transformation with prime modulation
void prime_transform_coords(double x, double y, int prime, double* new_x, double* new_y) {
    if (!new_x || !new_y || prime <= 1) {
        if (new_x) *new_x = x;
        if (new_y) *new_y = y;
        return;
    }
    
    // Apply rotation based on prime
    double angle = (2 * MATH_PI * prime) / (prime + 1);
    double cos_a = math_cos(angle);
    double sin_a = math_sin(angle);
    
    // Rotate coordinates
    *new_x = x * cos_a - y * sin_a;
    *new_y = x * sin_a + y * cos_a;
    
    // Apply scaling based on prime properties
    double scale = 1.0 + 0.1 * math_sin(prime);
    *new_x *= scale;
    *new_y *= scale;
}

// Fold coordinates onto a torus
void torus_fold_coords(double x, double y, double major_radius, double minor_radius, double* new_x, double* new_y) {
    if (!new_x || !new_y) {
        if (new_x) *new_x = 0;
        if (new_y) *new_y = 0;
        return;
    }
    
    // Map coordinates to torus surface
    double theta = x / major_radius;
    double phi = y / minor_radius;
    
    // Torus parametric equations (2D projection)
    *new_x = (major_radius + minor_radius * math_cos(phi)) * math_cos(theta);
    *new_y = (major_radius + minor_radius * math_cos(phi)) * math_sin(theta);
}

// Hilbert curve coordinate mapping
void hilbert_coords(int index, int order, double* x, double* y) {
    if (!x || !y || order <= 0 || index < 0) {
        if (x) *x = 0;
        if (y) *y = 0;
        return;
    }
    
    // Simple approximation of Hilbert curve
    int n = 1 << order; // Grid size
    int max_index = n * n - 1;
    
    if (index > max_index) index = max_index;
    
    // Basic implementation (not true Hilbert, but good approximation)
    int x_int = index % n;
    int y_int = index / n;
    
    // Apply some folding to create more curve-like behavior
    if ((y_int % 2) == 1) {
        x_int = n - 1 - x_int;
    }
    
    *x = (double)x_int;
    *y = (double)y_int;
}