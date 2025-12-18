/**
 * @file cllm_threading.c
 * @brief CLLM Threading System - Core Implementation
 * 
 * THREAD-CENTRIC ARCHITECTURE (88D):
 * - Threading is MANDATORY and initialized in cllm_create_model()
 * - 96 threads: 8 layers × 12 threads per layer (88 workers + 8 control)
 * - Token assignments are permanent (deterministic mapping)
 * - All parameters stored in thread-local CrystallineAbacus
 * - No separate initialization required
 */

#include "ai/cllm_threading.h"
#include "hierarchical_threading.h"
#include "message_passing.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ============================================================================
// INITIALIZATION & CLEANUP
// ============================================================================

bool cllm_initialize_88d_threading(CLLMModel* model, uint32_t base) {
    if (!model) {
        fprintf(stderr, "cllm_initialize_88d_threading: NULL model\n");
        return false;
    }
    
    // Check if already initialized
    if (model->threads) {
        fprintf(stderr, "cllm_initialize_88d_threading: Already initialized\n");
        return true;  // Already initialized is not an error
    }
    
    printf("Initializing 88D threading system for CLLM model...\n");
    
    // Create 88D thread pool (96 threads: 8 layers × 12 threads per layer)
    model->threads = hierarchical_thread_pool_create_88d(base);
    if (!model->threads) {
        fprintf(stderr, "Failed to create 88D thread pool\n");
        return false;
    }
    
    printf("  ✓ Created 88D thread pool (96 threads)\n");
    
    // Initialize statistics
    model->threading.total_work_units = 0;
    model->threading.work_stolen = 0;
    model->threading.parallel_efficiency = 0.0;
    model->threading.load_balance_score = 0.0;
    
    printf("88D threading system initialized successfully!\n");
    printf("  - Threads: 96 (8 layers × 12 threads)\n");
    printf("  - Base: %u (for CrystallineAbacus)\n", base);
    
    return true;
}

void cllm_cleanup_88d_threading(CLLMModel* model) {
    if (!model) return;
    
    printf("Cleaning up 88D threading system...\n");
    
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
    printf("  ✓ Freed geometry mappings\n");
    
    // Free threading barriers
    if (model->threading.forward_barrier) {
        pthread_barrier_destroy(model->threading.forward_barrier);
        free(model->threading.forward_barrier);
        model->threading.forward_barrier = NULL;
    }
    if (model->threading.backward_barrier) {
        pthread_barrier_destroy(model->threading.backward_barrier);
        free(model->threading.backward_barrier);
        model->threading.backward_barrier = NULL;
    }
    if (model->threading.optimizer_barrier) {
        pthread_barrier_destroy(model->threading.optimizer_barrier);
        free(model->threading.optimizer_barrier);
        model->threading.optimizer_barrier = NULL;
    }
    printf("  ✓ Destroyed threading barriers\n");
    
    // Stop and destroy thread pool
    if (model->threads) {
        hierarchical_thread_pool_free(model->threads);
        model->threads = NULL;
        printf("  ✓ Freed thread pool\n");
    }
    
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
    
    // Allocate mapping arrays
    model->threading.vertex_to_thread = calloc(num_vertices, sizeof(uint32_t));
    model->threading.edge_to_boundary = calloc(num_edges, sizeof(uint32_t));
    model->threading.face_to_layer = calloc(num_faces, sizeof(uint32_t));
    
    if (!model->threading.vertex_to_thread || !model->threading.edge_to_boundary ||
        !model->threading.face_to_layer) {
        fprintf(stderr, "Failed to allocate geometry mapping arrays\n");
        return false;
    }
    
    // Map vertices to threads (distribute across 88 worker threads)
    for (uint32_t i = 0; i < num_vertices; i++) {
        model->threading.vertex_to_thread[i] = i % 88;
    }
    
    // Map edges to boundaries (distribute across boundaries)
    for (uint32_t i = 0; i < num_edges; i++) {
        model->threading.edge_to_boundary[i] = i % 88;
    }
    
    // Map faces to layers (distribute across 8 layers)
    for (uint32_t i = 0; i < num_faces; i++) {
        model->threading.face_to_layer[i] = i % 8;
    }
    
    printf("  ✓ Mapped %u vertices to threads\n", num_vertices);
    printf("  ✓ Mapped %u edges to boundaries\n", num_edges);
    printf("  ✓ Mapped %u faces to layers\n", num_faces);
    
    return true;
}

// ============================================================================
// TOKEN → THREAD MAPPING (NEW)
// ============================================================================

uint32_t cllm_get_thread_for_token(const CLLMModel* model, uint32_t token_id) {
    if (!model || !model->token_assignments) return UINT32_MAX;
    if (token_id >= model->vocab_size) return UINT32_MAX;
    
    return model->token_assignments[token_id].thread_id;
}

HierarchicalThread* cllm_get_thread_for_token_direct(const CLLMModel* model, uint32_t token_id) {
    if (!model || !model->token_assignments) return NULL;
    if (token_id >= model->vocab_size) return NULL;
    
    return model->token_assignments[token_id].thread;
}

// ============================================================================
// WORK DISTRIBUTION (SIMPLIFIED)
// ============================================================================

bool cllm_distribute_work_88d(CLLMModel* model, void* work_items, size_t num_items) {
    if (!model || !model->threads) return false;
    
    // Work distribution is handled by the thread pool
    // This is a placeholder for future work distribution logic
    (void)work_items;
    (void)num_items;
    
    return true;
}

bool cllm_submit_work_item(CLLMModel* model, void* work_item) {
    if (!model || !model->threads || !work_item) return false;
    
    // Work submission is handled by the thread pool
    // This is a placeholder for future work submission logic
    
    return true;
}

bool cllm_wait_for_work_completion(CLLMModel* model) {
    if (!model || !model->threads) return false;
    
    // Wait for all threads to complete
    // This uses the global barrier
    if (model->threading.forward_barrier) {
        pthread_barrier_wait(model->threading.forward_barrier);
    }
    
    return true;
}

// ============================================================================
// STATISTICS
// ============================================================================

void cllm_update_threading_stats(CLLMModel* model) {
    if (!model || !model->threads) return;
    
    HierarchicalThreadPool* pool = model->threads;
    
    // Update statistics from thread pool
    model->threading.total_work_units = pool->total_work_items;
    model->threading.work_stolen = pool->total_work_stolen;
    
    // Calculate parallel efficiency (placeholder)
    // TODO: Implement actual efficiency calculation
    model->threading.parallel_efficiency = 0.95;  // Placeholder
    model->threading.load_balance_score = 0.90;   // Placeholder
}

void cllm_print_threading_stats(const CLLMModel* model) {
    if (!model) return;
    
    printf("\n=== 88D Threading Statistics ===\n");
    printf("Total work units: %lu\n", model->threading.total_work_units);
    printf("Work stolen: %lu\n", model->threading.work_stolen);
    printf("Parallel efficiency: %.2f%%\n", model->threading.parallel_efficiency * 100);
    printf("Load balance score: %.2f%%\n", model->threading.load_balance_score * 100);
    printf("================================\n\n");
}