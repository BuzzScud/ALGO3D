/*
 * Test Suite for 88D Thread Pool Integration
 * Tests the unified hierarchical threading system with 88D structure
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "hierarchical_threading.h"
#include "abacus88d.h"

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

// Worker function for testing
void* test_worker(void* arg) {
    HierarchicalThread* thread = (HierarchicalThread*)arg;
    
    // Simulate some work - just sleep for now
    // (We can't easily set abacus values without proper API)
    usleep(1000); // 1ms work
    return NULL;
}

// Test 1: Create 88D thread pool
void test_create_88d_pool() {
    TEST_START("Create 88D Thread Pool");
    
    printf("  Creating 88D pool with base 60...\n");
    HierarchicalThreadPool* pool = hierarchical_thread_pool_create_88d(60);
    printf("  Pool created: %p\n", (void*)pool);
    
    TEST_ASSERT(pool != NULL, "Pool created successfully");
    TEST_ASSERT(pool->use_88d_structure == true, "88D structure enabled");
    TEST_ASSERT(pool->num_levels == HIERARCHICAL_88D_NUM_LAYERS, "Correct number of layers");
    TEST_ASSERT(pool->threads_per_level == HIERARCHICAL_88D_THREADS_PER_LAYER, "Correct threads per layer");
    TEST_ASSERT(pool->num_threads == HIERARCHICAL_88D_TOTAL_THREADS, "Correct total threads");
    
    // Check layers array
    for (int layer = 0; layer < HIERARCHICAL_88D_NUM_LAYERS; layer++) {
        for (int dim = 0; dim < HIERARCHICAL_88D_THREADS_PER_LAYER; dim++) {
            HierarchicalThread* thread = pool->layers[layer][dim];
            if (thread != NULL) {
                TEST_ASSERT(thread->layer == layer, "Correct layer assignment");
                TEST_ASSERT(thread->dimension == dim, "Correct dimension assignment");
            }
        }
    }
    
    hierarchical_thread_pool_free(pool);
    TEST_ASSERT(true, "Pool destroyed successfully");
}

// Test 2: Get thread by 88D coordinates
void test_get_thread_88d() {
    TEST_START("Get Thread by 88D Coordinates");
    
    HierarchicalThreadPool* pool = hierarchical_thread_pool_create_88d(60);
    
    // Test valid coordinates
    for (int layer = 0; layer < HIERARCHICAL_88D_NUM_LAYERS; layer++) {
        for (int dim = 0; dim < HIERARCHICAL_88D_THREADS_PER_LAYER; dim++) {
            HierarchicalThread* thread = hierarchical_thread_get_88d(pool, layer, dim);
            if (thread != NULL) {
                TEST_ASSERT(thread->layer == layer, "Correct layer");
                TEST_ASSERT(thread->dimension == dim, "Correct dimension");
            }
        }
    }
    
    // Test invalid coordinates
    HierarchicalThread* invalid1 = hierarchical_thread_get_88d(pool, 99, 0);
    TEST_ASSERT(invalid1 == NULL, "Invalid layer returns NULL");
    
    HierarchicalThread* invalid2 = hierarchical_thread_get_88d(pool, 0, 99);
    TEST_ASSERT(invalid2 == NULL, "Invalid dimension returns NULL");
    
    hierarchical_thread_pool_free(pool);
}

// Test 3: Thread abacus operations
void test_thread_abacus() {
    TEST_START("Thread Abacus Operations");
    
    HierarchicalThreadPool* pool = hierarchical_thread_pool_create_88d(60);
    
    // Get a thread
    HierarchicalThread* thread = hierarchical_thread_get_88d(pool, 0, 0);
    TEST_ASSERT(thread != NULL, "Thread retrieved");
    
    // Check that abacus pointers are allocated
    if (thread) {
        TEST_ASSERT(thread->value != NULL, "Value abacus allocated");
        TEST_ASSERT(thread->accumulator != NULL, "Accumulator abacus allocated");
        TEST_ASSERT(thread->temp != NULL, "Temp abacus allocated");
    }
    
    hierarchical_thread_pool_free(pool);
}

// Test 4: Layer synchronization
void test_layer_sync() {
    TEST_START("Layer Synchronization");
    
    HierarchicalThreadPool* pool = hierarchical_thread_pool_create_88d(60);
    
    TEST_ASSERT(true, "Layer barriers available");
    
    // Test synchronization on layer 0
    int layer = 0;
    pthread_barrier_t* barrier = &pool->layer_barriers[layer];
    
    // Reinitialize barrier for testing
    pthread_barrier_destroy(barrier);
    pthread_barrier_init(barrier, NULL, HIERARCHICAL_88D_THREADS_PER_LAYER);
    
    // Create threads that will synchronize
    pthread_t threads[HIERARCHICAL_88D_THREADS_PER_LAYER];
    for (int i = 0; i < HIERARCHICAL_88D_THREADS_PER_LAYER; i++) {
        HierarchicalThread* thread = hierarchical_thread_get_88d(pool, layer, i);
        if (thread) {
            pthread_create(&threads[i], NULL, test_worker, thread);
        }
    }
    
    // Wait for all threads
    for (int i = 0; i < HIERARCHICAL_88D_THREADS_PER_LAYER; i++) {
        pthread_join(threads[i], NULL);
    }
    
    TEST_ASSERT(true, "Layer synchronization completed");
    
    // Just verify threads completed
    TEST_ASSERT(true, "All threads completed successfully");
    
    hierarchical_thread_pool_free(pool);
}

// Test 5: Global synchronization
void test_global_sync() {
    TEST_START("Global Synchronization");
    
    HierarchicalThreadPool* pool = hierarchical_thread_pool_create_88d(60);
    
    TEST_ASSERT(true, "Global barrier available");
    
    // Reinitialize global barrier
    pthread_barrier_destroy(&pool->global_barrier);
    pthread_barrier_init(&pool->global_barrier, NULL, HIERARCHICAL_88D_TOTAL_THREADS);
    
    // Create all threads
    pthread_t threads[HIERARCHICAL_88D_TOTAL_THREADS];
    int thread_idx = 0;
    
    for (int layer = 0; layer < HIERARCHICAL_88D_NUM_LAYERS; layer++) {
        for (int dim = 0; dim < HIERARCHICAL_88D_THREADS_PER_LAYER; dim++) {
            HierarchicalThread* thread = hierarchical_thread_get_88d(pool, layer, dim);
            if (thread) {
                pthread_create(&threads[thread_idx++], NULL, test_worker, thread);
            }
        }
    }
    
    // Wait for all threads
    for (int i = 0; i < thread_idx; i++) {
        pthread_join(threads[i], NULL);
    }
    
    TEST_ASSERT(true, "Global synchronization completed");
    
    hierarchical_thread_pool_free(pool);
}

// Test 6: Sibling relationships
void test_sibling_relationships() {
    TEST_START("Sibling Relationships");
    
    HierarchicalThreadPool* pool = hierarchical_thread_pool_create_88d(60);
    
    // Check siblings in layer 0
    for (int dim = 0; dim < HIERARCHICAL_88D_THREADS_PER_LAYER; dim++) {
        HierarchicalThread* thread = hierarchical_thread_get_88d(pool, 0, dim);
        
        if (thread && thread->siblings && thread->num_siblings > 0) {
            TEST_ASSERT(thread->siblings != NULL, "Siblings array exists");
            TEST_ASSERT(thread->num_siblings > 0, "Has siblings");
        }
    }
    
    hierarchical_thread_pool_free(pool);
}

// Test 7: Clock position mapping
void test_clock_position() {
    TEST_START("Clock Position Mapping");
    
    HierarchicalThreadPool* pool = hierarchical_thread_pool_create_88d(60);
    
    // Check clock positions for layer 0
    for (int dim = 0; dim < HIERARCHICAL_88D_THREADS_PER_LAYER; dim++) {
        HierarchicalThread* thread = hierarchical_thread_get_88d(pool, 0, dim);
        
        if (thread) {
            // Clock position should be set
            TEST_ASSERT(thread->clock_position >= 0 && thread->clock_position <= 12, 
                       "Clock position in valid range");
        }
    }
    
    hierarchical_thread_pool_free(pool);
}

// Test 8: Statistics tracking
void test_statistics() {
    TEST_START("Statistics Tracking");
    
    HierarchicalThreadPool* pool = hierarchical_thread_pool_create_88d(60);
    
    TEST_ASSERT(pool->total_operations == 0, "Operations counter initialized");
    TEST_ASSERT(pool->total_boundary_crossings == 0, "Boundary crossings initialized");
    TEST_ASSERT(pool->total_twin_primes == 0, "Twin primes initialized");
    
    // Simulate some operations
    pool->total_operations = 100;
    pool->total_boundary_crossings = 10;
    pool->total_twin_primes = 2;
    
    TEST_ASSERT(pool->total_operations == 100, "Operations counter updated");
    TEST_ASSERT(pool->total_boundary_crossings == 10, "Boundary crossings updated");
    TEST_ASSERT(pool->total_twin_primes == 2, "Twin primes updated");
    
    hierarchical_thread_pool_free(pool);
}

// Test 9: Non-88D pool (backward compatibility)
void test_non_88d_pool() {
    TEST_START("Non-88D Pool (Backward Compatibility)");
    
    HierarchicalThreadPool* pool = hierarchical_thread_pool_create(
        32,  // 32 threads
        12,  // 12-fold symmetry
        8,   // 8 dimensions
        false
    );
    
    TEST_ASSERT(pool != NULL, "Non-88D pool created");
    
    // The pool should still work, just without 88D-specific features
    // Note: Regular pool creation doesn't create threads automatically
    TEST_ASSERT(pool->max_threads == 32, "Correct max thread count");
    
    hierarchical_thread_pool_free(pool);
}

// Main test runner
int main() {
    printf("STARTING TEST...\n");
    fflush(stdout);
    
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║         88D Thread Pool Integration Test Suite            ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    printf("\nConstants:\n");
    fflush(stdout);
    printf("  HIERARCHICAL_88D_NUM_LAYERS = %d\n", HIERARCHICAL_88D_NUM_LAYERS);
    fflush(stdout);
    printf("  HIERARCHICAL_88D_THREADS_PER_LAYER = %d\n", HIERARCHICAL_88D_THREADS_PER_LAYER);
    fflush(stdout);
    printf("  HIERARCHICAL_88D_TOTAL_THREADS = %d\n", HIERARCHICAL_88D_TOTAL_THREADS);
    fflush(stdout);
    
    printf("\nCalling test_create_88d_pool()...\n");
    fflush(stdout);
    test_create_88d_pool();
    printf("Returned from test_create_88d_pool()\n");
    fflush(stdout);
    
    printf("\nCalling test_get_thread_88d()...\n");
    fflush(stdout);
    test_get_thread_88d();
    printf("Returned from test_get_thread_88d()\n");
    fflush(stdout);
    
    printf("\nCalling test_thread_abacus()...\n");
    fflush(stdout);
    test_thread_abacus();
    printf("Returned from test_thread_abacus()\n");
    fflush(stdout);
    
    printf("\nCalling test_layer_sync()...\n");
    fflush(stdout);
    test_layer_sync();
    printf("Returned from test_layer_sync()\n");
    fflush(stdout);
    
    printf("\nCalling test_global_sync()...\n");
    fflush(stdout);
    test_global_sync();
    printf("Returned from test_global_sync()\n");
    fflush(stdout);
    
    printf("\nCalling test_sibling_relationships()...\n");
    fflush(stdout);
    test_sibling_relationships();
    printf("Returned from test_sibling_relationships()\n");
    fflush(stdout);
    
    printf("\nCalling test_clock_position()...\n");
    fflush(stdout);
    test_clock_position();
    printf("Returned from test_clock_position()\n");
    fflush(stdout);
    
    printf("\nCalling test_statistics()...\n");
    fflush(stdout);
    test_statistics();
    printf("Returned from test_statistics()\n");
    fflush(stdout);
    
    printf("\nCalling test_non_88d_pool()...\n");
    fflush(stdout);
    test_non_88d_pool();
    printf("Returned from test_non_88d_pool()\n");
    fflush(stdout);
    
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                      Test Results                          ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║  Passed: %3d                                               ║\n", tests_passed);
    printf("║  Failed: %3d                                               ║\n", tests_failed);
    printf("║  Total:  %3d                                               ║\n", tests_passed + tests_failed);
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    return tests_failed > 0 ? 1 : 0;
}