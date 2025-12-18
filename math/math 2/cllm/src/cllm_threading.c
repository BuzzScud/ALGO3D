/**
 * @file cllm_threading.c
 * @brief CLLM 88D Threading System - Core Implementation
 * 
 * THREAD-CENTRIC ARCHITECTURE (88D):
 * - Threading is MANDATORY and initialized in cllm_create_model()
 * - 96 threads: 8 layers × 12 threads per layer (88 workers + 8 control)
 * - Token assignments are permanent (deterministic mapping)
 * - All parameters stored in thread-local CrystallineAbacus
 * - No separate initialization required
 */

#include "ai/cllm_threading.h"
#include "../../algorithms/include/hierarchical_threading.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ============================================================================
// CORE 88D THREADING API
// ============================================================================

HierarchicalThreadPool* cllm_get_thread_pool(CLLMModel* model) {
    if (!model) {
        fprintf(stderr, "ERROR: NULL model in cllm_get_thread_pool\n");
        return NULL;
    }
    
    // In 88D architecture, threading is ALWAYS enabled
    if (!model->threads) {
        fprintf(stderr, "FATAL: Model has no thread pool - this should never happen in 88D\n");
        return NULL;
    }
    
    return model->threads;
}

HierarchicalThread* cllm_get_thread(CLLMModel* model, uint8_t layer, uint8_t position) {
    HierarchicalThreadPool* pool = cllm_get_thread_pool(model);
    if (!pool) return NULL;
    
    // Validate layer and position
    if (layer >= 8 || position >= 12) {
        fprintf(stderr, "ERROR: Invalid layer=%u or position=%u\n", layer, position);
        return NULL;
    }
    
    return hierarchical_thread_get(pool, layer, position);
}

HierarchicalThread* cllm_get_token_thread(CLLMModel* model, uint32_t token_id) {
    if (!model || token_id >= model->vocab_size) {
        return NULL;
    }
    
    return model->token_assignments[token_id].thread;
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
    
    // Map edges to boundaries (use control threads)
    for (uint32_t i = 0; i < num_edges; i++) {
        uint8_t layer = (i % 8);  // Distribute across 8 layers
        model->threading.edge_to_boundary[i] = layer * 12;  // Position 0 = control
    }
    
    // Map faces to layers (geometric folding)
    for (uint32_t i = 0; i < num_faces; i++) {
        model->threading.face_to_layer[i] = i % 8;
    }
    
    printf("  ✓ Mapped geometry to 88D threads\n");
    printf("    Vertices: %u → threads\n", num_vertices);
    printf("    Edges: %u → boundaries\n", num_edges);
    printf("    Faces: %u → layers\n", num_faces);
    
    return true;
}

// ============================================================================
// WORK DISTRIBUTION
// ============================================================================

int cllm_start_threads(CLLMModel* model) {
    HierarchicalThreadPool* pool = cllm_get_thread_pool(model);
    if (!pool) return -1;
    
    return hierarchical_thread_pool_start(pool);
}

int cllm_stop_threads(CLLMModel* model) {
    HierarchicalThreadPool* pool = cllm_get_thread_pool(model);
    if (!pool) return -1;
    
    return hierarchical_thread_pool_stop(pool);
}

int cllm_wait_for_threads(CLLMModel* model) {
    HierarchicalThreadPool* pool = cllm_get_thread_pool(model);
    if (!pool) return -1;
    
    return hierarchical_thread_pool_wait(pool);
}

// ============================================================================
// THREAD-CENTRIC OPERATIONS
// ============================================================================

int cllm_start_thread(HierarchicalThread* thread) {
    if (!thread) return -1;
    
    return hierarchical_thread_start(thread, NULL, NULL);
}

int cllm_stop_thread(HierarchicalThread* thread) {
    if (!thread) return -1;
    
    return hierarchical_thread_stop(thread);
}

StateType cllm_get_thread_state(HierarchicalThread* thread) {
    if (!thread) return STATE_IDLE;
    
    return hierarchical_thread_get_state(thread);
}

// ============================================================================
// VALIDATION
// ============================================================================

bool cllm_validate_threading(CLLMModel* model) {
    if (!model) {
        fprintf(stderr, "ERROR: NULL model\n");
        return false;
    }
    
    if (!model->threads) {
        fprintf(stderr, "ERROR: No thread pool - threading is mandatory in 88D\n");
        return false;
    }
    
    // Validate thread pool structure
    HierarchicalThreadPool* pool = model->threads;
    if (pool->num_levels != 8) {
        fprintf(stderr, "ERROR: Expected 8 layers, got %u\n", pool->num_levels);
        return false;
    }
    
    if (pool->threads_per_level != 12) {
        fprintf(stderr, "ERROR: Expected 12 threads per layer, got %u\n", pool->threads_per_level);
        return false;
    }
    
    // Validate token assignments
    for (uint32_t i = 0; i < model->vocab_size; i++) {
        if (!model->token_assignments[i].thread) {
            fprintf(stderr, "ERROR: Token %u has no thread assignment\n", i);
            return false;
        }
    }
    
    printf("✓ 88D threading configuration validated\n");
    return true;
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

void cllm_print_threading_info(CLLMModel* model) {
    if (!model) {
        fprintf(stderr, "ERROR: NULL model\n");
        return;
    }
    
    HierarchicalThreadPool* pool = cllm_get_thread_pool(model);
    if (!pool) {
        fprintf(stderr, "ERROR: No thread pool\n");
        return;
    }
    
    printf("\n=== 88D Threading Information ===\n");
    printf("Total Threads: %u\n", pool->num_levels * pool->threads_per_level);
    printf("Layers: %u\n", pool->num_levels);
    printf("Threads per Layer: %u\n", pool->threads_per_level);
    printf("Worker Threads: %u\n", pool->num_levels * (pool->threads_per_level - 1));
    printf("Control Threads: %u\n", pool->num_levels);
    printf("Vocabulary Size: %u\n", model->vocab_size);
    printf("Token Assignments: %s\n", model->token_assignments ? "Complete" : "Incomplete");
    printf("==================================\n\n");
}