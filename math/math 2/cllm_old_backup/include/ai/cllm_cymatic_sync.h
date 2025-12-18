/**
 * @file cllm_cymatic_sync.h
 * @brief Cymatic frequency-based thread synchronization
 * 
 * Implements frequency-based barriers that synchronize threads at specific
 * cymatic frequencies (432 Hz, 528 Hz, Schumann resonance, etc.).
 * 
 * This provides natural timing synchronization based on harmonic frequencies
 * rather than arbitrary time intervals.
 */

#ifndef CLLM_CYMATIC_SYNC_H
#define CLLM_CYMATIC_SYNC_H

#include "ai/cllm_cymatic_frequencies.h"
#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Cymatic barrier for frequency-based synchronization
 * 
 * Synchronizes threads at a specific cymatic frequency, ensuring all threads
 * meet at regular intervals determined by the frequency period.
 */
typedef struct {
    pthread_barrier_t barrier;      /**< Pthread barrier for synchronization */
    pthread_mutex_t mutex;          /**< Mutex for thread-safe updates */
    uint64_t frequency_ns;          /**< Frequency period in nanoseconds */
    struct timespec last_sync;      /**< Time of last synchronization */
    uint64_t sync_count;            /**< Number of synchronizations */
    uint64_t total_wait_ns;         /**< Total wait time in nanoseconds */
    uint64_t min_wait_ns;           /**< Minimum wait time */
    uint64_t max_wait_ns;           /**< Maximum wait time */
    bool adaptive;                  /**< Skip sync if already past period */
    int num_threads;                /**< Number of threads using barrier */
} CymaticBarrier;

/**
 * @brief Cymatic barrier statistics
 */
typedef struct {
    uint64_t sync_count;            /**< Number of synchronizations */
    uint64_t total_wait_ns;         /**< Total wait time */
    uint64_t avg_wait_ns;           /**< Average wait time */
    uint64_t min_wait_ns;           /**< Minimum wait time */
    uint64_t max_wait_ns;           /**< Maximum wait time */
    double frequency_hz;            /**< Configured frequency */
    double actual_frequency_hz;     /**< Actual measured frequency */
} CymaticBarrierStats;

/**
 * @brief Create a cymatic barrier
 * 
 * @param num_threads Number of threads that will use this barrier
 * @param frequency_type Cymatic frequency type
 * @param adaptive If true, skip sync if already past period
 * @return Pointer to created barrier, or NULL on failure
 */
CymaticBarrier* cymatic_barrier_create(
    int num_threads,
    CymaticFrequencyType frequency_type,
    bool adaptive
);

/**
 * @brief Create a cymatic barrier with custom frequency
 * 
 * @param num_threads Number of threads that will use this barrier
 * @param frequency_hz Custom frequency in Hz
 * @param adaptive If true, skip sync if already past period
 * @return Pointer to created barrier, or NULL on failure
 */
CymaticBarrier* cymatic_barrier_create_custom(
    int num_threads,
    double frequency_hz,
    bool adaptive
);

/**
 * @brief Destroy a cymatic barrier
 * 
 * @param barrier Barrier to destroy
 */
void cymatic_barrier_destroy(CymaticBarrier* barrier);

/**
 * @brief Wait at cymatic barrier
 * 
 * Blocks until all threads reach the barrier, then waits until the next
 * frequency period before releasing all threads.
 * 
 * @param barrier Barrier to wait at
 * @return 0 on success, -1 on error
 */
int cymatic_barrier_wait(CymaticBarrier* barrier);

/**
 * @brief Try to wait at cymatic barrier (non-blocking check)
 * 
 * Checks if it's time to sync, but doesn't block if not.
 * 
 * @param barrier Barrier to check
 * @return 1 if synced, 0 if not time yet, -1 on error
 */
int cymatic_barrier_try_wait(CymaticBarrier* barrier);

/**
 * @brief Get barrier statistics
 * 
 * @param barrier Barrier to query
 * @param stats Output statistics structure
 * @return 0 on success, -1 on error
 */
int cymatic_barrier_get_stats(
    const CymaticBarrier* barrier,
    CymaticBarrierStats* stats
);

/**
 * @brief Reset barrier statistics
 * 
 * @param barrier Barrier to reset
 */
void cymatic_barrier_reset_stats(CymaticBarrier* barrier);

/**
 * @brief Print barrier statistics
 * 
 * @param barrier Barrier to print stats for
 */
void cymatic_barrier_print_stats(const CymaticBarrier* barrier);

/**
 * @brief Set barrier frequency
 * 
 * @param barrier Barrier to update
 * @param frequency_type New frequency type
 * @return 0 on success, -1 on error
 */
int cymatic_barrier_set_frequency(
    CymaticBarrier* barrier,
    CymaticFrequencyType frequency_type
);

/**
 * @brief Set custom barrier frequency
 * 
 * @param barrier Barrier to update
 * @param frequency_hz New frequency in Hz
 * @return 0 on success, -1 on error
 */
int cymatic_barrier_set_custom_frequency(
    CymaticBarrier* barrier,
    double frequency_hz
);

/**
 * @brief Calculate time difference in nanoseconds
 * 
 * @param end End time
 * @param start Start time
 * @return Difference in nanoseconds
 */
uint64_t timespec_diff_ns(const struct timespec* end, const struct timespec* start);

/**
 * @brief Convert nanoseconds to timespec
 * 
 * @param ns Nanoseconds
 * @return Timespec structure
 */
struct timespec ns_to_timespec(uint64_t ns);

/**
 * @brief Add nanoseconds to timespec
 * 
 * @param ts Timespec to modify
 * @param ns Nanoseconds to add
 * @return Modified timespec
 */
struct timespec timespec_add_ns(const struct timespec* ts, uint64_t ns);

#ifdef __cplusplus
}
#endif

#endif /* CLLM_CYMATIC_SYNC_H */
