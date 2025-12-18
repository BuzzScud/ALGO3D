#ifndef CLLM_SPHERE_STATS_H
#define CLLM_SPHERE_STATS_H

#include <stdint.h>
#include <stdatomic.h>
#include <time.h>

/**
 * Sphere Statistics
 * 
 * Thread-safe statistics tracking for each sphere in the hierarchy.
 * Uses atomic operations for lock-free updates.
 */

typedef struct {
    // Basic counters (atomic for thread safety)
    atomic_uint_fast64_t primes_processed;      // Total primes processed
    atomic_uint_fast64_t batches_completed;     // Total batches completed
    atomic_uint_fast64_t gradients_computed;    // Total gradients computed
    atomic_uint_fast64_t weights_updated;       // Total weight updates
    
    // Symmetry group tracking
    int symmetry_group;                         // Which symmetry group (0-11)
    atomic_uint_fast64_t primes_per_group[12];  // Primes processed per group
    
    // 144000 boundary tracking
    atomic_uint_fast64_t boundary_crossings;    // Times crossed 144000
    atomic_uint_fast64_t twin_prime_hits;       // Times hit 143999 or 144001
    atomic_uint_fast64_t near_boundary_primes;  // Primes within 100 of 144000
    
    // Performance metrics
    atomic_uint_fast64_t total_time_ns;         // Total processing time (nanoseconds)
    atomic_uint_fast64_t idle_time_ns;          // Time spent idle
    atomic_uint_fast64_t sync_time_ns;          // Time spent in synchronization
    
    // Work stealing statistics
    atomic_uint_fast64_t work_stolen_from;      // Times work was stolen from this sphere
    atomic_uint_fast64_t work_stolen_to;        // Times this sphere stole work
    atomic_uint_fast64_t work_items_stolen;     // Total work items stolen
    
    // Memory statistics
    atomic_uint_fast64_t cache_hits;            // Prime cache hits
    atomic_uint_fast64_t cache_misses;          // Prime cache misses
    atomic_uint_fast64_t memory_allocated;      // Total memory allocated (bytes)
    
    // Hierarchy statistics
    int hierarchy_level;                        // Level in hierarchy (0 = root)
    int num_children;                           // Number of child spheres
    atomic_uint_fast64_t messages_sent;         // Messages sent to siblings
    atomic_uint_fast64_t messages_received;     // Messages received from siblings
    
    // Error tracking
    atomic_uint_fast64_t errors_encountered;    // Total errors
    atomic_uint_fast64_t retries_attempted;     // Total retry attempts
    
    // Timestamps
    struct timespec creation_time;              // When sphere was created
    struct timespec last_update_time;           // Last statistics update
    
} SphereStatistics;

/**
 * Initialize sphere statistics
 * 
 * @param stats Pointer to statistics structure
 * @param symmetry_group Symmetry group (0-11)
 * @param hierarchy_level Level in hierarchy (0 = root)
 */
void cllm_sphere_stats_init(SphereStatistics* stats, 
                            int symmetry_group,
                            int hierarchy_level);

/**
 * Update prime processing statistics
 * 
 * @param stats Pointer to statistics structure
 * @param prime The prime that was processed
 * @param processing_time_ns Time taken to process (nanoseconds)
 */
void cllm_sphere_stats_record_prime(SphereStatistics* stats,
                                    uint64_t prime,
                                    uint64_t processing_time_ns);

/**
 * Update batch completion statistics
 * 
 * @param stats Pointer to statistics structure
 * @param batch_size Number of items in batch
 * @param batch_time_ns Time taken for batch (nanoseconds)
 */
void cllm_sphere_stats_record_batch(SphereStatistics* stats,
                                    uint64_t batch_size,
                                    uint64_t batch_time_ns);

/**
 * Update gradient computation statistics
 * 
 * @param stats Pointer to statistics structure
 * @param num_gradients Number of gradients computed
 */
void cllm_sphere_stats_record_gradients(SphereStatistics* stats,
                                        uint64_t num_gradients);

/**
 * Update weight update statistics
 * 
 * @param stats Pointer to statistics structure
 * @param num_weights Number of weights updated
 */
void cllm_sphere_stats_record_weights(SphereStatistics* stats,
                                      uint64_t num_weights);

/**
 * Record 144000 boundary crossing
 * 
 * @param stats Pointer to statistics structure
 * @param prime The prime that crossed the boundary
 */
void cllm_sphere_stats_record_boundary_crossing(SphereStatistics* stats,
                                                uint64_t prime);

/**
 * Record work stealing event
 * 
 * @param stats Pointer to statistics structure
 * @param stolen_from True if work was stolen FROM this sphere
 * @param num_items Number of work items stolen
 */
void cllm_sphere_stats_record_work_stealing(SphereStatistics* stats,
                                            int stolen_from,
                                            uint64_t num_items);

/**
 * Record cache access
 * 
 * @param stats Pointer to statistics structure
 * @param hit True if cache hit, false if miss
 */
void cllm_sphere_stats_record_cache_access(SphereStatistics* stats,
                                           int hit);

/**
 * Record message passing
 * 
 * @param stats Pointer to statistics structure
 * @param sent True if message sent, false if received
 */
void cllm_sphere_stats_record_message(SphereStatistics* stats,
                                      int sent);

/**
 * Record error
 * 
 * @param stats Pointer to statistics structure
 * @param retry True if this is a retry attempt
 */
void cllm_sphere_stats_record_error(SphereStatistics* stats,
                                    int retry);

/**
 * Record idle time
 * 
 * @param stats Pointer to statistics structure
 * @param idle_time_ns Time spent idle (nanoseconds)
 */
void cllm_sphere_stats_record_idle_time(SphereStatistics* stats,
                                        uint64_t idle_time_ns);

/**
 * Record synchronization time
 * 
 * @param stats Pointer to statistics structure
 * @param sync_time_ns Time spent in synchronization (nanoseconds)
 */
void cllm_sphere_stats_record_sync_time(SphereStatistics* stats,
                                        uint64_t sync_time_ns);

/**
 * Print sphere statistics
 * 
 * @param stats Pointer to statistics structure
 * @param sphere_id Identifier for this sphere
 */
void cllm_sphere_stats_print(const SphereStatistics* stats,
                             int sphere_id);

/**
 * Print detailed sphere statistics
 * 
 * @param stats Pointer to statistics structure
 * @param sphere_id Identifier for this sphere
 */
void cllm_sphere_stats_print_detailed(const SphereStatistics* stats,
                                      int sphere_id);

/**
 * Get cache hit rate
 * 
 * @param stats Pointer to statistics structure
 * @return Cache hit rate (0.0 to 1.0)
 */
double cllm_sphere_stats_get_cache_hit_rate(const SphereStatistics* stats);

/**
 * Get average processing time per prime
 * 
 * @param stats Pointer to statistics structure
 * @return Average time in nanoseconds
 */
double cllm_sphere_stats_get_avg_prime_time(const SphereStatistics* stats);

/**
 * Get utilization (non-idle time / total time)
 * 
 * @param stats Pointer to statistics structure
 * @return Utilization rate (0.0 to 1.0)
 */
double cllm_sphere_stats_get_utilization(const SphereStatistics* stats);

/**
 * Reset statistics (for new epoch)
 * 
 * @param stats Pointer to statistics structure
 */
void cllm_sphere_stats_reset(SphereStatistics* stats);

/**
 * Merge statistics from child sphere
 * 
 * @param parent Parent sphere statistics
 * @param child Child sphere statistics
 */
void cllm_sphere_stats_merge(SphereStatistics* parent,
                             const SphereStatistics* child);

#endif // CLLM_SPHERE_STATS_H
