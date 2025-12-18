/**
 * @file work_distribution.c
 * @brief Implementation of Generic Lock-Free Work Distribution System
 */

#include "work_distribution.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdatomic.h>

// ============================================================================
// ATOMIC OPERATIONS
// ============================================================================

static inline bool cas_ptr(void** ptr, void* expected, void* new_val) {
    return atomic_compare_exchange_strong((_Atomic(void*)*)ptr, &expected, new_val);
}

static inline uint64_t atomic_inc(uint64_t* ptr) {
    return atomic_fetch_add((_Atomic uint64_t*)ptr, 1);
}

static inline uint64_t atomic_load_u64(uint64_t* ptr) {
    return atomic_load((_Atomic uint64_t*)ptr);
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

uint64_t work_get_timestamp(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

const char* work_priority_name(WorkPriority priority) {
    switch (priority) {
        case WORK_PRIORITY_LOW: return "LOW";
        case WORK_PRIORITY_NORMAL: return "NORMAL";
        case WORK_PRIORITY_HIGH: return "HIGH";
        case WORK_PRIORITY_CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

const char* work_status_name(WorkStatus status) {
    switch (status) {
        case WORK_STATUS_PENDING: return "PENDING";
        case WORK_STATUS_ASSIGNED: return "ASSIGNED";
        case WORK_STATUS_RUNNING: return "RUNNING";
        case WORK_STATUS_COMPLETED: return "COMPLETED";
        case WORK_STATUS_FAILED: return "FAILED";
        case WORK_STATUS_CANCELLED: return "CANCELLED";
        default: return "UNKNOWN";
    }
}

// ============================================================================
// WORK QUEUE IMPLEMENTATION
// ============================================================================

static void work_queue_init(WorkQueue* queue) {
    if (!queue) return;
    
    // Create sentinel node
    WorkQueueNode* sentinel = (WorkQueueNode*)calloc(1, sizeof(WorkQueueNode));
    sentinel->item = NULL;
    sentinel->next = NULL;
    sentinel->version = 0;
    
    queue->head = sentinel;
    queue->tail = sentinel;
    queue->version_counter = 1;
    queue->enqueue_count = 0;
    queue->dequeue_count = 0;
    queue->steal_count = 0;
    queue->current_size = 0;
    queue->peak_size = 0;
}

static bool work_queue_enqueue(WorkQueue* queue, WorkItem* item) {
    if (!queue || !item) return false;
    
    // Create new node
    WorkQueueNode* node = (WorkQueueNode*)calloc(1, sizeof(WorkQueueNode));
    if (!node) return false;
    
    node->item = item;
    node->next = NULL;
    node->version = atomic_inc(&queue->version_counter);
    
    // Lock-free enqueue
    while (true) {
        WorkQueueNode* tail = queue->tail;
        WorkQueueNode* next = tail->next;
        
        if (tail == queue->tail) {
            if (next == NULL) {
                if (cas_ptr((void**)&tail->next, NULL, node)) {
                    cas_ptr((void**)&queue->tail, tail, node);
                    atomic_inc(&queue->enqueue_count);
                    atomic_inc(&queue->current_size);
                    
                    uint64_t size = atomic_load_u64(&queue->current_size);
                    if (size > queue->peak_size) {
                        queue->peak_size = size;
                    }
                    return true;
                }
            } else {
                cas_ptr((void**)&queue->tail, tail, next);
            }
        }
    }
}

static WorkItem* work_queue_dequeue(WorkQueue* queue) {
    if (!queue) return NULL;
    
    while (true) {
        WorkQueueNode* head = queue->head;
        WorkQueueNode* tail = queue->tail;
        WorkQueueNode* next = head->next;
        
        if (head == queue->head) {
            if (head == tail) {
                if (next == NULL) {
                    return NULL;  // Queue is empty
                }
                cas_ptr((void**)&queue->tail, tail, next);
            } else {
                WorkItem* item = next->item;
                
                if (cas_ptr((void**)&queue->head, head, next)) {
                    atomic_inc(&queue->dequeue_count);
                    
                    uint64_t size = atomic_load_u64(&queue->current_size);
                    if (size > 0) {
                        atomic_fetch_sub((_Atomic uint64_t*)&queue->current_size, 1);
                    }
                    
                    free(head);
                    return item;
                }
            }
        }
    }
}

static uint64_t work_queue_size(WorkQueue* queue) {
    if (!queue) return 0;
    return atomic_load_u64(&queue->current_size);
}

// ============================================================================
// WORK POOL IMPLEMENTATION
// ============================================================================

static WorkPool* work_pool_create(uint32_t worker_id) {
    WorkPool* pool = (WorkPool*)calloc(1, sizeof(WorkPool));
    if (!pool) return NULL;
    
    pool->worker_id = worker_id;
    work_queue_init(&pool->local_queue);
    
    return pool;
}

static void work_pool_destroy(WorkPool* pool) {
    if (!pool) return;
    
    // Drain queue
    while (work_queue_dequeue(&pool->local_queue) != NULL);
    
    free(pool);
}

// ============================================================================
// WORK DISTRIBUTOR IMPLEMENTATION
// ============================================================================

WorkDistributor* work_distributor_create(uint32_t num_workers, uint64_t pool_size) {
    WorkDistributor* dist = (WorkDistributor*)calloc(1, sizeof(WorkDistributor));
    if (!dist) return NULL;
    
    dist->num_workers = num_workers;
    dist->pool_size = pool_size;
    
    // Create worker pools
    dist->pools = (WorkPool**)calloc(num_workers, sizeof(WorkPool*));
    if (!dist->pools) {
        free(dist);
        return NULL;
    }
    
    for (uint32_t i = 0; i < num_workers; i++) {
        dist->pools[i] = work_pool_create(i);
        if (!dist->pools[i]) {
            for (uint32_t j = 0; j < i; j++) {
                work_pool_destroy(dist->pools[j]);
            }
            free(dist->pools);
            free(dist);
            return NULL;
        }
    }
    
    // Initialize global queue
    work_queue_init(&dist->global_queue);
    
    // Pre-allocate work items
    for (uint64_t i = 0; i < pool_size; i++) {
        WorkItem* item = (WorkItem*)calloc(1, sizeof(WorkItem));
        if (item) {
            item->next = dist->free_list;
            dist->free_list = item;
        }
    }
    
    return dist;
}

void work_distributor_destroy(WorkDistributor* distributor) {
    if (!distributor) return;
    
    // Destroy worker pools
    for (uint32_t i = 0; i < distributor->num_workers; i++) {
        work_pool_destroy(distributor->pools[i]);
    }
    free(distributor->pools);
    
    // Free work items
    WorkItem* current = distributor->free_list;
    while (current) {
        WorkItem* next = current->next;
        free(current);
        current = next;
    }
    
    free(distributor);
}

static WorkItem* work_alloc(WorkDistributor* distributor) {
    if (!distributor) return NULL;
    
    WorkItem* item = distributor->free_list;
    if (item) {
        distributor->free_list = item->next;
        atomic_inc(&distributor->allocated);
    } else {
        item = (WorkItem*)calloc(1, sizeof(WorkItem));
    }
    
    if (item) {
        memset(item, 0, sizeof(WorkItem));
        item->id = atomic_inc(&distributor->total_work_items);
        item->submit_time = work_get_timestamp();
    }
    
    return item;
}

static void work_free(WorkDistributor* distributor, WorkItem* item) {
    if (!distributor || !item) return;
    
    // Don't free data - caller is responsible for managing their own data
    // The work item just holds a pointer to it
    
    item->data = NULL;  // Clear pointer
    item->next = distributor->free_list;
    distributor->free_list = item;
    atomic_inc(&distributor->freed);
}

bool work_submit(
    WorkDistributor* distributor,
    uint32_t worker_id,
    void (*work_fn)(void* data),
    void* data,
    size_t data_size,
    WorkPriority priority
) {
    if (!distributor || worker_id >= distributor->num_workers || !work_fn) {
        return false;
    }
    
    WorkItem* item = work_alloc(distributor);
    if (!item) return false;
    
    item->priority = priority;
    item->status = WORK_STATUS_PENDING;
    item->work_fn = work_fn;
    item->data = data;
    item->data_size = data_size;
    item->owner_id = worker_id;
    
    WorkPool* pool = distributor->pools[worker_id];
    return work_queue_enqueue(&pool->local_queue, item);
}

bool work_submit_global(
    WorkDistributor* distributor,
    void (*work_fn)(void* data),
    void* data,
    size_t data_size,
    WorkPriority priority
) {
    if (!distributor || !work_fn) return false;
    
    WorkItem* item = work_alloc(distributor);
    if (!item) return false;
    
    item->priority = priority;
    item->status = WORK_STATUS_PENDING;
    item->work_fn = work_fn;
    item->data = data;
    item->data_size = data_size;
    item->owner_id = UINT32_MAX;  // No specific owner
    
    return work_queue_enqueue(&distributor->global_queue, item);
}

WorkItem* work_get(WorkDistributor* distributor, uint32_t worker_id) {
    if (!distributor || worker_id >= distributor->num_workers) {
        return NULL;
    }
    
    WorkPool* pool = distributor->pools[worker_id];
    
    // Try local queue first
    WorkItem* item = work_queue_dequeue(&pool->local_queue);
    if (item) {
        item->status = WORK_STATUS_ASSIGNED;
        item->worker_id = worker_id;
        atomic_inc(&pool->work_received);
        return item;
    }
    
    // Try global queue
    item = work_queue_dequeue(&distributor->global_queue);
    if (item) {
        item->status = WORK_STATUS_ASSIGNED;
        item->worker_id = worker_id;
        atomic_inc(&pool->work_received);
        return item;
    }
    
    return NULL;
}

WorkItem* work_steal(WorkDistributor* distributor, uint32_t thief_id) {
    if (!distributor || thief_id >= distributor->num_workers) {
        return NULL;
    }
    
    // Try to steal from the most loaded worker
    uint32_t victim_id = work_find_most_loaded_worker(distributor);
    if (victim_id == thief_id || victim_id >= distributor->num_workers) {
        return NULL;
    }
    
    WorkPool* victim_pool = distributor->pools[victim_id];
    WorkItem* item = work_queue_dequeue(&victim_pool->local_queue);
    
    if (item) {
        item->status = WORK_STATUS_ASSIGNED;
        item->worker_id = thief_id;
        
        WorkPool* thief_pool = distributor->pools[thief_id];
        atomic_inc(&thief_pool->work_stolen);
        atomic_inc(&victim_pool->local_queue.steal_count);
        atomic_inc(&distributor->stolen_items);
        
        return item;
    }
    
    return NULL;
}

void work_complete(WorkDistributor* distributor, WorkItem* item) {
    if (!distributor || !item) return;
    
    item->status = WORK_STATUS_COMPLETED;
    item->end_time = work_get_timestamp();
    
    uint64_t work_time = item->end_time - item->start_time;
    
    if (item->worker_id < distributor->num_workers) {
        WorkPool* pool = distributor->pools[item->worker_id];
        atomic_inc(&pool->work_completed);
        atomic_fetch_add((_Atomic uint64_t*)&pool->total_work_time, work_time);
    }
    
    atomic_inc(&distributor->completed_items);
    work_free(distributor, item);
}

void work_fail(WorkDistributor* distributor, WorkItem* item) {
    if (!distributor || !item) return;
    
    item->status = WORK_STATUS_FAILED;
    item->end_time = work_get_timestamp();
    
    atomic_inc(&distributor->failed_items);
    work_free(distributor, item);
}

// ============================================================================
// BATCH OPERATIONS
// ============================================================================

uint32_t work_submit_batch(
    WorkDistributor* distributor,
    uint32_t worker_id,
    void (**work_fns)(void*),
    void** data,
    size_t* data_sizes,
    WorkPriority* priorities,
    uint32_t count
) {
    if (!distributor || !work_fns) return 0;
    
    uint32_t submitted = 0;
    for (uint32_t i = 0; i < count; i++) {
        if (work_submit(distributor, worker_id, work_fns[i], data[i], 
                       data_sizes[i], priorities[i])) {
            submitted++;
        }
    }
    
    return submitted;
}

uint32_t work_get_batch(
    WorkDistributor* distributor,
    uint32_t worker_id,
    WorkItem** out_items,
    uint32_t max_count
) {
    if (!distributor || !out_items) return 0;
    
    uint32_t count = 0;
    for (uint32_t i = 0; i < max_count; i++) {
        WorkItem* item = work_get(distributor, worker_id);
        if (!item) break;
        
        out_items[count++] = item;
    }
    
    return count;
}

// ============================================================================
// LOAD BALANCING
// ============================================================================

uint64_t work_get_worker_load(WorkDistributor* distributor, uint32_t worker_id) {
    if (!distributor || worker_id >= distributor->num_workers) {
        return 0;
    }
    
    WorkPool* pool = distributor->pools[worker_id];
    return work_queue_size(&pool->local_queue);
}

uint64_t work_get_total_load(WorkDistributor* distributor) {
    if (!distributor) return 0;
    
    uint64_t total = work_queue_size(&distributor->global_queue);
    
    for (uint32_t i = 0; i < distributor->num_workers; i++) {
        total += work_get_worker_load(distributor, i);
    }
    
    return total;
}

uint32_t work_find_least_loaded_worker(WorkDistributor* distributor) {
    if (!distributor || distributor->num_workers == 0) {
        return UINT32_MAX;
    }
    
    uint32_t least_loaded = 0;
    uint64_t min_load = work_get_worker_load(distributor, 0);
    
    for (uint32_t i = 1; i < distributor->num_workers; i++) {
        uint64_t load = work_get_worker_load(distributor, i);
        if (load < min_load) {
            min_load = load;
            least_loaded = i;
        }
    }
    
    return least_loaded;
}

uint32_t work_find_most_loaded_worker(WorkDistributor* distributor) {
    if (!distributor || distributor->num_workers == 0) {
        return UINT32_MAX;
    }
    
    uint32_t most_loaded = 0;
    uint64_t max_load = work_get_worker_load(distributor, 0);
    
    for (uint32_t i = 1; i < distributor->num_workers; i++) {
        uint64_t load = work_get_worker_load(distributor, i);
        if (load > max_load) {
            max_load = load;
            most_loaded = i;
        }
    }
    
    return most_loaded;
}

void work_balance_load(WorkDistributor* distributor) {
    if (!distributor) return;
    
    // Simple load balancing: steal from most loaded to least loaded
    uint32_t most = work_find_most_loaded_worker(distributor);
    uint32_t least = work_find_least_loaded_worker(distributor);
    
    if (most == UINT32_MAX || least == UINT32_MAX || most == least) {
        return;
    }
    
    uint64_t most_load = work_get_worker_load(distributor, most);
    uint64_t least_load = work_get_worker_load(distributor, least);
    
    // Only balance if difference is significant
    if (most_load > least_load + 10) {
        // Steal some work
        uint32_t to_steal = (most_load - least_load) / 2;
        for (uint32_t i = 0; i < to_steal && i < 10; i++) {
            WorkItem* item = work_steal(distributor, least);
            if (!item) break;
            
            // Re-submit to least loaded worker
            WorkPool* pool = distributor->pools[least];
            work_queue_enqueue(&pool->local_queue, item);
        }
    }
}

// ============================================================================
// STATISTICS
// ============================================================================

void work_get_worker_stats(
    WorkDistributor* distributor,
    uint32_t worker_id,
    uint64_t* completed,
    uint64_t* stolen,
    uint64_t* received,
    uint64_t* avg_time
) {
    if (!distributor || worker_id >= distributor->num_workers) return;
    
    WorkPool* pool = distributor->pools[worker_id];
    
    if (completed) *completed = pool->work_completed;
    if (stolen) *stolen = pool->work_stolen;
    if (received) *received = pool->work_received;
    if (avg_time) {
        *avg_time = pool->work_completed > 0 ? 
                   pool->total_work_time / pool->work_completed : 0;
    }
}

void work_print_worker_stats(WorkDistributor* distributor, uint32_t worker_id) {
    if (!distributor || worker_id >= distributor->num_workers) return;
    
    WorkPool* pool = distributor->pools[worker_id];
    
    printf("\n=== Worker %u Statistics ===\n", worker_id);
    printf("Queue Size: %lu\n", work_queue_size(&pool->local_queue));
    printf("Work Completed: %lu\n", pool->work_completed);
    printf("Work Stolen: %lu\n", pool->work_stolen);
    printf("Work Received: %lu\n", pool->work_received);
    printf("Avg Work Time: %lu ns\n", 
           pool->work_completed > 0 ? pool->total_work_time / pool->work_completed : 0);
    printf("============================\n\n");
}

void work_print_system_stats(WorkDistributor* distributor) {
    if (!distributor) return;
    
    printf("\n=== Work Distribution System Statistics ===\n");
    printf("Workers: %u\n", distributor->num_workers);
    printf("Total Work Items: %lu\n", distributor->total_work_items);
    printf("Completed Items: %lu\n", distributor->completed_items);
    printf("Failed Items: %lu\n", distributor->failed_items);
    printf("Stolen Items: %lu\n", distributor->stolen_items);
    printf("Global Queue Size: %lu\n", work_queue_size(&distributor->global_queue));
    printf("Total System Load: %lu\n", work_get_total_load(distributor));
    printf("Pool Allocated: %lu\n", distributor->allocated);
    printf("Pool Freed: %lu\n", distributor->freed);
    printf("===========================================\n\n");
}