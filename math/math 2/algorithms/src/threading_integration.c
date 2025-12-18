/**
 * @file threading_integration.c
 * @brief Math-Threading Integration Implementation
 * 
 * Bridges math library (abacus, rainbow table, clock lattice) with threading system.
 */

#include "threading_integration.h"
#include "hierarchical_memory.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Helper function to compute GCD
static uint64_t gcd(uint64_t a, uint64_t b) {
    while (b != 0) {
        uint64_t temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

// ============================================================================
// THREAD ID TO GEOMETRIC POSITION
// ============================================================================

MathError thread_id_to_position(uint64_t thread_id, uint32_t base, ClockPosition* out_position) {
    if (!out_position || base < 2) {
        return MATH_ERROR_INVALID_ARG;
    }

    // Map thread ID to clock position
    // Use modular arithmetic to distribute threads across rings
    
    // Determine ring based on thread_id
    // Ring 0: 12 positions (0-11)
    // Ring 1: 60 positions (12-71)
    // Ring 2: 60 positions (72-131)
    // Ring 3: 100 positions (132+)
    
    uint32_t ring_sizes[] = {12, 60, 60, 100};
    uint64_t cumulative = 0;
    uint32_t ring = 0;
    
    for (ring = 0; ring < 4; ring++) {
        if (thread_id < cumulative + ring_sizes[ring]) {
            break;
        }
        cumulative += ring_sizes[ring];
    }
    
    // If thread_id exceeds all rings, use extended rings
    if (ring >= 4) {
        ring = 3 + (thread_id - cumulative) / 100;
        cumulative = 132 + ((ring - 3) * 100);
    }
    
    out_position->ring = ring;
    out_position->position = (uint32_t)(thread_id - cumulative);
    
    // Compute angle and radius
    uint32_t ring_size = (ring < 4) ? ring_sizes[ring] : 100;
    out_position->angle = (2.0 * M_PI * out_position->position) / ring_size;
    out_position->radius = 0.25 + (ring * 0.25);
    
    return MATH_SUCCESS;
}

MathError position_to_thread_id(const ClockPosition* position, uint32_t base, uint64_t* out_thread_id) {
    if (!position || !out_thread_id || base < 2) {
        return MATH_ERROR_INVALID_ARG;
    }

    uint32_t ring_sizes[] = {12, 60, 60, 100};
    uint64_t cumulative = 0;
    
    // Add up positions from previous rings
    for (uint32_t r = 0; r < position->ring && r < 4; r++) {
        cumulative += ring_sizes[r];
    }
    
    // Add extended rings if needed
    if (position->ring >= 4) {
        cumulative = 132 + ((position->ring - 3) * 100);
    }
    
    *out_thread_id = cumulative + position->position;
    
    return MATH_SUCCESS;
}

MathError thread_compute_nd_position(uint64_t thread_id, uint32_t num_dimensions, double* out_position) {
    if (!out_position || num_dimensions == 0) {
        return MATH_ERROR_INVALID_ARG;
    }

    ClockPosition pos;
    MathError err = thread_id_to_position(thread_id, 12, &pos);
    if (err != MATH_SUCCESS) {
        return err;
    }

    // First 2 dimensions: Use clock position directly
    if (num_dimensions >= 1) {
        out_position[0] = pos.radius * math_cos(pos.angle);
    }
    if (num_dimensions >= 2) {
        out_position[1] = pos.radius * math_sin(pos.angle);
    }
    if (num_dimensions >= 3) {
        out_position[2] = (double)pos.ring / 4.0;
    }

    // Higher dimensions: Use geometric transformations
    for (uint32_t d = 3; d < num_dimensions; d++) {
        double sum = 0.0;
        uint32_t prime = 2 + d;  // Simple prime sequence
        
        for (uint32_t j = 0; j < 3 && j < num_dimensions; j++) {
            sum += out_position[j] * math_cos(2.0 * M_PI * prime * d / num_dimensions);
        }
        
        out_position[d] = sum / math_sqrt((double)num_dimensions);
    }

    return MATH_SUCCESS;
}

// ============================================================================
// MEMORY SEGMENT TO ABACUS
// ============================================================================

CrystallineAbacus* segment_to_abacus(uint64_t segment_id, uint32_t base) {
    if (base < 2) {
        return NULL;
    }

    return abacus_from_uint64(segment_id, base);
}

MathError abacus_to_segment(const CrystallineAbacus* abacus, uint64_t* out_segment_id) {
    if (!abacus || !out_segment_id) {
        return MATH_ERROR_INVALID_ARG;
    }

    return abacus_to_uint64(abacus, out_segment_id);
}

// ============================================================================
// RAINBOW TABLE FOR THREADING
// ============================================================================

MathError create_thread_rainbow_table(uint64_t max_threads, uint32_t base, RainbowTable* out_table) {
    if (!out_table || max_threads == 0 || base < 2) {
        return MATH_ERROR_INVALID_ARG;
    }

    // Initialize rainbow table
    MathError err = rainbow_init(out_table, max_threads);
    if (err != MATH_SUCCESS) {
        return err;
    }

    // Populate with thread positions
    // Note: Rainbow table is designed for primes, so we adapt it for thread IDs
    // by treating thread IDs as if they were prime indices
    err = rainbow_populate_count(out_table, max_threads);
    if (err != MATH_SUCCESS) {
        return err;
    }

    return MATH_SUCCESS;
}

MathError rainbow_lookup_thread_position(const RainbowTable* table, uint64_t thread_id, ClockPosition* out_position) {
    if (!table || !out_position) {
        return MATH_ERROR_INVALID_ARG;
    }

    // For now, compute position directly
    // In a full implementation, we would cache positions in the rainbow table
    return thread_id_to_position(thread_id, 12, out_position);
}

// ============================================================================
// GEOMETRIC THREAD ORGANIZATION
// ============================================================================

MathError organize_threads_geometrically(uint32_t num_threads, uint32_t base, ClockPosition* out_positions) {
    if (!out_positions || num_threads == 0 || base < 2) {
        return MATH_ERROR_INVALID_ARG;
    }

    for (uint32_t i = 0; i < num_threads; i++) {
        MathError err = thread_id_to_position(i, base, &out_positions[i]);
        if (err != MATH_SUCCESS) {
            return err;
        }
    }

    return MATH_SUCCESS;
}

MathError compute_thread_neighbors(uint64_t thread_id, uint32_t base, uint64_t* out_neighbors, uint32_t* out_num_neighbors) {
    if (!out_neighbors || !out_num_neighbors) {
        return MATH_ERROR_NULL_POINTER;
    }

    ClockPosition pos;
    MathError err = thread_id_to_position(thread_id, base, &pos);
    if (err != MATH_SUCCESS) {
        return err;
    }

    // Compute 12 neighbors based on clock geometry
    uint32_t ring_sizes[] = {12, 60, 60, 100};
    uint32_t ring_size = (pos.ring < 4) ? ring_sizes[pos.ring] : 100;
    
    uint32_t count = 0;
    
    // Same ring neighbors (6 neighbors: ±1, ±2, ±3 positions)
    for (int offset = -3; offset <= 3; offset++) {
        if (offset == 0) continue;
        
        ClockPosition neighbor_pos = pos;
        neighbor_pos.position = (pos.position + offset + ring_size) % ring_size;
        
        uint64_t neighbor_id;
        err = position_to_thread_id(&neighbor_pos, base, &neighbor_id);
        if (err == MATH_SUCCESS && count < 12) {
            out_neighbors[count++] = neighbor_id;
        }
    }

    // Adjacent ring neighbors
    if (pos.ring > 0 && count < 12) {
        // Inner ring neighbor
        ClockPosition inner_pos = pos;
        inner_pos.ring = pos.ring - 1;
        uint32_t inner_ring_size = (inner_pos.ring < 4) ? ring_sizes[inner_pos.ring] : 100;
        inner_pos.position = pos.position % inner_ring_size;
        
        uint64_t inner_id;
        err = position_to_thread_id(&inner_pos, base, &inner_id);
        if (err == MATH_SUCCESS) {
            out_neighbors[count++] = inner_id;
        }
    }
    
    if (count < 12) {
        // Outer ring neighbor
        ClockPosition outer_pos = pos;
        outer_pos.ring = pos.ring + 1;
        uint32_t outer_ring_size = (outer_pos.ring < 4) ? ring_sizes[outer_pos.ring] : 100;
        outer_pos.position = pos.position % outer_ring_size;
        
        uint64_t outer_id;
        err = position_to_thread_id(&outer_pos, base, &outer_id);
        if (err == MATH_SUCCESS) {
            out_neighbors[count++] = outer_id;
        }
    }

    *out_num_neighbors = count;
    return MATH_SUCCESS;
}

bool threads_are_coprime(uint64_t thread_id1, uint64_t thread_id2) {
    // Avoid zero
    uint64_t a = thread_id1 + 1;
    uint64_t b = thread_id2 + 1;
    
    return gcd(a, b) == 1;
}

MathError compute_thread_coprime_matrix(uint32_t num_threads, bool* out_coprime_matrix) {
    if (!out_coprime_matrix || num_threads == 0) {
        return MATH_ERROR_INVALID_ARG;
    }

    for (uint32_t i = 0; i < num_threads; i++) {
        for (uint32_t j = 0; j < num_threads; j++) {
            out_coprime_matrix[i * num_threads + j] = threads_are_coprime(i, j);
        }
    }

    return MATH_SUCCESS;
}

// ============================================================================
// WORK DISTRIBUTION
// ============================================================================

MathError compute_work_distribution(uint32_t num_threads, uint64_t total_work, uint64_t* out_work_per_thread) {
    if (!out_work_per_thread || num_threads == 0) {
        return MATH_ERROR_INVALID_ARG;
    }

    // Equal distribution by default
    uint64_t base_work = total_work / num_threads;
    uint64_t remainder = total_work % num_threads;

    for (uint32_t i = 0; i < num_threads; i++) {
        out_work_per_thread[i] = base_work + (i < remainder ? 1 : 0);
    }

    return MATH_SUCCESS;
}

MathError balance_load_geometrically(uint32_t num_threads, const double* thread_loads, uint64_t* out_work_redistribution) {
    if (!thread_loads || !out_work_redistribution || num_threads == 0) {
        return MATH_ERROR_INVALID_ARG;
    }

    // Compute average load
    double total_load = 0.0;
    for (uint32_t i = 0; i < num_threads; i++) {
        total_load += thread_loads[i];
    }
    double avg_load = total_load / num_threads;

    // Redistribute work to balance load
    for (uint32_t i = 0; i < num_threads; i++) {
        double imbalance = thread_loads[i] - avg_load;
        out_work_redistribution[i] = (uint64_t)(math_abs(imbalance) * 1000.0);  // Scale for precision
    }

    return MATH_SUCCESS;
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

void print_thread_geometry(uint64_t thread_id, uint32_t base) {
    ClockPosition pos;
    if (thread_id_to_position(thread_id, base, &pos) != MATH_SUCCESS) {
        printf("Error: Failed to compute position for thread %lu\n", (unsigned long)thread_id);
        return;
    }

    printf("Thread %lu Geometry:\n", (unsigned long)thread_id);
    printf("  Ring: %u, Position: %u\n", pos.ring, pos.position);
    printf("  Symmetry Group: %lu\n", (unsigned long)(thread_id % base));
    printf("  Angle: %.2f°\n", (360.0 * pos.angle) / (2.0 * M_PI));
    printf("  Radius: %.2f\n", pos.radius);

    // Compute neighbors
    uint64_t neighbors[12];
    uint32_t num_neighbors;
    if (compute_thread_neighbors(thread_id, base, neighbors, &num_neighbors) == MATH_SUCCESS) {
        printf("  Neighbors (%u): ", num_neighbors);
        for (uint32_t i = 0; i < num_neighbors; i++) {
            printf("%lu ", (unsigned long)neighbors[i]);
        }
        printf("\n");
    }
}

bool validate_thread_organization(uint32_t num_threads, const ClockPosition* positions, uint32_t base) {
    if (!positions || num_threads == 0 || base < 2) {
        return false;
    }

    // Check all positions are valid (extended rings allowed for threading)
    for (uint32_t i = 0; i < num_threads; i++) {
        // For threading, we allow extended rings beyond the standard 0-3
        // Only validate basic structure
        uint32_t ring_sizes[] = {12, 60, 60, 100};
        uint32_t ring_size = (positions[i].ring < 4) ? ring_sizes[positions[i].ring] : 100;
        
        if (positions[i].position >= ring_size) {
            printf("Error: Thread %u has invalid position %u for ring %u (max: %u)\n", 
                   i, positions[i].position, positions[i].ring, ring_size);
            return false;
        }
    }

    // Verify round-trip conversion
    for (uint32_t i = 0; i < num_threads; i++) {
        uint64_t thread_id;
        if (position_to_thread_id(&positions[i], base, &thread_id) != MATH_SUCCESS) {
            printf("Error: Failed to convert position to thread ID for thread %u\n", i);
            return false;
        }

        ClockPosition expected;
        if (thread_id_to_position(i, base, &expected) != MATH_SUCCESS) {
            printf("Error: Failed to compute expected position for thread %u\n", i);
            return false;
        }

        if (positions[i].ring != expected.ring ||
            positions[i].position != expected.position) {
            printf("Error: Position mismatch for thread %u (got ring=%u pos=%u, expected ring=%u pos=%u)\n", 
                   i, positions[i].ring, positions[i].position, expected.ring, expected.position);
            return false;
        }
    }

    return true;
}