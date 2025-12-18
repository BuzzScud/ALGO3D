/**
 * Shared Memory System - Implementation
 * 
 * Three-tier memory model for efficient sharing between spheres.
 */

#include "ai/cllm_shared_memory.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
 * Default copy function (memcpy)
 */
void* shared_memory_default_copy(const void* src, size_t size) {
    if (!src || size == 0) {
        return NULL;
    }
    
    void* copy = malloc(size);
    if (!copy) {
        return NULL;
    }
    
    memcpy(copy, src, size);
    return copy;
}

/**
 * Default free function
 */
void shared_memory_default_free(void* ptr) {
    free(ptr);
}

/**
 * Create shared memory region
 */
SharedMemoryRegion* shared_memory_create(size_t size, SharedMemoryAccessMode mode) {
    return shared_memory_create_custom(size, mode, 
                                       shared_memory_default_copy,
                                       shared_memory_default_free);
}

/**
 * Create shared memory region with custom functions
 */
SharedMemoryRegion* shared_memory_create_custom(size_t size, 
                                                 SharedMemoryAccessMode mode,
                                                 CopyFunction copy_fn,
                                                 FreeFunction free_fn) {
    if (size == 0) {
        fprintf(stderr, "Error: size must be > 0\n");
        return NULL;
    }
    
    SharedMemoryRegion* region = (SharedMemoryRegion*)calloc(1, sizeof(SharedMemoryRegion));
    if (!region) {
        fprintf(stderr, "Error: Failed to allocate SharedMemoryRegion\n");
        return NULL;
    }
    
    // Allocate data
    region->data = malloc(size);
    if (!region->data) {
        fprintf(stderr, "Error: Failed to allocate data (%zu bytes)\n", size);
        free(region);
        return NULL;
    }
    
    // Initialize data to zero
    memset(region->data, 0, size);
    
    region->size = size;
    region->capacity = size;
    region->access_mode = mode;
    
    // Initialize rwlock
    if (pthread_rwlock_init(&region->rwlock, NULL) != 0) {
        fprintf(stderr, "Error: Failed to initialize rwlock\n");
        free(region->data);
        free(region);
        return NULL;
    }
    
    // Initialize atomics
    atomic_init(&region->num_readers, 0);
    atomic_init(&region->num_writers, 0);
    atomic_init(&region->version, 0);
    atomic_init(&region->read_count, 0);
    atomic_init(&region->write_count, 0);
    atomic_init(&region->copy_count, 0);
    
    // Set copy/free functions
    region->copy_fn = copy_fn ? copy_fn : shared_memory_default_copy;
    region->free_fn = free_fn ? free_fn : shared_memory_default_free;
    
    region->is_copy = false;
    region->original = NULL;
    
    return region;
}

/**
 * Free shared memory region
 */
void shared_memory_free(SharedMemoryRegion* region) {
    if (!region) {
        return;
    }
    
    // Destroy rwlock
    pthread_rwlock_destroy(&region->rwlock);
    
    // Free data
    if (region->data) {
        region->free_fn(region->data);
    }
    
    free(region);
}

/**
 * Acquire read access
 */
const void* shared_memory_read(SharedMemoryRegion* region) {
    if (!region) {
        return NULL;
    }
    
    // Increment read count
    atomic_fetch_add(&region->read_count, 1);
    
    switch (region->access_mode) {
        case SHARED_READ_ONLY:
            // No lock needed for read-only
            atomic_fetch_add(&region->num_readers, 1);
            return region->data;
            
        case SHARED_COPY_ON_WRITE:
        case SHARED_LOCKED_WRITE:
            // Acquire read lock
            if (pthread_rwlock_rdlock(&region->rwlock) != 0) {
                fprintf(stderr, "Error: Failed to acquire read lock\n");
                return NULL;
            }
            atomic_fetch_add(&region->num_readers, 1);
            return region->data;
    }
    
    return NULL;
}

/**
 * Release read access
 */
void shared_memory_release_read(SharedMemoryRegion* region) {
    if (!region) {
        return;
    }
    
    atomic_fetch_sub(&region->num_readers, 1);
    
    switch (region->access_mode) {
        case SHARED_READ_ONLY:
            // No lock to release
            break;
            
        case SHARED_COPY_ON_WRITE:
        case SHARED_LOCKED_WRITE:
            pthread_rwlock_unlock(&region->rwlock);
            break;
    }
}

/**
 * Acquire write access
 */
void* shared_memory_write(SharedMemoryRegion* region) {
    if (!region) {
        return NULL;
    }
    
    // Increment write count
    atomic_fetch_add(&region->write_count, 1);
    
    switch (region->access_mode) {
        case SHARED_READ_ONLY:
            // Cannot write to read-only memory
            fprintf(stderr, "Error: Cannot write to READ_ONLY memory\n");
            return NULL;
            
        case SHARED_COPY_ON_WRITE:
            // Create copy if this is the first write
            if (!region->is_copy) {
                void* copy = region->copy_fn(region->data, region->size);
                if (!copy) {
                    fprintf(stderr, "Error: Failed to create COW copy\n");
                    return NULL;
                }
                
                // Replace data with copy
                region->original = (SharedMemoryRegion*)region->data; // Store original pointer
                region->data = copy;
                region->is_copy = true;
                atomic_fetch_add(&region->copy_count, 1);
            }
            
            // Acquire write lock
            if (pthread_rwlock_wrlock(&region->rwlock) != 0) {
                fprintf(stderr, "Error: Failed to acquire write lock\n");
                return NULL;
            }
            
            atomic_fetch_add(&region->num_writers, 1);
            return region->data;
            
        case SHARED_LOCKED_WRITE:
            // Acquire write lock
            if (pthread_rwlock_wrlock(&region->rwlock) != 0) {
                fprintf(stderr, "Error: Failed to acquire write lock\n");
                return NULL;
            }
            
            atomic_fetch_add(&region->num_writers, 1);
            return region->data;
    }
    
    return NULL;
}

/**
 * Release write access
 */
void shared_memory_release_write(SharedMemoryRegion* region) {
    if (!region) {
        return;
    }
    
    atomic_fetch_sub(&region->num_writers, 1);
    
    // Increment version on write
    atomic_fetch_add(&region->version, 1);
    
    switch (region->access_mode) {
        case SHARED_READ_ONLY:
            // Should never get here
            break;
            
        case SHARED_COPY_ON_WRITE:
        case SHARED_LOCKED_WRITE:
            pthread_rwlock_unlock(&region->rwlock);
            break;
    }
}

/**
 * Get current version
 */
uint64_t shared_memory_get_version(const SharedMemoryRegion* region) {
    if (!region) {
        return 0;
    }
    
    return atomic_load(&region->version);
}

/**
 * Check if modified since version
 */
bool shared_memory_is_modified(const SharedMemoryRegion* region, uint64_t version) {
    if (!region) {
        return false;
    }
    
    return atomic_load(&region->version) != version;
}

/**
 * Resize shared memory region
 */
int shared_memory_resize(SharedMemoryRegion* region, size_t new_size) {
    if (!region) {
        return -1;
    }
    
    if (region->access_mode != SHARED_LOCKED_WRITE) {
        fprintf(stderr, "Error: Can only resize LOCKED_WRITE regions\n");
        return -1;
    }
    
    if (new_size == 0) {
        fprintf(stderr, "Error: new_size must be > 0\n");
        return -1;
    }
    
    // Reallocate data
    void* new_data = realloc(region->data, new_size);
    if (!new_data) {
        fprintf(stderr, "Error: Failed to reallocate to %zu bytes\n", new_size);
        return -1;
    }
    
    // If growing, zero new memory
    if (new_size > region->size) {
        memset((char*)new_data + region->size, 0, new_size - region->size);
    }
    
    region->data = new_data;
    region->size = new_size;
    region->capacity = new_size;
    
    // Increment version
    atomic_fetch_add(&region->version, 1);
    
    return 0;
}

/**
 * Get statistics
 */
void shared_memory_get_stats(const SharedMemoryRegion* region,
                             uint64_t* out_reads,
                             uint64_t* out_writes,
                             uint64_t* out_copies) {
    if (!region) {
        return;
    }
    
    if (out_reads) {
        *out_reads = atomic_load(&region->read_count);
    }
    
    if (out_writes) {
        *out_writes = atomic_load(&region->write_count);
    }
    
    if (out_copies) {
        *out_copies = atomic_load(&region->copy_count);
    }
}

/**
 * Print info
 */
void shared_memory_print_info(const SharedMemoryRegion* region, const char* name) {
    if (!region) {
        return;
    }
    
    printf("=== Shared Memory Region");
    if (name) {
        printf(": %s", name);
    }
    printf(" ===\n");
    
    printf("Size: %zu bytes\n", region->size);
    printf("Capacity: %zu bytes\n", region->capacity);
    
    printf("Access mode: ");
    switch (region->access_mode) {
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
    
    printf("Version: %lu\n", atomic_load(&region->version));
    printf("Active readers: %d\n", atomic_load(&region->num_readers));
    printf("Active writers: %d\n", atomic_load(&region->num_writers));
    
    printf("Statistics:\n");
    printf("  Reads: %lu\n", atomic_load(&region->read_count));
    printf("  Writes: %lu\n", atomic_load(&region->write_count));
    printf("  COW copies: %lu\n", atomic_load(&region->copy_count));
    
    if (region->is_copy) {
        printf("Is COW copy: YES\n");
    }
    
    printf("\n");
}

/**
 * Create read-only view
 */
SharedMemoryRegion* shared_memory_create_readonly_view(const void* data, size_t size) {
    if (!data || size == 0) {
        return NULL;
    }
    
    SharedMemoryRegion* region = (SharedMemoryRegion*)calloc(1, sizeof(SharedMemoryRegion));
    if (!region) {
        return NULL;
    }
    
    // Point to existing data (no copy)
    region->data = (void*)data;
    region->size = size;
    region->capacity = size;
    region->access_mode = SHARED_READ_ONLY;
    
    // Initialize rwlock (even though we won't use it for READ_ONLY)
    pthread_rwlock_init(&region->rwlock, NULL);
    
    // Initialize atomics
    atomic_init(&region->num_readers, 0);
    atomic_init(&region->num_writers, 0);
    atomic_init(&region->version, 0);
    atomic_init(&region->read_count, 0);
    atomic_init(&region->write_count, 0);
    atomic_init(&region->copy_count, 0);
    
    region->copy_fn = NULL;
    region->free_fn = NULL; // Don't free data we don't own
    region->is_copy = false;
    region->original = NULL;
    
    return region;
}

/**
 * Validate shared memory region
 */
bool shared_memory_validate(const SharedMemoryRegion* region) {
    if (!region) {
        fprintf(stderr, "Validation failed: NULL region\n");
        return false;
    }
    
    if (!region->data) {
        fprintf(stderr, "Validation failed: NULL data\n");
        return false;
    }
    
    if (region->size == 0) {
        fprintf(stderr, "Validation failed: size is 0\n");
        return false;
    }
    
    if (region->size > region->capacity) {
        fprintf(stderr, "Validation failed: size > capacity\n");
        return false;
    }
    
    int readers = atomic_load(&region->num_readers);
    int writers = atomic_load(&region->num_writers);
    
    if (readers < 0) {
        fprintf(stderr, "Validation failed: negative readers\n");
        return false;
    }
    
    if (writers < 0) {
        fprintf(stderr, "Validation failed: negative writers\n");
        return false;
    }
    
    if (writers > 1) {
        fprintf(stderr, "Validation failed: multiple writers\n");
        return false;
    }
    
    if (writers > 0 && readers > 0) {
        fprintf(stderr, "Validation failed: simultaneous readers and writers\n");
        return false;
    }
    
    return true;
}