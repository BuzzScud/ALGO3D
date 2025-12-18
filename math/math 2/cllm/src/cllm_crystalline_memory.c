#include "ai/cllm_crystalline_memory.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ============================================================================
// MEMORY BLOCK LIFECYCLE
// ============================================================================

CrystallineMemoryBlock* crystalline_memory_create(
    size_t total_size,
    int owner_sphere_id,
    int hierarchy_level
) {
    if (total_size == 0) {
        fprintf(stderr, "ERROR: Cannot create memory block with zero size\n");
        return NULL;
    }
    
    // Allocate block structure
    CrystallineMemoryBlock* block = calloc(1, sizeof(CrystallineMemoryBlock));
    if (!block) {
        fprintf(stderr, "ERROR: Failed to allocate memory block\n");
        return NULL;
    }
    
    // Calculate segment size (divide total by 12)
    block->segment_size = (total_size + NUM_SYMMETRY_GROUPS - 1) / NUM_SYMMETRY_GROUPS;
    
    // Round up to cache line boundary
    if (block->segment_size % CRYSTALLINE_CACHE_LINE != 0) {
        block->segment_size = ((block->segment_size / CRYSTALLINE_CACHE_LINE) + 1) * CRYSTALLINE_CACHE_LINE;
    }
    
    block->total_size = block->segment_size * NUM_SYMMETRY_GROUPS;
    block->owner_sphere_id = owner_sphere_id;
    block->hierarchy_level = hierarchy_level;
    
    // Allocate segments
    for (int i = 0; i < NUM_SYMMETRY_GROUPS; i++) {
        // Allocate cache-aligned memory
        block->segments[i].data = aligned_alloc(CRYSTALLINE_CACHE_LINE, block->segment_size);
        if (!block->segments[i].data) {
            fprintf(stderr, "ERROR: Failed to allocate segment %d\n", i);
            crystalline_memory_destroy(block);
            return NULL;
        }
        
        // Initialize segment
        memset(block->segments[i].data, 0, block->segment_size);
        block->segments[i].size = block->segment_size;
        block->segments[i].symmetry_group = i;
        block->segments[i].type = SEGMENT_WORKER;  // Default to worker
        block->segments[i].numa_node = -1;  // Unknown NUMA node
        
        atomic_init(&block->segments[i].read_count, 0);
        atomic_init(&block->segments[i].write_count, 0);
    }
    
    // Initialize shared memory pointers to NULL
    block->parent_shared = NULL;
    for (int i = 0; i < NUM_SYMMETRY_GROUPS; i++) {
        block->child_shared[i] = NULL;
    }
    
    // Initialize statistics
    atomic_init(&block->total_reads, 0);
    atomic_init(&block->total_writes, 0);
    atomic_init(&block->cache_hits, 0);
    atomic_init(&block->cache_misses, 0);
    
    return block;
}

void crystalline_memory_destroy(CrystallineMemoryBlock* block) {
    if (!block) return;
    
    // Free segments
    for (int i = 0; i < NUM_SYMMETRY_GROUPS; i++) {
        if (block->segments[i].data) {
            free(block->segments[i].data);
            block->segments[i].data = NULL;
        }
    }
    
    // NOTE: Shared memory regions are NOT freed here to avoid double-free
    // The shared memory is owned by the parent and should be freed by the parent
    // When destroying a parent, we free child_shared[]
    // When destroying a child, we just clear the parent_shared pointer
    
    // Only free child shared memory (parent owns these)
    for (int i = 0; i < NUM_SYMMETRY_GROUPS; i++) {
        if (block->child_shared[i]) {
            shared_memory_free(block->child_shared[i]);
            block->child_shared[i] = NULL;
        }
    }
    
    // Do NOT free parent_shared - it's owned by the parent
    block->parent_shared = NULL;
    
    // Free block
    free(block);
}

// ============================================================================
// SEGMENT ACCESS
// ============================================================================

CrystallineSegment* crystalline_memory_get_segment(
    CrystallineMemoryBlock* block,
    uint32_t symmetry_group
) {
    if (!block || symmetry_group >= NUM_SYMMETRY_GROUPS) {
        return NULL;
    }
    
    return &block->segments[symmetry_group];
}

size_t crystalline_segment_read(
    CrystallineSegment* segment,
    size_t offset,
    void* buffer,
    size_t size
) {
    if (!segment || !buffer || !segment->data) {
        return 0;
    }
    
    // Check bounds
    if (offset + size > segment->size) {
        size = segment->size - offset;
    }
    
    if (size == 0) {
        return 0;
    }
    
    // Copy data
    memcpy(buffer, (char*)segment->data + offset, size);
    
    // Update statistics
    atomic_fetch_add(&segment->read_count, 1);
    
    return size;
}

size_t crystalline_segment_write(
    CrystallineSegment* segment,
    size_t offset,
    const void* data,
    size_t size
) {
    if (!segment || !data || !segment->data) {
        return 0;
    }
    
    // Check bounds
    if (offset + size > segment->size) {
        size = segment->size - offset;
    }
    
    if (size == 0) {
        return 0;
    }
    
    // Copy data
    memcpy((char*)segment->data + offset, data, size);
    
    // Update statistics
    atomic_fetch_add(&segment->write_count, 1);
    
    return size;
}

// ============================================================================
// KISSING BOUNDARIES
// ============================================================================

KissingBoundary* crystalline_boundary_create(
    CrystallineMemoryBlock* block,
    uint32_t segment_a,
    uint32_t segment_b,
    size_t boundary_size
) {
    if (!block || segment_a >= NUM_SYMMETRY_GROUPS || segment_b >= NUM_SYMMETRY_GROUPS) {
        fprintf(stderr, "ERROR: Invalid segments for boundary\n");
        return NULL;
    }
    
    if (segment_a == segment_b) {
        fprintf(stderr, "ERROR: Cannot create boundary between same segment\n");
        return NULL;
    }
    
    // Allocate boundary structure
    KissingBoundary* boundary = calloc(1, sizeof(KissingBoundary));
    if (!boundary) {
        fprintf(stderr, "ERROR: Failed to allocate boundary\n");
        return NULL;
    }
    
    // Allocate boundary memory (cache-aligned)
    boundary->boundary_memory = aligned_alloc(CRYSTALLINE_CACHE_LINE, boundary_size);
    if (!boundary->boundary_memory) {
        fprintf(stderr, "ERROR: Failed to allocate boundary memory\n");
        free(boundary);
        return NULL;
    }
    
    memset(boundary->boundary_memory, 0, boundary_size);
    boundary->boundary_size = boundary_size;
    boundary->segment_a = segment_a;
    boundary->segment_b = segment_b;
    
    // Initialize lock
    atomic_flag_clear(&boundary->lock);
    atomic_init(&boundary->version, 0);
    
    // Initialize statistics
    atomic_init(&boundary->accesses_a, 0);
    atomic_init(&boundary->accesses_b, 0);
    
    return boundary;
}

void crystalline_boundary_destroy(KissingBoundary* boundary) {
    if (!boundary) return;
    
    if (boundary->boundary_memory) {
        free(boundary->boundary_memory);
    }
    
    free(boundary);
}

const void* crystalline_boundary_read(
    KissingBoundary* boundary,
    uint32_t segment_id
) {
    if (!boundary) return NULL;
    
    // Update access statistics
    if (segment_id == boundary->segment_a) {
        atomic_fetch_add(&boundary->accesses_a, 1);
    } else if (segment_id == boundary->segment_b) {
        atomic_fetch_add(&boundary->accesses_b, 1);
    }
    
    // Lock-free read
    return boundary->boundary_memory;
}

void* crystalline_boundary_write(
    KissingBoundary* boundary,
    uint32_t segment_id
) {
    if (!boundary) return NULL;
    
    // Acquire spinlock
    while (atomic_flag_test_and_set_explicit(&boundary->lock, memory_order_acquire)) {
        // Spin
    }
    
    // Update access statistics
    if (segment_id == boundary->segment_a) {
        atomic_fetch_add(&boundary->accesses_a, 1);
    } else if (segment_id == boundary->segment_b) {
        atomic_fetch_add(&boundary->accesses_b, 1);
    }
    
    // Increment version
    atomic_fetch_add(&boundary->version, 1);
    
    return boundary->boundary_memory;
}

void crystalline_boundary_release(KissingBoundary* boundary) {
    if (!boundary) return;
    
    // Release spinlock
    atomic_flag_clear_explicit(&boundary->lock, memory_order_release);
}

// ============================================================================
// HIERARCHICAL MEMORY
// ============================================================================

int crystalline_memory_link_parent_child(
    CrystallineMemoryBlock* parent,
    CrystallineMemoryBlock* child,
    uint32_t child_symmetry_group,
    size_t shared_size
) {
    if (!parent || !child || child_symmetry_group >= NUM_SYMMETRY_GROUPS) {
        return 0;
    }
    
    // Create shared memory region
    SharedMemoryRegion* shared = shared_memory_create(shared_size, SHARED_LOCKED_WRITE);
    if (!shared) {
        fprintf(stderr, "ERROR: Failed to create shared memory region\n");
        return 0;
    }
    
    // Link to parent's child array
    // NOTE: Do NOT free existing shared memory here - it may be referenced by child
    parent->child_shared[child_symmetry_group] = shared;
    
    // Link to child's parent pointer
    // NOTE: Do NOT free existing shared memory here - it may be referenced by parent
    child->parent_shared = shared;
    
    return 1;
}

SharedMemoryRegion* crystalline_memory_get_parent_shared(
    CrystallineMemoryBlock* block
) {
    if (!block) return NULL;
    return block->parent_shared;
}

SharedMemoryRegion* crystalline_memory_get_child_shared(
    CrystallineMemoryBlock* block,
    uint32_t child_symmetry_group
) {
    if (!block || child_symmetry_group >= NUM_SYMMETRY_GROUPS) {
        return NULL;
    }
    
    return block->child_shared[child_symmetry_group];
}

// ============================================================================
// STATISTICS
// ============================================================================

void crystalline_memory_get_stats(
    const CrystallineMemoryBlock* block,
    uint64_t* total_reads,
    uint64_t* total_writes,
    uint64_t* cache_hits,
    uint64_t* cache_misses
) {
    if (!block) return;
    
    if (total_reads) {
        *total_reads = atomic_load(&block->total_reads);
    }
    if (total_writes) {
        *total_writes = atomic_load(&block->total_writes);
    }
    if (cache_hits) {
        *cache_hits = atomic_load(&block->cache_hits);
    }
    if (cache_misses) {
        *cache_misses = atomic_load(&block->cache_misses);
    }
}

void crystalline_memory_print_stats(const CrystallineMemoryBlock* block) {
    if (!block) return;
    
    printf("=== Crystalline Memory Statistics ===\n");
    printf("Owner Sphere: %d\n", block->owner_sphere_id);
    printf("Hierarchy Level: %d\n", block->hierarchy_level);
    printf("Total Size: %zu bytes\n", block->total_size);
    printf("Segment Size: %zu bytes\n", block->segment_size);
    printf("Total Reads: %lu\n", (unsigned long)atomic_load(&block->total_reads));
    printf("Total Writes: %lu\n", (unsigned long)atomic_load(&block->total_writes));
    printf("Cache Hits: %lu\n", (unsigned long)atomic_load(&block->cache_hits));
    printf("Cache Misses: %lu\n", (unsigned long)atomic_load(&block->cache_misses));
    
    printf("\nPer-Segment Statistics:\n");
    for (int i = 0; i < NUM_SYMMETRY_GROUPS; i++) {
        printf("  Segment %d: reads=%lu, writes=%lu\n",
               i,
               (unsigned long)atomic_load(&block->segments[i].read_count),
               (unsigned long)atomic_load(&block->segments[i].write_count));
    }
    
    printf("====================================\n");
}

void crystalline_memory_reset_stats(CrystallineMemoryBlock* block) {
    if (!block) return;
    
    atomic_store(&block->total_reads, 0);
    atomic_store(&block->total_writes, 0);
    atomic_store(&block->cache_hits, 0);
    atomic_store(&block->cache_misses, 0);
    
    for (int i = 0; i < NUM_SYMMETRY_GROUPS; i++) {
        atomic_store(&block->segments[i].read_count, 0);
        atomic_store(&block->segments[i].write_count, 0);
    }
}

// ============================================================================
// UTILITIES
// ============================================================================

int crystalline_memory_validate(const CrystallineMemoryBlock* block) {
    if (!block) return 0;
    
    int valid = 1;
    
    // Check total size
    if (block->total_size == 0) {
        fprintf(stderr, "ERROR: Total size is zero\n");
        valid = 0;
    }
    
    // Check segment size
    if (block->segment_size == 0) {
        fprintf(stderr, "ERROR: Segment size is zero\n");
        valid = 0;
    }
    
    // Check segments
    for (int i = 0; i < NUM_SYMMETRY_GROUPS; i++) {
        if (!block->segments[i].data) {
            fprintf(stderr, "ERROR: Segment %d has NULL data\n", i);
            valid = 0;
        }
        
        if (block->segments[i].symmetry_group != (uint32_t)i) {
            fprintf(stderr, "ERROR: Segment %d has wrong symmetry group %u\n",
                    i, block->segments[i].symmetry_group);
            valid = 0;
        }
    }
    
    return valid;
}

void crystalline_memory_print(const CrystallineMemoryBlock* block) {
    if (!block) {
        printf("NULL memory block\n");
        return;
    }
    
    printf("=== Crystalline Memory Block ===\n");
    printf("Owner Sphere: %d\n", block->owner_sphere_id);
    printf("Hierarchy Level: %d\n", block->hierarchy_level);
    printf("Total Size: %zu bytes (%.2f MB)\n", 
           block->total_size, block->total_size / (1024.0 * 1024.0));
    printf("Segment Size: %zu bytes (%.2f KB)\n",
           block->segment_size, block->segment_size / 1024.0);
    printf("Number of Segments: %d\n", NUM_SYMMETRY_GROUPS);
    
    printf("\nSegments:\n");
    for (int i = 0; i < NUM_SYMMETRY_GROUPS; i++) {
        printf("  [%d] Group %u, Type %d, Size %zu bytes\n",
               i,
               block->segments[i].symmetry_group,
               block->segments[i].type,
               block->segments[i].size);
    }
    
    printf("\nShared Memory:\n");
    printf("  Parent Shared: %s\n", block->parent_shared ? "Yes" : "No");
    int child_count = 0;
    for (int i = 0; i < NUM_SYMMETRY_GROUPS; i++) {
        if (block->child_shared[i]) child_count++;
    }
    printf("  Child Shared: %d/%d\n", child_count, NUM_SYMMETRY_GROUPS);
    
    printf("================================\n");
}