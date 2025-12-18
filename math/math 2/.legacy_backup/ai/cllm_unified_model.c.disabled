/**
 * @file cllm_unified_model.c
 * @brief Unified CLLM Model Implementation
 * 
 * Consolidates fragmented model creation, initialization, and validation
 * into a single unified implementation based on Platonic solid geometry.
 */

#include "ai/cllm_unified_model.h"
#include "prime_float_math.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * Initialize clock lattice positions for all tokens
 */
static bool init_clock_lattice_positions(UnifiedCLLMModel* model) {
    if (!model) return false;
    
    // Allocate position arrays
    model->token_positions = calloc(model->vocab_size, sizeof(BabylonianClockPosition));
    model->angular_positions = calloc(model->vocab_size, sizeof(double));
    
    if (!model->token_positions || !model->angular_positions) {
        fprintf(stderr, "ERROR: Failed to allocate clock lattice positions\n");
        return false;
    }
    
    // Map each token to clock lattice position
    for (uint32_t i = 0; i < model->vocab_size; i++) {
        model->token_positions[i] = map_prime_index_to_clock(i + 1);  // 1-based index
        
        // Compute angular position: θ(n,k,λ,ω,ψ)
        // For now, use simple mapping based on clock position
        double angle = model->token_positions[i].angle;
        double radius = model->token_positions[i].radius;
        model->angular_positions[i] = angle + radius * 0.1;
    }
    
    return true;
}

/**
 * Allocate model parameters
 */
static bool allocate_model_parameters(UnifiedCLLMModel* model) {
    if (!model) return false;
    
    uint32_t vocab_size = model->vocab_size;
    uint32_t embedding_dim = model->embedding_dim;
    uint32_t hidden_dim = model->hidden_dim;
    uint32_t num_layers = model->num_layers;
    uint32_t max_seq_len = model->max_seq_len;
    
    // Embeddings
    model->embeddings = calloc(vocab_size * embedding_dim, sizeof(double));
    model->pos_encoding = calloc(max_seq_len * embedding_dim, sizeof(double));
    
    if (!model->embeddings || !model->pos_encoding) {
        fprintf(stderr, "ERROR: Failed to allocate embeddings\n");
        return false;
    }
    
    // Layers
    model->layers = calloc(num_layers, sizeof(*model->layers));
    if (!model->layers) {
        fprintf(stderr, "ERROR: Failed to allocate layers\n");
        return false;
    }
    
    for (uint32_t i = 0; i < num_layers; i++) {
        // Attention weights (Q, K, V combined)
        model->layers[i].qkv_weights = calloc(
            3 * embedding_dim * embedding_dim, 
            sizeof(double)
        );
        model->layers[i].output_weights = calloc(
            embedding_dim * embedding_dim, 
            sizeof(double)
        );
        
        // Feed-forward
        model->layers[i].ffn_w1 = calloc(embedding_dim * hidden_dim, sizeof(double));
        model->layers[i].ffn_w2 = calloc(hidden_dim * embedding_dim, sizeof(double));
        model->layers[i].ffn_b1 = calloc(hidden_dim, sizeof(double));
        model->layers[i].ffn_b2 = calloc(embedding_dim, sizeof(double));
        
        // Layer normalization
        model->layers[i].ln1_gamma = calloc(embedding_dim, sizeof(double));
        model->layers[i].ln1_beta = calloc(embedding_dim, sizeof(double));
        model->layers[i].ln2_gamma = calloc(embedding_dim, sizeof(double));
        model->layers[i].ln2_beta = calloc(embedding_dim, sizeof(double));
        
        // Check allocations
        if (!model->layers[i].qkv_weights || !model->layers[i].output_weights ||
            !model->layers[i].ffn_w1 || !model->layers[i].ffn_w2 ||
            !model->layers[i].ffn_b1 || !model->layers[i].ffn_b2 ||
            !model->layers[i].ln1_gamma || !model->layers[i].ln1_beta ||
            !model->layers[i].ln2_gamma || !model->layers[i].ln2_beta) {
            fprintf(stderr, "ERROR: Failed to allocate layer %u parameters\n", i);
            return false;
        }
    }
    
    // Output projection
    model->output_weights = calloc(embedding_dim * vocab_size, sizeof(double));
    model->output_bias = calloc(vocab_size, sizeof(double));
    
    if (!model->output_weights || !model->output_bias) {
        fprintf(stderr, "ERROR: Failed to allocate output projection\n");
        return false;
    }
    
    return true;
}

/**
 * Initialize weights with geometric structure
 */
static void initialize_geometric_weights(UnifiedCLLMModel* model) {
    if (!model) return;
    
    // Use small random initialization scaled by 1/sqrt(dim)
    double scale = 1.0 / prime_sqrt((double)model->embedding_dim);
    
    // Initialize embeddings
    for (uint32_t i = 0; i < model->vocab_size * model->embedding_dim; i++) {
        model->embeddings[i] = ((double)rand() / RAND_MAX - 0.5) * 2.0 * scale;
    }
    
    // Initialize positional encoding (sinusoidal with clock lattice influence)
    for (uint32_t pos = 0; pos < model->max_seq_len; pos++) {
        BabylonianClockPosition clock_pos = map_prime_index_to_clock(pos + 1);
        
        for (uint32_t i = 0; i < model->embedding_dim; i++) {
            double angle = clock_pos.angle + (double)i / model->embedding_dim * 2.0 * M_PI;
            model->pos_encoding[pos * model->embedding_dim + i] = prime_sin(angle);
        }
    }
    
    // Initialize layer weights
    for (uint32_t layer = 0; layer < model->num_layers; layer++) {
        // Attention weights
        for (uint32_t i = 0; i < 3 * model->embedding_dim * model->embedding_dim; i++) {
            model->layers[layer].qkv_weights[i] = 
                ((double)rand() / RAND_MAX - 0.5) * 2.0 * scale;
        }
        
        for (uint32_t i = 0; i < model->embedding_dim * model->embedding_dim; i++) {
            model->layers[layer].output_weights[i] = 
                ((double)rand() / RAND_MAX - 0.5) * 2.0 * scale;
        }
        
        // FFN weights
        double ffn_scale = 1.0 / prime_sqrt((double)model->hidden_dim);
        for (uint32_t i = 0; i < model->embedding_dim * model->hidden_dim; i++) {
            model->layers[layer].ffn_w1[i] = 
                ((double)rand() / RAND_MAX - 0.5) * 2.0 * ffn_scale;
        }
        
        for (uint32_t i = 0; i < model->hidden_dim * model->embedding_dim; i++) {
            model->layers[layer].ffn_w2[i] = 
                ((double)rand() / RAND_MAX - 0.5) * 2.0 * ffn_scale;
        }
        
        // Layer norm (gamma=1, beta=0)
        for (uint32_t i = 0; i < model->embedding_dim; i++) {
            model->layers[layer].ln1_gamma[i] = 1.0;
            model->layers[layer].ln1_beta[i] = 0.0;
            model->layers[layer].ln2_gamma[i] = 1.0;
            model->layers[layer].ln2_beta[i] = 0.0;
        }
    }
    
    // Initialize output projection
    for (uint32_t i = 0; i < model->embedding_dim * model->vocab_size; i++) {
        model->output_weights[i] = ((double)rand() / RAND_MAX - 0.5) * 2.0 * scale;
    }
    
    // Output bias initialized to zero
    memset(model->output_bias, 0, model->vocab_size * sizeof(double));
}

// ============================================================================
// PUBLIC API IMPLEMENTATION
// ============================================================================

UnifiedCLLMModel* unified_model_create(const UnifiedModelConfig* config) {
    if (!config) {
        fprintf(stderr, "ERROR: NULL configuration\n");
        return NULL;
    }
    
    printf("\n=== Creating Unified CLLM Model ===\n\n");
    
    // 1. Create Platonic model
    printf("Creating Platonic model (%s)...\n", 
           platonic_solid_name(config->solid_type));
    
    // Get geometry to calculate dimensions
    PlatonicGeometry geom = platonic_get_geometry(config->solid_type);
    
    PlatonicModelConfig platonic_config = {
        .solid_type = config->solid_type,
        .vocab_size = config->vocab_size,
        .embedding_dim = geom.vertices * 12,
        .hidden_dim = geom.edges * 12,
        .num_layers = geom.faces,
        .num_heads = 12,  // Always 12-fold symmetry
        .max_seq_len = config->max_seq_len,
        .enable_blind_recovery = config->enable_blind_recovery,
        .enable_fourier_transform = config->enable_harmonic_integration,
        .enable_cymatic_modulation = config->enable_harmonic_integration,
        .enable_tetration_optimizer = config->enable_harmonic_integration,
    };
    
    PlatonicModel* platonic = platonic_model_create(&platonic_config);
    if (!platonic) {
        fprintf(stderr, "ERROR: Failed to create Platonic model\n");
        return NULL;
    }
    
    // 2. Create unified model structure
    UnifiedCLLMModel* model = calloc(1, sizeof(UnifiedCLLMModel));
    if (!model) {
        fprintf(stderr, "ERROR: Failed to allocate unified model\n");
        platonic_model_free(platonic);
        return NULL;
    }
    
    model->platonic = platonic;
    model->geometry = platonic_get_geometry(config->solid_type);
    
    // 3. Set dimensions from geometry
    model->embedding_dim = model->geometry.vertices * 12;
    model->hidden_dim = model->geometry.edges * 12;
    model->num_layers = model->geometry.faces;
    model->num_heads = 12;
    model->vocab_size = config->vocab_size;
    model->max_seq_len = config->max_seq_len;
    
    printf("Dimensions derived from geometry:\n");
    printf("  Vertices: %u → Embedding dim: %u\n", 
           model->geometry.vertices, model->embedding_dim);
    printf("  Edges: %u → Hidden dim: %u\n", 
           model->geometry.edges, model->hidden_dim);
    printf("  Faces: %u → Layers: %u\n", 
           model->geometry.faces, model->num_layers);
    printf("  Symmetries: %u → Heads: %u\n", 
           model->geometry.symmetries, model->num_heads);
    
    // 4. Initialize clock lattice positions
    printf("Initializing clock lattice positions...\n");
    if (!init_clock_lattice_positions(model)) {
        fprintf(stderr, "ERROR: Failed to initialize clock lattice\n");
        unified_model_free(model);
        return NULL;
    }
    
    // 5. Allocate model parameters
    printf("Allocating model parameters...\n");
    if (!allocate_model_parameters(model)) {
        fprintf(stderr, "ERROR: Failed to allocate parameters\n");
        unified_model_free(model);
        return NULL;
    }
    
    // 6. Initialize weights
    printf("Initializing weights with geometric structure...\n");
    initialize_geometric_weights(model);
    
    // 7. Set feature flags
    model->blind_recovery_enabled = config->enable_blind_recovery;
    model->harmonic_enabled = config->enable_harmonic_integration;
    model->ntt_attention_enabled = config->enable_ntt_attention;
    
    // 8. Set optimizer parameters
    model->optimizer_type = config->optimizer_type;
    model->learning_rate = config->learning_rate;
    model->beta1 = config->beta1;
    model->beta2 = config->beta2;
    model->epsilon = config->epsilon;
    model->optimizer_step = 0;
    
    // 9. Initialize optimizer buffers if needed
    if (model->optimizer_type == OPTIMIZER_ADAM || 
        model->optimizer_type == OPTIMIZER_RMSPROP) {
        uint64_t total_params = unified_model_count_parameters(model);
        model->m = calloc(total_params, sizeof(double));
        model->v = calloc(total_params, sizeof(double));
        
        if (!model->m || !model->v) {
            fprintf(stderr, "ERROR: Failed to allocate optimizer buffers\n");
            unified_model_free(model);
            return NULL;
        }
    }
    
    // 10. Initialize metrics
    model->total_steps = 0;
    model->current_loss = 0.0;
    model->best_loss = 1e10;
    model->tokens_processed = 0;
    
    printf("✓ Unified model created successfully\n\n");
    
    return model;
}

void unified_model_free(UnifiedCLLMModel* model) {
    if (!model) return;
    
    // Free Platonic model
    if (model->platonic) {
        platonic_model_free(model->platonic);
    }
    
    // Free clock lattice positions
    free(model->token_positions);
    free(model->angular_positions);
    
    // Free embeddings
    free(model->embeddings);
    free(model->pos_encoding);
    
    // Free layers
    if (model->layers) {
        for (uint32_t i = 0; i < model->num_layers; i++) {
            free(model->layers[i].qkv_weights);
            free(model->layers[i].output_weights);
            free(model->layers[i].ffn_w1);
            free(model->layers[i].ffn_w2);
            free(model->layers[i].ffn_b1);
            free(model->layers[i].ffn_b2);
            free(model->layers[i].ln1_gamma);
            free(model->layers[i].ln1_beta);
            free(model->layers[i].ln2_gamma);
            free(model->layers[i].ln2_beta);
        }
        free(model->layers);
    }
    
    // Free output projection
    free(model->output_weights);
    free(model->output_bias);
    
    // Free optimizer buffers
    free(model->m);
    free(model->v);
    
    // Free model structure
    free(model);
}

bool unified_model_validate(const UnifiedCLLMModel* model) {
    if (!model) return false;
    
    // Check Euler's formula: V - E + F = 2
    int euler_check = model->geometry.vertices - model->geometry.edges + model->geometry.faces;
    if (euler_check != 2) {
        fprintf(stderr, "ERROR: Euler's formula violated: %u - %u + %u = %d (expected 2)\n",
                model->geometry.vertices, model->geometry.edges, 
                model->geometry.faces, euler_check);
        return false;
    }
    
    // Check dimension consistency
    if (model->embedding_dim != model->geometry.vertices * 12) {
        fprintf(stderr, "ERROR: Embedding dimension mismatch\n");
        return false;
    }
    
    if (model->hidden_dim != model->geometry.edges * 12) {
        fprintf(stderr, "ERROR: Hidden dimension mismatch\n");
        return false;
    }
    
    // Check parameter allocation
    if (!model->embeddings || !model->pos_encoding || !model->layers ||
        !model->output_weights || !model->output_bias) {
        fprintf(stderr, "ERROR: Parameters not allocated\n");
        return false;
    }
    
    // Check clock lattice positions
    if (!model->token_positions || !model->angular_positions) {
        fprintf(stderr, "ERROR: Clock lattice positions not initialized\n");
        return false;
    }
    
    return true;
}

void unified_model_print_info(const UnifiedCLLMModel* model) {
    if (!model) return;
    
    printf("\n╔══════════════════════════════════════════════════════════╗\n");
    printf("║  UNIFIED CLLM MODEL INFORMATION                         ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n\n");
    
    printf("Platonic Solid: %s\n", platonic_solid_name(model->platonic->config.solid_type));
    printf("\nGeometric Properties:\n");
    printf("  Vertices (V): %u\n", model->geometry.vertices);
    printf("  Edges (E):    %u\n", model->geometry.edges);
    printf("  Faces (F):    %u\n", model->geometry.faces);
    printf("  Euler Check:  V - E + F = %u - %u + %u = %d ✓\n",
           model->geometry.vertices, model->geometry.edges, model->geometry.faces,
           model->geometry.vertices - model->geometry.edges + model->geometry.faces);
    printf("  Symmetries:   %u\n", model->geometry.symmetries);
    printf("  Golden Ratio: %s\n", model->geometry.has_golden_ratio ? "Yes" : "No");
    
    printf("\nModel Dimensions:\n");
    printf("  Embedding:    %u (%u × 12)\n", model->embedding_dim, model->geometry.vertices);
    printf("  Hidden:       %u (%u × 12)\n", model->hidden_dim, model->geometry.edges);
    printf("  Layers:       %u\n", model->num_layers);
    printf("  Heads:        %u (12-fold symmetry)\n", model->num_heads);
    printf("  Vocab Size:   %u\n", model->vocab_size);
    printf("  Max Seq Len:  %u\n", model->max_seq_len);
    
    printf("\nFeatures:\n");
    printf("  Blind Recovery:     %s\n", model->blind_recovery_enabled ? "Enabled" : "Disabled");
    printf("  Harmonic Integration: %s\n", model->harmonic_enabled ? "Enabled" : "Disabled");
    printf("  NTT Attention:      %s\n", model->ntt_attention_enabled ? "Enabled" : "Disabled");
    
    printf("\nOptimizer:\n");
    const char* opt_name = model->optimizer_type == OPTIMIZER_SGD ? "SGD" :
                          model->optimizer_type == OPTIMIZER_ADAM ? "Adam" : "RMSprop";
    printf("  Type:          %s\n", opt_name);
    printf("  Learning Rate: %.6f\n", model->learning_rate);
    if (model->optimizer_type == OPTIMIZER_ADAM) {
        printf("  Beta1:         %.4f\n", model->beta1);
        printf("  Beta2:         %.4f\n", model->beta2);
    }
    
    printf("\nParameters:\n");
    uint64_t total_params = unified_model_count_parameters(model);
    printf("  Total:         %lu (%.2f M)\n", total_params, total_params / 1e6);
    
    printf("\nTraining Metrics:\n");
    printf("  Steps:         %lu\n", model->total_steps);
    printf("  Current Loss:  %.6f\n", model->current_loss);
    printf("  Best Loss:     %.6f\n", model->best_loss);
    printf("  Tokens:        %lu\n", model->tokens_processed);
    
    printf("\n");
}

uint64_t unified_model_count_parameters(const UnifiedCLLMModel* model) {
    if (!model) return 0;
    
    uint64_t count = 0;
    
    // Embeddings
    count += (uint64_t)model->vocab_size * model->embedding_dim;
    count += (uint64_t)model->max_seq_len * model->embedding_dim;
    
    // Layers
    for (uint32_t i = 0; i < model->num_layers; i++) {
        // Attention
        count += 3ULL * model->embedding_dim * model->embedding_dim;  // QKV
        count += (uint64_t)model->embedding_dim * model->embedding_dim;  // Output
        
        // FFN
        count += (uint64_t)model->embedding_dim * model->hidden_dim;  // W1
        count += (uint64_t)model->hidden_dim * model->embedding_dim;  // W2
        count += model->hidden_dim;  // b1
        count += model->embedding_dim;  // b2
        
        // Layer norm
        count += 4ULL * model->embedding_dim;  // 2 gamma + 2 beta
    }
    
    // Output projection
    count += (uint64_t)model->embedding_dim * model->vocab_size;
    count += model->vocab_size;
    
    return count;
}

PlatonicSolidType unified_model_infer_solid(
    uint32_t embedding_dim,
    uint32_t num_layers
) {
    // Try to match dimensions to Platonic solids
    // embedding_dim = vertices × 12
    uint32_t vertices = embedding_dim / 12;
    
    if (vertices == 4 && num_layers == 4) return PLATONIC_TETRAHEDRON;
    if (vertices == 8 && num_layers == 6) return PLATONIC_CUBE;
    if (vertices == 6 && num_layers == 8) return PLATONIC_OCTAHEDRON;
    if (vertices == 20 && num_layers == 12) return PLATONIC_DODECAHEDRON;
    if (vertices == 12 && num_layers == 20) return PLATONIC_ICOSAHEDRON;
    
    // Default to cube if no match
    return PLATONIC_CUBE;
}

// ============================================================================
// CONVERSION FUNCTIONS (for backward compatibility)
// ============================================================================

UnifiedCLLMModel* unified_model_from_cllm(CLLMModel* old_model) {
    if (!old_model) return NULL;
    
    // Infer Platonic solid from dimensions
    PlatonicSolidType solid = unified_model_infer_solid(
        old_model->embedding_dim,
        old_model->num_layers
    );
    
    // Create configuration
    UnifiedModelConfig config = {
        .solid_type = solid,
        .vocab_size = old_model->vocab_size,
        .max_seq_len = 512,  // Default
        .enable_blind_recovery = false,
        .enable_harmonic_integration = false,
        .enable_ntt_attention = false,
        .optimizer_type = OPTIMIZER_ADAM,
        .learning_rate = 0.0001,
        .beta1 = 0.9,
        .beta2 = 0.999,
        .epsilon = 1e-8,
    };
    
    // Create unified model
    UnifiedCLLMModel* unified = unified_model_create(&config);
    if (!unified) return NULL;
    
    // Copy weights if dimensions match
    // (This is a simplified version - full implementation would handle dimension mismatches)
    
    return unified;
}

CLLMModel* unified_model_to_cllm(UnifiedCLLMModel* unified) {
    if (!unified) return NULL;
    
    // Create old-format model
    // (This is a placeholder - full implementation would properly convert)
    
    return NULL;  // Not implemented yet
}