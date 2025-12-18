/**
 * @file abacus88d_unified.h
 * @brief Unified 88D Abacus with Integrated Threading
 * 
 * THIS IS THE SOLUTION.
 * 
 * Abacus88D IS the threading system. Not a wrapper, not a layer on top,
 * but the fundamental architecture where:
 * - 88 dimensions = 88 threads
 * - 8 layers = 8 hierarchy levels
 * - Geometric boundaries = shared memory
 * - Tetration towers = work queues
 * - Clock positions = thread IDs
 * 
 * This replaces:
 * - hierarchical_threading.h (absorbed)
 * - cllm_lattice_hierarchy.h (replaced)
 * - cllm_thread_pool.h (replaced)
 * - abacus88d_threading.h (shallow wrapper - deleted)
 */

#ifndef ALGORITHMS_ABACUS88D_UNIFIED_H
#define ALGORITHMS_ABACUS88D_UNIFIED_H

#include <math/abacus.h>
#include <math/clock_lattice.h>
#include <math/platonic_solids.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdatomic.h>

// Include components that will be absorbed
#include "message_passing.h"
#include "state_management.h"
#include "shared_memory_enhanced.h"
#include "hierarchical_memory.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * CONSTANTS
 * ============================================================================
 */

#define ABACUS88D_NUM_LAYERS 8
#define ABACUS88D_DIMS_PER_LAYER 11
#define ABACUS88D_TOTAL_DIMENSIONS 88  // 8 × 11
#define ABACUS88D_THREADS_PER_LAYER 12  // 11 workers + 1 control
#define ABACUS88D_TOTAL_THREADS 96      // 88 workers + 8 control

/* ============================================================================
 * THREAD STRUCTURE - THE FUNDAMENTAL UNIT
 * ============================================================================
 */

/**
 * @brief Thread role in 88D space
 */
typedef enum {
    ABACUS88D_ROLE_CONTROL,     // Control thread (position 0/12)
    ABACUS88D_ROLE_WORKER,      // Worker thread (positions 1-11)
    ABACUS88D_ROLE_COORDINATOR  // Layer coordinator
} Abacus88DThreadRole;

/**
 * @brief Thread state
 */
typedef enum {
    ABACUS88D_THREAD_INITIALIZING,
    ABACUS88D_THREAD_READY,
    ABACUS88D_THREAD_WORKING,
    ABACUS88D_THREAD_WAITING,
    ABACUS88D_THREAD_SYNCHRONIZING,
    ABACUS88D_THREAD_IDLE,
    ABACUS88D_THREAD_TERMINATING,
    ABACUS88D_THREAD_TERMINATED
} Abacus88DThreadState;

/**
 * @brief Work item for 88D thread
 */
typedef struct {
    uint64_t work_id;
    uint32_t priority;
    void* data;
    size_t data_size;
    void (*execute)(void* data, void* result);
    void* result;
} Abacus88DWorkItem;

/**
 * @brief 88D Thread - One per dimension
 * 
 * This IS a thread in the system. Not a wrapper, not a helper structure,
 * but THE thread. Its position in 88D space determines everything:
 * - Thread ID (from clock position)
 * - Hierarchy level (from layer)
 * - Neighbors (from geometric structure)
 * - Work queue (from tetration tower)
 * - Shared memory (from geometric boundaries)
 */
typedef struct Abacus88DThread {
    // ========================================================================
    // IDENTITY: FROM 88D POSITION
    // ========================================================================
    
    uint8_t layer;                      // Layer (0-7) = hierarchy level
    uint8_t dimension;                  // Dimension (0-10) = position in layer
    uint8_t clock_position;             // Clock position (1-12)
    uint32_t thread_id;                 // Unique ID = layer*12 + clock_position
    
    Abacus88DThreadRole role;           // Control or worker
    
    // ========================================================================
    // POSIX THREAD
    // ========================================================================
    
    pthread_t pthread;                  // POSIX thread handle
    atomic_bool running;                // Thread is running
    atomic_bool should_stop;            // Thread should stop
    
    // ========================================================================
    // GEOMETRIC POSITION: FROM PLATONIC SOLID
    // ========================================================================
    
    double position[3];                 // 3D position on Platonic solid
    PlatonicSolid* frame;               // Layer's Platonic solid frame
    uint32_t vertex_id;                 // Vertex ID on solid
    
    // ========================================================================
    // COMPUTATION: ABACUS VALUES
    // ========================================================================
    
    CrystallineAbacus* value;           // Current computational value
    CrystallineAbacus* accumulator;     // Gradient/result accumulator
    CrystallineAbacus* temp;            // Temporary computation space
    
    // ========================================================================
    // HIERARCHY: FROM 88D STRUCTURE
    // ========================================================================
    
    struct Abacus88DThread* parent;     // Parent layer thread (NULL if layer 0)
    struct Abacus88DThread* children[ABACUS88D_DIMS_PER_LAYER]; // Child threads
    uint32_t num_children;
    
    struct Abacus88DThread* neighbors[12]; // Same-layer neighbors (kissing spheres)
    uint32_t num_neighbors;
    
    struct Abacus88DThread* siblings[ABACUS88D_DIMS_PER_LAYER]; // Same-layer threads
    uint32_t num_siblings;
    
    // ========================================================================
    // SHARED MEMORY: FROM GEOMETRIC BOUNDARIES
    // ========================================================================
    
    SharedMemoryEnhanced* parent_boundary;     // Shared with parent
    SharedMemoryEnhanced* child_boundaries[ABACUS88D_DIMS_PER_LAYER];
    SharedMemoryEnhanced* neighbor_boundaries[12];
    
    HierarchicalMemoryBlock* local_memory;     // Thread-local memory
    
    // ========================================================================
    // WORK QUEUE: FROM TETRATION TOWER
    // ========================================================================
    
    Abacus88DWorkItem* work_queue;      // Work queue (ring buffer)
    uint32_t work_queue_size;
    atomic_uint work_head;              // Queue head (producer)
    atomic_uint work_tail;              // Queue tail (consumer)
    
    uint64_t work_completed;            // Total work completed
    uint64_t work_stolen;               // Work stolen from others
    uint64_t work_given;                // Work given to others
    
    // ========================================================================
    // STATE MANAGEMENT
    // ========================================================================
    
    atomic_int state;                   // Current state (Abacus88DThreadState)
    StateMachine* state_machine;        // State machine
    
    // ========================================================================
    // MESSAGE PASSING: LOCK-FREE
    // ========================================================================
    
    MessageQueue* inbox;                // Incoming messages
    MessageQueue* outbox;               // Outgoing messages
    uint64_t messages_sent;
    uint64_t messages_received;
    
    // ========================================================================
    // SYNCHRONIZATION
    // ========================================================================
    
    pthread_mutex_t thread_mutex;       // Thread-local mutex
    pthread_cond_t work_available;      // Work available condition
    pthread_cond_t work_complete;       // Work complete condition
    
    // ========================================================================
    // PERFORMANCE MONITORING
    // ========================================================================
    
    uint64_t operations_count;          // Total operations
    double total_time;                  // Total computation time
    double idle_time;                   // Total idle time
    uint64_t cache_hits;
    uint64_t cache_misses;
    
} Abacus88DThread;

/* ============================================================================
 * LAYER STRUCTURE - WITH INTEGRATED THREADING
 * ============================================================================
 */

/**
 * @brief Layer in 88D space - One hierarchy level
 */
typedef struct {
    // Dimensions (computational units)
    CrystallineAbacus* dimensions[ABACUS88D_DIMS_PER_LAYER];
    
    // Threads (one per dimension + control)
    Abacus88DThread threads[ABACUS88D_THREADS_PER_LAYER];
    Abacus88DThread* control_thread;    // Points to threads[0]
    
    // Geometric frame
    PlatonicSolid* frame;
    Layer88DType type;
    uint64_t scale_factor;
    
    // Layer-wide synchronization
    pthread_barrier_t layer_barrier;    // Synchronize all threads in layer
    pthread_mutex_t layer_lock;         // Layer-wide lock
    
    // Layer-wide state
    atomic_int active_threads;          // Number of active threads
    atomic_uint64_t layer_work_count;   // Total work in layer
    
} Layer88D;

/* ============================================================================
 * TETRATION SYSTEM - WORK DISTRIBUTION
 * ============================================================================
 */

/**
 * @brief Tetration tower - Maps to work queue
 */
typedef struct {
    uint32_t base;
    uint32_t depth;
    uint8_t source_layer;
    uint8_t target_layer;
    CrystallineAbacus* value;
    
    // Work queue for this tower
    Abacus88DWorkItem* work_items;
    uint32_t capacity;
    atomic_uint count;
    
} TetrationTower88D;

/**
 * @brief Tetration system - Global work distribution
 */
typedef struct {
    TetrationTower88D* towers;
    size_t num_towers;
    size_t capacity;
    
    // Tower-to-thread mapping
    uint32_t* tower_to_thread;          // Maps tower index to thread ID
    
} TetrationSystem88D;

/* ============================================================================
 * GEOMETRIC BOUNDARY - SHARED MEMORY
 * ============================================================================
 */

/**
 * @brief Geometric boundary - Where layers/dimensions meet
 */
typedef struct {
    uint8_t layer1;
    uint8_t dimension1;
    uint8_t layer2;
    uint8_t dimension2;
    
    // Shared memory region
    SharedMemoryEnhanced* shared_memory;
    
    // Boundary value
    CrystallineAbacus* shared_value;
    pthread_mutex_t boundary_lock;
    
    // Statistics
    uint64_t access_count;
    uint64_t conflict_count;
    
} GeometricBoundary88D;

/* ============================================================================
 * MAIN STRUCTURE - ABACUS88D IS THE THREADING SYSTEM
 * ============================================================================
 */

/**
 * @brief The complete 88-dimensional geometric abacus with integrated threading
 * 
 * THIS IS THE THREADING SYSTEM.
 * 
 * Everything is here:
 * - 88 dimensions = 88 worker threads
 * - 8 layers = 8 hierarchy levels
 * - 8 control threads (one per layer)
 * - Geometric boundaries = shared memory
 * - Tetration towers = work queues
 * - Clock lattice = thread IDs
 * 
 * There is no separate "thread pool" or "threading system".
 * This IS it.
 */
typedef struct Abacus88D {
    // ========================================================================
    // CORE: 88-DIMENSIONAL SPACE WITH THREADS
    // ========================================================================
    
    Layer88D layers[ABACUS88D_NUM_LAYERS];  // 8 layers with integrated threads
    
    // Quick access to all threads
    Abacus88DThread* all_threads[ABACUS88D_TOTAL_THREADS];
    uint32_t num_threads;
    
    // ========================================================================
    // WORK DISTRIBUTION: TETRATION TOWERS
    // ========================================================================
    
    TetrationSystem88D* tetration;      // Work distribution system
    
    // ========================================================================
    // SHARED MEMORY: GEOMETRIC BOUNDARIES
    // ========================================================================
    
    GeometricBoundary88D* boundaries;
    size_t num_boundaries;
    
    // ========================================================================
    // GLOBAL COORDINATION
    // ========================================================================
    
    ClockContext* clock;                // Clock lattice for prime generation
    
    // Global state
    atomic_bool initialized;
    atomic_bool running;
    atomic_bool should_stop;
    
    // Global synchronization
    pthread_barrier_t global_barrier;   // Synchronize all threads
    pthread_mutex_t global_lock;
    
    // ========================================================================
    // CONFIGURATION
    // ========================================================================
    
    uint32_t base;                      // Abacus base (60 for Babylonian)
    uint8_t active_layer;
    uint8_t active_dimension;
    
    // ========================================================================
    // GLOBAL STATISTICS
    // ========================================================================
    
    atomic_uint64_t total_operations;
    atomic_uint64_t total_messages;
    atomic_uint64_t total_work_items;
    atomic_uint64_t total_synchronizations;
    
    double start_time;
    double total_runtime;
    
} Abacus88D;

/* ============================================================================
 * LIFECYCLE FUNCTIONS
 * ============================================================================
 */

/**
 * @brief Create 88D abacus with integrated threading
 * 
 * This creates the ENTIRE system:
 * - 88 dimensions with values
 * - 96 threads (88 workers + 8 control)
 * - All geometric boundaries
 * - All work queues
 * - All synchronization primitives
 * 
 * @param base Abacus base (typically 60)
 * @return Complete 88D system or NULL on error
 */
Abacus88D* abacus88d_create_unified(uint32_t base);

/**
 * @brief Free 88D abacus
 * 
 * Stops all threads and frees all resources.
 */
void abacus88d_free_unified(Abacus88D* abacus);

/**
 * @brief Start all threads
 * 
 * Starts all 96 threads in the system.
 */
MathError abacus88d_start(Abacus88D* abacus);

/**
 * @brief Stop all threads
 * 
 * Gracefully stops all threads.
 */
MathError abacus88d_stop(Abacus88D* abacus);

/**
 * @brief Wait for all work to complete
 */
MathError abacus88d_wait(Abacus88D* abacus);

/* ============================================================================
 * WORK SUBMISSION
 * ============================================================================
 */

/**
 * @brief Submit work to specific dimension
 */
uint64_t abacus88d_submit_work(
    Abacus88D* abacus,
    uint8_t layer,
    uint8_t dimension,
    void* data,
    size_t data_size,
    void (*execute)(void*, void*),
    void* result
);

/**
 * @brief Submit work to best available thread
 */
uint64_t abacus88d_submit_work_auto(
    Abacus88D* abacus,
    void* data,
    size_t data_size,
    void (*execute)(void*, void*),
    void* result
);

/* ============================================================================
 * THREAD MANAGEMENT
 * ============================================================================
 */

/**
 * @brief Get thread by position
 */
Abacus88DThread* abacus88d_get_thread(
    Abacus88D* abacus,
    uint8_t layer,
    uint8_t dimension
);

/**
 * @brief Get thread statistics
 */
MathError abacus88d_get_thread_stats(
    Abacus88DThread* thread,
    uint64_t* operations,
    double* avg_time,
    double* idle_time
);

/* ============================================================================
 * SYNCHRONIZATION
 * ============================================================================
 */

/**
 * @brief Synchronize layer (all threads in layer wait)
 */
MathError abacus88d_sync_layer(Abacus88D* abacus, uint8_t layer);

/**
 * @brief Synchronize all threads
 */
MathError abacus88d_sync_all(Abacus88D* abacus);

/* ============================================================================
 * PERFORMANCE MONITORING
 * ============================================================================
 */

typedef struct {
    uint32_t num_threads;
    uint32_t active_threads;
    uint64_t total_operations;
    uint64_t total_messages;
    uint64_t total_work_items;
    double throughput;              // Operations per second
    double avg_thread_utilization;
    double load_balance_factor;     // 1.0 = perfect balance
} Abacus88DStats;

/**
 * @brief Get global statistics
 */
MathError abacus88d_get_stats(Abacus88D* abacus, Abacus88DStats* stats);

/**
 * @brief Print statistics
 */
void abacus88d_print_stats(Abacus88D* abacus);

#ifdef __cplusplus
}
#endif

#endif /* ALGORITHMS_ABACUS88D_UNIFIED_H */