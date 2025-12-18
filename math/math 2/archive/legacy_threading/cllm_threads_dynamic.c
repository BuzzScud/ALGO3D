/**
 * CLLM Dynamic Kissing Spheres Threading
 * 
 * Implements dynamic sphere creation based on available CPU cores.
 * Creates optimal hierarchy depth and distribution for any number of CPUs.
 * 
 * Key Features:
 * - Dynamic depth calculation
 * - Partial level creation (not all 12^N spheres)
 * - Even distribution across available CPUs
 * - Maintains 12-fold symmetry where possible
 */

#include "ai/cllm_lattice_hierarchy.h"
#include "cllm_threads.h"
#include "cllm_training.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

/**
 * Calculate optimal hierarchy depth for given number of CPUs
 * 
 * Strategy:
 * - Level 0: Always 1 (root control)
 * - Level 1: Always 12 (if CPUs >= 13)
 * - Level 2+: Distribute remaining CPUs
 * 
 * @param num_cpus Number of available CPU cores
 * @param out_levels Output array for spheres per level
 * @return Number of levels needed
 */
static int calculate_optimal_depth(int num_cpus, int* out_levels) {
    if (num_cpus <= 0) return 0;
    
    // Level 0: Root control thread
    out_levels[0] = 1;
    
    if (num_cpus == 1) {
        return 1;  // Only root
    }
    
    // Level 1: 12 kissing spheres (if we have enough CPUs)
    int remaining = num_cpus - 1;  // Subtract root
    
    if (remaining <= 12) {
        // Create only as many Level 1 spheres as we have CPUs
        out_levels[1] = remaining;
        return 2;
    }
    
    // We have more than 13 CPUs, create full Level 1
    out_levels[1] = 12;
    remaining -= 12;  // Now remaining = num_cpus - 13
    
    if (remaining == 0) {
        return 2;  // Exactly 13 CPUs
    }
    
    // Level 2: Distribute remaining CPUs
    // Each Level 1 sphere can have up to 12 children
    // Distribute evenly: remaining / 12 children per sphere
    
    int children_per_sphere = remaining / 12;
    int extra_children = remaining % 12;
    
    // Total Level 2 spheres
    out_levels[2] = remaining;
    
    printf("Dynamic depth calculation:\n");
    printf("  Total CPUs: %d\n", num_cpus);
    printf("  Level 0: %d (root)\n", out_levels[0]);
    printf("  Level 1: %d (kissing spheres)\n", out_levels[1]);
    printf("  Level 2: %d (distributed: %d per sphere + %d extra)\n", 
           out_levels[2], children_per_sphere, extra_children);
    
    return 3;  // 3 levels total
}

/**
 * Create dynamic kissing spheres system
 * 
 * Creates a hierarchical system optimized for the given number of CPUs.
 * Unlike the fixed-level version, this creates partial levels as needed.
 * 
 * @param num_cpus Number of CPU cores to utilize
 * @return New thread system, or NULL on error
 */
ThreadSystem* threads_create_dynamic(int num_cpus) {
    if (num_cpus <= 0) {
        fprintf(stderr, "ERROR: Invalid number of CPUs: %d\n", num_cpus);
        return NULL;
    }
    
    ThreadSystem* system = calloc(1, sizeof(ThreadSystem));
    if (!system) return NULL;
    
    // Calculate optimal depth
    int levels[4] = {0};
    system->num_levels = calculate_optimal_depth(num_cpus, levels);
    
    for (int i = 0; i < system->num_levels; i++) {
        system->spheres_per_level[i] = levels[i];
    }
    
    // Calculate total spheres
    system->total_spheres = 0;
    for (int i = 0; i < system->num_levels; i++) {
        system->total_spheres += system->spheres_per_level[i];
    }
    
    printf("Creating dynamic kissing spheres system:\n");
    printf("  CPUs: %d\n", num_cpus);
    printf("  Levels: %d\n", system->num_levels);
    printf("  Total spheres: %d\n", system->total_spheres);
    
    // Allocate sphere array
    system->all_spheres = calloc(system->total_spheres, sizeof(CLLMLatticeHierarchy*));
    if (!system->all_spheres) {
        free(system);
        return NULL;
    }
    
    // Create root sphere (level 0)
    int all_groups[12] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    system->root = lattice_hierarchy_create(0, 0, all_groups, 12, 0, NULL);
    if (!system->root) {
        free(system->all_spheres);
        free(system);
        return NULL;
    }
    system->all_spheres[0] = system->root;
    
    int sphere_index = 1;
    
    // Create level 1 spheres (up to 12)
    if (system->num_levels > 1) {
        int num_level1 = system->spheres_per_level[1];
        
        for (int g = 0; g < num_level1; g++) {
            int group[1] = {g};
            CLLMLatticeHierarchy* sphere = lattice_hierarchy_create(
                sphere_index, 1, group, 1, sphere_index % num_cpus, system->root
            );
            
            if (!sphere) {
                fprintf(stderr, "ERROR: Failed to create level 1 sphere %d\n", g);
                threads_free(system);
                return NULL;
            }
            
            system->all_spheres[sphere_index] = sphere;
            lattice_hierarchy_add_child(system->root, sphere);
            sphere_index++;
        }
        
        // Discover siblings at level 1
        if (num_level1 > 1) {
            CLLMLatticeHierarchy** level1_spheres = &system->all_spheres[1];
            lattice_hierarchy_discover_siblings(level1_spheres, num_level1);
        }
    }
    
    // Create level 2 spheres (distributed across level 1 parents)
    if (system->num_levels > 2 && system->spheres_per_level[2] > 0) {
        int num_level1 = system->spheres_per_level[1];
        int num_level2 = system->spheres_per_level[2];
        
        // Distribute level 2 spheres evenly across level 1 parents
        int children_per_parent = num_level2 / num_level1;
        int extra_children = num_level2 % num_level1;
        
        int child_index = 0;
        for (int parent_idx = 1; parent_idx <= num_level1; parent_idx++) {
            CLLMLatticeHierarchy* parent = system->all_spheres[parent_idx];
            
            // This parent gets base children + maybe 1 extra
            int num_children = children_per_parent + (child_index < extra_children ? 1 : 0);
            
            for (int c = 0; c < num_children; c++) {
                int group[1] = {c % 12};  // Cycle through symmetry groups
                CLLMLatticeHierarchy* sphere = lattice_hierarchy_create(
                    sphere_index, 2, group, 1, sphere_index % num_cpus, parent
                );
                
                if (!sphere) {
                    fprintf(stderr, "ERROR: Failed to create level 2 sphere\n");
                    threads_free(system);
                    return NULL;
                }
                
                system->all_spheres[sphere_index] = sphere;
                lattice_hierarchy_add_child(parent, sphere);
                sphere_index++;
            }
            
            // Discover siblings within this parent's children
            if (parent->num_children > 1) {
                lattice_hierarchy_discover_siblings(parent->children, parent->num_children);
            }
            
            child_index++;
        }
    }
    
    printf("Dynamic kissing spheres system created successfully\n");
    printf("  Actual spheres created: %d\n", sphere_index);
    
    return system;
}