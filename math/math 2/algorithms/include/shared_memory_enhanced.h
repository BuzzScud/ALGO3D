/**
 * @file shared_memory_enhanced.h
 * @brief Enhanced Shared Memory Features
 * 
 * Provides enhanced features for shared memory including:
 * - Version history tracking
 * - Invalidation callbacks
 * - Cache coherency protocol
 */

#ifndef SHARED_MEMORY_ENHANCED_H
#define SHARED_MEMORY_ENHANCED_H

#include "shared_memory.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Invalidation callback function type
 * 
 * Called when a shared memory region is modified.
 * 
 * @param user_data User-provided data
 * @param region_id Region ID that was modified
 * @param old_version Previous version number
 * @param new_version New version number
 */
typedef void (*SharedMemoryInvalidationCallback)(
    void* user_data,
    uint64_t region_id,
    uint64_t old_version,
    uint64_t new_version
);

/**
 * Version history entry
 */
typedef struct {
    uint64_t version;                // Version number
    uint64_t timestamp;              // Timestamp (nanoseconds since epoch)
    size_t size;                     // Size at this version
} SharedMemoryVersionEntry;

/**
 * Enhanced shared memory region
 * 
 * Extends SharedMemoryRegion with additional features.
 */
typedef struct {
    SharedMemoryRegion base;         // Base region
    
    // Version history
    SharedMemoryVersionEntry* version_history;
    size_t version_history_size;
    size_t version_history_capacity;
    
    // Invalidation callback
    SharedMemoryInvalidationCallback invalidation_cb;
    void* invalidation_user_data;
    
    // Region ID for rainbow table
    uint64_t region_id;
    
    // Additional statistics
    _Atomic(uint64_t) invalidation_count;
    
} SharedMemoryEnhanced;

/**
 * Create enhanced shared memory region
 * 
 * @param size Initial size in bytes
 * @param mode Access mode
 * @param region_id Unique region ID
 * @return Allocated region, or NULL on error
 */
SharedMemoryEnhanced* shared_memory_enhanced_create(size_t size,
                                                    SharedMemoryAccessMode mode,
                                                    uint64_t region_id);

/**
 * Free enhanced shared memory region
 * 
 * @param region Region to free
 */
void shared_memory_enhanced_free(SharedMemoryEnhanced* region);

/**
 * Enable version history tracking
 * 
 * @param region Region to enable history for
 * @param history_capacity Number of versions to track
 * @return 0 on success, -1 on error
 */
int shared_memory_enhanced_enable_history(SharedMemoryEnhanced* region,
                                         size_t history_capacity);

/**
 * Get version history
 * 
 * @param region Region to query
 * @param out_entries Output buffer for version entries
 * @param max_entries Maximum number of entries to return
 * @return Number of entries returned
 */
size_t shared_memory_enhanced_get_history(const SharedMemoryEnhanced* region,
                                         SharedMemoryVersionEntry* out_entries,
                                         size_t max_entries);

/**
 * Set invalidation callback
 * 
 * @param region Region to set callback for
 * @param callback Callback function
 * @param user_data User data to pass to callback
 */
void shared_memory_enhanced_set_callback(SharedMemoryEnhanced* region,
                                        SharedMemoryInvalidationCallback callback,
                                        void* user_data);

/**
 * Trigger invalidation callback
 * 
 * Called internally when region is modified.
 * 
 * @param region Region that was modified
 * @param old_version Previous version
 * @param new_version New version
 */
void shared_memory_enhanced_trigger_invalidation(SharedMemoryEnhanced* region,
                                                uint64_t old_version,
                                                uint64_t new_version);

/**
 * Get region ID
 * 
 * @param region Region to query
 * @return Region ID
 */
uint64_t shared_memory_enhanced_get_id(const SharedMemoryEnhanced* region);

/**
 * Get enhanced statistics
 * 
 * @param region Region to query
 * @param out_reads Output: number of reads
 * @param out_writes Output: number of writes
 * @param out_copies Output: number of COW copies
 * @param out_invalidations Output: number of invalidations
 * @param out_history_size Output: size of version history
 */
void shared_memory_enhanced_get_stats(const SharedMemoryEnhanced* region,
                                     uint64_t* out_reads,
                                     uint64_t* out_writes,
                                     uint64_t* out_copies,
                                     uint64_t* out_invalidations,
                                     size_t* out_history_size);

/**
 * Print enhanced region info (for debugging)
 * 
 * @param region Region to print
 * @param name Optional name for the region
 */
void shared_memory_enhanced_print_info(const SharedMemoryEnhanced* region,
                                      const char* name);

#ifdef __cplusplus
}
#endif

#endif /* SHARED_MEMORY_ENHANCED_H */