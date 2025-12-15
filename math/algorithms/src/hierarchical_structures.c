#include "hierarchical_structures.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================



// ============================================================================
// TREE NODE MANAGEMENT
// ============================================================================

HierarchicalNode* hierarchical_node_create(
    int node_id,
    int level,
    int max_children,
    int max_siblings,
    void* user_data,
    void (*free_user_data)(void*)
) {
    if (max_children < 0 || max_siblings < 0) {
        fprintf(stderr, "ERROR: Invalid max_children or max_siblings\n");
        return NULL;
    }
    
    HierarchicalNode* node = calloc(1, sizeof(HierarchicalNode));
    if (!node) {
        fprintf(stderr, "ERROR: Failed to allocate node\n");
        return NULL;
    }
    
    // Set identity
    node->node_id = node_id;
    node->level = level;
    
    // Allocate children array
    if (max_children > 0) {
        node->children = calloc(max_children, sizeof(HierarchicalNode*));
        if (!node->children) {
            fprintf(stderr, "ERROR: Failed to allocate children array\n");
            free(node);
            return NULL;
        }
    }
    node->max_children = max_children;
    node->num_children = 0;
    
    // Allocate siblings array
    if (max_siblings > 0) {
        node->siblings = calloc(max_siblings, sizeof(HierarchicalNode*));
        if (!node->siblings) {
            fprintf(stderr, "ERROR: Failed to allocate siblings array\n");
            free(node->children);
            free(node);
            return NULL;
        }
    }
    node->max_siblings = max_siblings;
    node->num_siblings = 0;
    
    // Set user data
    node->user_data = user_data;
    node->free_user_data = free_user_data;
    
    // Initialize parent
    node->parent = NULL;
    
    // Initialize state
    atomic_init(&node->state, 0);
    atomic_init(&node->thread_running, 0);
    
    // Initialize mutexes and condition variables
    pthread_mutex_init(&node->state_mutex, NULL);
    pthread_cond_init(&node->state_changed, NULL);
    
    return node;
}

void hierarchical_node_free(HierarchicalNode* node) {
    if (!node) return;
    
    // Free all children recursively
    for (int i = 0; i < node->num_children; i++) {
        hierarchical_node_free(node->children[i]);
    }
    
    // Free user data if provided
    if (node->user_data && node->free_user_data) {
        node->free_user_data(node->user_data);
    }
    
    // Free arrays
    free(node->children);
    free(node->siblings);
    
    // Destroy mutexes and condition variables
    pthread_mutex_destroy(&node->state_mutex);
    pthread_cond_destroy(&node->state_changed);
    
    // Free node
    free(node);
}

int hierarchical_node_add_child(HierarchicalNode* parent, HierarchicalNode* child) {
    if (!parent || !child) {
        fprintf(stderr, "ERROR: NULL parent or child\n");
        return -1;
    }
    
    if (parent->num_children >= parent->max_children) {
        fprintf(stderr, "ERROR: Parent has maximum children (%d)\n", parent->max_children);
        return -1;
    }
    
    // Add child to parent
    parent->children[parent->num_children++] = child;
    
    // Set parent reference
    child->parent = parent;
    
    return 0;
}

int hierarchical_node_remove_child(HierarchicalNode* parent, HierarchicalNode* child) {
    if (!parent || !child) {
        fprintf(stderr, "ERROR: NULL parent or child\n");
        return -1;
    }
    
    // Find child in parent's children array
    int found = -1;
    for (int i = 0; i < parent->num_children; i++) {
        if (parent->children[i] == child) {
            found = i;
            break;
        }
    }
    
    if (found == -1) {
        fprintf(stderr, "ERROR: Child not found in parent's children\n");
        return -1;
    }
    
    // Shift remaining children
    for (int i = found; i < parent->num_children - 1; i++) {
        parent->children[i] = parent->children[i + 1];
    }
    parent->num_children--;
    
    // Clear parent reference
    child->parent = NULL;
    
    return 0;
}

int hierarchical_node_add_sibling(HierarchicalNode* node, HierarchicalNode* sibling) {
    if (!node || !sibling) {
        fprintf(stderr, "ERROR: NULL node or sibling\n");
        return -1;
    }
    
    if (node->num_siblings >= node->max_siblings) {
        fprintf(stderr, "ERROR: Node has maximum siblings (%d)\n", node->max_siblings);
        return -1;
    }
    
    // Add sibling to node
    node->siblings[node->num_siblings++] = sibling;
    
    return 0;
}

void hierarchical_node_discover_siblings(HierarchicalNode** nodes, int num_nodes) {
    if (!nodes || num_nodes <= 1) return;
    
    // For each node, add all other nodes at the same level as siblings
    for (int i = 0; i < num_nodes; i++) {
        HierarchicalNode* node = nodes[i];
        if (!node) continue;
        
        for (int j = 0; j < num_nodes; j++) {
            if (i == j) continue;
            
            HierarchicalNode* other = nodes[j];
            if (!other) continue;
            
            // Only add if at same level
            if (node->level == other->level) {
                hierarchical_node_add_sibling(node, other);
            }
        }
    }
}

HierarchicalNode* hierarchical_node_find(HierarchicalNode* root, int node_id) {
    if (!root) return NULL;
    
    // Check if this is the node we're looking for
    if (root->node_id == node_id) {
        return root;
    }
    
    // Search children recursively
    for (int i = 0; i < root->num_children; i++) {
        HierarchicalNode* found = hierarchical_node_find(root->children[i], node_id);
        if (found) return found;
    }
    
    return NULL;
}

// Helper for traversal
static int traverse_preorder(HierarchicalNode* node, NodeVisitor visitor, void* user_data) {
    if (!node) return 0;
    
    // Visit node first
    int result = visitor(node, user_data);
    if (result != 0) return result;
    
    // Then visit children
    for (int i = 0; i < node->num_children; i++) {
        result = traverse_preorder(node->children[i], visitor, user_data);
        if (result != 0) return result;
    }
    
    return 0;
}

static int traverse_postorder(HierarchicalNode* node, NodeVisitor visitor, void* user_data) {
    if (!node) return 0;
    
    // Visit children first
    for (int i = 0; i < node->num_children; i++) {
        int result = traverse_postorder(node->children[i], visitor, user_data);
        if (result != 0) return result;
    }
    
    // Then visit node
    return visitor(node, user_data);
}

static int traverse_levelorder(HierarchicalNode* root, NodeVisitor visitor, void* user_data) {
    if (!root) return 0;
    
    // Simple level-order traversal using a queue
    // Allocate queue (max size = total nodes, estimated)
    int max_nodes = 1000;  // Reasonable default
    HierarchicalNode** queue = malloc(max_nodes * sizeof(HierarchicalNode*));
    if (!queue) return -1;
    
    int head = 0, tail = 0;
    queue[tail++] = root;
    
    while (head < tail) {
        HierarchicalNode* node = queue[head++];
        
        // Visit node
        int result = visitor(node, user_data);
        if (result != 0) {
            free(queue);
            return result;
        }
        
        // Add children to queue
        for (int i = 0; i < node->num_children; i++) {
            if (tail < max_nodes) {
                queue[tail++] = node->children[i];
            }
        }
    }
    
    free(queue);
    return 0;
}

int hierarchical_node_traverse(
    HierarchicalNode* root,
    TraversalOrder order,
    NodeVisitor visitor,
    void* user_data
) {
    if (!root || !visitor) return -1;
    
    switch (order) {
        case TRAVERSE_PREORDER:
            return traverse_preorder(root, visitor, user_data);
        case TRAVERSE_POSTORDER:
            return traverse_postorder(root, visitor, user_data);
        case TRAVERSE_LEVELORDER:
            return traverse_levelorder(root, visitor, user_data);
        default:
            return -1;
    }
}

int hierarchical_node_get_depth(const HierarchicalNode* root) {
    if (!root) return 0;
    
    int max_depth = 0;
    for (int i = 0; i < root->num_children; i++) {
        int child_depth = hierarchical_node_get_depth(root->children[i]);
        if (child_depth > max_depth) {
            max_depth = child_depth;
        }
    }
    
    return max_depth + 1;
}

int hierarchical_node_count(const HierarchicalNode* root) {
    if (!root) return 0;
    
    int count = 1;  // Count this node
    for (int i = 0; i < root->num_children; i++) {
        count += hierarchical_node_count(root->children[i]);
    }
    
    return count;
}

void hierarchical_node_print(const HierarchicalNode* root, int indent) {
    if (!root) return;
    
    // Print indentation
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
    
    // Print node info
    printf("Node %d (level %d, %d children, %d siblings)\n",
           root->node_id, root->level, root->num_children, root->num_siblings);
    
    // Print children recursively
    for (int i = 0; i < root->num_children; i++) {
        hierarchical_node_print(root->children[i], indent + 1);
    }
}

// ============================================================================
// STATE MANAGEMENT
// ============================================================================

int hierarchical_node_get_state(const HierarchicalNode* node) {
    if (!node) return -1;
    return atomic_load(&node->state);
}

void hierarchical_node_set_state(HierarchicalNode* node, int new_state) {
    if (!node) return;
    
    pthread_mutex_lock(&node->state_mutex);
    atomic_store(&node->state, new_state);
    pthread_cond_broadcast(&node->state_changed);
    pthread_mutex_unlock(&node->state_mutex);
}

int hierarchical_node_wait_for_state(
    HierarchicalNode* node,
    int target_state,
    int timeout_ms
) {
    if (!node) return -1;
    
    pthread_mutex_lock(&node->state_mutex);
    
    if (timeout_ms == 0) {
        // No timeout - wait indefinitely
        while (atomic_load(&node->state) != target_state) {
            pthread_cond_wait(&node->state_changed, &node->state_mutex);
        }
        pthread_mutex_unlock(&node->state_mutex);
        return 0;
    } else {
        // Wait with timeout
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += timeout_ms / 1000;
        ts.tv_nsec += (timeout_ms % 1000) * 1000000;
        if (ts.tv_nsec >= 1000000000) {
            ts.tv_sec++;
            ts.tv_nsec -= 1000000000;
        }
        
        while (atomic_load(&node->state) != target_state) {
            int result = pthread_cond_timedwait(&node->state_changed, &node->state_mutex, &ts);
            if (result == ETIMEDOUT) {
                pthread_mutex_unlock(&node->state_mutex);
                return -1;
            }
        }
        pthread_mutex_unlock(&node->state_mutex);
        return 0;
    }
}

// ============================================================================
// SYNCHRONIZATION BARRIER
// ============================================================================

SyncBarrier* sync_barrier_create(int num_threads) {
    if (num_threads <= 0) {
        fprintf(stderr, "ERROR: Invalid number of threads: %d\n", num_threads);
        return NULL;
    }
    
    SyncBarrier* barrier = malloc(sizeof(SyncBarrier));
    if (!barrier) {
        fprintf(stderr, "ERROR: Failed to allocate barrier\n");
        return NULL;
    }
    
    // Initialize POSIX barrier
    if (pthread_barrier_init(&barrier->barrier, NULL, num_threads) != 0) {
        fprintf(stderr, "ERROR: Failed to initialize pthread barrier\n");
        free(barrier);
        return NULL;
    }
    
    // Initialize atomic counters
    atomic_init(&barrier->arrived, 0);
    atomic_init(&barrier->required, num_threads);
    atomic_init(&barrier->generation, 0);
    
    return barrier;
}

void sync_barrier_free(SyncBarrier* barrier) {
    if (!barrier) return;
    
    pthread_barrier_destroy(&barrier->barrier);
    free(barrier);
}

int sync_barrier_wait(SyncBarrier* barrier) {
    if (!barrier) return -1;
    
    // Increment arrived counter
    atomic_fetch_add(&barrier->arrived, 1);
    
    // Wait at barrier
    int result = pthread_barrier_wait(&barrier->barrier);
    
    // Last thread to arrive resets the counter and increments generation
    if (result == PTHREAD_BARRIER_SERIAL_THREAD) {
        atomic_store(&barrier->arrived, 0);
        atomic_fetch_add(&barrier->generation, 1);
        return 0;
    }
    
    return (result == 0) ? 0 : -1;
}

void sync_barrier_reset(SyncBarrier* barrier) {
    if (!barrier) return;
    
    atomic_store(&barrier->arrived, 0);
    atomic_fetch_add(&barrier->generation, 1);
}

int sync_barrier_get_generation(const SyncBarrier* barrier) {
    if (!barrier) return -1;
    return atomic_load(&barrier->generation);
}

// ============================================================================
// WORK QUEUE
// ============================================================================

WorkQueue* work_queue_create(size_t capacity) {
    if (capacity == 0) {
        fprintf(stderr, "ERROR: Invalid capacity: %zu\n", capacity);
        return NULL;
    }
    
    WorkQueue* queue = malloc(sizeof(WorkQueue));
    if (!queue) {
        fprintf(stderr, "ERROR: Failed to allocate work queue\n");
        return NULL;
    }
    
    // Allocate items array
    queue->items = calloc(capacity, sizeof(WorkItem));
    if (!queue->items) {
        fprintf(stderr, "ERROR: Failed to allocate work items\n");
        free(queue);
        return NULL;
    }
    
    queue->capacity = capacity;
    atomic_init(&queue->head, 0);
    atomic_init(&queue->tail, 0);
    atomic_init(&queue->size, 0);
    
    // Initialize work stealing
    atomic_init(&queue->stealing_enabled, 0);
    atomic_init(&queue->stolen_from, 0);
    atomic_init(&queue->stolen_to, 0);
    
    // Initialize mutex
    pthread_mutex_init(&queue->mutex, NULL);
    
    return queue;
}

void work_queue_free(WorkQueue* queue) {
    if (!queue) return;
    
    // Free any remaining work items
    size_t size = atomic_load(&queue->size);
    if (size > 0) {
        size_t head = atomic_load(&queue->head);
        for (size_t i = 0; i < size; i++) {
            size_t idx = (head + i) % queue->capacity;
            WorkItem* item = &queue->items[idx];
            if (item->data && item->free_data) {
                item->free_data(item->data);
            }
        }
    }
    
    free(queue->items);
    pthread_mutex_destroy(&queue->mutex);
    free(queue);
}

int work_queue_enqueue(
    WorkQueue* queue,
    uint64_t id,
    void* data,
    void (*free_data)(void*)
) {
    if (!queue) return -1;
    
    pthread_mutex_lock(&queue->mutex);
    
    // Check if queue is full
    size_t size = atomic_load(&queue->size);
    if (size >= queue->capacity) {
        pthread_mutex_unlock(&queue->mutex);
        return -1;
    }
    
    // Add item to tail
    size_t tail = atomic_load(&queue->tail);
    WorkItem* item = &queue->items[tail];
    item->id = id;
    item->data = data;
    item->free_data = free_data;
    
    // Update tail and size
    atomic_store(&queue->tail, (tail + 1) % queue->capacity);
    atomic_fetch_add(&queue->size, 1);
    
    pthread_mutex_unlock(&queue->mutex);
    return 0;
}

int work_queue_dequeue(WorkQueue* queue, WorkItem* item) {
    if (!queue || !item) return -1;
    
    pthread_mutex_lock(&queue->mutex);
    
    // Check if queue is empty
    size_t size = atomic_load(&queue->size);
    if (size == 0) {
        pthread_mutex_unlock(&queue->mutex);
        return -1;
    }
    
    // Get item from head
    size_t head = atomic_load(&queue->head);
    *item = queue->items[head];
    
    // Clear the slot
    memset(&queue->items[head], 0, sizeof(WorkItem));
    
    // Update head and size
    atomic_store(&queue->head, (head + 1) % queue->capacity);
    atomic_fetch_sub(&queue->size, 1);
    
    pthread_mutex_unlock(&queue->mutex);
    return 0;
}

int work_queue_peek(const WorkQueue* queue, WorkItem* item) {
    if (!queue || !item) return -1;
    
    // Check if queue is empty
    size_t size = atomic_load(&queue->size);
    if (size == 0) {
        return -1;
    }
    
    // Get item from head without removing
    size_t head = atomic_load(&queue->head);
    *item = queue->items[head];
    
    return 0;
}

size_t work_queue_size(const WorkQueue* queue) {
    if (!queue) return 0;
    return atomic_load(&queue->size);
}

int work_queue_is_empty(const WorkQueue* queue) {
    return work_queue_size(queue) == 0;
}

int work_queue_is_full(const WorkQueue* queue) {
    if (!queue) return 1;
    return atomic_load(&queue->size) >= queue->capacity;
}

void work_queue_enable_stealing(WorkQueue* queue) {
    if (!queue) return;
    atomic_store(&queue->stealing_enabled, 1);
}

void work_queue_disable_stealing(WorkQueue* queue) {
    if (!queue) return;
    atomic_store(&queue->stealing_enabled, 0);
}

int work_queue_steal(WorkQueue* thief, WorkQueue* victim, WorkItem* item) {
    if (!thief || !victim || !item) return -1;
    
    // Check if stealing is enabled on victim
    if (!atomic_load(&victim->stealing_enabled)) {
        return -1;
    }
    
    pthread_mutex_lock(&victim->mutex);
    
    // Check if victim has work
    size_t size = atomic_load(&victim->size);
    if (size == 0) {
        pthread_mutex_unlock(&victim->mutex);
        return -1;
    }
    
    // Steal from tail (most recent work)
    size_t tail = atomic_load(&victim->tail);
    size_t steal_idx = (tail + victim->capacity - 1) % victim->capacity;
    *item = victim->items[steal_idx];
    
    // Clear the slot
    memset(&victim->items[steal_idx], 0, sizeof(WorkItem));
    
    // Update tail and size
    atomic_store(&victim->tail, steal_idx);
    atomic_fetch_sub(&victim->size, 1);
    
    // Update statistics
    atomic_fetch_add(&victim->stolen_from, 1);
    atomic_fetch_add(&thief->stolen_to, 1);
    
    pthread_mutex_unlock(&victim->mutex);
    return 0;
}

void work_queue_get_steal_stats(
    const WorkQueue* queue,
    uint64_t* stolen_from,
    uint64_t* stolen_to
) {
    if (!queue) return;
    
    if (stolen_from) {
        *stolen_from = atomic_load(&queue->stolen_from);
    }
    if (stolen_to) {
        *stolen_to = atomic_load(&queue->stolen_to);
    }
}
