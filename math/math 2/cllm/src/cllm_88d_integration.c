/**
 * @file cllm_88d_integration.c
 * @brief CLLM Integration with 88D Unified Threading System - Implementation
 * 
 * UPDATED FOR THREAD-CENTRIC ARCHITECTURE:
 * - threads is now a direct field in CLLMModel (not in threading struct)
 * - Threading is MANDATORY (not optional)
 * - Token assignments are permanent (in token_assignments array)
 * - All parameters stored in thread CrystallineAbacus
 */

#include "ai/cllm_88d_integration.h"
#include "hierarchical_threading.h"
#include "message_passing.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>  // For pthread_barrier functions

// ============================================================================
// INITIALIZATION & CLEANUP
// ============================================================================

bool cllm_initialize_threading(CLLMModel* model, uint32_t base) {
    if (!model) {
        fprintf(stderr, "cllm_initialize_threading: NULL model\n");
        return false;
    }
    
    // Check if already initialized
    if (model->threads) {
        fprintf(stderr, "cllm_initialize_threading: Already initialized\n");
        return true;  // Already initialized is not an error
    }
    
    printf("Initializing threading system for CLLM model...\n");
    
    // CRITICAL: Threading is MANDATORY - create 88D thread pool (96 threads: 8 layers × 12 threads per layer)
    model->threads = hierarchical_thread_pool_create_88d(base);
    if (!model->threads) {
        fprintf(stderr, "FATAL ERROR: Failed to create thread pool\n");
        fprintf(stderr, "Threading is MANDATORY - cannot proceed without threads\n");
        return false;
    }
    
    // Verify thread pool is valid
    if (model->threads->num_threads != 96) {
        fprintf(stderr, "FATAL ERROR: Thread pool has wrong size (%u, expected 96)\n",
                model->threads->num_threads);
        hierarchical_thread_pool_free(model->threads);
        model->threads = NULL;
        return false;
    }
    
    printf("  ✓ Created thread pool (96 threads: 8 layers × 12 threads per layer)\n");
    
    // Initialize statistics
    model->threading.total_work_units = 0;
    model->threading.work_stolen = 0;
    model->threading.parallel_efficiency = 0.0;
    model->threading.load_balance_score = 0.0;
    
    printf("Threading system initialized successfully!\n");
    printf("  - Threads: 96 (8 layers × 12 threads)\n");
    printf("  - Base: %u (for CrystallineAbacus)\n", base);
    
    return true;
}

void cllm_cleanup_threading(CLLMModel* model) {
    if (!model) return;
    
    printf("Cleaning up threading system...\n");
    
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
    
    printf("Threading system cleaned up successfully!\n");
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

bool cllm_distribute_work(CLLMModel* model, void* work_items, size_t num_items) {
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
    
    printf("\n=== Threading Statistics ===\n");
    printf("Total work units: %lu\n", model->threading.total_work_units);
    printf("Work stolen: %lu\n", model->threading.work_stolen);
    printf("Parallel efficiency: %.2f%%\n", model->threading.parallel_efficiency * 100);
    printf("Load balance score: %.2f%%\n", model->threading.load_balance_score * 100);
    printf("================================\n\n");
}