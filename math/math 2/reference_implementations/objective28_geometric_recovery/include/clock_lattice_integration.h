#ifndef CLOCK_LATTICE_INTEGRATION_H
#define CLOCK_LATTICE_INTEGRATION_H

#include <stdint.h>
#include <stdbool.h>

/**
 * Clock Lattice Integration
 * 
 * Integrates the Babylonian clock lattice structure for factor visualization
 * and geometric recovery enhancement.
 * 
 * The clock lattice provides a geometric structure for understanding prime factorization:
 * - Ring 0: Positions 1-12 (hours) - Outer ring
 * - Ring 1: Positions 1-60 (minutes)
 * - Ring 2: Positions 1-60 (seconds)
 * - Ring 3: Positions 1-100 (milliseconds) - Inner ring
 * - Ring 4+: Extended rings (1000 positions each)
 */

/**
 * Clock position for a prime number
 */
typedef struct {
    uint64_t prime;           // The prime number
    int prime_index;          // Index in prime sequence (1st prime, 2nd prime, etc.)
    
    // Clock lattice position
    int ring;                 // Which ring (0-3 for base, 4+ for extended)
    int position;             // Position on ring
    
    // Geometric coordinates
    double angle;             // Angle in radians (0 = 3 o'clock, counterclockwise)
    double radius;            // Radial distance from center
    double x;                 // Cartesian x coordinate
    double y;                 // Cartesian y coordinate
} ClockPosition;

/**
 * Clock factor visualization
 */
typedef struct {
    uint64_t n;               // The number being factored (n = p × q)
    uint64_t p;               // First prime factor
    uint64_t q;               // Second prime factor
    
    // Clock positions
    ClockPosition p_position; // Position of p on clock
    ClockPosition q_position; // Position of q on clock
    
    // Geometric relationships
    double angular_separation;        // Angular separation in radians
    double angular_separation_degrees; // Angular separation in degrees
    double euclidean_distance;        // Euclidean distance between p and q
    bool q_at_sacred_position;        // Is q at sacred position (π, 3 o'clock)?
} ClockFactorVisualization;

/**
 * Map prime to clock position
 * 
 * @param prime The prime number
 * @param prime_index The index of the prime (1st, 2nd, 3rd, etc.)
 * @return Clock position
 */
ClockPosition map_prime_to_clock(uint64_t prime, int prime_index);

/**
 * Visualize factors on clock lattice
 * 
 * Creates a visualization of p and q on the Babylonian clock lattice,
 * showing their positions, angular separation, and geometric relationships.
 * 
 * @param n The number being factored (n = p × q)
 * @param p First prime factor
 * @param q Second prime factor
 * @param p_index Index of p in prime sequence
 * @param q_index Index of q in prime sequence
 * @return Clock factor visualization (must be freed)
 */
ClockFactorVisualization* visualize_factors_on_clock(
    uint64_t n,
    uint64_t p,
    uint64_t q,
    int p_index,
    int q_index
);

/**
 * Free clock factor visualization
 */
void free_clock_factor_visualization(ClockFactorVisualization* viz);

/**
 * Print clock factor visualization
 */
void print_clock_factor_visualization(const ClockFactorVisualization* viz);

/**
 * Export clock factor visualization to file
 */
void export_clock_factor_visualization(
    const ClockFactorVisualization* viz,
    const char* filename
);

#endif // CLOCK_LATTICE_INTEGRATION_H