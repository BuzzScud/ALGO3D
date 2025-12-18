/**
 * @file cllm_88d_integration.c
 * @brief CLLM Integration with 88D Unified Threading System - Implementation
 */

#include "ai/cllm_88d_integration.h"
#include "hierarchical_threading.h"
#include "message_passing.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

// ============================================================================
// INITIALIZATION & CLEANUP
// ============================================================================

bool cllm_initialize_88d_threading(CLLMModel* model) {
    if (!model) {
        fprintf(stderr, "cllm_initialize_88d_threading: NULL model\n");
        return false;
    }
    
    // Check if already initialized
    if (model->threading.pool_88d) {
        fprintf(stderr, "cllm_initialize_88d_threading: Already initialized\n");
        return false;
    }
    
    printf("Initializing 88D threading system for CLLM model...\n");
    
    // Create 88D thread pool (96 threads: 8 layers × 12 threads per layer)
    // Base 2 for tetration (standard choice)
    HierarchicalThreadPool* pool = hierarchical_thread_pool_create_88d(2);
    if (!pool) {
        fprintf(stderr, "Failed to create 88D thread pool\n");
        return false;
    }
    
    model->threading.pool_88d = pool;
    model->threading.enabled = true;
    
    printf("  ✓ Created 88D thread pool (96 threads)\n");
    
    // Map geometry to threads
    if (!cllm_map_geometry_to_threads(model)) {
        fprintf(stderr, "Failed to map geometry to threads\n");
        hierarchical_thread_pool_stop(pool);
        model->threading.pool_88d = NULL;
        model->threading.enabled = false;
        return false;
    }
    
    printf("  ✓ Mapped geometry to threads\n");
    
    // Work queue and work stealing are built into the thread pool
    // We just store references for convenience
    model->threading.work_queue = pool;  // Pool handles work distribution
    model->threading.steal_pool = pool;  // Pool handles work stealing
    
    printf("  ✓ Work distribution integrated with thread pool\n");
    
    // Initialize statistics
    model->threading.total_work_units = 0;
    model->threading.work_stolen = 0;
    model->threading.parallel_efficiency = 0.0;
    model->threading.load_balance_score = 0.0;
    
    printf("88D threading system initialized successfully!\n");
    printf("  - Threads: 96 (8 layers × 12 threads)\n");
    printf("  - Vertices mapped: %u\n", model->geometry.vertices);
    printf("  - Edges mapped: %u\n", model->geometry.edges);
    printf("  - Faces mapped: %u\n", model->geometry.faces);
    
    return true;
}

void cllm_cleanup_88d_threading(CLLMModel* model) {
    if (!model) return;
    
    printf("Cleaning up 88D threading system...\n");
    
    // Free work stealing pool (if we had one)
    if (model->threading.steal_pool) {
        // Note: Work stealing is handled by the thread pool itself
        model->threading.steal_pool = NULL;
        printf("  ✓ Freed work stealing pool reference\n");
    }
    
    // Free work queue (if we had one)
    if (model->threading.work_queue) {
        // Note: Work queue is handled by the thread pool itself
        model->threading.work_queue = NULL;
        printf("  ✓ Freed work queue reference\n");
    }
    
    // Free geometry mappings
    if (model->threading.vertex_to_thread) {
        free(model->threading.vertex_to_thread);
        model->threading.vertex_to_thread = NULL;
    }
    if (model->threading.edge_to_boundary) {
        free(model->threading.edge_to_boundary);
        model->threading.edge_to_boundary = NULL;
    }
    if (model->threading.face_to_layer) {
        free(model->threading.face_to_layer);
        model->threading.face_to_layer = NULL;
    }
    if (model->threading.token_to_thread) {
        free(model->threading.token_to_thread);
        model->threading.token_to_thread = NULL;
    }
    printf("  ✓ Freed geometry mappings\n");
    
    // Stop and destroy thread pool
    if (model->threading.pool_88d) {
        HierarchicalThreadPool* pool = (HierarchicalThreadPool*)model->threading.pool_88d;
        hierarchical_thread_pool_stop(pool);
        // Note: No explicit destroy function - pool cleanup happens in stop
        model->threading.pool_88d = NULL;
        printf("  ✓ Stopped thread pool\n");
    }
    
    model->threading.enabled = false;
    
    printf("88D threading system cleaned up successfully!\n");
}

// ============================================================================
// GEOMETRY MAPPING
// ============================================================================

bool cllm_map_geometry_to_threads(CLLMModel* model) {
    if (!model) return false;
    
    uint32_t num_vertices = model->geometry.vertices;
    uint32_t num_edges = model->geometry.edges;
    uint32_t num_faces = model->geometry.faces;
    uint32_t vocab_size = model->vocab_size;
    
    // Allocate mapping arrays
    model->threading.vertex_to_thread = calloc(num_vertices, sizeof(uint32_t));
    model->threading.edge_to_boundary = calloc(num_edges, sizeof(uint32_t));
    model->threading.face_to_layer = calloc(num_faces, sizeof(uint32_t));
    model->threading.token_to_thread = calloc(vocab_size, sizeof(uint32_t));
    
    if (!model->threading.vertex_to_thread || !model->threading.edge_to_boundary ||
        !model->threading.face_to_layer || !model->threading.token_to_thread) {
        fprintf(stderr, "Failed to allocate geometry mapping arrays\n");
        return false;
    }
    
    // Map vertices to threads (round-robin across 88 worker threads)
    // Threads 0-87 are workers, 88-95 are control threads
    for (uint32_t i = 0; i < num_vertices; i++) {
        model->threading.vertex_to_thread[i] = i % 88;
    }
    
    // Map edges to boundaries (each edge connects two vertices)
    // Boundary ID is based on the edge's position in the geometry
    for (uint32_t i = 0; i < num_edges; i++) {
        model->threading.edge_to_boundary[i] = i % 88;
    }
    
    // Map faces to layers (distribute faces across 8 layers)
    for (uint32_t i = 0; i < num_faces; i++) {
        model->threading.face_to_layer[i] = i % 8;
    }
    
    // Map tokens to threads (hash-based distribution for load balancing)
    for (uint32_t i = 0; i < vocab_size; i++) {
        // Use a simple hash function to distribute tokens
        uint32_t hash = i * 2654435761u;  // Knuth's multiplicative hash
        model->threading.token_to_thread[i] = hash % 88;
    }
    
    return true;
}

uint32_t cllm_get_thread_for_vertex(const CLLMModel* model, uint32_t vertex_idx) {
    if (!model || !model->threading.vertex_to_thread) return UINT32_MAX;
    if (vertex_idx >= model->geometry.vertices) return UINT32_MAX;
    return model->threading.vertex_to_thread[vertex_idx];
}

uint32_t cllm_get_thread_for_token(const CLLMModel* model, uint32_t token_id) {
    if (!model || !model->threading.token_to_thread) return UINT32_MAX;
    if (token_id >= model->vocab_size) return UINT32_MAX;
    return model->threading.token_to_thread[token_id];
}

uint32_t cllm_get_layer_for_face(const CLLMModel* model, uint32_t face_idx) {
    if (!model || !model->threading.face_to_layer) return UINT32_MAX;
    if (face_idx >= model->geometry.faces) return UINT32_MAX;
    return model->threading.face_to_layer[face_idx];
}

// ============================================================================
// WORK DISTRIBUTION
// ============================================================================

bool cllm_distribute_work_88d(CLLMModel* model, CLLMWorkType work_type,
                              void* work_data, uint32_t num_items) {
    if (!model || !model->threading.enabled) return false;
    if (!model->threading.pool_88d) return false;
    
    // Work distribution is handled by the thread pool
    // For now, just track the work units
    model->threading.total_work_units += num_items;
    
    // TODO: Implement actual work distribution using the thread pool's work system
    printf("Work distribution: %u items of type %d (not yet fully implemented)\n", 
           num_items, work_type);
    
    return true;
}

bool cllm_submit_work_item(CLLMModel* model, const CLLMWorkItem* work_item) {
    if (!model || !model->threading.enabled) return false;
    if (!model->threading.pool_88d || !work_item) return false;
    
    // Work submission is handled by the thread pool
    model->threading.total_work_units++;
    
    // TODO: Implement actual work submission using the thread pool's work system
    return true;
}

void cllm_wait_for_work_completion(CLLMModel* model) {
    if (!model || !model->threading.enabled) return;
    if (!model->threading.pool_88d) return;
    
    // Use thread pool barrier to wait for all work to complete
    cllm_synchronize_threads(model);
}

// ============================================================================
// SYNCHRONIZATION
// ============================================================================

void cllm_synchronize_threads(CLLMModel* model) {
    if (!model || !model->threading.enabled) return;
    if (!model->threading.pool_88d) return;
    
    HierarchicalThreadPool* pool = (HierarchicalThreadPool*)model->threading.pool_88d;
    hierarchical_thread_pool_wait(pool);
}

void cllm_synchronize_layer(CLLMModel* model, uint32_t layer_idx) {
    if (!model || !model->threading.enabled) return;
    if (!model->threading.pool_88d) return;
    if (layer_idx >= 8) return;
    
    // TODO: Implement layer-specific synchronization
    // For now, just use global synchronization
    HierarchicalThreadPool* pool = (HierarchicalThreadPool*)model->threading.pool_88d;
    hierarchical_thread_pool_wait(pool);
}

void cllm_broadcast_message(CLLMModel* model, uint32_t message_type,
                           const void* data, size_t data_size) {
    if (!model || !model->threading.enabled) return;
    if (!model->threading.pool_88d) return;
    
    // TODO: Implement message broadcasting using the thread pool's message system
    // For now, just log the broadcast
    printf("Broadcasting message type %u to all threads (not yet fully implemented)\n", 
           message_type);
}

// ============================================================================
// STATISTICS & MONITORING
// ============================================================================

void cllm_get_threading_stats(const CLLMModel* model, HierarchicalThreadPoolStats* stats) {
    if (!model || !stats) return;
    if (!model->threading.pool_88d) return;
    
    HierarchicalThreadPool* pool = (HierarchicalThreadPool*)model->threading.pool_88d;
    hierarchical_thread_pool_get_stats(pool, stats);
}

void cllm_print_threading_stats(const CLLMModel* model) {
    if (!model) return;
    if (!model->threading.pool_88d) return;
    
    HierarchicalThreadPoolStats stats;
    cllm_get_threading_stats(model, &stats);
    
    printf("\n=== 88D Threading Statistics ===\n");
    printf("Total work units: %lu\n", model->threading.total_work_units);
    printf("Work stolen: %lu\n", model->threading.work_stolen);
    printf("Parallel efficiency: %.2f%%\n", model->threading.parallel_efficiency * 100.0);
    printf("Load balance score: %.2f\n", model->threading.load_balance_score);
    printf("Number of threads: %u\n", stats.num_threads);
    printf("Number of levels: %u\n", stats.num_levels);
    printf("Total messages: %lu\n", stats.total_messages);
    printf("Total work items: %lu\n", stats.total_work_items);
    printf("CPU utilization: %.2f%%\n", stats.avg_cpu_utilization * 100.0);
    printf("Load balance factor: %.2f\n", stats.load_balance_factor);
    printf("================================\n\n");
}

void cllm_reset_threading_stats(CLLMModel* model) {
    if (!model) return;
    
    model->threading.total_work_units = 0;
    model->threading.work_stolen = 0;
    model->threading.parallel_efficiency = 0.0;
    model->threading.load_balance_score = 0.0;
    
    // Note: hierarchical_thread_pool doesn't have a reset function yet
    // Statistics will be reset on next pool creation
}

// ============================================================================
// PARALLEL ML OPERATIONS (STUBS - TO BE IMPLEMENTED)
// ============================================================================

void cllm_parallel_embedding_lookup(CLLMModel* model, const uint32_t* token_ids,
                                   uint32_t batch_size, uint32_t seq_len,
                                   double* output) {
    // TODO: Implement parallel embedding lookup
    fprintf(stderr, "cllm_parallel_embedding_lookup: Not yet implemented\n");
}

void cllm_parallel_attention(CLLMModel* model, uint32_t layer_idx,
                            const double* input, uint32_t batch_size,
                            uint32_t seq_len, double* output) {
    // TODO: Implement parallel attention
    fprintf(stderr, "cllm_parallel_attention: Not yet implemented\n");
}

void cllm_parallel_ffn(CLLMModel* model, uint32_t layer_idx,
                      const double* input, uint32_t batch_size,
                      uint32_t seq_len, double* output) {
    // TODO: Implement parallel FFN
    fprintf(stderr, "cllm_parallel_ffn: Not yet implemented\n");
}

void cllm_parallel_gradient_accumulation(CLLMModel* model) {
    // TODO: Implement parallel gradient accumulation
    fprintf(stderr, "cllm_parallel_gradient_accumulation: Not yet implemented\n");
}

void cllm_parallel_weight_update(CLLMModel* model) {
    // TODO: Implement parallel weight update
    fprintf(stderr, "cllm_parallel_weight_update: Not yet implemented\n");
}