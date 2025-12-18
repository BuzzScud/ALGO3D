/**
 * @file work_distribution.h
 * @brief Generic Lock-Free Work Distribution System
 * 
 * A universal work distribution system with:
 * - Lock-free work queues
 * - Work stealing algorithm
 * - Batch prefetching
 * - Load balancing
 * - Dynamic work allocation
 * 
 * Design Philosophy:
 * - Lock-free for maximum concurrency
 * - Work stealing for load balancing
 * - Batch operations for efficiency
 * - Minimal contention
 * - Scalable to many threads
 */

#ifndef WORK_DISTRIBUTION_H
#define WORK_DISTRIBUTION_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// WORK ITEM DEFINITIONS
// ============================================================================

/**
 * Work item priority
 */
typedef enum {
    WORK_PRIORITY_LOW = 0,
    WORK_PRIORITY_NORMAL = 1,
    WORK_PRIORITY_HIGH = 2,
    WORK_PRIORITY_CRITICAL = 3
} WorkPriority;

/**
 * Work item status
 */
typedef enum {
    WORK_STATUS_PENDING = 0,
    WORK_STATUS_ASSIGNED = 1,
    WORK_STATUS_RUNNING = 2,
    WORK_STATUS_COMPLETED = 3,
    WORK_STATUS_FAILED = 4,
    WORK_STATUS_CANCELLED = 5
} WorkStatus;

/**
 * Work item
 */
typedef struct WorkItem {
    uint64_t id;
    WorkPriority priority;
    WorkStatus status;
    
    // Work function
    void (*work_fn)(void* data);
    void* data;
    size_t data_size;
    
    // Ownership
    uint32_t owner_id;
    uint32_t worker_id;
    
    // Timing
    uint64_t submit_time;
    uint64_t start_time;
    uint64_t end_time;
    
    // Queue management
    struct WorkItem* next;
    uint64_t version;  // ABA prevention
} WorkItem;

// ============================================================================
// WORK QUEUE STRUCTURES
// ============================================================================

/**
 * Lock-free work queue node
 */
typedef struct WorkQueueNode {
    WorkItem* item;
    struct WorkQueueNode* next;
    uint64_t version;
} WorkQueueNode;

/**
 * Lock-free work queue
 */
typedef struct WorkQueue {
    WorkQueueNode* head;
    WorkQueueNode* tail;
    uint64_t version_counter;
    
    // Statistics
    uint64_t enqueue_count;
    uint64_t dequeue_count;
    uint64_t steal_count;
    uint64_t current_size;
    uint64_t peak_size;
} WorkQueue;

/**
 * Work pool (per-worker queue)
 */
typedef struct WorkPool {
    uint32_t worker_id;
    WorkQueue local_queue;
    
    // Statistics
    uint64_t work_completed;
    uint64_t work_stolen;
    uint64_t work_received;
    uint64_t total_work_time;
} WorkPool;

// ============================================================================
// WORK DISTRIBUTION SYSTEM
// ============================================================================

/**
 * Work distribution system
 */
typedef struct WorkDistributor {
    WorkPool** pools;
    uint32_t num_workers;
    
    // Global work queue (for unassigned work)
    WorkQueue global_queue;
    
    // Work item pool
    WorkItem* free_list;
    uint64_t pool_size;
    uint64_t allocated;
    uint64_t freed;
    
    // Statistics
    uint64_t total_work_items;
    uint64_t completed_items;
    uint64_t failed_items;
    uint64_t stolen_items;
} WorkDistributor;

// ============================================================================
// WORK DISTRIBUTOR API
// ============================================================================

/**
 * Create a work distributor
 */
WorkDistributor* work_distributor_create(uint32_t num_workers, uint64_t pool_size);

/**
 * Destroy a work distributor
 */
void work_distributor_destroy(WorkDistributor* distributor);

/**
 * Submit work to a specific worker
 */
bool work_submit(
    WorkDistributor* distributor,
    uint32_t worker_id,
    void (*work_fn)(void* data),
    void* data,
    size_t data_size,
    WorkPriority priority
);

/**
 * Submit work to global queue (any worker can take it)
 */
bool work_submit_global(
    WorkDistributor* distributor,
    void (*work_fn)(void* data),
    void* data,
    size_t data_size,
    WorkPriority priority
);

/**
 * Get work for a worker (from local queue)
 */
WorkItem* work_get(WorkDistributor* distributor, uint32_t worker_id);

/**
 * Try to steal work from another worker
 */
WorkItem* work_steal(WorkDistributor* distributor, uint32_t thief_id);

/**
 * Complete a work item
 */
void work_complete(WorkDistributor* distributor, WorkItem* item);

/**
 * Fail a work item
 */
void work_fail(WorkDistributor* distributor, WorkItem* item);

// ============================================================================
// BATCH OPERATIONS
// ============================================================================

/**
 * Submit multiple work items at once
 */
uint32_t work_submit_batch(
    WorkDistributor* distributor,
    uint32_t worker_id,
    void (**work_fns)(void*),
    void** data,
    size_t* data_sizes,
    WorkPriority* priorities,
    uint32_t count
);

/**
 * Get multiple work items at once
 */
uint32_t work_get_batch(
    WorkDistributor* distributor,
    uint32_t worker_id,
    WorkItem** out_items,
    uint32_t max_count
);

// ============================================================================
// LOAD BALANCING
// ============================================================================

/**
 * Get worker load (number of pending items)
 */
uint64_t work_get_worker_load(WorkDistributor* distributor, uint32_t worker_id);

/**
 * Get total system load
 */
uint64_t work_get_total_load(WorkDistributor* distributor);

/**
 * Find least loaded worker
 */
uint32_t work_find_least_loaded_worker(WorkDistributor* distributor);

/**
 * Find most loaded worker
 */
uint32_t work_find_most_loaded_worker(WorkDistributor* distributor);

/**
 * Balance load across workers
 */
void work_balance_load(WorkDistributor* distributor);

// ============================================================================
// STATISTICS
// ============================================================================

/**
 * Get worker statistics
 */
void work_get_worker_stats(
    WorkDistributor* distributor,
    uint32_t worker_id,
    uint64_t* completed,
    uint64_t* stolen,
    uint64_t* received,
    uint64_t* avg_time
);

/**
 * Print worker statistics
 */
void work_print_worker_stats(WorkDistributor* distributor, uint32_t worker_id);

/**
 * Print system statistics
 */
void work_print_system_stats(WorkDistributor* distributor);

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * Get current timestamp
 */
uint64_t work_get_timestamp(void);

/**
 * Get priority name
 */
const char* work_priority_name(WorkPriority priority);

/**
 * Get status name
 */
const char* work_status_name(WorkStatus status);

#ifdef __cplusplus
}
#endif

#endif // WORK_DISTRIBUTION_H