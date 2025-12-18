#ifndef CLLM_SHARED_MEMORY_H
#define CLLM_SHARED_MEMORY_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <pthread.h>
#include <stdatomic.h>

// Use algorithms library types
#include "../../algorithms/include/shared_memory.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Shared Memory System for Kissing Spheres Architecture
 * 
 * NOTE: This now uses SharedMemoryRegion and SharedMemoryAccessMode 
 * from algorithms/shared_memory.h
 * 
 * The algorithms library provides:
 * - SharedMemoryRegion struct
 * - SharedMemoryAccessMode enum (SHARED_READ_ONLY, SHARED_COPY_ON_WRITE, SHARED_LOCKED_WRITE)
 * - All shared memory functions (create, free, read, write, etc.)
 * 
 * CLLM-specific extensions are defined below.
 */

// ============================================================================
// CLLM-SPECIFIC EXTENSIONS
// ============================================================================

/**
 * CLLM Shared Memory Metadata
 * 
 * Additional metadata for CLLM-specific shared memory regions.
 * Wraps the base SharedMemoryRegion from algorithms library.
 */
typedef struct {
    SharedMemoryRegion* region;       // Base region from algorithms library
    
    // CLLM-specific metadata
    int sphere_id;                    // Owner sphere ID
    int symmetry_group;               // Symmetry group
    uint64_t last_update_epoch;       // Last update epoch
    
    // CLLM-specific statistics
    _Atomic(uint64_t) gradient_updates;  // Number of gradient updates
    _Atomic(uint64_t) weight_updates;    // Number of weight updates
    
} CLLMSharedMemory;

// ============================================================================
// CLLM-SPECIFIC FUNCTIONS
// ============================================================================

/**
 * Create CLLM shared memory region
 * 
 * @param size Initial size in bytes
 * @param mode Access mode
 * @param sphere_id Owner sphere ID
 * @param symmetry_group Symmetry group
 * @return CLLM shared memory, or NULL on error
 */
CLLMSharedMemory* cllm_shared_memory_create(
    size_t size,
    SharedMemoryAccessMode mode,
    int sphere_id,
    int symmetry_group
);

/**
 * Free CLLM shared memory region
 * 
 * @param cllm_mem CLLM shared memory to free
 */
void cllm_shared_memory_free(CLLMSharedMemory* cllm_mem);

/**
 * Get base region from CLLM shared memory
 * 
 * @param cllm_mem CLLM shared memory
 * @return Base SharedMemoryRegion
 */
SharedMemoryRegion* cllm_shared_memory_get_region(CLLMSharedMemory* cllm_mem);

/**
 * Update gradient in shared memory
 * 
 * @param cllm_mem CLLM shared memory
 * @param gradient Gradient data
 * @param size Size of gradient
 * @return 0 on success, -1 on error
 */
int cllm_shared_memory_update_gradient(
    CLLMSharedMemory* cllm_mem,
    const float* gradient,
    size_t size
);

/**
 * Update weights in shared memory
 * 
 * @param cllm_mem CLLM shared memory
 * @param weights Weight data
 * @param size Size of weights
 * @return 0 on success, -1 on error
 */
int cllm_shared_memory_update_weights(
    CLLMSharedMemory* cllm_mem,
    const float* weights,
    size_t size
);

/**
 * Get CLLM shared memory statistics
 * 
 * @param cllm_mem CLLM shared memory
 * @param gradient_updates Output: number of gradient updates
 * @param weight_updates Output: number of weight updates
 */
void cllm_shared_memory_get_stats(
    const CLLMSharedMemory* cllm_mem,
    uint64_t* gradient_updates,
    uint64_t* weight_updates
);

// ============================================================================
// CONVENIENCE WRAPPERS
// ============================================================================

/**
 * Read from CLLM shared memory
 * 
 * Convenience wrapper around shared_memory_read()
 */
static inline const void* cllm_shared_memory_read(CLLMSharedMemory* cllm_mem) {
    return cllm_mem ? shared_memory_read(cllm_mem->region) : NULL;
}

/**
 * Write to CLLM shared memory
 * 
 * Convenience wrapper around shared_memory_write()
 */
static inline void* cllm_shared_memory_write(CLLMSharedMemory* cllm_mem) {
    return cllm_mem ? shared_memory_write(cllm_mem->region) : NULL;
}

/**
 * Release read access
 * 
 * Convenience wrapper around shared_memory_release_read()
 */
static inline void cllm_shared_memory_release_read(CLLMSharedMemory* cllm_mem) {
    if (cllm_mem) shared_memory_release_read(cllm_mem->region);
}

/**
 * Release write access
 * 
 * Convenience wrapper around shared_memory_release_write()
 */
static inline void cllm_shared_memory_release_write(CLLMSharedMemory* cllm_mem) {
    if (cllm_mem) shared_memory_release_write(cllm_mem->region);
}

#ifdef __cplusplus
}
#endif

#endif // CLLM_SHARED_MEMORY_H