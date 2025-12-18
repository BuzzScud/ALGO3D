#ifndef CLLM_CACHE_H
#define CLLM_CACHE_H

#include <stddef.h>

/**
 * Cache Optimization Utilities
 * 
 * Provides cache-aligned memory allocation and prefetching hints
 * for improved cache utilization and reduced memory latency
 */

// Cache line size (typically 64 bytes on modern CPUs)
#define CACHE_LINE_SIZE 64

/**
 * Allocate cache-aligned memory
 * 
 * @param size Number of bytes to allocate
 * @return Pointer to aligned memory, or NULL on failure
 */
void* cache_aligned_alloc(size_t size);

/**
 * Free cache-aligned memory
 * 
 * @param ptr Pointer to free (must be from cache_aligned_alloc)
 */
void cache_aligned_free(void* ptr);

/**
 * Prefetch data into cache (read)
 * 
 * @param addr Address to prefetch
 */
static inline void prefetch_read(const void* addr) {
    __builtin_prefetch(addr, 0, 3);  // 0 = read, 3 = high temporal locality
}

/**
 * Prefetch data into cache (write)
 * 
 * @param addr Address to prefetch
 */
static inline void prefetch_write(void* addr) {
    __builtin_prefetch(addr, 1, 3);  // 1 = write, 3 = high temporal locality
}

/**
 * Prefetch data with low temporal locality
 * Useful for streaming data that won't be reused
 * 
 * @param addr Address to prefetch
 */
static inline void prefetch_stream(const void* addr) {
    __builtin_prefetch(addr, 0, 0);  // 0 = read, 0 = no temporal locality
}

#endif /* CLLM_CACHE_H */