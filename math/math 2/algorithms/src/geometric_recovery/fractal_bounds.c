/**
 * @file fractal_bounds.c
 * @brief Fractal Partition Bounds for Geometric Recovery
 * 
 * UNIVERSAL GEOMETRIC MATHEMATICS - Works with ANY system!
 * 
 * Extracted from geometric_recovery.c and made universal.
 */

#include "geometric_recovery/fractal_bounds.h"
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * Compute fractal partition bounds
 */
FractalPartition compute_fractal_partition(
    const double** positions,
    uint32_t num_positions,
    const double* target,
    uint32_t num_dimensions,
    double threshold
) {
    FractalPartition partition;
    memset(&partition, 0, sizeof(FractalPartition));
    
    // Initialize bounds to extremes
    for (uint32_t d = 0; d < num_dimensions && d < FRACTAL_MAX_DIMENSIONS; d++) {
        partition.min_bounds[d] = 1e10;
        partition.max_bounds[d] = -1e10;
    }
    
    // Find positions in same partition as target
    uint32_t num_in_partition = 0;
    
    for (uint32_t i = 0; i < num_positions; i++) {
        // Compute distance to target
        double dist_sq = 0.0;
        for (uint32_t d = 0; d < num_dimensions && d < FRACTAL_MAX_DIMENSIONS; d++) {
            double diff = positions[i][d] - target[d];
            dist_sq += diff * diff;
        }
        
        double dist = math_sqrt(dist_sq);
        
        if (dist < threshold) {
            num_in_partition++;
            
            // Update bounds
            for (uint32_t d = 0; d < num_dimensions && d < FRACTAL_MAX_DIMENSIONS; d++) {
                if (positions[i][d] < partition.min_bounds[d]) {
                    partition.min_bounds[d] = positions[i][d];
                }
                if (positions[i][d] > partition.max_bounds[d]) {
                    partition.max_bounds[d] = positions[i][d];
                }
            }
        }
    }
    
    partition.num_points_in_partition = num_in_partition;
    
    // Compute partition size and volume
    double volume = 1.0;
    partition.min_dimensions = 0;
    
    for (uint32_t d = 0; d < num_dimensions && d < FRACTAL_MAX_DIMENSIONS; d++) {
        double range = partition.max_bounds[d] - partition.min_bounds[d];
        if (range > 0.01) {
            partition.min_dimensions++;
            volume *= range;
        }
    }
    
    partition.max_dimensions = num_dimensions;
    partition.volume = volume;
    partition.partition_size = (uint64_t)(volume * (1ULL << 52));
    
    // Compute confidence based on number of points in partition
    if (num_positions > 0) {
        partition.confidence = (double)num_in_partition / (double)num_positions;
    } else {
        partition.confidence = 0.0;
    }
    
    return partition;
}

/**
 * Check if position is within fractal bounds
 */
bool is_within_fractal_bounds(
    const FractalPartition* partition,
    const double* position,
    uint32_t num_dimensions
) {
    if (!partition || !position) {
        return false;
    }
    
    for (uint32_t d = 0; d < num_dimensions && d < FRACTAL_MAX_DIMENSIONS; d++) {
        if (position[d] < partition->min_bounds[d] || 
            position[d] > partition->max_bounds[d]) {
            return false;
        }
    }
    
    return true;
}

/**
 * Apply fractal bounds to position
 */
void apply_fractal_bounds(
    const FractalPartition* partition,
    double* position,
    uint32_t num_dimensions
) {
    if (!partition || !position) {
        return;
    }
    
    // Clamp each dimension to bounds
    for (uint32_t d = 0; d < num_dimensions && d < FRACTAL_MAX_DIMENSIONS; d++) {
        if (position[d] < partition->min_bounds[d]) {
            position[d] = partition->min_bounds[d];
        }
        if (position[d] > partition->max_bounds[d]) {
            position[d] = partition->max_bounds[d];
        }
    }
}

/**
 * Compute reduction factor
 */
double fractal_bounds_reduction_factor(
    const FractalPartition* partition,
    uint64_t original_space
) {
    if (partition->partition_size == 0) {
        return 1.0;
    }
    
    return (double)original_space / (double)partition->partition_size;
}

/**
 * Print fractal partition
 */
void print_fractal_partition(const FractalPartition* partition) {
    if (!partition) return;
    
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║  Fractal Partition Bounds                                  ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    printf("  Dimensions:\n");
    printf("    Active: %u / %u\n", partition->min_dimensions, partition->max_dimensions);
    printf("\n");
    
    printf("  Bounds (first 5 dimensions):\n");
    for (uint32_t d = 0; d < 5 && d < partition->max_dimensions; d++) {
        printf("    Dim %u: [%.6f, %.6f]\n", 
               d, partition->min_bounds[d], partition->max_bounds[d]);
    }
    printf("\n");
    
    printf("  Partition:\n");
    printf("    Size: %lu\n", partition->partition_size);
    printf("    Volume: %.6e\n", partition->volume);
    printf("    Points: %u\n", partition->num_points_in_partition);
    printf("    Confidence: %.2f%%\n", partition->confidence * 100.0);
    printf("\n");
}

/**
 * Export fractal partition
 */
void export_fractal_partition(
    const FractalPartition* partition,
    const char* filename
) {
    if (!partition || !filename) return;
    
    FILE* f = fopen(filename, "w");
    if (!f) {
        fprintf(stderr, "Failed to open %s for writing\n", filename);
        return;
    }
    
    fprintf(f, "# Fractal Partition Bounds\n");
    fprintf(f, "min_dimensions,%u\n", partition->min_dimensions);
    fprintf(f, "max_dimensions,%u\n", partition->max_dimensions);
    fprintf(f, "partition_size,%lu\n", partition->partition_size);
    fprintf(f, "volume,%.15e\n", partition->volume);
    fprintf(f, "num_points,%u\n", partition->num_points_in_partition);
    fprintf(f, "confidence,%.10f\n", partition->confidence);
    fprintf(f, "\n");
    
    fprintf(f, "# Bounds per dimension\n");
    fprintf(f, "dimension,min_bound,max_bound,range\n");
    for (uint32_t d = 0; d < partition->max_dimensions && d < FRACTAL_MAX_DIMENSIONS; d++) {
        double range = partition->max_bounds[d] - partition->min_bounds[d];
        fprintf(f, "%u,%.15e,%.15e,%.15e\n", 
                d, partition->min_bounds[d], partition->max_bounds[d], range);
    }
    
    fclose(f);
    printf("  ✓ Exported fractal partition to %s\n", filename);
}