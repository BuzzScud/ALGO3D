#ifndef CLLM_CRYSTALLINE_MEMORY_H
#define CLLM_CRYSTALLINE_MEMORY_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdatomic.h>
#include "ai/cllm_shared_memory.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Crystalline Memory System
 * 
 * Implements 12-fold memory organization based on kissing spheres geometry.
 * Memory is organized into 12 segments corresponding to the 12 symmetry groups.
 * 
 * Key Features:
 * - 12-fold symmetry in memory layout
 * - Cache-aligned segments (64-byte alignment)
 * - Hierarchical memory relationships
 * - Shared boundaries between adjacent segments
 * - NUMA-aware allocation
 */

// Number of symmetry groups (12-fold)
#define NUM_SYMMETRY_GROUPS 12

// Cache line size for alignment
#define CRYSTALLINE_CACHE_LINE 64

// Memory segment types
typedef enum {
    SEGMENT_CONTROL,      // Control thread memory (read-only for workers)
    SEGMENT_WORKER,       // Worker thread memory (read-write)
    SEGMENT_SHARED,       // Shared between parent/child
    SEGMENT_BOUNDARY      // Kissing boundary between siblings
} SegmentType;

/**
 * Crystalline Memory Segment
 * 
 * Represents one of the 12 memory segments in the crystalline structure.
 */
typedef struct {
    void* data;                    // Segment data (cache-aligned)
    size_t size;                   // Segment size in bytes
    uint32_t symmetry_group;       // Which symmetry group (0-11)
    SegmentType type;              // Segment type
    
    // Access tracking
    atomic_uint_fast64_t read_count;
    atomic_uint_fast64_t write_count;
    
    // NUMA information
    int numa_node;                 // NUMA node this segment is on
    
    // Padding to cache line
    char padding[CRYSTALLINE_CACHE_LINE - 
                 (sizeof(void*) + sizeof(size_t) + sizeof(uint32_t) + 
                  sizeof(SegmentType) + sizeof(atomic_uint_fast64_t) * 2 + 
                  sizeof(int)) % CRYSTALLINE_CACHE_LINE];
} CrystallineSegment;

/**
 * Crystalline Memory Block
 * 
 * A complete 12-fold memory structure with all segments.
 */
typedef struct {
    CrystallineSegment segments[NUM_SYMMETRY_GROUPS];  // 12 segments
    size_t total_size;                                  // Total allocated size
    size_t segment_size;                                // Size per segment
    
    // Ownership
    int owner_sphere_id;                                // Sphere that owns this block
    int hierarchy_level;                                // Level in hierarchy
    
    // Shared memory regions (for parent/child communication)
    SharedMemoryRegion* parent_shared;                  // Shared with parent
    SharedMemoryRegion* child_shared[NUM_SYMMETRY_GROUPS]; // Shared with children
    
    // Statistics
    atomic_uint_fast64_t total_reads;
    atomic_uint_fast64_t total_writes;
    atomic_uint_fast64_t cache_hits;
    atomic_uint_fast64_t cache_misses;
    
} CrystallineMemoryBlock;

/**
 * Kissing Boundary
 * 
 * Shared memory region between two adjacent segments (siblings).
 */
typedef struct {
    void* boundary_memory;                // Shared memory region
    size_t boundary_size;                 // Size of boundary
    
    uint32_t segment_a;                   // First segment (0-11)
    uint32_t segment_b;                   // Second segment (0-11)
    
    // Lock-free synchronization
    atomic_flag lock;                     // Spinlock for writes
    atomic_uint_fast32_t version;         // Version counter
    
    // Access tracking
    atomic_uint_fast64_t accesses_a;      // Accesses from segment A
    atomic_uint_fast64_t accesses_b;      // Accesses from segment B
    
} KissingBoundary;

// ============================================================================
// MEMORY BLOCK LIFECYCLE
// ============================================================================

/**
 * Create crystalline memory block
 * 
 * Allocates memory in 12-fold structure with cache alignment.
 * 
 * @param total_size Total size to allocate (divided into 12 segments)
 * @param owner_sphere_id Sphere ID that owns this block
 * @param hierarchy_level Level in hierarchy
 * @return Allocated block, or NULL on error
 */
CrystallineMemoryBlock* crystalline_memory_create(
    size_t total_size,
    int owner_sphere_id,
    int hierarchy_level
);

/**
 * Destroy crystalline memory block
 * 
 * Frees all segments and shared memory regions.
 * 
 * @param block Block to destroy
 */
void crystalline_memory_destroy(CrystallineMemoryBlock* block);

// ============================================================================
// SEGMENT ACCESS
// ============================================================================

/**
 * Get segment by symmetry group
 * 
 * @param block Memory block
 * @param symmetry_group Symmetry group (0-11)
 * @return Pointer to segment, or NULL if invalid
 */
CrystallineSegment* crystalline_memory_get_segment(
    CrystallineMemoryBlock* block,
    uint32_t symmetry_group
);

/**
 * Read from segment
 * 
 * Thread-safe read access to segment data.
 * 
 * @param segment Segment to read from
 * @param offset Offset within segment
 * @param buffer Buffer to read into
 * @param size Number of bytes to read
 * @return Number of bytes read, or 0 on error
 */
size_t crystalline_segment_read(
    CrystallineSegment* segment,
    size_t offset,
    void* buffer,
    size_t size
);

/**
 * Write to segment
 * 
 * Thread-safe write access to segment data.
 * 
 * @param segment Segment to write to
 * @param offset Offset within segment
 * @param data Data to write
 * @param size Number of bytes to write
 * @return Number of bytes written, or 0 on error
 */
size_t crystalline_segment_write(
    CrystallineSegment* segment,
    size_t offset,
    const void* data,
    size_t size
);

// ============================================================================
// KISSING BOUNDARIES
// ============================================================================

/**
 * Create kissing boundary between two segments
 * 
 * @param block Memory block
 * @param segment_a First segment (0-11)
 * @param segment_b Second segment (0-11)
 * @param boundary_size Size of boundary region
 * @return Created boundary, or NULL on error
 */
KissingBoundary* crystalline_boundary_create(
    CrystallineMemoryBlock* block,
    uint32_t segment_a,
    uint32_t segment_b,
    size_t boundary_size
);

/**
 * Destroy kissing boundary
 * 
 * @param boundary Boundary to destroy
 */
void crystalline_boundary_destroy(KissingBoundary* boundary);

/**
 * Access kissing boundary (read)
 * 
 * Lock-free read access to boundary memory.
 * 
 * @param boundary Boundary to access
 * @param segment_id Which segment is accessing (for tracking)
 * @return Pointer to boundary memory
 */
const void* crystalline_boundary_read(
    KissingBoundary* boundary,
    uint32_t segment_id
);

/**
 * Access kissing boundary (write)
 * 
 * Acquires spinlock for write access.
 * 
 * @param boundary Boundary to access
 * @param segment_id Which segment is accessing (for tracking)
 * @return Pointer to writable boundary memory
 */
void* crystalline_boundary_write(
    KissingBoundary* boundary,
    uint32_t segment_id
);

/**
 * Release kissing boundary write lock
 * 
 * @param boundary Boundary to release
 */
void crystalline_boundary_release(KissingBoundary* boundary);

// ============================================================================
// HIERARCHICAL MEMORY
// ============================================================================

/**
 * Link parent and child memory blocks
 * 
 * Creates shared memory region between parent and child.
 * 
 * @param parent Parent memory block
 * @param child Child memory block
 * @param child_symmetry_group Child's symmetry group (0-11)
 * @param shared_size Size of shared region
 * @return 1 on success, 0 on error
 */
int crystalline_memory_link_parent_child(
    CrystallineMemoryBlock* parent,
    CrystallineMemoryBlock* child,
    uint32_t child_symmetry_group,
    size_t shared_size
);

/**
 * Get parent shared memory
 * 
 * @param block Child memory block
 * @return Shared memory region with parent, or NULL
 */
SharedMemoryRegion* crystalline_memory_get_parent_shared(
    CrystallineMemoryBlock* block
);

/**
 * Get child shared memory
 * 
 * @param block Parent memory block
 * @param child_symmetry_group Child's symmetry group (0-11)
 * @return Shared memory region with child, or NULL
 */
SharedMemoryRegion* crystalline_memory_get_child_shared(
    CrystallineMemoryBlock* block,
    uint32_t child_symmetry_group
);

// ============================================================================
// STATISTICS
// ============================================================================

/**
 * Get memory block statistics
 * 
 * @param block Memory block
 * @param total_reads Output: total reads
 * @param total_writes Output: total writes
 * @param cache_hits Output: cache hits
 * @param cache_misses Output: cache misses
 */
void crystalline_memory_get_stats(
    const CrystallineMemoryBlock* block,
    uint64_t* total_reads,
    uint64_t* total_writes,
    uint64_t* cache_hits,
    uint64_t* cache_misses
);

/**
 * Print memory block statistics
 * 
 * @param block Memory block
 */
void crystalline_memory_print_stats(const CrystallineMemoryBlock* block);

/**
 * Reset memory block statistics
 * 
 * @param block Memory block
 */
void crystalline_memory_reset_stats(CrystallineMemoryBlock* block);

// ============================================================================
// UTILITIES
// ============================================================================

/**
 * Validate memory block
 * 
 * Checks for consistency and correctness.
 * 
 * @param block Memory block
 * @return 1 if valid, 0 if errors found
 */
int crystalline_memory_validate(const CrystallineMemoryBlock* block);

/**
 * Print memory block information
 * 
 * @param block Memory block
 */
void crystalline_memory_print(const CrystallineMemoryBlock* block);

#ifdef __cplusplus
}
#endif

#endif // CLLM_CRYSTALLINE_MEMORY_H
