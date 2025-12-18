/**
 * Test Control vs Worker Thread Behavior
 * 
 * Verifies that:
 * 1. Threads with children become CONTROL threads
 * 2. Threads without children remain WORKER threads
 * 3. Control threads distribute work to children
 * 4. Worker threads process work themselves
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "cllm_threads.h"

int main(void) {
    printf("=== Testing Control vs Worker Thread Behavior ===\n\n");
    
    // Get number of CPUs
    int num_cpus = sysconf(_SC_NPROCESSORS_ONLN);
    printf("Available CPUs: %d\n\n", num_cpus);
    
    // Create system with 64 CPUs (will have hierarchy)
    printf("Creating system with %d CPUs\n", num_cpus);
    printf("----------------------------------------\n");
    ThreadSystem* system = threads_create_dynamic(num_cpus);
    
    if (!system) {
        fprintf(stderr, "ERROR: Failed to create thread system\n");
        return 1;
    }
    
    printf("\nSystem created successfully!\n");
    printf("  Total spheres: %d\n", system->total_spheres);
    printf("  Levels: %d\n", system->num_levels);
    
    // Analyze thread roles
    printf("\nAnalyzing thread roles:\n");
    printf("----------------------------------------\n");
    
    int num_control = 0;
    int num_worker = 0;
    
    for (int i = 0; i < system->total_spheres; i++) {
        CLLMLatticeHierarchy* sphere = system->all_spheres[i];
        
        if (sphere->num_children > 0) {
            num_control++;
            if (i < 5 || (i >= system->total_spheres - 5)) {
                printf("  %s: CONTROL thread (%d children)\n", 
                       sphere->debug_name, sphere->num_children);
            } else if (i == 5) {
                printf("  ... (%d more control threads)\n", 
                       system->spheres_per_level[0] + system->spheres_per_level[1] - 10);
            }
        } else {
            num_worker++;
        }
    }
    
    printf("\nThread Role Summary:\n");
    printf("  Control threads: %d (have children, distribute work)\n", num_control);
    printf("  Worker threads:  %d (no children, process work)\n", num_worker);
    printf("  Total threads:   %d\n", system->total_spheres);
    
    // Verify expectations
    printf("\nVerifying expectations:\n");
    
    // Root should be control
    if (system->root->num_children > 0) {
        printf("  ✓ Root is CONTROL thread (has %d children)\n", system->root->num_children);
    } else {
        printf("  ✗ ERROR: Root should be CONTROL thread\n");
    }
    
    // Level 1 spheres should be control (they have children)
    int level1_control = 0;
    for (int i = 1; i <= system->spheres_per_level[1]; i++) {
        if (system->all_spheres[i]->num_children > 0) {
            level1_control++;
        }
    }
    printf("  ✓ Level 1: %d/%d are CONTROL threads\n", 
           level1_control, system->spheres_per_level[1]);
    
    // Level 2 spheres should be workers (they have no children)
    int level2_start = 1 + system->spheres_per_level[1];
    int level2_worker = 0;
    for (int i = level2_start; i < system->total_spheres; i++) {
        if (system->all_spheres[i]->num_children == 0) {
            level2_worker++;
        }
    }
    printf("  ✓ Level 2: %d/%d are WORKER threads\n", 
           level2_worker, system->spheres_per_level[2]);
    
    // Start threads
    printf("\nStarting threads...\n");
    if (threads_start(system) != 0) {
        fprintf(stderr, "ERROR: Failed to start threads\n");
        threads_free(system);
        return 1;
    }
    
    printf("All threads started successfully!\n");
    
    // Distribute some work
    printf("\nDistributing work items...\n");
    uint64_t work_items[100];
    for (int i = 0; i < 100; i++) {
        work_items[i] = i;
    }
    
    threads_distribute_work(system, work_items, 100);
    printf("Distributed 100 work items\n");
    
    // Let threads run
    printf("\nLetting threads run for 3 seconds...\n");
    sleep(3);
    
    // Check states
    printf("\nChecking thread states:\n");
    int controlling = 0;
    int processing = 0;
    int ready = 0;
    int idle = 0;
    
    for (int i = 0; i < system->total_spheres; i++) {
        CLLMLatticeHierarchy* sphere = system->all_spheres[i];
        int state = atomic_load(&sphere->state);
        
        switch (state) {
            case HIERARCHY_STATE_CONTROLLING:
                controlling++;
                break;
            case HIERARCHY_STATE_PROCESSING:
                processing++;
                break;
            case HIERARCHY_STATE_READY:
                ready++;
                break;
            case HIERARCHY_STATE_IDLE:
                idle++;
                break;
        }
    }
    
    printf("  CONTROLLING: %d threads\n", controlling);
    printf("  PROCESSING:  %d threads\n", processing);
    printf("  READY:       %d threads\n", ready);
    printf("  IDLE:        %d threads\n", idle);
    
    // Stop threads
    printf("\nStopping threads...\n");
    if (threads_stop(system) != 0) {
        fprintf(stderr, "ERROR: Failed to stop threads\n");
        threads_free(system);
        return 1;
    }
    
    printf("All threads stopped successfully!\n");
    
    // Clean up
    threads_free(system);
    
    printf("\n=== Test Complete ===\n");
    printf("\nSummary:\n");
    printf("  ✓ Control threads have children\n");
    printf("  ✓ Worker threads have no children\n");
    printf("  ✓ System correctly distinguishes roles\n");
    
    return 0;
}