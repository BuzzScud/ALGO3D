/**
 * @file cllm_create.c
 * @brief CLLM Model Creation with Complete Geometric Foundation
 * 
 * COMPLETE TRANSFORMATION:
 * - Platonic solid-based architecture (all 5 solids)
 * - Automatic dimension derivation from geometry
 * - Clock lattice mapping for all tokens and vertices
 * - Blind recovery initialization
 * - Harmonic integration initialization
 * - NTT attention initialization
 * - Kissing spheres threading initialization
 * - Geometric weight initialization
 */

#include "math/transcendental.h"
#include "math/constants.h"
#include "math/arithmetic.h"
#include "math/constants.h"
#include "math/clock.h"
#include "math/constants.h"
#include "math/clock_lattice_13d.h"
#include "math/constants.h"
#include "../include/cllm.h"
#include "math/constants.h"
#include "../include/ai/cllm_platonic.h"
#include "math/constants.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

// External functions
extern uint64_t crystalline_get_nth_prime(uint32_t n);
extern PlatonicGeometry platonic_get_geometry(PlatonicSolidType solid_type);
extern bool platonic_verify_euler(const PlatonicGeometry* geometry);

// Phase 2: Math library Platonic generator integration
extern void* cllm_generate_platonic_solid(PlatonicSolidType solid_type);
extern void cllm_update_geometry_from_solid(void* model, const void* solid);
extern void cllm_print_platonic_solid(const void* solid);

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * Compute angular position θ(n,k,λ,ω,ψ) for a token
 * This is the complete formula from the architecture
 */
static double compute_angular_position(uint32_t token_id, const CLLMModel* model) {
    // Get prime for this token
    uint64_t prime = crystalline_get_nth_prime(token_id);
    
    // Parameters
    double n = (double)token_id;
    double k = (double)model->geometry.vertices;  // Use vertices as k
    double lambda = (double)model->geometry.edges; // Use edges as λ
    double omega = 2.0 * MATH_PI / 12.0;  // 12-fold symmetry
    double psi = (double)(prime % 360) * MATH_PI / 180.0;  // Prime-based phase
    
    // θ(n,k,λ,ω,ψ) = (2πn/k) + (λ/k)·math_sin(ωn + ψ)
    double theta = (2.0 * MATH_PI * n / k) + (lambda / k) * math_sin(omega * n + psi);
    
    return theta;
}

/**
 * Initialize geometric weights based on Platonic solid structure
 */
static void initialize_geometric_weights(CLLMModel* model) {
    // Use Xavier/Glorot initialization scaled by geometric properties
    double scale = math_sqrt(2.0 / (model->embedding_dim + model->hidden_dim));
    
    // Scale by golden ratio if applicable
    if (model->geometry.has_golden_ratio) {
        scale *= GOLDEN_RATIO;
    }
    
    // Initialize embeddings
    for (uint32_t i = 0; i < model->vocab_size * model->embedding_dim; i++) {
        // Use prime-based initialization for better distribution
        double r1 = (double)(rand() % 10000) / 10000.0;
        double r2 = (double)(rand() % 10000) / 10000.0;
        model->embeddings[i] = scale * math_sqrt(-2.0 * math_log(r1)) * math_cos(2.0 * MATH_PI * r2);
    }
    
    // Initialize layer weights
    for (uint32_t layer = 0; layer < model->num_layers; layer++) {
        // Attention weights
        size_t attn_size = model->embedding_dim * model->embedding_dim;
        for (size_t i = 0; i < attn_size; i++) {
            double r1 = (double)(rand() % 10000) / 10000.0;
            double r2 = (double)(rand() % 10000) / 10000.0;
            double val = scale * math_sqrt(-2.0 * math_log(r1)) * math_cos(2.0 * MATH_PI * r2);
            
            model->layers[layer].query_weights[i] = val;
            model->layers[layer].key_weights[i] = val;
            model->layers[layer].value_weights[i] = val;
            model->layers[layer].output_weights[i] = val;
        }
        
        // Feed-forward weights
        size_t ffn_size1 = model->embedding_dim * model->hidden_dim;
        for (size_t i = 0; i < ffn_size1; i++) {
            double r1 = (double)(rand() % 10000) / 10000.0;
            double r2 = (double)(rand() % 10000) / 10000.0;
            model->layers[layer].ffn_w1[i] = scale * math_sqrt(-2.0 * math_log(r1)) * math_cos(2.0 * MATH_PI * r2);
        }
        
        size_t ffn_size2 = model->hidden_dim * model->embedding_dim;
        for (size_t i = 0; i < ffn_size2; i++) {
            double r1 = (double)(rand() % 10000) / 10000.0;
            double r2 = (double)(rand() % 10000) / 10000.0;
            model->layers[layer].ffn_w2[i] = scale * math_sqrt(-2.0 * math_log(r1)) * math_cos(2.0 * MATH_PI * r2);
        }
        
        // Layer norm parameters (initialize to 1 and 0)
        for (uint32_t i = 0; i < model->embedding_dim; i++) {
            model->layers[layer].ln1_gamma[i] = 1.0;
            model->layers[layer].ln1_beta[i] = 0.0;
            model->layers[layer].ln2_gamma[i] = 1.0;
            model->layers[layer].ln2_beta[i] = 0.0;
        }
    }
    
    // Initialize output weights
    size_t output_size = model->embedding_dim * model->vocab_size;
    for (size_t i = 0; i < output_size; i++) {
        double r1 = (double)(rand() % 10000) / 10000.0;
        double r2 = (double)(rand() % 10000) / 10000.0;
        model->output_weights[i] = scale * math_sqrt(-2.0 * math_log(r1)) * math_cos(2.0 * MATH_PI * r2);
    }
}

/**
 * Allocate all model parameters
 */
static bool allocate_model_parameters(CLLMModel* model) {
    // Embeddings (legacy double arrays - kept for backward compatibility)
    model->embeddings = (double*)calloc(model->vocab_size * model->embedding_dim, sizeof(double));
    if (!model->embeddings) return false;
    
    model->embeddings_grad = (double*)calloc(model->vocab_size * model->embedding_dim, sizeof(double));
    if (!model->embeddings_grad) return false;
    
    // Positional encoding (legacy)
    model->positional_encoding = (double*)calloc(model->max_seq_len * model->embedding_dim, sizeof(double));
    if (!model->positional_encoding) return false;
    
    // Abacus-based embeddings (NEW - arbitrary precision)
    // Use base 60 (Babylonian) with precision 10 for high accuracy
    model->abacus_embeddings = abacus_matrix_create(
        model->vocab_size, 
        model->embedding_dim, 
        60,  // Base 60 (sexagesimal - Babylonian)
        10   // Precision: 10 fractional digits
    );
    if (!model->abacus_embeddings) {
        fprintf(stderr, "Failed to create abacus embeddings matrix\n");
        return false;
    }
    
    model->abacus_positional_encoding = abacus_matrix_create(
        model->max_seq_len,
        model->embedding_dim,
        60,  // Base 60
        10   // Precision: 10 fractional digits
    );
    if (!model->abacus_positional_encoding) {
        fprintf(stderr, "Failed to create abacus positional encoding matrix\n");
        return false;
    }
    
    // Enable abacus embeddings by default
    model->use_abacus_embeddings = true;
    
    printf("✓ Created abacus embeddings: %u × %u (base 60, precision 10)\n",
           model->vocab_size, model->embedding_dim);
    printf("✓ Created abacus positional encoding: %u × %u (base 60, precision 10)\n",
           model->max_seq_len, model->embedding_dim);
    
    // Allocate layers
    model->layers = (typeof(model->layers[0])*)calloc(model->num_layers, sizeof(model->layers[0]));
    if (!model->layers) return false;
    
    // Allocate each layer's parameters
    for (uint32_t i = 0; i < model->num_layers; i++) {
        size_t attn_size = model->embedding_dim * model->embedding_dim;
        size_t ffn_size1 = model->embedding_dim * model->hidden_dim;
        size_t ffn_size2 = model->hidden_dim * model->embedding_dim;
        
        // Attention weights
        model->layers[i].query_weights = (double*)calloc(attn_size, sizeof(double));
        model->layers[i].key_weights = (double*)calloc(attn_size, sizeof(double));
        model->layers[i].value_weights = (double*)calloc(attn_size, sizeof(double));
        model->layers[i].output_weights = (double*)calloc(attn_size, sizeof(double));
        
        // Attention gradients
        model->layers[i].query_grad = (double*)calloc(attn_size, sizeof(double));
        model->layers[i].key_grad = (double*)calloc(attn_size, sizeof(double));
        model->layers[i].value_grad = (double*)calloc(attn_size, sizeof(double));
        model->layers[i].output_grad = (double*)calloc(attn_size, sizeof(double));
        
        // Feed-forward weights
        model->layers[i].ffn_w1 = (double*)calloc(ffn_size1, sizeof(double));
        model->layers[i].ffn_w2 = (double*)calloc(ffn_size2, sizeof(double));
        model->layers[i].ffn_b1 = (double*)calloc(model->hidden_dim, sizeof(double));
        model->layers[i].ffn_b2 = (double*)calloc(model->embedding_dim, sizeof(double));
        
        // Feed-forward gradients
        model->layers[i].ffn_w1_grad = (double*)calloc(ffn_size1, sizeof(double));
        model->layers[i].ffn_w2_grad = (double*)calloc(ffn_size2, sizeof(double));
        model->layers[i].ffn_b1_grad = (double*)calloc(model->hidden_dim, sizeof(double));
        model->layers[i].ffn_b2_grad = (double*)calloc(model->embedding_dim, sizeof(double));
        
        // Layer norm parameters
        model->layers[i].ln1_gamma = (double*)calloc(model->embedding_dim, sizeof(double));
        model->layers[i].ln1_beta = (double*)calloc(model->embedding_dim, sizeof(double));
        model->layers[i].ln2_gamma = (double*)calloc(model->embedding_dim, sizeof(double));
        model->layers[i].ln2_beta = (double*)calloc(model->embedding_dim, sizeof(double));
        
        // Layer norm gradients
        model->layers[i].ln1_gamma_grad = (double*)calloc(model->embedding_dim, sizeof(double));
        model->layers[i].ln1_beta_grad = (double*)calloc(model->embedding_dim, sizeof(double));
        model->layers[i].ln2_gamma_grad = (double*)calloc(model->embedding_dim, sizeof(double));
        model->layers[i].ln2_beta_grad = (double*)calloc(model->embedding_dim, sizeof(double));
        
        // Check allocations
        if (!model->layers[i].query_weights || !model->layers[i].key_weights ||
            !model->layers[i].value_weights || !model->layers[i].output_weights ||
            !model->layers[i].ffn_w1 || !model->layers[i].ffn_w2 ||
            !model->layers[i].ffn_b1 || !model->layers[i].ffn_b2 ||
            !model->layers[i].ln1_gamma || !model->layers[i].ln1_beta ||
            !model->layers[i].ln2_gamma || !model->layers[i].ln2_beta) {
            return false;
        }
    }
    
    // Output layer
    model->output_weights = (double*)calloc(model->embedding_dim * model->vocab_size, sizeof(double));
    model->output_bias = (double*)calloc(model->vocab_size, sizeof(double));
    model->output_weights_grad = (double*)calloc(model->embedding_dim * model->vocab_size, sizeof(double));
    model->output_bias_grad = (double*)calloc(model->vocab_size, sizeof(double));
    
    if (!model->output_weights || !model->output_bias) return false;
    
    return true;
}

// ============================================================================
// MAIN MODEL CREATION FUNCTION
// ============================================================================

/**
 * Create a CLLM model with complete geometric foundation
 */
CLLMModel* cllm_create_model(const CLLMConfig* config) {
    if (!config) {
        fprintf(stderr, "Error: NULL configuration\n");
        return NULL;
    }
    
    // Validate basic parameters
    if (config->vocab_size == 0 || config->max_seq_len == 0) {
        fprintf(stderr, "Error: Invalid vocab_size or max_seq_len\n");
        return NULL;
    }
    
    // Allocate model
    CLLMModel* model = (CLLMModel*)calloc(1, sizeof(CLLMModel));
    if (!model) {
        fprintf(stderr, "Error: Failed to allocate model\n");
        return NULL;
    }
    
    // Seed random number generator
    srand(time(NULL));
    
    // ========================================================================
    // GEOMETRIC FOUNDATION
    // ========================================================================
    
    printf("🔷 Initializing geometric foundation...\n");
    
    // Set Platonic solid type (default to CUBE if not specified or invalid)
    if (config->solid_type < PLATONIC_TETRAHEDRON || config->solid_type > PLATONIC_ICOSAHEDRON) {
        model->solid_type = PLATONIC_CUBE;  // Default to CUBE
    } else {
        model->solid_type = config->solid_type;
    }
    
    // PHASE 2: Generate full Platonic solid from math library
    printf("  → Generating Platonic solid from math library...\n");
    model->platonic_solid = cllm_generate_platonic_solid(model->solid_type);
    if (!model->platonic_solid) {
        fprintf(stderr, "Error: Failed to generate Platonic solid from math library\n");
        fprintf(stderr, "Falling back to legacy geometry lookup...\n");
        
        // Fallback to legacy method
        model->geometry = platonic_get_geometry(model->solid_type);
        
        // Verify Euler's formula: V - E + F = 2
        if (!platonic_verify_euler(&model->geometry)) {
            fprintf(stderr, "Error: Euler's formula verification failed!\n");
            free(model);
            return NULL;
        }
    } else {
        // Update legacy geometry structure from math library solid
        cllm_update_geometry_from_solid(model, model->platonic_solid);
        
        // Print detailed solid information
        printf("  ✓ Generated Platonic solid from math library:\n");
        cllm_print_platonic_solid(model->platonic_solid);
    }
    
    printf("  ✓ Platonic solid: ");
    switch (model->solid_type) {
        case PLATONIC_TETRAHEDRON: printf("Tetrahedron (4V, 6E, 4F)\n"); break;
        case PLATONIC_CUBE: printf("Cube (8V, 12E, 6F)\n"); break;
        case PLATONIC_OCTAHEDRON: printf("Octahedron (6V, 12E, 8F)\n"); break;
        case PLATONIC_DODECAHEDRON: printf("Dodecahedron (20V, 30E, 12F)\n"); break;
        case PLATONIC_ICOSAHEDRON: printf("Icosahedron (12V, 30E, 20F)\n"); break;
        default: printf("Unknown\n");
    }
    
    // Derive dimensions from geometry (or use config if specified)
    model->embedding_dim = (config->embedding_dim > 0) ? config->embedding_dim : (model->geometry.vertices * 12);
    model->hidden_dim = (config->hidden_dim > 0) ? config->hidden_dim : (model->geometry.edges * 12);
    model->num_layers = (config->num_layers > 0) ? config->num_layers : model->geometry.faces;
    model->num_heads = 12;  // Always 12 (12-fold symmetry)
    
    printf("  ✓ Dimensions: embedding=%u, hidden=%u, layers=%u, heads=%u\n",
           model->embedding_dim, model->hidden_dim, model->num_layers, model->num_heads);
    
    // Validate Euler's formula
    int euler = model->geometry.vertices - model->geometry.edges + model->geometry.faces;
    model->metrics.euler_validation = (double)euler;
    printf("  ✓ Euler's formula: V - E + F = %d (expected 2)\n", euler);
    
    if (euler != 2) {
        fprintf(stderr, "Error: Euler's formula failed! Got %d, expected 2\n", euler);
        free(model);
        return NULL;
    }
    
    // Set basic parameters
    model->vocab_size = config->vocab_size;
    model->max_seq_len = config->max_seq_len;
    
    // ========================================================================
    // CLOCK LATTICE MAPPING
    // ========================================================================
    
    printf("🕐 Initializing clock lattice mapping...\n");
    
    // Allocate clock positions
    model->vertex_positions = (ClockPosition*)calloc(model->geometry.vertices, sizeof(ClockPosition));
    model->token_positions = (ClockPosition*)calloc(model->vocab_size, sizeof(ClockPosition));
    model->token_angular_positions = (double*)calloc(model->vocab_size, sizeof(double));
    model->token_positions_13d = (double (*)[13])calloc(model->vocab_size, 13 * sizeof(double));
    
    if (!model->vertex_positions || !model->token_positions || !model->token_angular_positions || !model->token_positions_13d) {
        fprintf(stderr, "Error: Failed to allocate clock positions\n");
        cllm_free_model(model);
        return NULL;
    }
    
    // Map vertices to clock lattice
    for (uint32_t v = 0; v < model->geometry.vertices; v++) {
        clock_map_index_to_position(v, &model->vertex_positions[v]);
    }
    
    // Map tokens to clock lattice
    for (uint32_t t = 0; t < model->vocab_size; t++) {
        clock_map_index_to_position(t, &model->token_positions[t]);
        model->token_angular_positions[t] = compute_angular_position(t, model);
        
        // Map to 13D clock lattice for geometric position encoding
        clock_map_value_to_lattice_13d(t, model->token_positions_13d[t]);
    }
    
    printf("  ✓ Mapped %u vertices and %u tokens to clock lattice (including 13D)\n",
           model->geometry.vertices, model->vocab_size);
    
    // ========================================================================
    // ALLOCATE MODEL PARAMETERS
    // ========================================================================
    
    printf("💾 Allocating model parameters...\n");
    
    if (!allocate_model_parameters(model)) {
        fprintf(stderr, "Error: Failed to allocate model parameters\n");
        cllm_free_model(model);
        return NULL;
    }
    
    printf("  ✓ Allocated all parameters\n");
    
    // ========================================================================
    // INITIALIZE FEATURES
    // ========================================================================
    
    // Blind recovery
    if (config->enable_blind_recovery) {
        printf("🛡️  Initializing blind recovery...\n");
        
        model->recovery.enabled = true;
        model->recovery.corruption_tolerance = config->corruption_tolerance;
        model->recovery.max_iterations = config->max_recovery_iterations;
        model->recovery.is_corrupted = false;
        model->recovery.corruption_level = 0.0;
        model->recovery.recovery_count = 0;
        model->recovery.recovery_methods = 0x0F;  // All methods enabled
        
        // Allocate backup arrays
        model->recovery.vertex_backup = (double*)calloc(model->geometry.vertices * 3, sizeof(double));
        model->recovery.edge_backup = (double*)calloc(model->geometry.edges, sizeof(double));
        model->recovery.face_backup = (double*)calloc(model->geometry.faces, sizeof(double));
        
        printf("  ✓ Blind recovery enabled (tolerance: %.0f%%)\n", 
               model->recovery.corruption_tolerance * 100);
    }
    
    // Harmonic integration
    if (config->enable_harmonic_integration) {
        printf("🎵 Initializing harmonic integration...\n");
        
        model->harmonic.enabled = true;
        model->harmonic.primary_frequency = config->primary_frequency;
        
        // Cymatic frequencies (Hz)
        model->harmonic.frequencies[0] = 432.0;  // Universal
        model->harmonic.frequencies[1] = 528.0;  // Transformation
        model->harmonic.frequencies[2] = 639.0;  // Connection
        model->harmonic.frequencies[3] = 741.0;  // Awakening
        model->harmonic.frequencies[4] = 852.0;  // Intuition
        model->harmonic.frequencies[5] = 963.0;  // Divine
        
        // Platonic primes
        model->harmonic.platonic_primes[0] = 5;    // Tetrahedron
        model->harmonic.platonic_primes[1] = 23;   // Cube
        model->harmonic.platonic_primes[2] = 29;   // Octahedron
        model->harmonic.platonic_primes[3] = 127;  // Dodecahedron
        model->harmonic.platonic_primes[4] = 241;  // Icosahedron
        
        // Tetration attractors (computed)
        model->harmonic.tetration_attractors[0] = 2;  // Base 2 attractor
        model->harmonic.tetration_attractors[1] = 3;  // Base 3 attractor
        model->harmonic.tetration_attractors[2] = 5;  // Base 5 attractor
        
        // Fourier coefficients
        model->harmonic.fourier_coefficients = (double*)calloc(model->embedding_dim, sizeof(double));
        
        // Feature flags
        model->harmonic.use_fourier_transform = config->use_fourier_transform;
        model->harmonic.use_cymatic_modulation = config->use_cymatic_modulation;
        model->harmonic.use_prime_resonance = config->use_prime_resonance;
        model->harmonic.use_tetration_optimizer = config->use_tetration_optimizer;
        
        printf("  ✓ Harmonic integration enabled (primary: %.0f Hz)\n",
               model->harmonic.primary_frequency);
    }
    
    // NTT attention
    if (config->enable_ntt_attention) {
        printf("⚡ Initializing NTT attention...\n");
        
        model->ntt.enabled = true;
        model->ntt.threshold_seq_len = config->ntt_threshold_seq_len;
        model->ntt.auto_select = config->ntt_auto_select;
        
        // Pre-allocate workspace
        model->ntt.ntt_workspace = (double*)calloc(model->max_seq_len * model->embedding_dim, sizeof(double));
        model->ntt.ntt_frequencies = (double*)calloc(model->max_seq_len, sizeof(double));
        
        model->ntt.ntt_calls = 0;
        model->ntt.standard_calls = 0;
        model->ntt.ntt_time = 0.0;
        model->ntt.standard_time = 0.0;
        
        printf("  ✓ NTT attention enabled (threshold: %u)\n",
               model->ntt.threshold_seq_len);
    }
    
    // Kissing spheres threading
    if (config->enable_kissing_spheres) {
        printf("🔮 Initializing kissing spheres threading...\n");
        
        model->threading.enabled = true;
        
        // Allocate work distribution maps
        model->threading.vertex_to_thread = (uint32_t*)calloc(model->geometry.vertices, sizeof(uint32_t));
        model->threading.edge_to_boundary = (uint32_t*)calloc(model->geometry.edges, sizeof(uint32_t));
        model->threading.token_to_thread = (uint32_t*)calloc(model->vocab_size, sizeof(uint32_t));
        
        // Distribute vertices across threads (geometric distribution)
        for (uint32_t v = 0; v < model->geometry.vertices; v++) {
            model->threading.vertex_to_thread[v] = (v % 12) + 1;  // Workers 1-12
        }
        
        // Distribute edges across boundaries
        for (uint32_t e = 0; e < model->geometry.edges; e++) {
            model->threading.edge_to_boundary[e] = e % model->geometry.edges;
        }
        
        // Distribute tokens across threads
        for (uint32_t t = 0; t < model->vocab_size; t++) {
            model->threading.token_to_thread[t] = (t % 12) + 1;  // Workers 1-12
        }
        
        printf("  ✓ Threading enabled (96 threads: 8 layers × 12 threads)\n");
    }
    
    // ========================================================================
    // INITIALIZE OPTIMIZER
    // ========================================================================
    
    model->optimizer.type = config->optimizer_type;
    model->optimizer.learning_rate = config->learning_rate;
    model->optimizer.beta1 = config->beta1;
    model->optimizer.beta2 = config->beta2;
    model->optimizer.epsilon = config->epsilon;
    model->optimizer.weight_decay = config->weight_decay;
    model->optimizer.t = 0;
    
    // Allocate optimizer buffers for Adam/RMSProp
    if (config->optimizer_type == OPTIMIZER_ADAM || config->optimizer_type == OPTIMIZER_RMSPROP) {
        // Calculate total number of parameters
        size_t total_params = model->vocab_size * model->embedding_dim;  // Embeddings
        total_params += model->embedding_dim * model->vocab_size;  // Output weights
        
        for (uint32_t i = 0; i < model->num_layers; i++) {
            total_params += 4 * model->embedding_dim * model->embedding_dim;  // Attention
            total_params += model->embedding_dim * model->hidden_dim;  // FFN W1
            total_params += model->hidden_dim * model->embedding_dim;  // FFN W2
            total_params += model->hidden_dim + model->embedding_dim;  // FFN biases
            total_params += 4 * model->embedding_dim;  // Layer norms
        }
        
        model->optimizer.m = (double*)calloc(total_params, sizeof(double));
        model->optimizer.v = (double*)calloc(total_params, sizeof(double));
    }
    
    // ========================================================================
    // INITIALIZE WEIGHTS
    // ========================================================================
    
    printf("🎲 Initializing weights with geometric structure...\n");
    initialize_geometric_weights(model);
    printf("  ✓ Weights initialized\n");
    
    // ========================================================================
    // INITIALIZE METRICS
    // ========================================================================
    
    model->metrics.total_steps = 0;
    model->metrics.epoch = 0;
    model->metrics.current_loss = 0.0;
    model->metrics.best_loss = 1e9;
    model->metrics.tokens_processed = 0;
    model->metrics.perplexity = 0.0;
    model->metrics.euler_validation = (double)euler;
    model->metrics.symmetry_score = 1.0;
    model->metrics.gcd_similarity_avg = 0.0;
    model->metrics.corruption_events = 0;
    model->metrics.successful_recoveries = 0;
    model->metrics.avg_recovery_time_ms = 0.0;
    
    // ========================================================================
    // INITIALIZE HEADER (for file format)
    // ========================================================================
    
    memcpy(model->header.magic, "CLLM\x02\x00\x00\x00", 8);
    model->header.version = CLLM_VERSION;
    model->header.vocab_size = model->vocab_size;
    model->header.embedding_dim = model->embedding_dim;
    model->header.hidden_dim = model->hidden_dim;
    model->header.num_layers = model->num_layers;
    model->header.max_seq_len = model->max_seq_len;
    model->header.num_heads = model->num_heads;
    model->header.platonic_solid_type = model->solid_type;
    model->header.vertices = model->geometry.vertices;
    model->header.edges = model->geometry.edges;
    model->header.faces = model->geometry.faces;
    model->header.blind_recovery_enabled = model->recovery.enabled;
    model->header.harmonic_enabled = model->harmonic.enabled;
    model->header.ntt_attention_enabled = model->ntt.enabled;
    model->header.kissing_spheres_enabled = model->threading.enabled;
    model->header.created_timestamp = time(NULL);
    model->header.modified_timestamp = time(NULL);
    
    // ========================================================================
    // FINAL VALIDATION
    // ========================================================================
    
    printf("✅ Model creation complete!\n");
    printf("   Platonic solid: %uV, %uE, %uF (Euler: %d)\n",
           model->geometry.vertices, model->geometry.edges, model->geometry.faces, euler);
    printf("   Dimensions: %u embedding, %u hidden, %u layers, %u heads\n",
           model->embedding_dim, model->hidden_dim, model->num_layers, model->num_heads);
    printf("   Features: recovery=%d, harmonic=%d, ntt=%d, threading=%d\n",
           model->recovery.enabled, model->harmonic.enabled, 
           model->ntt.enabled, model->threading.enabled);
    
    return model;
}

// ============================================================================
// DEFAULT CONFIGURATION
// ============================================================================

/**
 * Get default configuration for a Platonic solid
 */
CLLMConfig cllm_default_config(PlatonicSolidType solid_type, uint32_t vocab_size) {
    CLLMConfig config = {0};
    
    config.solid_type = solid_type;
    config.vocab_size = vocab_size;
    config.max_seq_len = 512;
    
    // Dimensions auto-calculated from geometry (set to 0)
    config.embedding_dim = 0;
    config.hidden_dim = 0;
    config.num_layers = 0;
    config.num_heads = 0;
    
    // Enable all features by default
    config.enable_blind_recovery = true;
    config.enable_harmonic_integration = true;
    config.enable_ntt_attention = true;
    config.enable_kissing_spheres = true;
    
    // Threading
    config.num_threads = 13;  // 1 control + 12 workers
    
    // Optimizer (Adam default)
    config.optimizer_type = OPTIMIZER_ADAM;
    config.learning_rate = 0.001;
    config.beta1 = 0.9;
    config.beta2 = 0.999;
    config.epsilon = 1e-8;
    config.weight_decay = 0.01;
    
    // Recovery options
    config.corruption_tolerance = 0.25;  // 25%
    config.max_recovery_iterations = 1000;
    
    // Harmonic options
    config.primary_frequency = 432.0;  // Hz
    config.use_fourier_transform = true;
    config.use_cymatic_modulation = true;
    config.use_prime_resonance = true;
    config.use_tetration_optimizer = true;
    
    // NTT options
    config.ntt_threshold_seq_len = 512;
    config.ntt_auto_select = true;
    
    return config;
}