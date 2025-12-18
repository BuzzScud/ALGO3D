/**
 * @file abacus88d_threading.c
 * @brief Implementation of Abacus88D threading integration
 */

#include "abacus88d_threading.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* ============================================================================
 * HELPER FUNCTIONS
 * ============================================================================
 */

/**
 * @brief Get current time in seconds
 */
static double get_time_seconds(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

/**
 * @brief Worker thread function
 */
static void* worker_thread_func(void* arg) {
    HierarchicalThread* thread = (HierarchicalThread*)arg;
    // TODO: Get pool reference - for now this is a placeholder
    Abacus88DThreadPool* pool = NULL;
    if (!pool) return NULL;
    
    while (thread->running && !thread->should_stop) {
        // Get work from pool
        WorkItem* work = hierarchical_thread_get_work(thread);
        
        if (work) {
            double start_time = get_time_seconds();
            
            Abacus88DWorkItem* abacus_work = (Abacus88DWorkItem*)work->data;
            
            // Execute work based on type
            switch (abacus_work->type) {
                case ABACUS88D_WORK_BOUNDARY_DETECT: {
                    BoundaryInfo* boundary = (BoundaryInfo*)abacus_work->result;
                    CrystallineAbacus* value = (CrystallineAbacus*)abacus_work->data;
                    
                    geometric_detect_boundary_threadsafe(
                        pool->abacus,
                        abacus_work->layer,
                        abacus_work->dimension,
                        value,
                        boundary
                    );
                    
                    __sync_fetch_and_add(&pool->boundary_detections, 1);
                    break;
                }
                
                case ABACUS88D_WORK_TRANSFORM: {
                    TransformMatrix* transform = (TransformMatrix*)abacus_work->data;
                    CrystallineAbacus** values = (CrystallineAbacus**)abacus_work->result;
                    
                    geometric_apply_transform_threadsafe(
                        pool->abacus,
                        transform,
                        values[0],  // input
                        values[1]   // output
                    );
                    
                    __sync_fetch_and_add(&pool->transformations, 1);
                    break;
                }
                
                case ABACUS88D_WORK_HANDOFF: {
                    HandoffContext* context = (HandoffContext*)abacus_work->data;
                    CrystallineAbacus* value = (CrystallineAbacus*)abacus_work->result;
                    
                    geometric_execute_handoff_threadsafe(
                        pool->abacus,
                        pool->abacus,
                        context,
                        value
                    );
                    
                    __sync_fetch_and_add(&pool->handoffs, 1);
                    break;
                }
                
                case ABACUS88D_WORK_COMPUTE:
                case ABACUS88D_WORK_SYNC:
                    // Custom work types - handled by user callback
                    break;
            }
            
            double end_time = get_time_seconds();
            double work_time = end_time - start_time;
            
            // Update average work time (exponential moving average)
            pool->avg_work_time = 0.9 * pool->avg_work_time + 0.1 * work_time;
            
            // Mark work as complete
            hierarchical_thread_complete_work(thread, work->work_id, 0);
            
            __sync_fetch_and_add(&pool->work_completed, 1);
        } else {
            // No work available, sleep briefly
            usleep(100);
        }
    }
    
    return NULL;
}

/* ============================================================================
 * THREAD POOL MANAGEMENT
 * ============================================================================
 */

Abacus88DThreadPool* abacus88d_thread_pool_create(
    Abacus88D* abacus,
    uint32_t num_threads
) {
    if (!abacus) return NULL;
    
    // Auto-detect thread count if not specified
    if (num_threads == 0) {
        num_threads = ABACUS88D_NUM_LAYERS;  // One thread per layer
    }
    
    Abacus88DThreadPool* pool = calloc(1, sizeof(Abacus88DThreadPool));
    if (!pool) return NULL;
    
    pool->abacus = abacus;
    pool->num_threads = num_threads;
    pool->running = false;
    
    // Create underlying thread pool
    pool->thread_pool = hierarchical_thread_pool_create(
        num_threads,
        12,  // 12-fold symmetry
        NULL // No parent pool
    );
    
    if (!pool->thread_pool) {
        free(pool);
        return NULL;
    }
    
    // Initialize work pools array (will be populated when threads are created)
    for (int i = 0; i < ABACUS88D_NUM_LAYERS; i++) {
        pool->work_pools[i] = NULL;
    }
    
    // Initialize synchronization
    pthread_mutex_init(&pool->pool_mutex, NULL);
    pthread_cond_init(&pool->work_available, NULL);
    
    return pool;
}

void abacus88d_thread_pool_free(Abacus88DThreadPool* pool) {
    if (!pool) return;
    
    // Stop pool if running
    if (pool->running) {
        abacus88d_thread_pool_stop(pool);
    }
    
    // Work pools are managed by hierarchical threading system
    // No need to free them explicitly
    
    // Free thread pool
    if (pool->thread_pool) {
        hierarchical_thread_pool_free(pool->thread_pool);
    }
    
    // Destroy synchronization
    pthread_mutex_destroy(&pool->pool_mutex);
    pthread_cond_destroy(&pool->work_available);
    
    free(pool);
}

int abacus88d_thread_pool_start(Abacus88DThreadPool* pool) {
    if (!pool || pool->running) return -1;
    
    // Start underlying thread pool
    int result = hierarchical_thread_pool_start(pool->thread_pool);
    if (result != 0) return result;
    
    pool->running = true;
    return 0;
}

int abacus88d_thread_pool_stop(Abacus88DThreadPool* pool) {
    if (!pool || !pool->running) return -1;
    
    pool->running = false;
    
    // Stop underlying thread pool
    return hierarchical_thread_pool_stop(pool->thread_pool);
}

int abacus88d_thread_pool_wait(Abacus88DThreadPool* pool) {
    if (!pool) return -1;
    
    // Wait for all work to complete
    pthread_mutex_lock(&pool->pool_mutex);
    
    while (pool->work_submitted > pool->work_completed) {
        pthread_cond_wait(&pool->work_available, &pool->pool_mutex);
    }
    
    pthread_mutex_unlock(&pool->pool_mutex);
    
    return 0;
}

/* ============================================================================
 * WORK SUBMISSION
 * ============================================================================
 */

uint64_t abacus88d_submit_boundary_detection(
    Abacus88DThreadPool* pool,
    uint8_t layer,
    uint8_t dimension,
    const CrystallineAbacus* value,
    BoundaryInfo* boundary
) {
    if (!pool || !value || !boundary) return 0;
    if (layer >= ABACUS88D_NUM_LAYERS) return 0;
    if (dimension >= ABACUS88D_DIMS_PER_LAYER) return 0;
    
    // Create work item
    Abacus88DWorkItem* work_item = malloc(sizeof(Abacus88DWorkItem));
    if (!work_item) return 0;
    
    work_item->type = ABACUS88D_WORK_BOUNDARY_DETECT;
    work_item->layer = layer;
    work_item->dimension = dimension;
    work_item->priority = 1;
    work_item->data = (void*)value;
    work_item->result = boundary;
    
    // Submit work through hierarchical threading system
    uint64_t work_id = __sync_fetch_and_add(&pool->work_submitted, 1);
    
    // Use the thread pool's work submission
    if (pool->layer_threads[layer]) {
        hierarchical_thread_submit_work(
            pool->layer_threads[layer],
            work_item,
            sizeof(Abacus88DWorkItem),
            work_item->priority
        );
    }
    
    pthread_cond_signal(&pool->work_available);
    
    return work_id;
}

uint64_t abacus88d_submit_transformation(
    Abacus88DThreadPool* pool,
    const TransformMatrix* transform,
    const CrystallineAbacus* input,
    CrystallineAbacus* output
) {
    if (!pool || !transform || !input || !output) return 0;
    
    // Create work item
    Abacus88DWorkItem* work_item = malloc(sizeof(Abacus88DWorkItem));
    if (!work_item) return 0;
    
    work_item->type = ABACUS88D_WORK_TRANSFORM;
    work_item->layer = transform->source_layer;
    work_item->dimension = 0;  // Not dimension-specific
    work_item->priority = 1;
    work_item->data = (void*)transform;
    
    // Store input/output pointers
    CrystallineAbacus** values = malloc(2 * sizeof(CrystallineAbacus*));
    values[0] = (CrystallineAbacus*)input;
    values[1] = output;
    work_item->result = values;
    
    // Submit work through hierarchical threading system
    uint64_t work_id = __sync_fetch_and_add(&pool->work_submitted, 1);
    
    if (pool->layer_threads[transform->source_layer]) {
        hierarchical_thread_submit_work(
            pool->layer_threads[transform->source_layer],
            work_item,
            sizeof(Abacus88DWorkItem),
            work_item->priority
        );
    }
    
    pthread_cond_signal(&pool->work_available);
    
    return work_id;
}

uint64_t abacus88d_submit_handoff(
    Abacus88DThreadPool* pool,
    const HandoffContext* context,
    const CrystallineAbacus* value
) {
    if (!pool || !context || !value) return 0;
    
    // Create work item
    Abacus88DWorkItem* work_item = malloc(sizeof(Abacus88DWorkItem));
    if (!work_item) return 0;
    
    work_item->type = ABACUS88D_WORK_HANDOFF;
    work_item->layer = context->source_layer;
    work_item->dimension = context->source_dimension;
    work_item->priority = 2;  // Higher priority for handoffs
    work_item->data = (void*)context;
    work_item->result = (void*)value;
    
    // Submit work through hierarchical threading system
    uint64_t work_id = __sync_fetch_and_add(&pool->work_submitted, 1);
    
    if (pool->layer_threads[context->source_layer]) {
        hierarchical_thread_submit_work(
            pool->layer_threads[context->source_layer],
            work_item,
            sizeof(Abacus88DWorkItem),
            work_item->priority
        );
    }
    
    pthread_cond_signal(&pool->work_available);
    
    return work_id;
}

uint32_t abacus88d_submit_batch(
    Abacus88DThreadPool* pool,
    const Abacus88DWorkItem* items,
    uint32_t count
) {
    if (!pool || !items) return 0;
    
    uint32_t submitted = 0;
    
    for (uint32_t i = 0; i < count; i++) {
        uint64_t work_id = 0;
        
        switch (items[i].type) {
            case ABACUS88D_WORK_BOUNDARY_DETECT:
                work_id = abacus88d_submit_boundary_detection(
                    pool,
                    items[i].layer,
                    items[i].dimension,
                    (CrystallineAbacus*)items[i].data,
                    (BoundaryInfo*)items[i].result
                );
                break;
                
            case ABACUS88D_WORK_TRANSFORM:
                // Not implemented for batch yet
                break;
                
            case ABACUS88D_WORK_HANDOFF:
                work_id = abacus88d_submit_handoff(
                    pool,
                    (HandoffContext*)items[i].data,
                    (CrystallineAbacus*)items[i].result
                );
                break;
                
            default:
                break;
        }
        
        if (work_id > 0) {
            submitted++;
        }
    }
    
    return submitted;
}

/* ============================================================================
 * WORK STEALING
 * ============================================================================
 */

int abacus88d_set_work_stealing(
    Abacus88DThreadPool* pool,
    bool enabled
) {
    if (!pool) return -1;
    
    // Enable/disable work stealing in underlying thread pool
    for (uint32_t i = 0; i < pool->num_threads; i++) {
        if (pool->layer_threads[i]) {
            pool->layer_threads[i]->work_pool->allow_stealing = enabled;
        }
    }
    
    return 0;
}

int abacus88d_get_stealing_stats(
    Abacus88DThreadPool* pool,
    uint64_t* stolen_count,
    uint64_t* steal_attempts
) {
    if (!pool || !stolen_count || !steal_attempts) return -1;
    
    *stolen_count = pool->work_stolen;
    *steal_attempts = 0;  // TODO: Track steal attempts
    
    return 0;
}

/* ============================================================================
 * LOAD BALANCING
 * ============================================================================
 */

int abacus88d_balance_load(Abacus88DThreadPool* pool) {
    if (!pool) return -1;
    
    // Load balancing is handled automatically by the hierarchical threading system
    // through work stealing. This function is a no-op for now.
    
    return 0;
}

int abacus88d_get_load_stats(
    Abacus88DThreadPool* pool,
    uint64_t* loads,
    uint32_t count
) {
    if (!pool || !loads) return -1;
    
    uint32_t max_count = count < ABACUS88D_NUM_LAYERS ? count : ABACUS88D_NUM_LAYERS;
    
    // Get work completed per thread as a proxy for load
    for (uint32_t i = 0; i < max_count; i++) {
        if (pool->layer_threads[i]) {
            loads[i] = pool->layer_threads[i]->work_completed;
        } else {
            loads[i] = 0;
        }
    }
    
    return 0;
}

/* ============================================================================
 * PERFORMANCE MONITORING
 * ============================================================================
 */

int abacus88d_get_performance_stats(
    Abacus88DThreadPool* pool,
    Abacus88DPerformanceStats* stats
) {
    if (!pool || !stats) return -1;
    
    stats->work_submitted = pool->work_submitted;
    stats->work_completed = pool->work_completed;
    stats->work_stolen = pool->work_stolen;
    stats->boundary_detections = pool->boundary_detections;
    stats->transformations = pool->transformations;
    stats->handoffs = pool->handoffs;
    stats->avg_work_time = pool->avg_work_time;
    
    // Calculate throughput
    if (pool->avg_work_time > 0) {
        stats->throughput = 1.0 / pool->avg_work_time;
    } else {
        stats->throughput = 0.0;
    }
    
    // Count active threads
    stats->active_threads = 0;
    for (uint32_t i = 0; i < pool->num_threads; i++) {
        if (pool->layer_threads[i] && pool->layer_threads[i]->running) {
            stats->active_threads++;
        }
    }
    
    return 0;
}

void abacus88d_print_performance_stats(Abacus88DThreadPool* pool) {
    if (!pool) return;
    
    Abacus88DPerformanceStats stats;
    abacus88d_get_performance_stats(pool, &stats);
    
    printf("\n========================================\n");
    printf("Abacus88D Performance Statistics\n");
    printf("========================================\n");
    printf("Work Submitted:       %lu\n", stats.work_submitted);
    printf("Work Completed:       %lu\n", stats.work_completed);
    printf("Work Stolen:          %lu\n", stats.work_stolen);
    printf("Boundary Detections:  %lu\n", stats.boundary_detections);
    printf("Transformations:      %lu\n", stats.transformations);
    printf("Handoffs:             %lu\n", stats.handoffs);
    printf("Avg Work Time:        %.6f seconds\n", stats.avg_work_time);
    printf("Throughput:           %.2f items/second\n", stats.throughput);
    printf("Active Threads:       %u\n", stats.active_threads);
    printf("========================================\n\n");
}

int abacus88d_reset_performance_stats(Abacus88DThreadPool* pool) {
    if (!pool) return -1;
    
    pool->work_submitted = 0;
    pool->work_completed = 0;
    pool->work_stolen = 0;
    pool->boundary_detections = 0;
    pool->transformations = 0;
    pool->handoffs = 0;
    pool->avg_work_time = 0.0;
    
    return 0;
}