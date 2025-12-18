/**
 * @file shared_memory_enhanced.c
 * @brief Enhanced Shared Memory Features - Implementation
 */

#include "shared_memory_enhanced.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

// Helper to get current time in nanoseconds
static uint64_t get_time_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

/**
 * Create enhanced shared memory region
 */
SharedMemoryEnhanced* shared_memory_enhanced_create(size_t size,
                                                    SharedMemoryAccessMode mode,
                                                    uint64_t region_id) {
    if (size == 0) {
        fprintf(stderr, "Error: size must be > 0\n");
        return NULL;
    }
    
    SharedMemoryEnhanced* enhanced = (SharedMemoryEnhanced*)calloc(1, sizeof(SharedMemoryEnhanced));
    if (!enhanced) {
        fprintf(stderr, "Error: failed to allocate enhanced region\n");
        return NULL;
    }
    
    // Create base region using existing API
    SharedMemoryRegion* base = shared_memory_create(size, mode);
    if (!base) {
        fprintf(stderr, "Error: failed to create base region\n");
        free(enhanced);
        return NULL;
    }
    
    // Copy base region data
    memcpy(&enhanced->base, base, sizeof(SharedMemoryRegion));
    free(base); // Free the wrapper, we copied the data
    
    // Initialize enhanced fields
    enhanced->version_history = NULL;
    enhanced->version_history_size = 0;
    enhanced->version_history_capacity = 0;
    enhanced->invalidation_cb = NULL;
    enhanced->invalidation_user_data = NULL;
    enhanced->region_id = region_id;
    atomic_init(&enhanced->invalidation_count, 0);
    
    return enhanced;
}

/**
 * Free enhanced shared memory region
 */
void shared_memory_enhanced_free(SharedMemoryEnhanced* region) {
    if (!region) {
        return;
    }
    
    // Free version history
    if (region->version_history) {
        free(region->version_history);
    }
    
    // Free base region data
    pthread_rwlock_destroy(&region->base.rwlock);
    if (region->base.data) {
        region->base.free_fn(region->base.data);
    }
    
    free(region);
}

/**
 * Enable version history tracking
 */
int shared_memory_enhanced_enable_history(SharedMemoryEnhanced* region,
                                         size_t history_capacity) {
    if (!region || history_capacity == 0) {
        return -1;
    }
    
    // Allocate history buffer
    region->version_history = (SharedMemoryVersionEntry*)calloc(
        history_capacity,
        sizeof(SharedMemoryVersionEntry)
    );
    
    if (!region->version_history) {
        fprintf(stderr, "Error: failed to allocate version history\n");
        return -1;
    }
    
    region->version_history_capacity = history_capacity;
    region->version_history_size = 0;
    
    // Add initial version
    region->version_history[0].version = atomic_load(&region->base.version);
    region->version_history[0].timestamp = get_time_ns();
    region->version_history[0].size = region->base.size;
    region->version_history_size = 1;
    
    return 0;
}

/**
 * Get version history
 */
size_t shared_memory_enhanced_get_history(const SharedMemoryEnhanced* region,
                                         SharedMemoryVersionEntry* out_entries,
                                         size_t max_entries) {
    if (!region || !out_entries || max_entries == 0) {
        return 0;
    }
    
    if (!region->version_history) {
        return 0;
    }
    
    size_t count = (region->version_history_size < max_entries) ? 
                   region->version_history_size : max_entries;
    
    memcpy(out_entries, region->version_history, 
           count * sizeof(SharedMemoryVersionEntry));
    
    return count;
}

/**
 * Set invalidation callback
 */
void shared_memory_enhanced_set_callback(SharedMemoryEnhanced* region,
                                        SharedMemoryInvalidationCallback callback,
                                        void* user_data) {
    if (!region) {
        return;
    }
    
    region->invalidation_cb = callback;
    region->invalidation_user_data = user_data;
}

/**
 * Trigger invalidation callback
 */
void shared_memory_enhanced_trigger_invalidation(SharedMemoryEnhanced* region,
                                                uint64_t old_version,
                                                uint64_t new_version) {
    if (!region) {
        return;
    }
    
    // Increment invalidation count
    atomic_fetch_add(&region->invalidation_count, 1);
    
    // Add to version history if enabled
    if (region->version_history) {
        size_t idx = region->version_history_size % region->version_history_capacity;
        region->version_history[idx].version = new_version;
        region->version_history[idx].timestamp = get_time_ns();
        region->version_history[idx].size = region->base.size;
        
        if (region->version_history_size < region->version_history_capacity) {
            region->version_history_size++;
        }
    }
    
    // Call invalidation callback if set
    if (region->invalidation_cb) {
        region->invalidation_cb(region->invalidation_user_data,
                               region->region_id,
                               old_version,
                               new_version);
    }
}

/**
 * Get region ID
 */
uint64_t shared_memory_enhanced_get_id(const SharedMemoryEnhanced* region) {
    if (!region) {
        return 0;
    }
    
    return region->region_id;
}

/**
 * Get enhanced statistics
 */
void shared_memory_enhanced_get_stats(const SharedMemoryEnhanced* region,
                                     uint64_t* out_reads,
                                     uint64_t* out_writes,
                                     uint64_t* out_copies,
                                     uint64_t* out_invalidations,
                                     size_t* out_history_size) {
    if (!region) {
        return;
    }
    
    if (out_reads) {
        *out_reads = atomic_load(&region->base.read_count);
    }
    
    if (out_writes) {
        *out_writes = atomic_load(&region->base.write_count);
    }
    
    if (out_copies) {
        *out_copies = atomic_load(&region->base.copy_count);
    }
    
    if (out_invalidations) {
        *out_invalidations = atomic_load(&region->invalidation_count);
    }
    
    if (out_history_size) {
        *out_history_size = region->version_history_size;
    }
}

/**
 * Print enhanced region info
 */
void shared_memory_enhanced_print_info(const SharedMemoryEnhanced* region,
                                      const char* name) {
    if (!region) {
        return;
    }
    
    printf("=== Enhanced Shared Memory Region");
    if (name) {
        printf(": %s", name);
    }
    printf(" ===\n");
    
    printf("Region ID: %lu\n", region->region_id);
    printf("Size: %zu bytes\n", region->base.size);
    printf("Capacity: %zu bytes\n", region->base.capacity);
    
    printf("Access mode: ");
    switch (region->base.access_mode) {
        case SHARED_READ_ONLY:
            printf("READ_ONLY\n");
            break;
        case SHARED_COPY_ON_WRITE:
            printf("COPY_ON_WRITE\n");
            break;
        case SHARED_LOCKED_WRITE:
            printf("LOCKED_WRITE\n");
            break;
    }
    
    printf("Version: %lu\n", atomic_load(&region->base.version));
    printf("Active readers: %d\n", atomic_load(&region->base.num_readers));
    printf("Active writers: %d\n", atomic_load(&region->base.num_writers));
    
    printf("Statistics:\n");
    printf("  Reads: %lu\n", atomic_load(&region->base.read_count));
    printf("  Writes: %lu\n", atomic_load(&region->base.write_count));
    printf("  COW copies: %lu\n", atomic_load(&region->base.copy_count));
    printf("  Invalidations: %lu\n", atomic_load(&region->invalidation_count));
    
    if (region->version_history) {
        printf("  Version history: %zu entries (capacity: %zu)\n",
               region->version_history_size,
               region->version_history_capacity);
    }
    
    if (region->base.is_copy) {
        printf("Is COW copy: YES\n");
    }
    
    printf("\n");
}