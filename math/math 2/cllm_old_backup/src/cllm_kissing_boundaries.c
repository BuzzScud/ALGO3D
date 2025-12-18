/**
 * Kissing Boundary Optimization Implementation
 * 
 * Phase 3, Day 12
 */

#include "ai/cllm_kissing_boundaries.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ============================================================================
// LIFECYCLE
// ============================================================================

KissingBoundary* kissing_boundary_create(
    int sphere_a,
    int sphere_b,
    size_t size
) {
    if (sphere_a < 0 || sphere_b < 0) {
        fprintf(stderr, "ERROR: Invalid sphere IDs: %d, %d\n", sphere_a, sphere_b);
        return NULL;
    }
    
    if (size == 0) {
        size = KISSING_BOUNDARY_SIZE;
    }
    
    // Allocate structure
    KissingBoundary* boundary = calloc(1, sizeof(KissingBoundary));
    if (!boundary) {
        fprintf(stderr, "ERROR: Failed to allocate kissing boundary\n");
        return NULL;
    }
    
    // Allocate cache-aligned memory
    boundary->boundary_memory = aligned_alloc(64, size);
    if (!boundary->boundary_memory) {
        fprintf(stderr, "ERROR: Failed to allocate boundary memory\n");
        free(boundary);
        return NULL;
    }
    
    boundary->boundary_size = size;
    memset(boundary->boundary_memory, 0, size);
    
    // Set ownership
    boundary->sphere_a = sphere_a;
    boundary->sphere_b = sphere_b;
    boundary->owner_threads[0] = (uint32_t)sphere_a;
    boundary->owner_threads[1] = (uint32_t)sphere_b;
    
    // Initialize synchronization
    atomic_flag_clear(&boundary->write_lock);
    atomic_init(&boundary->version, 0);
    atomic_init(&boundary->readers, 0);
    
    // Initialize statistics
    atomic_init(&boundary->reads_a, 0);
    atomic_init(&boundary->reads_b, 0);
    atomic_init(&boundary->writes_a, 0);
    atomic_init(&boundary->writes_b, 0);
    atomic_init(&boundary->total_accesses, 0);
    atomic_init(&boundary->cache_invalidations, 0);
    atomic_init(&boundary->version_conflicts, 0);
    
    return boundary;
}

void kissing_boundary_destroy(KissingBoundary* boundary) {
    if (!boundary) return;
    
    if (boundary->boundary_memory) {
        free(boundary->boundary_memory);
    }
    
    free(boundary);
}

KissingBoundarySystem* kissing_boundary_system_create(int max_boundaries) {
    if (max_boundaries <= 0) {
        max_boundaries = MAX_KISSING_BOUNDARIES;
    }
    
    KissingBoundarySystem* system = calloc(1, sizeof(KissingBoundarySystem));
    if (!system) {
        fprintf(stderr, "ERROR: Failed to allocate kissing boundary system\n");
        return NULL;
    }
    
    system->boundaries = calloc(max_boundaries, sizeof(KissingBoundary));
    if (!system->boundaries) {
        fprintf(stderr, "ERROR: Failed to allocate boundaries array\n");
        free(system);
        return NULL;
    }
    
    system->max_boundaries = max_boundaries;
    system->num_boundaries = 0;
    
    // Initialize statistics
    atomic_init(&system->total_reads, 0);
    atomic_init(&system->total_writes, 0);
    atomic_init(&system->total_conflicts, 0);
    
    return system;
}

void kissing_boundary_system_destroy(KissingBoundarySystem* system) {
    if (!system) return;
    
    // Note: boundaries are not owned by the system, just referenced
    if (system->boundaries) {
        free(system->boundaries);
    }
    
    free(system);
}

// ============================================================================
// MEMORY ACCESS (LOCK-FREE READS)
// ============================================================================

size_t kissing_boundary_read(
    KissingBoundary* boundary,
    int sphere_id,
    size_t offset,
    void* buffer,
    size_t size
) {
    if (!boundary || !buffer) {
        return 0;
    }
    
    // Bounds checking
    if (offset + size > boundary->boundary_size) {
        fprintf(stderr, "ERROR: Boundary read out of bounds\n");
        return 0;
    }
    
    // Determine which sphere is reading
    bool is_sphere_a = (sphere_id == boundary->sphere_a);
    bool is_sphere_b = (sphere_id == boundary->sphere_b);
    
    if (!is_sphere_a && !is_sphere_b) {
        fprintf(stderr, "ERROR: Invalid sphere ID %d for boundary\n", sphere_id);
        return 0;
    }
    
    // Lock-free read: increment reader count
    atomic_fetch_add(&boundary->readers, 1);
    
    // Read version before access
    uint32_t version_before = atomic_load(&boundary->version);
    
    // Perform read
    memcpy(buffer, (char*)boundary->boundary_memory + offset, size);
    
    // Read version after access
    uint32_t version_after = atomic_load(&boundary->version);
    
    // Decrement reader count
    atomic_fetch_sub(&boundary->readers, 1);
    
    // Check for version conflict
    if (version_before != version_after) {
        atomic_fetch_add(&boundary->version_conflicts, 1);
    }
    
    // Update statistics
    if (is_sphere_a) {
        atomic_fetch_add(&boundary->reads_a, 1);
    } else {
        atomic_fetch_add(&boundary->reads_b, 1);
    }
    atomic_fetch_add(&boundary->total_accesses, 1);
    
    return size;
}

size_t kissing_boundary_write(
    KissingBoundary* boundary,
    int sphere_id,
    size_t offset,
    const void* buffer,
    size_t size
) {
    if (!boundary || !buffer) {
        return 0;
    }
    
    // Bounds checking
    if (offset + size > boundary->boundary_size) {
        fprintf(stderr, "ERROR: Boundary write out of bounds\n");
        return 0;
    }
    
    // Determine which sphere is writing
    bool is_sphere_a = (sphere_id == boundary->sphere_a);
    bool is_sphere_b = (sphere_id == boundary->sphere_b);
    
    if (!is_sphere_a && !is_sphere_b) {
        fprintf(stderr, "ERROR: Invalid sphere ID %d for boundary\n", sphere_id);
        return 0;
    }
    
    // Acquire write lock (spinlock)
    while (atomic_flag_test_and_set(&boundary->write_lock)) {
        // Spin
    }
    
    // Wait for all readers to finish
    while (atomic_load(&boundary->readers) > 0) {
        // Spin
    }
    
    // Perform write
    memcpy((char*)boundary->boundary_memory + offset, buffer, size);
    
    // Increment version (invalidate caches)
    atomic_fetch_add(&boundary->version, 1);
    atomic_fetch_add(&boundary->cache_invalidations, 1);
    
    // Release write lock
    atomic_flag_clear(&boundary->write_lock);
    
    // Update statistics
    if (is_sphere_a) {
        atomic_fetch_add(&boundary->writes_a, 1);
    } else {
        atomic_fetch_add(&boundary->writes_b, 1);
    }
    atomic_fetch_add(&boundary->total_accesses, 1);
    
    return size;
}

int kissing_boundary_atomic_exchange(
    KissingBoundary* boundary,
    int sphere_id,
    size_t offset,
    const void* new_value,
    void* old_value,
    size_t size
) {
    if (!boundary || !new_value || !old_value) {
        return 0;
    }
    
    // Only support 4 or 8 byte atomic operations
    if (size != 4 && size != 8) {
        fprintf(stderr, "ERROR: Atomic exchange only supports 4 or 8 bytes\n");
        return 0;
    }
    
    // Bounds checking
    if (offset + size > boundary->boundary_size) {
        fprintf(stderr, "ERROR: Atomic exchange out of bounds\n");
        return 0;
    }
    
    // Determine which sphere is accessing
    bool is_sphere_a = (sphere_id == boundary->sphere_a);
    bool is_sphere_b = (sphere_id == boundary->sphere_b);
    
    if (!is_sphere_a && !is_sphere_b) {
        fprintf(stderr, "ERROR: Invalid sphere ID %d for boundary\n", sphere_id);
        return 0;
    }
    
    // Acquire write lock
    while (atomic_flag_test_and_set(&boundary->write_lock)) {
        // Spin
    }
    
    // Wait for readers
    while (atomic_load(&boundary->readers) > 0) {
        // Spin
    }
    
    // Perform atomic exchange
    void* ptr = (char*)boundary->boundary_memory + offset;
    
    if (size == 4) {
        uint32_t* target = (uint32_t*)ptr;
        uint32_t* old = (uint32_t*)old_value;
        uint32_t* new = (uint32_t*)new_value;
        *old = *target;
        *target = *new;
    } else { // size == 8
        uint64_t* target = (uint64_t*)ptr;
        uint64_t* old = (uint64_t*)old_value;
        uint64_t* new = (uint64_t*)new_value;
        *old = *target;
        *target = *new;
    }
    
    // Increment version
    atomic_fetch_add(&boundary->version, 1);
    atomic_fetch_add(&boundary->cache_invalidations, 1);
    
    // Release write lock
    atomic_flag_clear(&boundary->write_lock);
    
    // Update statistics
    if (is_sphere_a) {
        atomic_fetch_add(&boundary->writes_a, 1);
    } else {
        atomic_fetch_add(&boundary->writes_b, 1);
    }
    atomic_fetch_add(&boundary->total_accesses, 1);
    
    return 1;
}

// ============================================================================
// CACHE COHERENCY
// ============================================================================

uint32_t kissing_boundary_get_version(const KissingBoundary* boundary) {
    if (!boundary) return 0;
    return atomic_load(&boundary->version);
}

void kissing_boundary_invalidate_cache(KissingBoundary* boundary) {
    if (!boundary) return;
    
    atomic_fetch_add(&boundary->version, 1);
    atomic_fetch_add(&boundary->cache_invalidations, 1);
}

int kissing_boundary_check_version(
    const KissingBoundary* boundary,
    uint32_t cached_version
) {
    if (!boundary) return 0;
    
    uint32_t current_version = atomic_load(&boundary->version);
    return (current_version == cached_version) ? 1 : 0;
}

// ============================================================================
// STATISTICS
// ============================================================================

void kissing_boundary_get_stats(
    const KissingBoundary* boundary,
    uint64_t* reads_a,
    uint64_t* reads_b,
    uint64_t* writes_a,
    uint64_t* writes_b
) {
    if (!boundary) return;
    
    if (reads_a) *reads_a = atomic_load(&boundary->reads_a);
    if (reads_b) *reads_b = atomic_load(&boundary->reads_b);
    if (writes_a) *writes_a = atomic_load(&boundary->writes_a);
    if (writes_b) *writes_b = atomic_load(&boundary->writes_b);
}

void kissing_boundary_print_stats(const KissingBoundary* boundary) {
    if (!boundary) return;
    
    printf("Kissing Boundary Statistics (Spheres %d <-> %d):\n",
           boundary->sphere_a, boundary->sphere_b);
    printf("  Reads A:  %llu\n", (unsigned long long)atomic_load(&boundary->reads_a));
    printf("  Reads B:  %llu\n", (unsigned long long)atomic_load(&boundary->reads_b));
    printf("  Writes A: %llu\n", (unsigned long long)atomic_load(&boundary->writes_a));
    printf("  Writes B: %llu\n", (unsigned long long)atomic_load(&boundary->writes_b));
    printf("  Total:    %llu\n", (unsigned long long)atomic_load(&boundary->total_accesses));
    printf("  Version:  %lu\n", (unsigned long)atomic_load(&boundary->version));
    printf("  Cache Invalidations: %llu\n", 
           (unsigned long long)atomic_load(&boundary->cache_invalidations));
    printf("  Version Conflicts:   %llu\n",
           (unsigned long long)atomic_load(&boundary->version_conflicts));
}

void kissing_boundary_reset_stats(KissingBoundary* boundary) {
    if (!boundary) return;
    
    atomic_store(&boundary->reads_a, 0);
    atomic_store(&boundary->reads_b, 0);
    atomic_store(&boundary->writes_a, 0);
    atomic_store(&boundary->writes_b, 0);
    atomic_store(&boundary->total_accesses, 0);
    atomic_store(&boundary->cache_invalidations, 0);
    atomic_store(&boundary->version_conflicts, 0);
}

void kissing_boundary_system_get_stats(
    const KissingBoundarySystem* system,
    uint64_t* total_reads,
    uint64_t* total_writes,
    uint64_t* total_conflicts
) {
    if (!system) return;
    
    if (total_reads) *total_reads = atomic_load(&system->total_reads);
    if (total_writes) *total_writes = atomic_load(&system->total_writes);
    if (total_conflicts) *total_conflicts = atomic_load(&system->total_conflicts);
}

void kissing_boundary_system_print_stats(const KissingBoundarySystem* system) {
    if (!system) return;
    
    printf("Kissing Boundary System Statistics:\n");
    printf("  Active Boundaries: %d / %d\n", 
           system->num_boundaries, system->max_boundaries);
    printf("  Total Reads:  %llu\n", 
           (unsigned long long)atomic_load(&system->total_reads));
    printf("  Total Writes: %llu\n",
           (unsigned long long)atomic_load(&system->total_writes));
    printf("  Total Conflicts: %llu\n",
           (unsigned long long)atomic_load(&system->total_conflicts));
}

// ============================================================================
// SYSTEM MANAGEMENT
// ============================================================================

int kissing_boundary_system_add(
    KissingBoundarySystem* system,
    KissingBoundary* boundary
) {
    if (!system || !boundary) {
        return -1;
    }
    
    if (system->num_boundaries >= system->max_boundaries) {
        fprintf(stderr, "ERROR: Boundary system is full\n");
        return -1;
    }
    
    // Add boundary (just store pointer, don't copy)
    system->boundaries[system->num_boundaries] = *boundary;
    system->num_boundaries++;
    
    return 0;
}

KissingBoundary* kissing_boundary_system_find(
    const KissingBoundarySystem* system,
    int sphere_a,
    int sphere_b
) {
    if (!system) return NULL;
    
    for (int i = 0; i < system->num_boundaries; i++) {
        KissingBoundary* b = &system->boundaries[i];
        
        // Check both orderings
        if ((b->sphere_a == sphere_a && b->sphere_b == sphere_b) ||
            (b->sphere_a == sphere_b && b->sphere_b == sphere_a)) {
            return b;
        }
    }
    
    return NULL;
}

int kissing_boundary_system_validate(const KissingBoundarySystem* system) {
    if (!system) {
        fprintf(stderr, "ERROR: NULL system\n");
        return 0;
    }
    
    if (!system->boundaries) {
        fprintf(stderr, "ERROR: NULL boundaries array\n");
        return 0;
    }
    
    if (system->num_boundaries < 0 || 
        system->num_boundaries > system->max_boundaries) {
        fprintf(stderr, "ERROR: Invalid boundary count\n");
        return 0;
    }
    
    // Validate each boundary
    for (int i = 0; i < system->num_boundaries; i++) {
        KissingBoundary* b = &system->boundaries[i];
        
        if (!b->boundary_memory) {
            fprintf(stderr, "ERROR: Boundary %d has NULL memory\n", i);
            return 0;
        }
        
        if (b->boundary_size == 0) {
            fprintf(stderr, "ERROR: Boundary %d has zero size\n", i);
            return 0;
        }
        
        if (b->sphere_a < 0 || b->sphere_b < 0) {
            fprintf(stderr, "ERROR: Boundary %d has invalid sphere IDs\n", i);
            return 0;
        }
    }
    
    return 1;
}
