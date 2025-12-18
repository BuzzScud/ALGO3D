/**
 * Simple Gradient Accumulation Test
 * 
 * Tests only the gradient accumulation mechanism without full training setup
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../algorithms/include/shared_memory_enhanced.h"
#include "../../algorithms/include/shared_memory.h"

#define TEST_GRADIENT_SIZE 1000
#define TEST_NUM_THREADS 12

int main(void) {
    printf("========================================\n");
    printf("  Simple Gradient Accumulation Test\n");
    printf("========================================\n");
    
    // Create shared memory for gradients
    printf("\nCreating shared memory...\n");
    SharedMemoryEnhanced* gradient_memory = shared_memory_enhanced_create(
        TEST_GRADIENT_SIZE * TEST_NUM_THREADS * sizeof(double),  // total size for all threads
        SHARED_LOCKED_WRITE,                   // access mode
        0                                      // region ID
    );
    
    if (!gradient_memory) {
        fprintf(stderr, "Failed to create shared memory\n");
        return -1;
    }
    printf("✓ Created shared memory with %d segments\n", TEST_NUM_THREADS);
    
    // Write known values to each thread's segment
    printf("\nWriting test values...\n");
    for (int t = 0; t < TEST_NUM_THREADS; t++) {
        double* segment = (double*)gradient_memory->base.data + (t * TEST_GRADIENT_SIZE);
        
        for (int i = 0; i < TEST_GRADIENT_SIZE; i++) {
            segment[i] = (double)(t + 1);  // Thread ID + 1
        }
    }
    printf("✓ Wrote values to all segments\n");
    
    // Accumulate gradients
    printf("\nAccumulating gradients...\n");
    double* accumulated = (double*)calloc(TEST_GRADIENT_SIZE, sizeof(double));
    if (!accumulated) {
        fprintf(stderr, "Failed to allocate accumulation buffer\n");
        shared_memory_enhanced_free(gradient_memory);
        return -1;
    }
    
    // Sum all thread contributions
    for (int t = 0; t < TEST_NUM_THREADS; t++) {
        double* segment = (double*)gradient_memory->base.data + (t * TEST_GRADIENT_SIZE);
        
        for (int i = 0; i < TEST_GRADIENT_SIZE; i++) {
            accumulated[i] += segment[i];
        }
    }
    
    // Average
    for (int i = 0; i < TEST_GRADIENT_SIZE; i++) {
        accumulated[i] /= TEST_NUM_THREADS;
    }
    printf("✓ Accumulated and averaged\n");
    
    // Verify
    printf("\nVerifying results...\n");
    double expected_sum = 0.0;
    for (int t = 0; t < TEST_NUM_THREADS; t++) {
        expected_sum += (double)(t + 1);
    }
    double expected_avg = expected_sum / TEST_NUM_THREADS;
    printf("  Expected average: %.2f\n", expected_avg);
    
    int errors = 0;
    for (int i = 0; i < TEST_GRADIENT_SIZE; i++) {
        double actual = accumulated[i];
        double diff = actual - expected_avg;
        if (diff < 0) diff = -diff;  // abs
        
        if (diff > 1e-6) {
            if (errors < 5) {
                fprintf(stderr, "  ERROR at index %d: expected %.2f, got %.2f\n",
                       i, expected_avg, actual);
            }
            errors++;
        }
    }
    
    if (errors > 0) {
        fprintf(stderr, "✗ FAILED: %d values incorrect\n", errors);
        free(accumulated);
        shared_memory_enhanced_free(gradient_memory);
        return -1;
    }
    
    printf("✓ All values correct!\n");
    
    // Cleanup
    free(accumulated);
    shared_memory_enhanced_free(gradient_memory);
    
    printf("\n========================================\n");
    printf("  ✓ TEST PASSED\n");
    printf("========================================\n");
    
    return 0;
}