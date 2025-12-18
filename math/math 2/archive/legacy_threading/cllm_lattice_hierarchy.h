#ifndef CLLM_LATTICE_HIERARCHY_H
#define CLLM_LATTICE_HIERARCHY_H

#include <stdint.h>
#include <pthread.h>
#include <stdatomic.h>
#include "lattice_sphere_positions.h"
#include "ai/cllm_sphere_stats.h"
#include "ai/cllm_message_queue.h"
#include "../../algorithms/include/hierarchical_prime_partitions.h"
#include "ai/cllm_shared_memory.h"
#include "ai/cllm_thread_allocation.h"
#include "ai/space_88d.h"

/**
 * Lattice Hierarchy Core
 * 
 * Represents a sphere in the hierarchical "Kissing Spheres" architecture.
 * Each sphere:
 * - Belongs to one or more symmetry groups (0-11)
 * - Has a parent sphere (except root)
 * - Has up to 12 child spheres
 * - Has 11 sibling spheres
 * - Processes primes in its assigned symmetry group(s)
 * - Maintains its own gradient buffers
 * - Communicates via lock-free message queues
 */

// Forward declarations
struct CLLMLatticeHierarchy;
typedef struct CLLMLatticeHierarchy CLLMLatticeHierarchy;

// ============================================================================
// HIERARCHY STATE
// ============================================================================

/**
 * Hierarchy State
 * 
 * Tracks the current state of a sphere in the hierarchy.
 */
typedef enum {
    HIERARCHY_STATE_INITIALIZING,   // Being initialized
    HIERARCHY_STATE_READY,           // Ready to process
    HIERARCHY_STATE_PROCESSING,      // Currently processing (leaf worker)
    HIERARCHY_STATE_CONTROLLING,     // Controlling children (non-leaf)
    HIERARCHY_STATE_WAITING,         // Waiting for synchronization
    HIERARCHY_STATE_ACCUMULATING,    // Accumulating gradients
    HIERARCHY_STATE_UPDATING,        // Updating weights
    HIERARCHY_STATE_IDLE,            // Idle (no work)
    HIERARCHY_STATE_TERMINATING,     // Being terminated
    HIERARCHY_STATE_TERMINATED       // Terminated
} HierarchyState;

/**
 * Synchronization Barrier
 * 
 * Used for coordinating between spheres at the same level.
 */
typedef struct {
    pthread_barrier_t barrier;       // POSIX barrier
    atomic_int arrived;              // Number of spheres arrived
    atomic_int required;             // Number of spheres required
    atomic_int generation;           // Barrier generation (for reuse)
} SyncBarrier;

// ============================================================================
// LATTICE HIERARCHY STRUCTURE
// ============================================================================

/**
 * CLLM Lattice Hierarchy
 * 
 * Complete sphere structure in the hierarchical architecture.
 */
struct CLLMLatticeHierarchy {
    // ========================================================================
    // IDENTITY
    // ========================================================================
    
    int sphere_id;                   // Unique sphere ID
    int hierarchy_level;             // Level in hierarchy (0 = root)
    
    // Symmetry groups (0-11)
    int num_symmetry_groups;         // Number of groups assigned
    int symmetry_groups[12];         // Assigned symmetry groups
    int primary_symmetry_group;      // Primary group (for single-group spheres)
    
    // Thread assignment
    int physical_thread_id;          // Physical thread ID
    pthread_t thread;                // Thread handle
    atomic_int thread_running;       // 1 if thread is running
    
    // ========================================================================
    // HIERARCHY RELATIONSHIPS
    // ========================================================================
    
    // Parent sphere (NULL for root)
    CLLMLatticeHierarchy* parent;
    
    // Child spheres (up to 12, one per symmetry group)
    int num_children;
    CLLMLatticeHierarchy* children[12];
    
    // Sibling spheres (up to 11 others at same level)
    int num_siblings;
    CLLMLatticeHierarchy* siblings[11];
    
    // ========================================================================
    // POSITION & GEOMETRY
    // ========================================================================
    
    SpherePosition* position;        // Position in lattice
    LatticePartition* partition;     // Lattice partition owned by this sphere
    
    // ========================================================================
    // 88D COMPUTATIONAL SPACE
    // ========================================================================
    
    Space88D* space;                 // 88-dimensional computational space
    uint64_t primes_generated;       // Total primes generated
    uint64_t current_prime;          // Current prime being processed
    
    // ========================================================================
    // STATE & SYNCHRONIZATION
    // ========================================================================
    
    atomic_int state;                // Current state (HierarchyState)
    
    // Mutexes for critical sections
    pthread_mutex_t state_mutex;     // Protects state transitions
    pthread_mutex_t children_mutex;  // Protects children array
    pthread_mutex_t gradient_mutex;  // Protects gradient operations
    
    // Condition variables for signaling
    pthread_cond_t state_changed;    // Signals state changes
    pthread_cond_t work_available;   // Signals work availability
    
    // Barriers for synchronization
    SyncBarrier* level_barrier;      // Barrier for this level
    SyncBarrier* epoch_barrier;      // Barrier for epoch completion
    
    // ========================================================================
    // MESSAGE PASSING
    // ========================================================================
    
    LockFreeMessageQueue* inbox;     // Incoming messages
    LockFreeMessageQueue* outbox;    // Outgoing messages (for debugging)
    
    atomic_uint_fast64_t messages_sent;     // Total messages sent
    atomic_uint_fast64_t messages_received; // Total messages received
    
    // ========================================================================
    // SHARED MEMORY
    // ========================================================================
    
    // Read-only access to parent's data
    SharedMemoryRegion* parent_weights;      // Parent's weights (read-only)
    SharedMemoryRegion* parent_lattice;      // Parent's lattice (read-only)
    
    // This sphere's shared data (for children to read)
    SharedMemoryRegion* shared_weights;      // This sphere's weights
    SharedMemoryRegion* shared_lattice;      // This sphere's lattice
    
    // ========================================================================
    // GRADIENT BUFFERS
    // ========================================================================
    
    // This sphere's gradient buffer
    float* gradient_buffer;          // Local gradient accumulation
    size_t gradient_buffer_size;     // Size of gradient buffer
    atomic_int gradient_ready;       // 1 if gradients ready
    
    // Accumulated gradients from children
    float* child_gradients;          // Accumulated child gradients
    atomic_int children_gradients_ready; // Count of children ready
    
    // ========================================================================
    // WORK MANAGEMENT
    // ========================================================================
    
    // Work queue for this sphere
    uint64_t* work_queue;            // Array of work items (batch indices)
    size_t work_queue_capacity;      // Capacity of work queue
    atomic_size_t work_queue_head;   // Head index (dequeue)
    atomic_size_t work_queue_tail;   // Tail index (enqueue)
    atomic_size_t work_queue_size;   // Current size
    
    // Work stealing
    atomic_int work_stealing_enabled; // 1 if work stealing enabled
    atomic_uint_fast64_t work_stolen_from; // Times work stolen from this sphere
    atomic_uint_fast64_t work_stolen_to;   // Times this sphere stole work
    
    // ========================================================================
    // STATISTICS
    // ========================================================================
    
    SphereStatistics stats;          // Performance statistics
    
    // ========================================================================
    // BOUNDARY AWARENESS
    // ========================================================================
    
    atomic_int near_144000_boundary; // 1 if currently near 144000
    atomic_uint_fast64_t boundary_crossings; // Times crossed 144000
    atomic_uint_fast64_t twin_prime_hits;    // Times hit twin primes
    
    // ========================================================================
    // CONFIGURATION
    // ========================================================================
    
    size_t batch_size;               // Batch size for processing
    int enable_work_stealing;        // Enable work stealing
    int enable_recursive_spawning;   // Enable recursive child spawning
    int max_hierarchy_depth;         // Maximum hierarchy depth
    
    // ========================================================================
    // DEBUGGING & PROFILING
    // ========================================================================
    
    uint64_t creation_time_ns;       // When sphere was created
    uint64_t start_time_ns;          // When processing started
    uint64_t total_processing_time_ns; // Total processing time
    
    // User data (for application-specific context)
    void* user_data;                 // Application-specific data pointer

    char debug_name[64];             // Debug name for logging
};

// ============================================================================
// HIERARCHY CREATION & DESTRUCTION
// ============================================================================

/**
 * Create a new lattice hierarchy sphere
 * 
 * @param sphere_id Unique sphere ID
 * @param hierarchy_level Level in hierarchy (0 = root)
 * @param symmetry_groups Array of symmetry groups (0-11)
 * @param num_symmetry_groups Number of symmetry groups
 * @param physical_thread_id Physical thread ID
 * @param parent Parent sphere (NULL for root)
 * @return Pointer to new sphere, or NULL on failure
 */
CLLMLatticeHierarchy* lattice_hierarchy_create(
    int sphere_id,
    int hierarchy_level,
    const int* symmetry_groups,
    int num_symmetry_groups,
    int physical_thread_id,
    CLLMLatticeHierarchy* parent
);

/**
 * Free a lattice hierarchy sphere
 * 
 * WARNING: This function is NOT thread-safe. Ensure the sphere's
 * thread has been stopped before calling this function.
 * 
 * @param sphere Sphere to free
 */
void lattice_hierarchy_free(CLLMLatticeHierarchy* sphere);

// ============================================================================
// HIERARCHY RELATIONSHIPS
// ============================================================================

/**
 * Add a child sphere
 * 
 * @param parent Parent sphere
 * @param child Child sphere
 * @return 1 on success, 0 on failure
 */
int lattice_hierarchy_add_child(CLLMLatticeHierarchy* parent,
                                CLLMLatticeHierarchy* child);

/**
 * Remove a child sphere
 * 
 * @param parent Parent sphere
 * @param child Child sphere
 * @return 1 on success, 0 on failure
 */
int lattice_hierarchy_remove_child(CLLMLatticeHierarchy* parent,
                                   CLLMLatticeHierarchy* child);

/**
 * Add a sibling sphere
 * 
 * @param sphere This sphere
 * @param sibling Sibling sphere
 * @return 1 on success, 0 on failure
 */
int lattice_hierarchy_add_sibling(CLLMLatticeHierarchy* sphere,
                                  CLLMLatticeHierarchy* sibling);

/**
 * Discover and link all siblings at the same level
 * 
 * @param spheres Array of all spheres at this level
 * @param num_spheres Number of spheres
 */
void lattice_hierarchy_discover_siblings(CLLMLatticeHierarchy** spheres,
                                        int num_spheres);

/**
 * Get child sphere for specific symmetry group
 * 
 * @param parent Parent sphere
 * @param symmetry_group Symmetry group (0-11)
 * @return Child sphere, or NULL if not found
 */
CLLMLatticeHierarchy* lattice_hierarchy_get_child_for_group(
    CLLMLatticeHierarchy* parent,
    int symmetry_group
);

/**
 * Get sibling sphere for specific symmetry group
 * 
 * @param sphere This sphere
 * @param symmetry_group Symmetry group (0-11)
 * @return Sibling sphere, or NULL if not found
 */
CLLMLatticeHierarchy* lattice_hierarchy_get_sibling_for_group(
    CLLMLatticeHierarchy* sphere,
    int symmetry_group
);

// ============================================================================
// STATE MANAGEMENT
// ============================================================================

/**
 * Get current state
 * 
 * @param sphere Sphere to query
 * @return Current state
 */
HierarchyState lattice_hierarchy_get_state(const CLLMLatticeHierarchy* sphere);

/**
 * Set state
 * 
 * Thread-safe state transition with signaling.
 * 
 * @param sphere Sphere to update
 * @param new_state New state
 */
void lattice_hierarchy_set_state(CLLMLatticeHierarchy* sphere,
                                HierarchyState new_state);

/**
 * Wait for specific state
 * 
 * Blocks until sphere reaches specified state.
 * 
 * @param sphere Sphere to wait on
 * @param target_state Target state
 * @param timeout_ms Timeout in milliseconds (0 = no timeout)
 * @return 1 if state reached, 0 on timeout
 */
int lattice_hierarchy_wait_for_state(CLLMLatticeHierarchy* sphere,
                                    HierarchyState target_state,
                                    int timeout_ms);

/**
 * Get state name
 * 
 * @param state State to get name for
 * @return String representation of state
 */
const char* lattice_hierarchy_state_name(HierarchyState state);

// ============================================================================
// SYNCHRONIZATION
// ============================================================================

/**
 * Create a synchronization barrier
 * 
 * @param num_spheres Number of spheres that will use this barrier
 * @return Pointer to barrier, or NULL on failure
 */
SyncBarrier* sync_barrier_create(int num_spheres);

/**
 * Free a synchronization barrier
 * 
 * @param barrier Barrier to free
 */
void sync_barrier_free(SyncBarrier* barrier);

/**
 * Wait at barrier
 * 
 * Blocks until all spheres have arrived at the barrier.
 * 
 * @param barrier Barrier to wait at
 * @return 1 on success, 0 on failure
 */
int sync_barrier_wait(SyncBarrier* barrier);

/**
 * Reset barrier for reuse
 * 
 * @param barrier Barrier to reset
 */
void sync_barrier_reset(SyncBarrier* barrier);

// ============================================================================
// MESSAGE PASSING
// ============================================================================

/**
 * Send message to sphere
 * 
 * @param sender Sending sphere
 * @param receiver Receiving sphere
 * @param message Message to send
 * @return 1 on success, 0 on failure
 */
int lattice_hierarchy_send_message(CLLMLatticeHierarchy* sender,
                                   CLLMLatticeHierarchy* receiver,
                                   SphereMessage* message);

/**
 * Broadcast message to all siblings
 * 
 * @param sender Sending sphere
 * @param message Message to broadcast
 * @return Number of siblings message was sent to
 */
int lattice_hierarchy_broadcast_to_siblings(CLLMLatticeHierarchy* sender,
                                           SphereMessage* message);

/**
 * Broadcast message to all children
 * 
 * @param parent Parent sphere
 * @param message Message to broadcast
 * @return Number of children message was sent to
 */
int lattice_hierarchy_broadcast_to_children(CLLMLatticeHierarchy* parent,
                                           SphereMessage* message);

/**
 * Process incoming messages
 * 
 * Processes all messages in the inbox.
 * 
 * @param sphere Sphere to process messages for
 * @return Number of messages processed
 */
int lattice_hierarchy_process_messages(CLLMLatticeHierarchy* sphere);

// ============================================================================
// WORK MANAGEMENT
// ============================================================================

/**
 * Add work to queue
 * 
 * @param sphere Sphere to add work to
 * @param work_item Work item (batch index)
 * @return 1 on success, 0 if queue full
 */
int lattice_hierarchy_add_work(CLLMLatticeHierarchy* sphere,
                              uint64_t work_item);

/**
 * Get work from queue
 * 
 * @param sphere Sphere to get work from
 * @param work_item Pointer to store work item
 * @return 1 if work available, 0 if queue empty
 */
int lattice_hierarchy_get_work(CLLMLatticeHierarchy* sphere,
                              uint64_t* work_item);

/**
 * Steal work from sibling
 * 
 * @param thief Sphere stealing work
 * @param victim Sphere to steal from
 * @param work_item Pointer to store stolen work item
 * @return 1 if work stolen, 0 if no work available
 */
int lattice_hierarchy_steal_work(CLLMLatticeHierarchy* thief,
                                CLLMLatticeHierarchy* victim,
                                uint64_t* work_item);

/**
 * Get work queue size
 * 
 * @param sphere Sphere to query
 * @return Number of work items in queue
 */
size_t lattice_hierarchy_work_queue_size(const CLLMLatticeHierarchy* sphere);

// ============================================================================
// BOUNDARY AWARENESS
// ============================================================================

/**
 * Check if sphere is near 144000 boundary
 * 
 * @param sphere Sphere to check
 * @return 1 if near boundary, 0 otherwise
 */
int lattice_hierarchy_is_near_boundary(const CLLMLatticeHierarchy* sphere);

/**
 * Notify boundary crossing
 * 
 * Called when a prime crosses the 144000 boundary.
 * 
 * @param sphere Sphere that crossed boundary
 * @param prime Prime that crossed
 */
void lattice_hierarchy_notify_boundary_crossing(CLLMLatticeHierarchy* sphere,
                                               uint64_t prime);

/**
 * Notify twin prime hit
 * 
 * Called when a twin prime (143999 or 144001) is encountered.
 * 
 * @param sphere Sphere that hit twin prime
 * @param prime Twin prime value
 */
void lattice_hierarchy_notify_twin_prime(CLLMLatticeHierarchy* sphere,
                                        uint64_t prime);

// ============================================================================
// UTILITIES
// ============================================================================

/**
 * Print hierarchy information
 * 
 * @param sphere Sphere to print
 */
void lattice_hierarchy_print(const CLLMLatticeHierarchy* sphere);

/**
 * Print detailed hierarchy information
 * 
 * @param sphere Sphere to print
 */
void lattice_hierarchy_print_detailed(const CLLMLatticeHierarchy* sphere);

/**
 * Print hierarchy tree
 * 
 * Prints the entire hierarchy tree starting from this sphere.
 * 
 * @param sphere Root sphere
 * @param indent Indentation level
 */
void lattice_hierarchy_print_tree(const CLLMLatticeHierarchy* sphere,
                                 int indent);

/**
 * Validate hierarchy
 * 
 * Checks for consistency and correctness.
 * 
 * @param sphere Sphere to validate
 * @return 1 if valid, 0 if errors found
 */
int lattice_hierarchy_validate(const CLLMLatticeHierarchy* sphere);

/**
 * Get hierarchy depth
 * 
 * Returns the maximum depth of the hierarchy tree.
 * 
 * @param sphere Root sphere
 * @return Maximum depth
 */
int lattice_hierarchy_get_depth(const CLLMLatticeHierarchy* sphere);

/**
 * Count total spheres in hierarchy
 * 
 * @param sphere Root sphere
 * @return Total number of spheres
 */
int lattice_hierarchy_count_spheres(const CLLMLatticeHierarchy* sphere);

// ============================================================================
// DEPTH MANAGEMENT
// ============================================================================

/**
 * Calculate maximum hierarchy depth based on available cores
 * 
 * Determines the optimal maximum depth for the hierarchy tree based on
 * the number of available CPU cores. Uses the formula:
 * 
 * max_depth = floor(log12(cores)) + 1
 * 
 * This ensures that at maximum depth, we don't exceed available cores.
 * 
 * Examples:
 * - 12 cores -> depth 2 (1 + 12 threads)
 * - 144 cores -> depth 3 (1 + 12 + 144 threads)
 * - 1728 cores -> depth 4 (1 + 12 + 144 + 1728 threads)
 * 
 * @param available_cores Number of available CPU cores
 * @return Maximum recommended hierarchy depth (minimum 1, maximum 5)
 */
int calculate_max_depth(int available_cores);

/**
 * Check if spawning is allowed at current depth
 * 
 * Determines if a sphere at the given depth can spawn children based on:
 * 1. Current depth vs. maximum allowed depth (MAX_HIERARCHY_DEPTH = 5)
 * 2. Available CPU cores for the next level
 * 3. Total thread count limit (144,000)
 * 
 * @param current_depth Current hierarchy depth (0 = root)
 * @param available_cores Number of available CPU cores
 * @param current_thread_count Current total number of threads
 * @return 1 if spawning is allowed, 0 otherwise
 */
int can_spawn_at_depth(int current_depth, int available_cores, int current_thread_count);

/**
 * Get recommended number of children for current depth
 * 
 * Calculates the optimal number of children to spawn based on:
 * - Available CPU cores
 * - Current hierarchy depth
 * - Workload requirements
 * 
 * Always returns a value between 1 and 12 to maintain 12-fold symmetry.
 * 
 * @param current_depth Current hierarchy depth
 * @param available_cores Number of available CPU cores
 * @param pending_batches Number of pending work batches
 * @return Recommended number of children (1-12)
 */
int get_recommended_children_count(int current_depth, int available_cores, int pending_batches);

#endif // CLLM_LATTICE_HIERARCHY_H