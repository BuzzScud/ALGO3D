/**
 * @file test_cllm_adapter.c
 * @brief Tests for CLLM Algorithm Library Adapter
 */

#include "ai/cllm_algorithm_adapter.h"
#include "ai/cllm_lattice_hierarchy.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

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
// TEST 1: Adapter Pool Creation
// ============================================================================

bool test_adapter_pool_creation() {
    printf("Creating adapter pool with 12 spheres...\n");
    
    CLLMAdapterPool* pool = cllm_adapter_pool_create(
        12,  // 12 spheres
        12,  // 12-fold symmetry
        13   // 13 dimensions
    );
    
    if (!pool) {
        printf("Failed to create adapter pool\n");
        return false;
    }
    
    printf("Adapter pool created successfully\n");
    printf("  Max adapters: %u\n", pool->max_adapters);
    printf("  Symmetry fold: %u\n", pool->symmetry_fold);
    printf("  Dimensions: %u\n", pool->num_dimensions);
    printf("  Initialized: %s\n", pool->initialized ? "yes" : "no");
    
    // Verify thread pool was created
    if (!pool->thread_pool) {
        printf("Thread pool not created\n");
        cllm_adapter_pool_free(pool);
        return false;
    }
    
    printf("Thread pool created successfully\n");
    
    cllm_adapter_pool_free(pool);
    return true;
}

// ============================================================================
// TEST 2: State Mapping
// ============================================================================

bool test_state_mapping() {
    printf("Testing state mapping between CLLM and generic states...\n");
    
    // Test CLLM -> Generic mapping
    struct {
        HierarchyState cllm_state;
        StateType expected_generic;
    } cllm_to_generic_tests[] = {
        {HIERARCHY_STATE_INITIALIZING, STATE_INITIALIZING},
        {HIERARCHY_STATE_READY, STATE_READY},
        {HIERARCHY_STATE_PROCESSING, STATE_RUNNING},
        {HIERARCHY_STATE_WAITING, STATE_WAITING},
        {HIERARCHY_STATE_IDLE, STATE_IDLE},
        {HIERARCHY_STATE_TERMINATING, STATE_STOPPING},
        {HIERARCHY_STATE_TERMINATED, STATE_STOPPED},
    };
    
    for (size_t i = 0; i < sizeof(cllm_to_generic_tests) / sizeof(cllm_to_generic_tests[0]); i++) {
        StateType result = cllm_state_to_generic(cllm_to_generic_tests[i].cllm_state);
        if (result != cllm_to_generic_tests[i].expected_generic) {
            printf("CLLM->Generic mapping failed for state %d: expected %d, got %d\n",
                   cllm_to_generic_tests[i].cllm_state,
                   cllm_to_generic_tests[i].expected_generic,
                   result);
            return false;
        }
    }
    
    printf("CLLM->Generic mapping: OK\n");
    
    // Test Generic -> CLLM mapping
    struct {
        StateType generic_state;
        HierarchyState expected_cllm;
    } generic_to_cllm_tests[] = {
        {STATE_INITIALIZING, HIERARCHY_STATE_INITIALIZING},
        {STATE_READY, HIERARCHY_STATE_READY},
        {STATE_RUNNING, HIERARCHY_STATE_PROCESSING},
        {STATE_WAITING, HIERARCHY_STATE_WAITING},
        {STATE_IDLE, HIERARCHY_STATE_IDLE},
        {STATE_STOPPING, HIERARCHY_STATE_TERMINATING},
        {STATE_STOPPED, HIERARCHY_STATE_TERMINATED},
    };
    
    for (size_t i = 0; i < sizeof(generic_to_cllm_tests) / sizeof(generic_to_cllm_tests[0]); i++) {
        HierarchyState result = generic_state_to_cllm(generic_to_cllm_tests[i].generic_state);
        if (result != generic_to_cllm_tests[i].expected_cllm) {
            printf("Generic->CLLM mapping failed for state %d: expected %d, got %d\n",
                   generic_to_cllm_tests[i].generic_state,
                   generic_to_cllm_tests[i].expected_cllm,
                   result);
            return false;
        }
    }
    
    printf("Generic->CLLM mapping: OK\n");
    
    return true;
}

// ============================================================================
// TEST 3: Adapter Creation
// ============================================================================

bool test_adapter_creation() {
    printf("Testing adapter creation...\n");
    
    // Create adapter pool
    CLLMAdapterPool* pool = cllm_adapter_pool_create(4, 12, 13);
    if (!pool) {
        return false;
    }
    
    // Create a minimal CLLM hierarchy structure
    CLLMLatticeHierarchy* cllm_hierarchy = calloc(1, sizeof(CLLMLatticeHierarchy));
    if (!cllm_hierarchy) {
        cllm_adapter_pool_free(pool);
        return false;
    }
    
    cllm_hierarchy->sphere_id = 0;
    cllm_hierarchy->hierarchy_level = 0;
    cllm_hierarchy->num_symmetry_groups = 1;
    cllm_hierarchy->symmetry_groups[0] = 0;
    cllm_hierarchy->primary_symmetry_group = 0;
    atomic_init(&cllm_hierarchy->state, HIERARCHY_STATE_INITIALIZING);
    
    // Create adapter
    CLLMAlgorithmAdapter* adapter = cllm_adapter_create(cllm_hierarchy, pool);
    if (!adapter) {
        printf("Failed to create adapter\n");
        free(cllm_hierarchy);
        cllm_adapter_pool_free(pool);
        return false;
    }
    
    printf("Adapter created successfully\n");
    printf("  Adapter ID: %u\n", adapter->adapter_id);
    printf("  Initialized: %s\n", adapter->initialized ? "yes" : "no");
    
    // Initialize adapter
    int result = cllm_adapter_initialize(adapter);
    if (result != 0) {
        printf("Failed to initialize adapter\n");
        cllm_adapter_free(adapter);
        free(cllm_hierarchy);
        cllm_adapter_pool_free(pool);
        return false;
    }
    
    printf("Adapter initialized successfully\n");
    printf("  Generic thread created: %s\n", adapter->generic_thread ? "yes" : "no");
    
    // Cleanup
    cllm_adapter_free(adapter);
    free(cllm_hierarchy);
    cllm_adapter_pool_free(pool);
    
    return true;
}

// ============================================================================
// TEST 4: Gradient Buffer Allocation
// ============================================================================

bool test_gradient_buffer_allocation() {
    printf("Testing gradient buffer allocation...\n");
    
    // Create adapter pool
    CLLMAdapterPool* pool = cllm_adapter_pool_create(2, 12, 13);
    if (!pool) {
        return false;
    }
    
    // Create CLLM hierarchy
    CLLMLatticeHierarchy* cllm_hierarchy = calloc(1, sizeof(CLLMLatticeHierarchy));
    if (!cllm_hierarchy) {
        cllm_adapter_pool_free(pool);
        return false;
    }
    
    cllm_hierarchy->sphere_id = 0;
    cllm_hierarchy->hierarchy_level = 0;
    atomic_init(&cllm_hierarchy->state, HIERARCHY_STATE_INITIALIZING);
    pthread_mutex_init(&cllm_hierarchy->gradient_mutex, NULL);
    
    // Create and initialize adapter
    CLLMAlgorithmAdapter* adapter = cllm_adapter_create(cllm_hierarchy, pool);
    if (!adapter) {
        pthread_mutex_destroy(&cllm_hierarchy->gradient_mutex);
        free(cllm_hierarchy);
        cllm_adapter_pool_free(pool);
        return false;
    }
    
    cllm_adapter_initialize(adapter);
    
    // Allocate gradient buffer
    size_t buffer_size = 1000;
    int result = cllm_adapter_alloc_gradient_buffer(adapter, buffer_size);
    
    if (result != 0) {
        printf("Failed to allocate gradient buffer\n");
        cllm_adapter_free(adapter);
        pthread_mutex_destroy(&cllm_hierarchy->gradient_mutex);
        free(cllm_hierarchy);
        cllm_adapter_pool_free(pool);
        return false;
    }
    
    printf("Gradient buffer allocated successfully\n");
    printf("  Buffer size: %zu\n", cllm_hierarchy->gradient_buffer_size);
    printf("  Gradient buffer: %p\n", (void*)cllm_hierarchy->gradient_buffer);
    printf("  Child gradients: %p\n", (void*)cllm_hierarchy->child_gradients);
    
    // Verify buffers are allocated
    if (!cllm_hierarchy->gradient_buffer || !cllm_hierarchy->child_gradients) {
        printf("Buffers not properly allocated\n");
        cllm_adapter_free(adapter);
        pthread_mutex_destroy(&cllm_hierarchy->gradient_mutex);
        free(cllm_hierarchy);
        cllm_adapter_pool_free(pool);
        return false;
    }
    
    // Cleanup
    free(cllm_hierarchy->gradient_buffer);
    free(cllm_hierarchy->child_gradients);
    cllm_adapter_free(adapter);
    pthread_mutex_destroy(&cllm_hierarchy->gradient_mutex);
    free(cllm_hierarchy);
    cllm_adapter_pool_free(pool);
    
    return true;
}

// ============================================================================
// TEST 5: State Operations
// ============================================================================

bool test_state_operations() {
    printf("Testing state operations...\n");
    
    // Create adapter pool
    CLLMAdapterPool* pool = cllm_adapter_pool_create(2, 12, 13);
    if (!pool) {
        return false;
    }
    
    // Create CLLM hierarchy
    CLLMLatticeHierarchy* cllm_hierarchy = calloc(1, sizeof(CLLMLatticeHierarchy));
    if (!cllm_hierarchy) {
        cllm_adapter_pool_free(pool);
        return false;
    }
    
    cllm_hierarchy->sphere_id = 0;
    cllm_hierarchy->hierarchy_level = 0;
    atomic_init(&cllm_hierarchy->state, HIERARCHY_STATE_INITIALIZING);
    
    // Create and initialize adapter
    CLLMAlgorithmAdapter* adapter = cllm_adapter_create(cllm_hierarchy, pool);
    if (!adapter) {
        free(cllm_hierarchy);
        cllm_adapter_pool_free(pool);
        return false;
    }
    
    cllm_adapter_initialize(adapter);
    
    // Test state transitions
    printf("Testing state transition: INITIALIZING -> READY\n");
    TransitionResult result = cllm_adapter_change_state(adapter, HIERARCHY_STATE_READY);
    
    if (result != TRANSITION_SUCCESS) {
        printf("State transition failed: %d\n", result);
        cllm_adapter_free(adapter);
        free(cllm_hierarchy);
        cllm_adapter_pool_free(pool);
        return false;
    }
    
    // Verify state changed
    HierarchyState current_state = cllm_adapter_get_state(adapter);
    printf("Current state: %d (expected: %d)\n", current_state, HIERARCHY_STATE_READY);
    
    if (current_state != HIERARCHY_STATE_READY) {
        printf("State not updated correctly\n");
        cllm_adapter_free(adapter);
        free(cllm_hierarchy);
        cllm_adapter_pool_free(pool);
        return false;
    }
    
    printf("State operations working correctly\n");
    
    // Cleanup
    cllm_adapter_free(adapter);
    free(cllm_hierarchy);
    cllm_adapter_pool_free(pool);
    
    return true;
}

// ============================================================================
// MAIN
// ============================================================================

int main() {
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║  CLLM Algorithm Library Adapter Tests                         ║\n");
    printf("║  Testing adapter between CLLM and algorithm library            ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n\n");
    
    TEST(adapter_pool_creation);
    TEST(state_mapping);
    TEST(adapter_creation);
    TEST(gradient_buffer_allocation);
    TEST(state_operations);
    
    printf("\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║  Test Results                                                  ║\n");
    printf("╠════════════════════════════════════════════════════════════════╣\n");
    printf("║  Passed: %2d                                                    ║\n", tests_passed);
    printf("║  Failed: %2d                                                    ║\n", tests_failed);
    printf("║  Total:  %2d                                                    ║\n", tests_passed + tests_failed);
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    return (tests_failed == 0) ? 0 : 1;
}