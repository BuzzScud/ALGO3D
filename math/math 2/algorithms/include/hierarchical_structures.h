#ifndef HIERARCHICAL_STRUCTURES_H
#define HIERARCHICAL_STRUCTURES_H

#include <stdint.h>
#include <stddef.h>
#include <pthread.h>
#include <stdatomic.h>
#include "pthread_barrier_compat.h"  // For macOS compatibility

/**
 * Generic Hierarchical Structures
 * 
 * Provides generic tree structures, synchronization barriers, and work queues
 * extracted from the CLLM lattice hierarchy implementation.
 * 
 * This module provides broadly applicable algorithms for:
 * - Hierarchical tree management (parent-child-sibling relationships)
 * - Synchronization barriers for coordinating parallel operations
 * - Work queues with work stealing support
 * 
 * All CLLM-specific logic (sphere positions, prime generation, gradients)
 * remains in libcllm.so.
 */

// ============================================================================
// TREE NODE STRUCTURE
// ============================================================================

/**
 * Generic Tree Node
 * 
 * Represents a node in a hierarchical tree structure with:
 * - Parent-child relationships
 * - Sibling relationships
 * - User data pointer for application-specific data
 * - Thread-safe state management
 */
typedef struct HierarchicalNode {
    // Identity
    int node_id;                        // Unique node ID
    int level;                          // Level in hierarchy (0 = root)
    
    // Hierarchy relationships
    struct HierarchicalNode* parent;    // Parent node (NULL for root)
    int num_children;                   // Number of children
    struct HierarchicalNode** children; // Array of child nodes
    int max_children;                   // Maximum children capacity
    
    int num_siblings;                   // Number of siblings
    struct HierarchicalNode** siblings; // Array of sibling nodes
    int max_siblings;                   // Maximum siblings capacity
    
    // User data
    void* user_data;                    // Application-specific data
    void (*free_user_data)(void*);      // Function to free user data
    
    // State management
    atomic_int state;                   // Current state (application-defined)
    pthread_mutex_t state_mutex;        // Protects state transitions
    pthread_cond_t state_changed;       // Signals state changes
    
    // Thread management
    pthread_t thread;                   // Thread handle (if threaded)
    atomic_int thread_running;          // 1 if thread is running
    
} HierarchicalNode;

/**
 * Tree Traversal Order
 */
typedef enum {
    TRAVERSE_PREORDER,      // Visit node, then children
    TRAVERSE_POSTORDER,     // Visit children, then node
    TRAVERSE_LEVELORDER     // Visit level by level (breadth-first)
} TraversalOrder;

/**
 * Tree Traversal Callback
 * 
 * Called for each node during traversal.
 * Return 0 to continue, non-zero to stop traversal.
 */
typedef int (*NodeVisitor)(HierarchicalNode* node, void* user_data);

// ============================================================================
// SYNCHRONIZATION BARRIER
// ============================================================================

/**
 * Synchronization Barrier
 * 
 * Coordinates multiple threads/nodes at synchronization points.
 * Reusable barrier with generation counter.
 */
typedef struct {
    pthread_barrier_t barrier;       // POSIX barrier
    atomic_int arrived;              // Number arrived
    atomic_int required;             // Number required
    atomic_int generation;           // Barrier generation (for reuse)
} SyncBarrier;

// ============================================================================
// WORK QUEUE
// ============================================================================

/**
 * Work Item
 * 
 * Generic work item for work queues.
 */
typedef struct {
    uint64_t id;                     // Work item ID
    void* data;                      // Work data
    void (*free_data)(void*);        // Function to free data
} WorkItem;

/**
 * Work Queue
 * 
 * Lock-free work queue with work stealing support.
 */
typedef struct {
    WorkItem* items;                 // Array of work items
    size_t capacity;                 // Queue capacity
    atomic_size_t head;              // Head index (dequeue)
    atomic_size_t tail;              // Tail index (enqueue)
    atomic_size_t size;              // Current size
    
    // Work stealing
    atomic_int stealing_enabled;     // 1 if work stealing enabled
    atomic_uint_fast64_t stolen_from; // Times work stolen from this queue
    atomic_uint_fast64_t stolen_to;   // Times this queue stole work
    
    pthread_mutex_t mutex;           // Protects queue operations
} WorkQueue;

// ============================================================================
// TREE MANAGEMENT FUNCTIONS
// ============================================================================

/**
 * Create a hierarchical tree node
 * 
 * @param node_id Unique node ID
 * @param level Level in hierarchy (0 = root)
 * @param max_children Maximum number of children
 * @param max_siblings Maximum number of siblings
 * @param user_data Application-specific data
 * @param free_user_data Function to free user data (can be NULL)
 * @return New node, or NULL on error
 */
HierarchicalNode* hierarchical_node_create(
    int node_id,
    int level,
    int max_children,
    int max_siblings,
    void* user_data,
    void (*free_user_data)(void*)
);

/**
 * Free a hierarchical node and all its children
 * 
 * @param node Node to free
 */
void hierarchical_node_free(HierarchicalNode* node);

/**
 * Add a child node to a parent
 * 
 * @param parent Parent node
 * @param child Child node to add
 * @return 0 on success, -1 on error
 */
int hierarchical_node_add_child(HierarchicalNode* parent, HierarchicalNode* child);

/**
 * Remove a child node from a parent
 * 
 * @param parent Parent node
 * @param child Child node to remove
 * @return 0 on success, -1 on error
 */
int hierarchical_node_remove_child(HierarchicalNode* parent, HierarchicalNode* child);

/**
 * Add a sibling node
 * 
 * @param node Node to add sibling to
 * @param sibling Sibling node to add
 * @return 0 on success, -1 on error
 */
int hierarchical_node_add_sibling(HierarchicalNode* node, HierarchicalNode* sibling);

/**
 * Discover siblings among an array of nodes at the same level
 * 
 * @param nodes Array of nodes
 * @param num_nodes Number of nodes
 */
void hierarchical_node_discover_siblings(HierarchicalNode** nodes, int num_nodes);

/**
 * Find a node by ID in the tree
 * 
 * @param root Root node to start search
 * @param node_id Node ID to find
 * @return Node if found, NULL otherwise
 */
HierarchicalNode* hierarchical_node_find(HierarchicalNode* root, int node_id);

/**
 * Traverse the tree
 * 
 * @param root Root node to start traversal
 * @param order Traversal order
 * @param visitor Callback function for each node
 * @param user_data User data passed to callback
 * @return 0 if traversal completed, non-zero if stopped early
 */
int hierarchical_node_traverse(
    HierarchicalNode* root,
    TraversalOrder order,
    NodeVisitor visitor,
    void* user_data
);

/**
 * Get the depth of the tree
 * 
 * @param root Root node
 * @return Maximum depth of tree
 */
int hierarchical_node_get_depth(const HierarchicalNode* root);

/**
 * Count total nodes in the tree
 * 
 * @param root Root node
 * @return Total number of nodes
 */
int hierarchical_node_count(const HierarchicalNode* root);

/**
 * Print tree structure (for debugging)
 * 
 * @param root Root node
 * @param indent Indentation level
 */
void hierarchical_node_print(const HierarchicalNode* root, int indent);

// ============================================================================
// STATE MANAGEMENT FUNCTIONS
// ============================================================================

/**
 * Get node state
 * 
 * @param node Node
 * @return Current state
 */
int hierarchical_node_get_state(const HierarchicalNode* node);

/**
 * Set node state
 * 
 * @param node Node
 * @param new_state New state
 */
void hierarchical_node_set_state(HierarchicalNode* node, int new_state);

/**
 * Wait for node to reach a specific state
 * 
 * @param node Node
 * @param target_state Target state to wait for
 * @param timeout_ms Timeout in milliseconds (0 = no timeout)
 * @return 0 if state reached, -1 on timeout or error
 */
int hierarchical_node_wait_for_state(
    HierarchicalNode* node,
    int target_state,
    int timeout_ms
);

// ============================================================================
// SYNCHRONIZATION BARRIER FUNCTIONS
// ============================================================================

/**
 * Create a synchronization barrier
 * 
 * @param num_threads Number of threads to synchronize
 * @return New barrier, or NULL on error
 */
SyncBarrier* sync_barrier_create(int num_threads);

/**
 * Free a synchronization barrier
 * 
 * @param barrier Barrier to free
 */
void sync_barrier_free(SyncBarrier* barrier);

/**
 * Wait at a synchronization barrier
 * 
 * @param barrier Barrier to wait at
 * @return 0 on success, -1 on error
 */
int sync_barrier_wait(SyncBarrier* barrier);

/**
 * Reset a synchronization barrier for reuse
 * 
 * @param barrier Barrier to reset
 */
void sync_barrier_reset(SyncBarrier* barrier);

/**
 * Get barrier generation (for tracking reuse)
 * 
 * @param barrier Barrier
 * @return Current generation number
 */
int sync_barrier_get_generation(const SyncBarrier* barrier);

// ============================================================================
// WORK QUEUE FUNCTIONS
// ============================================================================

/**
 * Create a work queue
 * 
 * @param capacity Maximum number of work items
 * @return New work queue, or NULL on error
 */
WorkQueue* work_queue_create(size_t capacity);

/**
 * Free a work queue
 * 
 * @param queue Queue to free
 */
void work_queue_free(WorkQueue* queue);

/**
 * Enqueue a work item
 * 
 * @param queue Work queue
 * @param id Work item ID
 * @param data Work data
 * @param free_data Function to free data (can be NULL)
 * @return 0 on success, -1 on error (queue full)
 */
int work_queue_enqueue(
    WorkQueue* queue,
    uint64_t id,
    void* data,
    void (*free_data)(void*)
);

/**
 * Dequeue a work item
 * 
 * @param queue Work queue
 * @param item Output work item
 * @return 0 on success, -1 if queue empty
 */
int work_queue_dequeue(WorkQueue* queue, WorkItem* item);

/**
 * Peek at next work item without removing it
 * 
 * @param queue Work queue
 * @param item Output work item
 * @return 0 on success, -1 if queue empty
 */
int work_queue_peek(const WorkQueue* queue, WorkItem* item);

/**
 * Get work queue size
 * 
 * @param queue Work queue
 * @return Current number of items in queue
 */
size_t work_queue_size(const WorkQueue* queue);

/**
 * Check if work queue is empty
 * 
 * @param queue Work queue
 * @return 1 if empty, 0 otherwise
 */
int work_queue_is_empty(const WorkQueue* queue);

/**
 * Check if work queue is full
 * 
 * @param queue Work queue
 * @return 1 if full, 0 otherwise
 */
int work_queue_is_full(const WorkQueue* queue);

/**
 * Enable work stealing for a queue
 * 
 * @param queue Work queue
 */
void work_queue_enable_stealing(WorkQueue* queue);

/**
 * Disable work stealing for a queue
 * 
 * @param queue Work queue
 */
void work_queue_disable_stealing(WorkQueue* queue);

/**
 * Steal work from another queue
 * 
 * @param thief Queue stealing work
 * @param victim Queue to steal from
 * @param item Output work item
 * @return 0 on success, -1 if no work to steal
 */
int work_queue_steal(WorkQueue* thief, WorkQueue* victim, WorkItem* item);

/**
 * Get work stealing statistics
 * 
 * @param queue Work queue
 * @param stolen_from Output: times work stolen from this queue
 * @param stolen_to Output: times this queue stole work
 */
void work_queue_get_steal_stats(
    const WorkQueue* queue,
    uint64_t* stolen_from,
    uint64_t* stolen_to
);

#endif // HIERARCHICAL_STRUCTURES_H
