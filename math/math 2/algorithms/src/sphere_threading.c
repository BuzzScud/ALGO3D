/**
 * Sphere Threading Model Implementation
 * 
 * Generic threading model based on kissing spheres geometry.
 * Uses clock lattice structure for deterministic neighbor assignment.
 * 
 * PHASE 1 WEEK 2: Migrated to NEW math library
 * - Replaced math_sqrt with math_sqrt (1 call)
 * Total: 1 function call migrated to NEW math library
 */

#include "sphere_threading.h"
#include "math/transcendental.h"  // PHASE 1: NEW math library
#include "math/arithmetic.h"       // PHASE 1: NEW math library
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Forward declarations to avoid type conflicts with OLD library
// We only need the function, not the full header
#include "math/clock.h"

// ============================================================================
// INITIALIZATION
// ============================================================================

SphereThreadingModel* sphere_threading_init(uint32_t num_spheres) {
    if (num_spheres == 0) {
        fprintf(stderr, "ERROR: num_spheres must be > 0\n");
        return NULL;
    }
    
    // Allocate model
    SphereThreadingModel* model = calloc(1, sizeof(SphereThreadingModel));
    if (!model) {
        fprintf(stderr, "ERROR: Failed to allocate threading model\n");
        return NULL;
    }
    
    model->num_spheres = num_spheres;
    model->total_work_units = 0;
    model->cache_optimized = 0;
    model->total_work_processed = 0;
    model->total_time = 0.0;
    model->app_context = NULL;
    
    // Allocate spheres
    model->spheres = calloc(num_spheres, sizeof(Sphere));
    if (!model->spheres) {
        fprintf(stderr, "ERROR: Failed to allocate spheres\n");
        free(model);
        return NULL;
    }
    
    // Allocate neighbor array (flattened)
    model->neighbor_array = calloc(num_spheres * SPHERE_NUM_NEIGHBORS, sizeof(uint32_t));
    if (!model->neighbor_array) {
        fprintf(stderr, "ERROR: Failed to allocate neighbor array\n");
        free(model->spheres);
        free(model);
        return NULL;
    }
    
    // Initialize spheres with neighbors using clock geometry
    for (uint32_t i = 0; i < num_spheres; i++) {
        Sphere* sphere = &model->spheres[i];
        sphere->id = i;
        sphere->num_neighbors = SPHERE_NUM_NEIGHBORS;
        sphere->symmetry_group = sphere_get_symmetry_group(i);
        sphere->work_data = NULL;
        sphere->work_size = 0;
        sphere->work_units_processed = 0;
        sphere->processing_time = 0.0;
        
        // Assign neighbors using clock geometry
        for (uint32_t group = 0; group < SPHERE_NUM_NEIGHBORS; group++) {
            uint32_t neighbor_id = sphere_find_neighbor_by_geometry(
                num_spheres, i, group
            );
            sphere->neighbors[group] = neighbor_id;
            model->neighbor_array[i * SPHERE_NUM_NEIGHBORS + group] = neighbor_id;
        }
    }
    
    return model;
}

void sphere_threading_free(SphereThreadingModel* model) {
    if (!model) return;
    
    if (model->spheres) {
        // Free any work data
        for (uint32_t i = 0; i < model->num_spheres; i++) {
            if (model->spheres[i].work_data) {
                free(model->spheres[i].work_data);
            }
        }
        free(model->spheres);
    }
    
    if (model->neighbor_array) {
        free(model->neighbor_array);
    }
    
    free(model);
}

// ============================================================================
// NEIGHBOR OPERATIONS
// ============================================================================

uint32_t sphere_find_neighbor_by_geometry(
    uint32_t num_spheres,
    uint32_t sphere_id,
    uint32_t target_group
) {
    // Map sphere to clock position
    ClockPosition pos;
    clock_map_index_to_position((uint64_t)sphere_id, &pos);
    
    // Calculate positions per ring
    uint32_t positions_in_ring;
    if (pos.ring == 0) {
        positions_in_ring = 12;
    } else if (pos.ring == 1 || pos.ring == 2) {
        positions_in_ring = 60;
    } else if (pos.ring == 3) {
        positions_in_ring = 100;
    } else {
        positions_in_ring = 1000;
    }
    
    // Neighbor offset based on target symmetry group
    // Each group gets evenly spaced neighbors around the ring
    uint32_t offset = (positions_in_ring * target_group) / 12;
    
    // Calculate neighbor position in same ring
    uint32_t neighbor_pos = (pos.position + offset) % positions_in_ring;
    
    // Convert back to sphere ID
    uint32_t base_index = 0;
    if (pos.ring == 0) {
        base_index = 0;
    } else if (pos.ring == 1) {
        base_index = 12;
    } else if (pos.ring == 2) {
        base_index = 12 + 60;
    } else if (pos.ring == 3) {
        base_index = 12 + 60 + 60;
    } else {
        base_index = 12 + 60 + 60 + 100 + (pos.ring - 4) * 1000;
    }
    
    uint32_t neighbor_id = base_index + neighbor_pos;
    
    // Ensure within bounds
    if (neighbor_id >= num_spheres) {
        neighbor_id = neighbor_id % num_spheres;
    }
    
    return neighbor_id;
}

int sphere_get_neighbors(
    const SphereThreadingModel* model,
    uint32_t sphere_id,
    uint32_t* neighbors
) {
    if (!model || !neighbors) return -1;
    if (sphere_id >= model->num_spheres) return -1;
    
    const Sphere* sphere = &model->spheres[sphere_id];
    memcpy(neighbors, sphere->neighbors, SPHERE_NUM_NEIGHBORS * sizeof(uint32_t));
    
    return SPHERE_NUM_NEIGHBORS;
}

// ============================================================================
// CACHE OPTIMIZATION
// ============================================================================

int sphere_optimize_cache_locality(
    SphereThreadingModel* model,
    size_t work_data_size
) {
    if (!model) return -1;
    if (model->cache_optimized) return 0; // Already optimized
    
    // For each sphere, sort its neighbors by memory address
    for (uint32_t i = 0; i < model->num_spheres; i++) {
        Sphere* sphere = &model->spheres[i];
        
        // Simple bubble sort (only 12 elements, very fast)
        for (uint32_t pass = 0; pass < sphere->num_neighbors - 1; pass++) {
            for (uint32_t j = 0; j < sphere->num_neighbors - pass - 1; j++) {
                uint32_t id1 = sphere->neighbors[j];
                uint32_t id2 = sphere->neighbors[j + 1];
                
                // Compare memory addresses (work data offsets)
                size_t addr1 = id1 * work_data_size;
                size_t addr2 = id2 * work_data_size;
                
                // Sort by address (ascending)
                if (addr1 > addr2) {
                    // Swap
                    sphere->neighbors[j] = id2;
                    sphere->neighbors[j + 1] = id1;
                    
                    // Also update flattened array
                    model->neighbor_array[i * SPHERE_NUM_NEIGHBORS + j] = id2;
                    model->neighbor_array[i * SPHERE_NUM_NEIGHBORS + j + 1] = id1;
                }
            }
        }
    }
    
    model->cache_optimized = 1;
    return 0;
}

// ============================================================================
// WORK DISTRIBUTION
// ============================================================================

int sphere_assign_work(
    SphereThreadingModel* model,
    void* work_units,
    uint32_t num_work_units,
    size_t work_unit_size
) {
    if (!model || !work_units) return -1;
    if (num_work_units == 0 || work_unit_size == 0) return -1;
    
    model->total_work_units = num_work_units;
    
    // Distribute work across spheres based on symmetry groups
    // This ensures balanced distribution
    uint32_t work_per_sphere = num_work_units / model->num_spheres;
    uint32_t remainder = num_work_units % model->num_spheres;
    
    uint32_t work_offset = 0;
    for (uint32_t i = 0; i < model->num_spheres; i++) {
        Sphere* sphere = &model->spheres[i];
        
        // Calculate work for this sphere
        uint32_t sphere_work = work_per_sphere;
        if (i < remainder) {
            sphere_work++; // Distribute remainder
        }
        
        if (sphere_work > 0) {
            // Allocate work data
            sphere->work_size = sphere_work * work_unit_size;
            sphere->work_data = malloc(sphere->work_size);
            if (!sphere->work_data) {
                fprintf(stderr, "ERROR: Failed to allocate work data for sphere %u\n", i);
                return -1;
            }
            
            // Copy work units
            memcpy(sphere->work_data,
                   (char*)work_units + work_offset * work_unit_size,
                   sphere->work_size);
            
            work_offset += sphere_work;
        }
    }
    
    return 0;
}

int sphere_process_work(
    SphereThreadingModel* model,
    SphereWorkFunction work_fn,
    void* context
) {
    if (!model || !work_fn) return -1;
    
    // Process work for each sphere
    for (uint32_t i = 0; i < model->num_spheres; i++) {
        Sphere* sphere = &model->spheres[i];
        
        // Get neighbors
        uint32_t neighbors[SPHERE_NUM_NEIGHBORS];
        memcpy(neighbors, sphere->neighbors, sizeof(neighbors));
        
        // Execute work function
        int result = work_fn(sphere, neighbors, context);
        if (result != 0) {
            fprintf(stderr, "ERROR: Work function failed for sphere %u\n", i);
            return -1;
        }
    }
    
    return 0;
}

// ============================================================================
// STATISTICS
// ============================================================================

int sphere_get_statistics(
    const SphereThreadingModel* model,
    uint32_t sphere_id,
    SphereStatistics* stats
) {
    if (!model || !stats) return -1;
    if (sphere_id >= model->num_spheres) return -1;
    
    const Sphere* sphere = &model->spheres[sphere_id];
    
    stats->sphere_id = sphere->id;
    stats->symmetry_group = sphere->symmetry_group;
    stats->work_units_processed = sphere->work_units_processed;
    stats->processing_time = sphere->processing_time;
    stats->avg_time_per_unit = (sphere->work_units_processed > 0) ?
        sphere->processing_time / sphere->work_units_processed : 0.0;
    stats->num_neighbors = sphere->num_neighbors;
    
    return 0;
}

int sphere_get_system_statistics(
    const SphereThreadingModel* model,
    SystemStatistics* stats
) {
    if (!model || !stats) return -1;
    
    stats->num_spheres = model->num_spheres;
    stats->total_work_processed = model->total_work_processed;
    stats->total_time = model->total_time;
    stats->avg_time_per_sphere = (model->num_spheres > 0) ?
        model->total_time / model->num_spheres : 0.0;
    
    // Calculate load balance score
    // Perfect balance = all spheres have same work
    if (model->num_spheres > 0) {
        double avg_work = (double)model->total_work_processed / model->num_spheres;
        double variance = 0.0;
        
        for (uint32_t i = 0; i < model->num_spheres; i++) {
            double diff = model->spheres[i].work_units_processed - avg_work;
            variance += diff * diff;
        }
        
        variance /= model->num_spheres;
        double std_dev = math_sqrt(variance);
        
        // Score: 1.0 = perfect balance, 0.0 = worst balance
        stats->load_balance_score = (avg_work > 0) ? 
            1.0 / (1.0 + std_dev / avg_work) : 1.0;
    } else {
        stats->load_balance_score = 1.0;
    }
    
    return 0;
}

// ============================================================================
// VALIDATION
// ============================================================================

int sphere_verify_model(const SphereThreadingModel* model) {
    if (!model) return 0;
    
    int errors = 0;
    
    // Verify each sphere
    for (uint32_t i = 0; i < model->num_spheres; i++) {
        const Sphere* sphere = &model->spheres[i];
        
        // Check neighbor count
        if (sphere->num_neighbors != SPHERE_NUM_NEIGHBORS) {
            fprintf(stderr, "ERROR: Sphere %u has %u neighbors (expected %d)\n",
                    i, sphere->num_neighbors, SPHERE_NUM_NEIGHBORS);
            errors++;
        }
        
        // Check symmetry group
        if (sphere->symmetry_group != sphere_get_symmetry_group(i)) {
            fprintf(stderr, "ERROR: Sphere %u has wrong symmetry group\n", i);
            errors++;
        }
        
        // Check neighbors
        for (uint32_t j = 0; j < sphere->num_neighbors; j++) {
            uint32_t neighbor_id = sphere->neighbors[j];
            
            // Check bounds
            if (neighbor_id >= model->num_spheres) {
                fprintf(stderr, "ERROR: Sphere %u has invalid neighbor %u\n",
                        i, neighbor_id);
                errors++;
            }
            
            // Note: A sphere CAN be its own neighbor in some geometric configurations
            // This is acceptable for the clock geometry
        }
    }
    
    return (errors == 0) ? 1 : 0;
}
