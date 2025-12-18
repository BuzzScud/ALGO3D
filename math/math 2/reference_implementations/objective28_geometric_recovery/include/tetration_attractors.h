#ifndef TETRATION_ATTRACTORS_H
#define TETRATION_ATTRACTORS_H

#include <stdint.h>
#include <stdbool.h>

/**
 * Tetration Attractors for ECDLP Recovery
 * 
 * Implements modular tetration with damping for entropy reduction.
 * Based on compression example and master plan spec.
 * 
 * Key concepts:
 * - Tetration: base^^height = base^(base^(base^...))
 * - Attractors: Natural convergence points
 * - Damping: Entropy reduction factor (0.95 typical)
 * - Euler totient: For modular reduction
 * 
 * Uses ONLY prime_* functions (NO math.h)
 */

// Tetration attractor structure
typedef struct {
    uint32_t base;              // Base (2, 3, 5, 7, 11, 13)
    uint32_t height;            // Tower height (2, 3, 4)
    uint64_t value;             // Computed value (mod modulus)
    double damping;             // Entropy reduction factor [0, 1]
    uint64_t modulus;           // Modulus for computation
} TetrationAttractor;

// Attractor system
typedef struct {
    TetrationAttractor* attractors;     // Array of attractors
    uint32_t num_attractors;            // Number of attractors
    uint64_t modulus;                   // Global modulus
    double default_damping;             // Default damping factor
} AttractorSystem;

/**
 * Compute Euler's totient function φ(n)
 * 
 * φ(n) = count of numbers ≤ n that are coprime to n
 * Used for modular exponent reduction
 */
uint64_t euler_totient(uint64_t n);

/**
 * Compute modular tetration: base^^height mod modulus
 * 
 * Uses recursive right-association:
 * base^^3 = base^(base^base)
 * 
 * With Euler totient reduction for efficiency
 */
uint64_t modular_tetration(uint32_t base, uint32_t height, uint64_t modulus);

/**
 * Compute tetration with damping for entropy reduction
 * 
 * Applies damping factor at each level:
 * exp_i = (exp_{i-1} * damping) mod φ(modulus)
 * 
 * Simulates phonon adjustment for convergence
 */
uint64_t tetration_with_damping(
    uint32_t base,
    uint32_t height,
    double damping,
    uint64_t modulus
);

/**
 * Create attractor system with standard configuration
 * 
 * Bases: 2, 3, 5, 7, 11, 13 (6 bases)
 * Heights: 2, 3, 4 (3 heights)
 * Total: 18 attractors
 * 
 * @param modulus Modulus for computation (e.g., 2^256 - 2^32 - 977)
 * @param damping Damping factor (0.95 typical)
 */
AttractorSystem* create_attractor_system(uint64_t modulus, double damping);

/**
 * Free attractor system
 */
void free_attractor_system(AttractorSystem* system);

/**
 * Compute all attractors in system
 */
void compute_all_attractors(AttractorSystem* system);

/**
 * Find nearest attractor to target value
 * 
 * @param system Attractor system
 * @param target Target value
 * @return Nearest attractor (or NULL)
 */
TetrationAttractor* find_nearest_attractor(AttractorSystem* system, uint64_t target);

/**
 * Find N nearest attractors to target
 * 
 * @param system Attractor system
 * @param target Target value
 * @param n Number of attractors to find
 * @param out_attractors Output array (caller must allocate)
 * @return Number of attractors found
 */
uint32_t find_n_nearest_attractors(
    AttractorSystem* system,
    uint64_t target,
    uint32_t n,
    TetrationAttractor** out_attractors
);

/**
 * Compute bias toward attractor
 * 
 * Returns value in [0, 1] indicating how strongly to bias toward attractor
 * Higher bias = closer to attractor
 */
double compute_attractor_bias(uint64_t value, TetrationAttractor* attractor);

/**
 * Adjust damping based on convergence rate
 * 
 * If converging: decrease damping (allow more exploration)
 * If diverging: increase damping (force convergence)
 */
double adjust_damping(double current_damping, double convergence_rate);

/**
 * Print attractor system (for debugging)
 */
void print_attractor_system(AttractorSystem* system);

/**
 * Print single attractor
 */
void print_attractor(TetrationAttractor* attr);

#endif // TETRATION_ATTRACTORS_H