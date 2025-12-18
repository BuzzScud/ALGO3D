/*
 * Debug test for 88D pool creation
 */

#include <stdio.h>
#include <stdlib.h>
#include "hierarchical_threading.h"
#include "hierarchical_memory.h"
#include "work_distribution.h"
#include "state_management.h"

int main() {
    printf("\n88D Pool Creation Debug\n");
    printf("=======================\n\n");
    
    uint32_t num_threads = 96;
    uint32_t symmetry_fold = 12;
    uint32_t num_dimensions = 12;
    
    printf("Parameters:\n");
    printf("  num_threads = %u\n", num_threads);
    printf("  symmetry_fold = %u\n", symmetry_fold);
    printf("  num_dimensions = %u\n", num_dimensions);
    printf("\n");
    
    // Test 1: Create hierarchical memory
    printf("Test 1: Creating hierarchical memory...\n");
    HierarchicalMemoryBlock* memory = hierarchical_memory_create(
        num_threads * 1024 * 1024,  // 96 MB
        symmetry_fold,              // 12 segments
        num_dimensions,             // 12 dimensions
        0,                          // owner_id
        0                           // hierarchy_level
    );
    if (memory) {
        printf("  ✓ Hierarchical memory created\n");
        hierarchical_memory_destroy(memory);
    } else {
        printf("  ✗ Failed to create hierarchical memory\n");
        return 1;
    }
    
    // Test 2: Create work distributor
    printf("\nTest 2: Creating work distributor...\n");
    WorkDistributor* distributor = work_distributor_create(num_threads, 1000);
    if (distributor) {
        printf("  ✓ Work distributor created\n");
        work_distributor_destroy(distributor);
    } else {
        printf("  ✗ Failed to create work distributor\n");
        return 1;
    }
    
    // Test 3: Create state manager
    printf("\nTest 3: Creating state manager...\n");
    StateManager* state_mgr = state_manager_create(num_threads, 100);
    if (state_mgr) {
        printf("  ✓ State manager created\n");
        state_manager_destroy(state_mgr);
    } else {
        printf("  ✗ Failed to create state manager\n");
        return 1;
    }
    
    // Test 4: Create full pool
    printf("\nTest 4: Creating full thread pool...\n");
    HierarchicalThreadPool* pool = hierarchical_thread_pool_create(
        num_threads,
        symmetry_fold,
        num_dimensions,
        false
    );
    if (pool) {
        printf("  ✓ Thread pool created\n");
        hierarchical_thread_pool_free(pool);
    } else {
        printf("  ✗ Failed to create thread pool\n");
        return 1;
    }
    
    printf("\n✓ All tests passed!\n\n");
    return 0;
}