/*
 * CLLM Metrics System Implementation
 * Real-time metrics collection and reporting
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "ai/cllm_training.h"
#include "ai/cllm_metrics.h"

// Global callback
static CLLMMetricsCallback g_metrics_callback = NULL;
static void* g_metrics_user_data = NULL;

/*
 * Get current timestamp in milliseconds
 */
static uint64_t get_timestamp_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

/*
 * Initialize metrics system
 */
void cllm_metrics_init(CLLMMetrics* metrics) {
    if (!metrics) return;
    
    memset(metrics, 0, sizeof(CLLMMetrics));
    
    // Initialize thread metrics
    for (int i = 0; i < MAX_TRACKED_THREADS; i++) {
        metrics->threads[i].thread_id = -1;
        metrics->threads[i].sphere_id = -1;
        metrics->threads[i].parent_thread_id = -1;
        metrics->threads[i].state = THREAD_STATE_IDLE;
        metrics->threads[i].role = THREAD_ROLE_WORKER;
        metrics->threads[i].hierarchy_level = 0;
    }
    
    // Initialize framework status (all enabled by default)
    metrics->framework.using_lattice_embeddings = true;
    metrics->framework.using_angular_attention = true;
    metrics->framework.using_crystalline_loss = true;
    metrics->framework.using_cymatic_training = true;
    metrics->framework.using_ntt_attention = true;
    metrics->framework.using_simd_acceleration = true;
    
    metrics->control_thread_id = 0;  // Node Zero
    metrics->timestamp_ms = get_timestamp_ms();
}

/*
 * Register metrics callback
 */
void cllm_metrics_set_callback(
    CLLMMetricsCallback callback,
    void* user_data
) {
    g_metrics_callback = callback;
    g_metrics_user_data = user_data;
}

/*
 * Collect metrics from training system
 */
void cllm_metrics_collect(
    CLLMMetrics* metrics,
    void* training_ptr,
    void* thread_system_ptr
) {
    if (!metrics) return;
    
    CLLMTraining* training = (CLLMTraining*)training_ptr;
    // ThreadedTrainingSystem* thread_system = (ThreadedTrainingSystem*)thread_system_ptr;
    
    // Update timestamp
    metrics->timestamp_ms = get_timestamp_ms();
    metrics->update_count++;
    
    // Collect training metrics
    if (training) {
        metrics->training.current_epoch = training->current_epoch;
        metrics->training.total_epochs = training->config.num_epochs;
        metrics->training.current_step = training->current_step;
        metrics->training.current_loss = training->current_loss;
        metrics->training.best_loss = training->best_loss;
        
        // Calculate elapsed time
        if (training->start_time > 0) {
            time_t now = time(NULL);
            metrics->training.elapsed_time_seconds = 
                difftime(now, training->start_time);
        }
    }
    
    // Collect thread metrics from thread_system
    // (This will be implemented when we integrate with cllm_training_functions.c)
    if (thread_system_ptr) {
        // TODO: Collect thread states, positions, workloads
        // This requires access to ThreadedTrainingSystem internals
    }
    
    // Call callback if registered
    if (g_metrics_callback) {
        g_metrics_callback(metrics, g_metrics_user_data);
    }
}

/*
 * Update thread metrics
 */
void cllm_metrics_update_thread(
    CLLMMetrics* metrics,
    int thread_id,
    ThreadState state,
    double workload,
    const double position[3]
) {
    if (!metrics || thread_id < 0 || thread_id >= MAX_TRACKED_THREADS) return;
    
    ThreadMetrics* thread = &metrics->threads[thread_id];
    
    // Update state
    thread->state = state;
    thread->current_workload = workload;
    
    // Update position if provided
    if (position) {
        thread->position[0] = position[0];
        thread->position[1] = position[1];
        thread->position[2] = position[2];
    }
    
    // Update active thread count
    if (state == THREAD_STATE_WORKING || state == THREAD_STATE_CONTROL) {
        // Thread is active
    }
}

/*
 * Update training metrics
 */
void cllm_metrics_update_training(
    CLLMMetrics* metrics,
    int epoch,
    int step,
    float loss
) {
    if (!metrics) return;
    
    metrics->training.current_epoch = epoch;
    metrics->training.current_step = step;
    metrics->training.current_loss = loss;
    
    // Update best loss
    if (loss < metrics->training.best_loss || metrics->training.best_loss == 0.0) {
        metrics->training.best_loss = loss;
    }
}

/*
 * Update performance metrics
 */
void cllm_metrics_update_performance(
    CLLMMetrics* metrics,
    float tokens_per_sec,
    float cache_hit_rate,
    size_t memory_used
) {
    if (!metrics) return;
    
    metrics->training.tokens_per_second = tokens_per_sec;
    metrics->performance.cache_hit_rate = cache_hit_rate;
    metrics->performance.memory_used_bytes = memory_used;
    
    // Update peak memory
    if (memory_used > metrics->performance.memory_peak_bytes) {
        metrics->performance.memory_peak_bytes = (size_t)memory_used;
    }
}

/*
 * Get thread by ID
 */
ThreadMetrics* cllm_metrics_get_thread(
    CLLMMetrics* metrics,
    int thread_id
) {
    if (!metrics || thread_id < 0 || thread_id >= MAX_TRACKED_THREADS) {
        return NULL;
    }
    
    if (metrics->threads[thread_id].thread_id == thread_id) {
        return &metrics->threads[thread_id];
    }
    
    return NULL;
}

/*
 * Get control thread (Node Zero)
 */
ThreadMetrics* cllm_metrics_get_control_thread(
    CLLMMetrics* metrics
) {
    if (!metrics) return NULL;
    return cllm_metrics_get_thread(metrics, metrics->control_thread_id);
}

/*
 * Get all worker threads
 */
int cllm_metrics_get_worker_threads(
    CLLMMetrics* metrics,
    ThreadMetrics** workers,
    int max_workers
) {
    if (!metrics || !workers) return 0;
    
    int count = 0;
    for (int i = 0; i < MAX_TRACKED_THREADS && count < max_workers; i++) {
        ThreadMetrics* thread = &metrics->threads[i];
        if (thread->thread_id >= 0 && 
            thread->state != THREAD_STATE_IDLE &&
            thread->state != THREAD_STATE_TERMINATED &&
            thread->role != THREAD_ROLE_CONTROL) {
            workers[count++] = thread;
        }
    }
    
    return count;
}

/*
 * Print metrics summary
 */
void cllm_metrics_print(const CLLMMetrics* metrics) {
    if (!metrics) return;
    
    printf("\n=== CLLM Metrics Summary ===\n");
    printf("Timestamp: %lu ms\n", (unsigned long)metrics->timestamp_ms);
    printf("Update count: %lu\n", (unsigned long)metrics->update_count);
    
    printf("\nTraining:\n");
    printf("  Epoch: %d/%d\n", 
           metrics->training.current_epoch,
           metrics->training.total_epochs);
    printf("  Step: %d\n", metrics->training.current_step);
    printf("  Current loss: %.6f\n", metrics->training.current_loss);
    printf("  Best loss: %.6f\n", metrics->training.best_loss);
    printf("  Tokens/sec: %.2f\n", metrics->training.tokens_per_second);
    
    printf("\nFramework Status:\n");
    printf("  Lattice embeddings: %s\n", 
           metrics->framework.using_lattice_embeddings ? "✓" : "✗");
    printf("  Angular attention: %s\n",
           metrics->framework.using_angular_attention ? "✓" : "✗");
    printf("  Crystalline loss: %s\n",
           metrics->framework.using_crystalline_loss ? "✓" : "✗");
    printf("  Cymatic training: %s\n",
           metrics->framework.using_cymatic_training ? "✓" : "✗");
    printf("  NTT attention: %s\n",
           metrics->framework.using_ntt_attention ? "✓" : "✗");
    printf("  SIMD acceleration: %s\n",
           metrics->framework.using_simd_acceleration ? "✓" : "✗");
    
    printf("\nPerformance:\n");
    printf("  Cache hit rate: %.2f%%\n", 
           metrics->performance.cache_hit_rate * 100.0);
    printf("  Memory used: %.2f MB\n",
           metrics->performance.memory_used_bytes / (1024.0f * 1024.0f));
    printf("  Memory peak: %.2f MB\n",
           metrics->performance.memory_peak_bytes / (1024.0f * 1024.0f));
    printf("  Active threads: %d\n", metrics->performance.active_threads);
    
    printf("\nThreads:\n");
    printf("  Total threads: %d\n", metrics->num_threads);
    printf("  Control thread (Node Zero): %d\n", metrics->control_thread_id);
    printf("  Max hierarchy depth: %d\n", metrics->max_hierarchy_depth);
    
    // Print active threads
    int active_count = 0;
    for (int i = 0; i < MAX_TRACKED_THREADS; i++) {
        const ThreadMetrics* thread = &metrics->threads[i];
        if (thread->thread_id >= 0 && 
            thread->state != THREAD_STATE_IDLE &&
            thread->state != THREAD_STATE_TERMINATED) {
            active_count++;
        }
    }
    printf("  Active threads: %d\n", active_count);
    
    printf("===========================\n\n");
}

/*
 * Create metrics system
 */
CLLMMetrics* cllm_metrics_create(int max_threads) {
    CLLMMetrics* metrics = (CLLMMetrics*)calloc(1, sizeof(CLLMMetrics));
    if (!metrics) return NULL;
    
    cllm_metrics_init(metrics);
    metrics->num_threads = max_threads;
    
    return metrics;
}

/*
 * Free metrics system
 */
void cllm_metrics_free(CLLMMetrics* metrics) {
    if (metrics) {
        free(metrics);
    }
}

/*
 * Update thread state only
 */
void cllm_metrics_update_thread_state(CLLMMetrics* metrics, int thread_id, ThreadState state) {
    if (!metrics || thread_id < 0 || thread_id >= MAX_TRACKED_THREADS) return;
    
    ThreadMetrics* thread = &metrics->threads[thread_id];
    thread->state = state;
    thread->thread_id = thread_id;
}

/*
 * Update thread workload only
 */
void cllm_metrics_update_thread_workload(CLLMMetrics* metrics, int thread_id, int batches_processed) {
    if (!metrics || thread_id < 0 || thread_id >= MAX_TRACKED_THREADS) return;
    
    ThreadMetrics* thread = &metrics->threads[thread_id];
    thread->batches_processed = batches_processed;
    thread->thread_id = thread_id;
}

/*
 * Update training progress (epoch, step, total_steps)
 */
void cllm_metrics_update_training_progress(CLLMMetrics* metrics, int epoch, int step, int total_steps) {
    if (!metrics) return;
    
    metrics->training.current_epoch = epoch;
    metrics->training.current_step = step;
    // Store total_steps in a way that makes sense - we can use it to calculate progress
    (void)total_steps;  // For now, unused but available for future use
}

/*
 * Update loss value
 */
void cllm_metrics_update_loss(CLLMMetrics* metrics, float loss) {
    if (!metrics) return;
    
    metrics->training.current_loss = loss;
    
    // Update best loss if this is better
    if (loss < metrics->training.best_loss || metrics->training.best_loss == 0.0) {
        metrics->training.best_loss = loss;
    }
}

/*
 * Update framework status
 */
void cllm_metrics_update_framework_status(CLLMMetrics* metrics, int lattice_active, 
                                         int angular_active, int ntt_active, int cymatic_active) {
    if (!metrics) return;
    
    metrics->framework.using_lattice_embeddings = lattice_active;
    metrics->framework.using_angular_attention = angular_active;
    metrics->framework.using_ntt_attention = ntt_active;
    metrics->framework.using_cymatic_training = cymatic_active;
}

/*
 * Invoke all registered callbacks
 */
void cllm_metrics_invoke_callbacks(CLLMMetrics* metrics) {
    if (!metrics) return;
    
    // Invoke the global callback if registered
    if (g_metrics_callback) {
        g_metrics_callback(metrics, g_metrics_user_data);
    }
}