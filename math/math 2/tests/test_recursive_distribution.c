/**
 * Test Recursive Work Distribution
 * 
 * Verifies that work flows recursively through the hierarchy:
 * Root → Level 1 → Level 2
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "cllm_threads.h"

int main(void) {
    printf("=== Testing Recursive Work Distribution ===\n\n");
    
    // Create system with 64 CPUs (3 levels)
    int num_cpus = 64;
    printf("Creating system with %d CPUs\n", num_cpus);
    ThreadSystem* system = threads_create_dynamic(num_cpus);
    
    if (!system) {
        fprintf(stderr, "ERROR: Failed to create thread system\n");
        return 1;
    }
    
    printf("\nSystem created:\n");
    printf("  Level 0: %d sphere (root control)\n", system->spheres_per_level[0]);
    printf("  Level 1: %d spheres (control)\n", system->spheres_per_level[1]);
    printf("  Level 2: %d spheres (workers)\n", system->spheres_per_level[2]);
    
    // Start threads
    printf("\nStarting threads...\n");
    if (threads_start(system) != 0) {
        fprintf(stderr, "ERROR: Failed to start threads\n");
        threads_free(system);
        return 1;
    }
    
    printf("All threads started\n");
    
    // Give threads time to initialize
    sleep(1);
    
    // Distribute work
    printf("\nDistributing 100 work items...\n");
    uint64_t work_items[100];
    for (int i = 0; i < 100; i++) {
        work_items[i] = i;
    }
    
    threads_distribute_work(system, work_items, 100);
    printf("Work distributed to Level 1\n");
    
    // Let work flow through hierarchy
    printf("\nWaiting 5 seconds for work to flow through hierarchy...\n");
    sleep(5);
    
    // Check work queue sizes at each level
    printf("\nChecking work distribution:\n");
    
    printf("\nLevel 0 (Root):\n");
    printf("  Work queue size: %zu\n", 
           atomic_load(&system->root->work_queue_size));
    
    printf("\nLevel 1 (Control threads):\n");
    int level1_total_work = 0;
    for (int i = 1; i <= system->spheres_per_level[1]; i++) {
        size_t queue_size = atomic_load(&system->all_spheres[i]->work_queue_size);
        level1_total_work += queue_size;
        if (i <= 3) {
            printf("  Sphere %d: %zu items\n", i, queue_size);
        }
    }
    printf("  Total work in Level 1: %d items\n", level1_total_work);
    
    printf("\nLevel 2 (Worker threads):\n");
    int level2_total_work = 0;
    int level2_start = 1 + system->spheres_per_level[1];
    for (int i = level2_start; i < system->total_spheres; i++) {
        size_t queue_size = atomic_load(&system->all_spheres[i]->work_queue_size);
        level2_total_work += queue_size;
    }
    printf("  Total work in Level 2: %d items\n", level2_total_work);
    
    // Check if work reached workers
    printf("\nVerification:\n");
    if (level2_total_work > 0) {
        printf("  ✓ Work successfully flowed to Level 2 workers\n");
        printf("  ✓ Recursive distribution is working\n");
    } else {
        printf("  ✗ No work reached Level 2 workers\n");
        printf("  ✗ Recursive distribution may not be working\n");
    }
    
    // Stop threads
    printf("\nStopping threads...\n");
    if (threads_stop(system) != 0) {
        fprintf(stderr, "ERROR: Failed to stop threads\n");
        threads_free(system);
        return 1;
    }
    
    printf("All threads stopped\n");
    
    // Clean up
    threads_free(system);
    
    printf("\n=== Test Complete ===\n");
    
    return 0;
}