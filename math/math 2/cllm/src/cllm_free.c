/**
 * @file cllm_free.c
 * @brief Free CLLM Model with Complete Cleanup
 * 
 * UPDATED FOR THREAD-CENTRIC ARCHITECTURE:
 * - Frees 88D thread pool and all thread-local storage
 * - Frees token assignments and thread parameters
 * - Frees layer info and threading barriers
 * - NO flat arrays to free (they don't exist anymore)
 * - All parameters stored in thread CrystallineAbacus
 */

#include "ai/cllm.h"
#include "ai/cllm_generic_interface.h"
#include "hierarchical_threading.h"
#include <stdlib.h>
#include <stdio.h>

/**
 * Free a CLLM model and all associated memory
 */
void cllm_free_model(CLLMModel* model) {
    // IMMEDIATE DEBUG - write to stderr FIRST
    fprintf(stderr, "DEBUG: cllm_free_model ENTERED, model=%p\n", (void*)model);
    
    if (!model) {
        fprintf(stderr, "DEBUG: model is NULL, returning\n");
        return;
    }
    
    fprintf(stderr, "DEBUG: model is NOT NULL, continuing\n");
    
    printf("🗑️  Freeing CLLM model...\n");
    fflush(stdout);
    
    // Free vocabulary
    printf("  → Freeing vocabulary...\n");
    fflush(stdout);
    if (model->vocabulary) {
        // TEMPORARILY DISABLED TO DEBUG HANG
        // cllm_vocab_destroy(model->vocabulary);
        model->vocabulary = NULL;
    }
    printf("  ✓ Freed vocabulary (skipped for debug)\n");
    fflush(stdout);
    
    // ========================================================================
    // FREE GENERIC MODEL INTERFACE
    // ========================================================================
    
    printf("  → Freeing generic model interface...\n");
    fflush(stdout);
    if (model->generic_interface) {
        cllm_free_generic_interface((GenericModel*)model->generic_interface);
        model->generic_interface = NULL;
    }
    printf("  ✓ Freed generic model interface\n");
    fflush(stdout);
    
    // ========================================================================
    // FREE CLOCK LATTICE MAPPING
    // ========================================================================
    
    if (model->vertex_positions) {
        free(model->vertex_positions);
        model->vertex_positions = NULL;
    }
    
    if (model->token_positions) {
        free(model->token_positions);
        model->token_positions = NULL;
    }
    
    if (model->token_angular_positions) {
        free(model->token_angular_positions);
        model->token_angular_positions = NULL;
    }
    
    if (model->token_positions_13d) {
        free(model->token_positions_13d);
        model->token_positions_13d = NULL;
    }
    
    printf("  ✓ Freed clock lattice mappings\n");
    
    // ========================================================================
    // FREE 88D THREAD-CENTRIC ARCHITECTURE
    // ========================================================================
    
    // Free token assignments
    if (model->token_assignments) {
        free(model->token_assignments);
        model->token_assignments = NULL;
    }
    printf("  ✓ Freed token assignments\n");
    
    // Free thread parameters
    if (model->thread_params) {
        // Free token ID arrays for each thread
        for (uint32_t i = 0; i < 96; i++) {
            if (model->thread_params[i].token_ids) {
                free(model->thread_params[i].token_ids);
                model->thread_params[i].token_ids = NULL;
            }
        }
        free(model->thread_params);
        model->thread_params = NULL;
    }
    printf("  ✓ Freed thread parameters\n");
    
    // Free layer info
    if (model->layer_info) {
        for (uint32_t i = 0; i < model->num_layers; i++) {
            if (model->layer_info[i].worker_threads) {
                free(model->layer_info[i].worker_threads);
                model->layer_info[i].worker_threads = NULL;
            }
        }
        free(model->layer_info);
        model->layer_info = NULL;
    }
    printf("  ✓ Freed layer info\n");
    
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
    printf("  ✓ Freed threading barriers\n");
    
    // Free 88D thread pool (this frees all thread-local storage)
    // All parameters stored in thread CrystallineAbacus are freed here
    printf("  → About to free 88D thread pool...\n");
    fflush(stdout);
    if (model->threads) {
        hierarchical_thread_pool_free(model->threads);
        model->threads = NULL;
        printf("  ✓ Freed 88D thread pool (including all thread-local parameters)\n");
        fflush(stdout);
    }
    
    // ========================================================================
    // LEGACY REMOVED: No flat arrays to free
    // ========================================================================
    
    // DELETED: model->embeddings (now in thread CrystallineAbacus)
    // DELETED: model->embeddings_grad (now in thread accumulators)
    // DELETED: model->positional_encoding (now in threads)
    // DELETED: model->abacus_embeddings (now in threads)
    // DELETED: model->abacus_positional_encoding (now in threads)
    // DELETED: model->layers[i].query_weights (now in threads)
    // DELETED: model->layers[i].key_weights (now in threads)
    // DELETED: model->layers[i].value_weights (now in threads)
    // DELETED: model->layers[i].ffn_w1 (now in threads)
    // DELETED: model->layers[i].ffn_w2 (now in threads)
    // DELETED: model->layers[i].ln1_gamma (now in threads)
    // DELETED: model->layers[i].ln1_beta (now in threads)
    // DELETED: model->output_weights (now in threads)
    // DELETED: model->output_bias (now in threads)
    // All parameters are stored in thread CrystallineAbacus and freed with the thread pool
    
    printf("  ✓ All thread-local parameters freed with thread pool\n");
    
    // ========================================================================
    // FREE BLIND RECOVERY STATE
    // ========================================================================
    
    if (model->recovery.enabled) {
        if (model->recovery.vertex_backup) {
            free(model->recovery.vertex_backup);
            model->recovery.vertex_backup = NULL;
        }
        
        if (model->recovery.edge_backup) {
            free(model->recovery.edge_backup);
            model->recovery.edge_backup = NULL;
        }
        
        if (model->recovery.face_backup) {
            free(model->recovery.face_backup);
            model->recovery.face_backup = NULL;
        }
        printf("  ✓ Freed blind recovery state\n");
    }
    
    // ========================================================================
    // FREE HARMONIC INTEGRATION STATE
    // ========================================================================
    
    if (model->harmonic.enabled) {
        if (model->harmonic.fourier_coefficients) {
            free(model->harmonic.fourier_coefficients);
            model->harmonic.fourier_coefficients = NULL;
        }
        printf("  ✓ Freed harmonic integration state\n");
    }
    
    // ========================================================================
    // FREE NTT ATTENTION STATE
    // ========================================================================
    
    if (model->ntt.enabled) {
        // PHASE 2: NTT workspace removed - was in thread-local storage
        // Nothing to free here anymore
        printf("  ✓ NTT attention state (thread-local, freed with threads)\n");
    }
    
    // ========================================================================
    // FREE THREADING GEOMETRY MAPPINGS
    // ========================================================================
    
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
    printf("  ✓ Freed threading geometry mappings\n");
    
    // ========================================================================
    // FREE OPTIMIZER STATE (LEGACY REMOVED)
    // ========================================================================
    
    // DELETED: model->optimizer.m (now in thread CrystallineAbacus)
    // DELETED: model->optimizer.v (now in thread CrystallineAbacus)
    // Optimizer state is now stored in thread temp storage
    
    // ========================================================================
    // FREE FILE FORMAT STRUCTURES
    // ========================================================================
    
    if (model->tokens) {
        free(model->tokens);
        model->tokens = NULL;
    }
    
    // ========================================================================
    // FREE PLATONIC SOLID
    // ========================================================================
    
    if (model->platonic_solid) {
        // Use proper type from math library
        extern void platonic_free(PlatonicSolid* solid);
        platonic_free((PlatonicSolid*)model->platonic_solid);
        model->platonic_solid = NULL;
    }
    printf("  ✓ Freed Platonic solid\n");
    
    // ========================================================================
    // FREE MODEL STRUCTURE
    // ========================================================================
    
    free(model);
    
    printf("  ✓ Model freed successfully\n");
    printf("  ✓ Thread-centric architecture cleanup complete\n");
}

/**
 * Validate model integrity
 * Checks Euler's formula, symmetry, and thread-centric architecture
 */
bool cllm_validate_model(const CLLMModel* model) {
    if (!model) return false;
    
    printf("🔍 Validating model...\n");
    
    // Check Euler's formula: V - E + F = 2
    int euler = model->geometry.vertices - model->geometry.edges + model->geometry.faces;
    if (euler != 2) {
        fprintf(stderr, "  ✗ Euler's formula failed: V - E + F = %d (expected 2)\n", euler);
        return false;
    }
    printf("  ✓ Euler's formula: V - E + F = %d\n", euler);
    
    // Check dimensions are consistent with geometry
    uint32_t expected_embedding = model->geometry.vertices * 12;
    uint32_t expected_hidden = model->geometry.edges * 12;
    uint32_t expected_layers = model->geometry.faces;
    
    if (model->embedding_dim != expected_embedding) {
        fprintf(stderr, "  ⚠ Warning: embedding_dim=%u, expected %u (V×12)\n",
                model->embedding_dim, expected_embedding);
    }
    
    if (model->hidden_dim != expected_hidden) {
        fprintf(stderr, "  ⚠ Warning: hidden_dim=%u, expected %u (E×12)\n",
                model->hidden_dim, expected_hidden);
    }
    
    if (model->num_layers != expected_layers) {
        fprintf(stderr, "  ⚠ Warning: num_layers=%u, expected %u (F)\n",
                model->num_layers, expected_layers);
    }
    
    // Check 12-fold symmetry
    if (model->num_heads != 12) {
        fprintf(stderr, "  ✗ Invalid num_heads: %u (expected 12)\n", model->num_heads);
        return false;
    }
    printf("  ✓ 12-fold symmetry: num_heads = 12\n");
    
    // Check thread-centric architecture
    if (!model->threads) {
        fprintf(stderr, "  ✗ 88D thread pool not initialized\n");
        return false;
    }
    printf("  ✓ 88D thread pool initialized\n");
    
    if (!model->token_assignments) {
        fprintf(stderr, "  ✗ Token assignments not initialized\n");
        return false;
    }
    printf("  ✓ Token assignments initialized\n");
    
    if (!model->thread_params) {
        fprintf(stderr, "  ✗ Thread parameters not initialized\n");
        return false;
    }
    printf("  ✓ Thread parameters initialized\n");
    
    if (!model->layer_info) {
        fprintf(stderr, "  ✗ Layer info not initialized\n");
        return false;
    }
    printf("  ✓ Layer info initialized\n");
    
    // Check threading barriers
    if (!model->threading.forward_barrier || !model->threading.backward_barrier || 
        !model->threading.optimizer_barrier) {
        fprintf(stderr, "  ✗ Threading barriers not initialized\n");
        return false;
    }
    printf("  ✓ Threading barriers initialized\n");
    
    // Check clock lattice mapping
    if (!model->token_positions || !model->token_angular_positions) {
        fprintf(stderr, "  ✗ Clock lattice mapping not initialized\n");
        return false;
    }
    printf("  ✓ Clock lattice mapping initialized\n");
    
    printf("✅ Model validation passed (thread-centric architecture)\n");
    return true;
}