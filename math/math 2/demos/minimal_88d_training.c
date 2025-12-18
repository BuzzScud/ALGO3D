/**
 * @file minimal_88d_training.c
 * @brief Minimal prototype to validate 88D training design
 * 
 * This demonstrates the core concepts:
 * 1. Create 88D thread pool
 * 2. Store values in thread CrystallineAbacus
 * 3. Accumulate across threads (like gradient accumulation)
 * 4. Update values (like parameter updates)
 * 5. Verify correctness
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "hierarchical_threading.h"
#include "math/abacus.h"

// Helper function to get uint64 from abacus
static uint64_t get_uint64(const CrystallineAbacus* abacus) {
    uint64_t value = 0;
    abacus_to_uint64(abacus, &value);
    return value;
}

// Helper function to set uint64 in abacus
static void set_uint64(CrystallineAbacus* abacus, uint64_t value) {
    CrystallineAbacus* temp = abacus_from_uint64(value, 60);
    if (temp) {
        // Copy temp to abacus
        abacus_init_zero(abacus);
        abacus_add(abacus, abacus, temp);
        abacus_free(temp);
    }
}

int main(void) {
    printf("=== Minimal 88D Training Prototype ===\n\n");
    
    // Step 1: Create 88D thread pool
    printf("Step 1: Creating 88D thread pool...\n");
    HierarchicalThreadPool* pool = hierarchical_thread_pool_create_88d(60);
    if (!pool) {
        fprintf(stderr, "ERROR: Failed to create thread pool\n");
        return 1;
    }
    printf("  ✓ Created pool with %u threads\n", pool->num_threads);
    printf("  ✓ 88D structure: %s\n", pool->use_88d_structure ? "enabled" : "disabled");
    printf("\n");
    
    // Step 2: Store test values in layer 0 threads (simulating parameters)
    printf("Step 2: Storing test values in layer 0 threads...\n");
    for (int dim = 1; dim <= 11; dim++) {
        HierarchicalThread* thread = hierarchical_thread_get_88d(pool, 0, dim);
        if (!thread) {
            fprintf(stderr, "ERROR: Failed to get thread [0][%d]\n", dim);
            hierarchical_thread_pool_free(pool);
            return 1;
        }
        
        // Set parameter value: dim * 10
        set_uint64(thread->value, dim * 10);
        
        // Set gradient value: dim
        set_uint64(thread->accumulator, dim);
        
        printf("  Thread [0][%d]: value=%lu, gradient=%lu\n", 
               dim, 
               get_uint64(thread->value),
               get_uint64(thread->accumulator));
    }
    printf("\n");
    
    // Step 3: Accumulate gradients in control thread (simulating gradient accumulation)
    printf("Step 3: Accumulating gradients in control thread...\n");
    HierarchicalThread* control = pool->control_threads[0];
    if (!control) {
        fprintf(stderr, "ERROR: Failed to get control thread\n");
        hierarchical_thread_pool_free(pool);
        return 1;
    }
    
    // Zero control's accumulator
    abacus_init_zero(control->accumulator);
    
    // Accumulate from all workers
    for (int dim = 1; dim <= 11; dim++) {
        HierarchicalThread* worker = hierarchical_thread_get_88d(pool, 0, dim);
        
        // Add worker's gradient to control's accumulator
        pthread_mutex_lock(&control->gradient_lock);
        abacus_add(control->accumulator, control->accumulator, worker->accumulator);
        pthread_mutex_unlock(&control->gradient_lock);
    }
    
    uint64_t total_gradient = get_uint64(control->accumulator);
    printf("  Total accumulated gradient: %lu\n", total_gradient);
    printf("  Expected: 1+2+3+...+11 = 66\n");
    printf("  Result: %s\n", (total_gradient == 66) ? "✓ CORRECT" : "✗ INCORRECT");
    printf("\n");
    
    // Step 4: Update values (simulating parameter update with learning_rate=1.0)
    printf("Step 4: Updating parameters (learning_rate=1.0)...\n");
    for (int dim = 1; dim <= 11; dim++) {
        HierarchicalThread* thread = hierarchical_thread_get_88d(pool, 0, dim);
        
        // parameter -= learning_rate * gradient
        // With learning_rate=1.0: parameter -= gradient
        abacus_sub(thread->value, thread->value, thread->accumulator);
        
        uint64_t new_value = get_uint64(thread->value);
        uint64_t expected = dim * 10 - dim;
        printf("  Thread [0][%d]: new_value=%lu, expected=%lu %s\n",
               dim, new_value, expected,
               (new_value == expected) ? "✓" : "✗");
    }
    printf("\n");
    
    // Step 5: Verify all updates are correct
    printf("Step 5: Final verification...\n");
    int all_correct = 1;
    for (int dim = 1; dim <= 11; dim++) {
        HierarchicalThread* thread = hierarchical_thread_get_88d(pool, 0, dim);
        uint64_t value = get_uint64(thread->value);
        uint64_t expected = dim * 10 - dim;
        if (value != expected) {
            all_correct = 0;
            printf("  ✗ Thread [0][%d]: %lu != %lu\n", dim, value, expected);
        }
    }
    
    if (all_correct) {
        printf("  ✓ All values correct!\n");
    }
    printf("\n");
    
    // Step 6: Test cross-layer operations
    printf("Step 6: Testing cross-layer operations...\n");
    
    // Store value in layer 0, dimension 1
    HierarchicalThread* thread_0_1 = hierarchical_thread_get_88d(pool, 0, 1);
    set_uint64(thread_0_1->value, 100);
    
    // Store value in layer 1, dimension 1
    HierarchicalThread* thread_1_1 = hierarchical_thread_get_88d(pool, 1, 1);
    set_uint64(thread_1_1->value, 200);
    
    // Add them (simulating cross-layer operation)
    CrystallineAbacus* result = abacus_new(60);
    abacus_add(result, thread_0_1->value, thread_1_1->value);
    
    uint64_t cross_layer_result = get_uint64(result);
    printf("  Layer 0 value: %lu\n", get_uint64(thread_0_1->value));
    printf("  Layer 1 value: %lu\n", get_uint64(thread_1_1->value));
    printf("  Sum: %lu (expected 300)\n", cross_layer_result);
    printf("  Result: %s\n", (cross_layer_result == 300) ? "✓ CORRECT" : "✗ INCORRECT");
    
    abacus_free(result);
    printf("\n");
    
    // Cleanup
    printf("Cleanup: Freeing thread pool...\n");
    hierarchical_thread_pool_free(pool);
    printf("  ✓ Pool freed\n\n");
    
    // Final result
    if (all_correct && total_gradient == 66 && cross_layer_result == 300) {
        printf("=== ✓ ALL TESTS PASSED ===\n");
        printf("\nThe 88D design is validated!\n");
        printf("Ready to implement full training system.\n");
        return 0;
    } else {
        printf("=== ✗ SOME TESTS FAILED ===\n");
        return 1;
    }
}