/*
 * CLLM Metrics System
 * Real-time metrics collection and reporting for UI integration
 * 
 * This system provides a bridge between the training system and UI,
 * allowing real-time visualization of:
 * - Thread states and hierarchy
 * - Sphere positions and workloads
 * - Training progress and performance
 * - Mathematical framework status
 */

#ifndef CLLM_METRICS_H
#define CLLM_METRICS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Maximum number of threads to track
#define MAX_TRACKED_THREADS 144  // 12 + 12*12 (2 levels)

// Thread states
typedef enum {
    THREAD_STATE_IDLE = 0,
    THREAD_STATE_WORKING = 1,
    THREAD_STATE_CONTROL = 2,
    THREAD_STATE_WAITING = 3,
    THREAD_STATE_TERMINATED = 4
} ThreadState;

// Thread role
typedef enum {
    THREAD_ROLE_WORKER = 0,
    THREAD_ROLE_CONTROL = 1,
    THREAD_ROLE_BOTH = 2  // Worker that can become control
} ThreadRole;

// Per-thread metrics
typedef struct {
    int thread_id;
    int sphere_id;
    int parent_thread_id;
    int hierarchy_level;
    ThreadState state;
    ThreadRole role;
    
    // Position in 3D space (for visualization)
    float position[3];
    
    // Workload metrics
    int batches_processed;
    float current_workload;  // 0.0 to 1.0
    
    // Performance metrics
    float tokens_per_second;
    float average_batch_time_ms;
    
    // Symmetry group (0-11)
    int symmetry_group;
    
    // Child threads (for hierarchy visualization)
    int num_children;
    int child_thread_ids[12];
} ThreadMetrics;

// Training metrics
typedef struct {
    // Training progress
    int current_epoch;
    int total_epochs;
    int current_step;
    int total_steps;
    
    // Loss metrics
    float current_loss;
    float best_loss;
    float average_loss;
    
    // Performance metrics
    float tokens_per_second;
    float batches_per_second;
    float samples_per_second;
    
    // Time metrics
    double elapsed_time_seconds;
    double estimated_time_remaining_seconds;
} TrainingMetrics;

// Mathematical framework status
typedef struct {
    bool using_lattice_embeddings;    // L(n,d,k,λ) formula
    bool using_angular_attention;     // θ(n,k,λ,ω,ψ) formula
    bool using_crystalline_loss;      // GCD-based loss
    bool using_cymatic_training;      // Cymatic resonance
    bool using_ntt_attention;         // NTT-based attention
    bool using_simd_acceleration;     // SIMD operations
} FrameworkStatus;

// Performance metrics
typedef struct {
    // Speedup metrics
    float gcd_similarity_speedup;     // vs dot product
    float ntt_attention_speedup;      // vs standard attention
    float ulam_spiral_speedup;        // cache locality improvement
    
    // Cache metrics
    float cache_hit_rate;
    float cache_miss_rate;
    
    // Memory metrics
    size_t memory_used_bytes;
    size_t memory_peak_bytes;
    
    // Thread metrics
    float thread_utilization;         // 0.0 to 1.0
    int active_threads;
    int idle_threads;
} PerformanceMetrics;

// Complete metrics structure
typedef struct {
    // Thread hierarchy
    int num_threads;
    ThreadMetrics threads[MAX_TRACKED_THREADS];
    int control_thread_id;  // Node Zero
    int max_hierarchy_depth;
    
    // Training metrics
    TrainingMetrics training;
    
    // Framework status
    FrameworkStatus framework;
    
    // Performance metrics
    PerformanceMetrics performance;
    
    // Timestamp
    uint64_t timestamp_ms;
    
    // Update counter
    uint64_t update_count;
} CLLMMetrics;

// Callback function type
typedef void (*CLLMMetricsCallback)(const CLLMMetrics* metrics, void* user_data);

/*
 * Initialize metrics system
 */
void cllm_metrics_init(CLLMMetrics* metrics);

/*
 * Register metrics callback
 * The callback will be called periodically during training
 */
void cllm_metrics_set_callback(
    CLLMMetricsCallback callback,
    void* user_data
);

/*
 * Collect metrics from training system
 * Note: CLLMTraining and ThreadedTrainingSystem are defined in cllm_training.h
 */
void cllm_metrics_collect(
    CLLMMetrics* metrics,
    void* training,
    void* thread_system
);

/*
 * Update thread metrics
 */
void cllm_metrics_update_thread(
    CLLMMetrics* metrics,
    int thread_id,
    ThreadState state,
    double workload,
    const double position[3]
);

/*
 * Update training metrics
 */
void cllm_metrics_update_training(
    CLLMMetrics* metrics,
    int epoch,
    int step,
    float loss
);

/*
 * Update performance metrics
 */
void cllm_metrics_update_performance(
    CLLMMetrics* metrics,
    float tokens_per_sec,
    float cache_hit_rate,
    size_t memory_used
);

/*
 * Get thread by ID
 */
ThreadMetrics* cllm_metrics_get_thread(
    CLLMMetrics* metrics,
    int thread_id
);

/*
 * Get control thread (Node Zero)
 */
ThreadMetrics* cllm_metrics_get_control_thread(
    CLLMMetrics* metrics
);

/*
 * Get all worker threads
 */
int cllm_metrics_get_worker_threads(
    CLLMMetrics* metrics,
    ThreadMetrics** workers,
    int max_workers
);

/*
 * Print metrics summary (for debugging)
 */
void cllm_metrics_print(const CLLMMetrics* metrics);

/*
 * Create metrics system
 */
CLLMMetrics* cllm_metrics_create(int max_threads);

/*
 * Free metrics system
 */
void cllm_metrics_free(CLLMMetrics* metrics);

/*
 * Update thread state only
 */
void cllm_metrics_update_thread_state(CLLMMetrics* metrics, int thread_id, ThreadState state);

/*
 * Update thread workload only
 */
void cllm_metrics_update_thread_workload(CLLMMetrics* metrics, int thread_id, int batches_processed);

/*
 * Update training progress (epoch, step, total_steps)
 */
void cllm_metrics_update_training_progress(CLLMMetrics* metrics, int epoch, int step, int total_steps);

/*
 * Update loss value
 */
void cllm_metrics_update_loss(CLLMMetrics* metrics, float loss);

/*
 * Update framework status
 */
void cllm_metrics_update_framework_status(CLLMMetrics* metrics, int lattice_active, 
                                         int angular_active, int ntt_active, int cymatic_active);

/*
 * Invoke all registered callbacks
 */
void cllm_metrics_invoke_callbacks(CLLMMetrics* metrics);

#ifdef __cplusplus
}
#endif

#endif // CLLM_METRICS_H