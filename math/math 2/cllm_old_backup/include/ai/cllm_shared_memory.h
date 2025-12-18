#ifndef CLLM_SHARED_MEMORY_H
#define CLLM_SHARED_MEMORY_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <pthread.h>
#include <stdatomic.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Shared Memory System for Kissing Spheres Architecture
 * 
 * Implements a three-tier memory model:
 * 1. READ_ONLY: Immutable shared data (no locks required)
 * 2. COPY_ON_WRITE: Lazy copying on first write
 * 3. LOCKED_WRITE: Explicit synchronization for writes
 * 
 * This enables efficient sharing between parent/child/sibling spheres
 * while maintaining thread safety and minimizing lock contention.
 */

// Cache line size for alignment (prevents false sharing)
#define CACHE_LINE_SIZE 64

// Shared memory access modes
typedef enum {
    SHARED_READ_ONLY,        // Immutable after creation (no locks)
    SHARED_COPY_ON_WRITE,    // Copy on first write (lazy)
    SHARED_LOCKED_WRITE      // Explicit locking for writes
} SharedMemoryAccessMode;

// Forward declarations for function pointers
typedef void* (*CopyFunction)(const void* src, size_t size);
typedef void (*FreeFunction)(void* ptr);

/**
 * Shared Memory Region
 * 
 * Represents a region of memory that can be shared between multiple
 * threads/spheres with different access patterns.
 */
typedef struct SharedMemoryRegion {
    // Memory
    void* data;                      // Pointer to actual data
    size_t size;                     // Current size in bytes
    size_t capacity;                 // Allocated capacity in bytes
    
    // Access control
    pthread_rwlock_t rwlock;         // Read-write lock
    _Atomic(int) num_readers;        // Number of active readers
    _Atomic(int) num_writers;        // Number of active writers
    
    // Mode
    SharedMemoryAccessMode access_mode;
    
    // Versioning (for cache coherency)
    _Atomic(uint64_t) version;       // Incremented on each write
    
    // Copy-on-write support
    CopyFunction copy_fn;            // Function to copy data
    FreeFunction free_fn;            // Function to free data
    bool is_copy;                    // True if this is a COW copy
    struct SharedMemoryRegion* original; // Original region (for COW)
    
    // Statistics
    _Atomic(uint64_t) read_count;    // Number of reads
    _Atomic(uint64_t) write_count;   // Number of writes
    _Atomic(uint64_t) copy_count;    // Number of COW copies made
    
    // Padding to cache line boundary
    char padding[CACHE_LINE_SIZE - 
                 (sizeof(void*) + sizeof(size_t) * 2 + 
                  sizeof(pthread_rwlock_t) + sizeof(_Atomic(int)) * 2 +
                  sizeof(SharedMemoryAccessMode) + sizeof(_Atomic(uint64_t)) +
                  sizeof(CopyFunction) + sizeof(FreeFunction) + 
                  sizeof(bool) + sizeof(void*) + sizeof(_Atomic(uint64_t)) * 3) % CACHE_LINE_SIZE];
    
} SharedMemoryRegion;

/**
 * Create shared memory region
 * 
 * @param size Initial size in bytes
 * @param mode Access mode (READ_ONLY, COPY_ON_WRITE, LOCKED_WRITE)
 * @return Allocated region, or NULL on error
 */
SharedMemoryRegion* shared_memory_create(size_t size, SharedMemoryAccessMode mode);

/**
 * Create shared memory region with custom copy/free functions
 * 
 * @param size Initial size in bytes
 * @param mode Access mode
 * @param copy_fn Custom copy function (for COW)
 * @param free_fn Custom free function
 * @return Allocated region, or NULL on error
 */
SharedMemoryRegion* shared_memory_create_custom(size_t size, 
                                                 SharedMemoryAccessMode mode,
                                                 CopyFunction copy_fn,
                                                 FreeFunction free_fn);

/**
 * Free shared memory region
 * 
 * @param region Region to free
 */
void shared_memory_free(SharedMemoryRegion* region);

/**
 * Acquire read access to shared memory
 * 
 * For READ_ONLY: No lock, just returns pointer
 * For COPY_ON_WRITE: Acquires read lock
 * For LOCKED_WRITE: Acquires read lock
 * 
 * @param region Region to read from
 * @return Pointer to data, or NULL on error
 */
const void* shared_memory_read(SharedMemoryRegion* region);

/**
 * Release read access
 * 
 * @param region Region to release
 */
void shared_memory_release_read(SharedMemoryRegion* region);

/**
 * Acquire write access to shared memory
 * 
 * For READ_ONLY: Returns NULL (immutable)
 * For COPY_ON_WRITE: Creates copy if needed, returns writable pointer
 * For LOCKED_WRITE: Acquires write lock, returns writable pointer
 * 
 * @param region Region to write to
 * @return Pointer to writable data, or NULL on error
 */
void* shared_memory_write(SharedMemoryRegion* region);

/**
 * Release write access
 * 
 * @param region Region to release
 */
void shared_memory_release_write(SharedMemoryRegion* region);

/**
 * Get current version of shared memory
 * 
 * Used for cache coherency checks.
 * 
 * @param region Region to query
 * @return Current version number
 */
uint64_t shared_memory_get_version(const SharedMemoryRegion* region);

/**
 * Check if shared memory has been modified since version
 * 
 * @param region Region to check
 * @param version Version to compare against
 * @return true if modified, false otherwise
 */
bool shared_memory_is_modified(const SharedMemoryRegion* region, uint64_t version);

/**
 * Resize shared memory region
 * 
 * Only allowed for LOCKED_WRITE mode.
 * Caller must hold write lock.
 * 
 * @param region Region to resize
 * @param new_size New size in bytes
 * @return 0 on success, -1 on error
 */
int shared_memory_resize(SharedMemoryRegion* region, size_t new_size);

/**
 * Get statistics for shared memory region
 * 
 * @param region Region to query
 * @param out_reads Output: number of reads
 * @param out_writes Output: number of writes
 * @param out_copies Output: number of COW copies
 */
void shared_memory_get_stats(const SharedMemoryRegion* region,
                             uint64_t* out_reads,
                             uint64_t* out_writes,
                             uint64_t* out_copies);

/**
 * Print shared memory region info (for debugging)
 * 
 * @param region Region to print
 * @param name Optional name for the region
 */
void shared_memory_print_info(const SharedMemoryRegion* region, const char* name);

/**
 * Default copy function (memcpy)
 * 
 * @param src Source data
 * @param size Size in bytes
 * @return Allocated copy, or NULL on error
 */
void* shared_memory_default_copy(const void* src, size_t size);

/**
 * Default free function (free)
 * 
 * @param ptr Pointer to free
 */
void shared_memory_default_free(void* ptr);

/**
 * Create read-only view of existing data
 * 
 * Creates a SharedMemoryRegion that wraps existing data without copying.
 * The data must remain valid for the lifetime of the region.
 * 
 * @param data Pointer to existing data
 * @param size Size of data in bytes
 * @return Allocated region, or NULL on error
 */
SharedMemoryRegion* shared_memory_create_readonly_view(const void* data, size_t size);

/**
 * Validate shared memory region
 * 
 * Checks for consistency and correctness.
 * 
 * @param region Region to validate
 * @return true if valid, false otherwise
 */
bool shared_memory_validate(const SharedMemoryRegion* region);

#ifdef __cplusplus
}
#endif

#endif /* CLLM_SHARED_MEMORY_H */
