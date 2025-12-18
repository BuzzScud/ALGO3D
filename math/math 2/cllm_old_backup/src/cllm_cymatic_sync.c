/**
 * @file cllm_cymatic_sync.c
 * @brief Implementation of cymatic frequency-based thread synchronization
 */

#include "ai/cllm_cymatic_sync.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

/**
 * @brief Calculate time difference in nanoseconds
 */
uint64_t timespec_diff_ns(const struct timespec* end, const struct timespec* start) {
    uint64_t end_ns = (uint64_t)end->tv_sec * NS_PER_SECOND + (uint64_t)end->tv_nsec;
    uint64_t start_ns = (uint64_t)start->tv_sec * NS_PER_SECOND + (uint64_t)start->tv_nsec;
    
    if (end_ns < start_ns) {
        return 0;
    }
    
    return end_ns - start_ns;
}

/**
 * @brief Convert nanoseconds to timespec
 */
struct timespec ns_to_timespec(uint64_t ns) {
    struct timespec ts;
    ts.tv_sec = ns / NS_PER_SECOND;
    ts.tv_nsec = ns % NS_PER_SECOND;
    return ts;
}

/**
 * @brief Add nanoseconds to timespec
 */
struct timespec timespec_add_ns(const struct timespec* ts, uint64_t ns) {
    struct timespec result;
    uint64_t total_ns = (uint64_t)ts->tv_sec * NS_PER_SECOND + (uint64_t)ts->tv_nsec + ns;
    result.tv_sec = total_ns / NS_PER_SECOND;
    result.tv_nsec = total_ns % NS_PER_SECOND;
    return result;
}

/**
 * @brief Create a cymatic barrier
 */
CymaticBarrier* cymatic_barrier_create(
    int num_threads,
    CymaticFrequencyType frequency_type,
    bool adaptive
) {
    if (num_threads <= 0) {
        fprintf(stderr, "ERROR: Invalid num_threads: %d\n", num_threads);
        return NULL;
    }
    
    uint64_t frequency_ns = cymatic_get_period_ns(frequency_type);
    if (frequency_ns == 0) {
        fprintf(stderr, "ERROR: Invalid frequency type\n");
        return NULL;
    }
    
    CymaticBarrier* barrier = (CymaticBarrier*)calloc(1, sizeof(CymaticBarrier));
    if (!barrier) {
        fprintf(stderr, "ERROR: Failed to allocate barrier\n");
        return NULL;
    }
    
    // Initialize barrier
    if (pthread_barrier_init(&barrier->barrier, NULL, num_threads) != 0) {
        fprintf(stderr, "ERROR: Failed to initialize pthread barrier\n");
        free(barrier);
        return NULL;
    }
    
    // Initialize mutex
    if (pthread_mutex_init(&barrier->mutex, NULL) != 0) {
        fprintf(stderr, "ERROR: Failed to initialize mutex\n");
        pthread_barrier_destroy(&barrier->barrier);
        free(barrier);
        return NULL;
    }
    
    // Set parameters
    barrier->frequency_ns = frequency_ns;
    barrier->adaptive = adaptive;
    barrier->num_threads = num_threads;
    
    // Initialize timing
    clock_gettime(CLOCK_MONOTONIC, &barrier->last_sync);
    
    // Initialize statistics
    barrier->sync_count = 0;
    barrier->total_wait_ns = 0;
    barrier->min_wait_ns = UINT64_MAX;
    barrier->max_wait_ns = 0;
    
    return barrier;
}

/**
 * @brief Create a cymatic barrier with custom frequency
 */
CymaticBarrier* cymatic_barrier_create_custom(
    int num_threads,
    double frequency_hz,
    bool adaptive
) {
    if (num_threads <= 0) {
        fprintf(stderr, "ERROR: Invalid num_threads: %d\n", num_threads);
        return NULL;
    }
    
    if (frequency_hz <= 0.0) {
        fprintf(stderr, "ERROR: Invalid frequency: %f\n", frequency_hz);
        return NULL;
    }
    
    uint64_t frequency_ns = cymatic_frequency_to_ns(frequency_hz);
    if (frequency_ns == 0) {
        fprintf(stderr, "ERROR: Failed to convert frequency\n");
        return NULL;
    }
    
    CymaticBarrier* barrier = (CymaticBarrier*)calloc(1, sizeof(CymaticBarrier));
    if (!barrier) {
        fprintf(stderr, "ERROR: Failed to allocate barrier\n");
        return NULL;
    }
    
    // Initialize barrier
    if (pthread_barrier_init(&barrier->barrier, NULL, num_threads) != 0) {
        fprintf(stderr, "ERROR: Failed to initialize pthread barrier\n");
        free(barrier);
        return NULL;
    }
    
    // Initialize mutex
    if (pthread_mutex_init(&barrier->mutex, NULL) != 0) {
        fprintf(stderr, "ERROR: Failed to initialize mutex\n");
        pthread_barrier_destroy(&barrier->barrier);
        free(barrier);
        return NULL;
    }
    
    // Set parameters
    barrier->frequency_ns = frequency_ns;
    barrier->adaptive = adaptive;
    barrier->num_threads = num_threads;
    
    // Initialize timing
    clock_gettime(CLOCK_MONOTONIC, &barrier->last_sync);
    
    // Initialize statistics
    barrier->sync_count = 0;
    barrier->total_wait_ns = 0;
    barrier->min_wait_ns = UINT64_MAX;
    barrier->max_wait_ns = 0;
    
    return barrier;
}

/**
 * @brief Destroy a cymatic barrier
 */
void cymatic_barrier_destroy(CymaticBarrier* barrier) {
    if (!barrier) {
        return;
    }
    
    pthread_barrier_destroy(&barrier->barrier);
    pthread_mutex_destroy(&barrier->mutex);
    free(barrier);
}

/**
 * @brief Wait at cymatic barrier
 */
int cymatic_barrier_wait(CymaticBarrier* barrier) {
    if (!barrier) {
        return -1;
    }
    
    struct timespec wait_start, wait_end, now;
    clock_gettime(CLOCK_MONOTONIC, &wait_start);
    
    // Calculate time since last sync
    uint64_t elapsed_ns = timespec_diff_ns(&wait_start, &barrier->last_sync);
    
    // Calculate sleep time
    uint64_t sleep_ns = 0;
    if (elapsed_ns < barrier->frequency_ns) {
        sleep_ns = barrier->frequency_ns - elapsed_ns;
    } else if (!barrier->adaptive) {
        // Not adaptive - still wait for next period
        uint64_t periods_passed = elapsed_ns / barrier->frequency_ns;
        uint64_t next_period = (periods_passed + 1) * barrier->frequency_ns;
        sleep_ns = next_period - elapsed_ns;
    }
    // If adaptive and already past period, don't sleep
    
    // Sleep until next frequency period
    if (sleep_ns > 0) {
        struct timespec sleep_time = ns_to_timespec(sleep_ns);
        nanosleep(&sleep_time, NULL);
    }
    
    // Synchronize all threads at barrier
    int result = pthread_barrier_wait(&barrier->barrier);
    
    // One thread updates timing (the one that gets PTHREAD_BARRIER_SERIAL_THREAD)
    if (result == PTHREAD_BARRIER_SERIAL_THREAD) {
        clock_gettime(CLOCK_MONOTONIC, &now);
        
        // Update statistics (thread-safe)
        pthread_mutex_lock(&barrier->mutex);
        
        // Update last sync time
        barrier->last_sync = now;
        
        // Update statistics
        clock_gettime(CLOCK_MONOTONIC, &wait_end);
        uint64_t wait_time = timespec_diff_ns(&wait_end, &wait_start);
        
        barrier->sync_count++;
        barrier->total_wait_ns += wait_time;
        
        if (wait_time < barrier->min_wait_ns) {
            barrier->min_wait_ns = wait_time;
        }
        if (wait_time > barrier->max_wait_ns) {
            barrier->max_wait_ns = wait_time;
        }
        
        pthread_mutex_unlock(&barrier->mutex);
    }
    
    return (result == PTHREAD_BARRIER_SERIAL_THREAD || result == 0) ? 0 : -1;
}

/**
 * @brief Try to wait at cymatic barrier (non-blocking check)
 */
int cymatic_barrier_try_wait(CymaticBarrier* barrier) {
    if (!barrier) {
        return -1;
    }
    
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    
    // Calculate time since last sync
    uint64_t elapsed_ns = timespec_diff_ns(&now, &barrier->last_sync);
    
    // Check if it's time to sync
    if (elapsed_ns >= barrier->frequency_ns) {
        return cymatic_barrier_wait(barrier);
    }
    
    return 0;  // Not time yet
}

/**
 * @brief Get barrier statistics
 */
int cymatic_barrier_get_stats(
    const CymaticBarrier* barrier,
    CymaticBarrierStats* stats
) {
    if (!barrier || !stats) {
        return -1;
    }
    
    memset(stats, 0, sizeof(CymaticBarrierStats));
    
    stats->sync_count = barrier->sync_count;
    stats->total_wait_ns = barrier->total_wait_ns;
    stats->min_wait_ns = barrier->min_wait_ns;
    stats->max_wait_ns = barrier->max_wait_ns;
    
    if (barrier->sync_count > 0) {
        stats->avg_wait_ns = barrier->total_wait_ns / barrier->sync_count;
    }
    
    stats->frequency_hz = cymatic_ns_to_frequency(barrier->frequency_ns);
    
    // Calculate actual frequency from sync times
    if (barrier->sync_count > 1) {
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        uint64_t total_time_ns = timespec_diff_ns(&now, &barrier->last_sync);
        if (total_time_ns > 0) {
            stats->actual_frequency_hz = (double)(barrier->sync_count - 1) * NS_PER_SECOND / total_time_ns;
        }
    }
    
    return 0;
}

/**
 * @brief Reset barrier statistics
 */
void cymatic_barrier_reset_stats(CymaticBarrier* barrier) {
    if (!barrier) {
        return;
    }
    
    pthread_mutex_lock(&barrier->mutex);
    
    barrier->sync_count = 0;
    barrier->total_wait_ns = 0;
    barrier->min_wait_ns = UINT64_MAX;
    barrier->max_wait_ns = 0;
    
    clock_gettime(CLOCK_MONOTONIC, &barrier->last_sync);
    
    pthread_mutex_unlock(&barrier->mutex);
}

/**
 * @brief Print barrier statistics
 */
void cymatic_barrier_print_stats(const CymaticBarrier* barrier) {
    if (!barrier) {
        return;
    }
    
    CymaticBarrierStats stats;
    if (cymatic_barrier_get_stats(barrier, &stats) != 0) {
        return;
    }
    
    printf("\n=== Cymatic Barrier Statistics ===\n");
    printf("Configured frequency: %.2f Hz\n", stats.frequency_hz);
    printf("Actual frequency:     %.2f Hz\n", stats.actual_frequency_hz);
    printf("Sync count:           %lu\n", stats.sync_count);
    printf("Total wait time:      %.2f ms\n", stats.total_wait_ns / 1000000.0);
    printf("Average wait time:    %.2f ms\n", stats.avg_wait_ns / 1000000.0);
    printf("Min wait time:        %.2f ms\n", stats.min_wait_ns / 1000000.0);
    printf("Max wait time:        %.2f ms\n", stats.max_wait_ns / 1000000.0);
    printf("===================================\n\n");
}

/**
 * @brief Set barrier frequency
 */
int cymatic_barrier_set_frequency(
    CymaticBarrier* barrier,
    CymaticFrequencyType frequency_type
) {
    if (!barrier) {
        return -1;
    }
    
    uint64_t frequency_ns = cymatic_get_period_ns(frequency_type);
    if (frequency_ns == 0) {
        return -1;
    }
    
    pthread_mutex_lock(&barrier->mutex);
    barrier->frequency_ns = frequency_ns;
    pthread_mutex_unlock(&barrier->mutex);
    
    return 0;
}

/**
 * @brief Set custom barrier frequency
 */
int cymatic_barrier_set_custom_frequency(
    CymaticBarrier* barrier,
    double frequency_hz
) {
    if (!barrier || frequency_hz <= 0.0) {
        return -1;
    }
    
    uint64_t frequency_ns = cymatic_frequency_to_ns(frequency_hz);
    if (frequency_ns == 0) {
        return -1;
    }
    
    pthread_mutex_lock(&barrier->mutex);
    barrier->frequency_ns = frequency_ns;
    pthread_mutex_unlock(&barrier->mutex);
    
    return 0;
}
