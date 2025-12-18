#ifndef CLLM_CONTROL_PROCESS_H
#define CLLM_CONTROL_PROCESS_H

#include "ai/cllm_lattice_hierarchy.h"
#include "ai/cllm_sphere_stats.h"
#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// CONTROL PROCESS STATES
// ============================================================================

typedef enum {
    CONTROL_STATE_INITIALIZING,  // System is initializing
    CONTROL_STATE_RUNNING,       // System is running normally
    CONTROL_STATE_PAUSED,        // Training is paused
    CONTROL_STATE_STOPPING,      // System is shutting down
    CONTROL_STATE_STOPPED        // System has stopped
} ControlProcessState;

// ============================================================================
// CONTROL COMMANDS
// ============================================================================

typedef enum {
    CONTROL_CMD_START,           // Start the system
    CONTROL_CMD_STOP,            // Stop the system
    CONTROL_CMD_PAUSE,           // Pause training
    CONTROL_CMD_RESUME,          // Resume training
    CONTROL_CMD_START_EPOCH,     // Begin new epoch
    CONTROL_CMD_END_EPOCH,       // Complete epoch
    CONTROL_CMD_CHECKPOINT,      // Save system state
    CONTROL_CMD_RESTORE,         // Load system state
    CONTROL_CMD_REBALANCE,       // Rebalance hierarchy
    CONTROL_CMD_SPAWN_SPHERE,    // Create new sphere
    CONTROL_CMD_TERMINATE_SPHERE // Remove sphere
} ControlCommand;

// ============================================================================
// SYSTEM CONFIGURATION
// ============================================================================

typedef struct {
    // Hierarchy configuration
    uint32_t max_hierarchy_depth;      // Maximum depth of sphere hierarchy
    uint32_t max_spheres_per_level;    // Maximum spheres at each level
    uint32_t initial_sphere_count;     // Initial number of spheres to create
    
    // Training configuration
    uint32_t batch_size;               // Batch size for training
    uint32_t max_epochs;               // Maximum number of epochs
    double learning_rate;              // Learning rate
    
    // Resource limits
    uint32_t max_threads;              // Maximum number of threads
    size_t max_memory_bytes;           // Maximum memory usage
    
    // Synchronization settings
    uint32_t sync_interval_batches;    // Batches between synchronizations
    uint32_t checkpoint_interval_epochs; // Epochs between checkpoints
    
    // Health monitoring
    uint32_t health_check_interval_ms; // Health check interval
    double sphere_timeout_seconds;     // Timeout for sphere operations
    
    // 144,000 boundary settings
    bool enable_boundary_awareness;    // Enable 144,000 boundary detection
    bool enable_twin_prime_tracking;   // Enable twin prime tracking
} SystemConfiguration;

// ============================================================================
// EPOCH STATE
// ============================================================================

typedef struct {
    uint32_t current_epoch;            // Current epoch number
    uint32_t total_batches;            // Total batches in epoch
    uint32_t completed_batches;        // Completed batches
    
    double epoch_start_time;           // Epoch start timestamp
    double epoch_duration;             // Duration of last epoch
    
    double total_loss;                 // Accumulated loss
    double average_loss;               // Average loss per batch
    
    uint64_t primes_processed;         // Primes processed in epoch
    uint64_t gradients_computed;       // Gradients computed
    uint64_t weights_updated;          // Weight updates performed
    
    bool epoch_in_progress;            // Is epoch currently running
    pthread_barrier_t epoch_barrier;   // Barrier for epoch synchronization
} EpochState;

// ============================================================================
// SYSTEM HEALTH
// ============================================================================

typedef struct {
    uint32_t active_spheres;           // Number of active spheres
    uint32_t idle_spheres;             // Number of idle spheres
    uint32_t failed_spheres;           // Number of failed spheres
    
    double cpu_utilization;            // CPU utilization percentage
    double memory_utilization;         // Memory utilization percentage
    
    uint64_t total_messages_sent;      // Total messages sent
    uint64_t total_messages_received;  // Total messages received
    uint64_t message_queue_depth;      // Current message queue depth
    
    uint64_t work_items_pending;       // Pending work items
    uint64_t work_items_completed;     // Completed work items
    
    double last_health_check_time;     // Last health check timestamp
} SystemHealth;

// ============================================================================
// CONTROL PROCESS STRUCTURE
// ============================================================================

typedef struct ControlProcess {
    // State
    ControlProcessState state;         // Current control process state
    pthread_mutex_t state_mutex;       // Mutex for state changes
    pthread_cond_t state_cond;         // Condition variable for state changes
    
    // Configuration
    SystemConfiguration config;        // System configuration
    
    // Hierarchy
    CLLMLatticeHierarchy* root_sphere; // Root of sphere hierarchy
    uint32_t total_sphere_count;       // Total number of spheres
    pthread_mutex_t hierarchy_mutex;   // Mutex for hierarchy operations
    
    // Epoch management
    EpochState epoch_state;            // Current epoch state
    pthread_mutex_t epoch_mutex;       // Mutex for epoch operations
    
    // Statistics
    SphereStatistics system_stats;     // Aggregated system statistics
    pthread_mutex_t stats_mutex;       // Mutex for statistics
    
    // Health monitoring
    SystemHealth health;               // System health metrics
    pthread_t health_monitor_thread;   // Health monitoring thread
    bool health_monitor_running;       // Is health monitor active
    
    // Synchronization
    pthread_barrier_t global_barrier;  // Global synchronization barrier
    pthread_mutex_t sync_mutex;        // Mutex for synchronization
    
    // Checkpoint/Restore
    char checkpoint_path[256];         // Path for checkpoints
    uint32_t checkpoint_version;       // Current checkpoint version
    
} ControlProcess;

// ============================================================================
// LIFECYCLE FUNCTIONS
// ============================================================================

/**
 * Create and initialize a control process
 * 
 * @param config System configuration
 * @return Pointer to created control process, or NULL on failure
 */
ControlProcess* control_process_create(const SystemConfiguration* config);

/**
 * Free a control process and all associated resources
 * 
 * @param cp Control process to free
 */
void control_process_free(ControlProcess* cp);

/**
 * Start the control process and initialize the sphere hierarchy
 * 
 * @param cp Control process
 * @return true on success, false on failure
 */
bool control_process_start(ControlProcess* cp);

/**
 * Stop the control process and shut down all spheres
 * 
 * @param cp Control process
 * @return true on success, false on failure
 */
bool control_process_stop(ControlProcess* cp);

/**
 * Pause training (spheres remain active but stop processing)
 * 
 * @param cp Control process
 * @return true on success, false on failure
 */
bool control_process_pause(ControlProcess* cp);

/**
 * Resume training after pause
 * 
 * @param cp Control process
 * @return true on success, false on failure
 */
bool control_process_resume(ControlProcess* cp);

// ============================================================================
// EPOCH MANAGEMENT
// ============================================================================

/**
 * Start a new training epoch
 * 
 * @param cp Control process
 * @param total_batches Total number of batches in epoch
 * @return true on success, false on failure
 */
bool control_process_start_epoch(ControlProcess* cp, uint32_t total_batches);

/**
 * End the current training epoch
 * 
 * @param cp Control process
 * @return true on success, false on failure
 */
bool control_process_end_epoch(ControlProcess* cp);

/**
 * Wait for all spheres to complete the current epoch
 * 
 * @param cp Control process
 * @param timeout_seconds Maximum time to wait (0 = infinite)
 * @return true if all spheres completed, false on timeout
 */
bool control_process_wait_epoch_complete(ControlProcess* cp, double timeout_seconds);

/**
 * Get current epoch statistics
 * 
 * @param cp Control process
 * @param epoch_state Output buffer for epoch state
 * @return true on success, false on failure
 */
bool control_process_get_epoch_stats(ControlProcess* cp, EpochState* epoch_state);

// ============================================================================
// SPHERE LIFECYCLE MANAGEMENT
// ============================================================================

/**
 * Spawn a new sphere in the hierarchy
 * 
 * @param cp Control process
 * @param parent_id Parent sphere ID (0 for root)
 * @param symmetry_group Symmetry group assignment
 * @return Sphere ID of created sphere, or 0 on failure
 */
uint32_t control_process_spawn_sphere(ControlProcess* cp, uint32_t parent_id, 
                                      uint32_t symmetry_group);

/**
 * Terminate a sphere and remove it from the hierarchy
 * 
 * @param cp Control process
 * @param sphere_id Sphere ID to terminate
 * @return true on success, false on failure
 */
bool control_process_terminate_sphere(ControlProcess* cp, uint32_t sphere_id);

/**
 * Rebalance the sphere hierarchy for optimal load distribution
 * 
 * @param cp Control process
 * @return true on success, false on failure
 */
bool control_process_rebalance_hierarchy(ControlProcess* cp);

/**
 * Check health of a specific sphere
 * 
 * @param cp Control process
 * @param sphere_id Sphere ID to check
 * @return true if healthy, false if unhealthy or not found
 */
bool control_process_check_sphere_health(ControlProcess* cp, uint32_t sphere_id);

/**
 * Recover a failed sphere
 * 
 * @param cp Control process
 * @param sphere_id Sphere ID to recover
 * @return true on success, false on failure
 */
bool control_process_recover_sphere(ControlProcess* cp, uint32_t sphere_id);

// ============================================================================
// SYSTEM-WIDE OPERATIONS
// ============================================================================

/**
 * Broadcast updated weights to all spheres
 * 
 * @param cp Control process
 * @param weights Pointer to weight data
 * @param weight_count Number of weights
 * @return true on success, false on failure
 */
bool control_process_broadcast_weights(ControlProcess* cp, const double* weights, 
                                       size_t weight_count);

/**
 * Collect and aggregate gradients from all spheres
 * 
 * @param cp Control process
 * @param gradients Output buffer for aggregated gradients
 * @param gradient_count Number of gradients to collect
 * @return true on success, false on failure
 */
bool control_process_collect_gradients(ControlProcess* cp, double* gradients, 
                                       size_t gradient_count);

/**
 * Synchronize all spheres at a global barrier
 * 
 * @param cp Control process
 * @param timeout_seconds Maximum time to wait (0 = infinite)
 * @return true if all spheres synchronized, false on timeout
 */
bool control_process_synchronize_all(ControlProcess* cp, double timeout_seconds);

/**
 * Save system state to checkpoint
 * 
 * @param cp Control process
 * @param checkpoint_name Name for checkpoint
 * @return true on success, false on failure
 */
bool control_process_checkpoint(ControlProcess* cp, const char* checkpoint_name);

/**
 * Restore system state from checkpoint
 * 
 * @param cp Control process
 * @param checkpoint_name Name of checkpoint to restore
 * @return true on success, false on failure
 */
bool control_process_restore(ControlProcess* cp, const char* checkpoint_name);

// ============================================================================
// STATISTICS & MONITORING
// ============================================================================

/**
 * Get aggregated system-wide statistics
 * 
 * @param cp Control process
 * @param stats Output buffer for statistics
 * @return true on success, false on failure
 */
bool control_process_get_system_stats(ControlProcess* cp, SphereStatistics* stats);

/**
 * Get statistics for a specific sphere
 * 
 * @param cp Control process
 * @param sphere_id Sphere ID
 * @param stats Output buffer for statistics
 * @return true on success, false on failure
 */
bool control_process_get_sphere_stats(ControlProcess* cp, uint32_t sphere_id, 
                                      SphereStatistics* stats);

/**
 * Get current system health metrics
 * 
 * @param cp Control process
 * @param health Output buffer for health metrics
 * @return true on success, false on failure
 */
bool control_process_get_system_health(ControlProcess* cp, SystemHealth* health);

/**
 * Print the current sphere hierarchy structure
 * 
 * @param cp Control process
 */
void control_process_print_hierarchy(const ControlProcess* cp);

/**
 * Print detailed system statistics
 * 
 * @param cp Control process
 */
void control_process_print_stats(const ControlProcess* cp);

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * Get current control process state
 * 
 * @param cp Control process
 * @return Current state
 */
ControlProcessState control_process_get_state(const ControlProcess* cp);

/**
 * Get string representation of control process state
 * 
 * @param state State to convert
 * @return String representation
 */
const char* control_process_state_to_string(ControlProcessState state);

/**
 * Find a sphere by ID in the hierarchy
 * 
 * @param cp Control process
 * @param sphere_id Sphere ID to find
 * @return Pointer to sphere, or NULL if not found
 */
CLLMLatticeHierarchy* control_process_find_sphere(ControlProcess* cp, 
                                                  uint32_t sphere_id);

/**
 * Count total number of spheres in hierarchy
 * 
 * @param cp Control process
 * @return Total sphere count
 */
uint32_t control_process_count_spheres(const ControlProcess* cp);

/**
 * Validate control process integrity
 * 
 * @param cp Control process
 * @return true if valid, false if corrupted
 */
bool control_process_validate(const ControlProcess* cp);

#ifdef __cplusplus
}
#endif

#endif // CLLM_CONTROL_PROCESS_H