/**
 * @file fractal_bounds.h
 * @brief Fractal Partition Bounds for Geometric Recovery
 * 
 * UNIVERSAL GEOMETRIC MATHEMATICS - Works with ANY system!
 * 
 * Implements fractal partition bounds:
 * - Compute bounds on search space
 * - Ensure positions lie within valid partitions
 * - Use self-similar structure for validation
 * 
 * NO crypto-specific dependencies - works with raw uint64_t data.
 * 
 * Thesis Reference: Section 16.3 - Fractal Partition Bounds
 */

#ifndef FRACTAL_BOUNDS_H
#define FRACTAL_BOUNDS_H

#include <stdint.h>
#include <stdbool.h>

#define FRACTAL_MAX_DIMENSIONS 13

/**
 * Fractal partition bounds
 */
typedef struct {
    uint32_t min_dimensions;                        // Minimum dimensions needed
    uint32_t max_dimensions;                        // Maximum dimensions needed
    double min_bounds[FRACTAL_MAX_DIMENSIONS];      // Lower bounds per dimension
    double max_bounds[FRACTAL_MAX_DIMENSIONS];      // Upper bounds per dimension
    uint64_t partition_size;                        // Size of partition
    double confidence;                              // Confidence in bounds
    uint32_t num_points_in_partition;               // Number of points in partition
    double volume;                                  // Geometric volume
} FractalPartition;

/**
 * Compute fractal partition bounds
 * 
 * Finds the fractal partition containing the target position
 * by analyzing nearby known positions.
 * 
 * @param positions Array of known positions in 13D space
 * @param num_positions Number of known positions
 * @param target Target position to find partition for
 * @param num_dimensions Number of dimensions (typically 13)
 * @param threshold Distance threshold for partition membership
 * @return Fractal partition bounds
 */
FractalPartition compute_fractal_partition(
    const double** positions,
    uint32_t num_positions,
    const double* target,
    uint32_t num_dimensions,
    double threshold
);

/**
 * Check if position is within fractal bounds
 * 
 * @param partition Fractal partition
 * @param position Position to check
 * @param num_dimensions Number of dimensions
 * @return true if position is within bounds, false otherwise
 */
bool is_within_fractal_bounds(
    const FractalPartition* partition,
    const double* position,
    uint32_t num_dimensions
);

/**
 * Apply fractal bounds to position
 * 
 * Clamps position to be within the fractal partition bounds.
 * 
 * @param partition Fractal partition
 * @param position Position to clamp (modified in place)
 * @param num_dimensions Number of dimensions
 */
void apply_fractal_bounds(
    const FractalPartition* partition,
    double* position,
    uint32_t num_dimensions
);

/**
 * Compute reduction factor from fractal bounds
 * 
 * @param partition Fractal partition
 * @param original_space Original search space size
 * @return Reduction factor (original_space / partition_size)
 */
double fractal_bounds_reduction_factor(
    const FractalPartition* partition,
    uint64_t original_space
);

/**
 * Print fractal partition information
 * 
 * @param partition Fractal partition to print
 */
void print_fractal_partition(const FractalPartition* partition);

/**
 * Export fractal partition to file
 * 
 * @param partition Fractal partition to export
 * @param filename Output filename
 */
void export_fractal_partition(
    const FractalPartition* partition,
    const char* filename
);

#endif // FRACTAL_BOUNDS_H