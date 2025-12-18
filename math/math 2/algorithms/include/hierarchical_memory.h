/**
 * @file hierarchical_memory.h
 * @brief Hierarchical Memory System - Inspired by Platonic Solid Generator
 * 
 * This implements a dynamic, scalable memory system based on geometric principles
 * from the Platonic Solid Generator. Key features:
 * 
 * - Dynamic scaling (segments, dimensions)
 * - Geometric organization (abacus-based)
 * - Self-similar replication
 * - Coprime relationships
 * - Cache-aligned segments
 * - NUMA-aware allocation
 * 
 * DESIGN PRINCIPLES (from Platonic Solid Generator):
 * 1. Start small, scale dynamically (not pre-allocate)
 * 2. Monitor stability at multiple scales
 * 3. Use self-similar replication
 * 4. Organize geometrically (abacus + rainbow table)
 * 5. Track scaling history
 */

#ifndef ALGORITHMS_HIERARCHICAL_MEMORY_H
#define ALGORITHMS_HIERARCHICAL_MEMORY_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdatomic.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// CONSTANTS
// ============================================================================

// Cache line size for alignment (prevents false sharing)
#define HIERARCHICAL_CACHE_LINE 64

// Default minimum segments (12-fold symmetry)
#define HIERARCHICAL_MIN_SEGMENTS 12

// Default minimum dimensions
#define HIERARCHICAL_MIN_DIMENSIONS 13

// ============================================================================
// SEGMENT TYPES
// ============================================================================

/**
 * Memory segment type
 */
typedef enum {
    SEGMENT_CONTROL,      // Control thread memory (read-only for workers)
    SEGMENT_WORKER,       // Worker thread memory (read-write)
    SEGMENT_SHARED,       // Shared between parent/child
    SEGMENT_BOUNDARY      // Kissing boundary between siblings
} SegmentType;

// ============================================================================
// HIERARCHICAL SEGMENT
// ============================================================================

/**
 * Hierarchical Memory Segment
 * 
 * Represents one segment in the hierarchical memory structure.
 * Inspired by Platonic Solid vertices.
 */
typedef struct {
    // Data
    void* data;                    // Segment data (cache-aligned)
    size_t size;                   // Segment size in bytes
    size_t capacity;               // Allocated capacity
    
    // Identity
    uint32_t segment_id;           // Segment ID (0 to num_segments-1)
    uint32_t symmetry_group;       // Which symmetry group (0 to N-1)
    SegmentType type;              // Segment type
    
    // Geometric position (like Platonic vertex)
    double* position;              // Position in N-dimensional space
    uint32_t num_dimensions;       // Number of dimensions
    
    // Access tracking (atomic for thread safety)
    atomic_uint_fast64_t read_count;
    atomic_uint_fast64_t write_count;
    
    // NUMA information
    int numa_node;                 // NUMA node this segment is on
    
    // Padding to cache line boundary
    char padding[HIERARCHICAL_CACHE_LINE - 
                 (sizeof(void*) * 2 + sizeof(size_t) * 2 + 
                  sizeof(uint32_t) * 3 + sizeof(SegmentType) + 
                  sizeof(atomic_uint_fast64_t) * 2 + sizeof(int)) % HIERARCHICAL_CACHE_LINE];
} __attribute__((aligned(HIERARCHICAL_CACHE_LINE))) HierarchicalSegment;

// ============================================================================
// KISSING BOUNDARY
// ============================================================================

/**
 * Kissing Boundary
 * 
 * Shared memory region between two adjacent segments (siblings).
 * Inspired by Platonic Solid edges.
 */
typedef struct {
    // Boundary memory
    void* boundary_memory;         // Shared memory region
    size_t boundary_size;          // Size of boundary
    
    // Connected segments
    uint32_t segment_a;            // First segment ID
    uint32_t segment_b;            // Second segment ID
    
    // Lock-free synchronization
    atomic_flag lock;              // Spinlock for writes
    atomic_uint_fast32_t version;  // Version counter
    
    // Access tracking (atomic)
    atomic_uint_fast64_t accesses_a;  // Accesses from segment A
    atomic_uint_fast64_t accesses_b;  // Accesses from segment B
    
    // Padding to cache line
    char padding[HIERARCHICAL_CACHE_LINE - 
                 (sizeof(void*) + sizeof(size_t) + sizeof(uint32_t) * 2 + 
                  sizeof(atomic_flag) + sizeof(atomic_uint_fast32_t) + 
                  sizeof(atomic_uint_fast64_t) * 2) % HIERARCHICAL_CACHE_LINE];
} __attribute__((aligned(HIERARCHICAL_CACHE_LINE))) KissingBoundary;

// ============================================================================
// HIERARCHICAL MEMORY BLOCK
// ============================================================================

/**
 * Hierarchical Memory Block
 * 
 * Complete hierarchical memory structure with dynamic scaling.
 * Inspired by Platonic Solid Model.
 */
typedef struct {
    // Identity
    char block_id[64];             // Unique identifier
    int owner_id;                  // Owner ID
    int hierarchy_level;           // Level in hierarchy
    
    // Segments (like Platonic vertices) - DYNAMIC
    HierarchicalSegment* segments; // Array of segments
    uint32_t num_segments;         // Current number of segments
    uint32_t max_segments;         // Maximum allowed (for scaling)
    uint32_t min_segments;         // Minimum required (12 for 12-fold)
    
    // Dimensions (like Platonic dimensions) - DYNAMIC
    uint32_t num_dimensions;       // Current dimensions
    uint32_t max_dimensions;       // Maximum allowed (for scaling)
    uint32_t min_dimensions;       // Minimum required (13)
    
    // Boundaries (like Platonic edges)
    KissingBoundary** boundaries;  // Array of boundaries
    uint32_t num_boundaries;       // Number of boundaries
    uint32_t max_boundaries;       // Maximum boundaries
    
    // Coprime relationships (like Platonic coprime matrix)
    uint64_t** coprime_matrix;     // [num_dimensions * num_dimensions]
    
    // Total memory
    size_t total_size;             // Total allocated size
    size_t segment_size;           // Size per segment
    
    // Statistics (atomic)
    atomic_uint_fast64_t total_reads;
    atomic_uint_fast64_t total_writes;
    atomic_uint_fast64_t cache_hits;
    atomic_uint_fast64_t cache_misses;
    
    // Scaling history (like Platonic scaling)
    uint32_t num_segment_scales;   // How many times segments scaled
    uint32_t num_dimension_scales;  // How many times dimensions scaled
    uint32_t* segment_scale_history;  // [num_segment_scales]
    uint32_t* dimension_scale_history; // [num_dimension_scales]
    
    // Synchronization
    pthread_mutex_t block_mutex;   // Protects block operations
    
} HierarchicalMemoryBlock;

// ============================================================================
// LIFECYCLE
// ============================================================================

/**
 * Create hierarchical memory block
 * 
 * Creates a memory block with dynamic scaling capability.
 * Starts with minimum complexity, scales up as needed.
 * 
 * @param total_size Total size to allocate (divided into segments)
 * @param initial_segments Initial number of segments (>= min_segments)
 * @param initial_dimensions Initial number of dimensions (>= min_dimensions)
 * @param owner_id Owner ID
 * @param hierarchy_level Level in hierarchy
 * @return Allocated block, or NULL on error
 */
HierarchicalMemoryBlock* hierarchical_memory_create(
    size_t total_size,
    uint32_t initial_segments,
    uint32_t initial_dimensions,
    int owner_id,
    int hierarchy_level
);

/**
 * Destroy hierarchical memory block
 * 
 * Frees all segments, boundaries, and associated data.
 * 
 * @param block Block to destroy
 */
void hierarchical_memory_destroy(HierarchicalMemoryBlock* block);

// ============================================================================
// SEGMENT ACCESS
// ============================================================================

/**
 * Get segment by ID
 * 
 * @param block Memory block
 * @param segment_id Segment ID (0 to num_segments-1)
 * @return Pointer to segment, or NULL if invalid
 */
HierarchicalSegment* hierarchical_memory_get_segment(
    HierarchicalMemoryBlock* block,
    uint32_t segment_id
);

/**
 * Get segment by symmetry group
 * 
 * @param block Memory block
 * @param symmetry_group Symmetry group (0 to N-1)
 * @return Pointer to segment, or NULL if invalid
 */
HierarchicalSegment* hierarchical_memory_get_segment_by_symmetry(
    HierarchicalMemoryBlock* block,
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
size_t hierarchical_segment_read(
    HierarchicalSegment* segment,
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
size_t hierarchical_segment_write(
    HierarchicalSegment* segment,
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
 * @param segment_a First segment ID
 * @param segment_b Second segment ID
 * @param boundary_size Size of boundary region
 * @return Created boundary, or NULL on error
 */
KissingBoundary* hierarchical_boundary_create(
    HierarchicalMemoryBlock* block,
    uint32_t segment_a,
    uint32_t segment_b,
    size_t boundary_size
);

/**
 * Destroy kissing boundary
 * 
 * @param boundary Boundary to destroy
 */
void hierarchical_boundary_destroy(KissingBoundary* boundary);

/**
 * Access kissing boundary (read)
 * 
 * Lock-free read access to boundary memory.
 * 
 * @param boundary Boundary to access
 * @param segment_id Which segment is accessing (for tracking)
 * @return Pointer to boundary memory
 */
const void* hierarchical_boundary_read(
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
void* hierarchical_boundary_write(
    KissingBoundary* boundary,
    uint32_t segment_id
);

/**
 * Release kissing boundary write lock
 * 
 * @param boundary Boundary to release
 */
void hierarchical_boundary_release(KissingBoundary* boundary);

// ============================================================================
// DYNAMIC SCALING (inspired by Platonic Solid Generator)
// ============================================================================

/**
 * Scale up segments
 * 
 * Increases number of segments using self-similar replication.
 * Inspired by platonic_scale_vertices().
 * 
 * @param block Memory block
 * @param scale_factor Multiply segments by this (e.g., 2 for doubling)
 * @return true if scaled successfully
 */
bool hierarchical_memory_scale_segments(
    HierarchicalMemoryBlock* block,
    uint32_t scale_factor
);

/**
 * Scale up dimensions
 * 
 * Increases number of dimensions using prime-based projection.
 * Inspired by platonic_scale_dimensions().
 * 
 * @param block Memory block
 * @param additional_dimensions How many dimensions to add
 * @return true if scaled successfully
 */
bool hierarchical_memory_scale_dimensions(
    HierarchicalMemoryBlock* block,
    uint32_t additional_dimensions
);

/**
 * Check if scaling is needed
 * 
 * Determines if block should scale based on usage patterns.
 * 
 * @param block Memory block
 * @param out_scale_segments Output: should scale segments
 * @param out_scale_dimensions Output: should scale dimensions
 * @return true if any scaling needed
 */
bool hierarchical_memory_should_scale(
    const HierarchicalMemoryBlock* block,
    bool* out_scale_segments,
    bool* out_scale_dimensions
);

// ============================================================================
// GEOMETRIC ORGANIZATION (abacus + rainbow table integration)
// ============================================================================

/**
 * Organize segments geometrically
 * 
 * Positions segments in N-dimensional space based on geometric principles.
 * Uses abacus for positioning, rainbow table for lookup.
 * 
 * @param block Memory block
 * @return 0 on success, -1 on error
 */
int hierarchical_memory_organize_geometrically(
    HierarchicalMemoryBlock* block
);

/**
 * Map segment to geometric position
 * 
 * Computes geometric position for a segment based on its ID and symmetry group.
 * 
 * @param segment_id Segment ID
 * @param symmetry_group Symmetry group
 * @param num_dimensions Number of dimensions
 * @param out_position Output position [num_dimensions]
 * @return 0 on success, -1 on error
 */
int hierarchical_memory_compute_position(
    uint32_t segment_id,
    uint32_t symmetry_group,
    uint32_t num_dimensions,
    double* out_position
);

// ============================================================================
// COPRIME RELATIONSHIPS
// ============================================================================

/**
 * Compute coprime matrix
 * 
 * Computes coprime relationships between all dimensions.
 * Used to determine valid communication patterns.
 * 
 * @param block Memory block
 * @return 0 on success, -1 on error
 */
int hierarchical_memory_compute_coprime_matrix(
    HierarchicalMemoryBlock* block
);

/**
 * Check if two segments can communicate
 * 
 * Uses coprime relationships to determine if communication is allowed.
 * 
 * @param block Memory block
 * @param segment_a First segment ID
 * @param segment_b Second segment ID
 * @return true if communication allowed
 */
bool hierarchical_memory_can_communicate(
    const HierarchicalMemoryBlock* block,
    uint32_t segment_a,
    uint32_t segment_b
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
void hierarchical_memory_get_stats(
    const HierarchicalMemoryBlock* block,
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
void hierarchical_memory_print_stats(const HierarchicalMemoryBlock* block);

/**
 * Reset memory block statistics
 * 
 * @param block Memory block
 */
void hierarchical_memory_reset_stats(HierarchicalMemoryBlock* block);

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
int hierarchical_memory_validate(const HierarchicalMemoryBlock* block);

/**
 * Print memory block information
 * 
 * @param block Memory block
 */
void hierarchical_memory_print(const HierarchicalMemoryBlock* block);

/**
 * Get scaling history
 * 
 * @param block Memory block
 * @param out_segment_scales Output: segment scale history
 * @param out_num_segment_scales Output: number of segment scales
 * @param out_dimension_scales Output: dimension scale history
 * @param out_num_dimension_scales Output: number of dimension scales
 */
void hierarchical_memory_get_scaling_history(
    const HierarchicalMemoryBlock* block,
    uint32_t** out_segment_scales,
    uint32_t* out_num_segment_scales,
    uint32_t** out_dimension_scales,
    uint32_t* out_num_dimension_scales
);

#ifdef __cplusplus
}
#endif

#endif // ALGORITHMS_HIERARCHICAL_MEMORY_H