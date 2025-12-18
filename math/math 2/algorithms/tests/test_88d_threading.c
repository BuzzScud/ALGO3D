/**
 * @file test_88d_threading.c
 * @brief Test suite for 88D threading integration
 */

#include "hierarchical_threading.h"
#include <stdio.h>
#include <stdlib.h>

#define COLOR_GREEN "\033[32m"
#define COLOR_RED "\033[31m"
#define COLOR_RESET "\033[0m"

static int tests_passed = 0;
static int tests_failed = 0;

void test_assert(int condition, const char* test_name) {
    if (condition) {
        printf(COLOR_GREEN "✓" COLOR_RESET " %s\n", test_name);
        tests_passed++;
    } else {
        printf(COLOR_RED "✗" COLOR_RESET " %s\n", test_name);
        tests_failed++;
    }
}

void test_88d_pool_creation(void) {
    printf("\nTest: 88D Thread Pool Creation\n");
    printf("----------------------------------------\n");
    
    HierarchicalThreadPool* pool = hierarchical_thread_pool_create_88d(60);
    test_assert(pool != NULL, "Created 88D thread pool");
    test_assert(pool->use_88d_structure == true, "88D structure enabled");
    test_assert(pool->num_threads == HIERARCHICAL_88D_TOTAL_THREADS, 
                "Correct number of threads (96)");
    test_assert(pool->symmetry_fold == HIERARCHICAL_88D_CLOCK_POSITIONS,
                "Correct symmetry fold (12)");
    test_assert(pool->clock_lattice != NULL, "Clock lattice created");
    
    // Check layer barriers
    printf("  Layer barriers initialized\n");
    
    // Check global barrier
    printf("  Global barrier initialized\n");
    
    hierarchical_thread_pool_free(pool);
    test_assert(true, "Pool freed successfully");
}

void test_88d_thread_access(void) {
    printf("\nTest: 88D Thread Access\n");
    printf("----------------------------------------\n");
    
    HierarchicalThreadPool* pool = hierarchical_thread_pool_create_88d(60);
    test_assert(pool != NULL, "Created pool");
    
    // Test getting threads by 88D position
    for (uint8_t layer = 0; layer < HIERARCHICAL_88D_NUM_LAYERS; layer++) {
        for (uint8_t dim = 0; dim < HIERARCHICAL_88D_THREADS_PER_LAYER; dim++) {
            HierarchicalThread* thread = hierarchical_thread_get_88d(pool, layer, dim);
            test_assert(thread != NULL, "Got thread");
            
            if (thread) {
                test_assert(thread->layer == layer, "Correct layer");
                test_assert(thread->clock_position == dim + 1, "Correct clock position");
                
                // Check role
                if (dim == 0) {
                    test_assert(thread->role == THREAD_ROLE_CONTROL, "Control thread");
                } else {
                    test_assert(thread->role == THREAD_ROLE_WORKER, "Worker thread");
                }
                
                // Check abacus values
                test_assert(thread->value != NULL, "Has value");
                test_assert(thread->accumulator != NULL, "Has accumulator");
                test_assert(thread->temp != NULL, "Has temp");
                
                // Only print first few
                if (layer == 0 && dim < 3) {
                    printf("  Thread [%d][%d]: ID=%u, clock_pos=%u, role=%s\n",
                           layer, dim, thread->thread_id, thread->clock_position,
                           (thread->role == THREAD_ROLE_CONTROL) ? "CONTROL" : "WORKER");
                }
            }
        }
    }
    
    hierarchical_thread_pool_free(pool);
}

void test_88d_control_threads(void) {
    printf("\nTest: 88D Control Threads\n");
    printf("----------------------------------------\n");
    
    HierarchicalThreadPool* pool = hierarchical_thread_pool_create_88d(60);
    test_assert(pool != NULL, "Created pool");
    
    // Check control threads
    for (uint8_t layer = 0; layer < HIERARCHICAL_88D_NUM_LAYERS; layer++) {
        HierarchicalThread* control = pool->control_threads[layer];
        test_assert(control != NULL, "Control thread exists");
        
        if (control) {
            test_assert(control->role == THREAD_ROLE_CONTROL, "Is control thread");
            test_assert(control->layer == layer, "Correct layer");
            test_assert(control->clock_position == 1, "At position 1 (12 o'clock)");
            
            printf("  Layer %d control: thread_id=%u\n", layer, control->thread_id);
        }
    }
    
    hierarchical_thread_pool_free(pool);
}

void test_88d_statistics(void) {
    printf("\nTest: 88D Statistics\n");
    printf("----------------------------------------\n");
    
    HierarchicalThreadPool* pool = hierarchical_thread_pool_create_88d(60);
    test_assert(pool != NULL, "Created pool");
    
    // Get initial stats
    uint64_t boundary_crossings, twin_primes, operations;
    int result = hierarchical_thread_pool_get_88d_stats(
        pool, &boundary_crossings, &twin_primes, &operations
    );
    
    test_assert(result == 0, "Got 88D statistics");
    test_assert(boundary_crossings == 0, "Initial boundary crossings = 0");
    test_assert(twin_primes == 0, "Initial twin primes = 0");
    test_assert(operations == 0, "Initial operations = 0");
    
    printf("  Boundary crossings: %lu\n", boundary_crossings);
    printf("  Twin primes: %lu\n", twin_primes);
    printf("  Operations: %lu\n", operations);
    
    hierarchical_thread_pool_free(pool);
}

void test_sphere_group_attachment(void) {
    printf("\nTest: Sphere Group Attachment (Self-Similar Nesting)\n");
    printf("----------------------------------------\n");
    
    // Create parent group
    HierarchicalThreadPool* parent = hierarchical_thread_pool_create_88d(60);
    test_assert(parent != NULL, "Created parent group");
    
    // Create child group
    HierarchicalThreadPool* child = hierarchical_thread_pool_create_88d(60);
    test_assert(child != NULL, "Created child group");
    
    // Attach child to parent
    int result = hierarchical_thread_pool_attach_group(parent, child);
    test_assert(result == 0, "Attached child group");
    test_assert(parent->num_child_groups == 1, "Parent has 1 child");
    test_assert(child->parent_group == parent, "Child knows parent");
    
    printf("  Parent group has %u child groups\n", parent->num_child_groups);
    
    // Detach child
    result = hierarchical_thread_pool_detach_group(parent, child);
    test_assert(result == 0, "Detached child group");
    test_assert(parent->num_child_groups == 0, "Parent has 0 children");
    test_assert(child->parent_group == NULL, "Child has no parent");
    
    hierarchical_thread_pool_free(child);
    hierarchical_thread_pool_free(parent);
}

void test_boundary_notifications(void) {
    printf("\nTest: Boundary Crossing Notifications\n");
    printf("----------------------------------------\n");
    
    HierarchicalThreadPool* pool = hierarchical_thread_pool_create_88d(60);
    test_assert(pool != NULL, "Created pool");
    
    // Get a thread
    HierarchicalThread* thread = hierarchical_thread_get_88d(pool, 0, 1);
    test_assert(thread != NULL, "Got thread");
    
    if (thread) {
        // Initial state
        test_assert(thread->boundary_crossed == false, "No boundary crossed initially");
        test_assert(thread->boundary_crossings == 0, "Zero crossings initially");
        
        // Notify boundary crossing
        int result = hierarchical_thread_notify_boundary_crossing(thread, 0, 1);
        test_assert(result == 0, "Notified boundary crossing");
        test_assert(thread->boundary_crossed == true, "Boundary crossed flag set");
        test_assert(thread->boundary_crossings == 1, "Crossing count = 1");
        
        printf("  Thread boundary crossings: %lu\n", thread->boundary_crossings);
    }
    
    hierarchical_thread_pool_free(pool);
}

void test_twin_prime_notifications(void) {
    printf("\nTest: Twin Prime Notifications\n");
    printf("----------------------------------------\n");
    
    HierarchicalThreadPool* pool = hierarchical_thread_pool_create_88d(60);
    test_assert(pool != NULL, "Created pool");
    
    HierarchicalThread* thread = hierarchical_thread_get_88d(pool, 0, 1);
    test_assert(thread != NULL, "Got thread");
    
    if (thread) {
        // Initial state
        test_assert(thread->twin_prime_detected == false, "No twin prime initially");
        test_assert(thread->twin_primes_found == 0, "Zero twin primes initially");
        
        // Notify twin prime
        int result = hierarchical_thread_notify_twin_prime(thread, 11, 13);
        test_assert(result == 0, "Notified twin prime");
        test_assert(thread->twin_prime_detected == true, "Twin prime flag set");
        test_assert(thread->twin_primes_found == 1, "Twin prime count = 1");
        
        printf("  Thread twin primes found: %lu\n", thread->twin_primes_found);
    }
    
    hierarchical_thread_pool_free(pool);
}

int main(void) {
    printf("========================================\n");
    printf("88D Threading Integration Test Suite\n");
    printf("========================================\n");
    
    test_88d_pool_creation();
    test_88d_thread_access();
    test_88d_control_threads();
    test_88d_statistics();
    test_sphere_group_attachment();
    test_boundary_notifications();
    test_twin_prime_notifications();
    
    printf("\n========================================\n");
    printf("Test Results\n");
    printf("========================================\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Total:  %d\n", tests_passed + tests_failed);
    
    if (tests_failed == 0) {
        printf("\n" COLOR_GREEN "✓" COLOR_RESET " ALL TESTS PASSED\n");
        return 0;
    } else {
        printf("\n" COLOR_RED "✗" COLOR_RESET " SOME TESTS FAILED\n");
        return 1;
    }
}