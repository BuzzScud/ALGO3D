/**
 * @file test_hierarchical_threading_week5.c
 * @brief Week 5 Integration Tests - Complete Hierarchical Threading System
 * 
 * Tests the unified integration of all Week 1-4 components:
 * - Week 1: Hierarchical Memory + Abacus Integration
 * - Week 2: Shared Memory + Rainbow Table
 * - Week 3: Message Passing System
 * - Week 4: State Management + Work Distribution
 */

#include "hierarchical_threading.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

// Test counter
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) \
    printf("\n=== Test: %s ===\n", #name); \
    if (test_##name()) { \
        printf("✓ %s passed\n", #name); \
        tests_passed++; \
    } else { \
        printf("✗ %s FAILED\n", #name); \
        tests_failed++; \
    }

// ============================================================================
// TEST 1: Thread Pool Creation
// ============================================================================

bool test_thread_pool_creation() {
    printf("Creating thread pool with 12 threads, 12-fold symmetry, 13 dimensions...\n");
    fflush(stdout);
    
    HierarchicalThreadPool* pool = hierarchical_thread_pool_create(
        12,     // 12 threads
        12,     // 12-fold symmetry
        13,     // 13 dimensions
        false   // not NUMA-aware for testing
    );
    
    printf("Pool creation returned: %p\n", (void*)pool);
    fflush(stdout);
    
    if (!pool) {
        printf("Failed to create thread pool\n");
        return false;
    }
    
    printf("Thread pool created successfully\n");
    printf("  Max threads: %u\n", pool->max_threads);
    printf("  Symmetry fold: %u\n", pool->symmetry_fold);
    printf("  Dimensions: %u\n", pool->num_dimensions);
    printf("  Initialized: %s\n", pool->initialized ? "yes" : "no");
    fflush(stdout);
    
    // Verify components
    printf("Checking global_memory: %p\n", (void*)pool->global_memory);
    fflush(stdout);
    if (!pool->global_memory) {
        printf("Global memory not created\n");
        hierarchical_thread_pool_free(pool);
        return false;
    }
    
    printf("Checking work_distributor: %p\n", (void*)pool->work_distributor);
    fflush(stdout);
    if (!pool->work_distributor) {
        printf("Work distributor not created\n");
        hierarchical_thread_pool_free(pool);
        return false;
    }
    
    printf("Checking state_manager: %p\n", (void*)pool->state_manager);
    fflush(stdout);
    if (!pool->state_manager) {
        printf("State manager not created\n");
        hierarchical_thread_pool_free(pool);
        return false;
    }
    
    printf("Freeing pool...\n");
    fflush(stdout);
    hierarchical_thread_pool_free(pool);
    printf("Pool freed successfully\n");
    fflush(stdout);
    return true;
}

// ============================================================================
// TEST 2: Thread Creation
// ============================================================================

bool test_thread_creation() {
    printf("Creating individual threads...\n");
    
    HierarchicalThreadPool* pool = hierarchical_thread_pool_create(4, 12, 13, false);
    if (!pool) {
        return false;
    }
    
    // Create root thread
    HierarchicalThread* root = hierarchical_thread_create(
        0,
        THREAD_ROLE_CONTROL,
        NULL,
        pool
    );
    
    if (!root) {
        printf("Failed to create root thread\n");
        hierarchical_thread_pool_free(pool);
        return false;
    }
    
    printf("Root thread created: ID=%u, Role=%d, State=%d\n",
           root->thread_id, root->role, root->current_state);
    
    // Create child threads
    HierarchicalThread* child1 = hierarchical_thread_create(
        1,
        THREAD_ROLE_WORKER,
        root,
        pool
    );
    
    HierarchicalThread* child2 = hierarchical_thread_create(
        2,
        THREAD_ROLE_WORKER,
        root,
        pool
    );
    
    if (!child1 || !child2) {
        printf("Failed to create child threads\n");
        hierarchical_thread_free(root);
        hierarchical_thread_pool_free(pool);
        return false;
    }
    
    printf("Child threads created: %u and %u\n", child1->thread_id, child2->thread_id);
    printf("Root has %u children\n", root->num_children);
    
    // Verify hierarchy
    if (root->num_children != 2) {
        printf("Expected 2 children, got %u\n", root->num_children);
        hierarchical_thread_free(child2);
        hierarchical_thread_free(child1);
        hierarchical_thread_free(root);
        hierarchical_thread_pool_free(pool);
        return false;
    }
    
    if (child1->parent != root || child2->parent != root) {
        printf("Parent pointers not set correctly\n");
        hierarchical_thread_free(child2);
        hierarchical_thread_free(child1);
        hierarchical_thread_free(root);
        hierarchical_thread_pool_free(pool);
        return false;
    }
    
    // Cleanup
    hierarchical_thread_free(child2);
    hierarchical_thread_free(child1);
    hierarchical_thread_free(root);
    hierarchical_thread_pool_free(pool);
    
    return true;
}

// ============================================================================
// TEST 3: Geometric Positioning
// ============================================================================

bool test_geometric_positioning() {
    printf("Testing geometric positioning of threads...\n");
    
    HierarchicalThreadPool* pool = hierarchical_thread_pool_create(12, 12, 13, false);
    if (!pool) {
        return false;
    }
    
    // Create threads and check their positions
    HierarchicalThread* threads[12];
    for (uint32_t i = 0; i < 12; i++) {
        threads[i] = hierarchical_thread_create(i, THREAD_ROLE_WORKER, NULL, pool);
        if (!threads[i]) {
            printf("Failed to create thread %u\n", i);
            for (uint32_t j = 0; j < i; j++) {
                hierarchical_thread_free(threads[j]);
            }
            hierarchical_thread_pool_free(pool);
            return false;
        }
    }
    
    // Verify positions are set
    for (uint32_t i = 0; i < 12; i++) {
        if (!threads[i]->position) {
            printf("Thread %u has no position\n", i);
            for (uint32_t j = 0; j < 12; j++) {
                hierarchical_thread_free(threads[j]);
            }
            hierarchical_thread_pool_free(pool);
            return false;
        }
        
        printf("Thread %u: symmetry_group=%u, position[0]=%.3f, position[1]=%.3f\n",
               i, threads[i]->symmetry_group,
               threads[i]->position[0], threads[i]->position[1]);
    }
    
    // Calculate distances between threads
    double dist_0_1 = hierarchical_thread_distance(threads[0], threads[1]);
    double dist_0_6 = hierarchical_thread_distance(threads[0], threads[6]);
    
    printf("Distance between thread 0 and 1: %.3f\n", dist_0_1);
    printf("Distance between thread 0 and 6: %.3f\n", dist_0_6);
    
    // Cleanup
    for (uint32_t i = 0; i < 12; i++) {
        hierarchical_thread_free(threads[i]);
    }
    hierarchical_thread_pool_free(pool);
    
    return true;
}

// ============================================================================
// TEST 4: Neighbor Operations
// ============================================================================

bool test_neighbor_operations() {
    printf("Testing neighbor operations...\n");
    
    HierarchicalThreadPool* pool = hierarchical_thread_pool_create(4, 12, 13, false);
    if (!pool) {
        return false;
    }
    
    HierarchicalThread* thread0 = hierarchical_thread_create(0, THREAD_ROLE_WORKER, NULL, pool);
    HierarchicalThread* thread1 = hierarchical_thread_create(1, THREAD_ROLE_WORKER, NULL, pool);
    HierarchicalThread* thread2 = hierarchical_thread_create(2, THREAD_ROLE_WORKER, NULL, pool);
    
    if (!thread0 || !thread1 || !thread2) {
        printf("Failed to create threads\n");
        if (thread0) hierarchical_thread_free(thread0);
        if (thread1) hierarchical_thread_free(thread1);
        if (thread2) hierarchical_thread_free(thread2);
        hierarchical_thread_pool_free(pool);
        return false;
    }
    
    // Add thread pool references
    pool->threads = malloc(4 * sizeof(HierarchicalThread*));
    pool->threads[0] = thread0;
    pool->threads[1] = thread1;
    pool->threads[2] = thread2;
    pool->num_threads = 3;
    
    // Add neighbors
    double dist = hierarchical_thread_distance(thread0, thread1);
    int result = hierarchical_thread_add_neighbor(thread0, 1, THREAD_REL_SIBLING, dist, pool);
    
    if (result != 0) {
        printf("Failed to add neighbor\n");
        free(pool->threads);
        hierarchical_thread_free(thread2);
        hierarchical_thread_free(thread1);
        hierarchical_thread_free(thread0);
        hierarchical_thread_pool_free(pool);
        return false;
    }
    
    printf("Added neighbor 1 to thread 0\n");
    printf("Thread 0 now has %u neighbors\n", thread0->num_neighbors);
    
    // Get neighbor
    ThreadNeighbor* neighbor = hierarchical_thread_get_neighbor(thread0, 1);
    if (!neighbor) {
        printf("Failed to get neighbor\n");
        free(pool->threads);
        hierarchical_thread_free(thread2);
        hierarchical_thread_free(thread1);
        hierarchical_thread_free(thread0);
        hierarchical_thread_pool_free(pool);
        return false;
    }
    
    printf("Retrieved neighbor: ID=%u, relationship=%d, distance=%.3f\n",
           neighbor->thread_id, neighbor->relationship, neighbor->distance);
    
    // Get boundary
    SharedMemoryEnhanced* boundary = hierarchical_thread_get_boundary(thread0, 1);
    if (!boundary) {
        printf("Failed to get boundary\n");
        free(pool->threads);
        hierarchical_thread_free(thread2);
        hierarchical_thread_free(thread1);
        hierarchical_thread_free(thread0);
        hierarchical_thread_pool_free(pool);
        return false;
    }
    
    printf("Boundary memory created: size=%zu\n", boundary->base.size);
    
    // Cleanup
    free(pool->threads);
    hierarchical_thread_free(thread2);
    hierarchical_thread_free(thread1);
    hierarchical_thread_free(thread0);
    hierarchical_thread_pool_free(pool);
    
    return true;
}

// ============================================================================
// TEST 5: State Management Integration
// ============================================================================

bool test_state_management() {
    printf("Testing state management integration...\n");
    
    HierarchicalThreadPool* pool = hierarchical_thread_pool_create(2, 12, 13, false);
    if (!pool) {
        return false;
    }
    
    HierarchicalThread* thread = hierarchical_thread_create(0, THREAD_ROLE_WORKER, NULL, pool);
    if (!thread) {
        hierarchical_thread_pool_free(pool);
        return false;
    }
    
    // Check initial state
    StateType initial_state = hierarchical_thread_get_state(thread);
    printf("Initial state: %d\n", initial_state);
    
    if (initial_state != STATE_INITIALIZED) {
        printf("Expected STATE_INITIALIZED (%d), got %d\n", STATE_INITIALIZED, initial_state);
        hierarchical_thread_free(thread);
        hierarchical_thread_pool_free(pool);
        return false;
    }
    
    // Transition to READY
    TransitionResult result = hierarchical_thread_change_state(thread, STATE_READY);
    if (result != TRANSITION_SUCCESS) {
        printf("Failed to transition to READY: %d\n", result);
        hierarchical_thread_free(thread);
        hierarchical_thread_pool_free(pool);
        return false;
    }
    
    StateType new_state = hierarchical_thread_get_state(thread);
    printf("New state: %d\n", new_state);
    
    if (new_state != STATE_READY) {
        printf("State transition failed\n");
        hierarchical_thread_free(thread);
        hierarchical_thread_pool_free(pool);
        return false;
    }
    
    // Transition to RUNNING
    result = hierarchical_thread_change_state(thread, STATE_RUNNING);
    if (result != TRANSITION_SUCCESS) {
        printf("Failed to transition to RUNNING: %d\n", result);
        hierarchical_thread_free(thread);
        hierarchical_thread_pool_free(pool);
        return false;
    }
    
    printf("Successfully transitioned through states\n");
    
    hierarchical_thread_free(thread);
    hierarchical_thread_pool_free(pool);
    
    return true;
}

// ============================================================================
// TEST 6: Memory Operations
// ============================================================================

bool test_memory_operations() {
    printf("Testing memory operations...\n");
    
    HierarchicalThreadPool* pool = hierarchical_thread_pool_create(2, 12, 13, false);
    if (!pool) {
        return false;
    }
    
    HierarchicalThread* thread = hierarchical_thread_create(0, THREAD_ROLE_WORKER, NULL, pool);
    if (!thread) {
        hierarchical_thread_pool_free(pool);
        return false;
    }
    
    // Allocate local memory
    void* local_mem = hierarchical_thread_alloc_local(thread, 1024);
    if (!local_mem) {
        printf("Failed to allocate local memory\n");
        hierarchical_thread_free(thread);
        hierarchical_thread_pool_free(pool);
        return false;
    }
    
    printf("Allocated local memory: %p\n", local_mem);
    
    // Test writing to local memory
    memset(local_mem, 0xAB, 1024);
    printf("Wrote to local memory successfully\n");
    
    hierarchical_thread_free(thread);
    hierarchical_thread_pool_free(pool);
    
    return true;
}

// ============================================================================
// TEST 7: Statistics Collection
// ============================================================================

bool test_statistics() {
    printf("Testing statistics collection...\n");
    
    HierarchicalThreadPool* pool = hierarchical_thread_pool_create(4, 12, 13, false);
    if (!pool) {
        return false;
    }
    
    HierarchicalThread* thread = hierarchical_thread_create(0, THREAD_ROLE_WORKER, NULL, pool);
    if (!thread) {
        hierarchical_thread_pool_free(pool);
        return false;
    }
    
    // Get thread statistics
    HierarchicalThreadStats stats;
    int result = hierarchical_thread_get_stats(thread, &stats);
    
    if (result != 0) {
        printf("Failed to get thread statistics\n");
        hierarchical_thread_free(thread);
        hierarchical_thread_pool_free(pool);
        return false;
    }
    
    printf("Thread Statistics:\n");
    printf("  Thread ID: %u\n", stats.thread_id);
    printf("  Role: %d\n", stats.role);
    printf("  State: %d\n", stats.current_state);
    printf("  Children: %u\n", stats.num_children);
    printf("  Neighbors: %u\n", stats.num_neighbors);
    printf("  Messages sent: %lu\n", stats.messages_sent);
    printf("  Messages received: %lu\n", stats.messages_received);
    printf("  Work completed: %lu\n", stats.work_completed);
    
    // Get pool statistics
    pool->threads = malloc(4 * sizeof(HierarchicalThread*));
    pool->threads[0] = thread;
    pool->num_threads = 1;
    
    HierarchicalThreadPoolStats pool_stats;
    result = hierarchical_thread_pool_get_stats(pool, &pool_stats);
    
    if (result != 0) {
        printf("Failed to get pool statistics\n");
        free(pool->threads);
        hierarchical_thread_free(thread);
        hierarchical_thread_pool_free(pool);
        return false;
    }
    
    printf("\nPool Statistics:\n");
    printf("  Threads: %u\n", pool_stats.num_threads);
    printf("  Symmetry: %u-fold\n", pool_stats.symmetry_fold);
    printf("  Total messages: %lu\n", pool_stats.total_messages);
    printf("  Total work items: %lu\n", pool_stats.total_work_items);
    
    free(pool->threads);
    hierarchical_thread_free(thread);
    hierarchical_thread_pool_free(pool);
    
    return true;
}

// ============================================================================
// TEST 8: Find Nearest Neighbors
// ============================================================================

bool test_find_nearest_neighbors() {
    printf("Testing nearest neighbor finding...\n");
    
    HierarchicalThreadPool* pool = hierarchical_thread_pool_create(12, 12, 13, false);
    if (!pool) {
        return false;
    }
    
    // Create threads
    HierarchicalThread* threads[12];
    for (uint32_t i = 0; i < 12; i++) {
        threads[i] = hierarchical_thread_create(i, THREAD_ROLE_WORKER, NULL, pool);
        if (!threads[i]) {
            for (uint32_t j = 0; j < i; j++) {
                hierarchical_thread_free(threads[j]);
            }
            hierarchical_thread_pool_free(pool);
            return false;
        }
    }
    
    // Set up pool threads array
    pool->threads = threads;
    pool->num_threads = 12;
    
    // Find 3 nearest neighbors to thread 0
    uint32_t neighbors[3];
    uint32_t count = hierarchical_thread_find_nearest_neighbors(threads[0], pool, 3, neighbors);
    
    printf("Found %u nearest neighbors to thread 0:\n", count);
    for (uint32_t i = 0; i < count; i++) {
        double dist = hierarchical_thread_distance(threads[0], threads[neighbors[i]]);
        printf("  Neighbor %u: thread %u (distance %.3f)\n", i, neighbors[i], dist);
    }
    
    if (count != 3) {
        printf("Expected 3 neighbors, got %u\n", count);
        for (uint32_t i = 0; i < 12; i++) {
            hierarchical_thread_free(threads[i]);
        }
        hierarchical_thread_pool_free(pool);
        return false;
    }
    
    // Cleanup
    for (uint32_t i = 0; i < 12; i++) {
        hierarchical_thread_free(threads[i]);
    }
    hierarchical_thread_pool_free(pool);
    
    return true;
}

// ============================================================================
// TEST 9: Complete Integration Test
// ============================================================================

bool test_complete_integration() {
    printf("Testing complete system integration...\n");
    
    // Create a pool with 12 threads (dodecahedron symmetry)
    HierarchicalThreadPool* pool = hierarchical_thread_pool_create(12, 12, 13, false);
    if (!pool) {
        return false;
    }
    
    printf("Created pool with 12-fold symmetry\n");
    
    // Create hierarchical structure: 1 root + 11 workers
    HierarchicalThread* root = hierarchical_thread_create(0, THREAD_ROLE_CONTROL, NULL, pool);
    if (!root) {
        hierarchical_thread_pool_free(pool);
        return false;
    }
    
    HierarchicalThread* workers[11];
    for (uint32_t i = 0; i < 11; i++) {
        workers[i] = hierarchical_thread_create(i + 1, THREAD_ROLE_WORKER, root, pool);
        if (!workers[i]) {
            for (uint32_t j = 0; j < i; j++) {
                hierarchical_thread_free(workers[j]);
            }
            hierarchical_thread_free(root);
            hierarchical_thread_pool_free(pool);
            return false;
        }
    }
    
    printf("Created hierarchical structure: 1 root + 11 workers\n");
    printf("Root has %u children\n", root->num_children);
    
    // Set up pool threads array
    pool->threads = malloc(12 * sizeof(HierarchicalThread*));
    pool->threads[0] = root;
    for (uint32_t i = 0; i < 11; i++) {
        pool->threads[i + 1] = workers[i];
    }
    pool->num_threads = 12;
    
    // Add neighbors between workers (kissing spheres)
    for (uint32_t i = 0; i < 11; i++) {
        for (uint32_t j = i + 1; j < 11; j++) {
            double dist = hierarchical_thread_distance(workers[i], workers[j]);
            if (dist < 2.0) {  // Threshold for "kissing"
                hierarchical_thread_add_neighbor(workers[i], j + 1, THREAD_REL_NEIGHBOR, dist, pool);
            }
        }
    }
    
    printf("Added neighbor relationships\n");
    
    // Test state transitions
    hierarchical_thread_change_state(root, STATE_READY);
    for (uint32_t i = 0; i < 11; i++) {
        hierarchical_thread_change_state(workers[i], STATE_READY);
    }
    
    printf("All threads in READY state\n");
    
    // Get statistics
    HierarchicalThreadPoolStats stats;
    hierarchical_thread_pool_get_stats(pool, &stats);
    
    printf("\nFinal Pool Statistics:\n");
    printf("  Threads: %u\n", stats.num_threads);
    printf("  Symmetry: %u-fold\n", stats.symmetry_fold);
    printf("  Total memory: %zu bytes\n", stats.total_memory_used);
    
    // Cleanup
    for (uint32_t i = 0; i < 11; i++) {
        hierarchical_thread_free(workers[i]);
    }
    hierarchical_thread_free(root);
    free(pool->threads);
    hierarchical_thread_pool_free(pool);
    
    printf("Complete integration test passed!\n");
    
    return true;
}

// ============================================================================
// MAIN
// ============================================================================

int main() {
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║  Week 5 Integration Tests: Hierarchical Threading System      ║\n");
    printf("║  Testing complete integration of Weeks 1-4                    ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n\n");
    
    TEST(thread_pool_creation);
    TEST(thread_creation);
    TEST(geometric_positioning);
    TEST(neighbor_operations);
    TEST(state_management);
    TEST(memory_operations);
    TEST(statistics);
    TEST(find_nearest_neighbors);
    TEST(complete_integration);
    
    printf("\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║  Test Results                                                  ║\n");
    printf("╠════════════════════════════════════════════════════════════════╣\n");
    printf("║  Passed: %2d                                                    ║\n", tests_passed);
    printf("║  Failed: %2d                                                    ║\n", tests_failed);
    printf("║  Total:  %2d                                                    ║\n", tests_passed + tests_failed);
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    return (tests_failed == 0) ? 0 : 1;
}