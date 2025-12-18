/*
 * Test Suite for CLLM Thread Adapter
 * Tests the compatibility layer between CLLMLatticeHierarchy and HierarchicalThread
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ai/cllm_thread_adapter.h"

// Test counters
static int tests_passed = 0;
static int tests_failed = 0;

// Test macros
#define TEST_START(name) printf("\n=== Testing: %s ===\n", name)
#define TEST_ASSERT(condition, message) \
    do { \
        if (condition) { \
            printf("  ✓ %s\n", message); \
            tests_passed++; \
        } else { \
            printf("  ✗ %s\n", message); \
            tests_failed++; \
        } \
    } while(0)

// Test 1: Global pool management
void test_global_pool() {
    TEST_START("Global Pool Management");
    
    HierarchicalThreadPool* pool = cllm_adapter_get_global_pool();
    TEST_ASSERT(pool != NULL, "Global pool created");
    TEST_ASSERT(pool->use_88d_structure == true, "Pool uses 88D structure");
    TEST_ASSERT(pool->num_threads == HIERARCHICAL_88D_TOTAL_THREADS, "Correct thread count");
    
    // Don't destroy - it's global
}

// Test 2: Adapter creation
void test_adapter_creation() {
    TEST_START("Adapter Creation");
    
    HierarchicalThreadPool* pool = cllm_adapter_get_global_pool();
    
    CLLMLatticeHierarchy* sphere = cllm_adapter_create(0, 0, pool);
    TEST_ASSERT(sphere != NULL, "Adapter created");
    TEST_ASSERT(cllm_adapter_is_adapter(sphere) == 1, "Is recognized as adapter");
    
    HierarchicalThread* thread = cllm_adapter_get_thread(sphere);
    TEST_ASSERT(thread != NULL, "Has underlying thread");
    
    HierarchicalThreadPool* retrieved_pool = cllm_adapter_get_pool(sphere);
    TEST_ASSERT(retrieved_pool == pool, "Pool reference correct");
    
    cllm_adapter_destroy(sphere);
    TEST_ASSERT(true, "Adapter destroyed");
}

// Test 3: State management
void test_state_management() {
    TEST_START("State Management");
    
    HierarchicalThreadPool* pool = cllm_adapter_get_global_pool();
    CLLMLatticeHierarchy* sphere = cllm_adapter_create(1, 0, pool);
    
    int state = cllm_adapter_get_state(sphere);
    TEST_ASSERT(state == HIERARCHY_STATE_INITIALIZING, "Initial state correct");
    
    int result = cllm_adapter_set_state(sphere, HIERARCHY_STATE_READY);
    TEST_ASSERT(result == 0, "State set successfully");
    
    state = cllm_adapter_get_state(sphere);
    TEST_ASSERT(state == HIERARCHY_STATE_READY, "State updated correctly");
    
    cllm_adapter_destroy(sphere);
}

// Test 4: Gradient operations
void test_gradient_operations() {
    TEST_START("Gradient Operations");
    
    HierarchicalThreadPool* pool = cllm_adapter_get_global_pool();
    CLLMLatticeHierarchy* sphere = cllm_adapter_create(2, 0, pool);
    
    size_t buffer_size = 0;
    float* gradient_buffer = cllm_adapter_get_gradient_buffer(sphere, &buffer_size);
    TEST_ASSERT(gradient_buffer != NULL, "Gradient buffer allocated");
    TEST_ASSERT(buffer_size > 0, "Gradient buffer has size");
    
    // Create test gradient
    float test_gradient[100];
    for (int i = 0; i < 100; i++) {
        test_gradient[i] = (float)i;
    }
    
    int result = cllm_adapter_accumulate_gradient(sphere, test_gradient, 100);
    TEST_ASSERT(result == 0, "Gradient accumulated");
    
    // Verify accumulation
    TEST_ASSERT(gradient_buffer[0] == 0.0f, "First element correct");
    TEST_ASSERT(gradient_buffer[50] == 50.0f, "Middle element correct");
    TEST_ASSERT(gradient_buffer[99] == 99.0f, "Last element correct");
    
    cllm_adapter_destroy(sphere);
}

// Test 5: Conversion helpers
void test_conversion_helpers() {
    TEST_START("Conversion Helpers");
    
    HierarchicalThreadPool* pool = cllm_adapter_get_global_pool();
    
    // Test CLLM → HierarchicalThread
    CLLMLatticeHierarchy* sphere = cllm_adapter_create(3, 0, pool);
    HierarchicalThread* thread = cllm_to_hierarchical_thread(sphere);
    TEST_ASSERT(thread != NULL, "CLLM → HierarchicalThread conversion works");
    
    // Test HierarchicalThread → CLLM
    CLLMLatticeHierarchy* converted = hierarchical_thread_to_cllm(thread);
    TEST_ASSERT(converted != NULL, "HierarchicalThread → CLLM conversion works");
    TEST_ASSERT(cllm_adapter_is_adapter(converted) == 1, "Converted is adapter");
    
    cllm_adapter_destroy(sphere);
    cllm_adapter_destroy(converted);
}

// Test 6: Multiple adapters
void test_multiple_adapters() {
    TEST_START("Multiple Adapters");
    
    HierarchicalThreadPool* pool = cllm_adapter_get_global_pool();
    
    CLLMLatticeHierarchy* spheres[10];
    for (int i = 0; i < 10; i++) {
        spheres[i] = cllm_adapter_create(i + 10, 0, pool);
        TEST_ASSERT(spheres[i] != NULL, "Adapter created");
    }
    
    // Verify all are independent
    for (int i = 0; i < 10; i++) {
        HierarchicalThread* thread = cllm_adapter_get_thread(spheres[i]);
        TEST_ASSERT(thread != NULL, "Each has thread");
    }
    
    // Clean up
    for (int i = 0; i < 10; i++) {
        cllm_adapter_destroy(spheres[i]);
    }
    
    TEST_ASSERT(true, "All adapters destroyed");
}

// Test 7: Thread access
void test_thread_access() {
    TEST_START("Thread Access");
    
    HierarchicalThreadPool* pool = cllm_adapter_get_global_pool();
    CLLMLatticeHierarchy* sphere = cllm_adapter_create(20, 0, pool);
    
    HierarchicalThread* thread = cllm_adapter_get_thread(sphere);
    TEST_ASSERT(thread != NULL, "Thread retrieved");
    TEST_ASSERT(thread->thread_id == 20, "Thread ID correct");
    
    // Check 88D integration
    if (thread->value) {
        TEST_ASSERT(thread->value != NULL, "Thread has value abacus");
        TEST_ASSERT(thread->accumulator != NULL, "Thread has accumulator abacus");
        TEST_ASSERT(thread->temp != NULL, "Thread has temp abacus");
    }
    
    cllm_adapter_destroy(sphere);
}

// Main test runner
int main() {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║         CLLM Thread Adapter Test Suite                    ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    test_global_pool();
    test_adapter_creation();
    test_state_management();
    test_gradient_operations();
    test_conversion_helpers();
    test_multiple_adapters();
    test_thread_access();
    
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                      Test Results                          ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║  Passed: %3d                                               ║\n", tests_passed);
    printf("║  Failed: %3d                                               ║\n", tests_failed);
    printf("║  Total:  %3d                                               ║\n", tests_passed + tests_failed);
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    // Clean up global pool
    cllm_adapter_destroy_global_pool();
    
    return tests_failed > 0 ? 1 : 0;
}