/**
 * @file test_threadsafe_ops.c
 * @brief Test suite for thread-safe geometric operations
 */

#include "abacus88d.h"
#include "geometric_space_ops.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_THREADS 8
#define OPERATIONS_PER_THREAD 100

// Test colors
#define COLOR_GREEN "\033[32m"
#define COLOR_RED "\033[31m"
#define COLOR_RESET "\033[0m"

// Test counters
static int tests_passed = 0;
static int tests_failed = 0;

// Shared test data
typedef struct {
    Abacus88D* abacus;
    int thread_id;
    int operations_completed;
} ThreadData;

void test_assert(int condition, const char* test_name) {
    if (condition) {
        printf(COLOR_GREEN "✓" COLOR_RESET " %s\n", test_name);
        tests_passed++;
    } else {
        printf(COLOR_RED "✗" COLOR_RESET " %s\n", test_name);
        tests_failed++;
    }
}

// Thread function for concurrent boundary detection
void* thread_boundary_detection(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    
    for (int i = 0; i < OPERATIONS_PER_THREAD; i++) {
        // Create test value
        CrystallineAbacus* value = abacus_from_uint64(100 + data->thread_id * 100 + i, 60);
        
        // Detect boundary (thread-safe)
        BoundaryInfo boundary;
        uint8_t layer = i % ABACUS88D_NUM_LAYERS;
        uint8_t dimension = i % ABACUS88D_DIMS_PER_LAYER;
        
        MathError err = geometric_detect_boundary_threadsafe(
            data->abacus, layer, dimension, value, &boundary
        );
        
        if (err == MATH_SUCCESS) {
            data->operations_completed++;
        }
        
        abacus_free(value);
        
        // Small delay to increase contention
        usleep(10);
    }
    
    return NULL;
}

// Thread function for concurrent transformations
void* thread_transformations(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    
    for (int i = 0; i < OPERATIONS_PER_THREAD; i++) {
        // Create test value
        CrystallineAbacus* input = abacus_from_uint64(50 + data->thread_id * 10 + i, 60);
        CrystallineAbacus* output = abacus_new(60);
        
        // Calculate transformation
        TransformMatrix transform;
        uint8_t source_layer = i % ABACUS88D_NUM_LAYERS;
        uint8_t target_layer = (i + 1) % ABACUS88D_NUM_LAYERS;
        
        MathError err = geometric_calculate_transform(
            source_layer, target_layer, &transform
        );
        
        if (err == MATH_SUCCESS) {
            // Apply transformation (thread-safe)
            err = geometric_apply_transform_threadsafe(
                data->abacus, &transform, input, output
            );
            
            if (err == MATH_SUCCESS) {
                data->operations_completed++;
            }
        }
        
        abacus_free(input);
        abacus_free(output);
        
        usleep(10);
    }
    
    return NULL;
}

// Thread function for concurrent handoffs
void* thread_handoffs(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    
    for (int i = 0; i < OPERATIONS_PER_THREAD; i++) {
        // Create test value
        CrystallineAbacus* value = abacus_from_uint64(200 + data->thread_id * 50 + i, 60);
        
        // Prepare handoff context
        HandoffContext context;
        context.source_layer = i % ABACUS88D_NUM_LAYERS;
        context.source_dimension = i % ABACUS88D_DIMS_PER_LAYER;
        context.target_layer = (i + 1) % ABACUS88D_NUM_LAYERS;
        context.target_dimension = (i + 1) % ABACUS88D_DIMS_PER_LAYER;
        context.requires_scaling = (context.source_layer != context.target_layer);
        context.scale_factor = 1.0;
        context.at_shared_vertex = false;
        context.vertex_id = 0;
        
        // Execute handoff (thread-safe)
        MathError err = geometric_execute_handoff_threadsafe(
            data->abacus, data->abacus, &context, value
        );
        
        if (err == MATH_SUCCESS) {
            data->operations_completed++;
        }
        
        abacus_free(value);
        
        usleep(10);
    }
    
    return NULL;
}

void test_concurrent_boundary_detection(void) {
    printf("\nTest: Concurrent Boundary Detection\n");
    printf("----------------------------------------\n");
    
    // Create shared abacus
    Abacus88D* abacus = abacus88d_create(60);
    test_assert(abacus != NULL, "Created shared Abacus88D");
    
    // Create threads
    pthread_t threads[NUM_THREADS];
    ThreadData thread_data[NUM_THREADS];
    
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_data[i].abacus = abacus;
        thread_data[i].thread_id = i;
        thread_data[i].operations_completed = 0;
        
        int result = pthread_create(&threads[i], NULL, 
                                    thread_boundary_detection, &thread_data[i]);
        test_assert(result == 0, "Created thread");
    }
    
    // Wait for all threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Check results
    int total_operations = 0;
    for (int i = 0; i < NUM_THREADS; i++) {
        total_operations += thread_data[i].operations_completed;
    }
    
    printf("  Total operations: %d / %d\n", total_operations, 
           NUM_THREADS * OPERATIONS_PER_THREAD);
    test_assert(total_operations == NUM_THREADS * OPERATIONS_PER_THREAD,
                "All operations completed successfully");
    
    abacus88d_free(abacus);
}

void test_concurrent_transformations(void) {
    printf("\nTest: Concurrent Transformations\n");
    printf("----------------------------------------\n");
    
    Abacus88D* abacus = abacus88d_create(60);
    test_assert(abacus != NULL, "Created shared Abacus88D");
    
    pthread_t threads[NUM_THREADS];
    ThreadData thread_data[NUM_THREADS];
    
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_data[i].abacus = abacus;
        thread_data[i].thread_id = i;
        thread_data[i].operations_completed = 0;
        
        pthread_create(&threads[i], NULL, thread_transformations, &thread_data[i]);
    }
    
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    int total_operations = 0;
    for (int i = 0; i < NUM_THREADS; i++) {
        total_operations += thread_data[i].operations_completed;
    }
    
    printf("  Total operations: %d / %d\n", total_operations,
           NUM_THREADS * OPERATIONS_PER_THREAD);
    test_assert(total_operations == NUM_THREADS * OPERATIONS_PER_THREAD,
                "All transformations completed successfully");
    
    abacus88d_free(abacus);
}

void test_concurrent_handoffs(void) {
    printf("\nTest: Concurrent Handoffs\n");
    printf("----------------------------------------\n");
    
    Abacus88D* abacus = abacus88d_create(60);
    test_assert(abacus != NULL, "Created shared Abacus88D");
    
    pthread_t threads[NUM_THREADS];
    ThreadData thread_data[NUM_THREADS];
    
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_data[i].abacus = abacus;
        thread_data[i].thread_id = i;
        thread_data[i].operations_completed = 0;
        
        pthread_create(&threads[i], NULL, thread_handoffs, &thread_data[i]);
    }
    
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    int total_operations = 0;
    for (int i = 0; i < NUM_THREADS; i++) {
        total_operations += thread_data[i].operations_completed;
    }
    
    printf("  Total operations: %d / %d\n", total_operations,
           NUM_THREADS * OPERATIONS_PER_THREAD);
    test_assert(total_operations == NUM_THREADS * OPERATIONS_PER_THREAD,
                "All handoffs completed successfully");
    
    abacus88d_free(abacus);
}

void test_batch_operations(void) {
    printf("\nTest: Batch Thread-Safe Operations\n");
    printf("----------------------------------------\n");
    
    Abacus88D* abacus = abacus88d_create(60);
    test_assert(abacus != NULL, "Created Abacus88D");
    
    // Prepare batch data
    const int batch_size = 10;
    uint8_t layers[batch_size];
    uint8_t dimensions[batch_size];
    const CrystallineAbacus* values[batch_size];
    BoundaryInfo boundaries[batch_size];
    
    for (int i = 0; i < batch_size; i++) {
        layers[i] = i % ABACUS88D_NUM_LAYERS;
        dimensions[i] = i % ABACUS88D_DIMS_PER_LAYER;
        
        CrystallineAbacus* val = abacus_from_uint64(100 + i * 10, 60);
        values[i] = val;
    }
    
    // Execute batch boundary detection
    MathError err = geometric_detect_boundaries_batch_threadsafe(
        abacus, layers, dimensions, values, boundaries, batch_size
    );
    
    test_assert(err == MATH_SUCCESS, "Batch boundary detection succeeded");
    
    // Verify results
    int valid_boundaries = 0;
    for (int i = 0; i < batch_size; i++) {
        if (boundaries[i].current_layer == layers[i] &&
            boundaries[i].current_dimension == dimensions[i]) {
            valid_boundaries++;
        }
    }
    
    test_assert(valid_boundaries == batch_size, "All boundaries valid");
    
    // Cleanup
    for (int i = 0; i < batch_size; i++) {
        abacus_free((CrystallineAbacus*)values[i]);
    }
    
    abacus88d_free(abacus);
}

int main(void) {
    printf("========================================\n");
    printf("Thread-Safe Operations Test Suite\n");
    printf("========================================\n");
    
    test_concurrent_boundary_detection();
    test_concurrent_transformations();
    test_concurrent_handoffs();
    test_batch_operations();
    
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