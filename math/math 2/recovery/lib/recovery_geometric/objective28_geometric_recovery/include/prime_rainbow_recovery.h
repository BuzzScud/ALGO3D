/**
 * prime_rainbow_recovery.h - Geometric Recovery Using Prime Rainbow Tables
 * 
 * Integrates the prime-based mathematical framework with geometric recovery.
 * Uses rainbow table mapping: prime → (angle, radius, frequency, layer)
 * 
 * Key principles:
 * - NO math.h - uses ONLY prime_* functions
 * - 7-layer folding for self-similar structure
 * - Golden ratio (φ) spiral mapping
 * - Cymatic frequency resonance (432 Hz base)
 * - Modular tetration for bounding
 */

#ifndef PRIME_RAINBOW_RECOVERY_H
#define PRIME_RAINBOW_RECOVERY_H

#include <stdint.h>
#include <stdbool.h>

// Prime-based math functions (from crystalline framework)
static inline double prime_multiply(double a, double b);
static inline double prime_divide(double a, double b);
static inline double prime_add(double a, double b);
static inline double prime_subtract(double a, double b);
static inline double math_sqrt(double x);
static inline double math_sin(double x);
static inline double math_cos(double x);
static inline double math_fmod(double x, double y);
static inline double math_abs(double x);

// Constants
#define PHI 1.61803398874989484820  // Golden ratio
#define PI 3.14159265358979323846
#define BASE_FREQ 432.0              // Cymatic base frequency

// Rainbow table entry (from framework)
typedef struct {
    int prime;
    double angle;        // θ = prime × φ
    double radius;       // r = √prime × 10
    double frequency;    // 432 Hz × 2^(prime/12)
    int layer;           // Layer 0-6 (7-fold)
    double fold_x;       // Folded x coordinate
    double fold_y;       // Folded y coordinate
    double vector_x;     // Tangent vector x
    double vector_y;     // Tangent vector y
} PrimeRainbowEntry;

// Rainbow table
typedef struct {
    PrimeRainbowEntry* entries;
    int count;
    int capacity;
    double fold_progression[7];  // 7-layer folding
} PrimeRainbowTable;

// Recovery context using rainbow tables
typedef struct {
    PrimeRainbowTable* rainbow_table;
    uint64_t min_k;
    uint64_t max_k;
    
    // Search configuration
    int num_layers;
    int64_t search_ranges[3];   // Coarse, medium, fine
    uint64_t search_steps[3];
    
    // Statistics
    uint64_t total_searches;
    uint64_t successful_searches;
} PrimeRainbowRecoveryContext;

/**
 * Initialize prime rainbow table
 * Generates rainbow entries for primes up to limit
 */
PrimeRainbowTable* init_prime_rainbow_table(int max_primes);

/**
 * Free rainbow table
 */
void free_prime_rainbow_table(PrimeRainbowTable* table);

/**
 * Add prime to rainbow table
 * Computes: angle, radius, frequency, layer, fold coordinates
 */
void add_prime_to_rainbow(PrimeRainbowTable* table, int prime);

/**
 * Apply 7-layer folding to rainbow table
 * Creates self-similar fractal structure
 */
void apply_rainbow_folding(PrimeRainbowTable* table, double fold_amount);

/**
 * Initialize recovery context with rainbow table
 */
PrimeRainbowRecoveryContext* init_prime_rainbow_recovery(
    uint64_t min_k,
    uint64_t max_k,
    int num_primes
);

/**
 * Free recovery context
 */
void free_prime_rainbow_recovery(PrimeRainbowRecoveryContext* ctx);

/**
 * Find nearest rainbow entries to target angle
 * Uses prime-based distance metric
 */
int find_nearest_rainbow_entries(
    PrimeRainbowTable* table,
    double target_angle,
    PrimeRainbowEntry* nearest[3]
);

/**
 * Compute weighted k estimate from rainbow entries
 * Uses 7-layer folding and cymatic frequencies
 */
uint64_t compute_rainbow_k_estimate(
    PrimeRainbowEntry* entries[3],
    int num_entries,
    double target_angle
);

/**
 * Recover k using prime rainbow tables
 * Main recovery function with 7-layer folding
 */
uint64_t recover_k_with_rainbow(
    double target_angle,
    PrimeRainbowRecoveryContext* ctx,
    double* confidence
);

/**
 * Modular tetration for bounding
 * Proves infinite towers stabilize
 */
uint64_t modular_tetration(uint64_t base, int height, uint64_t mod);

/**
 * Compute cymatic resonance score
 * Measures alignment with 432 Hz harmonics
 */
double compute_cymatic_resonance(
    double frequency,
    int harmonic_depth
);

#endif // PRIME_RAINBOW_RECOVERY_H
