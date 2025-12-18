#ifndef CLLM_THREAD_POOL_H
#define CLLM_THREAD_POOL_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdatomic.h>

// Maximum number of threads allowed in the entire system
#define MAX_THREADS 144000

// Thread pool statistics
typedef struct {
    atomic_uint_fast64_t threads_created;
    atomic_uint_fast64_t threads_destroyed;
    atomic_uint_fast64_t spawn_requests;
    atomic_uint_fast64_t spawn_rejections;
    atomic_uint_fast64_t reuse_count;
    atomic_uint_fast64_t peak_thread_count;
} ThreadPoolStats;

// Thread entry in the pool
typedef struct ThreadEntry {
    pthread_t thread_id;
    int sphere_id;
    int hierarchy_level;
    int symmetry_group;
    bool active;
    uint64_t creation_time_ns;
    uint64_t last_active_time_ns;
    struct ThreadEntry* next;
} ThreadEntry;

// Thread pool structure
typedef struct {
    // Thread tracking
    ThreadEntry** threads;          // Array of thread entries
    size_t capacity;                // Maximum capacity (144000)
    atomic_size_t active_count;     // Current number of active threads
    atomic_size_t total_count;      // Total threads ever created
    
    // Thread reuse
    ThreadEntry* free_list;         // List of reusable thread entries
    atomic_size_t free_count;       // Number of entries in free list
    
    // Synchronization
    pthread_mutex_t pool_mutex;     // Protects pool operations
    pthread_mutex_t free_list_mutex; // Protects free list
    
    // Statistics
    ThreadPoolStats stats;
    
    // Configuration
    bool enable_reuse;              // Enable thread entry reuse
    size_t reuse_threshold;         // Minimum free entries before reuse
} ThreadPool;

// ============================================================================
// THREAD POOL LIFECYCLE
// ============================================================================

/**
 * Create a new thread pool
 * 
 * Initializes a thread pool with the specified capacity (max 144,000).
 * 
 * @param capacity Maximum number of threads (capped at MAX_THREADS)
 * @param enable_reuse Enable thread entry reuse
 * @return Pointer to thread pool, or NULL on failure
 */
ThreadPool* thread_pool_create(size_t capacity, bool enable_reuse);

/**
 * Destroy thread pool
 * 
 * Frees all resources associated with the thread pool.
 * Does NOT terminate running threads - caller must ensure threads are stopped.
 * 
 * @param pool Thread pool to destroy
 */
void thread_pool_destroy(ThreadPool* pool);

// ============================================================================
// THREAD REGISTRATION
// ============================================================================

/**
 * Register a new thread in the pool
 * 
 * Adds a thread to the pool's tracking system.
 * 
 * @param pool Thread pool
 * @param thread_id pthread_t of the thread
 * @param sphere_id Sphere ID
 * @param hierarchy_level Hierarchy level
 * @param symmetry_group Symmetry group (0-11)
 * @return Thread entry pointer, or NULL if pool is full
 */
ThreadEntry* thread_pool_register(ThreadPool* pool, 
                                   pthread_t thread_id,
                                   int sphere_id,
                                   int hierarchy_level,
                                   int symmetry_group);

/**
 * Unregister a thread from the pool
 * 
 * Removes a thread from the pool's tracking system.
 * 
 * @param pool Thread pool
 * @param thread_id pthread_t of the thread
 * @return 1 on success, 0 on failure
 */
int thread_pool_unregister(ThreadPool* pool, pthread_t thread_id);

// ============================================================================
// SPAWN CONTROL
// ============================================================================

/**
 * Check if spawning is allowed
 * 
 * Determines if the pool can accommodate additional threads.
 * 
 * @param pool Thread pool
 * @param num_threads Number of threads to spawn
 * @return true if spawning is allowed, false otherwise
 */
bool thread_pool_can_spawn(const ThreadPool* pool, size_t num_threads);

/**
 * Reserve thread slots
 * 
 * Atomically reserves slots for new threads.
 * Must be followed by thread_pool_register() for each reserved slot.
 * 
 * @param pool Thread pool
 * @param num_threads Number of threads to reserve
 * @return true if reservation successful, false if pool is full
 */
bool thread_pool_reserve(ThreadPool* pool, size_t num_threads);

/**
 * Release reserved thread slots
 * 
 * Releases previously reserved slots that were not used.
 * 
 * @param pool Thread pool
 * @param num_threads Number of threads to release
 */
void thread_pool_release(ThreadPool* pool, size_t num_threads);

// ============================================================================
// THREAD QUERIES
// ============================================================================

/**
 * Get current active thread count
 * 
 * @param pool Thread pool
 * @return Number of active threads
 */
size_t thread_pool_get_active_count(const ThreadPool* pool);

/**
 * Get total thread count (including inactive)
 * 
 * @param pool Thread pool
 * @return Total number of threads
 */
size_t thread_pool_get_total_count(const ThreadPool* pool);

/**
 * Get available thread slots
 * 
 * @param pool Thread pool
 * @return Number of available slots
 */
size_t thread_pool_get_available(const ThreadPool* pool);

/**
 * Find thread entry by pthread_t
 * 
 * @param pool Thread pool
 * @param thread_id pthread_t to find
 * @return Thread entry pointer, or NULL if not found
 */
ThreadEntry* thread_pool_find(const ThreadPool* pool, pthread_t thread_id);

// ============================================================================
// STATISTICS
// ============================================================================

/**
 * Get thread pool statistics
 * 
 * @param pool Thread pool
 * @param stats Pointer to stats structure to fill
 */
void thread_pool_get_stats(const ThreadPool* pool, ThreadPoolStats* stats);

/**
 * Print thread pool statistics
 * 
 * @param pool Thread pool
 */
void thread_pool_print_stats(const ThreadPool* pool);

/**
 * Reset thread pool statistics
 * 
 * @param pool Thread pool
 */
void thread_pool_reset_stats(ThreadPool* pool);

// ============================================================================
// UTILITIES
// ============================================================================

/**
 * Validate thread pool state
 * 
 * Checks for consistency and correctness.
 * 
 * @param pool Thread pool
 * @return 1 if valid, 0 if errors found
 */
int thread_pool_validate(const ThreadPool* pool);

/**
 * Print thread pool information
 * 
 * @param pool Thread pool
 */
void thread_pool_print(const ThreadPool* pool);

#endif // CLLM_THREAD_POOL_H
