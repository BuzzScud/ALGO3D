#include "hierarchical_structures.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>

// Test counter
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) \
    do { \
        printf("Testing %s... ", name); \
        fflush(stdout); \
    } while(0)

#define PASS() \
    do { \
        printf("✓ PASS\n"); \
        tests_passed++; \
    } while(0)

#define FAIL(msg) \
    do { \
        printf("✗ FAIL: %s\n", msg); \
        tests_failed++; \
    } while(0)

// ============================================================================
// TREE NODE TESTS
// ============================================================================

void test_node_creation() {
    TEST("Node Creation");
    
    HierarchicalNode* node = hierarchical_node_create(1, 0, 12, 11, NULL, NULL);
    
    if (!node) {
        FAIL("Failed to create node");
        return;
    }
    
    if (node->node_id != 1 || node->level != 0) {
        FAIL("Incorrect node properties");
        hierarchical_node_free(node);
        return;
    }
    
    if (node->max_children != 12 || node->max_siblings != 11) {
        FAIL("Incorrect capacity");
        hierarchical_node_free(node);
        return;
    }
    
    hierarchical_node_free(node);
    PASS();
}

void test_parent_child_relationship() {
    TEST("Parent-Child Relationship");
    
    HierarchicalNode* parent = hierarchical_node_create(1, 0, 12, 11, NULL, NULL);
    HierarchicalNode* child1 = hierarchical_node_create(2, 1, 12, 11, NULL, NULL);
    HierarchicalNode* child2 = hierarchical_node_create(3, 1, 12, 11, NULL, NULL);
    
    if (!parent || !child1 || !child2) {
        FAIL("Failed to create nodes");
        return;
    }
    
    // Add children
    if (hierarchical_node_add_child(parent, child1) != 0) {
        FAIL("Failed to add child1");
        return;
    }
    
    if (hierarchical_node_add_child(parent, child2) != 0) {
        FAIL("Failed to add child2");
        return;
    }
    
    // Verify relationships
    if (parent->num_children != 2) {
        FAIL("Incorrect number of children");
        hierarchical_node_free(parent);
        return;
    }
    
    if (child1->parent != parent || child2->parent != parent) {
        FAIL("Incorrect parent references");
        hierarchical_node_free(parent);
        return;
    }
    
    hierarchical_node_free(parent);
    PASS();
}

void test_sibling_relationships() {
    TEST("Sibling Relationships");
    
    HierarchicalNode* node1 = hierarchical_node_create(1, 1, 12, 11, NULL, NULL);
    HierarchicalNode* node2 = hierarchical_node_create(2, 1, 12, 11, NULL, NULL);
    HierarchicalNode* node3 = hierarchical_node_create(3, 1, 12, 11, NULL, NULL);
    
    if (!node1 || !node2 || !node3) {
        FAIL("Failed to create nodes");
        return;
    }
    
    // Add siblings manually
    if (hierarchical_node_add_sibling(node1, node2) != 0) {
        FAIL("Failed to add sibling");
        return;
    }
    
    if (hierarchical_node_add_sibling(node1, node3) != 0) {
        FAIL("Failed to add sibling");
        return;
    }
    
    if (node1->num_siblings != 2) {
        FAIL("Incorrect number of siblings");
        hierarchical_node_free(node1);
        hierarchical_node_free(node2);
        hierarchical_node_free(node3);
        return;
    }
    
    hierarchical_node_free(node1);
    hierarchical_node_free(node2);
    hierarchical_node_free(node3);
    PASS();
}

void test_sibling_discovery() {
    TEST("Sibling Discovery");
    
    HierarchicalNode* nodes[3];
    nodes[0] = hierarchical_node_create(1, 1, 12, 11, NULL, NULL);
    nodes[1] = hierarchical_node_create(2, 1, 12, 11, NULL, NULL);
    nodes[2] = hierarchical_node_create(3, 1, 12, 11, NULL, NULL);
    
    if (!nodes[0] || !nodes[1] || !nodes[2]) {
        FAIL("Failed to create nodes");
        return;
    }
    
    // Discover siblings
    hierarchical_node_discover_siblings(nodes, 3);
    
    // Each node should have 2 siblings
    if (nodes[0]->num_siblings != 2 || 
        nodes[1]->num_siblings != 2 || 
        nodes[2]->num_siblings != 2) {
        FAIL("Incorrect sibling discovery");
        for (int i = 0; i < 3; i++) hierarchical_node_free(nodes[i]);
        return;
    }
    
    for (int i = 0; i < 3; i++) hierarchical_node_free(nodes[i]);
    PASS();
}

void test_node_find() {
    TEST("Node Find");
    
    HierarchicalNode* root = hierarchical_node_create(1, 0, 12, 11, NULL, NULL);
    HierarchicalNode* child1 = hierarchical_node_create(2, 1, 12, 11, NULL, NULL);
    HierarchicalNode* child2 = hierarchical_node_create(3, 1, 12, 11, NULL, NULL);
    HierarchicalNode* grandchild = hierarchical_node_create(4, 2, 12, 11, NULL, NULL);
    
    hierarchical_node_add_child(root, child1);
    hierarchical_node_add_child(root, child2);
    hierarchical_node_add_child(child1, grandchild);
    
    // Find nodes
    HierarchicalNode* found = hierarchical_node_find(root, 4);
    if (!found || found->node_id != 4) {
        FAIL("Failed to find grandchild");
        hierarchical_node_free(root);
        return;
    }
    
    found = hierarchical_node_find(root, 2);
    if (!found || found->node_id != 2) {
        FAIL("Failed to find child1");
        hierarchical_node_free(root);
        return;
    }
    
    found = hierarchical_node_find(root, 999);
    if (found != NULL) {
        FAIL("Found non-existent node");
        hierarchical_node_free(root);
        return;
    }
    
    hierarchical_node_free(root);
    PASS();
}

// Visitor callback for traversal tests
static int visit_count = 0;
static int visit_order[10];

static int count_visitor(HierarchicalNode* node, void* user_data) {
    visit_order[visit_count++] = node->node_id;
    return 0;
}

void test_preorder_traversal() {
    TEST("Preorder Traversal");
    
    HierarchicalNode* root = hierarchical_node_create(1, 0, 12, 11, NULL, NULL);
    HierarchicalNode* child1 = hierarchical_node_create(2, 1, 12, 11, NULL, NULL);
    HierarchicalNode* child2 = hierarchical_node_create(3, 1, 12, 11, NULL, NULL);
    HierarchicalNode* grandchild = hierarchical_node_create(4, 2, 12, 11, NULL, NULL);
    
    hierarchical_node_add_child(root, child1);
    hierarchical_node_add_child(root, child2);
    hierarchical_node_add_child(child1, grandchild);
    
    visit_count = 0;
    hierarchical_node_traverse(root, TRAVERSE_PREORDER, count_visitor, NULL);
    
    // Preorder: root, child1, grandchild, child2
    int expected[] = {1, 2, 4, 3};
    int correct = 1;
    for (int i = 0; i < 4; i++) {
        if (visit_order[i] != expected[i]) {
            correct = 0;
            break;
        }
    }
    
    if (!correct || visit_count != 4) {
        FAIL("Incorrect preorder traversal");
        hierarchical_node_free(root);
        return;
    }
    
    hierarchical_node_free(root);
    PASS();
}

void test_postorder_traversal() {
    TEST("Postorder Traversal");
    
    HierarchicalNode* root = hierarchical_node_create(1, 0, 12, 11, NULL, NULL);
    HierarchicalNode* child1 = hierarchical_node_create(2, 1, 12, 11, NULL, NULL);
    HierarchicalNode* child2 = hierarchical_node_create(3, 1, 12, 11, NULL, NULL);
    HierarchicalNode* grandchild = hierarchical_node_create(4, 2, 12, 11, NULL, NULL);
    
    hierarchical_node_add_child(root, child1);
    hierarchical_node_add_child(root, child2);
    hierarchical_node_add_child(child1, grandchild);
    
    visit_count = 0;
    hierarchical_node_traverse(root, TRAVERSE_POSTORDER, count_visitor, NULL);
    
    // Postorder: grandchild, child1, child2, root
    int expected[] = {4, 2, 3, 1};
    int correct = 1;
    for (int i = 0; i < 4; i++) {
        if (visit_order[i] != expected[i]) {
            correct = 0;
            break;
        }
    }
    
    if (!correct || visit_count != 4) {
        FAIL("Incorrect postorder traversal");
        hierarchical_node_free(root);
        return;
    }
    
    hierarchical_node_free(root);
    PASS();
}

void test_levelorder_traversal() {
    TEST("Level-Order Traversal");
    
    HierarchicalNode* root = hierarchical_node_create(1, 0, 12, 11, NULL, NULL);
    HierarchicalNode* child1 = hierarchical_node_create(2, 1, 12, 11, NULL, NULL);
    HierarchicalNode* child2 = hierarchical_node_create(3, 1, 12, 11, NULL, NULL);
    HierarchicalNode* grandchild = hierarchical_node_create(4, 2, 12, 11, NULL, NULL);
    
    hierarchical_node_add_child(root, child1);
    hierarchical_node_add_child(root, child2);
    hierarchical_node_add_child(child1, grandchild);
    
    visit_count = 0;
    hierarchical_node_traverse(root, TRAVERSE_LEVELORDER, count_visitor, NULL);
    
    // Level-order: root, child1, child2, grandchild
    int expected[] = {1, 2, 3, 4};
    int correct = 1;
    for (int i = 0; i < 4; i++) {
        if (visit_order[i] != expected[i]) {
            correct = 0;
            break;
        }
    }
    
    if (!correct || visit_count != 4) {
        FAIL("Incorrect level-order traversal");
        hierarchical_node_free(root);
        return;
    }
    
    hierarchical_node_free(root);
    PASS();
}

void test_tree_depth() {
    TEST("Tree Depth");
    
    HierarchicalNode* root = hierarchical_node_create(1, 0, 12, 11, NULL, NULL);
    HierarchicalNode* child1 = hierarchical_node_create(2, 1, 12, 11, NULL, NULL);
    HierarchicalNode* grandchild = hierarchical_node_create(3, 2, 12, 11, NULL, NULL);
    
    hierarchical_node_add_child(root, child1);
    hierarchical_node_add_child(child1, grandchild);
    
    int depth = hierarchical_node_get_depth(root);
    if (depth != 3) {
        FAIL("Incorrect tree depth");
        hierarchical_node_free(root);
        return;
    }
    
    hierarchical_node_free(root);
    PASS();
}

void test_node_count() {
    TEST("Node Count");
    
    HierarchicalNode* root = hierarchical_node_create(1, 0, 12, 11, NULL, NULL);
    HierarchicalNode* child1 = hierarchical_node_create(2, 1, 12, 11, NULL, NULL);
    HierarchicalNode* child2 = hierarchical_node_create(3, 1, 12, 11, NULL, NULL);
    HierarchicalNode* grandchild = hierarchical_node_create(4, 2, 12, 11, NULL, NULL);
    
    hierarchical_node_add_child(root, child1);
    hierarchical_node_add_child(root, child2);
    hierarchical_node_add_child(child1, grandchild);
    
    int count = hierarchical_node_count(root);
    if (count != 4) {
        FAIL("Incorrect node count");
        hierarchical_node_free(root);
        return;
    }
    
    hierarchical_node_free(root);
    PASS();
}

// ============================================================================
// STATE MANAGEMENT TESTS
// ============================================================================

void test_state_management() {
    TEST("State Management");
    
    HierarchicalNode* node = hierarchical_node_create(1, 0, 12, 11, NULL, NULL);
    
    // Set and get state
    hierarchical_node_set_state(node, 42);
    int state = hierarchical_node_get_state(node);
    
    if (state != 42) {
        FAIL("Incorrect state");
        hierarchical_node_free(node);
        return;
    }
    
    hierarchical_node_free(node);
    PASS();
}

// Thread function for state wait test
void* state_waiter_thread(void* arg) {
    HierarchicalNode* node = (HierarchicalNode*)arg;
    
    // Wait for state 100
    int result = hierarchical_node_wait_for_state(node, 100, 5000);
    
    return (void*)(intptr_t)result;
}

void test_state_wait() {
    TEST("State Wait");
    
    HierarchicalNode* node = hierarchical_node_create(1, 0, 12, 11, NULL, NULL);
    hierarchical_node_set_state(node, 0);
    
    // Start thread that waits for state 100
    pthread_t thread;
    pthread_create(&thread, NULL, state_waiter_thread, node);
    
    // Sleep briefly then set state
    usleep(100000);  // 100ms
    hierarchical_node_set_state(node, 100);
    
    // Wait for thread
    void* result;
    pthread_join(thread, &result);
    
    if ((intptr_t)result != 0) {
        FAIL("State wait failed");
        hierarchical_node_free(node);
        return;
    }
    
    hierarchical_node_free(node);
    PASS();
}

// ============================================================================
// SYNCHRONIZATION BARRIER TESTS
// ============================================================================

void test_barrier_creation() {
    TEST("Barrier Creation");
    
    SyncBarrier* barrier = sync_barrier_create(4);
    
    if (!barrier) {
        FAIL("Failed to create barrier");
        return;
    }
    
    if (atomic_load(&barrier->required) != 4) {
        FAIL("Incorrect barrier count");
        sync_barrier_free(barrier);
        return;
    }
    
    sync_barrier_free(barrier);
    PASS();
}

// Thread function for barrier test
static SyncBarrier* test_barrier = NULL;
static atomic_int barrier_count = ATOMIC_VAR_INIT(0);

void* barrier_thread(void* arg) {
    int id = (int)(intptr_t)arg;
    
    // Increment counter before barrier
    atomic_fetch_add(&barrier_count, 1);
    
    // Wait at barrier
    sync_barrier_wait(test_barrier);
    
    // All threads should have reached here together
    return NULL;
}

void test_barrier_synchronization() {
    TEST("Barrier Synchronization");
    
    const int num_threads = 4;
    test_barrier = sync_barrier_create(num_threads);
    atomic_store(&barrier_count, 0);
    
    pthread_t threads[num_threads];
    
    // Create threads
    for (int i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, barrier_thread, (void*)(intptr_t)i);
    }
    
    // Wait for all threads
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // All threads should have incremented the counter
    if (atomic_load(&barrier_count) != num_threads) {
        FAIL("Not all threads reached barrier");
        sync_barrier_free(test_barrier);
        return;
    }
    
    sync_barrier_free(test_barrier);
    PASS();
}

// ============================================================================
// WORK QUEUE TESTS
// ============================================================================

void test_work_queue_creation() {
    TEST("Work Queue Creation");
    
    WorkQueue* queue = work_queue_create(100);
    
    if (!queue) {
        FAIL("Failed to create work queue");
        return;
    }
    
    if (queue->capacity != 100) {
        FAIL("Incorrect capacity");
        work_queue_free(queue);
        return;
    }
    
    if (!work_queue_is_empty(queue)) {
        FAIL("New queue should be empty");
        work_queue_free(queue);
        return;
    }
    
    work_queue_free(queue);
    PASS();
}

void test_work_queue_enqueue_dequeue() {
    TEST("Work Queue Enqueue/Dequeue");
    
    WorkQueue* queue = work_queue_create(10);
    
    // Enqueue items
    for (int i = 0; i < 5; i++) {
        int* data = malloc(sizeof(int));
        *data = i * 10;
        if (work_queue_enqueue(queue, i, data, free) != 0) {
            FAIL("Failed to enqueue");
            work_queue_free(queue);
            return;
        }
    }
    
    if (work_queue_size(queue) != 5) {
        FAIL("Incorrect queue size");
        work_queue_free(queue);
        return;
    }
    
    // Dequeue items
    for (int i = 0; i < 5; i++) {
        WorkItem item;
        if (work_queue_dequeue(queue, &item) != 0) {
            FAIL("Failed to dequeue");
            work_queue_free(queue);
            return;
        }
        
        if (item.id != (uint64_t)i) {
            FAIL("Incorrect item ID");
            free(item.data);
            work_queue_free(queue);
            return;
        }
        
        int* data = (int*)item.data;
        if (*data != i * 10) {
            FAIL("Incorrect item data");
            free(item.data);
            work_queue_free(queue);
            return;
        }
        
        free(item.data);
    }
    
    if (!work_queue_is_empty(queue)) {
        FAIL("Queue should be empty");
        work_queue_free(queue);
        return;
    }
    
    work_queue_free(queue);
    PASS();
}

void test_work_queue_peek() {
    TEST("Work Queue Peek");
    
    WorkQueue* queue = work_queue_create(10);
    
    int* data = malloc(sizeof(int));
    *data = 42;
    work_queue_enqueue(queue, 1, data, free);
    
    // Peek at item
    WorkItem item;
    if (work_queue_peek(queue, &item) != 0) {
        FAIL("Failed to peek");
        work_queue_free(queue);
        return;
    }
    
    if (item.id != 1 || *(int*)item.data != 42) {
        FAIL("Incorrect peeked item");
        work_queue_free(queue);
        return;
    }
    
    // Queue should still have the item
    if (work_queue_size(queue) != 1) {
        FAIL("Peek should not remove item");
        work_queue_free(queue);
        return;
    }
    
    work_queue_free(queue);
    PASS();
}

void test_work_queue_full() {
    TEST("Work Queue Full");
    
    WorkQueue* queue = work_queue_create(3);
    
    // Fill queue
    for (int i = 0; i < 3; i++) {
        work_queue_enqueue(queue, i, NULL, NULL);
    }
    
    if (!work_queue_is_full(queue)) {
        FAIL("Queue should be full");
        work_queue_free(queue);
        return;
    }
    
    // Try to enqueue when full
    if (work_queue_enqueue(queue, 99, NULL, NULL) == 0) {
        FAIL("Should not be able to enqueue when full");
        work_queue_free(queue);
        return;
    }
    
    work_queue_free(queue);
    PASS();
}

void test_work_stealing() {
    TEST("Work Stealing");
    
    WorkQueue* victim = work_queue_create(10);
    WorkQueue* thief = work_queue_create(10);
    
    // Enable stealing on victim
    work_queue_enable_stealing(victim);
    
    // Add work to victim
    for (int i = 0; i < 5; i++) {
        int* data = malloc(sizeof(int));
        *data = i;
        work_queue_enqueue(victim, i, data, free);
    }
    
    // Steal work
    WorkItem item;
    if (work_queue_steal(thief, victim, &item) != 0) {
        FAIL("Failed to steal work");
        work_queue_free(victim);
        work_queue_free(thief);
        return;
    }
    
    // Verify stolen item (should be most recent = 4)
    if (item.id != 4 || *(int*)item.data != 4) {
        FAIL("Incorrect stolen item");
        free(item.data);
        work_queue_free(victim);
        work_queue_free(thief);
        return;
    }
    free(item.data);
    
    // Verify sizes
    if (work_queue_size(victim) != 4) {
        FAIL("Victim queue size incorrect after steal");
        work_queue_free(victim);
        work_queue_free(thief);
        return;
    }
    
    // Verify statistics
    uint64_t stolen_from, stolen_to;
    work_queue_get_steal_stats(victim, &stolen_from, &stolen_to);
    if (stolen_from != 1) {
        FAIL("Incorrect steal statistics");
        work_queue_free(victim);
        work_queue_free(thief);
        return;
    }
    
    work_queue_free(victim);
    work_queue_free(thief);
    PASS();
}

void test_work_stealing_disabled() {
    TEST("Work Stealing Disabled");
    
    WorkQueue* victim = work_queue_create(10);
    WorkQueue* thief = work_queue_create(10);
    
    // Do NOT enable stealing on victim
    
    // Add work to victim
    work_queue_enqueue(victim, 1, NULL, NULL);
    
    // Try to steal (should fail)
    WorkItem item;
    if (work_queue_steal(thief, victim, &item) == 0) {
        FAIL("Should not be able to steal when disabled");
        work_queue_free(victim);
        work_queue_free(thief);
        return;
    }
    
    work_queue_free(victim);
    work_queue_free(thief);
    PASS();
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main() {
    printf("=== Hierarchical Structures Tests ===\n\n");
    
    // Tree node tests
    test_node_creation();
    test_parent_child_relationship();
    test_sibling_relationships();
    test_sibling_discovery();
    test_node_find();
    test_preorder_traversal();
    test_postorder_traversal();
    test_levelorder_traversal();
    test_tree_depth();
    test_node_count();
    
    // State management tests
    test_state_management();
    test_state_wait();
    
    // Synchronization barrier tests
    test_barrier_creation();
    test_barrier_synchronization();
    
    // Work queue tests
    test_work_queue_creation();
    test_work_queue_enqueue_dequeue();
    test_work_queue_peek();
    test_work_queue_full();
    test_work_stealing();
    test_work_stealing_disabled();
    
    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Total:  %d\n", tests_passed + tests_failed);
    
    if (tests_failed == 0) {
        printf("\n✓ All tests passed!\n");
        return 0;
    } else {
        printf("\n✗ Some tests failed!\n");
        return 1;
    }
}
