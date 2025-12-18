/**
 * clock_lattice_integration.c - Clock Lattice Integration for Geometric Recovery
 * 
 * This module implements Phase 3 of the geometric recovery algorithm:
 * integrating the Babylonian clock lattice for factor visualization and G refinement.
 * 
 * The clock lattice provides a geometric structure for understanding prime factorization:
 * - Ring 0: Positions 1-12 (hours) - Outer ring
 * - Ring 1: Positions 1-60 (minutes)
 * - Ring 2: Positions 1-60 (seconds)
 * - Ring 3: Positions 1-100 (milliseconds) - Inner ring
 * - Ring 4+: Extended rings (1000 positions each)
 */

#include "../include/clock_lattice_integration.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "math/arithmetic.h"
#include "math/transcendental.h"

// Ring structure for Babylonian clock
#define RING_0_SIZE 12
#define RING_1_SIZE 60
#define RING_2_SIZE 60
#define RING_3_SIZE 100
#define EXTENDED_RING_SIZE 1000

/**
 * Get the ring for a given prime index
 */
static int get_prime_ring(int prime_index) {
    if (prime_index <= RING_0_SIZE) return 0;
    if (prime_index <= RING_0_SIZE + RING_1_SIZE) return 1;
    if (prime_index <= RING_0_SIZE + RING_1_SIZE + RING_2_SIZE) return 2;
    if (prime_index <= RING_0_SIZE + RING_1_SIZE + RING_2_SIZE + RING_3_SIZE) return 3;
    
    // Extended rings (4+)
    int remaining = prime_index - (RING_0_SIZE + RING_1_SIZE + RING_2_SIZE + RING_3_SIZE);
    return 4 + (remaining / EXTENDED_RING_SIZE);
}

/**
 * Get position on ring for a given prime index
 */
static int get_position_on_ring(int prime_index, int ring) {
    int base = 0;
    int ring_size = 0;
    
    switch (ring) {
        case 0:
            base = 0;
            ring_size = RING_0_SIZE;
            break;
        case 1:
            base = RING_0_SIZE;
            ring_size = RING_1_SIZE;
            break;
        case 2:
            base = RING_0_SIZE + RING_1_SIZE;
            ring_size = RING_2_SIZE;
            break;
        case 3:
            base = RING_0_SIZE + RING_1_SIZE + RING_2_SIZE;
            ring_size = RING_3_SIZE;
            break;
        default: // Extended rings
            base = RING_0_SIZE + RING_1_SIZE + RING_2_SIZE + RING_3_SIZE;
            base += (ring - 4) * EXTENDED_RING_SIZE;
            ring_size = EXTENDED_RING_SIZE;
            break;
    }
    
    int position = prime_index - base;
    return (position > 0 && position <= ring_size) ? position : 1;
}

/**
 * Get clock angle for a position on a ring
 */
static double get_clock_angle(int position, int ring) {
    int ring_size = 0;
    
    switch (ring) {
        case 0: ring_size = RING_0_SIZE; break;
        case 1: ring_size = RING_1_SIZE; break;
        case 2: ring_size = RING_2_SIZE; break;
        case 3: ring_size = RING_3_SIZE; break;
        default: ring_size = EXTENDED_RING_SIZE; break;
    }
    
    // Convert position to angle (0 = 12 o'clock, clockwise)
    // Subtract 90 degrees to start at 3 o'clock (standard math convention)
    double angle = (double)position / ring_size * 2.0 * 3.14159265359;
    angle -= 3.14159265359 / 2.0; // Rotate to start at 3 o'clock
    
    return angle;
}

/**
 * Get radial distance for a ring
 */
static double get_radial_distance(int ring) {
    // Base radius for ring 0
    double base_radius = 1.0;
    
    // Each ring is slightly further out
    switch (ring) {
        case 0: return base_radius * 1.0;
        case 1: return base_radius * 1.2;
        case 2: return base_radius * 1.4;
        case 3: return base_radius * 1.6;
        default: return base_radius * (1.6 + (ring - 3) * 0.15);
    }
}

/**
 * Map prime to clock position
 */
ClockPosition map_prime_to_clock(uint64_t prime, int prime_index) {
    ClockPosition pos;
    pos.prime = prime;
    pos.prime_index = prime_index;
    
    // Get ring and position
    pos.ring = get_prime_ring(prime_index);
    pos.position = get_position_on_ring(prime_index, pos.ring);
    
    // Get angle and radius
    pos.angle = get_clock_angle(pos.position, pos.ring);
    pos.radius = get_radial_distance(pos.ring);
    
    // Compute Cartesian coordinates
    pos.x = pos.radius * math_cos(pos.angle);
    pos.y = pos.radius * math_sin(pos.angle);
    
    return pos;
}

/**
 * Visualize factors on clock lattice
 */
ClockFactorVisualization* visualize_factors_on_clock(
    uint64_t n,
    uint64_t p,
    uint64_t q,
    int p_index,
    int q_index
) {
    ClockFactorVisualization* viz = (ClockFactorVisualization*)calloc(1, sizeof(ClockFactorVisualization));
    if (!viz) return NULL;
    
    viz->n = n;
    viz->p = p;
    viz->q = q;
    
    // Map p and q to clock positions
    viz->p_position = map_prime_to_clock(p, p_index);
    viz->q_position = map_prime_to_clock(q, q_index);
    
    // Compute angular separation
    double angle_diff = viz->q_position.angle - viz->p_position.angle;
    
    // Normalize to [0, 2π]
    while (angle_diff < 0) angle_diff += 2.0 * 3.14159265359;
    while (angle_diff > 2.0 * 3.14159265359) angle_diff -= 2.0 * 3.14159265359;
    
    viz->angular_separation = angle_diff;
    viz->angular_separation_degrees = angle_diff * 180.0 / 3.14159265359;
    
    // Compute Euclidean distance
    double dx = viz->q_position.x - viz->p_position.x;
    double dy = viz->q_position.y - viz->p_position.y;
    viz->euclidean_distance = math_sqrt(dx * dx + dy * dy);
    
    // Check if q is at sacred position (π, 3 o'clock)
    // Sacred position is at angle 0 (3 o'clock in standard math convention)
    double q_angle_normalized = viz->q_position.angle;
    while (q_angle_normalized < 0) q_angle_normalized += 2.0 * 3.14159265359;
    while (q_angle_normalized > 2.0 * 3.14159265359) q_angle_normalized -= 2.0 * 3.14159265359;
    
    // Check if close to 0 (3 o'clock) or π (9 o'clock)
    double tolerance = 0.1; // ~5.7 degrees
    double diff_from_pi = q_angle_normalized - 3.14159265359;
    if (diff_from_pi < 0) diff_from_pi = -diff_from_pi;
    
    viz->q_at_sacred_position = 
        (q_angle_normalized < tolerance) || 
        (q_angle_normalized > 2.0 * 3.14159265359 - tolerance) ||
        (diff_from_pi < tolerance);
    
    return viz;
}

/**
 * Free clock factor visualization
 */
void free_clock_factor_visualization(ClockFactorVisualization* viz) {
    if (viz) {
        free(viz);
    }
}

/**
 * Print clock factor visualization
 */
void print_clock_factor_visualization(const ClockFactorVisualization* viz) {
    if (!viz) return;
    
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║  Clock Lattice Factor Visualization                       ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    printf("  n = %lu = %lu × %lu\n\n", viz->n, viz->p, viz->q);
    
    printf("  p = %lu (Prime #%d)\n", viz->p, viz->p_position.prime_index);
    printf("    Ring:     %d\n", viz->p_position.ring);
    printf("    Position: %d\n", viz->p_position.position);
    printf("    Angle:    %.2f° (%.4f rad)\n", 
           viz->p_position.angle * 180.0 / 3.14159265359, viz->p_position.angle);
    printf("    Radius:   %.2f\n", viz->p_position.radius);
    printf("    (x, y):   (%.2f, %.2f)\n\n", viz->p_position.x, viz->p_position.y);
    
    printf("  q = %lu (Prime #%d)\n", viz->q, viz->q_position.prime_index);
    printf("    Ring:     %d\n", viz->q_position.ring);
    printf("    Position: %d\n", viz->q_position.position);
    printf("    Angle:    %.2f° (%.4f rad)\n", 
           viz->q_position.angle * 180.0 / 3.14159265359, viz->q_position.angle);
    printf("    Radius:   %.2f\n", viz->q_position.radius);
    printf("    (x, y):   (%.2f, %.2f)\n", viz->q_position.x, viz->q_position.y);
    printf("    Sacred:   %s\n\n", viz->q_at_sacred_position ? "✓ YES" : "✗ NO");
    
    printf("  Geometric Relationships:\n");
    printf("    Angular Separation: %.2f° (%.4f rad)\n", 
           viz->angular_separation_degrees, viz->angular_separation);
    printf("    Euclidean Distance: %.4f\n", viz->euclidean_distance);
    printf("\n");
    
    if (viz->q_at_sacred_position) {
        printf("  ✓ q is at SACRED POSITION (π, 3 o'clock)\n");
        printf("    This indicates special geometric significance!\n\n");
    }
}

/**
 * Export clock factor visualization to file
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
    fprintf(f, "\n");
    
    fprintf(f, "# p position\n");
    fprintf(f, "p_prime_index,%d\n", viz->p_position.prime_index);
    fprintf(f, "p_ring,%d\n", viz->p_position.ring);
    fprintf(f, "p_position,%d\n", viz->p_position.position);
    fprintf(f, "p_angle,%.6f\n", viz->p_position.angle);
    fprintf(f, "p_radius,%.6f\n", viz->p_position.radius);
    fprintf(f, "p_x,%.6f\n", viz->p_position.x);
    fprintf(f, "p_y,%.6f\n", viz->p_position.y);
    fprintf(f, "\n");
    
    fprintf(f, "# q position\n");
    fprintf(f, "q_prime_index,%d\n", viz->q_position.prime_index);
    fprintf(f, "q_ring,%d\n", viz->q_position.ring);
    fprintf(f, "q_position,%d\n", viz->q_position.position);
    fprintf(f, "q_angle,%.6f\n", viz->q_position.angle);
    fprintf(f, "q_radius,%.6f\n", viz->q_position.radius);
    fprintf(f, "q_x,%.6f\n", viz->q_position.x);
    fprintf(f, "q_y,%.6f\n", viz->q_position.y);
    fprintf(f, "q_sacred,%d\n", viz->q_at_sacred_position);
    fprintf(f, "\n");
    
    fprintf(f, "# Geometric relationships\n");
    fprintf(f, "angular_separation_rad,%.6f\n", viz->angular_separation);
    fprintf(f, "angular_separation_deg,%.6f\n", viz->angular_separation_degrees);
    fprintf(f, "euclidean_distance,%.6f\n", viz->euclidean_distance);
    
    fclose(f);
    
    printf("  ✓ Exported visualization to %s\n", filename);
}
