/**
 * @file cllm_hierarchical_embeddings.c
 * @brief Implementation of Hierarchical Memory Structure for Embeddings
 */

#include "ai/cllm_hierarchical_embeddings.h"
#include "math/constants.h"
#include "ai/cllm_abacus_matrix.h"
#include "math/constants.h"
#include "math/clock.h"
#include "math/constants.h"
#include "math/arithmetic.h"
#include "math/constants.h"
#include "math/transcendental.h"
#include "math/constants.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// TOKEN-TO-SPHERE MAPPING
// ============================================================================

/**
 * Map token to sphere using clock lattice geometry
 */
uint32_t hemb_map_token_to_sphere(uint32_t token_id, uint32_t num_spheres) {
    if (num_spheres == 0) return 0;
    
    // Use clock position to determine sphere
    ClockPosition pos;
    clock_map_index_to_position(token_id + 1, &pos);
    
    // Map angle to sphere (12-fold symmetry)
    // Angle is in [0, 2π), divide into num_spheres sectors
    double sector_size = 2.0 * MATH_PI / num_spheres;
    uint32_t sphere_id = (uint32_t)(pos.angle / sector_size);
    
    return sphere_id % num_spheres;
}

/**
 * Initialize token-to-sphere mapping
 */
int hemb_init_token_mapping(HierarchicalEmbeddings* hemb) {
    if (!hemb || !hemb->token_to_sphere_map) return -1;
    
    printf("Initializing token-to-sphere mapping...\n");
    
    // Map each token to a sphere using clock geometry
    for (uint32_t token = 0; token < hemb->vocab_size; token++) {
        hemb->token_to_sphere_map[token] = 
            hemb_map_token_to_sphere(token, hemb->num_spheres);
    }
    
    // Count tokens per sphere
    uint32_t* counts = (uint32_t*)calloc(hemb->num_spheres, sizeof(uint32_t));
    if (!counts) return -1;
    
    for (uint32_t token = 0; token < hemb->vocab_size; token++) {
        counts[hemb->token_to_sphere_map[token]]++;
    }
    
    // Print distribution
    printf("  Token distribution across spheres:\n");
    for (uint32_t s = 0; s < hemb->num_spheres; s++) {
        printf("    Sphere %2u: %5u tokens\n", s, counts[s]);
    }
    
    // Store owned tokens in each sphere view
    for (uint32_t s = 0; s < hemb->num_spheres; s++) {
        SphereLocalView* view = &hemb->sphere_views[s];
        view->num_owned_tokens = counts[s];
        view->owned_tokens = (uint32_t*)malloc(counts[s] * sizeof(uint32_t));
        if (!view->owned_tokens) {
            free(counts);
            return -1;
        }
    }
    
    // Fill owned token arrays
    uint32_t* indices = (uint32_t*)calloc(hemb->num_spheres, sizeof(uint32_t));
    if (!indices) {
        free(counts);
        return -1;
    }
    
    for (uint32_t token = 0; token < hemb->vocab_size; token++) {
        uint32_t sphere = hemb->token_to_sphere_map[token];
        SphereLocalView* view = &hemb->sphere_views[sphere];
        view->owned_tokens[indices[sphere]++] = token;
    }
    
    free(counts);
    free(indices);
    
    printf("✓ Token-to-sphere mapping initialized\n");
    return 0;
}

// ============================================================================
// SPHERE NEIGHBOR OPERATIONS
// ============================================================================

/**
 * Find neighbor sphere using kissing sphere geometry
 */
uint32_t hemb_find_neighbor(
    uint32_t sphere_id,
    uint32_t neighbor_idx,
    uint32_t num_spheres
) {
    if (neighbor_idx >= HEMB_NEIGHBORS_PER_SPHERE) return sphere_id;
    if (num_spheres <= 1) return sphere_id;
    
    // Use clock-based neighbor finding
    // Each sphere has 12 neighbors arranged in clock positions
    // Neighbor i is at position: (sphere_id + offset[i]) % num_spheres
    
    // Offsets based on 12-fold symmetry
    static const int32_t offsets[12] = {
        1, -1,           // Adjacent (3 o'clock, 9 o'clock)
        2, -2,           // Skip one (6 o'clock, 12 o'clock)
        3, -3,           // Skip two
        4, -4,           // Skip three
        5, -5,           // Skip four
        6, -6            // Opposite
    };
    
    int32_t neighbor = (int32_t)sphere_id + offsets[neighbor_idx];
    
    // Handle wraparound
    while (neighbor < 0) neighbor += num_spheres;
    while (neighbor >= (int32_t)num_spheres) neighbor -= num_spheres;
    
    return (uint32_t)neighbor;
}

/**
 * Initialize sphere neighbor relationships
 */
int hemb_init_neighbors(HierarchicalEmbeddings* hemb) {
    if (!hemb || !hemb->sphere_neighbors) return -1;
    
    printf("Initializing sphere neighbor relationships...\n");
    
    // For each sphere, find its 12 neighbors
    for (uint32_t s = 0; s < hemb->num_spheres; s++) {
        for (uint32_t n = 0; n < HEMB_NEIGHBORS_PER_SPHERE; n++) {
            uint32_t neighbor = hemb_find_neighbor(s, n, hemb->num_spheres);
            hemb->sphere_neighbors[s * HEMB_NEIGHBORS_PER_SPHERE + n] = neighbor;
        }
    }
    
    printf("✓ Sphere neighbors initialized\n");
    return 0;
}

/**
 * Get boundary buffer between two spheres
 */
SphereBoundary* hemb_get_boundary(
    HierarchicalEmbeddings* hemb,
    uint32_t sphere_a,
    uint32_t sphere_b
) {
    if (!hemb || sphere_a >= hemb->num_spheres || sphere_b >= hemb->num_spheres) {
        return NULL;
    }
    
    // Check if they are neighbors
    uint32_t* neighbors_a = &hemb->sphere_neighbors[sphere_a * HEMB_NEIGHBORS_PER_SPHERE];
    
    for (uint32_t i = 0; i < HEMB_NEIGHBORS_PER_SPHERE; i++) {
        if (neighbors_a[i] == sphere_b) {
            // Found neighbor relationship
            uint32_t boundary_idx = sphere_a * HEMB_NEIGHBORS_PER_SPHERE + i;
            return &hemb->boundaries[boundary_idx];
        }
    }
    
    return NULL; // Not neighbors
}

// ============================================================================
// INITIALIZATION & CLEANUP
// ============================================================================

/**
 * Create hierarchical embeddings structure
 */
HierarchicalEmbeddings* hemb_create(
    uint32_t vocab_size,
    uint32_t embedding_dim,
    uint32_t num_spheres,
    uint32_t base,
    uint32_t precision
) {
    if (vocab_size == 0 || embedding_dim == 0 || num_spheres == 0) {
        fprintf(stderr, "ERROR: Invalid parameters to hemb_create\n");
        return NULL;
    }
    
    if (num_spheres > HEMB_MAX_SPHERES) {
        fprintf(stderr, "ERROR: num_spheres (%u) exceeds maximum (%u)\n",
                num_spheres, HEMB_MAX_SPHERES);
        return NULL;
    }
    
    printf("Creating hierarchical embeddings:\n");
    printf("  Vocab size: %u\n", vocab_size);
    printf("  Embedding dim: %u\n", embedding_dim);
    printf("  Num spheres: %u\n", num_spheres);
    printf("  Abacus base: %u, precision: %u\n", base, precision);
    
    HierarchicalEmbeddings* hemb = (HierarchicalEmbeddings*)calloc(
        1, sizeof(HierarchicalEmbeddings)
    );
    if (!hemb) {
        fprintf(stderr, "ERROR: Failed to allocate HierarchicalEmbeddings\n");
        return NULL;
    }
    
    // Set dimensions
    hemb->vocab_size = vocab_size;
    hemb->embedding_dim = embedding_dim;
    hemb->num_spheres = num_spheres;
    hemb->tokens_per_sphere = (vocab_size + num_spheres - 1) / num_spheres;
    
    // Create master abacus (READ_ONLY tier)
    printf("  Creating master abacus...\n");
    hemb->master_embeddings = abacus_matrix_create(
        vocab_size, embedding_dim, base, precision
    );
    if (!hemb->master_embeddings) {
        fprintf(stderr, "ERROR: Failed to create master embeddings\n");
        hemb_free(hemb);
        return NULL;
    }
    
    // Initialize master lock
    if (pthread_rwlock_init(&hemb->master_lock, NULL) != 0) {
        fprintf(stderr, "ERROR: Failed to initialize master lock\n");
        hemb_free(hemb);
        return NULL;
    }
    
    // Allocate token-to-sphere mapping
    hemb->token_to_sphere_map = (uint32_t*)malloc(vocab_size * sizeof(uint32_t));
    if (!hemb->token_to_sphere_map) {
        fprintf(stderr, "ERROR: Failed to allocate token mapping\n");
        hemb_free(hemb);
        return NULL;
    }
    
    // Allocate sphere local views (COPY_ON_WRITE tier)
    printf("  Creating sphere local views...\n");
    hemb->sphere_views = (SphereLocalView*)calloc(
        num_spheres, sizeof(SphereLocalView)
    );
    if (!hemb->sphere_views) {
        fprintf(stderr, "ERROR: Failed to allocate sphere views\n");
        hemb_free(hemb);
        return NULL;
    }
    
    // Initialize each sphere view
    for (uint32_t s = 0; s < num_spheres; s++) {
        SphereLocalView* view = &hemb->sphere_views[s];
        view->sphere_id = s;
        view->has_modifications = false;
        view->modification_epoch = 0;
        view->local_embeddings = NULL; // Created on first write
        
        // Allocate cache
        view->cache_capacity = 100; // Cache 100 embeddings per sphere
        view->cache_size = 0;
        view->cache = (double*)malloc(
            view->cache_capacity * embedding_dim * sizeof(double)
        );
        view->cached_token_ids = (uint32_t*)malloc(
            view->cache_capacity * sizeof(uint32_t)
        );
        
        if (!view->cache || !view->cached_token_ids) {
            fprintf(stderr, "ERROR: Failed to allocate cache for sphere %u\n", s);
            hemb_free(hemb);
            return NULL;
        }
    }
    
    // Allocate sphere neighbors
    hemb->sphere_neighbors = (uint32_t*)malloc(
        num_spheres * HEMB_NEIGHBORS_PER_SPHERE * sizeof(uint32_t)
    );
    if (!hemb->sphere_neighbors) {
        fprintf(stderr, "ERROR: Failed to allocate sphere neighbors\n");
        hemb_free(hemb);
        return NULL;
    }
    
    // Allocate boundary buffers (LOCKED_WRITE tier)
    printf("  Creating boundary buffers...\n");
    hemb->num_boundaries = num_spheres * HEMB_NEIGHBORS_PER_SPHERE;
    hemb->boundaries = (SphereBoundary*)calloc(
        hemb->num_boundaries, sizeof(SphereBoundary)
    );
    if (!hemb->boundaries) {
        fprintf(stderr, "ERROR: Failed to allocate boundaries\n");
        hemb_free(hemb);
        return NULL;
    }
    
    // Initialize each boundary
    hemb->boundary_buffer_size = HEMB_DEFAULT_BOUNDARY_SIZE;
    for (uint32_t i = 0; i < hemb->num_boundaries; i++) {
        SphereBoundary* boundary = &hemb->boundaries[i];
        
        // Create gradient buffer
        boundary->gradient_buffer = abacus_matrix_create(
            hemb->boundary_buffer_size, embedding_dim, base, precision
        );
        if (!boundary->gradient_buffer) {
            fprintf(stderr, "ERROR: Failed to create boundary buffer %u\n", i);
            hemb_free(hemb);
            return NULL;
        }
        
        // Allocate token ID array
        boundary->token_ids = (uint32_t*)malloc(
            hemb->boundary_buffer_size * sizeof(uint32_t)
        );
        if (!boundary->token_ids) {
            fprintf(stderr, "ERROR: Failed to allocate boundary token IDs %u\n", i);
            hemb_free(hemb);
            return NULL;
        }
        
        boundary->num_tokens = 0;
        boundary->capacity = hemb->boundary_buffer_size;
        boundary->last_sync_epoch = 0;
        
        // Initialize spinlock
        if (pthread_spin_init(&boundary->lock, PTHREAD_PROCESS_PRIVATE) != 0) {
            fprintf(stderr, "ERROR: Failed to initialize boundary lock %u\n", i);
            hemb_free(hemb);
            return NULL;
        }
    }
    
    // Set configuration
    hemb->sync_epoch = 0;
    hemb->sync_frequency = 1000; // Sync every 1000 updates
    hemb->updates_since_sync = 0;
    hemb->auto_sync = true;
    hemb->cache_size_per_sphere = 100;
    
    // Initialize mappings
    if (hemb_init_token_mapping(hemb) != 0) {
        fprintf(stderr, "ERROR: Failed to initialize token mapping\n");
        hemb_free(hemb);
        return NULL;
    }
    
    if (hemb_init_neighbors(hemb) != 0) {
        fprintf(stderr, "ERROR: Failed to initialize neighbors\n");
        hemb_free(hemb);
        return NULL;
    }
    
    printf("✓ Hierarchical embeddings created successfully\n");
    return hemb;
}

/**
 * Free hierarchical embeddings structure
 */
void hemb_free(HierarchicalEmbeddings* hemb) {
    if (!hemb) return;
    
    // Free master embeddings
    if (hemb->master_embeddings) {
        abacus_matrix_free(hemb->master_embeddings);
    }
    
    // Destroy master lock
    pthread_rwlock_destroy(&hemb->master_lock);
    
    // Free token mapping
    free(hemb->token_to_sphere_map);
    
    // Free sphere views
    if (hemb->sphere_views) {
        for (uint32_t s = 0; s < hemb->num_spheres; s++) {
            SphereLocalView* view = &hemb->sphere_views[s];
            if (view->local_embeddings) {
                abacus_matrix_free(view->local_embeddings);
            }
            free(view->owned_tokens);
            free(view->cache);
            free(view->cached_token_ids);
        }
        free(hemb->sphere_views);
    }
    
    // Free sphere neighbors
    free(hemb->sphere_neighbors);
    
    // Free boundaries
    if (hemb->boundaries) {
        for (uint32_t i = 0; i < hemb->num_boundaries; i++) {
            SphereBoundary* boundary = &hemb->boundaries[i];
            if (boundary->gradient_buffer) {
                abacus_matrix_free(boundary->gradient_buffer);
            }
            free(boundary->token_ids);
            pthread_spin_destroy(&boundary->lock);
        }
        free(hemb->boundaries);
    }
    
    free(hemb);
}

// ============================================================================
// EMBEDDING OPERATIONS
// ============================================================================

/**
 * Lookup embedding (forward pass)
 */
int hemb_lookup_embedding(
    HierarchicalEmbeddings* hemb,
    uint32_t token_id,
    uint32_t thread_id,
    double* output
) {
    if (!hemb || !output || token_id >= hemb->vocab_size) {
        return -1;
    }
    
    if (thread_id >= hemb->num_spheres) {
        thread_id = 0; // Default to sphere 0
    }
    
    hemb->total_lookups++;
    
    // Determine which sphere owns this token
    uint32_t owner_sphere = hemb->token_to_sphere_map[token_id];
    SphereLocalView* view = &hemb->sphere_views[thread_id];
    
    // Check cache first
    for (uint32_t i = 0; i < view->cache_size; i++) {
        if (view->cached_token_ids[i] == token_id) {
            // Cache hit!
            memcpy(output, 
                   &view->cache[i * hemb->embedding_dim],
                   hemb->embedding_dim * sizeof(double));
            view->cache_hits++;
            return 0;
        }
    }
    view->cache_misses++;
    
    // Check if thread owns this token and has local modifications
    if (thread_id == owner_sphere && view->has_modifications) {
        // Read from local view (COPY_ON_WRITE tier)
        MathError err = abacus_matrix_get_row_as_doubles(
            view->local_embeddings,
            token_id,
            output
        );
        if (err != MATH_SUCCESS) {
            return -1;
        }
        view->local_reads++;
        hemb->local_reads++;
    } else {
        // Read from master (READ_ONLY tier, lock-free)
        pthread_rwlock_rdlock(&hemb->master_lock);
        MathError err = abacus_matrix_get_row_as_doubles(
            hemb->master_embeddings,
            token_id,
            output
        );
        pthread_rwlock_unlock(&hemb->master_lock);
        
        if (err != MATH_SUCCESS) {
            return -1;
        }
        hemb->master_reads++;
    }
    
    // Add to cache if space available
    if (view->cache_size < view->cache_capacity) {
        memcpy(&view->cache[view->cache_size * hemb->embedding_dim],
               output,
               hemb->embedding_dim * sizeof(double));
        view->cached_token_ids[view->cache_size] = token_id;
        view->cache_size++;
    }
    
    return 0;
}

/**
 * Update embedding gradient (backward pass)
 */
int hemb_update_gradient(
    HierarchicalEmbeddings* hemb,
    uint32_t token_id,
    uint32_t thread_id,
    const double* gradient,
    double learning_rate
) {
    if (!hemb || !gradient || token_id >= hemb->vocab_size) {
        return -1;
    }
    
    if (thread_id >= hemb->num_spheres) {
        thread_id = 0;
    }
    
    hemb->total_updates++;
    hemb->updates_since_sync++;
    
    // Determine which sphere owns this token
    uint32_t owner_sphere = hemb->token_to_sphere_map[token_id];
    
    if (thread_id == owner_sphere) {
        // This thread owns the token - update local view
        SphereLocalView* view = &hemb->sphere_views[thread_id];
        
        // Create local view if needed (COPY_ON_WRITE)
        if (!view->has_modifications) {
            // Copy from master to local
            view->local_embeddings = abacus_matrix_create(
                hemb->vocab_size,
                hemb->embedding_dim,
                hemb->master_embeddings->base,
                hemb->master_embeddings->precision
            );
            if (!view->local_embeddings) {
                return -1;
            }
            
            // Copy master to local (only owned tokens for efficiency)
            pthread_rwlock_rdlock(&hemb->master_lock);
            for (uint32_t i = 0; i < view->num_owned_tokens; i++) {
                uint32_t token = view->owned_tokens[i];
                double* temp = (double*)malloc(hemb->embedding_dim * sizeof(double));
                if (temp) {
                    abacus_matrix_get_row_as_doubles(hemb->master_embeddings, token, temp);
                    abacus_matrix_set_row_from_doubles(view->local_embeddings, token, temp);
                    free(temp);
                }
            }
            pthread_rwlock_unlock(&hemb->master_lock);
            
            view->has_modifications = true;
            view->modification_epoch = hemb->sync_epoch;
        }
        
        // Apply gradient to local view
        double* current = (double*)malloc(hemb->embedding_dim * sizeof(double));
        if (!current) return -1;
        
        abacus_matrix_get_row_as_doubles(view->local_embeddings, token_id, current);
        
        for (uint32_t i = 0; i < hemb->embedding_dim; i++) {
            current[i] -= learning_rate * gradient[i];
        }
        
        abacus_matrix_set_row_from_doubles(view->local_embeddings, token_id, current);
        free(current);
        
        view->local_writes++;
        
        // Invalidate cache entry
        for (uint32_t i = 0; i < view->cache_size; i++) {
            if (view->cached_token_ids[i] == token_id) {
                // Remove from cache by swapping with last
                if (i < view->cache_size - 1) {
                    memcpy(&view->cache[i * hemb->embedding_dim],
                           &view->cache[(view->cache_size - 1) * hemb->embedding_dim],
                           hemb->embedding_dim * sizeof(double));
                    view->cached_token_ids[i] = view->cached_token_ids[view->cache_size - 1];
                }
                view->cache_size--;
                break;
            }
        }
        
    } else {
        // Token owned by different sphere - use boundary buffer
        SphereBoundary* boundary = hemb_get_boundary(hemb, thread_id, owner_sphere);
        if (!boundary) {
            // Not neighbors - this shouldn't happen often
            // For now, skip the update
            return 0;
        }
        
        // Add gradient to boundary buffer (atomic)
        pthread_spin_lock(&boundary->lock);
        
        // Check if token already in buffer
        int32_t token_idx = -1;
        for (uint32_t i = 0; i < boundary->num_tokens; i++) {
            if (boundary->token_ids[i] == token_id) {
                token_idx = i;
                break;
            }
        }
        
        if (token_idx < 0) {
            // Add new token to buffer
            if (boundary->num_tokens < boundary->capacity) {
                token_idx = boundary->num_tokens;
                boundary->token_ids[token_idx] = token_id;
                boundary->num_tokens++;
                
                // Initialize gradient to zero
                double* zeros = (double*)calloc(hemb->embedding_dim, sizeof(double));
                if (zeros) {
                    abacus_matrix_set_row_from_doubles(
                        boundary->gradient_buffer, token_idx, zeros
                    );
                    free(zeros);
                }
            } else {
                // Buffer full - skip update for now
                pthread_spin_unlock(&boundary->lock);
                return 0;
            }
        }
        
        // Accumulate gradient
        double* current_grad = (double*)malloc(hemb->embedding_dim * sizeof(double));
        if (current_grad) {
            abacus_matrix_get_row_as_doubles(
                boundary->gradient_buffer, token_idx, current_grad
            );
            
            for (uint32_t i = 0; i < hemb->embedding_dim; i++) {
                current_grad[i] += learning_rate * gradient[i];
            }
            
            abacus_matrix_set_row_from_doubles(
                boundary->gradient_buffer, token_idx, current_grad
            );
            free(current_grad);
        }
        
        boundary->total_updates++;
        pthread_spin_unlock(&boundary->lock);
        
        hemb->boundary_writes++;
    }
    
    // Check if auto-sync is needed
    if (hemb->auto_sync && hemb_needs_sync(hemb)) {
        return hemb_synchronize(hemb);
    }
    
    return 0;
}

// ============================================================================
// SYNCHRONIZATION
// ============================================================================

/**
 * Check if synchronization is needed
 */
bool hemb_needs_sync(HierarchicalEmbeddings* hemb) {
    if (!hemb) return false;
    return hemb->updates_since_sync >= hemb->sync_frequency;
}

/**
 * Synchronize specific sphere
 */
int hemb_synchronize_sphere(HierarchicalEmbeddings* hemb, uint32_t sphere_id) {
    if (!hemb || sphere_id >= hemb->num_spheres) return -1;
    
    SphereLocalView* view = &hemb->sphere_views[sphere_id];
    
    // Skip if no modifications
    if (!view->has_modifications) return 0;
    
    // Merge local view back to master
    pthread_rwlock_wrlock(&hemb->master_lock);
    
    for (uint32_t i = 0; i < view->num_owned_tokens; i++) {
        uint32_t token = view->owned_tokens[i];
        double* temp = (double*)malloc(hemb->embedding_dim * sizeof(double));
        if (temp) {
            abacus_matrix_get_row_as_doubles(view->local_embeddings, token, temp);
            abacus_matrix_set_row_from_doubles(hemb->master_embeddings, token, temp);
            free(temp);
        }
    }
    
    pthread_rwlock_unlock(&hemb->master_lock);
    
    // Clear modifications flag
    view->has_modifications = false;
    
    return 0;
}

/**
 * Synchronize all tiers
 */
int hemb_synchronize(HierarchicalEmbeddings* hemb) {
    if (!hemb) return -1;
    
    printf("Synchronizing hierarchical embeddings (epoch %lu)...\n", hemb->sync_epoch);
    
    // Step 1: Merge boundary buffers into local views
    for (uint32_t i = 0; i < hemb->num_boundaries; i++) {
        SphereBoundary* boundary = &hemb->boundaries[i];
        
        if (boundary->num_tokens == 0) continue;
        
        pthread_spin_lock(&boundary->lock);
        
        // Process each gradient in boundary
        for (uint32_t j = 0; j < boundary->num_tokens; j++) {
            uint32_t token_id = boundary->token_ids[j];
            uint32_t owner_sphere = hemb->token_to_sphere_map[token_id];
            SphereLocalView* view = &hemb->sphere_views[owner_sphere];
            
            // Ensure local view exists
            if (!view->has_modifications) {
                // Create local view
                view->local_embeddings = abacus_matrix_create(
                    hemb->vocab_size,
                    hemb->embedding_dim,
                    hemb->master_embeddings->base,
                    hemb->master_embeddings->precision
                );
                view->has_modifications = true;
            }
            
            // Apply boundary gradient to local view
            double* grad = (double*)malloc(hemb->embedding_dim * sizeof(double));
            double* current = (double*)malloc(hemb->embedding_dim * sizeof(double));
            if (grad && current) {
                abacus_matrix_get_row_as_doubles(boundary->gradient_buffer, j, grad);
                abacus_matrix_get_row_as_doubles(view->local_embeddings, token_id, current);
                
                for (uint32_t k = 0; k < hemb->embedding_dim; k++) {
                    current[k] -= grad[k];
                }
                
                abacus_matrix_set_row_from_doubles(view->local_embeddings, token_id, current);
            }
            free(grad);
            free(current);
        }
        
        // Clear boundary buffer
        boundary->num_tokens = 0;
        boundary->last_sync_epoch = hemb->sync_epoch;
        boundary->total_syncs++;
        
        pthread_spin_unlock(&boundary->lock);
    }
    
    // Step 2: Merge local views back to master
    for (uint32_t s = 0; s < hemb->num_spheres; s++) {
        hemb_synchronize_sphere(hemb, s);
    }
    
    // Update epoch
    hemb->sync_epoch++;
    hemb->updates_since_sync = 0;
    hemb->total_syncs++;
    
    printf("✓ Synchronization complete (epoch %lu)\n", hemb->sync_epoch);
    return 0;
}

// ============================================================================
// INITIALIZATION FROM DOUBLES
// ============================================================================

/**
 * Initialize master embeddings from double array
 */
int hemb_init_from_doubles(
    HierarchicalEmbeddings* hemb,
    const double* embeddings
) {
    if (!hemb || !embeddings) return -1;
    
    printf("Initializing hierarchical embeddings from doubles...\n");
    
    pthread_rwlock_wrlock(&hemb->master_lock);
    MathError err = abacus_matrix_from_doubles(hemb->master_embeddings, embeddings);
    pthread_rwlock_unlock(&hemb->master_lock);
    
    if (err != MATH_SUCCESS) {
        fprintf(stderr, "ERROR: Failed to initialize from doubles (error %d)\n", err);
        return -1;
    }
    
    printf("✓ Hierarchical embeddings initialized\n");
    return 0;
}

/**
 * Export master embeddings to double array
 */
int hemb_export_to_doubles(
    HierarchicalEmbeddings* hemb,
    double* output
) {
    if (!hemb || !output) return -1;
    
    pthread_rwlock_rdlock(&hemb->master_lock);
    MathError err = abacus_matrix_to_doubles(hemb->master_embeddings, output);
    pthread_rwlock_unlock(&hemb->master_lock);
    
    return (err == MATH_SUCCESS) ? 0 : -1;
}

// ============================================================================
// STATISTICS & DEBUGGING
// ============================================================================

/**
 * Get memory usage
 */
size_t hemb_memory_usage(HierarchicalEmbeddings* hemb) {
    if (!hemb) return 0;
    
    size_t total = sizeof(HierarchicalEmbeddings);
    
    // Master embeddings
    total += abacus_matrix_memory_usage(hemb->master_embeddings);
    
    // Token mapping
    total += hemb->vocab_size * sizeof(uint32_t);
    
    // Sphere views
    total += hemb->num_spheres * sizeof(SphereLocalView);
    for (uint32_t s = 0; s < hemb->num_spheres; s++) {
        SphereLocalView* view = &hemb->sphere_views[s];
        if (view->local_embeddings) {
            total += abacus_matrix_memory_usage(view->local_embeddings);
        }
        total += view->num_owned_tokens * sizeof(uint32_t);
        total += view->cache_capacity * hemb->embedding_dim * sizeof(double);
        total += view->cache_capacity * sizeof(uint32_t);
    }
    
    // Boundaries
    total += hemb->num_boundaries * sizeof(SphereBoundary);
    for (uint32_t i = 0; i < hemb->num_boundaries; i++) {
        total += abacus_matrix_memory_usage(hemb->boundaries[i].gradient_buffer);
        total += hemb->boundaries[i].capacity * sizeof(uint32_t);
    }
    
    // Neighbors
    total += hemb->num_spheres * HEMB_NEIGHBORS_PER_SPHERE * sizeof(uint32_t);
    
    return total;
}

/**
 * Print statistics
 */
void hemb_print_stats(HierarchicalEmbeddings* hemb) {
    if (!hemb) return;
    
    printf("\n=== Hierarchical Embeddings Statistics ===\n");
    printf("Configuration:\n");
    printf("  Vocab size: %u\n", hemb->vocab_size);
    printf("  Embedding dim: %u\n", hemb->embedding_dim);
    printf("  Num spheres: %u\n", hemb->num_spheres);
    printf("  Tokens per sphere: %u\n", hemb->tokens_per_sphere);
    printf("  Sync epoch: %lu\n", hemb->sync_epoch);
    printf("  Sync frequency: %lu\n", hemb->sync_frequency);
    
    printf("\nOperations:\n");
    printf("  Total lookups: %lu\n", hemb->total_lookups);
    printf("  Total updates: %lu\n", hemb->total_updates);
    printf("  Total syncs: %lu\n", hemb->total_syncs);
    printf("  Master reads: %lu\n", hemb->master_reads);
    printf("  Local reads: %lu\n", hemb->local_reads);
    printf("  Boundary writes: %lu\n", hemb->boundary_writes);
    
    printf("\nMemory:\n");
    size_t total_memory = hemb_memory_usage(hemb);
    printf("  Total memory: %.2f MB\n", total_memory / (1024.0 * 1024.0));
    printf("  Master: %.2f MB\n", 
           abacus_matrix_memory_usage(hemb->master_embeddings) / (1024.0 * 1024.0));
    
    // Cache statistics
    uint64_t total_hits = 0, total_misses = 0;
    for (uint32_t s = 0; s < hemb->num_spheres; s++) {
        total_hits += hemb->sphere_views[s].cache_hits;
        total_misses += hemb->sphere_views[s].cache_misses;
    }
    double hit_rate = (total_hits + total_misses > 0) ?
        (double)total_hits / (total_hits + total_misses) : 0.0;
    printf("  Cache hit rate: %.2f%%\n", hit_rate * 100.0);
    
    printf("==========================================\n\n");
}

/**
 * Validate structure integrity
 */
int hemb_validate(HierarchicalEmbeddings* hemb) {
    if (!hemb) return -1;
    
    // Check master embeddings
    if (!hemb->master_embeddings) {
        fprintf(stderr, "ERROR: Master embeddings is NULL\n");
        return -1;
    }
    
    // Check token mapping
    if (!hemb->token_to_sphere_map) {
        fprintf(stderr, "ERROR: Token mapping is NULL\n");
        return -1;
    }
    
    for (uint32_t token = 0; token < hemb->vocab_size; token++) {
        if (hemb->token_to_sphere_map[token] >= hemb->num_spheres) {
            fprintf(stderr, "ERROR: Invalid sphere mapping for token %u\n", token);
            return -1;
        }
    }
    
    // Check sphere views
    if (!hemb->sphere_views) {
        fprintf(stderr, "ERROR: Sphere views is NULL\n");
        return -1;
    }
    
    // Check boundaries
    if (!hemb->boundaries) {
        fprintf(stderr, "ERROR: Boundaries is NULL\n");
        return -1;
    }
    
    printf("✓ Hierarchical embeddings structure is valid\n");
    return 0;
}