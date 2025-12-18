/**
 * Hierarchical Memory Implementation
 * 
 * Manages parent-child memory relationships with shared boundaries.
 */

#include "ai/cllm_hierarchical_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// LIFECYCLE
// ============================================================================

/**
 * Allocate hierarchical memory for parent and children
 */
HierarchicalMemory* hierarchical_memory_create(
    int parent_sphere_id,
    int num_children
) {
    if (num_children < 0 || num_children > MAX_CHILDREN) {
        fprintf(stderr, "ERROR: Invalid num_children=%d (must be 0-12)\n", num_children);
        return NULL;
    }
    
    // Allocate structure
    HierarchicalMemory* mem = calloc(1, sizeof(HierarchicalMemory));
    if (!mem) {
        fprintf(stderr, "ERROR: Failed to allocate hierarchical memory\n");
        return NULL;
    }
    
    // Set ownership
    mem->parent_sphere_id = parent_sphere_id;
    mem->num_children = num_children;
    
    // Initialize child IDs to -1 (no child)
    for (int i = 0; i < MAX_CHILDREN; i++) {
        mem->child_sphere_ids[i] = -1;
    }
    
    // Allocate parent memory (cache-aligned)
    mem->parent_memory = aligned_alloc(64, CONTROL_MEMORY_SIZE);
    if (!mem->parent_memory) {
        fprintf(stderr, "ERROR: Failed to allocate parent memory\n");
        free(mem);
        return NULL;
    }
    mem->parent_size = CONTROL_MEMORY_SIZE;
    memset(mem->parent_memory, 0, CONTROL_MEMORY_SIZE);
    
    // Allocate child memories and shared boundaries
    for (int i = 0; i < num_children; i++) {
        // Allocate child memory (cache-aligned)
        mem->child_memories[i] = aligned_alloc(64, WORKER_MEMORY_SIZE);
        if (!mem->child_memories[i]) {
            fprintf(stderr, "ERROR: Failed to allocate child %d memory\n", i);
            // Cleanup
            for (int j = 0; j < i; j++) {
                free(mem->child_memories[j]);
                free(mem->shared_boundaries[j]);
            }
            free(mem->parent_memory);
            free(mem);
            return NULL;
        }
        mem->child_sizes[i] = WORKER_MEMORY_SIZE;
        memset(mem->child_memories[i], 0, WORKER_MEMORY_SIZE);
        
        // Allocate shared boundary (cache-aligned)
        mem->shared_boundaries[i] = aligned_alloc(64, SHARED_BOUNDARY_SIZE);
        if (!mem->shared_boundaries[i]) {
            fprintf(stderr, "ERROR: Failed to allocate boundary %d\n", i);
            // Cleanup
            free(mem->child_memories[i]);
            for (int j = 0; j < i; j++) {
                free(mem->child_memories[j]);
                free(mem->shared_boundaries[j]);
            }
            free(mem->parent_memory);
            free(mem);
            return NULL;
        }
        mem->boundary_sizes[i] = SHARED_BOUNDARY_SIZE;
        memset(mem->shared_boundaries[i], 0, SHARED_BOUNDARY_SIZE);
    }
    
    // Initialize statistics
    atomic_init(&mem->parent_accesses, 0);
    for (int i = 0; i < MAX_CHILDREN; i++) {
        atomic_init(&mem->child_accesses[i], 0);
        atomic_init(&mem->boundary_accesses[i], 0);
    }
    
    return mem;
}

/**
 * Destroy hierarchical memory
 */
void hierarchical_memory_destroy(HierarchicalMemory* mem) {
    if (!mem) return;
    
    // Free parent memory
    if (mem->parent_memory) {
        free(mem->parent_memory);
    }
    
    // Free child memories and boundaries
    for (int i = 0; i < MAX_CHILDREN; i++) {
        if (mem->child_memories[i]) {
            free(mem->child_memories[i]);
        }
        if (mem->shared_boundaries[i]) {
            free(mem->shared_boundaries[i]);
        }
    }
    
    free(mem);
}

// ============================================================================
// MEMORY ACCESS
// ============================================================================

/**
 * Access parent memory
 */
size_t hierarchical_memory_access_parent(
    HierarchicalMemory* mem,
    size_t offset,
    void* buffer,
    size_t size,
    int write
) {
    if (!mem || !buffer || !mem->parent_memory) {
        return 0;
    }
    
    // Check bounds
    if (offset + size > mem->parent_size) {
        fprintf(stderr, "ERROR: Parent memory access out of bounds\n");
        return 0;
    }
    
    // Perform access
    void* ptr = (char*)mem->parent_memory + offset;
    if (write) {
        memcpy(ptr, buffer, size);
    } else {
        memcpy(buffer, ptr, size);
    }
    
    // Update statistics
    atomic_fetch_add(&mem->parent_accesses, 1);
    
    return size;
}

/**
 * Access child memory
 */
size_t hierarchical_memory_access_child(
    HierarchicalMemory* mem,
    int child_index,
    size_t offset,
    void* buffer,
    size_t size,
    int write
) {
    if (!mem || !buffer) {
        return 0;
    }
    
    if (child_index < 0 || child_index >= MAX_CHILDREN) {
        fprintf(stderr, "ERROR: Invalid child_index=%d\n", child_index);
        return 0;
    }
    
    if (!mem->child_memories[child_index]) {
        fprintf(stderr, "ERROR: Child %d memory not allocated\n", child_index);
        return 0;
    }
    
    // Check bounds
    if (offset + size > mem->child_sizes[child_index]) {
        fprintf(stderr, "ERROR: Child memory access out of bounds\n");
        return 0;
    }
    
    // Perform access
    void* ptr = (char*)mem->child_memories[child_index] + offset;
    if (write) {
        memcpy(ptr, buffer, size);
    } else {
        memcpy(buffer, ptr, size);
    }
    
    // Update statistics
    atomic_fetch_add(&mem->child_accesses[child_index], 1);
    
    return size;
}

/**
 * Access shared boundary
 */
size_t hierarchical_memory_access_boundary(
    HierarchicalMemory* mem,
    int child_index,
    size_t offset,
    void* buffer,
    size_t size,
    int write
) {
    if (!mem || !buffer) {
        return 0;
    }
    
    if (child_index < 0 || child_index >= MAX_CHILDREN) {
        fprintf(stderr, "ERROR: Invalid child_index=%d\n", child_index);
        return 0;
    }
    
    if (!mem->shared_boundaries[child_index]) {
        fprintf(stderr, "ERROR: Boundary %d not allocated\n", child_index);
        return 0;
    }
    
    // Check bounds
    if (offset + size > mem->boundary_sizes[child_index]) {
        fprintf(stderr, "ERROR: Boundary access out of bounds\n");
        return 0;
    }
    
    // Perform access
    void* ptr = (char*)mem->shared_boundaries[child_index] + offset;
    if (write) {
        memcpy(ptr, buffer, size);
    } else {
        memcpy(buffer, ptr, size);
    }
    
    // Update statistics
    atomic_fetch_add(&mem->boundary_accesses[child_index], 1);
    
    return size;
}

// ============================================================================
// CHILD MANAGEMENT
// ============================================================================

/**
 * Add child to hierarchical memory
 */
int hierarchical_memory_add_child(
    HierarchicalMemory* mem,
    int child_index,
    int child_sphere_id
) {
    if (!mem) {
        return 0;
    }
    
    if (child_index < 0 || child_index >= MAX_CHILDREN) {
        fprintf(stderr, "ERROR: Invalid child_index=%d\n", child_index);
        return 0;
    }
    
    if (child_index >= mem->num_children) {
        fprintf(stderr, "ERROR: child_index=%d exceeds num_children=%d\n",
                child_index, mem->num_children);
        return 0;
    }
    
    // Set child ID
    mem->child_sphere_ids[child_index] = child_sphere_id;
    
    return 1;
}

/**
 * Remove child from hierarchical memory
 */
int hierarchical_memory_remove_child(
    HierarchicalMemory* mem,
    int child_index
) {
    if (!mem) {
        return 0;
    }
    
    if (child_index < 0 || child_index >= MAX_CHILDREN) {
        fprintf(stderr, "ERROR: Invalid child_index=%d\n", child_index);
        return 0;
    }
    
    // Clear child ID
    mem->child_sphere_ids[child_index] = -1;
    
    return 1;
}

/**
 * Get child sphere ID
 */
int hierarchical_memory_get_child_id(
    const HierarchicalMemory* mem,
    int child_index
) {
    if (!mem) {
        return -1;
    }
    
    if (child_index < 0 || child_index >= MAX_CHILDREN) {
        return -1;
    }
    
    return mem->child_sphere_ids[child_index];
}

// ============================================================================
// STATISTICS
// ============================================================================

/**
 * Get memory access statistics
 */
void hierarchical_memory_get_stats(
    const HierarchicalMemory* mem,
    uint64_t* parent_accesses,
    uint64_t* total_child_accesses,
    uint64_t* total_boundary_accesses
) {
    if (!mem) return;
    
    if (parent_accesses) {
        *parent_accesses = atomic_load(&mem->parent_accesses);
    }
    
    if (total_child_accesses) {
        uint64_t total = 0;
        for (int i = 0; i < mem->num_children; i++) {
            total += atomic_load(&mem->child_accesses[i]);
        }
        *total_child_accesses = total;
    }
    
    if (total_boundary_accesses) {
        uint64_t total = 0;
        for (int i = 0; i < mem->num_children; i++) {
            total += atomic_load(&mem->boundary_accesses[i]);
        }
        *total_boundary_accesses = total;
    }
}

/**
 * Print memory statistics
 */
void hierarchical_memory_print_stats(const HierarchicalMemory* mem) {
    if (!mem) {
        printf("NULL hierarchical memory\n");
        return;
    }
    
    printf("Hierarchical Memory Statistics:\n");
    printf("  Parent Sphere ID: %d\n", mem->parent_sphere_id);
    printf("  Number of Children: %d\n", mem->num_children);
    printf("  Parent Accesses: %lu\n", atomic_load(&mem->parent_accesses));
    
    for (int i = 0; i < mem->num_children; i++) {
        printf("  Child %d (ID=%d):\n", i, mem->child_sphere_ids[i]);
        printf("    Memory Accesses: %lu\n", atomic_load(&mem->child_accesses[i]));
        printf("    Boundary Accesses: %lu\n", atomic_load(&mem->boundary_accesses[i]));
    }
}

/**
 * Reset memory statistics
 */
void hierarchical_memory_reset_stats(HierarchicalMemory* mem) {
    if (!mem) return;
    
    atomic_store(&mem->parent_accesses, 0);
    
    for (int i = 0; i < MAX_CHILDREN; i++) {
        atomic_store(&mem->child_accesses[i], 0);
        atomic_store(&mem->boundary_accesses[i], 0);
    }
}

// ============================================================================
// UTILITIES
// ============================================================================

/**
 * Validate hierarchical memory
 */
int hierarchical_memory_validate(const HierarchicalMemory* mem) {
    if (!mem) {
        fprintf(stderr, "ERROR: NULL hierarchical memory\n");
        return 0;
    }
    
    // Check parent memory
    if (!mem->parent_memory) {
        fprintf(stderr, "ERROR: Parent memory not allocated\n");
        return 0;
    }
    
    if (mem->parent_size != CONTROL_MEMORY_SIZE) {
        fprintf(stderr, "ERROR: Invalid parent size=%zu (expected %d)\n",
                mem->parent_size, CONTROL_MEMORY_SIZE);
        return 0;
    }
    
    // Check num_children
    if (mem->num_children < 0 || mem->num_children > MAX_CHILDREN) {
        fprintf(stderr, "ERROR: Invalid num_children=%d\n", mem->num_children);
        return 0;
    }
    
    // Check child memories and boundaries
    for (int i = 0; i < mem->num_children; i++) {
        if (!mem->child_memories[i]) {
            fprintf(stderr, "ERROR: Child %d memory not allocated\n", i);
            return 0;
        }
        
        if (mem->child_sizes[i] != WORKER_MEMORY_SIZE) {
            fprintf(stderr, "ERROR: Invalid child %d size=%zu (expected %d)\n",
                    i, mem->child_sizes[i], WORKER_MEMORY_SIZE);
            return 0;
        }
        
        if (!mem->shared_boundaries[i]) {
            fprintf(stderr, "ERROR: Boundary %d not allocated\n", i);
            return 0;
        }
        
        if (mem->boundary_sizes[i] != SHARED_BOUNDARY_SIZE) {
            fprintf(stderr, "ERROR: Invalid boundary %d size=%zu (expected %d)\n",
                    i, mem->boundary_sizes[i], SHARED_BOUNDARY_SIZE);
            return 0;
        }
    }
    
    // Check that unused children are NULL
    for (int i = mem->num_children; i < MAX_CHILDREN; i++) {
        if (mem->child_memories[i] != NULL) {
            fprintf(stderr, "ERROR: Unused child %d has allocated memory\n", i);
            return 0;
        }
        if (mem->shared_boundaries[i] != NULL) {
            fprintf(stderr, "ERROR: Unused child %d has allocated boundary\n", i);
            return 0;
        }
    }
    
    return 1;
}

/**
 * Print hierarchical memory information
 */
void hierarchical_memory_print(const HierarchicalMemory* mem) {
    if (!mem) {
        printf("NULL hierarchical memory\n");
        return;
    }
    
    printf("Hierarchical Memory:\n");
    printf("  Parent Sphere ID: %d\n", mem->parent_sphere_id);
    printf("  Parent Memory: %p (size=%zu)\n", mem->parent_memory, mem->parent_size);
    printf("  Number of Children: %d\n", mem->num_children);
    
    for (int i = 0; i < mem->num_children; i++) {
        printf("  Child %d (ID=%d):\n", i, mem->child_sphere_ids[i]);
        printf("    Memory: %p (size=%zu)\n", 
               mem->child_memories[i], mem->child_sizes[i]);
        printf("    Boundary: %p (size=%zu)\n",
               mem->shared_boundaries[i], mem->boundary_sizes[i]);
    }
}