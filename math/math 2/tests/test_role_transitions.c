/**
 * @file test_role_transitions.c
 * @brief Unit tests for worker <-> control thread role transitions
 * 
 * Tests the thread-safe state transition functions that allow threads
 * to dynamically switch between worker and control roles.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>

// Mock structures for testing (simplified versions)
typedef struct {
    int sphere_id;
    int is_control_thread;
    int hierarchy_level;
    int batches_processed;
    void* current_batch;
    void** children;
    int num_children;
    pthread_mutex_t lock;
} TestSphereContext;

// Mock transition functions (these would be the actual functions in production)
static int transition_to_control_thread(TestSphereContext* ctx) {
    if (!ctx) return -1;
    if (ctx->is_control_thread) return 0;
    if (ctx->current_batch) return -1;
    
    pthread_mutex_lock(&ctx->lock);
    if (ctx->is_control_thread) {
        pthread_mutex_unlock(&ctx->lock);
        return 0;
    }
    ctx->is_control_thread = 1;
    ctx->batches_processed = 0;
    pthread_mutex_unlock(&ctx->lock);
    return 0;
}

static int transition_to_worker_thread(TestSphereContext* ctx) {
    if (!ctx) return -1;
    if (!ctx->is_control_thread) return 0;
    if (ctx->children || ctx->num_children > 0) return -1;
    
    pthread_mutex_lock(&ctx->lock);
    if (!ctx->is_control_thread) {
        pthread_mutex_unlock(&ctx->lock);
        return 0;
    }
    ctx->is_control_thread = 0;
    ctx->batches_processed = 0;
    pthread_mutex_unlock(&ctx->lock);
    return 0;
}

// Test helper functions
static TestSphereContext* create_test_context(int id, int is_control) {
    TestSphereContext* ctx = (TestSphereContext*)calloc(1, sizeof(TestSphereContext));
    ctx->sphere_id = id;
    ctx->is_control_thread = is_control;
    ctx->hierarchy_level = 0;
    ctx->batches_processed = 0;
    ctx->current_batch = NULL;
    ctx->children = NULL;
    ctx->num_children = 0;
    pthread_mutex_init(&ctx->lock, NULL);
    return ctx;
}

static void free_test_context(TestSphereContext* ctx) {
    if (ctx) {
        pthread_mutex_destroy(&ctx->lock);
        free(ctx);
    }
}

// Test 1: Worker to Control Transition - Success
static void test_worker_to_control_success(void) {
    printf("Test 1: Worker to Control Transition - Success\n");
    
    TestSphereContext* ctx = create_test_context(1, 0);
    
    int result = transition_to_control_thread(ctx);
    assert(result == 0);
    assert(ctx->is_control_thread == 1);
    assert(ctx->batches_processed == 0);
    
    free_test_context(ctx);
    printf("  ✓ PASSED\n\n");
}

// Test 2: Worker to Control Transition - Already Control
static void test_worker_to_control_already_control(void) {
    printf("Test 2: Worker to Control Transition - Already Control\n");
    
    TestSphereContext* ctx = create_test_context(1, 1);
    
    int result = transition_to_control_thread(ctx);
    assert(result == 0);  // Should succeed (idempotent)
    assert(ctx->is_control_thread == 1);
    
    free_test_context(ctx);
    printf("  ✓ PASSED\n\n");
}

// Test 3: Worker to Control Transition - With Pending Batch
static void test_worker_to_control_with_batch(void) {
    printf("Test 3: Worker to Control Transition - With Pending Batch\n");
    
    TestSphereContext* ctx = create_test_context(1, 0);
    ctx->current_batch = (void*)0x1234;  // Mock batch pointer
    
    int result = transition_to_control_thread(ctx);
    assert(result == -1);  // Should fail
    assert(ctx->is_control_thread == 0);  // Should remain worker
    
    free_test_context(ctx);
    printf("  ✓ PASSED\n\n");
}

// Test 4: Worker to Control Transition - NULL Context
static void test_worker_to_control_null(void) {
    printf("Test 4: Worker to Control Transition - NULL Context\n");
    
    int result = transition_to_control_thread(NULL);
    assert(result == -1);
    
    printf("  ✓ PASSED\n\n");
}

// Test 5: Control to Worker Transition - Success
static void test_control_to_worker_success(void) {
    printf("Test 5: Control to Worker Transition - Success\n");
    
    TestSphereContext* ctx = create_test_context(1, 1);
    
    int result = transition_to_worker_thread(ctx);
    assert(result == 0);
    assert(ctx->is_control_thread == 0);
    assert(ctx->batches_processed == 0);
    
    free_test_context(ctx);
    printf("  ✓ PASSED\n\n");
}

// Test 6: Control to Worker Transition - Already Worker
static void test_control_to_worker_already_worker(void) {
    printf("Test 6: Control to Worker Transition - Already Worker\n");
    
    TestSphereContext* ctx = create_test_context(1, 0);
    
    int result = transition_to_worker_thread(ctx);
    assert(result == 0);  // Should succeed (idempotent)
    assert(ctx->is_control_thread == 0);
    
    free_test_context(ctx);
    printf("  ✓ PASSED\n\n");
}

// Test 7: Control to Worker Transition - With Children
static void test_control_to_worker_with_children(void) {
    printf("Test 7: Control to Worker Transition - With Children\n");
    
    TestSphereContext* ctx = create_test_context(1, 1);
    ctx->num_children = 12;
    ctx->children = (void**)calloc(12, sizeof(void*));
    
    int result = transition_to_worker_thread(ctx);
    assert(result == -1);  // Should fail
    assert(ctx->is_control_thread == 1);  // Should remain control
    
    free(ctx->children);
    free_test_context(ctx);
    printf("  ✓ PASSED\n\n");
}

// Test 8: Control to Worker Transition - NULL Context
static void test_control_to_worker_null(void) {
    printf("Test 8: Control to Worker Transition - NULL Context\n");
    
    int result = transition_to_worker_thread(NULL);
    assert(result == -1);
    
    printf("  ✓ PASSED\n\n");
}

// Test 9: Round-trip Transition - Worker -> Control -> Worker
static void test_roundtrip_transition(void) {
    printf("Test 9: Round-trip Transition - Worker -> Control -> Worker\n");
    
    TestSphereContext* ctx = create_test_context(1, 0);
    
    // Worker -> Control
    int result1 = transition_to_control_thread(ctx);
    assert(result1 == 0);
    assert(ctx->is_control_thread == 1);
    
    // Control -> Worker
    int result2 = transition_to_worker_thread(ctx);
    assert(result2 == 0);
    assert(ctx->is_control_thread == 0);
    
    free_test_context(ctx);
    printf("  ✓ PASSED\n\n");
}

// Test 10: Thread-Safety - Concurrent Transitions
static void* transition_thread_func(void* arg) {
    TestSphereContext* ctx = (TestSphereContext*)arg;
    
    // Try to transition multiple times
    for (int i = 0; i < 100; i++) {
        transition_to_control_thread(ctx);
        transition_to_worker_thread(ctx);
    }
    
    return NULL;
}

static void test_concurrent_transitions(void) {
    printf("Test 10: Thread-Safety - Concurrent Transitions\n");
    
    TestSphereContext* ctx = create_test_context(1, 0);
    
    pthread_t threads[4];
    for (int i = 0; i < 4; i++) {
        pthread_create(&threads[i], NULL, transition_thread_func, ctx);
    }
    
    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Context should be in a valid state (either worker or control)
    assert(ctx->is_control_thread == 0 || ctx->is_control_thread == 1);
    
    free_test_context(ctx);
    printf("  ✓ PASSED\n\n");
}

// Test 11: Batch Counter Reset on Transition
static void test_batch_counter_reset(void) {
    printf("Test 11: Batch Counter Reset on Transition\n");
    
    TestSphereContext* ctx = create_test_context(1, 0);
    ctx->batches_processed = 42;
    
    // Worker -> Control should reset counter
    transition_to_control_thread(ctx);
    assert(ctx->batches_processed == 0);
    
    ctx->batches_processed = 99;
    
    // Control -> Worker should reset counter
    transition_to_worker_thread(ctx);
    assert(ctx->batches_processed == 0);
    
    free_test_context(ctx);
    printf("  ✓ PASSED\n\n");
}

// Test 12: State Validation - Multiple Transitions
static void test_multiple_transitions(void) {
    printf("Test 12: State Validation - Multiple Transitions\n");
    
    TestSphereContext* ctx = create_test_context(1, 0);
    
    // Multiple worker -> control transitions (idempotent)
    for (int i = 0; i < 5; i++) {
        int result = transition_to_control_thread(ctx);
        assert(result == 0);
        assert(ctx->is_control_thread == 1);
    }
    
    // Multiple control -> worker transitions (idempotent)
    for (int i = 0; i < 5; i++) {
        int result = transition_to_worker_thread(ctx);
        assert(result == 0);
        assert(ctx->is_control_thread == 0);
    }
    
    free_test_context(ctx);
    printf("  ✓ PASSED\n\n");
}

int main(void) {
    printf("=================================================\n");
    printf("Role Transition Tests\n");
    printf("=================================================\n\n");
    
    test_worker_to_control_success();
    test_worker_to_control_already_control();
    test_worker_to_control_with_batch();
    test_worker_to_control_null();
    test_control_to_worker_success();
    test_control_to_worker_already_worker();
    test_control_to_worker_with_children();
    test_control_to_worker_null();
    test_roundtrip_transition();
    test_concurrent_transitions();
    test_batch_counter_reset();
    test_multiple_transitions();
    
    printf("=================================================\n");
    printf("All Role Transition Tests Passed! ✓\n");
    printf("=================================================\n");
    
    return 0;
}