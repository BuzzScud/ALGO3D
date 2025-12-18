/**
 * Kissing Boundary Optimization
 * 
 * Optimizes memory access at kissing boundaries between adjacent spheres.
 * Provides lock-free read access and minimal locking for writes.
 * 
 * Phase 3, Day 12 Implementation
 */

#ifndef CLLM_KISSING_BOUNDARIES_H
#define CLLM_KISSING_BOUNDARIES_H

#include <stdint.h>
#include <stddef.h>
#include <stdatomic.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// CONSTANTS
// ============================================================================

#define KISSING_BOUNDARY_SIZE (64 * 1024)  // 64 KB per boundary
#define MAX_KISSING_BOUNDARIES 66           // 12 choose 2 = 66 boundaries per level

// ============================================================================
// STRUCTURES
// ============================================================================

/**
 * Kissing Boundary
 * 
 * Shared memory region between two adjacent spheres (siblings).
 * Optimized for lock-free reads and minimal locking for writes.
 */
typedef struct {
    // Memory region
    void* boundary_memory;                  // Shared memory (cache-aligned)
    size_t boundary_size;                   // Size of boundary
    
    // Ownership
    uint32_t owner_threads[2];              // Two threads sharing boundary
    int sphere_a;                           // First sphere ID
    int sphere_b;                           // Second sphere ID
    
    // Synchronization (lock-free reads, spinlock for writes)
    atomic_flag write_lock;                 // Spinlock for writes
    atomic_uint_fast32_t version;           // Version counter for cache coherency
    atomic_uint_fast32_t readers;           // Number of active readers
    
    // Access tracking
    atomic_uint_fast64_t reads_a;           // Reads from sphere A
    atomic_uint_fast64_t reads_b;           // Reads from sphere B
    atomic_uint_fast64_t writes_a;          // Writes from sphere A
    atomic_uint_fast64_t writes_b;          // Writes from sphere B
    atomic_uint_fast64_t total_accesses;    // Total accesses
    
    // Cache coherency
    atomic_uint_fast64_t cache_invalidations; // Cache invalidation count
    atomic_uint_fast64_t version_conflicts;   // Version conflict count
    
} KissingBoundary;

/**
 * Kissing Boundary System
 * 
 * Manages all kissing boundaries for a hierarchy level.
 */
typedef struct {
    KissingBoundary* boundaries;            // Array of boundaries
    int num_boundaries;                     // Number of active boundaries
    int max_boundaries;                     // Maximum boundaries
    
    // Global statistics
    atomic_uint_fast64_t total_reads;       // Total reads across all boundaries
    atomic_uint_fast64_t total_writes;      // Total writes across all boundaries
    atomic_uint_fast64_t total_conflicts;   // Total conflicts
    
} KissingBoundarySystem;

// ============================================================================
// LIFECYCLE
// ============================================================================

/**
 * Create kissing boundary
 * 
 * @param sphere_a First sphere ID
 * @param sphere_b Second sphere ID
 * @param size Boundary size (default: KISSING_BOUNDARY_SIZE)
 * @return Allocated boundary, or NULL on error
 */
KissingBoundary* kissing_boundary_create(
    int sphere_a,
    int sphere_b,
    size_t size
);

/**
 * Destroy kissing boundary
 * 
 * @param boundary Boundary to destroy
 */
void kissing_boundary_destroy(KissingBoundary* boundary);

/**
 * Create kissing boundary system
 * 
 * @param max_boundaries Maximum number of boundaries
 * @return Allocated system, or NULL on error
 */
KissingBoundarySystem* kissing_boundary_system_create(int max_boundaries);

/**
 * Destroy kissing boundary system
 * 
 * @param system System to destroy
 */
void kissing_boundary_system_destroy(KissingBoundarySystem* system);

// ============================================================================
// MEMORY ACCESS (LOCK-FREE READS)
// ============================================================================

/**
 * Read from kissing boundary (lock-free)
 * 
 * Lock-free read access for maximum performance.
 * 
 * @param boundary Kissing boundary
 * @param sphere_id Sphere ID (A or B)
 * @param offset Offset in boundary
 * @param buffer Buffer to read into
 * @param size Size to read
 * @return Number of bytes read, or 0 on error
 */
size_t kissing_boundary_read(
    KissingBoundary* boundary,
    int sphere_id,
    size_t offset,
    void* buffer,
    size_t size
);

/**
 * Write to kissing boundary (minimal locking)
 * 
 * Uses spinlock for writes to ensure consistency.
 * 
 * @param boundary Kissing boundary
 * @param sphere_id Sphere ID (A or B)
 * @param offset Offset in boundary
 * @param buffer Buffer to write from
 * @param size Size to write
 * @return Number of bytes written, or 0 on error
 */
size_t kissing_boundary_write(
    KissingBoundary* boundary,
    int sphere_id,
    size_t offset,
    const void* buffer,
    size_t size
);

/**
 * Atomic exchange on kissing boundary
 * 
 * Atomically exchanges data at boundary.
 * 
 * @param boundary Kissing boundary
 * @param sphere_id Sphere ID (A or B)
 * @param offset Offset in boundary
 * @param new_value New value to write
 * @param old_value Buffer to store old value
 * @param size Size of value (must be 4 or 8 bytes)
 * @return 1 on success, 0 on error
 */
int kissing_boundary_atomic_exchange(
    KissingBoundary* boundary,
    int sphere_id,
    size_t offset,
    const void* new_value,
    void* old_value,
    size_t size
);

// ============================================================================
// CACHE COHERENCY
// ============================================================================

/**
 * Get boundary version
 * 
 * Returns current version for cache coherency checks.
 * 
 * @param boundary Kissing boundary
 * @return Current version
 */
uint32_t kissing_boundary_get_version(const KissingBoundary* boundary);

/**
 * Invalidate cache
 * 
 * Increments version to invalidate cached copies.
 * 
 * @param boundary Kissing boundary
 */
void kissing_boundary_invalidate_cache(KissingBoundary* boundary);

/**
 * Check version
 * 
 * Checks if cached version is still valid.
 * 
 * @param boundary Kissing boundary
 * @param cached_version Cached version to check
 * @return 1 if valid, 0 if invalid
 */
int kissing_boundary_check_version(
    const KissingBoundary* boundary,
    uint32_t cached_version
);

// ============================================================================
// STATISTICS
// ============================================================================

/**
 * Get boundary statistics
 * 
 * @param boundary Kissing boundary
 * @param reads_a Output: reads from sphere A
 * @param reads_b Output: reads from sphere B
 * @param writes_a Output: writes from sphere A
 * @param writes_b Output: writes from sphere B
 */
void kissing_boundary_get_stats(
    const KissingBoundary* boundary,
    uint64_t* reads_a,
    uint64_t* reads_b,
    uint64_t* writes_a,
    uint64_t* writes_b
);

/**
 * Print boundary statistics
 * 
 * @param boundary Kissing boundary
 */
void kissing_boundary_print_stats(const KissingBoundary* boundary);

/**
 * Reset boundary statistics
 * 
 * @param boundary Kissing boundary
 */
void kissing_boundary_reset_stats(KissingBoundary* boundary);

/**
 * Get system statistics
 * 
 * @param system Kissing boundary system
 * @param total_reads Output: total reads
 * @param total_writes Output: total writes
 * @param total_conflicts Output: total conflicts
 */
void kissing_boundary_system_get_stats(
    const KissingBoundarySystem* system,
    uint64_t* total_reads,
    uint64_t* total_writes,
    uint64_t* total_conflicts
);

/**
 * Print system statistics
 * 
 * @param system Kissing boundary system
 */
void kissing_boundary_system_print_stats(const KissingBoundarySystem* system);

// ============================================================================
// SYSTEM MANAGEMENT
// ============================================================================

/**
 * Add boundary to system
 * 
 * @param system Kissing boundary system
 * @param boundary Boundary to add
 * @return 0 on success, -1 on error
 */
int kissing_boundary_system_add(
    KissingBoundarySystem* system,
    KissingBoundary* boundary
);

/**
 * Find boundary between two spheres
 * 
 * @param system Kissing boundary system
 * @param sphere_a First sphere ID
 * @param sphere_b Second sphere ID
 * @return Boundary, or NULL if not found
 */
KissingBoundary* kissing_boundary_system_find(
    const KissingBoundarySystem* system,
    int sphere_a,
    int sphere_b
);

/**
 * Validate boundary system
 * 
 * @param system Kissing boundary system
 * @return 1 if valid, 0 if invalid
 */
int kissing_boundary_system_validate(const KissingBoundarySystem* system);

#ifdef __cplusplus
}
#endif

#endif // CLLM_KISSING_BOUNDARIES_H
