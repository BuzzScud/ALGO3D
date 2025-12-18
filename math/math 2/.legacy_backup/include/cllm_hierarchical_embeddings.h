/**
 * @file cllm_hierarchical_embeddings.h
 * @brief Hierarchical Memory Structure for CrystallineAbacus Embeddings
 * 
 * This module implements a hierarchical memory architecture for embeddings
 * using CrystallineAbacus with sphere packing and 12-fold symmetry.
 * 
 * Key Features:
 * - Shared master abacus (READ_ONLY tier)
 * - Thread-local views (COPY_ON_WRITE tier)
 * - Kissing sphere boundaries (shared memory)
 * - Lock-free reads, minimal write contention
 * - Optimal cache locality through geometric packing
 * 
 * Architecture:
 * - Tokens mapped to spheres using clock lattice geometry
 * - Each sphere has 12 neighbors (kissing spheres)
 * - Boundary buffers for cross-sphere communication
 * - Synchronization at epoch boundaries
 */

#ifndef CLLM_HIERARCHICAL_EMBEDDINGS_H
#define CLLM_HIERARCHICAL_EMBEDDINGS_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include "cllm_abacus_matrix.h"
#include "clock_lattice.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// CONSTANTS
// ============================================================================

#define HEMB_MAX_SPHERES 144           // Maximum number of spheres (12^2)
#define HEMB_NEIGHBORS_PER_SPHERE 12   // Kissing sphere neighbors
#define HEMB_DEFAULT_BOUNDARY_SIZE 1024 // Default boundary buffer size (tokens)

// ============================================================================
// MEMORY TIER TYPES
// ============================================================================

/**
 * Memory tier for hierarchical access
 */
typedef enum {
    HEMB_TIER_READ_ONLY,      // Shared master abacus (lock-free reads)
    HEMB_TIER_COPY_ON_WRITE,  // Thread-local modifications
    HEMB_TIER_LOCKED_WRITE    // Critical updates (gradient accumulation)
} HEmbMemoryTier;

// ============================================================================
// SPHERE BOUNDARY STRUCTURE
// ============================================================================

/**
 * Boundary buffer between two kissing spheres
 * Used for cross-sphere gradient communication
 */
typedef struct {
    uint32_t sphere_a;              // First sphere ID
    uint32_t sphere_b;              // Second sphere ID
    
    // Gradient accumulation buffer
    AbacusMatrix* gradient_buffer;  // Accumulated gradients
    uint32_t* token_ids;            // Token IDs in buffer
    uint32_t num_tokens;            // Number of tokens in buffer
    uint32_t capacity;              // Buffer capacity
    
    // Synchronization
    pthread_spinlock_t lock;        // Spinlock for atomic updates
    uint64_t last_sync_epoch;       // Last synchronization epoch
    
    // Statistics
    uint64_t total_updates;         // Total gradient updates
    uint64_t total_syncs;           // Total synchronizations
    
} SphereBoundary;

// ============================================================================
// SPHERE LOCAL VIEW STRUCTURE
// ============================================================================

/**
 * Thread-local view of embeddings for one sphere
 * Implements COPY_ON_WRITE tier
 */
typedef struct {
    uint32_t sphere_id;             // Sphere ID
    
    // Local modifications
    AbacusMatrix* local_embeddings; // Local copy of embeddings
    bool has_modifications;         // Dirty flag
    uint64_t modification_epoch;    // When modifications started
    
    // Token ownership
    uint32_t* owned_tokens;         // Token IDs owned by this sphere
    uint32_t num_owned_tokens;      // Number of owned tokens
    
    // Cache
    double* cache;                  // Double cache for frequently accessed
    uint32_t* cached_token_ids;     // Token IDs in cache
    uint32_t cache_size;            // Current cache size
    uint32_t cache_capacity;        // Maximum cache size
    
    // Statistics
    uint64_t cache_hits;            // Cache hit count
    uint64_t cache_misses;          // Cache miss count
    uint64_t local_reads;           // Reads from local view
    uint64_t local_writes;          // Writes to local view
    
} SphereLocalView;

// ============================================================================
// HIERARCHICAL EMBEDDINGS STRUCTURE
// ============================================================================

/**
 * Hierarchical embeddings with sphere-based memory organization
 */
typedef struct {
    // Dimensions
    uint32_t vocab_size;            // Total vocabulary size
    uint32_t embedding_dim;         // Embedding dimension
    uint32_t num_spheres;           // Number of spheres (threads)
    
    // Master abacus (READ_ONLY tier)
    AbacusMatrix* master_embeddings; // Shared master [vocab_size × embedding_dim]
    pthread_rwlock_t master_lock;    // Read-write lock for master
    
    // Token-to-sphere mapping
    uint32_t* token_to_sphere_map;   // [vocab_size] → sphere_id
    uint32_t tokens_per_sphere;      // Average tokens per sphere
    
    // Sphere local views (COPY_ON_WRITE tier)
    SphereLocalView* sphere_views;   // [num_spheres] local views
    
    // Kissing sphere boundaries (LOCKED_WRITE tier)
    SphereBoundary* boundaries;      // [num_spheres × 12] boundary buffers
    uint32_t num_boundaries;         // Total number of boundaries
    
    // Neighbor relationships
    uint32_t* sphere_neighbors;      // [num_spheres × 12] neighbor IDs
    
    // Synchronization
    uint64_t sync_epoch;             // Current synchronization epoch
    uint64_t sync_frequency;         // Sync every N updates
    uint64_t updates_since_sync;     // Updates since last sync
    
    // Configuration
    uint32_t boundary_buffer_size;   // Size of boundary buffers
    uint32_t cache_size_per_sphere;  // Cache size per sphere
    bool auto_sync;                  // Automatic synchronization
    
    // Statistics
    uint64_t total_lookups;          // Total embedding lookups
    uint64_t total_updates;          // Total gradient updates
    uint64_t total_syncs;            // Total synchronizations
    uint64_t master_reads;           // Reads from master
    uint64_t local_reads;            // Reads from local views
    uint64_t boundary_writes;        // Writes to boundaries
    
} HierarchicalEmbeddings;

// ============================================================================
// INITIALIZATION & CLEANUP
// ============================================================================

/**
 * Create hierarchical embeddings structure
 * 
 * @param vocab_size Vocabulary size
 * @param embedding_dim Embedding dimension
 * @param num_spheres Number of spheres (threads)
 * @param base Abacus base (e.g., 60 for Babylonian)
 * @param precision Abacus precision
 * @return Initialized structure, or NULL on error
 */
HierarchicalEmbeddings* hemb_create(
    uint32_t vocab_size,
    uint32_t embedding_dim,
    uint32_t num_spheres,
    uint32_t base,
    uint32_t precision
);

/**
 * Free hierarchical embeddings structure
 * 
 * @param hemb Structure to free
 */
void hemb_free(HierarchicalEmbeddings* hemb);

// ============================================================================
// TOKEN-TO-SPHERE MAPPING
// ============================================================================

/**
 * Map token to sphere using clock lattice geometry
 * 
 * Uses Babylonian clock structure to deterministically assign
 * tokens to spheres with 12-fold symmetry.
 * 
 * @param token_id Token ID
 * @param num_spheres Number of spheres
 * @return Sphere ID (0 to num_spheres-1)
 */
uint32_t hemb_map_token_to_sphere(uint32_t token_id, uint32_t num_spheres);

/**
 * Initialize token-to-sphere mapping
 * 
 * @param hemb Hierarchical embeddings
 * @return 0 on success, -1 on error
 */
int hemb_init_token_mapping(HierarchicalEmbeddings* hemb);

// ============================================================================
// SPHERE NEIGHBOR OPERATIONS
// ============================================================================

/**
 * Find neighbor sphere using kissing sphere geometry
 * 
 * @param sphere_id Reference sphere ID
 * @param neighbor_idx Neighbor index (0-11)
 * @param num_spheres Total number of spheres
 * @return Neighbor sphere ID
 */
uint32_t hemb_find_neighbor(
    uint32_t sphere_id,
    uint32_t neighbor_idx,
    uint32_t num_spheres
);

/**
 * Initialize sphere neighbor relationships
 * 
 * @param hemb Hierarchical embeddings
 * @return 0 on success, -1 on error
 */
int hemb_init_neighbors(HierarchicalEmbeddings* hemb);

/**
 * Get boundary buffer between two spheres
 * 
 * @param hemb Hierarchical embeddings
 * @param sphere_a First sphere ID
 * @param sphere_b Second sphere ID
 * @return Boundary buffer, or NULL if not neighbors
 */
SphereBoundary* hemb_get_boundary(
    HierarchicalEmbeddings* hemb,
    uint32_t sphere_a,
    uint32_t sphere_b
);

// ============================================================================
// EMBEDDING OPERATIONS
// ============================================================================

/**
 * Lookup embedding (forward pass)
 * 
 * Reads from appropriate tier based on ownership and modifications.
 * Lock-free for reads from master tier.
 * 
 * @param hemb Hierarchical embeddings
 * @param token_id Token ID
 * @param thread_id Thread/sphere ID
 * @param output Output buffer [embedding_dim]
 * @return 0 on success, -1 on error
 */
int hemb_lookup_embedding(
    HierarchicalEmbeddings* hemb,
    uint32_t token_id,
    uint32_t thread_id,
    double* output
);

/**
 * Update embedding gradient (backward pass)
 * 
 * Writes to appropriate tier based on ownership.
 * Uses local view for owned tokens, boundary buffer for others.
 * 
 * @param hemb Hierarchical embeddings
 * @param token_id Token ID
 * @param thread_id Thread/sphere ID
 * @param gradient Gradient vector [embedding_dim]
 * @param learning_rate Learning rate
 * @return 0 on success, -1 on error
 */
int hemb_update_gradient(
    HierarchicalEmbeddings* hemb,
    uint32_t token_id,
    uint32_t thread_id,
    const double* gradient,
    double learning_rate
);

/**
 * Batch lookup embeddings
 * 
 * @param hemb Hierarchical embeddings
 * @param token_ids Token IDs [num_tokens]
 * @param num_tokens Number of tokens
 * @param thread_id Thread/sphere ID
 * @param output Output buffer [num_tokens × embedding_dim]
 * @return 0 on success, -1 on error
 */
int hemb_lookup_batch(
    HierarchicalEmbeddings* hemb,
    const uint32_t* token_ids,
    uint32_t num_tokens,
    uint32_t thread_id,
    double* output
);

// ============================================================================
// SYNCHRONIZATION
// ============================================================================

/**
 * Synchronize all tiers
 * 
 * Merges boundary buffers → local views → master abacus
 * 
 * @param hemb Hierarchical embeddings
 * @return 0 on success, -1 on error
 */
int hemb_synchronize(HierarchicalEmbeddings* hemb);

/**
 * Synchronize specific sphere
 * 
 * @param hemb Hierarchical embeddings
 * @param sphere_id Sphere ID
 * @return 0 on success, -1 on error
 */
int hemb_synchronize_sphere(HierarchicalEmbeddings* hemb, uint32_t sphere_id);

/**
 * Check if synchronization is needed
 * 
 * @param hemb Hierarchical embeddings
 * @return true if sync needed, false otherwise
 */
bool hemb_needs_sync(HierarchicalEmbeddings* hemb);

// ============================================================================
// INITIALIZATION FROM DOUBLES
// ============================================================================

/**
 * Initialize master embeddings from double array
 * 
 * @param hemb Hierarchical embeddings
 * @param embeddings Double array [vocab_size × embedding_dim]
 * @return 0 on success, -1 on error
 */
int hemb_init_from_doubles(
    HierarchicalEmbeddings* hemb,
    const double* embeddings
);

/**
 * Export master embeddings to double array
 * 
 * @param hemb Hierarchical embeddings
 * @param output Output buffer [vocab_size × embedding_dim]
 * @return 0 on success, -1 on error
 */
int hemb_export_to_doubles(
    HierarchicalEmbeddings* hemb,
    double* output
);

// ============================================================================
// STATISTICS & DEBUGGING
// ============================================================================

/**
 * Print statistics
 * 
 * @param hemb Hierarchical embeddings
 */
void hemb_print_stats(HierarchicalEmbeddings* hemb);

/**
 * Get memory usage
 * 
 * @param hemb Hierarchical embeddings
 * @return Total memory usage in bytes
 */
size_t hemb_memory_usage(HierarchicalEmbeddings* hemb);

/**
 * Validate structure integrity
 * 
 * @param hemb Hierarchical embeddings
 * @return 0 if valid, -1 if invalid
 */
int hemb_validate(HierarchicalEmbeddings* hemb);

#ifdef __cplusplus
}
#endif

#endif // CLLM_HIERARCHICAL_EMBEDDINGS_H