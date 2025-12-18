#include "../include/tetration_attractors.h"
#include <stdlib.h>
#include <stdio.h>

// Prime math functions (crystalline library - NO math.h!)
extern double math_sqrt(double x);
extern double math_pow(double x, double y);

// Inline fabs for double precision
static inline double math_abs(double x) {
    return (x < 0.0) ? -x : x;
}

// Inline abs for uint64_t
static inline uint64_t u64_abs_diff(uint64_t a, uint64_t b) {
    return (a > b) ? (a - b) : (b - a);
}

uint64_t euler_totient(uint64_t n) {
    if (n == 1) return 1;
    
    uint64_t result = n;
    uint64_t p = 2;
    uint64_t n_copy = n;
    
    // Factor n and compute φ(n)
    while (p * p <= n_copy) {
        if (n_copy % p == 0) {
            // Remove all factors of p
            while (n_copy % p == 0) {
                n_copy /= p;
            }
            // φ(n) = n * (1 - 1/p) for each prime factor
            result -= result / p;
        }
        p++;
    }
    
    // If n_copy > 1, then it's a prime factor
    if (n_copy > 1) {
        result -= result / n_copy;
    }
    
    return result;
}

// Modular exponentiation: base^exp mod modulus
static uint64_t mod_pow(uint64_t base, uint64_t exp, uint64_t modulus) {
    if (modulus == 1) return 0;
    
    uint64_t result = 1;
    base = base % modulus;
    
    while (exp > 0) {
        if (exp % 2 == 1) {
            result = (result * base) % modulus;
        }
        exp = exp >> 1;
        base = (base * base) % modulus;
    }
    
    return result;
}

uint64_t modular_tetration(uint32_t base, uint32_t height, uint64_t modulus) {
    if (height == 0) return 1;
    if (height == 1) return base % modulus;
    if (modulus == 1) return 0;
    
    // Compute φ(modulus) for exponent reduction
    uint64_t phi = euler_totient(modulus);
    
    // Recursively compute exponent
    uint64_t exp = modular_tetration(base, height - 1, phi);
    
    // Compute base^exp mod modulus
    return mod_pow(base, exp, modulus);
}

uint64_t tetration_with_damping(
    uint32_t base,
    uint32_t height,
    double damping,
    uint64_t modulus
) {
    if (height == 0) return 1;
    if (height == 1) return base % modulus;
    if (modulus == 1) return 0;
    
    uint64_t phi = euler_totient(modulus);
    uint64_t current_exp = base;
    
    // Build tower from bottom up with damping
    for (uint32_t h = 1; h < height; h++) {
        // Apply damping (entropy reduction)
        current_exp = (uint64_t)((double)current_exp * damping);
        current_exp = current_exp % phi;
        
        // Modular exponentiation
        current_exp = mod_pow(base, current_exp, modulus);
    }
    
    return current_exp;
}

AttractorSystem* create_attractor_system(uint64_t modulus, double damping) {
    AttractorSystem* system = malloc(sizeof(AttractorSystem));
    if (!system) return NULL;
    
    // Standard configuration: 6 bases × 3 heights = 18 attractors
    uint32_t bases[] = {2, 3, 5, 7, 11, 13};
    uint32_t heights[] = {2, 3, 4};
    
    system->num_attractors = 6 * 3;
    system->modulus = modulus;
    system->default_damping = damping;
    
    system->attractors = malloc(system->num_attractors * sizeof(TetrationAttractor));
    if (!system->attractors) {
        free(system);
        return NULL;
    }
    
    // Initialize attractors
    uint32_t idx = 0;
    for (uint32_t b = 0; b < 6; b++) {
        for (uint32_t h = 0; h < 3; h++) {
            system->attractors[idx].base = bases[b];
            system->attractors[idx].height = heights[h];
            system->attractors[idx].damping = damping;
            system->attractors[idx].modulus = modulus;
            system->attractors[idx].value = 0;  // Computed later
            idx++;
        }
    }
    
    return system;
}

void free_attractor_system(AttractorSystem* system) {
    if (!system) return;
    if (system->attractors) free(system->attractors);
    free(system);
}

void compute_all_attractors(AttractorSystem* system) {
    if (!system || !system->attractors) return;
    
    for (uint32_t i = 0; i < system->num_attractors; i++) {
        TetrationAttractor* attr = &system->attractors[i];
        
        attr->value = tetration_with_damping(
            attr->base,
            attr->height,
            attr->damping,
            attr->modulus
        );
    }
}

TetrationAttractor* find_nearest_attractor(AttractorSystem* system, uint64_t target) {
    if (!system || !system->attractors || system->num_attractors == 0) return NULL;
    
    TetrationAttractor* nearest = &system->attractors[0];
    uint64_t min_dist = u64_abs_diff(nearest->value, target);
    
    for (uint32_t i = 1; i < system->num_attractors; i++) {
        uint64_t dist = u64_abs_diff(system->attractors[i].value, target);
        if (dist < min_dist) {
            min_dist = dist;
            nearest = &system->attractors[i];
        }
    }
    
    return nearest;
}

uint32_t find_n_nearest_attractors(
    AttractorSystem* system,
    uint64_t target,
    uint32_t n,
    TetrationAttractor** out_attractors
) {
    if (!system || !system->attractors || !out_attractors || n == 0) return 0;
    
    // Create array of all attractors with distances
    typedef struct {
        TetrationAttractor* attr;
        uint64_t distance;
    } AttrDist;
    
    AttrDist* dists = malloc(system->num_attractors * sizeof(AttrDist));
    if (!dists) return 0;
    
    // Compute distances
    for (uint32_t i = 0; i < system->num_attractors; i++) {
        dists[i].attr = &system->attractors[i];
        dists[i].distance = u64_abs_diff(system->attractors[i].value, target);
    }
    
    // Sort by distance (simple bubble sort for small arrays)
    for (uint32_t i = 0; i < system->num_attractors; i++) {
        for (uint32_t j = i + 1; j < system->num_attractors; j++) {
            if (dists[j].distance < dists[i].distance) {
                AttrDist temp = dists[i];
                dists[i] = dists[j];
                dists[j] = temp;
            }
        }
    }
    
    // Copy top N
    uint32_t num_found = (n < system->num_attractors) ? n : system->num_attractors;
    for (uint32_t i = 0; i < num_found; i++) {
        out_attractors[i] = dists[i].attr;
    }
    
    free(dists);
    return num_found;
}

double compute_attractor_bias(uint64_t value, TetrationAttractor* attractor) {
    if (!attractor) return 0.0;
    
    uint64_t dist = u64_abs_diff(value, attractor->value);
    
    // Bias = 1 / (1 + distance)
    // Closer to attractor = higher bias
    double bias = 1.0 / (1.0 + (double)dist);
    
    return bias;
}

double adjust_damping(double current_damping, double convergence_rate) {
    // If converging (rate > 0): decrease damping (allow exploration)
    // If diverging (rate < 0): increase damping (force convergence)
    
    double adjustment = convergence_rate * 0.01;  // Small adjustment
    double new_damping = current_damping - adjustment;
    
    // Clamp to [0.8, 0.99]
    if (new_damping < 0.8) new_damping = 0.8;
    if (new_damping > 0.99) new_damping = 0.99;
    
    return new_damping;
}

void print_attractor(TetrationAttractor* attr) {
    if (!attr) return;
    
    printf("  Base: %u, Height: %u, Value: %lu, Damping: %.3f\n",
           attr->base, attr->height, attr->value, attr->damping);
}

void print_attractor_system(AttractorSystem* system) {
    if (!system) return;
    
    printf("Attractor System:\n");
    printf("  Modulus: %lu\n", system->modulus);
    printf("  Default Damping: %.3f\n", system->default_damping);
    printf("  Number of Attractors: %u\n\n", system->num_attractors);
    
    printf("Attractors:\n");
    for (uint32_t i = 0; i < system->num_attractors; i++) {
        printf("  [%2u] ", i);
        print_attractor(&system->attractors[i]);
    }
}
