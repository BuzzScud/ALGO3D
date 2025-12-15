/**
 * Generic Memory Management Implementation
 * 
 * Provides efficient memory allocation with cache optimization.
 */

#include "memory_management.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

// Platform-specific prefetch
#ifdef __GNUC__
#define PREFETCH(ptr) __builtin_prefetch(ptr)
#else
#define PREFETCH(ptr) ((void)0)
#endif

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * Align size to cache line
 */
static size_t align_to_cache_line(size_t size) {
    return ((size + MEMORY_CACHE_LINE_SIZE - 1) / MEMORY_CACHE_LINE_SIZE) * MEMORY_CACHE_LINE_SIZE;
}

/**
 * Check if size is power of 2
 */
static int is_power_of_2(size_t x) {
    return (x != 0) && ((x & (x - 1)) == 0);
}

/**
 * Align pointer to boundary
 */
__attribute__((unused))
static void* align_pointer(void* ptr, size_t alignment) {
    uintptr_t addr = (uintptr_t)ptr;
    uintptr_t aligned = (addr + alignment - 1) & ~(alignment - 1);
    return (void*)aligned;
}

// ============================================================================
// INITIALIZATION
// ============================================================================

MemoryManager* memory_create(int enable_pooling, int enable_alignment) {
    MemoryManager* manager = (MemoryManager*)calloc(1, sizeof(MemoryManager));
    if (!manager) {
        return NULL;
    }
    
    manager->enable_pooling = enable_pooling;
    manager->enable_alignment = enable_alignment;
    manager->enable_statistics = 1;
    manager->num_pools = 0;
    
    return manager;
}

void memory_free(MemoryManager* manager) {
    if (!manager) {
        return;
    }
    
    // Free all pools
    for (uint32_t i = 0; i < manager->num_pools; i++) {
        MemoryPool* pool = &manager->pools[i];
        if (pool->base) {
            free(pool->base);
        }
        if (pool->blocks) {
            free(pool->blocks);
        }
    }
    
    free(manager);
}

// ============================================================================
// POOL MANAGEMENT
// ============================================================================

int memory_create_pool(
    MemoryManager* manager,
    MemoryPoolType type,
    size_t pool_size,
    size_t block_size
) {
    if (!manager || manager->num_pools >= MEMORY_MAX_POOLS) {
        return -1;
    }
    
    MemoryPool* pool = &manager->pools[manager->num_pools];
    
    // Align pool size to cache line
    pool_size = align_to_cache_line(pool_size);
    
    // Allocate pool memory
    pool->base = aligned_alloc(MEMORY_CACHE_LINE_SIZE, pool_size);
    if (!pool->base) {
        return -1;
    }
    
    pool->type = type;
    pool->total_size = pool_size;
    pool->used_size = 0;
    pool->block_size = block_size;
    pool->num_blocks = pool_size / block_size;
    
    // Allocate block metadata
    pool->blocks = (MemoryBlock*)calloc(pool->num_blocks, sizeof(MemoryBlock));
    if (!pool->blocks) {
        free(pool->base);
        return -1;
    }
    
    // Initialize blocks
    for (uint32_t i = 0; i < pool->num_blocks; i++) {
        pool->blocks[i].data = (char*)pool->base + (i * block_size);
        pool->blocks[i].size = block_size;
        pool->blocks[i].alignment = MEMORY_CACHE_LINE_SIZE;
        pool->blocks[i].in_use = 0;
        pool->blocks[i].next = (i < pool->num_blocks - 1) ? &pool->blocks[i + 1] : NULL;
    }
    
    pool->free_list = &pool->blocks[0];
    
    manager->num_pools++;
    
    return 0;
}

int memory_destroy_pool(MemoryManager* manager, uint32_t pool_index) {
    if (!manager || pool_index >= manager->num_pools) {
        return -1;
    }
    
    MemoryPool* pool = &manager->pools[pool_index];
    
    if (pool->base) {
        free(pool->base);
        pool->base = NULL;
    }
    
    if (pool->blocks) {
        free(pool->blocks);
        pool->blocks = NULL;
    }
    
    return 0;
}

// ============================================================================
// ALLOCATION
// ============================================================================

void* memory_alloc(MemoryManager* manager, size_t size) {
    if (!manager || size == 0) {
        return NULL;
    }
    
    void* ptr = NULL;
    
    // Try to allocate from pool if enabled
    if (manager->enable_pooling) {
        for (uint32_t i = 0; i < manager->num_pools; i++) {
            MemoryPool* pool = &manager->pools[i];
            
            if (pool->free_list && size <= pool->block_size) {
                MemoryBlock* block = pool->free_list;
                pool->free_list = block->next;
                block->in_use = 1;
                block->next = NULL;
                
                pool->used_size += block->size;
                ptr = block->data;
                break;
            }
        }
    }
    
    // Fall back to system allocation
    if (!ptr) {
        if (manager->enable_alignment) {
            ptr = aligned_alloc(MEMORY_CACHE_LINE_SIZE, align_to_cache_line(size));
            if (ptr && manager->enable_statistics) {
                manager->cache_hits++;
            }
        } else {
            ptr = malloc(size);
            if (ptr && manager->enable_statistics) {
                manager->cache_misses++;
            }
        }
    }
    
    // Update statistics
    if (ptr && manager->enable_statistics) {
        manager->total_allocated += size;
        manager->num_allocations++;
        
        size_t current_usage = manager->total_allocated - manager->total_freed;
        if (current_usage > manager->peak_usage) {
            manager->peak_usage = current_usage;
        }
    }
    
    return ptr;
}

void* memory_alloc_aligned(
    MemoryManager* manager,
    size_t size,
    size_t alignment
) {
    if (!manager || size == 0 || !is_power_of_2(alignment)) {
        return NULL;
    }
    
    void* ptr = aligned_alloc(alignment, ((size + alignment - 1) / alignment) * alignment);
    
    // Update statistics
    if (ptr && manager->enable_statistics) {
        manager->total_allocated += size;
        manager->num_allocations++;
        
        if (alignment == MEMORY_CACHE_LINE_SIZE) {
            manager->cache_hits++;
        } else {
            manager->cache_misses++;
        }
        
        size_t current_usage = manager->total_allocated - manager->total_freed;
        if (current_usage > manager->peak_usage) {
            manager->peak_usage = current_usage;
        }
    }
    
    return ptr;
}

void* memory_alloc_cache_aligned(MemoryManager* manager, size_t size) {
    return memory_alloc_aligned(manager, size, MEMORY_CACHE_LINE_SIZE);
}

void* memory_realloc(MemoryManager* manager, void* ptr, size_t new_size) {
    if (!manager) {
        return NULL;
    }
    
    if (!ptr) {
        return memory_alloc(manager, new_size);
    }
    
    if (new_size == 0) {
        memory_dealloc(manager, ptr);
        return NULL;
    }
    
    void* new_ptr = realloc(ptr, new_size);
    
    // Update statistics (approximate)
    if (new_ptr && manager->enable_statistics) {
        manager->num_allocations++;
    }
    
    return new_ptr;
}

void memory_dealloc(MemoryManager* manager, void* ptr) {
    if (!manager || !ptr) {
        return;
    }
    
    // Check if pointer is from a pool
    int from_pool = 0;
    
    if (manager->enable_pooling) {
        for (uint32_t i = 0; i < manager->num_pools; i++) {
            MemoryPool* pool = &manager->pools[i];
            
            if (ptr >= pool->base && (char*)ptr < (char*)pool->base + pool->total_size) {
                // Find the block
                for (uint32_t j = 0; j < pool->num_blocks; j++) {
                    if (pool->blocks[j].data == ptr && pool->blocks[j].in_use) {
                        pool->blocks[j].in_use = 0;
                        pool->blocks[j].next = pool->free_list;
                        pool->free_list = &pool->blocks[j];
                        pool->used_size -= pool->blocks[j].size;
                        from_pool = 1;
                        break;
                    }
                }
                break;
            }
        }
    }
    
    // Free from system if not from pool
    if (!from_pool) {
        free(ptr);
    }
    
    // Update statistics
    if (manager->enable_statistics) {
        manager->num_frees++;
    }
}

// ============================================================================
// SPHERE-SPECIFIC ALLOCATION
// ============================================================================

void* memory_alloc_sphere_array(
    MemoryManager* manager,
    uint32_t num_spheres,
    size_t sphere_size
) {
    if (!manager || num_spheres == 0 || sphere_size == 0) {
        return NULL;
    }
    
    // Align sphere size to cache line
    size_t aligned_size = align_to_cache_line(sphere_size);
    size_t total_size = aligned_size * num_spheres;
    
    return memory_alloc_cache_aligned(manager, total_size);
}

void* memory_alloc_neighbor_array(
    MemoryManager* manager,
    uint32_t num_spheres,
    uint32_t neighbors_per_sphere
) {
    if (!manager || num_spheres == 0 || neighbors_per_sphere == 0) {
        return NULL;
    }
    
    size_t total_size = num_spheres * neighbors_per_sphere * sizeof(uint32_t);
    
    return memory_alloc_cache_aligned(manager, total_size);
}

// ============================================================================
// CACHE OPTIMIZATION
// ============================================================================

int memory_optimize_cache_layout(
    MemoryManager* manager,
    void* data,
    size_t element_size,
    uint32_t num_elements
) {
    if (!manager || !data || element_size == 0 || num_elements == 0) {
        return -1;
    }
    
    // Allocate temporary buffer
    size_t aligned_size = align_to_cache_line(element_size);
    void* temp = memory_alloc_cache_aligned(manager, aligned_size * num_elements);
    if (!temp) {
        return -1;
    }
    
    // Copy data with cache-aligned layout
    for (uint32_t i = 0; i < num_elements; i++) {
        memcpy((char*)temp + (i * aligned_size),
               (char*)data + (i * element_size),
               element_size);
    }
    
    // Copy back
    memcpy(data, temp, element_size * num_elements);
    
    memory_dealloc(manager, temp);
    
    return 0;
}

void memory_prefetch(const void* ptr, size_t size) {
    if (!ptr || size == 0) {
        return;
    }
    
    // Prefetch cache lines
    const char* p = (const char*)ptr;
    const char* end = p + size;
    
    while (p < end) {
        PREFETCH(p);
        p += MEMORY_CACHE_LINE_SIZE;
    }
}

// ============================================================================
// STATISTICS
// ============================================================================

int memory_get_statistics(
    const MemoryManager* manager,
    MemoryStatistics* stats
) {
    if (!manager || !stats) {
        return -1;
    }
    
    stats->total_allocated = manager->total_allocated;
    stats->total_freed = manager->total_freed;
    stats->current_usage = manager->total_allocated - manager->total_freed;
    stats->peak_usage = manager->peak_usage;
    stats->num_allocations = manager->num_allocations;
    stats->num_frees = manager->num_frees;
    stats->cache_hits = manager->cache_hits;
    stats->cache_misses = manager->cache_misses;
    
    uint32_t total_cache_ops = manager->cache_hits + manager->cache_misses;
    stats->cache_hit_rate = total_cache_ops > 0 ? 
        (double)manager->cache_hits / total_cache_ops : 0.0;
    
    // Calculate fragmentation (simplified)
    size_t total_pool_size = 0;
    size_t used_pool_size = 0;
    
    for (uint32_t i = 0; i < manager->num_pools; i++) {
        total_pool_size += manager->pools[i].total_size;
        used_pool_size += manager->pools[i].used_size;
    }
    
    stats->fragmentation_ratio = total_pool_size > 0 ?
        1.0 - ((double)used_pool_size / total_pool_size) : 0.0;
    
    return 0;
}

void memory_print_statistics(const MemoryManager* manager, FILE* output) {
    if (!manager || !output) {
        return;
    }
    
    MemoryStatistics stats;
    if (memory_get_statistics(manager, &stats) != 0) {
        return;
    }
    
    fprintf(output, "\n");
    fprintf(output, "========================================\n");
    fprintf(output, "  Memory Management Statistics\n");
    fprintf(output, "========================================\n");
    fprintf(output, "  Total Allocated:  %zu bytes\n", stats.total_allocated);
    fprintf(output, "  Total Freed:      %zu bytes\n", stats.total_freed);
    fprintf(output, "  Current Usage:    %zu bytes\n", stats.current_usage);
    fprintf(output, "  Peak Usage:       %zu bytes\n", stats.peak_usage);
    fprintf(output, "  Allocations:      %u\n", stats.num_allocations);
    fprintf(output, "  Frees:            %u\n", stats.num_frees);
    fprintf(output, "  Cache Hits:       %u\n", stats.cache_hits);
    fprintf(output, "  Cache Misses:     %u\n", stats.cache_misses);
    fprintf(output, "  Cache Hit Rate:   %.2f%%\n", stats.cache_hit_rate * 100.0);
    fprintf(output, "  Fragmentation:    %.2f%%\n", stats.fragmentation_ratio * 100.0);
    fprintf(output, "========================================\n\n");
}

void memory_reset_statistics(MemoryManager* manager) {
    if (!manager) {
        return;
    }
    
    manager->total_allocated = 0;
    manager->total_freed = 0;
    manager->peak_usage = 0;
    manager->num_allocations = 0;
    manager->num_frees = 0;
    manager->cache_hits = 0;
    manager->cache_misses = 0;
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

int memory_is_cache_aligned(const void* ptr) {
    return ((uintptr_t)ptr % MEMORY_CACHE_LINE_SIZE) == 0;
}

size_t memory_get_cache_line_size(void) {
    return MEMORY_CACHE_LINE_SIZE;
}

int memory_validate(const MemoryManager* manager) {
    if (!manager) {
        return 0;
    }
    
    // Check pools
    for (uint32_t i = 0; i < manager->num_pools; i++) {
        const MemoryPool* pool = &manager->pools[i];
        
        if (!pool->base || !pool->blocks) {
            return 0;
        }
        
        if (pool->used_size > pool->total_size) {
            return 0;
        }
    }
    
    return 1;
}
