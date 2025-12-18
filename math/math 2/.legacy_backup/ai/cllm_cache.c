/**
 * Cache Optimization Utilities
 * 
 * Provides cache-aligned memory allocation for improved performance
 */

#include <stdlib.h>
#include <stddef.h>
#include "../include/cllm_cache.h"

#ifdef _WIN32
#include <malloc.h>
#endif

/**
 * Allocate cache-aligned memory
 */
void* cache_aligned_alloc(size_t size) {
    if (size == 0) return NULL;
    
#ifdef _WIN32
    // Windows: use _aligned_malloc
    return _aligned_malloc(size, CACHE_LINE_SIZE);
#else
    // POSIX: use posix_memalign
    void* ptr = NULL;
    if (posix_memalign(&ptr, CACHE_LINE_SIZE, size) != 0) {
        return NULL;
    }
    return ptr;
#endif
}

/**
 * Free cache-aligned memory
 */
void cache_aligned_free(void* ptr) {
    if (!ptr) return;
    
#ifdef _WIN32
    _aligned_free(ptr);
#else
    free(ptr);
#endif
}