/**
 * @file clock_lattice_integration.h
 * @brief Clock Lattice Integration for Factor Visualization
 * 
 * UNIVERSAL GEOMETRIC MATHEMATICS - Works with ANY system!
 * 
 * Maps prime factors to positions on the 13D clock lattice
 * and visualizes their geometric relationships using the
 * Babylonian structure (12, 60, 60, 100).
 * 
 * NO crypto-specific dependencies - works with raw uint64_t data.
 */

#ifndef CLOCK_LATTICE_INTEGRATION_H
#define CLOCK_LATTICE_INTEGRATION_H

#include <stdint.h>
#include <stdbool.h>

/**
 * Clock factor visualization result
 */
typedef struct {
    // Input
    uint64_t n;                    // The number (n = p × q)
    uint64_t p;                    // First prime factor
    uint64_t q;                    // Second prime factor
    int p_index;                   // Prime index of p
    int q_index;                   // Prime index of q
    
    // p position on clock
    int p_ring;                    // Ring number (0-3)
    int p_position;                // Position on ring
    double p_angle;                // Angle in radians
    double p_clock_value;          // Normalized clock value [0, 1)
    
    // q position on clock
    int q_ring;                    // Ring number (0-3)
    int q_position;                // Position on ring
    double q_angle;                // Angle in radians
    double q_clock_value;          // Normalized clock value [0, 1)
    
    // Geometric relationships
    double geometric_distance;     // Distance on clock lattice
    double angular_separation;     // Angular separation
    double ring_distance;          // Distance between rings
    
    // Babylonian structure
    int total_positions;           // Total positions (4,320,000)
    double resolution;             // Angular resolution
    
    // Validation
    bool mapping_valid;            // Is mapping valid?
    double confidence;             // Confidence in mapping
} ClockFactorVisualization;

/**
 * Create clock factor visualization
 * 
 * Maps prime factors p and q to positions on the 13D clock lattice
 * and computes their geometric relationships.
 * 
 * @param n The number (n = p × q)
 * @param p First prime factor
 * @param q Second prime factor
 * @param p_index Prime index of p (position in prime sequence)
 * @param q_index Prime index of q (position in prime sequence)
 * @return Visualization result or NULL on error
 */
ClockFactorVisualization* visualize_factors_on_clock(
    uint64_t n,
    uint64_t p,
    uint64_t q,
    int p_index,
    int q_index
);

/**
 * Map prime to clock position
 * 
 * @param prime Prime number
 * @param prime_index Index in prime sequence
 * @param ring_out Pointer to store ring number
 * @param position_out Pointer to store position on ring
 * @param angle_out Pointer to store angle in radians
 * @return true on success, false on error
 */
bool map_prime_to_clock(
    uint64_t prime,
    int prime_index,
    int* ring_out,
    int* position_out,
    double* angle_out
);

/**
 * Compute geometric distance between two clock positions
 * 
 * @param ring1 First ring number
 * @param position1 First position
 * @param ring2 Second ring number
 * @param position2 Second position
 * @return Geometric distance
 */
double compute_geometric_distance(
    int ring1,
    int position1,
    int ring2,
    int position2
);

/**
 * Compute angular separation between two angles
 * 
 * @param angle1 First angle in radians
 * @param angle2 Second angle in radians
 * @return Angular separation in radians [0, π]
 */
double compute_angular_separation(
    double angle1,
    double angle2
);

/**
 * Get ring size (number of positions on ring)
 * 
 * @param ring Ring number (0-3)
 * @return Number of positions on ring
 */
int get_ring_size(int ring);

/**
 * Get total clock positions
 * 
 * @return Total positions (12 × 60 × 60 × 100 = 4,320,000)
 */
int get_total_clock_positions(void);

/**
 * Print clock factor visualization
 * 
 * @param viz Visualization to print
 */
void print_clock_factor_visualization(const ClockFactorVisualization* viz);

/**
 * Export clock factor visualization to file
 * 
 * @param viz Visualization to export
 * @param filename Output filename
 */
void export_clock_factor_visualization(
    const ClockFactorVisualization* viz,
    const char* filename
);

/**
 * Free clock factor visualization
 * 
 * @param viz Visualization to free
 */
void free_clock_factor_visualization(ClockFactorVisualization* viz);

#endif // CLOCK_LATTICE_INTEGRATION_H