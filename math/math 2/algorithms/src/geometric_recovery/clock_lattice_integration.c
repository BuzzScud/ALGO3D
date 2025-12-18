/**
 * @file clock_lattice_integration.c
 * @brief Clock Lattice Integration for Factor Visualization
 * 
 * UNIVERSAL GEOMETRIC MATHEMATICS - Works with ANY system!
 * 
 * Maps prime factors to positions on the 13D clock lattice
 * and visualizes their geometric relationships.
 */

#include "geometric_recovery/clock_lattice_integration.h"
#include "math/constants.h"
#include "math/types.h"
#include "math/constants.h"
#include "math/transcendental.h"
#include "math/constants.h"
#include "math/arithmetic.h"
#include "math/constants.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// clock lattice structure
#define RING_0_SIZE 12    // Hours
#define RING_1_SIZE 60    // Minutes
#define RING_2_SIZE 60    // Seconds
#define RING_3_SIZE 100   // Milliseconds

/**
 * Get ring size
 */
int get_ring_size(int ring) {
    switch (ring) {
        case 0: return RING_0_SIZE;
        case 1: return RING_1_SIZE;
        case 2: return RING_2_SIZE;
        case 3: return RING_3_SIZE;
        default: return 0;
    }
}

/**
 * Get total clock positions
 */
int get_total_clock_positions(void) {
    return RING_0_SIZE * RING_1_SIZE * RING_2_SIZE * RING_3_SIZE;
}

/**
 * Map prime to clock position
 */
bool map_prime_to_clock(
    uint64_t prime,
    int prime_index,
    int* ring_out,
    int* position_out,
    double* angle_out
) {
    (void)prime; // Reserved for future use in prime-based positioning
    if (!ring_out || !position_out || !angle_out) {
        return false;
    }
    
    // Use prime index to determine ring and position
    // This creates a deterministic mapping from primes to clock positions
    
    int total_positions = get_total_clock_positions();
    int clock_position = prime_index % total_positions;
    
    // Decompose into ring positions
    int pos = clock_position;
    
    // Ring 3 (milliseconds) - decompose position into ring coordinates
    pos /= RING_3_SIZE;
    
    // Ring 2 (seconds)
    pos /= RING_2_SIZE;
    
    // Ring 1 (minutes)
    pos /= RING_1_SIZE;
    
    // Ring 0 (hours)
    int r0 = pos % RING_0_SIZE;
    
    // For simplicity, use the outermost ring (ring 0) as primary
    *ring_out = 0;
    *position_out = r0;
    
    // Compute angle
    *angle_out = (2.0 * MATH_PI * r0) / RING_0_SIZE;
    
    return true;
}

/**
 * Compute angular separation
 */
double compute_angular_separation(
    double angle1,
    double angle2
) {
    double diff = math_abs(angle1 - angle2);
    
    // Normalize to [0, π]
    if (diff > MATH_PI) {
        diff = 2.0 * MATH_PI - diff;
    }
    
    return diff;
}

/**
 * Compute geometric distance
 */
double compute_geometric_distance(
    int ring1,
    int position1,
    int ring2,
    int position2
) {
    // If on same ring, use angular distance
    if (ring1 == ring2) {
        int ring_size = get_ring_size(ring1);
        int pos_diff = abs(position1 - position2);
        
        // Wrap around
        if (pos_diff > ring_size / 2) {
            pos_diff = ring_size - pos_diff;
        }
        
        return (double)pos_diff / ring_size;
    }
    
    // If on different rings, use Euclidean distance
    // Treat rings as concentric circles with radii proportional to ring number
    double r1 = (double)(ring1 + 1);
    double r2 = (double)(ring2 + 1);
    
    int size1 = get_ring_size(ring1);
    int size2 = get_ring_size(ring2);
    
    double angle1 = (2.0 * MATH_PI * position1) / size1;
    double angle2 = (2.0 * MATH_PI * position2) / size2;
    
    double x1 = r1 * math_cos(angle1);
    double y1 = r1 * math_sin(angle1);
    double x2 = r2 * math_cos(angle2);
    double y2 = r2 * math_sin(angle2);
    
    return math_sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

/**
 * Create clock factor visualization
 */
ClockFactorVisualization* visualize_factors_on_clock(
    uint64_t n,
    uint64_t p,
    uint64_t q,
    int p_index,
    int q_index
) {
    if (n == 0 || p == 0 || q == 0) {
        return NULL;
    }
    
    ClockFactorVisualization* viz = (ClockFactorVisualization*)calloc(
        1, sizeof(ClockFactorVisualization)
    );
    if (!viz) {
        return NULL;
    }
    
    // Store input
    viz->n = n;
    viz->p = p;
    viz->q = q;
    viz->p_index = p_index;
    viz->q_index = q_index;
    
    // Map p to clock
    if (!map_prime_to_clock(p, p_index, &viz->p_ring, &viz->p_position, &viz->p_angle)) {
        free(viz);
        return NULL;
    }
    
    viz->p_clock_value = (double)viz->p_position / get_ring_size(viz->p_ring);
    
    // Map q to clock
    if (!map_prime_to_clock(q, q_index, &viz->q_ring, &viz->q_position, &viz->q_angle)) {
        free(viz);
        return NULL;
    }
    
    viz->q_clock_value = (double)viz->q_position / get_ring_size(viz->q_ring);
    
    // Compute geometric relationships
    viz->geometric_distance = compute_geometric_distance(
        viz->p_ring, viz->p_position,
        viz->q_ring, viz->q_position
    );
    
    viz->angular_separation = compute_angular_separation(
        viz->p_angle, viz->q_angle
    );
    
    viz->ring_distance = abs(viz->p_ring - viz->q_ring);
    
    // base-60 structure
    viz->total_positions = get_total_clock_positions();
    viz->resolution = 2.0 * MATH_PI / viz->total_positions;
    
    // Validation
    viz->mapping_valid = (p * q == n);
    viz->confidence = viz->mapping_valid ? 1.0 : 0.0;
    
    return viz;
}

/**
 * Print clock factor visualization
 */
void print_clock_factor_visualization(const ClockFactorVisualization* viz) {
    if (!viz) return;
    
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║  Clock Lattice Factor Visualization                       ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    printf("  Input:\n");
    printf("    n = %lu\n", viz->n);
    printf("    p = %lu (index %d)\n", viz->p, viz->p_index);
    printf("    q = %lu (index %d)\n", viz->q, viz->q_index);
    printf("    Verification: p × q = %lu %s\n\n", 
           viz->p * viz->q,
           viz->mapping_valid ? "✓" : "✗");
    
    printf("  p Clock Position:\n");
    printf("    Ring: %d (%s)\n", viz->p_ring, 
           viz->p_ring == 0 ? "Hours" :
           viz->p_ring == 1 ? "Minutes" :
           viz->p_ring == 2 ? "Seconds" : "Milliseconds");
    printf("    Position: %d / %d\n", viz->p_position, get_ring_size(viz->p_ring));
    printf("    Angle: %.4f rad (%.2f°)\n", viz->p_angle, viz->p_angle * 180.0 / MATH_PI);
    printf("    Clock Value: %.6f\n\n", viz->p_clock_value);
    
    printf("  q Clock Position:\n");
    printf("    Ring: %d (%s)\n", viz->q_ring,
           viz->q_ring == 0 ? "Hours" :
           viz->q_ring == 1 ? "Minutes" :
           viz->q_ring == 2 ? "Seconds" : "Milliseconds");
    printf("    Position: %d / %d\n", viz->q_position, get_ring_size(viz->q_ring));
    printf("    Angle: %.4f rad (%.2f°)\n", viz->q_angle, viz->q_angle * 180.0 / MATH_PI);
    printf("    Clock Value: %.6f\n\n", viz->q_clock_value);
    
    printf("  Geometric Relationships:\n");
    printf("    Geometric Distance: %.6f\n", viz->geometric_distance);
    printf("    Angular Separation: %.4f rad (%.2f°)\n", 
           viz->angular_separation, viz->angular_separation * 180.0 / MATH_PI);
    printf("    Ring Distance: %.0f\n\n", viz->ring_distance);
    
    printf("  base-60 Structure:\n");
    printf("    Total Positions: %d\n", viz->total_positions);
    printf("    Resolution: %.10f rad\n", viz->resolution);
    printf("    Confidence: %.2f%%\n", viz->confidence * 100.0);
    
    printf("\n");
}

/**
 * Export clock factor visualization
 */
void export_clock_factor_visualization(
    const ClockFactorVisualization* viz,
    const char* filename
) {
    if (!viz || !filename) return;
    
    FILE* f = fopen(filename, "w");
    if (!f) {
        fprintf(stderr, "Failed to open %s for writing\n", filename);
        return;
    }
    
    fprintf(f, "# Clock Lattice Factor Visualization\n");
    fprintf(f, "n,%lu\n", viz->n);
    fprintf(f, "p,%lu\n", viz->p);
    fprintf(f, "q,%lu\n", viz->q);
    fprintf(f, "p_index,%d\n", viz->p_index);
    fprintf(f, "q_index,%d\n", viz->q_index);
    fprintf(f, "\n");
    
    fprintf(f, "# p Clock Position\n");
    fprintf(f, "p_ring,%d\n", viz->p_ring);
    fprintf(f, "p_position,%d\n", viz->p_position);
    fprintf(f, "p_angle,%.10f\n", viz->p_angle);
    fprintf(f, "p_clock_value,%.10f\n", viz->p_clock_value);
    fprintf(f, "\n");
    
    fprintf(f, "# q Clock Position\n");
    fprintf(f, "q_ring,%d\n", viz->q_ring);
    fprintf(f, "q_position,%d\n", viz->q_position);
    fprintf(f, "q_angle,%.10f\n", viz->q_angle);
    fprintf(f, "q_clock_value,%.10f\n", viz->q_clock_value);
    fprintf(f, "\n");
    
    fprintf(f, "# Geometric Relationships\n");
    fprintf(f, "geometric_distance,%.10f\n", viz->geometric_distance);
    fprintf(f, "angular_separation,%.10f\n", viz->angular_separation);
    fprintf(f, "ring_distance,%.0f\n", viz->ring_distance);
    fprintf(f, "\n");
    
    fprintf(f, "# base-60 Structure\n");
    fprintf(f, "total_positions,%d\n", viz->total_positions);
    fprintf(f, "resolution,%.15f\n", viz->resolution);
    fprintf(f, "confidence,%.10f\n", viz->confidence);
    
    fclose(f);
    
    printf("  ✓ Exported visualization to %s\n", filename);
}

/**
 * Free clock factor visualization
 */
void free_clock_factor_visualization(ClockFactorVisualization* viz) {
    if (viz) {
        free(viz);
    }
}