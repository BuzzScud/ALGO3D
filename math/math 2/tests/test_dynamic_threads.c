/**
 * Test Dynamic Threading System
 * 
 * Tests the dynamic kissing spheres threading with 64 CPUs
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "cllm_threads.h"

int main(void) {
    printf("=== Testing Dynamic Kissing Spheres Threading ===\n\n");
    
    // Get number of CPUs
    int num_cpus = sysconf(_SC_NPROCESSORS_ONLN);
    printf("Available CPUs: %d\n\n", num_cpus);
    
    // Test 1: Create system with all CPUs
    printf("Test 1: Creating system with %d CPUs\n", num_cpus);
    printf("----------------------------------------\n");
    ThreadSystem* system = threads_create_dynamic(num_cpus);
    
    if (!system) {
        fprintf(stderr, "ERROR: Failed to create thread system\n");
        return 1;
    }
    
    printf("\nSystem created successfully!\n");
    printf("  Total spheres: %d\n", system->total_spheres);
    printf("  Levels: %d\n", system->num_levels);
    for (int i = 0; i < system->num_levels; i++) {
        printf("    Level %d: %d spheres\n", i, system->spheres_per_level[i]);
    }
    
    // Verify sphere relationships
    printf("\nVerifying sphere relationships:\n");
    printf("  Root sphere: %d children\n", system->root->num_children);
    
    if (system->num_levels > 1) {
        printf("  Level 1 spheres:\n");
        for (int i = 0; i < system->spheres_per_level[1]; i++) {
            CLLMLatticeHierarchy* sphere = system->all_spheres[1 + i];
            printf("    Sphere %d: %d children, %d siblings\n", 
                   sphere->sphere_id, sphere->num_children, sphere->num_siblings);
        }
    }
    
    // Start threads
    printf("\nStarting threads...\n");
    if (threads_start(system) != 0) {
        fprintf(stderr, "ERROR: Failed to start threads\n");
        threads_free(system);
        return 1;
    }
    
    printf("All threads started successfully!\n");
    
    // Let threads run for a bit
    printf("\nLetting threads run for 2 seconds...\n");
    sleep(2);
    
    // Stop threads
    printf("\nStopping threads...\n");
    if (threads_stop(system) != 0) {
        fprintf(stderr, "ERROR: Failed to stop threads\n");
        threads_free(system);
        return 1;
    }
    
    printf("All threads stopped successfully!\n");
    
    // Print statistics
    printf("\nThread Statistics:\n");
    threads_print_stats(system);
    
    // Clean up
    threads_free(system);
    
    printf("\n=== Test Complete ===\n");
    
    return 0;
}