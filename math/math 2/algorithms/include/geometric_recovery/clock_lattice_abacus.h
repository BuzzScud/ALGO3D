/**
 * @file clock_lattice_abacus.h
 * @brief Pure Abacus Clock Lattice Integration for Geometric Recovery
 * 
 * Revolutionary O(1) geometric recovery using clock lattice structure
 * with pure CrystallineAbacus operations (zero floating-point).
 * 
 * KEY INNOVATIONS:
 * - O(1) prime factorization via clock position
 * - Pure Abacus arithmetic (no floating-point errors)
 * - Deterministic geometric validation
 * - base-60 mathematics foundation (12, 60, 60, 100)
 * - Natural phase relationships
 * 
 * MASTER_PLAN COMPLIANCE:
 * - RULE 1: No external math libraries (pure Abacus)
 * - RULE 4: base-60 mathematics foundation
 * - RULE 5: O(1) deterministic operations
 * - RULE 8: Clock lattice validation (position IS the prime)
 */

#ifndef CLOCK_LATTICE_ABACUS_H
#define CLOCK_LATTICE_ABACUS_H

#include "math/abacus.h"
#include "math/clock.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// CORE DATA STRUCTURES
// ============================================================================

/**
 * @brief Clock position using Pure Abacus
 * 
 * Represents a position on the clock lattice lattice with arbitrary precision.
 */
typedef struct {
    uint32_t ring;                    // Ring number (0-3)
    uint32_t position;                // Position on ring
    CrystallineAbacus* angle;         // Angle in radians (base-60)
    CrystallineAbacus* clock_value;   // Normalized [0, 1)
    uint32_t base;                    // Abacus base
} ClockPositionAbacus;

/**
 * @brief Clock factor visualization using Pure Abacus
 * 
 * Visualizes prime factorization on the clock lattice with geometric relationships.
 */
typedef struct {
    // Input
    CrystallineAbacus* n;             // The number (n = p × q)
    CrystallineAbacus* p;             // First prime factor
    CrystallineAbacus* q;             // Second prime factor
    uint32_t p_index;                 // Prime index of p
    uint32_t q_index;                 // Prime index of q
    
    // Positions on clock
    ClockPositionAbacus* p_position;  // p position on clock
    ClockPositionAbacus* q_position;  // q position on clock
    
    // Geometric relationships
    CrystallineAbacus* geometric_distance;  // Distance on clock lattice
    CrystallineAbacus* angular_separation;  // Angular separation
    CrystallineAbacus* ring_distance;       // Distance between rings
    
    // base-60 structure
    uint32_t total_positions;         // Total positions (4,320,000)
    CrystallineAbacus* resolution;    // Angular resolution
    
    uint32_t base;                    // Abacus base
} ClockFactorVisualizationAbacus;

/**
 * @brief Prime factorization using Pure Abacus
 * 
 * Represents complete prime factorization with clock lattice positions.
 */
typedef struct {
    ClockPositionAbacus** positions;  // Clock positions of factors
    uint32_t num_factors;             // Number of factors
    CrystallineAbacus** factors;      // Pure Abacus factors
    uint32_t base;                    // Abacus base
} PrimeFactorizationAbacus;

/**
 * @brief Tetration attractor using Pure Abacus
 * 
 * Represents tetration attractor on clock lattice with geometric convergence.
 */
typedef struct {
    ClockPositionAbacus* position;    // Position on clock
    CrystallineAbacus* value;         // Attractor value
    uint32_t convergence_iterations;  // Iterations to converge (O(1) via geometry)
    CrystallineAbacus* convergence_rate; // Rate of convergence
    uint32_t base;                    // Abacus base
} TetrationAttractorAbacus;

// ============================================================================
// CLOCK POSITION OPERATIONS
// ============================================================================

/**
 * @brief Create clock position
 * 
 * @param ring Ring number (0-3)
 * @param position Position on ring
 * @param base Abacus base
 * @return Clock position or NULL on error
 */
ClockPositionAbacus* create_clock_position_abacus(
    uint32_t ring,
    uint32_t position,
    uint32_t base
);

/**
 * @brief Free clock position
 * 
 * @param pos Clock position to free
 */
void free_clock_position_abacus(ClockPositionAbacus* pos);

/**
 * @brief Map prime to clock position (Pure Abacus)
 * 
 * Uses the math library's clock_map_prime_to_position() for O(1) mapping.
 * 
 * @param prime Prime number (Pure Abacus)
 * @param position Output clock position
 * @param base Abacus base
 * @param precision Precision for angle calculations
 * @return MATH_SUCCESS or error code
 */
MathError map_prime_to_clock_abacus(
    const CrystallineAbacus* prime,
    ClockPositionAbacus* position,
    uint32_t base,
    uint32_t precision
);

/**
 * @brief Map clock position to prime (Pure Abacus)
 * 
 * Uses the math library's clock_position_to_prime() for O(1) mapping.
 * 
 * @param position Clock position
 * @param prime Output prime number
 * @param base Abacus base
 * @return MATH_SUCCESS or error code
 */
MathError map_clock_to_prime_abacus(
    const ClockPositionAbacus* position,
    CrystallineAbacus* prime,
    uint32_t base
);

// ============================================================================
// GEOMETRIC OPERATIONS
// ============================================================================

/**
 * @brief Compute angular separation (Pure Abacus)
 * 
 * Computes the angular separation between two positions on the clock.
 * 
 * @param pos1 First position
 * @param pos2 Second position
 * @param base Abacus base
 * @param precision Precision for calculations
 * @return Angular separation or NULL on error
 */
CrystallineAbacus* compute_angular_separation_abacus(
    const ClockPositionAbacus* pos1,
    const ClockPositionAbacus* pos2,
    uint32_t base,
    uint32_t precision
);

/**
 * @brief Compute geometric distance (Pure Abacus)
 * 
 * Computes the geometric distance on the clock lattice.
 * 
 * @param pos1 First position
 * @param pos2 Second position
 * @param base Abacus base
 * @param precision Precision for calculations
 * @return Geometric distance or NULL on error
 */
CrystallineAbacus* compute_geometric_distance_abacus(
    const ClockPositionAbacus* pos1,
    const ClockPositionAbacus* pos2,
    uint32_t base,
    uint32_t precision
);

/**
 * @brief Compute ring distance (Pure Abacus)
 * 
 * Computes the distance between rings.
 * 
 * @param ring1 First ring
 * @param ring2 Second ring
 * @param base Abacus base
 * @return Ring distance or NULL on error
 */
CrystallineAbacus* compute_ring_distance_abacus(
    uint32_t ring1,
    uint32_t ring2,
    uint32_t base
);

// ============================================================================
// PRIME FACTORIZATION (O(1) via Clock Lattice)
// ============================================================================

/**
 * @brief O(1) prime factorization via clock lattice (Pure Abacus)
 * 
 * Revolutionary O(1) factorization using clock lattice structure.
 * Position on clock determines the prime factors.
 * 
 * @param n Number to factorize
 * @param base Abacus base
 * @param precision Precision for calculations
 * @return Prime factorization or NULL on error
 */
PrimeFactorizationAbacus* clock_lattice_factorize_abacus(
    const CrystallineAbacus* n,
    uint32_t base,
    uint32_t precision
);

/**
 * @brief Free prime factorization
 * 
 * @param factorization Factorization to free
 */
void free_prime_factorization_abacus(PrimeFactorizationAbacus* factorization);

/**
 * @brief Validate factorization (Pure Abacus)
 * 
 * Validates that the factorization is correct using geometric structure.
 * 
 * @param n Original number
 * @param factorization Factorization to validate
 * @param base Abacus base
 * @return true if valid, false otherwise
 */
bool validate_factorization_abacus(
    const CrystallineAbacus* n,
    const PrimeFactorizationAbacus* factorization,
    uint32_t base
);

// ============================================================================
// FACTOR VISUALIZATION
// ============================================================================

/**
 * @brief Visualize factors on clock lattice (Pure Abacus)
 * 
 * Creates a complete visualization of prime factors on the clock lattice
 * with all geometric relationships.
 * 
 * @param n Number to visualize
 * @param p First prime factor
 * @param q Second prime factor
 * @param base Abacus base
 * @param precision Precision for calculations
 * @return Visualization or NULL on error
 */
ClockFactorVisualizationAbacus* visualize_factors_abacus(
    const CrystallineAbacus* n,
    const CrystallineAbacus* p,
    const CrystallineAbacus* q,
    uint32_t base,
    uint32_t precision
);

/**
 * @brief Free factor visualization
 * 
 * @param viz Visualization to free
 */
void free_factor_visualization_abacus(ClockFactorVisualizationAbacus* viz);

// ============================================================================
// TETRATION ATTRACTORS
// ============================================================================

/**
 * @brief Find tetration attractor via clock lattice (Pure Abacus)
 * 
 * Uses geometric convergence on clock spiral for O(1) attractor detection.
 * 
 * @param base_value Base for tetration
 * @param target Target value
 * @param clock_base Abacus base
 * @param precision Precision for calculations
 * @return Tetration attractor or NULL on error
 */
TetrationAttractorAbacus* clock_lattice_tetration_abacus(
    const CrystallineAbacus* base_value,
    const CrystallineAbacus* target,
    uint32_t clock_base,
    uint32_t precision
);

/**
 * @brief Free tetration attractor
 * 
 * @param attractor Attractor to free
 */
void free_tetration_attractor_abacus(TetrationAttractorAbacus* attractor);

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * @brief Get total clock positions
 * 
 * Returns the total number of positions on the clock lattice lattice.
 * 
 * @return Total positions (4,320,000 = 12 × 60 × 60 × 100)
 */
uint32_t get_total_clock_positions_abacus(void);

/**
 * @brief Get ring size
 * 
 * Returns the size of a specific ring.
 * 
 * @param ring Ring number (0-3)
 * @return Ring size or 0 if invalid
 */
uint32_t get_ring_size_abacus(uint32_t ring);

/**
 * @brief Compute clock resolution (Pure Abacus)
 * 
 * Computes the angular resolution of the clock lattice.
 * 
 * @param base Abacus base
 * @param precision Precision for calculations
 * @return Angular resolution or NULL on error
 */
CrystallineAbacus* compute_clock_resolution_abacus(
    uint32_t base,
    uint32_t precision
);

#ifdef __cplusplus
}
#endif

#endif // CLOCK_LATTICE_ABACUS_H