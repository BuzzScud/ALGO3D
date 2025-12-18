/**
 * Kissing Spheres Threading Demo
 * 
 * Demonstrates the proper implementation of the kissing spheres threading system:
 * - Creates hierarchical sphere system
 * - Distributes work across spheres
 * - Shows work stealing between siblings
 * - Displays statistics
 */

#include "cllm_threads.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char** argv) {
    printf("========================================\n");
    printf("CLLM Kissing Spheres Threading Demo\n");
    printf("========================================\n\n");
    
    // Parse command line arguments
    int num_levels = 2;  // Default: root + 12 kissing spheres
    int num_work_items = 1000;
    int run_time_seconds = 5;
    
    if (argc > 1) num_levels = atoi(argv[1]);
    if (argc > 2) num_work_items = atoi(argv[2]);
    if (argc > 3) run_time_seconds = atoi(argv[3]);
    
    printf("Configuration:\n");
    printf("  Hierarchy levels: %d\n", num_levels);
    printf("  Work items: %d\n", num_work_items);
    printf("  Run time: %d seconds\n\n", run_time_seconds);
    
    // Create kissing spheres system
    printf("Creating kissing spheres system...\n");
    ThreadSystem* system = threads_create(num_levels);
    if (!system) {
        fprintf(stderr, "ERROR: Failed to create kissing spheres system\n");
        return 1;
    }
    printf("\n");
    
    // Start all sphere threads
    printf("Starting sphere threads...\n");
    if (threads_start(system) != 0) {
        fprintf(stderr, "ERROR: Failed to start sphere threads\n");
        threads_free(system);
        return 1;
    }
    printf("\n");
    
    // Give threads time to initialize
    sleep(1);
    
    // Create work items
    printf("Creating %d work items...\n", num_work_items);
    uint64_t* work_items = malloc(num_work_items * sizeof(uint64_t));
    for (int i = 0; i < num_work_items; i++) {
        work_items[i] = i;
    }
    
    // Distribute work
    printf("Distributing work to spheres...\n");
    if (threads_distribute_work(system, work_items, num_work_items) != 0) {
        fprintf(stderr, "ERROR: Failed to distribute work\n");
        free(work_items);
        threads_stop(system);
        threads_free(system);
        return 1;
    }
    printf("\n");
    
    // Let system run
    printf("Running for %d seconds...\n", run_time_seconds);
    for (int i = 0; i < run_time_seconds; i++) {
        sleep(1);
        printf("  %d/%d seconds elapsed\n", i+1, run_time_seconds);
    }
    printf("\n");
    
    // Print statistics
    threads_print_stats(system);
    
    // Stop system
    printf("Stopping sphere threads...\n");
    threads_stop(system);
    printf("\n");
    
    // Clean up
    free(work_items);
    threads_free(system);
    
    printf("Demo completed successfully!\n");
    
    return 0;
}