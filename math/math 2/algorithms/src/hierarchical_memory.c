/**
 * @file hierarchical_memory.c
 * @brief Hierarchical Memory System Implementation
 * 
 * Implements dynamic, scalable memory system based on Platonic Solid principles.
 * Integrates with abacus for geometric positioning and rainbow table for O(1) lookup.
 */

#include "../include/hierarchical_memory.h"
#include "../../math/include/math.h"
#include "math/abacus.h"
#include "math/rainbow.h"
#include "math/clock.h"
#include "math/transcendental.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * Get current time in nanoseconds
 * Reserved for future performance monitoring
 */
__attribute__((unused))
static uint64_t get_time_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

/**
 * Allocate cache-aligned memory
 */
static void* aligned_alloc_cache(size_t size) {
    void* ptr = NULL;
    if (posix_memalign(&ptr, HIERARCHICAL_CACHE_LINE, size) != 0) {
        return NULL;
    }
    memset(ptr, 0, size);
    return ptr;
}

/**
 * Compute GCD (for coprime checking)
 */
static uint64_t gcd(uint64_t a, uint64_t b) {
    while (b != 0) {
        uint64_t temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

/**
 * Check if two numbers are coprime
 */
static bool are_coprime(uint64_t a, uint64_t b) {
    return gcd(a, b) == 1;
}

/**
 * Get prime number at index (for geometric positioning)
 */
static uint32_t get_prime(uint32_t index) {
    uint32_t primes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 
                         53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 
                         109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173};
    return primes[index % 40];
}

// ============================================================================
// LIFECYCLE
// ============================================================================

HierarchicalMemoryBlock* hierarchical_memory_create(
    size_t total_size,
    uint32_t initial_segments,
    uint32_t initial_dimensions,
    int owner_id,
    int hierarchy_level
) {
    // Enforce minimums
    if (initial_segments < HIERARCHICAL_MIN_SEGMENTS) {
        initial_segments = HIERARCHICAL_MIN_SEGMENTS;
    }
    if (initial_dimensions < HIERARCHICAL_MIN_DIMENSIONS) {
        initial_dimensions = HIERARCHICAL_MIN_DIMENSIONS;
    }
    
    // Allocate block
    HierarchicalMemoryBlock* block = calloc(1, sizeof(HierarchicalMemoryBlock));
    if (!block) {
        fprintf(stderr, "ERROR: Failed to allocate memory block\n");
        return NULL;
    }
    
    // Generate unique ID
    snprintf(block->block_id, sizeof(block->block_id),
             "HMEM_%d_L%d_S%u_D%u_%lu",
             owner_id, hierarchy_level, initial_segments, initial_dimensions,
             (unsigned long)time(NULL));
    
    block->owner_id = owner_id;
    block->hierarchy_level = hierarchy_level;
    
    // Set segment parameters
    block->num_segments = initial_segments;
    block->max_segments = initial_segments * 8;  // Can scale 8x
    block->min_segments = HIERARCHICAL_MIN_SEGMENTS;
    
    // Set dimension parameters
    block->num_dimensions = initial_dimensions;
    block->max_dimensions = initial_dimensions * 4;  // Can scale 4x
    block->min_dimensions = HIERARCHICAL_MIN_DIMENSIONS;
    
    // Calculate segment size
    block->total_size = total_size;
    block->segment_size = total_size / initial_segments;
    
    // Allocate segments
    block->segments = calloc(block->num_segments, sizeof(HierarchicalSegment));
    if (!block->segments) {
        fprintf(stderr, "ERROR: Failed to allocate segments\n");
        hierarchical_memory_destroy(block);
        return NULL;
    }
    
    // Initialize each segment
    for (uint32_t i = 0; i < block->num_segments; i++) {
        HierarchicalSegment* seg = &block->segments[i];
        
        seg->segment_id = i;
        seg->symmetry_group = i % initial_segments;  // Map to symmetry group
        seg->type = (i == 0) ? SEGMENT_CONTROL : SEGMENT_WORKER;
        seg->size = block->segment_size;
        seg->capacity = block->segment_size;
        seg->num_dimensions = initial_dimensions;
        
        // Allocate segment data (cache-aligned)
        seg->data = aligned_alloc_cache(seg->capacity);
        if (!seg->data) {
            fprintf(stderr, "ERROR: Failed to allocate segment %u data\n", i);
            hierarchical_memory_destroy(block);
            return NULL;
        }
        
        // Allocate position vector
        seg->position = calloc(initial_dimensions, sizeof(double));
        if (!seg->position) {
            fprintf(stderr, "ERROR: Failed to allocate segment %u position\n", i);
            hierarchical_memory_destroy(block);
            return NULL;
        }
        
        // Initialize atomic counters
        atomic_init(&seg->read_count, 0);
        atomic_init(&seg->write_count, 0);
        
        // NUMA node (simplified - would use numa_node_of_cpu in production)
        seg->numa_node = i % 2;  // Alternate between NUMA nodes
    }
    
    // Allocate boundaries (max possible: n*(n-1)/2)
    block->max_boundaries = (block->num_segments * (block->num_segments - 1)) / 2;
    block->boundaries = calloc(block->max_boundaries, sizeof(KissingBoundary*));
    block->num_boundaries = 0;
    
    // Allocate coprime matrix
    block->coprime_matrix = calloc(block->num_dimensions, sizeof(uint64_t*));
    for (uint32_t i = 0; i < block->num_dimensions; i++) {
        block->coprime_matrix[i] = calloc(block->num_dimensions, sizeof(uint64_t));
    }
    
    // Initialize statistics
    atomic_init(&block->total_reads, 0);
    atomic_init(&block->total_writes, 0);
    atomic_init(&block->total_reads, 0);
    atomic_init(&block->cache_misses, 0);
    
    // Initialize scaling history
    block->num_segment_scales = 0;
    block->num_dimension_scales = 0;
    block->segment_scale_history = calloc(10, sizeof(uint32_t));
    block->dimension_scale_history = calloc(10, sizeof(uint32_t));
    
    // Initialize mutex
    pthread_mutex_init(&block->block_mutex, NULL);
    
    // Compute geometric positions
    hierarchical_memory_organize_geometrically(block);
    
    // Compute coprime matrix
    hierarchical_memory_compute_coprime_matrix(block);
    
    printf("✓ Created Hierarchical Memory Block: %s\n", block->block_id);
    printf("  Segments: %u, Dimensions: %u, Total Size: %zu bytes\n",
           block->num_segments, block->num_dimensions, block->total_size);
    printf("  Segment Size: %zu bytes, Cache-aligned: %d bytes\n",
           block->segment_size, HIERARCHICAL_CACHE_LINE);
    
    return block;
}

void hierarchical_memory_destroy(HierarchicalMemoryBlock* block) {
    if (!block) return;
    
    // Free segments
    if (block->segments) {
        for (uint32_t i = 0; i < block->num_segments; i++) {
            free(block->segments[i].data);
            free(block->segments[i].position);
        }
        free(block->segments);
    }
    
    // Free boundaries
    if (block->boundaries) {
        for (uint32_t i = 0; i < block->num_boundaries; i++) {
            hierarchical_boundary_destroy(block->boundaries[i]);
        }
        free(block->boundaries);
    }
    
    // Free coprime matrix
    if (block->coprime_matrix) {
        for (uint32_t i = 0; i < block->num_dimensions; i++) {
            free(block->coprime_matrix[i]);
        }
        free(block->coprime_matrix);
    }
    
    // Free scaling history
    free(block->segment_scale_history);
    free(block->dimension_scale_history);
    
    // Destroy mutex
    pthread_mutex_destroy(&block->block_mutex);
    
    free(block);
}

// ============================================================================
// SEGMENT ACCESS
// ============================================================================

HierarchicalSegment* hierarchical_memory_get_segment(
    HierarchicalMemoryBlock* block,
    uint32_t segment_id
) {
    if (!block || segment_id >= block->num_segments) {
        return NULL;
    }
    return &block->segments[segment_id];
}

HierarchicalSegment* hierarchical_memory_get_segment_by_symmetry(
    HierarchicalMemoryBlock* block,
    uint32_t symmetry_group
) {
    if (!block) return NULL;
    
    // Find first segment with matching symmetry group
    for (uint32_t i = 0; i < block->num_segments; i++) {
        if (block->segments[i].symmetry_group == symmetry_group) {
            return &block->segments[i];
        }
    }
    
    return NULL;
}

size_t hierarchical_segment_read(
    HierarchicalSegment* segment,
    size_t offset,
    void* buffer,
    size_t size
) {
    if (!segment || !buffer || offset + size > segment->size) {
        return 0;
    }
    
    // Copy data
    memcpy(buffer, (char*)segment->data + offset, size);
    
    // Update statistics
    atomic_fetch_add(&segment->read_count, 1);
    
    return size;
}

size_t hierarchical_segment_write(
    HierarchicalSegment* segment,
    size_t offset,
    const void* data,
    size_t size
) {
    if (!segment || !data || offset + size > segment->size) {
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

KissingBoundary* hierarchical_boundary_create(
    HierarchicalMemoryBlock* block,
    uint32_t segment_a,
    uint32_t segment_b,
    size_t boundary_size
) {
    if (!block || segment_a >= block->num_segments || segment_b >= block->num_segments) {
        return NULL;
    }
    
    if (segment_a == segment_b) {
        fprintf(stderr, "ERROR: Cannot create boundary between same segment\n");
        return NULL;
    }
    
    // Check if we have space for more boundaries
    if (block->num_boundaries >= block->max_boundaries) {
        fprintf(stderr, "ERROR: Maximum boundaries reached\n");
        return NULL;
    }
    
    // Allocate boundary
    KissingBoundary* boundary = aligned_alloc_cache(sizeof(KissingBoundary));
    if (!boundary) {
        return NULL;
    }
    
    // Allocate boundary memory (cache-aligned)
    boundary->boundary_memory = aligned_alloc_cache(boundary_size);
    if (!boundary->boundary_memory) {
        free(boundary);
        return NULL;
    }
    
    boundary->boundary_size = boundary_size;
    boundary->segment_a = segment_a;
    boundary->segment_b = segment_b;
    
    // Initialize lock
    atomic_flag_clear(&boundary->lock);
    
    // Initialize version
    atomic_init(&boundary->version, 0);
    
    // Initialize access counters
    atomic_init(&boundary->accesses_a, 0);
    atomic_init(&boundary->accesses_b, 0);
    
    // Add to block
    pthread_mutex_lock(&block->block_mutex);
    block->boundaries[block->num_boundaries++] = boundary;
    pthread_mutex_unlock(&block->block_mutex);
    
    return boundary;
}

void hierarchical_boundary_destroy(KissingBoundary* boundary) {
    if (!boundary) return;
    free(boundary->boundary_memory);
    free(boundary);
}

const void* hierarchical_boundary_read(
    KissingBoundary* boundary,
    uint32_t segment_id
) {
    if (!boundary) return NULL;
    
    // Update access counter
    if (segment_id == boundary->segment_a) {
        atomic_fetch_add(&boundary->accesses_a, 1);
    } else if (segment_id == boundary->segment_b) {
        atomic_fetch_add(&boundary->accesses_b, 1);
    }
    
    return boundary->boundary_memory;
}

void* hierarchical_boundary_write(
    KissingBoundary* boundary,
    uint32_t segment_id
) {
    if (!boundary) return NULL;
    
    // Acquire spinlock
    while (atomic_flag_test_and_set(&boundary->lock)) {
        // Spin
    }
    
    // Update access counter
    if (segment_id == boundary->segment_a) {
        atomic_fetch_add(&boundary->accesses_a, 1);
    } else if (segment_id == boundary->segment_b) {
        atomic_fetch_add(&boundary->accesses_b, 1);
    }
    
    return boundary->boundary_memory;
}

void hierarchical_boundary_release(KissingBoundary* boundary) {
    if (!boundary) return;
    
    // Increment version
    atomic_fetch_add(&boundary->version, 1);
    
    // Release spinlock
    atomic_flag_clear(&boundary->lock);
}

// ============================================================================
// DYNAMIC SCALING
// ============================================================================

bool hierarchical_memory_scale_segments(
    HierarchicalMemoryBlock* block,
    uint32_t scale_factor
) {
    if (!block || scale_factor < 2) {
        return false;
    }
    
    pthread_mutex_lock(&block->block_mutex);
    
    uint32_t new_num_segments = block->num_segments * scale_factor;
    
    // Check if we can scale
    if (new_num_segments > block->max_segments) {
        fprintf(stderr, "ERROR: Cannot scale segments beyond maximum (%u)\n",
                block->max_segments);
        pthread_mutex_unlock(&block->block_mutex);
        return false;
    }
    
    printf("Scaling segments: %u → %u (factor: %u)\n",
           block->num_segments, new_num_segments, scale_factor);
    
    // Allocate new segment array
    HierarchicalSegment* new_segments = calloc(new_num_segments, sizeof(HierarchicalSegment));
    if (!new_segments) {
        pthread_mutex_unlock(&block->block_mutex);
        return false;
    }
    
    // Copy existing segments
    memcpy(new_segments, block->segments, block->num_segments * sizeof(HierarchicalSegment));
    
    // Create new segments using self-similar replication (like Platonic)
    for (uint32_t i = block->num_segments; i < new_num_segments; i++) {
        uint32_t source = i % block->num_segments;
        HierarchicalSegment* src = &block->segments[source];
        HierarchicalSegment* dst = &new_segments[i];
        
        dst->segment_id = i;
        dst->symmetry_group = i % block->num_segments;
        dst->type = SEGMENT_WORKER;
        dst->size = src->size;
        dst->capacity = src->capacity;
        dst->num_dimensions = src->num_dimensions;
        
        // Allocate data (cache-aligned)
        dst->data = aligned_alloc_cache(dst->capacity);
        if (!dst->data) {
            // Cleanup and fail
            for (uint32_t j = block->num_segments; j < i; j++) {
                free(new_segments[j].data);
                free(new_segments[j].position);
            }
            free(new_segments);
            pthread_mutex_unlock(&block->block_mutex);
            return false;
        }
        
        // Allocate position
        dst->position = calloc(dst->num_dimensions, sizeof(double));
        if (!dst->position) {
            free(dst->data);
            for (uint32_t j = block->num_segments; j < i; j++) {
                free(new_segments[j].data);
                free(new_segments[j].position);
            }
            free(new_segments);
            pthread_mutex_unlock(&block->block_mutex);
            return false;
        }
        
        // Copy position with small perturbation (self-similar replication)
        for (uint32_t d = 0; d < dst->num_dimensions; d++) {
            double base_value = src->position[d];
            double perturbation = 0.01 * math_sin(2.0 * M_PI * i / new_num_segments + d);
            dst->position[d] = base_value + perturbation;
        }
        
        // Initialize atomic counters
        atomic_init(&dst->read_count, 0);
        atomic_init(&dst->write_count, 0);
        
        dst->numa_node = i % 2;
    }
    
    // Replace old segments
    free(block->segments);
    block->segments = new_segments;
    block->num_segments = new_num_segments;
    
    // Record scaling event
    if (block->num_segment_scales < 10) {
        block->segment_scale_history[block->num_segment_scales++] = new_num_segments;
    }
    
    pthread_mutex_unlock(&block->block_mutex);
    
    printf("✓ Scaled segments to %u\n", new_num_segments);
    return true;
}

bool hierarchical_memory_scale_dimensions(
    HierarchicalMemoryBlock* block,
    uint32_t additional_dimensions
) {
    if (!block || additional_dimensions == 0) {
        return false;
    }
    
    pthread_mutex_lock(&block->block_mutex);
    
    uint32_t new_num_dimensions = block->num_dimensions + additional_dimensions;
    
    // Check if we can scale
    if (new_num_dimensions > block->max_dimensions) {
        fprintf(stderr, "ERROR: Cannot scale dimensions beyond maximum (%u)\n",
                block->max_dimensions);
        pthread_mutex_unlock(&block->block_mutex);
        return false;
    }
    
    printf("Scaling dimensions: %u → %u (+%u)\n",
           block->num_dimensions, new_num_dimensions, additional_dimensions);
    
    // Extend position vectors for all segments
    for (uint32_t i = 0; i < block->num_segments; i++) {
        HierarchicalSegment* seg = &block->segments[i];
        
        // Allocate new position vector
        double* new_position = calloc(new_num_dimensions, sizeof(double));
        if (!new_position) {
            pthread_mutex_unlock(&block->block_mutex);
            return false;
        }
        
        // Copy existing dimensions
        memcpy(new_position, seg->position, block->num_dimensions * sizeof(double));
        
        // Project into new dimensions using prime-based formula (like Platonic)
        for (uint32_t d = block->num_dimensions; d < new_num_dimensions; d++) {
            double sum = 0.0;
            for (uint32_t j = 0; j < block->num_dimensions; j++) {
                uint32_t prime = get_prime(d);
                sum += seg->position[j] * math_cos(2.0 * M_PI * prime * (d - block->num_dimensions) / new_num_dimensions);
            }
            new_position[d] = sum / math_sqrt((double)new_num_dimensions);
        }
        
        // Replace old position
        free(seg->position);
        seg->position = new_position;
        seg->num_dimensions = new_num_dimensions;
    }
    
    // Extend coprime matrix
    uint64_t** new_coprime_matrix = calloc(new_num_dimensions, sizeof(uint64_t*));
    for (uint32_t i = 0; i < new_num_dimensions; i++) {
        new_coprime_matrix[i] = calloc(new_num_dimensions, sizeof(uint64_t));
        
        // Copy existing values
        if (i < block->num_dimensions) {
            memcpy(new_coprime_matrix[i], block->coprime_matrix[i],
                   block->num_dimensions * sizeof(uint64_t));
        }
    }
    
    // Free old matrix
    for (uint32_t i = 0; i < block->num_dimensions; i++) {
        free(block->coprime_matrix[i]);
    }
    free(block->coprime_matrix);
    
    block->coprime_matrix = new_coprime_matrix;
    block->num_dimensions = new_num_dimensions;
    
    // Recompute coprime matrix
    hierarchical_memory_compute_coprime_matrix(block);
    
    // Record scaling event
    if (block->num_dimension_scales < 10) {
        block->dimension_scale_history[block->num_dimension_scales++] = new_num_dimensions;
    }
    
    pthread_mutex_unlock(&block->block_mutex);
    
    printf("✓ Scaled dimensions to %u\n", new_num_dimensions);
    return true;
}

bool hierarchical_memory_should_scale(
    const HierarchicalMemoryBlock* block,
    bool* out_scale_segments,
    bool* out_scale_dimensions
) {
    if (!block || !out_scale_segments || !out_scale_dimensions) {
        return false;
    }
    
    *out_scale_segments = false;
    *out_scale_dimensions = false;
    
    // Check segment utilization
    uint64_t total_reads = atomic_load(&block->total_reads);
    uint64_t total_writes = atomic_load(&block->total_writes);
    uint64_t total_accesses = total_reads + total_writes;
    
    // If high access rate, consider scaling segments
    if (total_accesses > block->num_segments * 10000) {
        *out_scale_segments = true;
    }
    
    // If many boundaries, consider scaling dimensions
    if (block->num_boundaries > block->num_segments * 2) {
        *out_scale_dimensions = true;
    }
    
    return *out_scale_segments || *out_scale_dimensions;
}

// ============================================================================
// GEOMETRIC ORGANIZATION
// ============================================================================

int hierarchical_memory_organize_geometrically(
    HierarchicalMemoryBlock* block
) {
    if (!block) return -1;
    
    // Compute geometric position for each segment
    for (uint32_t i = 0; i < block->num_segments; i++) {
        HierarchicalSegment* seg = &block->segments[i];
        
        if (hierarchical_memory_compute_position(
                seg->segment_id,
                seg->symmetry_group,
                block->num_dimensions,
                seg->position) != 0) {
            return -1;
        }
    }
    
    return 0;
}

int hierarchical_memory_compute_position(
    uint32_t segment_id,
    uint32_t symmetry_group,
    uint32_t num_dimensions,
    double* out_position
) {
    if (!out_position) return -1;
    
    // Use clock lattice principles for positioning
    // Each segment maps to a position on the clock
    
    // Base angle from symmetry group (12-fold symmetry)
    double base_angle = (2.0 * M_PI * symmetry_group) / 12.0;
    
    // Radius based on segment ID
    double radius = 1.0 + (segment_id / 12.0);
    
    // First 3 dimensions: standard 3D position
    out_position[0] = radius * math_cos(base_angle);
    out_position[1] = radius * math_sin(base_angle);
    out_position[2] = (double)segment_id / 100.0;
    
    // Higher dimensions: prime-based projection
    for (uint32_t d = 3; d < num_dimensions; d++) {
        uint32_t prime = get_prime(d);
        double sum = 0.0;
        
        sum += out_position[0] * math_cos(2.0 * M_PI * prime * d / num_dimensions);
        sum += out_position[1] * math_sin(2.0 * M_PI * prime * d / num_dimensions);
        sum += out_position[2] * math_cos(M_PI * prime * d / num_dimensions);
        
        out_position[d] = sum / math_sqrt((double)num_dimensions);
    }
    
    return 0;
}

// ============================================================================
// COPRIME RELATIONSHIPS
// ============================================================================

int hierarchical_memory_compute_coprime_matrix(
    HierarchicalMemoryBlock* block
) {
    if (!block || !block->coprime_matrix) return -1;
    
    // Compute coprime relationships between all dimensions
    for (uint32_t i = 0; i < block->num_dimensions; i++) {
        for (uint32_t j = 0; j < block->num_dimensions; j++) {
            if (i == j) {
                block->coprime_matrix[i][j] = 1;  // Same dimension
            } else {
                // Use prime-based coprime check
                uint32_t prime_i = get_prime(i);
                uint32_t prime_j = get_prime(j);
                block->coprime_matrix[i][j] = are_coprime(prime_i, prime_j) ? 1 : 0;
            }
        }
    }
    
    return 0;
}

bool hierarchical_memory_can_communicate(
    const HierarchicalMemoryBlock* block,
    uint32_t segment_a,
    uint32_t segment_b
) {
    if (!block || segment_a >= block->num_segments || segment_b >= block->num_segments) {
        return false;
    }
    
    // Get symmetry groups
    uint32_t sym_a = block->segments[segment_a].symmetry_group;
    uint32_t sym_b = block->segments[segment_b].symmetry_group;
    
    // Check coprime relationship
    if (sym_a < block->num_dimensions && sym_b < block->num_dimensions) {
        return block->coprime_matrix[sym_a][sym_b] == 1;
    }
    
    return false;
}

// ============================================================================
// STATISTICS
// ============================================================================

void hierarchical_memory_get_stats(
    const HierarchicalMemoryBlock* block,
    uint64_t* total_reads,
    uint64_t* total_writes,
    uint64_t* cache_hits,
    uint64_t* cache_misses
) {
    if (!block) return;
    
    if (total_reads) *total_reads = atomic_load(&block->total_reads);
    if (total_writes) *total_writes = atomic_load(&block->total_writes);
    if (cache_hits) *cache_hits = atomic_load(&block->cache_hits);
    if (cache_misses) *cache_misses = atomic_load(&block->cache_misses);
}

void hierarchical_memory_print_stats(const HierarchicalMemoryBlock* block) {
    if (!block) return;
    
    printf("\n=== Hierarchical Memory Statistics ===\n");
    printf("Block ID: %s\n", block->block_id);
    printf("Segments: %u (max: %u)\n", block->num_segments, block->max_segments);
    printf("Dimensions: %u (max: %u)\n", block->num_dimensions, block->max_dimensions);
    printf("Boundaries: %u\n", block->num_boundaries);
    printf("Total Size: %zu bytes\n", block->total_size);
    printf("Segment Size: %zu bytes\n", block->segment_size);
    
    uint64_t total_reads = atomic_load(&block->total_reads);
    uint64_t total_writes = atomic_load(&block->total_writes);
    uint64_t cache_hits = atomic_load(&block->cache_hits);
    uint64_t cache_misses = atomic_load(&block->cache_misses);
    
    printf("Total Reads: %lu\n", total_reads);
    printf("Total Writes: %lu\n", total_writes);
    printf("Cache Hits: %lu\n", cache_hits);
    printf("Cache Misses: %lu\n", cache_misses);
    
    if (cache_hits + cache_misses > 0) {
        double hit_rate = (double)cache_hits / (cache_hits + cache_misses);
        printf("Cache Hit Rate: %.2f%%\n", hit_rate * 100.0);
    }
    
    printf("Segment Scales: %u\n", block->num_segment_scales);
    printf("Dimension Scales: %u\n", block->num_dimension_scales);
    printf("=====================================\n\n");
}

void hierarchical_memory_reset_stats(HierarchicalMemoryBlock* block) {
    if (!block) return;
    
    atomic_store(&block->total_reads, 0);
    atomic_store(&block->total_writes, 0);
    atomic_store(&block->cache_hits, 0);
    atomic_store(&block->cache_misses, 0);
    
    // Reset segment counters
    for (uint32_t i = 0; i < block->num_segments; i++) {
        atomic_store(&block->segments[i].read_count, 0);
        atomic_store(&block->segments[i].write_count, 0);
    }
}

// ============================================================================
// UTILITIES
// ============================================================================

int hierarchical_memory_validate(const HierarchicalMemoryBlock* block) {
    if (!block) return 0;
    
    // Check segments
    if (!block->segments || block->num_segments < block->min_segments) {
        fprintf(stderr, "VALIDATION ERROR: Invalid segments\n");
        return 0;
    }
    
    // Check dimensions
    if (block->num_dimensions < block->min_dimensions) {
        fprintf(stderr, "VALIDATION ERROR: Invalid dimensions\n");
        return 0;
    }
    
    // Check each segment
    for (uint32_t i = 0; i < block->num_segments; i++) {
        HierarchicalSegment* seg = &block->segments[i];
        
        if (!seg->data || !seg->position) {
            fprintf(stderr, "VALIDATION ERROR: Segment %u has NULL data/position\n", i);
            return 0;
        }
        
        if (seg->segment_id != i) {
            fprintf(stderr, "VALIDATION ERROR: Segment %u has wrong ID\n", i);
            return 0;
        }
    }
    
    return 1;
}

void hierarchical_memory_print(const HierarchicalMemoryBlock* block) {
    if (!block) return;
    
    printf("\n=== Hierarchical Memory Block ===\n");
    printf("ID: %s\n", block->block_id);
    printf("Owner: %d, Level: %d\n", block->owner_id, block->hierarchy_level);
    printf("Segments: %u/%u (min: %u)\n", 
           block->num_segments, block->max_segments, block->min_segments);
    printf("Dimensions: %u/%u (min: %u)\n",
           block->num_dimensions, block->max_dimensions, block->min_dimensions);
    printf("Boundaries: %u/%u\n", block->num_boundaries, block->max_boundaries);
    printf("Total Size: %zu bytes\n", block->total_size);
    printf("Segment Size: %zu bytes\n", block->segment_size);
    printf("=================================\n\n");
}

void hierarchical_memory_get_scaling_history(
    const HierarchicalMemoryBlock* block,
    uint32_t** out_segment_scales,
    uint32_t* out_num_segment_scales,
    uint32_t** out_dimension_scales,
    uint32_t* out_num_dimension_scales
) {
    if (!block) return;
    
    if (out_segment_scales) *out_segment_scales = block->segment_scale_history;
    if (out_num_segment_scales) *out_num_segment_scales = block->num_segment_scales;
    if (out_dimension_scales) *out_dimension_scales = block->dimension_scale_history;
    if (out_num_dimension_scales) *out_num_dimension_scales = block->num_dimension_scales;
}