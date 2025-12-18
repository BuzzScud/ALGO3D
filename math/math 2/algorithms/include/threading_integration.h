/**
 * @file threading_integration.h
 * @brief Integration between Math Library and Threading System
 * 
 * Provides functions to integrate abacus, rainbow table, and clock lattice
 * with hierarchical threading and memory systems.
 * 
 * Key Features:
 * - Thread ID to geometric position mapping using clock lattice
 * - Memory segment to abacus representation
 * - Rainbow table for O(1) thread lookups
 * - Geometric thread organization
 */

#ifndef THREADING_INTEGRATION_H
#define THREADING_INTEGRATION_H

#include "math/types.h"
#include "math/abacus.h"
#include "math/rainbow.h"
#include "math/clock.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// THREAD ID TO GEOMETRIC POSITION
// ============================================================================

/**
 * Map thread ID to geometric position on clock lattice
 * 
 * Uses clock lattice to compute geometric position for a thread.
 * 
 * @param thread_id Thread ID
 * @param base Number base (typically 12 for 12-fold symmetry)
 * @param out_position Output clock position
 * @return MATH_SUCCESS on success, error code otherwise
 */
MathError thread_id_to_position(
    uint64_t thread_id,
    uint32_t base,
    ClockPosition* out_position
);

/**
 * Map geometric position to thread ID
 * 
 * Inverse of thread_id_to_position.
 * 
 * @param position Clock position
 * @param base Number base
 * @param out_thread_id Output thread ID
 * @return MATH_SUCCESS on success, error code otherwise
 */
MathError position_to_thread_id(
    const ClockPosition* position,
    uint32_t base,
    uint64_t* out_thread_id
);

/**
 * Compute N-dimensional position for thread
 * 
 * Projects thread position into N-dimensional space using
 * geometric transformations based on clock position.
 * 
 * @param thread_id Thread ID
 * @param num_dimensions Number of dimensions
 * @param out_position Output position [num_dimensions]
 * @return MATH_SUCCESS on success, error code otherwise
 */
MathError thread_compute_nd_position(
    uint64_t thread_id,
    uint32_t num_dimensions,
    double* out_position
);

// ============================================================================
// MEMORY SEGMENT TO ABACUS
// ============================================================================

/**
 * Map memory segment to abacus representation
 * 
 * Converts segment ID to abacus for geometric operations.
 * 
 * @param segment_id Segment ID
 * @param base Number base
 * @return Abacus representation, or NULL on error
 */
CrystallineAbacus* segment_to_abacus(
    uint64_t segment_id,
    uint32_t base
);

/**
 * Map abacus to memory segment ID
 * 
 * Inverse of segment_to_abacus.
 * 
 * @param abacus Abacus representation
 * @param out_segment_id Output segment ID
 * @return MATH_SUCCESS on success, error code otherwise
 */
MathError abacus_to_segment(
    const CrystallineAbacus* abacus,
    uint64_t* out_segment_id
);

// ============================================================================
// RAINBOW TABLE FOR THREADING
// ============================================================================

/**
 * Create rainbow table for thread position lookups
 * 
 * Populates rainbow table with thread ID to position mappings
 * for O(1) lookup performance.
 * 
 * @param max_threads Maximum number of threads
 * @param base Number base
 * @param out_table Output rainbow table
 * @return MATH_SUCCESS on success, error code otherwise
 */
MathError create_thread_rainbow_table(
    uint64_t max_threads,
    uint32_t base,
    RainbowTable* out_table
);

/**
 * Lookup thread position in rainbow table
 * 
 * O(1) lookup of thread's geometric position.
 * 
 * @param table Rainbow table
 * @param thread_id Thread ID
 * @param out_position Output clock position
 * @return MATH_SUCCESS on success, error code otherwise
 */
MathError rainbow_lookup_thread_position(
    const RainbowTable* table,
    uint64_t thread_id,
    ClockPosition* out_position
);

// ============================================================================
// GEOMETRIC THREAD ORGANIZATION
// ============================================================================

/**
 * Organize threads geometrically using clock lattice
 * 
 * Computes geometric positions for all threads based on
 * clock lattice structure.
 * 
 * @param num_threads Number of threads
 * @param base Number base
 * @param out_positions Output positions [num_threads]
 * @return MATH_SUCCESS on success, error code otherwise
 */
MathError organize_threads_geometrically(
    uint32_t num_threads,
    uint32_t base,
    ClockPosition* out_positions
);

/**
 * Compute 12 neighbors for a thread using kissing sphere geometry
 * 
 * Finds the 12 nearest neighbors based on clock lattice geometry.
 * 
 * @param thread_id Thread ID
 * @param base Number base
 * @param out_neighbors Output neighbor IDs [12]
 * @param out_num_neighbors Output number of neighbors found
 * @return MATH_SUCCESS on success, error code otherwise
 */
MathError compute_thread_neighbors(
    uint64_t thread_id,
    uint32_t base,
    uint64_t* out_neighbors,
    uint32_t* out_num_neighbors
);

/**
 * Check if two threads are coprime
 * 
 * Determines if two thread IDs have coprime relationship,
 * which affects their communication patterns.
 * 
 * @param thread_id1 First thread ID
 * @param thread_id2 Second thread ID
 * @return true if coprime, false otherwise
 */
bool threads_are_coprime(
    uint64_t thread_id1,
    uint64_t thread_id2
);

/**
 * Compute coprime matrix for threads
 * 
 * Creates a matrix indicating which thread pairs are coprime.
 * 
 * @param num_threads Number of threads
 * @param out_coprime_matrix Output matrix [num_threads * num_threads]
 * @return MATH_SUCCESS on success, error code otherwise
 */
MathError compute_thread_coprime_matrix(
    uint32_t num_threads,
    bool* out_coprime_matrix
);

// ============================================================================
// WORK DISTRIBUTION
// ============================================================================

/**
 * Compute work distribution based on geometric positions
 * 
 * Distributes work across threads based on their geometric
 * positions in the clock lattice.
 * 
 * @param num_threads Number of threads
 * @param total_work Total amount of work to distribute
 * @param out_work_per_thread Output work allocation [num_threads]
 * @return MATH_SUCCESS on success, error code otherwise
 */
MathError compute_work_distribution(
    uint32_t num_threads,
    uint64_t total_work,
    uint64_t* out_work_per_thread
);

/**
 * Balance load across threads using geometric transformations
 * 
 * Redistributes work to balance load based on current thread loads
 * and geometric relationships.
 * 
 * @param num_threads Number of threads
 * @param thread_loads Current load per thread [num_threads]
 * @param out_work_redistribution Output redistribution [num_threads]
 * @return MATH_SUCCESS on success, error code otherwise
 */
MathError balance_load_geometrically(
    uint32_t num_threads,
    const double* thread_loads,
    uint64_t* out_work_redistribution
);

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * Print thread geometry information
 * 
 * Displays geometric position and properties for a thread.
 * 
 * @param thread_id Thread ID
 * @param base Number base
 */
void print_thread_geometry(
    uint64_t thread_id,
    uint32_t base
);

/**
 * Validate thread organization
 * 
 * Checks that thread positions are valid and properly organized.
 * 
 * @param num_threads Number of threads
 * @param positions Thread positions [num_threads]
 * @param base Number base
 * @return true if valid, false otherwise
 */
bool validate_thread_organization(
    uint32_t num_threads,
    const ClockPosition* positions,
    uint32_t base
);

#ifdef __cplusplus
}
#endif

#endif // THREADING_INTEGRATION_H