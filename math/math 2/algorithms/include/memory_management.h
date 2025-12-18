#ifndef ALGORITHMS_MEMORY_MANAGEMENT_H
#define ALGORITHMS_MEMORY_MANAGEMENT_H

/**
 * Generic Memory Management for Sphere-Based Structures
 * 
 * Provides efficient memory allocation and management for sphere-based
 * data structures with cache optimization and memory pooling.
 * 
 * This is a generic memory management system that can be used by:
 * - Sphere threading systems
 * - Visualization systems
 * - Any data structure with spatial locality
 * 
 * Key features:
 * - Cache-aware allocation
 * - Memory pooling for reduced fragmentation
 * - Alignment optimization
 * - Memory statistics and profiling
 */

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// CONSTANTS
// ============================================================================

#define MEMORY_CACHE_LINE_SIZE 64
#define MEMORY_DEFAULT_POOL_SIZE (1024 * 1024)  // 1MB
#define MEMORY_MAX_POOLS 16

// ============================================================================
// MEMORY POOL TYPES
// ============================================================================

/**
 * Memory Pool Type
 * 
 * Different pool types for different allocation patterns.
 */
typedef enum {
    MEMORY_POOL_SMALL,      // Small allocations (< 256 bytes)
    MEMORY_POOL_MEDIUM,     // Medium allocations (256 - 4KB)
    MEMORY_POOL_LARGE,      // Large allocations (> 4KB)
    MEMORY_POOL_ALIGNED     // Cache-aligned allocations
} MemoryPoolType;

// ============================================================================
// DATA STRUCTURES
// ============================================================================

/**
 * Memory Block
 * 
 * Represents a single allocated memory block.
 */
typedef struct MemoryBlock {
    void* data;                 // Pointer to allocated data
    size_t size;                // Size of allocation
    size_t alignment;           // Alignment requirement
    struct MemoryBlock* next;   // Next block in pool
    int in_use;                 // 1 if in use, 0 if free
} MemoryBlock;

/**
 * Memory Pool
 * 
 * Pool of pre-allocated memory blocks.
 */
typedef struct {
    MemoryPoolType type;        // Pool type
    void* base;                 // Base address of pool
    size_t total_size;          // Total pool size
    size_t used_size;           // Currently used size
    size_t block_size;          // Size of each block
    uint32_t num_blocks;        // Number of blocks
    MemoryBlock* blocks;        // Array of blocks
    MemoryBlock* free_list;     // Free list head
} MemoryPool;

/**
 * Memory Manager
 * 
 * Main memory management structure.
 */
typedef struct {
    // Pools
    MemoryPool pools[MEMORY_MAX_POOLS];
    uint32_t num_pools;
    
    // Statistics
    size_t total_allocated;     // Total bytes allocated
    size_t total_freed;         // Total bytes freed
    size_t peak_usage;          // Peak memory usage
    uint32_t num_allocations;   // Number of allocations
    uint32_t num_frees;         // Number of frees
    uint32_t cache_hits;        // Cache-aligned allocations
    uint32_t cache_misses;      // Non-aligned allocations
    
    // Configuration
    int enable_pooling;         // Enable memory pooling
    int enable_alignment;       // Enable cache alignment
    int enable_statistics;      // Enable statistics tracking
    
} MemoryManager;

/**
 * Memory Statistics
 * 
 * Detailed memory usage statistics.
 */
typedef struct {
    size_t total_allocated;
    size_t total_freed;
    size_t current_usage;
    size_t peak_usage;
    uint32_t num_allocations;
    uint32_t num_frees;
    uint32_t cache_hits;
    uint32_t cache_misses;
    double cache_hit_rate;
    double fragmentation_ratio;
} MemoryStatistics;

// ============================================================================
// INITIALIZATION
// ============================================================================

/**
 * Create memory manager
 * 
 * @param enable_pooling Enable memory pooling
 * @param enable_alignment Enable cache alignment
 * @return Initialized memory manager, or NULL on error
 */
MemoryManager* memory_create(int enable_pooling, int enable_alignment);

/**
 * Free memory manager
 * 
 * @param manager Memory manager to free
 */
void memory_free(MemoryManager* manager);

// ============================================================================
// POOL MANAGEMENT
// ============================================================================

/**
 * Create memory pool
 * 
 * @param manager Memory manager
 * @param type Pool type
 * @param pool_size Total pool size
 * @param block_size Size of each block
 * @return 0 on success, -1 on error
 */
int memory_create_pool(
    MemoryManager* manager,
    MemoryPoolType type,
    size_t pool_size,
    size_t block_size
);

/**
 * Destroy memory pool
 * 
 * @param manager Memory manager
 * @param pool_index Pool index
 * @return 0 on success, -1 on error
 */
int memory_destroy_pool(MemoryManager* manager, uint32_t pool_index);

// ============================================================================
// ALLOCATION
// ============================================================================

/**
 * Allocate memory
 * 
 * @param manager Memory manager
 * @param size Size to allocate
 * @return Pointer to allocated memory, or NULL on error
 */
void* memory_alloc(MemoryManager* manager, size_t size);

/**
 * Allocate aligned memory
 * 
 * @param manager Memory manager
 * @param size Size to allocate
 * @param alignment Alignment requirement (must be power of 2)
 * @return Pointer to allocated memory, or NULL on error
 */
void* memory_alloc_aligned(
    MemoryManager* manager,
    size_t size,
    size_t alignment
);

/**
 * Allocate cache-aligned memory
 * 
 * Allocates memory aligned to cache line boundaries for optimal performance.
 * 
 * @param manager Memory manager
 * @param size Size to allocate
 * @return Pointer to allocated memory, or NULL on error
 */
void* memory_alloc_cache_aligned(MemoryManager* manager, size_t size);

/**
 * Reallocate memory
 * 
 * @param manager Memory manager
 * @param ptr Pointer to existing allocation
 * @param new_size New size
 * @return Pointer to reallocated memory, or NULL on error
 */
void* memory_realloc(MemoryManager* manager, void* ptr, size_t new_size);

/**
 * Free memory
 * 
 * @param manager Memory manager
 * @param ptr Pointer to free
 */
void memory_dealloc(MemoryManager* manager, void* ptr);

// ============================================================================
// SPHERE-SPECIFIC ALLOCATION
// ============================================================================

/**
 * Allocate sphere array
 * 
 * Allocates an array of sphere structures with optimal cache alignment.
 * 
 * @param manager Memory manager
 * @param num_spheres Number of spheres
 * @param sphere_size Size of each sphere structure
 * @return Pointer to allocated array, or NULL on error
 */
void* memory_alloc_sphere_array(
    MemoryManager* manager,
    uint32_t num_spheres,
    size_t sphere_size
);

/**
 * Allocate neighbor array
 * 
 * Allocates neighbor arrays with cache-aware layout.
 * 
 * @param manager Memory manager
 * @param num_spheres Number of spheres
 * @param neighbors_per_sphere Neighbors per sphere
 * @return Pointer to allocated array, or NULL on error
 */
void* memory_alloc_neighbor_array(
    MemoryManager* manager,
    uint32_t num_spheres,
    uint32_t neighbors_per_sphere
);

// ============================================================================
// CACHE OPTIMIZATION
// ============================================================================

/**
 * Optimize memory layout for cache
 * 
 * Reorganizes memory to improve cache locality.
 * 
 * @param manager Memory manager
 * @param data Data to optimize
 * @param element_size Size of each element
 * @param num_elements Number of elements
 * @return 0 on success, -1 on error
 */
int memory_optimize_cache_layout(
    MemoryManager* manager,
    void* data,
    size_t element_size,
    uint32_t num_elements
);

/**
 * Prefetch memory
 * 
 * Hints to the CPU to prefetch memory into cache.
 * 
 * @param ptr Pointer to prefetch
 * @param size Size to prefetch
 */
void memory_prefetch(const void* ptr, size_t size);

// ============================================================================
// STATISTICS
// ============================================================================

/**
 * Get memory statistics
 * 
 * @param manager Memory manager
 * @param stats Output statistics
 * @return 0 on success, -1 on error
 */
int memory_get_statistics(
    const MemoryManager* manager,
    MemoryStatistics* stats
);

/**
 * Print memory statistics
 * 
 * @param manager Memory manager
 * @param output Output file
 */
void memory_print_statistics(const MemoryManager* manager, FILE* output);

/**
 * Reset statistics
 * 
 * @param manager Memory manager
 */
void memory_reset_statistics(MemoryManager* manager);

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * Check if pointer is cache-aligned
 * 
 * @param ptr Pointer to check
 * @return 1 if aligned, 0 otherwise
 */
int memory_is_cache_aligned(const void* ptr);

/**
 * Get cache line size
 * 
 * @return Cache line size in bytes
 */
size_t memory_get_cache_line_size(void);

/**
 * Validate memory manager
 * 
 * @param manager Memory manager
 * @return 1 if valid, 0 if issues found
 */
int memory_validate(const MemoryManager* manager);

#ifdef __cplusplus
}
#endif

#endif /* ALGORITHMS_MEMORY_MANAGEMENT_H */
