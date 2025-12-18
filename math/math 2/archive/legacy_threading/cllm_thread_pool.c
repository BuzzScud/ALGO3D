#include "ai/cllm_thread_pool.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

static uint64_t get_time_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

// ============================================================================
// THREAD POOL LIFECYCLE
// ============================================================================

ThreadPool* thread_pool_create(size_t capacity, bool enable_reuse) {
    // Cap capacity at MAX_THREADS
    if (capacity > MAX_THREADS) {
        capacity = MAX_THREADS;
    }
    
    if (capacity == 0) {
        fprintf(stderr, "ERROR: Thread pool capacity must be > 0\n");
        return NULL;
    }
    
    // Allocate pool
    ThreadPool* pool = calloc(1, sizeof(ThreadPool));
    if (!pool) {
        fprintf(stderr, "ERROR: Failed to allocate thread pool\n");
        return NULL;
    }
    
    // Allocate thread array
    pool->threads = calloc(capacity, sizeof(ThreadEntry*));
    if (!pool->threads) {
        fprintf(stderr, "ERROR: Failed to allocate thread array\n");
        free(pool);
        return NULL;
    }
    
    // Initialize pool
    pool->capacity = capacity;
    atomic_init(&pool->active_count, 0);
    atomic_init(&pool->total_count, 0);
    pool->free_list = NULL;
    atomic_init(&pool->free_count, 0);
    pool->enable_reuse = enable_reuse;
    pool->reuse_threshold = capacity / 10;  // Reuse when 10% free
    
    // Initialize mutexes
    pthread_mutex_init(&pool->pool_mutex, NULL);
    pthread_mutex_init(&pool->free_list_mutex, NULL);
    
    // Initialize statistics
    atomic_init(&pool->stats.threads_created, 0);
    atomic_init(&pool->stats.threads_destroyed, 0);
    atomic_init(&pool->stats.spawn_requests, 0);
    atomic_init(&pool->stats.spawn_rejections, 0);
    atomic_init(&pool->stats.reuse_count, 0);
    atomic_init(&pool->stats.peak_thread_count, 0);
    
    return pool;
}

void thread_pool_destroy(ThreadPool* pool) {
    if (!pool) return;
    
    // Free all thread entries
    pthread_mutex_lock(&pool->pool_mutex);
    
    for (size_t i = 0; i < pool->capacity; i++) {
        if (pool->threads[i]) {
            free(pool->threads[i]);
            pool->threads[i] = NULL;
        }
    }
    
    pthread_mutex_unlock(&pool->pool_mutex);
    
    // Free free list
    pthread_mutex_lock(&pool->free_list_mutex);
    
    ThreadEntry* current = pool->free_list;
    while (current) {
        ThreadEntry* next = current->next;
        free(current);
        current = next;
    }
    
    pthread_mutex_unlock(&pool->free_list_mutex);
    
    // Free thread array
    free(pool->threads);
    
    // Destroy mutexes
    pthread_mutex_destroy(&pool->pool_mutex);
    pthread_mutex_destroy(&pool->free_list_mutex);
    
    // Free pool
    free(pool);
}

// ============================================================================
// THREAD REGISTRATION
// ============================================================================

ThreadEntry* thread_pool_register(ThreadPool* pool, 
                                   pthread_t thread_id,
                                   int sphere_id,
                                   int hierarchy_level,
                                   int symmetry_group) {
    if (!pool) return NULL;
    
    pthread_mutex_lock(&pool->pool_mutex);
    
    // Check if pool is full
    size_t active = atomic_load(&pool->active_count);
    if (active >= pool->capacity) {
        pthread_mutex_unlock(&pool->pool_mutex);
        fprintf(stderr, "ERROR: Thread pool is full (%zu/%zu)\n", 
                active, pool->capacity);
        return NULL;
    }
    
    // Try to reuse an entry from free list
    ThreadEntry* entry = NULL;
    
    if (pool->enable_reuse) {
        pthread_mutex_lock(&pool->free_list_mutex);
        
        if (pool->free_list) {
            entry = pool->free_list;
            pool->free_list = entry->next;
            atomic_fetch_sub(&pool->free_count, 1);
            atomic_fetch_add(&pool->stats.reuse_count, 1);
        }
        
        pthread_mutex_unlock(&pool->free_list_mutex);
    }
    
    // Allocate new entry if no reuse
    if (!entry) {
        entry = calloc(1, sizeof(ThreadEntry));
        if (!entry) {
            pthread_mutex_unlock(&pool->pool_mutex);
            fprintf(stderr, "ERROR: Failed to allocate thread entry\n");
            return NULL;
        }
    }
    
    // Initialize entry
    entry->thread_id = thread_id;
    entry->sphere_id = sphere_id;
    entry->hierarchy_level = hierarchy_level;
    entry->symmetry_group = symmetry_group;
    entry->active = true;
    entry->creation_time_ns = get_time_ns();
    entry->last_active_time_ns = entry->creation_time_ns;
    entry->next = NULL;
    
    // Find empty slot
    size_t slot = 0;
    for (slot = 0; slot < pool->capacity; slot++) {
        if (!pool->threads[slot]) {
            pool->threads[slot] = entry;
            break;
        }
    }
    
    // Update counters
    atomic_fetch_add(&pool->active_count, 1);
    atomic_fetch_add(&pool->total_count, 1);
    atomic_fetch_add(&pool->stats.threads_created, 1);
    
    // Update peak count
    size_t current_count = atomic_load(&pool->active_count);
    size_t peak = atomic_load(&pool->stats.peak_thread_count);
    while (current_count > peak) {
        if (atomic_compare_exchange_weak(&pool->stats.peak_thread_count, 
                                        &peak, current_count)) {
            break;
        }
    }
    
    pthread_mutex_unlock(&pool->pool_mutex);
    
    return entry;
}

int thread_pool_unregister(ThreadPool* pool, pthread_t thread_id) {
    if (!pool) return 0;
    
    pthread_mutex_lock(&pool->pool_mutex);
    
    // Find thread entry
    ThreadEntry* entry = NULL;
    size_t slot = 0;
    
    for (slot = 0; slot < pool->capacity; slot++) {
        if (pool->threads[slot] && 
            pthread_equal(pool->threads[slot]->thread_id, thread_id)) {
            entry = pool->threads[slot];
            pool->threads[slot] = NULL;
            break;
        }
    }
    
    if (!entry) {
        pthread_mutex_unlock(&pool->pool_mutex);
        return 0;
    }
    
    // Mark as inactive
    entry->active = false;
    
    // Update counters
    atomic_fetch_sub(&pool->active_count, 1);
    atomic_fetch_add(&pool->stats.threads_destroyed, 1);
    
    // Add to free list if reuse is enabled
    if (pool->enable_reuse) {
        pthread_mutex_lock(&pool->free_list_mutex);
        
        entry->next = pool->free_list;
        pool->free_list = entry;
        atomic_fetch_add(&pool->free_count, 1);
        
        pthread_mutex_unlock(&pool->free_list_mutex);
    } else {
        // Free immediately
        free(entry);
    }
    
    pthread_mutex_unlock(&pool->pool_mutex);
    
    return 1;
}

// ============================================================================
// SPAWN CONTROL
// ============================================================================

bool thread_pool_can_spawn(const ThreadPool* pool, size_t num_threads) {
    if (!pool) return false;
    
    size_t active = atomic_load(&pool->active_count);
    size_t available = pool->capacity - active;
    
    return available >= num_threads;
}

bool thread_pool_reserve(ThreadPool* pool, size_t num_threads) {
    if (!pool) return false;
    
    atomic_fetch_add(&pool->stats.spawn_requests, 1);
    
    // Check if we have enough space
    size_t active = atomic_load(&pool->active_count);
    if (active + num_threads > pool->capacity) {
        atomic_fetch_add(&pool->stats.spawn_rejections, 1);
        return false;
    }
    
    // Atomically reserve slots
    size_t expected = active;
    while (expected + num_threads <= pool->capacity) {
        if (atomic_compare_exchange_weak(&pool->active_count, 
                                        &expected, 
                                        expected + num_threads)) {
            return true;
        }
    }
    
    atomic_fetch_add(&pool->stats.spawn_rejections, 1);
    return false;
}

void thread_pool_release(ThreadPool* pool, size_t num_threads) {
    if (!pool) return;
    
    atomic_fetch_sub(&pool->active_count, num_threads);
}

// ============================================================================
// THREAD QUERIES
// ============================================================================

size_t thread_pool_get_active_count(const ThreadPool* pool) {
    if (!pool) return 0;
    return atomic_load(&pool->active_count);
}

size_t thread_pool_get_total_count(const ThreadPool* pool) {
    if (!pool) return 0;
    return atomic_load(&pool->total_count);
}

size_t thread_pool_get_available(const ThreadPool* pool) {
    if (!pool) return 0;
    
    size_t active = atomic_load(&pool->active_count);
    if (active >= pool->capacity) {
        return 0;
    }
    
    return pool->capacity - active;
}

ThreadEntry* thread_pool_find(const ThreadPool* pool, pthread_t thread_id) {
    if (!pool) return NULL;
    
    pthread_mutex_lock((pthread_mutex_t*)&pool->pool_mutex);
    
    ThreadEntry* result = NULL;
    
    for (size_t i = 0; i < pool->capacity; i++) {
        if (pool->threads[i] && 
            pthread_equal(pool->threads[i]->thread_id, thread_id)) {
            result = pool->threads[i];
            break;
        }
    }
    
    pthread_mutex_unlock((pthread_mutex_t*)&pool->pool_mutex);
    
    return result;
}

// ============================================================================
// STATISTICS
// ============================================================================

void thread_pool_get_stats(const ThreadPool* pool, ThreadPoolStats* stats) {
    if (!pool || !stats) return;
    
    stats->threads_created = atomic_load(&pool->stats.threads_created);
    stats->threads_destroyed = atomic_load(&pool->stats.threads_destroyed);
    stats->spawn_requests = atomic_load(&pool->stats.spawn_requests);
    stats->spawn_rejections = atomic_load(&pool->stats.spawn_rejections);
    stats->reuse_count = atomic_load(&pool->stats.reuse_count);
    stats->peak_thread_count = atomic_load(&pool->stats.peak_thread_count);
}

void thread_pool_print_stats(const ThreadPool* pool) {
    if (!pool) return;
    
    ThreadPoolStats stats;
    thread_pool_get_stats(pool, &stats);
    
    printf("=== Thread Pool Statistics ===\n");
    printf("Threads Created:     %lu\n", (unsigned long)stats.threads_created);
    printf("Threads Destroyed:   %lu\n", (unsigned long)stats.threads_destroyed);
    printf("Spawn Requests:      %lu\n", (unsigned long)stats.spawn_requests);
    printf("Spawn Rejections:    %lu\n", (unsigned long)stats.spawn_rejections);
    printf("Reuse Count:         %lu\n", (unsigned long)stats.reuse_count);
    printf("Peak Thread Count:   %lu\n", (unsigned long)stats.peak_thread_count);
    
    if (stats.spawn_requests > 0) {
        double rejection_rate = (double)stats.spawn_rejections / 
                               (double)stats.spawn_requests * 100.0;
        printf("Rejection Rate:      %.2f%%\n", rejection_rate);
    }
    
    printf("==============================\n");
}

void thread_pool_reset_stats(ThreadPool* pool) {
    if (!pool) return;
    
    atomic_store(&pool->stats.threads_created, 0);
    atomic_store(&pool->stats.threads_destroyed, 0);
    atomic_store(&pool->stats.spawn_requests, 0);
    atomic_store(&pool->stats.spawn_rejections, 0);
    atomic_store(&pool->stats.reuse_count, 0);
    atomic_store(&pool->stats.peak_thread_count, 0);
}

// ============================================================================
// UTILITIES
// ============================================================================

int thread_pool_validate(const ThreadPool* pool) {
    if (!pool) return 0;
    
    int valid = 1;
    
    // Check capacity
    if (pool->capacity == 0 || pool->capacity > MAX_THREADS) {
        fprintf(stderr, "ERROR: Invalid capacity: %zu\n", pool->capacity);
        valid = 0;
    }
    
    // Check active count
    size_t active = atomic_load(&pool->active_count);
    if (active > pool->capacity) {
        fprintf(stderr, "ERROR: Active count (%zu) exceeds capacity (%zu)\n",
                active, pool->capacity);
        valid = 0;
    }
    
    // Check thread array
    if (!pool->threads) {
        fprintf(stderr, "ERROR: Thread array is NULL\n");
        valid = 0;
    }
    
    return valid;
}

void thread_pool_print(const ThreadPool* pool) {
    if (!pool) {
        printf("NULL thread pool\n");
        return;
    }
    
    printf("=== Thread Pool ===\n");
    printf("Capacity:        %zu\n", pool->capacity);
    printf("Active Threads:  %zu\n", thread_pool_get_active_count(pool));
    printf("Total Threads:   %zu\n", thread_pool_get_total_count(pool));
    printf("Available Slots: %zu\n", thread_pool_get_available(pool));
    printf("Free List Size:  %zu\n", (size_t)atomic_load(&pool->free_count));
    printf("Reuse Enabled:   %s\n", pool->enable_reuse ? "Yes" : "No");
    printf("===================\n");
}