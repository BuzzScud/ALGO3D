/**
 * @file hierarchical_threading.h
 * @brief Unified Hierarchical Threading System with 88D Integration
 * 
 * THE UNIFIED THREADING SOLUTION
 * 
 * This is the complete integration of:
 * - Week 1-4 components (hierarchical memory, shared memory, messages, state, work)
 * - 88D Geometric Structure (8 layers × 11 dimensions = 88 dimensions)
 * - Kissing Spheres Topology (12-fold symmetry)
 * - Abacus Computation (CrystallineAbacus integration)
 * - Self-Similar Nesting (sphere groups can attach to other groups)
 * 
 * Design Philosophy:
 * - 88 dimensions = 88 threads (natural parallelism)
 * - 8 layers = 8 hierarchy levels (natural hierarchy)
 * - Geometric boundaries = shared memory (natural communication)
 * - Clock positions = thread IDs (deterministic)
 * - Sphere threading with neighbor operations
 * - Lock-free communication
 * - Work stealing for load balancing
 * 
 * Key Features:
 * - 88D geometric organization (8 layers × 11 dimensions)
 * - 12-fold symmetry (kissing spheres)
 * - Parent-child-sibling-neighbor relationships
 * - Hierarchical + shared memory
 * - Lock-free message passing
 * - State machines with full tracking
 * - Work distribution with stealing
 * - Complete statistics and monitoring
 * - Self-similar nesting (attach to other sphere groups)
 * - NUMA-aware allocation
 * 
 * This replaces:
 * - sphere_threading.h (absorbed)
 * - threading.h (absorbed)
 * - CLLMLatticeHierarchy (replaced)
 * - cllm_thread_pool.h (replaced)
 */

#ifndef HIERARCHICAL_THREADING_H
#define HIERARCHICAL_THREADING_H

#include "hierarchical_memory.h"
#include "shared_memory.h"
#include "shared_memory_enhanced.h"
#include "message_passing.h"
#include "state_management.h"
#include "work_distribution.h"
#include "abacus88d.h"
#include <math/types.h>
#include <math/clock.h>
#include <stdint.h>

// Forward declaration for GeometricMatrix (to avoid circular dependency)
struct GeometricMatrix;
#include <stdbool.h>
#include <stddef.h>
#include <pthread.h>
#include <stdatomic.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declaration for generic model interface
struct GenericModel;

// ============================================================================
// CONSTANTS
// ============================================================================

// Original constants
#define HIERARCHICAL_THREAD_MAX_NEIGHBORS 12
#define HIERARCHICAL_THREAD_MAX_CHILDREN 12
#define HIERARCHICAL_THREAD_BROADCAST_ID 0xFFFFFFFF

// 88D Integration constants
#define HIERARCHICAL_88D_NUM_LAYERS 8
#define HIERARCHICAL_88D_DIMS_PER_LAYER 11
#define HIERARCHICAL_88D_TOTAL_DIMENSIONS 88  // 8 × 11
#define HIERARCHICAL_88D_THREADS_PER_LAYER 12  // 11 workers + 1 control
#define HIERARCHICAL_88D_TOTAL_THREADS 96      // 88 workers + 8 control
#define HIERARCHICAL_88D_CLOCK_POSITIONS 12    // 12-fold symmetry

// Note: Layer88DType is defined in abacus88d.h, we use it here

// ============================================================================
// THREAD TYPES
// ============================================================================

/**
 * Thread role in hierarchy
 */
typedef enum {
    THREAD_ROLE_CONTROL,    // Control thread (root of hierarchy)
    THREAD_ROLE_WORKER,     // Worker thread (leaf node)
    THREAD_ROLE_MANAGER,    // Manager thread (intermediate node)
    THREAD_ROLE_HELPER      // Helper thread (temporary)
} ThreadRole;

/**
 * Thread relationship type
 */
typedef enum {
    THREAD_REL_PARENT,      // Parent thread
    THREAD_REL_CHILD,       // Child thread
    THREAD_REL_SIBLING,     // Sibling thread (same parent)
    THREAD_REL_NEIGHBOR,    // Neighbor thread (kissing sphere)
    THREAD_REL_NONE         // No relationship
} ThreadRelationType;

// ============================================================================
// THREAD NEIGHBOR STRUCTURE
// ============================================================================

/**
 * Thread neighbor information
 */
typedef struct {
    uint32_t thread_id;                 // Neighbor thread ID
    ThreadRelationType relationship;    // Relationship type
    double distance;                    // Geometric distance
    SharedMemoryEnhanced* boundary;     // Shared boundary memory
} ThreadNeighbor;

// ============================================================================
// HIERARCHICAL THREAD STRUCTURE
// ============================================================================

/**
 * Hierarchical Thread
 * 
 * Represents one thread in the hierarchical threading system.
 * Integrates all Week 1-4 components.
 */
// ============================================================================
// WORK QUEUE FOR TRAINING
// ============================================================================

typedef enum {
    TRAINING_WORK_TYPE_FORWARD,
    TRAINING_WORK_TYPE_BACKWARD
} TrainingWorkType;

typedef struct TrainingWorkItem {
    TrainingWorkType type;
    uint32_t token_id;
    uint32_t target_id;  // For backward pass
    struct TrainingWorkItem* next;
} TrainingWorkItem;

// ============================================================================
// HIERARCHICAL THREAD STRUCTURE
// ============================================================================

typedef struct HierarchicalThread {
    // Identity
    uint32_t thread_id;                 // Unique thread ID
    pthread_t pthread;                  // POSIX thread handle
    ThreadRole role;                    // Thread role
    
    // Hierarchy
    struct HierarchicalThread* parent;  // Parent thread
    struct HierarchicalThread** children; // Child threads
    uint32_t num_children;              // Number of children
    uint32_t max_children;              // Maximum children
    
    // Neighbors (kissing spheres)
    ThreadNeighbor neighbors[HIERARCHICAL_THREAD_MAX_NEIGHBORS];
    uint32_t num_neighbors;             // Number of neighbors
    
    // Week 1: Hierarchical Memory
    HierarchicalMemoryBlock* memory;    // Thread's hierarchical memory
    uint32_t memory_segment_id;         // Primary memory segment
    
    // Week 2: Shared Memory
    SharedMemoryEnhanced* local_shared; // Local shared memory
    SharedMemoryEnhanced* parent_shared; // Shared with parent
    SharedMemoryEnhanced** child_shared; // Shared with children
    
    // Week 3: Message Passing
    MessageQueue* inbox;                // Incoming messages
    MessageQueue* outbox;               // Outgoing messages
    uint64_t messages_sent;             // Total messages sent
    uint64_t messages_received;         // Total messages received
    
    // Week 4: State Management
    StateMachine* state_machine;        // Thread state machine
    StateType current_state;            // Current state (cached)
    
    // Week 4: Work Distribution
    WorkPool* work_pool;                // Thread's work pool
    uint64_t work_completed;            // Total work completed
    uint64_t work_stolen;               // Work stolen from others
    
    // Geometric Position (from abacus)
    double* position;                   // Position in N-dimensional space
    uint32_t num_dimensions;            // Number of dimensions
    uint32_t symmetry_group;            // Symmetry group (0 to N-1)
    
    // Thread Control
    volatile bool running;              // Thread is running
    volatile bool should_stop;          // Thread should stop
    pthread_mutex_t control_mutex;      // Control mutex
    pthread_cond_t control_cond;        // Control condition variable
    
    // Statistics
    uint64_t start_time;                // Thread start time
    uint64_t total_runtime;             // Total runtime (nanoseconds)
    uint64_t context_switches;          // Number of context switches
    
    // NUMA
    int numa_node;                      // NUMA node this thread is on
    
    // ========================================================================
    // 88D INTEGRATION
    // ========================================================================
    
    // 88D Position
    uint8_t layer;                      // Layer (0-7) = hierarchy level
    uint8_t dimension;                  // Dimension (0-10) = position in layer
    uint8_t clock_position;             // Clock position (1-12)
    
    // Geometric Frame
    PlatonicSolid* platonic_frame;      // Layer's Platonic solid frame
    uint32_t vertex_id;                 // Vertex ID on solid
    
    // Abacus Computation
    CrystallineAbacus* value;           // Current computational value
    CrystallineAbacus* accumulator;     // Gradient/result accumulator
    CrystallineAbacus* temp;            // Temporary computation space
    
    // ========================================================================
    // PHASE 2: THREAD-LOCAL PARAMETER STORAGE
    // ========================================================================
    
    // LEGACY: Flat array parameter storage (being phased out)
    CrystallineAbacus** parameters;     // Array of parameters
    uint32_t num_parameters;            // Number of parameters
    uint32_t max_parameters;            // Maximum parameters (capacity)
    
    // NEW: Geometric matrix parameter storage (88D architecture)
    struct GeometricMatrix** geometric_params;      // Array of geometric matrices
    struct GeometricMatrix** geometric_gradients;   // Gradient matrices
    struct GeometricMatrix** geometric_momentum;    // Momentum matrices (Adam)
    struct GeometricMatrix** geometric_velocity;    // Velocity matrices (Adam)
    uint32_t num_geometric_params;                  // Number of geometric parameters
    uint32_t max_geometric_params;                  // Maximum geometric parameters
    
    // Parameter metadata
    struct {
        char name[64];                  // Parameter name
        uint32_t* shape;                // Parameter shape
        uint32_t num_dims;              // Number of dimensions
        size_t total_elements;          // Total elements
        bool requires_grad;             // Needs gradients
        bool is_initialized;            // Is initialized
    } *param_metadata;                  // [num_parameters]
    
    // Gradient storage (same structure as parameters)
    CrystallineAbacus** gradients;      // Array of gradients
    
    // Optimizer state (Adam)
    CrystallineAbacus** momentum;       // First moment (m)
    CrystallineAbacus** velocity;       // Second moment (v)
    uint64_t optimizer_step;            // Optimizer step count
    
    // Parameter locks (for thread-safe access)
    pthread_mutex_t* param_locks;       // One lock per parameter
    pthread_mutex_t param_list_lock;    // Lock for parameter list operations
    
    // ========================================================================
    
    // Sibling Relationships (same layer)
    struct HierarchicalThread** siblings;  // Sibling threads
    uint32_t num_siblings;              // Number of siblings
    
    // Boundary Notifications
    bool near_boundary;                 // Near geometric boundary
    bool boundary_crossed;              // Crossed boundary
    uint64_t boundary_crossings;        // Total boundary crossings
    
    // Twin Prime Notifications
    bool twin_prime_detected;           // Twin prime detected
    uint64_t twin_primes_found;         // Total twin primes found
    
    // Gradient Accumulation (for CLLM)
    void* gradient_buffer;              // Gradient buffer
    size_t gradient_buffer_size;        // Buffer size
    pthread_mutex_t gradient_lock;      // Gradient lock
    
    // Batch Processing
    void** batch_queue;                 // Batch queue
    uint32_t batch_capacity;            // Batch capacity
    atomic_uint batch_count;            // Current batch count
    
    // CLLM Training Integration (reuse existing patterns)
    void* thread_local_training_ctx;    // ThreadLocalTrainingContext* (from legacy)
    double* activation_buffer;          // Forward pass activations
    size_t activation_buffer_size;      // Activation buffer size
    double* cached_qkv;                 // Cached Q, K, V for backward pass
    size_t cached_qkv_size;             // Cache size
    
    // Model Reference (for accessing model parameters during computation)
    struct GenericModel* model;         // Generic model interface (no circular dependency)
    
    // Work Queue (for training)
    TrainingWorkItem* work_queue_head;  // Head of work queue
    TrainingWorkItem* work_queue_tail;  // Tail of work queue
    pthread_mutex_t work_queue_mutex;   // Work queue mutex
    uint32_t work_queue_size;           // Current queue size
    
    // Completion tracking
    volatile bool should_exit;          // Thread should exit
    pthread_barrier_t* completion_barrier; // Barrier for completion signaling
    
} HierarchicalThread;

// ============================================================================
// THREAD POOL STRUCTURE
// ============================================================================

/**
 * Hierarchical Thread Pool
 * 
 * Manages a pool of hierarchical threads organized geometrically.
 */
typedef struct {
    // Threads
    HierarchicalThread** threads;       // Array of threads
    uint32_t num_threads;               // Number of threads
    uint32_t max_threads;               // Maximum threads
    
    // Hierarchy
    HierarchicalThread* root;           // Root control thread
    uint32_t num_levels;                // Number of hierarchy levels
    uint32_t threads_per_level;         // Threads per level
    
    // Shared Components
    HierarchicalMemoryBlock* global_memory;  // Global hierarchical memory
    WorkDistributor* work_distributor;       // Global work distributor
    StateManager* state_manager;             // Global state manager
    
    // Configuration
    uint32_t symmetry_fold;             // N-fold symmetry (default 12)
    uint32_t num_dimensions;            // Number of dimensions
    bool numa_aware;                    // Use NUMA-aware allocation
    
    // Statistics
    uint64_t total_messages;            // Total messages passed
    uint64_t total_work_items;          // Total work items processed
    uint64_t total_state_changes;       // Total state changes
    
    // Control
    pthread_mutex_t pool_mutex;         // Pool-wide mutex
    volatile bool initialized;          // Pool is initialized
    volatile bool running;              // Pool is running
    
    // ========================================================================
    // 88D INTEGRATION
    // ========================================================================
    
    // 88D Organization
    bool use_88d_structure;             // Use 88D organization
    HierarchicalThread* layers[HIERARCHICAL_88D_NUM_LAYERS][HIERARCHICAL_88D_THREADS_PER_LAYER];
    HierarchicalThread* control_threads[HIERARCHICAL_88D_NUM_LAYERS];  // One per layer
    
    // Geometric Boundaries (shared memory between layers/dimensions)
    SharedMemoryEnhanced** geometric_boundaries;
    uint32_t num_boundaries;
    
    // Clock Lattice (for prime generation and thread IDs)
    ClockContext* clock_lattice;
    
    // Platonic Solid Frames (one per layer)
    PlatonicSolid* layer_frames[HIERARCHICAL_88D_NUM_LAYERS];
    
    // Layer Barriers (synchronize all threads in a layer)
    pthread_barrier_t layer_barriers[HIERARCHICAL_88D_NUM_LAYERS];
    
    // Global Barrier (synchronize all threads)
    pthread_barrier_t global_barrier;
    
    // 88D Statistics
    _Atomic uint64_t total_boundary_crossings;
    _Atomic uint64_t total_twin_primes;
    _Atomic uint64_t total_operations;
    
    // Self-Similar Nesting (attach to other sphere groups)
    struct HierarchicalThreadPool* parent_group;  // Parent sphere group
    struct HierarchicalThreadPool** child_groups; // Child sphere groups
    uint32_t num_child_groups;
    uint32_t max_child_groups;
    
    // ========================================================================
    // ADAPTIVE THREADING (for limited cores)
    // ========================================================================
    
    // Adaptive Configuration
    bool use_adaptive_threading;        // Use adaptive threading
    uint32_t max_physical_threads;      // Maximum physical OS threads
    uint32_t num_logical_threads;       // Number of logical threads (always 96)
    
    // Physical Thread Pool
    pthread_t* physical_threads;        // Physical OS threads
    uint32_t num_physical_threads;      // Number of physical threads created
    
    // Work Stealing
    bool work_stealing_enabled;         // Enable work stealing
    _Atomic uint64_t total_work_stolen; // Total work items stolen
    
    // Shared Memory Pools (one per physical thread)
    void** shared_memory_pools;         // Shared memory pools
    size_t memory_pool_size;            // Size of each pool
    
} HierarchicalThreadPool;

// ============================================================================
// THREAD POOL OPERATIONS
// ============================================================================

/**
 * Create hierarchical thread pool
 * 
 * @param num_threads Number of threads to create
 * @param symmetry_fold N-fold symmetry (e.g., 12 for dodecahedron)
 * @param num_dimensions Number of dimensions for geometric organization
 * @param numa_aware Use NUMA-aware allocation
 * @return Allocated thread pool, or NULL on error
 */
HierarchicalThreadPool* hierarchical_thread_pool_create_general(
    uint32_t num_threads,
    uint32_t symmetry_fold,
    uint32_t num_dimensions,
    bool numa_aware
);

/**
 * Free hierarchical thread pool
 * 
 * @param pool Pool to free
 */
void hierarchical_thread_pool_free(HierarchicalThreadPool* pool);

/**
 * Start thread pool
 * 
 * Starts all threads in the pool.
 * 
 * @param pool Pool to start
 * @return 0 on success, -1 on error
 */
int hierarchical_thread_pool_start(HierarchicalThreadPool* pool);

/**
 * Stop thread pool
 * 
 * Stops all threads in the pool gracefully.
 * 
 * @param pool Pool to stop
 * @return 0 on success, -1 on error
 */
int hierarchical_thread_pool_stop(HierarchicalThreadPool* pool);

/**
 * Wait for thread pool to finish
 * 
 * Blocks until all threads have stopped.
 * 
 * @param pool Pool to wait for
 * @return 0 on success, -1 on error
 */
int hierarchical_thread_pool_wait(HierarchicalThreadPool* pool);

// ============================================================================
// THREAD OPERATIONS
// ============================================================================

/**
 * Create hierarchical thread
 * 
 * @param thread_id Unique thread ID
 * @param role Thread role
 * @param parent Parent thread (NULL for root)
 * @param pool Thread pool this thread belongs to
 * @return Allocated thread, or NULL on error
 */
HierarchicalThread* hierarchical_thread_create(
    uint32_t thread_id,
    ThreadRole role,
    HierarchicalThread* parent,
    HierarchicalThreadPool* pool
);

/**
 * Free hierarchical thread
 * 
 * @param thread Thread to free
 */
void hierarchical_thread_free(HierarchicalThread* thread);

/**
 * Start thread
 * 
 * @param thread Thread to start
 * @param work_fn Work function to execute
 * @param work_data Data to pass to work function
 * @return 0 on success, -1 on error
 */
int hierarchical_thread_start(
    HierarchicalThread* thread,
    void* (*work_fn)(void*),
    void* work_data
);

/**
 * Stop thread
 * 
 * @param thread Thread to stop
 * @return 0 on success, -1 on error
 */
int hierarchical_thread_stop(HierarchicalThread* thread);

/**
 * Join thread
 * 
 * @param thread Thread to join
 * @return 0 on success, -1 on error
 */
int hierarchical_thread_join(HierarchicalThread* thread);

// ============================================================================
// NEIGHBOR OPERATIONS
// ============================================================================

/**
 * Add neighbor to thread
 * 
 * Establishes a neighbor relationship between two threads.
 * Creates a shared boundary memory region.
 * 
 * @param thread Thread to add neighbor to
 * @param neighbor_id Neighbor thread ID
 * @param relationship Relationship type
 * @param distance Geometric distance
 * @param pool Thread pool (for looking up neighbor)
 * @return 0 on success, -1 on error
 */
int hierarchical_thread_add_neighbor(
    HierarchicalThread* thread,
    uint32_t neighbor_id,
    ThreadRelationType relationship,
    double distance,
    HierarchicalThreadPool* pool
);

/**
 * Get neighbor by ID
 * 
 * @param thread Thread to query
 * @param neighbor_id Neighbor thread ID
 * @return Neighbor info, or NULL if not found
 */
ThreadNeighbor* hierarchical_thread_get_neighbor(
    HierarchicalThread* thread,
    uint32_t neighbor_id
);

/**
 * Get all neighbors of a specific type
 * 
 * @param thread Thread to query
 * @param relationship Relationship type to filter by
 * @param out_neighbors Output buffer for neighbors
 * @param max_neighbors Maximum number of neighbors to return
 * @return Number of neighbors returned
 */
uint32_t hierarchical_thread_get_neighbors_by_type(
    HierarchicalThread* thread,
    ThreadRelationType relationship,
    ThreadNeighbor* out_neighbors,
    uint32_t max_neighbors
);

/**
 * Access neighbor boundary
 * 
 * Gets the shared boundary memory with a neighbor.
 * 
 * @param thread Thread to query
 * @param neighbor_id Neighbor thread ID
 * @return Shared boundary memory, or NULL if not found
 */
SharedMemoryEnhanced* hierarchical_thread_get_boundary(
    HierarchicalThread* thread,
    uint32_t neighbor_id
);

// ============================================================================
// MESSAGE OPERATIONS
// ============================================================================

/**
 * Send message to thread
 * 
 * @param sender Sender thread
 * @param receiver_id Receiver thread ID (or BROADCAST)
 * @param type Message type
 * @param priority Message priority
 * @param data Message data
 * @param data_size Data size in bytes
 * @param pool Thread pool (for routing)
 * @return 0 on success, -1 on error
 */
int hierarchical_thread_send_message(
    HierarchicalThread* sender,
    uint32_t receiver_id,
    MessageType type,
    MessagePriority priority,
    void* data,
    size_t data_size,
    HierarchicalThreadPool* pool
);

/**
 * Receive message
 * 
 * @param thread Thread to receive message
 * @param timeout_ms Timeout in milliseconds (0 = no wait, -1 = infinite)
 * @return Received message, or NULL if none available
 */
Message* hierarchical_thread_receive_message(
    HierarchicalThread* thread,
    int timeout_ms
);

/**
 * Broadcast message to all threads
 * 
 * @param sender Sender thread
 * @param type Message type
 * @param priority Message priority
 * @param data Message data
 * @param data_size Data size in bytes
 * @param pool Thread pool
 * @return Number of messages sent, or -1 on error
 */
int hierarchical_thread_broadcast_message(
    HierarchicalThread* sender,
    MessageType type,
    MessagePriority priority,
    void* data,
    size_t data_size,
    HierarchicalThreadPool* pool
);

// ============================================================================
// STATE OPERATIONS
// ============================================================================

/**
 * Change thread state
 * 
 * @param thread Thread to change state
 * @param new_state New state
 * @return Transition result
 */
TransitionResult hierarchical_thread_change_state(
    HierarchicalThread* thread,
    StateType new_state
);

/**
 * Get thread state
 * 
 * @param thread Thread to query
 * @return Current state
 */
StateType hierarchical_thread_get_state(HierarchicalThread* thread);

/**
 * Register state change callback
 * 
 * @param thread Thread to register callback for
 * @param callback Callback function
 * @param user_data User data to pass to callback
 * @return 0 on success, -1 on error
 */
int hierarchical_thread_register_state_callback(
    HierarchicalThread* thread,
    StateChangeCallback callback,
    void* user_data
);

// ============================================================================
// WORK OPERATIONS
// ============================================================================

/**
 * Submit work to thread
 * 
 * @param thread Thread to submit work to
 * @param work_fn Work function
 * @param data Work data
 * @param data_size Data size in bytes
 * @param priority Work priority
 * @return Work item ID, or 0 on error
 */
uint64_t hierarchical_thread_submit_work(
    HierarchicalThread* thread,
    void (*work_fn)(void*),
    void* data,
    size_t data_size,
    WorkPriority priority
);

/**
 * Get work from thread pool
 * 
 * Tries to get work from local queue first, then steals from neighbors.
 * 
 * @param thread Thread requesting work
 * @return Work item, or NULL if none available
 */
WorkItem* hierarchical_thread_get_work(HierarchicalThread* thread);

/**
 * Complete work item
 * 
 * @param thread Thread completing work
 * @param item Work item to complete
 * @param success Whether work completed successfully
 * @return 0 on success, -1 on error
 */
int hierarchical_thread_complete_work(
    HierarchicalThread* thread,
    WorkItem* item,
    bool success
);

// ============================================================================
// MEMORY OPERATIONS
// ============================================================================

/**
 * Allocate thread-local memory
 * 
 * @param thread Thread to allocate for
 * @param size Size in bytes
 * @return Allocated memory, or NULL on error
 */
void* hierarchical_thread_alloc_local(HierarchicalThread* thread, size_t size);

/**
 * Allocate shared memory with parent
 * 
 * @param thread Thread to allocate for
 * @param size Size in bytes
 * @param mode Access mode
 * @return Allocated shared memory, or NULL on error
 */
SharedMemoryEnhanced* hierarchical_thread_alloc_parent_shared(
    HierarchicalThread* thread,
    size_t size,
    SharedMemoryAccessMode mode
);

/**
 * Allocate shared memory with child
 * 
 * @param thread Thread to allocate for
 * @param child_id Child thread ID
 * @param size Size in bytes
 * @param mode Access mode
 * @return Allocated shared memory, or NULL on error
 */
SharedMemoryEnhanced* hierarchical_thread_alloc_child_shared(
    HierarchicalThread* thread,
    uint32_t child_id,
    size_t size,
    SharedMemoryAccessMode mode
);

/**
 * Access neighbor boundary memory
 * 
 * @param thread Thread accessing boundary
 * @param neighbor_id Neighbor thread ID
 * @param mode Access mode
 * @return Boundary memory, or NULL on error
 */
void* hierarchical_thread_access_boundary(
    HierarchicalThread* thread,
    uint32_t neighbor_id,
    SharedMemoryAccessMode mode
);

// ============================================================================
// STATISTICS & MONITORING
// ============================================================================

/**
 * Thread statistics
 */
typedef struct {
    uint32_t thread_id;
    ThreadRole role;
    StateType current_state;
    
    // Hierarchy
    uint32_t num_children;
    uint32_t num_neighbors;
    
    // Messages
    uint64_t messages_sent;
    uint64_t messages_received;
    uint64_t messages_pending;
    
    // Work
    uint64_t work_completed;
    uint64_t work_stolen;
    uint64_t work_pending;
    
    // Memory
    size_t local_memory_used;
    size_t shared_memory_used;
    size_t boundary_memory_used;
    
    // Timing
    uint64_t total_runtime;
    uint64_t state_durations[10];  // Duration in each standard state
    
    // Performance
    double cpu_utilization;
    double work_throughput;
    double message_latency;
    
} HierarchicalThreadStats;

/**
 * Get thread statistics
 * 
 * @param thread Thread to query
 * @param stats Output statistics
 * @return 0 on success, -1 on error
 */
int hierarchical_thread_get_stats(
    HierarchicalThread* thread,
    HierarchicalThreadStats* stats
);

/**
 * Pool statistics
 */
typedef struct {
    uint32_t num_threads;
    uint32_t num_levels;
    uint32_t symmetry_fold;
    
    // Aggregate statistics
    uint64_t total_messages;
    uint64_t total_work_items;
    uint64_t total_state_changes;
    
    // Memory usage
    size_t total_memory_used;
    size_t total_shared_memory;
    size_t total_boundary_memory;
    
    // Performance
    double avg_cpu_utilization;
    double avg_work_throughput;
    double avg_message_latency;
    double load_balance_factor;  // 1.0 = perfect balance
    
} HierarchicalThreadPoolStats;

/**
 * Get pool statistics
 * 
 * @param pool Pool to query
 * @param stats Output statistics
 * @return 0 on success, -1 on error
 */
int hierarchical_thread_pool_get_stats(
    HierarchicalThreadPool* pool,
    HierarchicalThreadPoolStats* stats
);

/**
 * Print thread statistics
 * 
 * @param thread Thread to print stats for
 */
void hierarchical_thread_print_stats(HierarchicalThread* thread);

/**
 * Print pool statistics
 * 
 * @param pool Pool to print stats for
 */
void hierarchical_thread_pool_print_stats(HierarchicalThreadPool* pool);

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * Get thread by ID
 * 
 * @param pool Thread pool
 * @param thread_id Thread ID to find
 * @return Thread, or NULL if not found
 */
HierarchicalThread* hierarchical_thread_pool_get_thread(
    HierarchicalThreadPool* pool,
    uint32_t thread_id
);

/**
 * Calculate geometric distance between threads
 * 
 * @param thread1 First thread
 * @param thread2 Second thread
 * @return Euclidean distance in N-dimensional space
 */
double hierarchical_thread_distance(
    HierarchicalThread* thread1,
    HierarchicalThread* thread2
);

/**
 * Find nearest neighbors
 * 
 * Finds the K nearest neighbors to a thread based on geometric distance.
 * 
 * @param thread Thread to find neighbors for
 * @param pool Thread pool
 * @param k Number of neighbors to find
 * @param out_neighbors Output buffer for neighbor IDs
 * @return Number of neighbors found
 */
uint32_t hierarchical_thread_find_nearest_neighbors(
    HierarchicalThread* thread,
    HierarchicalThreadPool* pool,
    uint32_t k,
    uint32_t* out_neighbors
);

// ============================================================================
// 88D-SPECIFIC FUNCTIONS
// ============================================================================

/**
 * Create hierarchical thread pool (88D specialized version)
 * 
 * Creates a thread pool with 88D organization (8 layers × 11 dimensions).
 * This is a convenience wrapper around the general hierarchical_thread_pool_create.
 * 
 * @param base Abacus base (typically 60 for base-60)
 * @return Thread pool or NULL on error
 */
HierarchicalThreadPool* hierarchical_thread_pool_create(uint32_t base);

/**
 * Get thread by position
 * 
 * @param pool Thread pool
 * @param layer Layer (0-7)
 * @param dimension Dimension (0-10)
 * @return Thread or NULL if not found
 */
HierarchicalThread* hierarchical_thread_get(
    HierarchicalThreadPool* pool,
    uint8_t layer,
    uint8_t dimension
);

/**
 * Synchronize layer
 * 
 * All threads in the layer wait at barrier.
 * 
 * @param pool Thread pool
 * @param layer Layer to synchronize
 * @return 0 on success, -1 on error
 */
int hierarchical_thread_sync_layer(
    HierarchicalThreadPool* pool,
    uint8_t layer
);

/**
 * Synchronize all threads
 * 
 * All threads in the pool wait at global barrier.
 * 
 * @param pool Thread pool
 * @return 0 on success, -1 on error
 */
int hierarchical_thread_sync_all(HierarchicalThreadPool* pool);

/**
 * Notify boundary crossing
 * 
 * Called when a thread crosses a geometric boundary.
 * 
 * @param thread Thread that crossed
 * @param from_layer Source layer
 * @param to_layer Target layer
 * @return 0 on success, -1 on error
 */
int hierarchical_thread_notify_boundary_crossing(
    HierarchicalThread* thread,
    uint8_t from_layer,
    uint8_t to_layer
);

/**
 * Notify twin prime detection
 * 
 * Called when a thread detects a twin prime.
 * 
 * @param thread Thread that detected
 * @param prime1 First prime
 * @param prime2 Second prime
 * @return 0 on success, -1 on error
 */
int hierarchical_thread_notify_twin_prime(
    HierarchicalThread* thread,
    uint64_t prime1,
    uint64_t prime2
);

/**
 * Get sibling threads
 * 
 * Returns all threads in the same layer.
 * 
 * @param thread Thread
 * @param out_siblings Output buffer for sibling pointers
 * @param max_siblings Maximum siblings to return
 * @return Number of siblings found
 */
uint32_t hierarchical_thread_get_siblings(
    HierarchicalThread* thread,
    HierarchicalThread** out_siblings,
    uint32_t max_siblings
);

/**
 * Attach sphere group
 * 
 * Attaches another sphere group as a child (self-similar nesting).
 * 
 * @param parent Parent sphere group
 * @param child Child sphere group to attach
 * @return 0 on success, -1 on error
 */
int hierarchical_thread_pool_attach_group(
    HierarchicalThreadPool* parent,
    HierarchicalThreadPool* child
);

/**
 * Detach sphere group
 * 
 * Detaches a child sphere group.
 * 
 * @param parent Parent sphere group
 * @param child Child sphere group to detach
 * @return 0 on success, -1 on error
 */
int hierarchical_thread_pool_detach_group(
    HierarchicalThreadPool* parent,
    HierarchicalThreadPool* child
);

/**
 * Get 88D statistics
 * 
 * Returns statistics specific to 88D organization.
 * 
 * @param pool Thread pool
 * @param out_boundary_crossings Output: total boundary crossings
 * @param out_twin_primes Output: total twin primes found
 * @param out_operations Output: total operations
 * @return 0 on success, -1 on error
 */
int hierarchical_thread_pool_get_88d_stats(
    HierarchicalThreadPool* pool,
    uint64_t* out_boundary_crossings,
    uint64_t* out_twin_primes,
    uint64_t* out_operations
);

/**
 * 88D Worker loop (internal)
 * Main loop that processes work items in worker threads
 * 
 * @param arg HierarchicalThread pointer
 * @return NULL
 */
void* hierarchical_thread_worker(void* arg);

// ============================================================================
// WORK QUEUE OPERATIONS
// ============================================================================

// ============================================================================
// OPTIMIZER OPERATIONS (DAY 10)
// ============================================================================

/**
 * Clear all gradients in thread
 * 
 * @param thread Thread to clear gradients for
 * @return 0 on success, -1 on error
 */
int worker_clear_gradients(HierarchicalThread* thread);

/**
 * Apply optimizer to thread's parameters
 * 
 * Uses Adam optimizer:
 *   m = β1*m + (1-β1)*grad
 *   v = β2*v + (1-β2)*grad²
 *   param -= lr * m / (√v + ε)
 * 
 * @param thread Thread that owns the parameters
 * @param learning_rate Learning rate
 * @param beta1 Adam beta1 (momentum decay, default 0.9)
 * @param beta2 Adam beta2 (velocity decay, default 0.999)
 * @param epsilon Adam epsilon (numerical stability, default 1e-8)
 * @return 0 on success, -1 on error
 */
int worker_apply_optimizer(
    HierarchicalThread* thread,
    double learning_rate,
    double beta1,
    double beta2,
    double epsilon
);

// ============================================================================
// GRADIENT OPERATIONS (DAY 9)
// ============================================================================

/**
 * Compute gradients for FFN parameters
 * 
 * @param thread Thread that owns the computation
 * @param grad_output Gradient from next layer (double array)
 * @param input Input that was used in forward pass (double array)
 * @param hidden Hidden layer from forward pass (double array)
 * @param embedding_dim Embedding dimension
 * @param hidden_dim Hidden dimension
 * @param grad_input Gradient to pass to previous layer (output)
 * @return 0 on success, -1 on error
 */
int worker_compute_ffn_gradients(
    HierarchicalThread* thread,
    const double* grad_output,
    const double* input,
    const double* hidden,
    uint32_t embedding_dim,
    uint32_t hidden_dim,
    double* grad_input
);

/**
 * Compute gradients for attention parameters
 * 
 * @param thread Thread that owns the computation
 * @param grad_output Gradient from next layer
 * @param Q Query matrix from forward pass
 * @param K Key matrix from forward pass
 * @param V Value matrix from forward pass
 * @param input Input from forward pass
 * @param embedding_dim Embedding dimension
 * @param grad_input Gradient to pass to previous layer (output)
 * @return 0 on success, -1 on error
 */
int worker_compute_attention_gradients(
    HierarchicalThread* thread,
    const double* grad_output,
    const double* Q,
    const double* K,
    const double* V,
    const double* input,
    uint32_t embedding_dim,
    double* grad_input
);

/**
 * Compute gradients for embeddings
 * 
 * @param thread Thread that owns the embedding
 * @param token_id Token ID
 * @param grad_output Gradient from next layer
 * @param embedding_dim Embedding dimension
 * @return 0 on success, -1 on error
 */
int worker_compute_embedding_gradients(
    HierarchicalThread* thread,
    uint32_t token_id,
    const double* grad_output,
    uint32_t embedding_dim
);

// ============================================================================
// FFN OPERATIONS (DAY 8)
// ============================================================================

/**
 * Compute FFN (Feed-Forward Network) using thread's local parameters
 * 
 * Computes:
 *   hidden = ReLU(input × W_ffn1)
 *   output = hidden × W_ffn2
 * 
 * @param thread Thread that owns the computation
 * @param input Input from attention (double array)
 * @param embedding_dim Embedding dimension
 * @param hidden_dim Hidden dimension
 * @param output Output (double array, pre-allocated)
 * @return 0 on success, -1 on error
 */
int worker_compute_ffn(
    HierarchicalThread* thread,
    const double* input,
    uint32_t embedding_dim,
    uint32_t hidden_dim,
    double* output
);

// ============================================================================
// ATTENTION OPERATIONS (DAY 7)
// ============================================================================

/**
 * Compute Q, K, V for a token using thread's local parameters
 * 
 * @param thread Thread that owns the computation
 * @param input Input embedding (double array)
 * @param embedding_dim Embedding dimension
 * @param Q Output Q (double array, pre-allocated)
 * @param K Output K (double array, pre-allocated)
 * @param V Output V (double array, pre-allocated)
 * @return 0 on success, -1 on error
 */
int worker_compute_qkv(
    HierarchicalThread* thread,
    const double* input,
    uint32_t embedding_dim,
    double* Q,
    double* K,
    double* V
);

/**
 * Share K, V with neighbors via shared boundaries
 * 
 * @param thread Thread sharing its K, V
 * @param K K matrix to share (double array)
 * @param V V matrix to share (double array)
 * @param dim Dimension of K and V
 * @return 0 on success, -1 on error
 */
int worker_share_kv(
    HierarchicalThread* thread,
    const double* K,
    const double* V,
    uint32_t dim
);

/**
 * Collect K, V from neighbors via shared boundaries
 * 
 * @param thread Thread collecting K, V
 * @param neighbor_K Array of K matrices from neighbors (pre-allocated)
 * @param neighbor_V Array of V matrices from neighbors (pre-allocated)
 * @param dim Dimension of K and V
 * @param max_neighbors Maximum number of neighbors
 * @return Number of neighbors collected, or -1 on error
 */
int worker_collect_neighbor_kv(
    HierarchicalThread* thread,
    double** neighbor_K,
    double** neighbor_V,
    uint32_t dim,
    uint32_t max_neighbors
);

// ============================================================================
// EMBEDDING OPERATIONS (DAY 6)
// ============================================================================

/**
 * Get embedding for a token from thread's local storage
 * 
 * This function retrieves the embedding for a specific token from the thread
 * that owns it. The embedding is stored in the thread's parameter storage
 * as a CrystallineAbacus.
 * 
 * @param thread Thread that owns the token's embedding
 * @param token_id Token ID to get embedding for
 * @param output Output buffer (CrystallineAbacus) to store the embedding
 * @return 0 on success, -1 on error
 */
int worker_get_embedding(
    HierarchicalThread* thread,
    uint32_t token_id,
    CrystallineAbacus* output
);

// ============================================================================
// WORK QUEUE OPERATIONS
// ============================================================================

/**
 * Enqueue work item to thread's work queue
 * 
 * @param thread Target thread
 * @param type Work type (forward or backward)
 * @param token_id Token ID to process
 * @param target_id Target token ID (for backward pass)
 * @return 0 on success, -1 on error
 */
int hierarchical_thread_enqueue_work(
    HierarchicalThread* thread,
    TrainingWorkType type,
    uint32_t token_id,
    uint32_t target_id
);

/**
 * Dequeue work item from thread's work queue
 * 
 * @param thread Source thread
 * @return Work item or NULL if queue is empty
 */
TrainingWorkItem* hierarchical_thread_dequeue_work(HierarchicalThread* thread);

/**
 * Free work item
 * 
 * @param item Work item to free
 */
void hierarchical_thread_free_work_item(TrainingWorkItem* item);

/**
 * Get work queue size
 * 
 * @param thread Thread
 * @return Number of items in work queue
 */
uint32_t hierarchical_thread_get_work_queue_size(HierarchicalThread* thread);

#ifdef __cplusplus
}
#endif

// ============================================================================
// INFERENCE HELPER FUNCTIONS (DAY 12)
// ============================================================================

/**
 * Collect logits from Layer 7 threads
 * 
 * This function collects the output values from all Layer 7 worker threads
 * and concatenates them into a single logits array.
 * 
 * @param pool The 88D thread pool
 * @param logits Output array to store logits (must be pre-allocated)
 * @param vocab_size Total vocabulary size
 * @return 0 on success, -1 on error
 */
int collect_logits_from_layer7(HierarchicalThreadPool* pool, 
                                double* logits,
                                uint32_t vocab_size);

/**
 * Apply temperature scaling to logits
 * 
 * Divides all logits by the temperature parameter to control randomness.
 * Higher temperature = more random, lower temperature = more deterministic.
 * 
 * @param logits The logits array (modified in place)
 * @param vocab_size Size of vocabulary
 * @param temperature Temperature parameter (> 0)
 * @return 0 on success, -1 on error
 */
int apply_temperature_to_logits(double* logits,
                                 uint32_t vocab_size,
                                 double temperature);

/**
 * Apply softmax to logits to get probabilities
 * 
 * Converts logits to probabilities using softmax:
 * P(i) = exp(logit[i]) / sum(exp(logit[j]) for all j)
 * 
 * @param logits The logits array (modified in place to probabilities)
 * @param vocab_size Size of vocabulary
 * @return 0 on success, -1 on error
 */
int apply_softmax_to_logits(double* logits, uint32_t vocab_size);

#endif // HIERARCHICAL_THREADING_H