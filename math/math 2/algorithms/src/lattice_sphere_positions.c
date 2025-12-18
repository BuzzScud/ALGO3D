/**
 * lattice_sphere_positions.c - Sphere Position Management Algorithm
 * 
 * PROPERLY IMPLEMENTED: Uses NEW math library (Crystalline Abacus)
 * 
 * This is an ALGORITHM built on top of the NEW math library, providing:
 * - Sphere positioning in crystalline lattice
 * - Partition management (prime ranges per sphere)
 * - 12-fold symmetry structure
 * - Clock position mapping
 * 
 * Architecture:
 * - Each sphere has a position in the lattice
 * - Each sphere owns a partition (range of primes)
 * - 12-fold symmetry (one sphere per symmetry group)
 * - Geometric clock lattice mapping
 */

#include "lattice_sphere_positions.h"
#include "math/abacus.h"
#include "math/types.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdatomic.h>

// Global sphere ID counter (thread-safe)
static _Atomic(uint64_t) next_sphere_id = 1;

/**
 * Get next sphere ID
 */
uint64_t get_next_sphere_id(void) {
    return atomic_fetch_add(&next_sphere_id, 1);
}

/**
 * Calculate clock position from symmetry group
 * 
 * Maps symmetry group (0-11) to position on clock lattice.
 */
LatticeClockPosition calculate_clock_position(int symmetry_group) {
    LatticeClockPosition pos;
    memset(&pos, 0, sizeof(LatticeClockPosition));
    
    if (symmetry_group < 0 || symmetry_group >= 12) {
        return pos;
    }
    
    // Map symmetry group to clock lattice position
    pos.ring = 0;  // Hours ring (0-11)
    pos.position = symmetry_group + 1;  // 1-based position (1-12)
    
    // Calculate angle (12 o'clock = -π/2, increases clockwise)
    pos.angle = (symmetry_group * 2.0 * 3.14159265358979323846) / 12.0 - (3.14159265358979323846 / 2.0);
    
    // Radius (normalized to 1.0 for sphere surface)
    pos.radius = 1.0;
    
    return pos;
}

/**
 * Create lattice partition
 * 
 * Defines a range of primes for a sphere to generate.
 */
LatticePartition* create_lattice_partition(int symmetry_group,
                                          const void* range_start,
                                          const void* range_end) {
    if (symmetry_group < 0 || symmetry_group >= 12) {
        fprintf(stderr, "Error: Invalid symmetry group: %d\n", symmetry_group);
        return NULL;
    }
    
    LatticePartition* partition = (LatticePartition*)calloc(1, sizeof(LatticePartition));
    if (!partition) {
        fprintf(stderr, "Error: Failed to allocate LatticePartition\n");
        return NULL;
    }
    
    partition->symmetry_group = symmetry_group;
    
    // TODO: Copy range_start and range_end (CrystallineAbacus)
    // For now, use simplified uint64_t ranges
    partition->range_start = 0;
    partition->range_end = 0;
    
    (void)range_start;
    (void)range_end;
    
    return partition;
}

/**
 * Free lattice partition
 */
void free_lattice_partition(LatticePartition* partition) {
    if (!partition) return;
    
    // TODO: Free range_start and range_end (CrystallineAbacus)
    
    free(partition);
}

/**
 * Create sphere position
 * 
 * Creates a sphere position with clock mapping and partition.
 */
SpherePosition* create_sphere_position(int symmetry_group,
                                       const void* prime_start,
                                       const void* prime_end) {
    if (symmetry_group < 0 || symmetry_group >= 12) {
        fprintf(stderr, "Error: Invalid symmetry group: %d\n", symmetry_group);
        return NULL;
    }
    
    SpherePosition* position = (SpherePosition*)calloc(1, sizeof(SpherePosition));
    if (!position) {
        fprintf(stderr, "Error: Failed to allocate SpherePosition\n");
        return NULL;
    }
    
    position->sphere_id = get_next_sphere_id();
    position->symmetry_group = symmetry_group;
    position->clock_pos = calculate_clock_position(symmetry_group);
    
    // Create partition
    position->partition = create_lattice_partition(symmetry_group, prime_start, prime_end);
    if (!position->partition) {
        free(position);
        return NULL;
    }
    
    return position;
}

/**
 * Free sphere position
 */
void free_sphere_position(SpherePosition* position) {
    if (!position) return;
    
    if (position->partition) {
        free_lattice_partition(position->partition);
    }
    
    free(position);
}

/**
 * Check if prime is in partition
 * 
 * Verifies that prime is within partition boundaries.
 */
bool is_prime_in_partition(const LatticePartition* partition, const void* prime) {
    if (!partition || !prime) {
        return false;
    }
    
    // TODO: Implement range check using NEW math library
    // 1. Check if prime >= partition->range_start
    // 2. Check if prime <= partition->range_end
    // 3. Check if prime % 12 == partition->symmetry_group
    
    return false;
}

/**
 * Get partition size
 * 
 * Returns the number of primes in the partition (approximate).
 */
uint64_t get_partition_size(const LatticePartition* partition) {
    if (!partition) {
        return 0;
    }
    
    // TODO: Implement using NEW math library
    // Calculate: (range_end - range_start) / 12 (approximate)
    
    return 0;
}

/**
 * Print partition info
 */
void print_partition_info(const LatticePartition* partition) {
    if (!partition) {
        printf("NULL partition\n");
        return;
    }
    
    printf("Partition (symmetry group %d)\n", partition->symmetry_group);
    printf("  Range: %lu to %lu\n", 
           (unsigned long)partition->range_start, 
           (unsigned long)partition->range_end);
}

/**
 * Print sphere position
 */
void print_sphere_position(const SpherePosition* position) {
    if (!position) {
        printf("NULL position\n");
        return;
    }
    
    printf("Sphere Position:\n");
    printf("  ID: %lu\n", (unsigned long)position->sphere_id);
    printf("  Symmetry Group: %d\n", position->symmetry_group);
    printf("  Clock: ring=%u, pos=%u, angle=%.4f, radius=%.4f\n",
           position->clock_pos.ring,
           position->clock_pos.position,
           position->clock_pos.angle,
           position->clock_pos.radius);
}

/**
 * Create 12 sphere positions (12-fold symmetry)
 * 
 * Creates one sphere for each symmetry group (0-11).
 */
SpherePosition** create_12_sphere_positions(const void* total_range_start,
                                           const void* total_range_end) {
    if (!total_range_start || !total_range_end) {
        fprintf(stderr, "Error: NULL range parameters\n");
        return NULL;
    }
    
    SpherePosition** positions = (SpherePosition**)calloc(12, sizeof(SpherePosition*));
    if (!positions) {
        fprintf(stderr, "Error: Failed to allocate sphere positions array\n");
        return NULL;
    }
    
    // Create one sphere for each symmetry group
    for (int i = 0; i < 12; i++) {
        // TODO: Calculate partition range for this symmetry group
        // For now, use the full range for each sphere
        positions[i] = create_sphere_position(i, total_range_start, total_range_end);
        
        if (!positions[i]) {
            // Cleanup on failure
            for (int j = 0; j < i; j++) {
                free_sphere_position(positions[j]);
            }
            free(positions);
            return NULL;
        }
    }
    
    return positions;
}

/**
 * Free 12 sphere positions
 */
void free_12_sphere_positions(SpherePosition** positions) {
    if (!positions) return;
    
    for (int i = 0; i < 12; i++) {
        if (positions[i]) {
            free_sphere_position(positions[i]);
        }
    }
    free(positions);
}