#ifndef CLLM_HIERARCHICAL_MEMORY_H
#define CLLM_HIERARCHICAL_MEMORY_H

#include <stdint.h>
#include <stddef.h>
#include <stdatomic.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Hierarchical Memory System
 * 
 * Manages parent-child memory relationships in the recursive sphere hierarchy.
 * Provides shared boundary regions for efficient parent-child communication.
 */

// Memory sizes
#define CONTROL_MEMORY_SIZE (1024 * 1024)      // 1 MB for control threads
#define WORKER_MEMORY_SIZE (512 * 1024)        // 512 KB for worker threads
#define SHARED_BOUNDARY_SIZE (64 * 1024)       // 64 KB shared boundary

// Maximum children per parent (12-fold symmetry)
#define MAX_CHILDREN 12

/**
 * Hierarchical Memory Structure
 * 
 * Manages memory for a parent and its children with shared boundaries.
 */
typedef struct {
    // Parent memory
    void* parent_memory;                    // Parent's control memory
    size_t parent_size;                     // Size of parent memory
    
    // Children memory
    void* child_memories[MAX_CHILDREN];     // Children's work memory
    size_t child_sizes[MAX_CHILDREN];       // Size of each child memory
    int num_children;                       // Number of active children
    
    // Shared boundaries
    void* shared_boundaries[MAX_CHILDREN];  // Shared memory with each child
    size_t boundary_sizes[MAX_CHILDREN];    // Size of each boundary
    
    // Ownership
    int parent_sphere_id;                   // Parent sphere ID
    int child_sphere_ids[MAX_CHILDREN];     // Child sphere IDs
    
    // Statistics
    atomic_uint_fast64_t parent_accesses;   // Parent memory accesses
    atomic_uint_fast64_t child_accesses[MAX_CHILDREN];    // Child memory accesses
    atomic_uint_fast64_t boundary_accesses[MAX_CHILDREN]; // Boundary accesses
    
} HierarchicalMemory;

// ============================================================================
// LIFECYCLE
// ============================================================================

/**
 * Allocate hierarchical memory for parent and children
 * 
 * @param parent_sphere_id Parent sphere ID
 * @param num_children Number of children (1-12)
 * @return Allocated hierarchical memory, or NULL on error
 */
HierarchicalMemory* hierarchical_memory_create(
    int parent_sphere_id,
    int num_children
);

/**
 * Destroy hierarchical memory
 * 
 * @param mem Hierarchical memory to destroy
 */
void hierarchical_memory_destroy(HierarchicalMemory* mem);

// ============================================================================
// MEMORY ACCESS
// ============================================================================

/**
 * Access parent memory
 * 
 * @param mem Hierarchical memory
 * @param offset Offset in parent memory
 * @param buffer Buffer to read/write
 * @param size Size to read/write
 * @param write 1 for write, 0 for read
 * @return Number of bytes accessed, or 0 on error
 */
size_t hierarchical_memory_access_parent(
    HierarchicalMemory* mem,
    size_t offset,
    void* buffer,
    size_t size,
    int write
);

/**
 * Access child memory
 * 
 * @param mem Hierarchical memory
 * @param child_index Child index (0-11)
 * @param offset Offset in child memory
 * @param buffer Buffer to read/write
 * @param size Size to read/write
 * @param write 1 for write, 0 for read
 * @return Number of bytes accessed, or 0 on error
 */
size_t hierarchical_memory_access_child(
    HierarchicalMemory* mem,
    int child_index,
    size_t offset,
    void* buffer,
    size_t size,
    int write
);

/**
 * Access shared boundary
 * 
 * @param mem Hierarchical memory
 * @param child_index Child index (0-11)
 * @param offset Offset in boundary
 * @param buffer Buffer to read/write
 * @param size Size to read/write
 * @param write 1 for write, 0 for read
 * @return Number of bytes accessed, or 0 on error
 */
size_t hierarchical_memory_access_boundary(
    HierarchicalMemory* mem,
    int child_index,
    size_t offset,
    void* buffer,
    size_t size,
    int write
);

// ============================================================================
// CHILD MANAGEMENT
// ============================================================================

/**
 * Add child to hierarchical memory
 * 
 * @param mem Hierarchical memory
 * @param child_index Child index (0-11)
 * @param child_sphere_id Child sphere ID
 * @return 1 on success, 0 on error
 */
int hierarchical_memory_add_child(
    HierarchicalMemory* mem,
    int child_index,
    int child_sphere_id
);

/**
 * Remove child from hierarchical memory
 * 
 * @param mem Hierarchical memory
 * @param child_index Child index (0-11)
 * @return 1 on success, 0 on error
 */
int hierarchical_memory_remove_child(
    HierarchicalMemory* mem,
    int child_index
);

/**
 * Get child sphere ID
 * 
 * @param mem Hierarchical memory
 * @param child_index Child index (0-11)
 * @return Child sphere ID, or -1 if no child
 */
int hierarchical_memory_get_child_id(
    const HierarchicalMemory* mem,
    int child_index
);

// ============================================================================
// STATISTICS
// ============================================================================

/**
 * Get memory access statistics
 * 
 * @param mem Hierarchical memory
 * @param parent_accesses Output: parent memory accesses
 * @param total_child_accesses Output: total child memory accesses
 * @param total_boundary_accesses Output: total boundary accesses
 */
void hierarchical_memory_get_stats(
    const HierarchicalMemory* mem,
    uint64_t* parent_accesses,
    uint64_t* total_child_accesses,
    uint64_t* total_boundary_accesses
);

/**
 * Print memory statistics
 * 
 * @param mem Hierarchical memory
 */
void hierarchical_memory_print_stats(const HierarchicalMemory* mem);

/**
 * Reset memory statistics
 * 
 * @param mem Hierarchical memory
 */
void hierarchical_memory_reset_stats(HierarchicalMemory* mem);

// ============================================================================
// UTILITIES
// ============================================================================

/**
 * Validate hierarchical memory
 * 
 * @param mem Hierarchical memory
 * @return 1 if valid, 0 if errors found
 */
int hierarchical_memory_validate(const HierarchicalMemory* mem);

/**
 * Print hierarchical memory information
 * 
 * @param mem Hierarchical memory
 */
void hierarchical_memory_print(const HierarchicalMemory* mem);

#ifdef __cplusplus
}
#endif

#endif // CLLM_HIERARCHICAL_MEMORY_H
