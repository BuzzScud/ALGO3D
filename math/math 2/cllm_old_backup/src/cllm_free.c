/**
 * @file cllm_free.c
 * @brief Free CLLM Model with Complete Cleanup
 * 
 * Properly frees all allocated memory including:
 * - Geometric structures
 * - Clock lattice mappings
 * - Blind recovery state
 * - Harmonic integration state
 * - NTT attention workspace
 * - Kissing spheres threading state
 * - All model parameters and gradients
 */

#include "../include/cllm.h"
#include "../include/ai/cllm_88d_integration.h"
#include <stdlib.h>
#include <stdio.h>

/**
 * Free a CLLM model and all associated memory
 */
void cllm_free_model(CLLMModel* model) {
    if (!model) return;
    
    printf("🗑️  Freeing CLLM model...\n");
    
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
    
    // ========================================================================
    // FREE MODEL PARAMETERS
    // ========================================================================
    
    // Embeddings
    if (model->embeddings) {
        free(model->embeddings);
        model->embeddings = NULL;
    }
    
    if (model->embeddings_grad) {
        free(model->embeddings_grad);
        model->embeddings_grad = NULL;
    }
    
    if (model->positional_encoding) {
        free(model->positional_encoding);
        model->positional_encoding = NULL;
    }
    
    // Layers
    if (model->layers) {
        for (uint32_t i = 0; i < model->num_layers; i++) {
            // Attention weights
            if (model->layers[i].query_weights) free(model->layers[i].query_weights);
            if (model->layers[i].key_weights) free(model->layers[i].key_weights);
            if (model->layers[i].value_weights) free(model->layers[i].value_weights);
            if (model->layers[i].output_weights) free(model->layers[i].output_weights);
            
            // Attention gradients
            if (model->layers[i].query_grad) free(model->layers[i].query_grad);
            if (model->layers[i].key_grad) free(model->layers[i].key_grad);
            if (model->layers[i].value_grad) free(model->layers[i].value_grad);
            if (model->layers[i].output_grad) free(model->layers[i].output_grad);
            
            // Feed-forward weights
            if (model->layers[i].ffn_w1) free(model->layers[i].ffn_w1);
            if (model->layers[i].ffn_w2) free(model->layers[i].ffn_w2);
            if (model->layers[i].ffn_b1) free(model->layers[i].ffn_b1);
            if (model->layers[i].ffn_b2) free(model->layers[i].ffn_b2);
            
            // Feed-forward gradients
            if (model->layers[i].ffn_w1_grad) free(model->layers[i].ffn_w1_grad);
            if (model->layers[i].ffn_w2_grad) free(model->layers[i].ffn_w2_grad);
            if (model->layers[i].ffn_b1_grad) free(model->layers[i].ffn_b1_grad);
            if (model->layers[i].ffn_b2_grad) free(model->layers[i].ffn_b2_grad);
            
            // Layer norm parameters
            if (model->layers[i].ln1_gamma) free(model->layers[i].ln1_gamma);
            if (model->layers[i].ln1_beta) free(model->layers[i].ln1_beta);
            if (model->layers[i].ln2_gamma) free(model->layers[i].ln2_gamma);
            if (model->layers[i].ln2_beta) free(model->layers[i].ln2_beta);
            
            // Layer norm gradients
            if (model->layers[i].ln1_gamma_grad) free(model->layers[i].ln1_gamma_grad);
            if (model->layers[i].ln1_beta_grad) free(model->layers[i].ln1_beta_grad);
            if (model->layers[i].ln2_gamma_grad) free(model->layers[i].ln2_gamma_grad);
            if (model->layers[i].ln2_beta_grad) free(model->layers[i].ln2_beta_grad);
        }
        free(model->layers);
        model->layers = NULL;
    }
    
    // Output layer
    if (model->output_weights) {
        free(model->output_weights);
        model->output_weights = NULL;
    }
    
    if (model->output_bias) {
        free(model->output_bias);
        model->output_bias = NULL;
    }
    
    if (model->output_weights_grad) {
        free(model->output_weights_grad);
        model->output_weights_grad = NULL;
    }
    
    if (model->output_bias_grad) {
        free(model->output_bias_grad);
        model->output_bias_grad = NULL;
    }
    
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
    }
    
    // ========================================================================
    // FREE HARMONIC INTEGRATION STATE
    // ========================================================================
    
    if (model->harmonic.enabled) {
        if (model->harmonic.fourier_coefficients) {
            free(model->harmonic.fourier_coefficients);
            model->harmonic.fourier_coefficients = NULL;
        }
    }
    
    // ========================================================================
    // FREE NTT ATTENTION STATE
    // ========================================================================
    
    if (model->ntt.enabled) {
        if (model->ntt.ntt_workspace) {
            free(model->ntt.ntt_workspace);
            model->ntt.ntt_workspace = NULL;
        }
        
        if (model->ntt.ntt_frequencies) {
            free(model->ntt.ntt_frequencies);
            model->ntt.ntt_frequencies = NULL;
        }
    }
    
    // ========================================================================
    // FREE 88D UNIFIED THREADING SYSTEM
    // ========================================================================
    
    if (model->threading.enabled) {
        printf("  🔮 Cleaning up 88D threading system...\n");
        cllm_cleanup_88d_threading(model);
        printf("  ✓ 88D threading system cleaned up\n");
    }
    
    // ========================================================================
    // FREE OPTIMIZER STATE
    // ========================================================================
    
    if (model->optimizer.m) {
        free(model->optimizer.m);
        model->optimizer.m = NULL;
    }
    
    if (model->optimizer.v) {
        free(model->optimizer.v);
        model->optimizer.v = NULL;
    }
    
    // ========================================================================
    // FREE FILE FORMAT STRUCTURES
    // ========================================================================
    
    if (model->tokens) {
        free(model->tokens);
        model->tokens = NULL;
    }
    
    // ========================================================================
    // FREE GEOMETRY (if dynamically allocated)
    // ========================================================================
    
    // Note: PlatonicGeometry is a simple struct with no dynamic allocations
    // (vertices, edges, faces, symmetries, edge_length, has_golden_ratio)
    // No need to free anything here
    
    // ========================================================================
    // FREE MODEL STRUCTURE
    // ========================================================================
    
    free(model);
    
    printf("  ✓ Model freed successfully\n");
}

/**
 * Validate model integrity
 * Checks Euler's formula, symmetry, and other geometric properties
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
    
    // Check parameters are allocated
    if (!model->embeddings || !model->layers || !model->output_weights) {
        fprintf(stderr, "  ✗ Model parameters not allocated\n");
        return false;
    }
    printf("  ✓ Model parameters allocated\n");
    
    // Check clock lattice mapping
    if (!model->token_positions || !model->token_angular_positions) {
        fprintf(stderr, "  ✗ Clock lattice mapping not initialized\n");
        return false;
    }
    printf("  ✓ Clock lattice mapping initialized\n");
    
    printf("✅ Model validation passed\n");
    return true;
}
