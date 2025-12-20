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
#include "math/clock.h"
#include "math/clock_lattice_13d.h"
#include "ai/cllm.h"
#include "ai/cllm_platonic.h"
#include "ai/cllm_generic_interface.h"
#include "hierarchical_threading.h"
#include "thread_parameters.h"
#include "pthread_barrier_compat.h"  // For macOS-compatible barrier functions
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
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
    
    // TODO: Initialize embeddings in thread-local CrystallineAbacus storage
    // For now, embeddings are initialized when threads are created
    // This will be properly implemented when we add weight initialization to threads
    
    (void)scale;  // Suppress unused warning
    
    printf("  Note: Weight initialization now happens in thread-local storage\n");
    printf("  Embeddings and layer weights are stored in thread CrystallineAbacus\n");
}

/**
 * Allocate all model parameters
 */
static bool allocate_model_parameters(CLLMModel* model) {
    // ========================================================================
    // STEP 1: CREATE 88D THREAD POOL (MANDATORY)
    // ========================================================================
    
    printf("  → Creating thread pool (MANDATORY)...\n");
    model->threads = hierarchical_thread_pool_create(60);  // Base 60 for CrystallineAbacus
    if (!model->threads) {
        fprintf(stderr, "\n");
        fprintf(stderr, "╔════════════════════════════════════════════════════════╗\n");
        fprintf(stderr, "║        FATAL ERROR: THREAD POOL CREATION FAILED         ║\n");
        fprintf(stderr, "╚════════════════════════════════════════════════════════╝\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "Threading is MANDATORY in this architecture.\n");
        fprintf(stderr, "There is NO sequential fallback.\n");
        fprintf(stderr, "Model creation cannot proceed without 88D thread pool.\n");
        fprintf(stderr, "\n");
        abort();  // CRITICAL: Abort immediately - no fallback possible
    }
    
    // Verify thread pool is valid
    if (model->threads->num_threads != 96) {
        fprintf(stderr, "FATAL ERROR: Thread pool has wrong size (%u, expected 96)\n",
                model->threads->num_threads);
        hierarchical_thread_pool_free(model->threads);
        model->threads = NULL;
        fprintf(stderr, "Threading is MANDATORY - model creation cannot proceed.\n");
        abort();  // CRITICAL: Abort immediately - no fallback possible
    }
    
    printf("  ✓ Created thread pool: 96 threads (8 layers × 12 threads per layer)\n");
    
    // Create generic model interface (NO circular dependency!)
    GenericModel* generic = cllm_create_generic_interface(model);
    if (!generic) {
        fprintf(stderr, "FATAL: Failed to create generic model interface\n");
        return false;
    }
    
    // Set generic interface in all threads (for worker loop access)
    for (uint32_t i = 0; i < 96; i++) {
        HierarchicalThread* thread = model->threads->threads[i];
        if (thread) {
            thread->model = generic;  // Generic interface, not CLLMModel directly
        }
    }
    
    // Store generic interface in model for cleanup
    model->generic_interface = generic;
    
    printf("  ✓ Created generic model interface (no circular dependency)\n");
    
    // ========================================================================
    // STEP 2: ALLOCATE TOKEN ASSIGNMENTS (PERMANENT)
    // ========================================================================
    
    printf("  → Allocating token → thread assignments...\n");
    model->token_assignments = calloc(model->vocab_size, sizeof(*model->token_assignments));
    if (!model->token_assignments) {
        fprintf(stderr, "Failed to allocate token assignments\n");
        return false;
    }
    
    // Assign each token to a thread permanently
    for (uint32_t token_id = 0; token_id < model->vocab_size; token_id++) {
        // Deterministic assignment based on token ID
        uint8_t layer = token_id % 8;
        uint8_t dimension = (token_id / 8) % 11 + 1;  // 1-11 (skip 0 which is control)
        
        model->token_assignments[token_id].layer = layer;
        model->token_assignments[token_id].dimension = dimension;
        model->token_assignments[token_id].thread_id = layer * 12 + dimension;
        
        // Get direct pointer to thread
        model->token_assignments[token_id].thread = 
            hierarchical_thread_get(model->threads, layer, dimension);
        
        if (!model->token_assignments[token_id].thread) {
            fprintf(stderr, "Failed to get thread [%d][%d] for token %u\n", 
                    layer, dimension, token_id);
            return false;
        }
    }
    
    printf("  ✓ Assigned %u tokens to threads (deterministic mapping)\n", model->vocab_size);
    
    // ========================================================================
    // STEP 3: ALLOCATE THREAD PARAMETERS
    // ========================================================================
    
    printf("  → Allocating thread parameters...\n");
    model->thread_params = calloc(96, sizeof(*model->thread_params));
    if (!model->thread_params) {
        fprintf(stderr, "Failed to allocate thread parameters\n");
        return false;
    }
    
    // Initialize thread parameters
    for (uint8_t layer = 0; layer < 8; layer++) {
        for (uint8_t dim = 0; dim <= 11; dim++) {
            uint32_t thread_idx = layer * 12 + dim;
            
            model->thread_params[thread_idx].layer_id = layer;
            model->thread_params[thread_idx].is_control_thread = (dim == 0);
            model->thread_params[thread_idx].is_worker_thread = (dim >= 1 && dim <= 11);
            model->thread_params[thread_idx].num_tokens_assigned = 0;
            model->thread_params[thread_idx].token_ids = NULL;
        }
    }
    
    // Count tokens per thread and allocate token ID arrays
    for (uint32_t token_id = 0; token_id < model->vocab_size; token_id++) {
        uint32_t thread_idx = model->token_assignments[token_id].thread_id;
        model->thread_params[thread_idx].num_tokens_assigned++;
    }
    
    // Allocate token ID arrays
    for (uint32_t thread_idx = 0; thread_idx < 96; thread_idx++) {
        if (model->thread_params[thread_idx].num_tokens_assigned > 0) {
            model->thread_params[thread_idx].token_ids = 
                calloc(model->thread_params[thread_idx].num_tokens_assigned, sizeof(uint32_t));
            if (!model->thread_params[thread_idx].token_ids) {
                fprintf(stderr, "Failed to allocate token IDs for thread %u\n", thread_idx);
                return false;
            }
        }
    }
    
    // Fill token ID arrays
    uint32_t* token_counts = calloc(96, sizeof(uint32_t));
    for (uint32_t token_id = 0; token_id < model->vocab_size; token_id++) {
        uint32_t thread_idx = model->token_assignments[token_id].thread_id;
        uint32_t idx = token_counts[thread_idx]++;
        model->thread_params[thread_idx].token_ids[idx] = token_id;
    }
    free(token_counts);
    
    printf("  ✓ Allocated thread parameters for 96 threads\n");
    
    // ========================================================================
    // STEP 4: ALLOCATE LAYER INFO
    // ========================================================================
    
    printf("  → Allocating layer info...\n");
    model->layer_info = calloc(model->num_layers, sizeof(*model->layer_info));
    if (!model->layer_info) {
        fprintf(stderr, "Failed to allocate layer info\n");
        return false;
    }
    
    for (uint32_t layer = 0; layer < model->num_layers; layer++) {
        // Get control thread for this layer
        model->layer_info[layer].control_thread = 
            hierarchical_thread_get(model->threads, layer, 0);
        
        // Allocate worker thread array
        model->layer_info[layer].worker_threads = calloc(11, sizeof(HierarchicalThread*));
        if (!model->layer_info[layer].worker_threads) {
            fprintf(stderr, "Failed to allocate worker threads for layer %u\n", layer);
            return false;
        }
        
        // Get worker threads
        for (uint8_t dim = 1; dim <= 11; dim++) {
            model->layer_info[layer].worker_threads[dim - 1] = 
                hierarchical_thread_get(model->threads, layer, dim);
        }
    }
    
    printf("  ✓ Allocated layer info for %u layers\n", model->num_layers);
    
    // ========================================================================
    // STEP 5: INITIALIZE THREADING BARRIERS
    // ========================================================================
    
    printf("  → Initializing threading barriers...\n");
    
    model->threading.forward_barrier = malloc(sizeof(pthread_barrier_t_compat));
    model->threading.backward_barrier = malloc(sizeof(pthread_barrier_t_compat));
    model->threading.optimizer_barrier = malloc(sizeof(pthread_barrier_t_compat));
    
    if (!model->threading.forward_barrier || !model->threading.backward_barrier || 
        !model->threading.optimizer_barrier) {
        fprintf(stderr, "Failed to allocate threading barriers\n");
        return false;
    }
    
    // Initialize barriers for 96 threads (88 workers + 8 control) - macOS compatible
    pthread_barrier_init_compat(model->threading.forward_barrier, NULL, 96);
    pthread_barrier_init_compat(model->threading.backward_barrier, NULL, 96);
    pthread_barrier_init_compat(model->threading.optimizer_barrier, NULL, 96);
    
    printf("  ✓ Initialized threading barriers\n");
    
    // ========================================================================
    // STEP 6: DISTRIBUTE PARAMETERS TO THREADS (NEW - DAY 5)
    // ========================================================================
    
    printf("  → Distributing parameters to threads...\n");
    
    // Import thread parameter functions
    extern int thread_allocate_all_parameters(
        HierarchicalThread* thread,
        uint32_t embedding_dim,
        uint32_t hidden_dim,
        uint32_t vocab_size,
        uint32_t num_tokens_assigned
    );
    
    extern int thread_initialize_all_parameters(
        HierarchicalThread* thread,
        ParameterInitMethod method
    );
    
    // Distribute parameters to each thread based on its role
    for (uint8_t layer = 0; layer < 8; layer++) {
        for (uint8_t dim = 0; dim <= 11; dim++) {
            uint32_t thread_idx = layer * 12 + dim;
            HierarchicalThread* thread = hierarchical_thread_get(model->threads, layer, dim);
            
            if (!thread) {
                fprintf(stderr, "Failed to get thread [%d][%d]\n", layer, dim);
                return false;
            }
            
            // Get number of tokens assigned to this thread
            uint32_t num_tokens = model->thread_params[thread_idx].num_tokens_assigned;
            
            // Allocate parameters based on thread role
            int result = thread_allocate_all_parameters(
                thread,
                model->embedding_dim,
                model->hidden_dim,
                model->vocab_size,
                num_tokens
            );
            
            if (result != 0) {
                fprintf(stderr, "Failed to allocate parameters for thread [%d][%d]\n", layer, dim);
                return false;
            }
            
            // Initialize parameters using Xavier initialization
            result = thread_initialize_all_parameters(thread, PARAM_INIT_XAVIER);
            if (result != 0) {
                fprintf(stderr, "Failed to initialize parameters for thread [%d][%d]\n", layer, dim);
                return false;
            }
        }
    }
    
    printf("  ✓ Distributed parameters to 96 threads\n");
    printf("  ✓ Initialized all parameters using Xavier initialization\n");
    
    // ========================================================================
    // LEGACY REMOVED: No flat arrays allocated
    // ========================================================================
    
    printf("  ✓ Thread-centric architecture initialized\n");
    printf("  ✓ All parameters stored in thread CrystallineAbacus\n");
    
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
        
        // PHASE 2: NTT workspace removed - computation now in thread-local storage
        // Each thread performs NTT in its own CrystallineAbacus temp storage
        // model->ntt.ntt_workspace = (double*)calloc(model->max_seq_len * model->embedding_dim, sizeof(double));
        // model->ntt.ntt_frequencies = (double*)calloc(model->max_seq_len, sizeof(double));
        
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
        
        // Threading is now ALWAYS enabled via threads
        // Token assignments are in model->token_assignments (already allocated)
        
        // Allocate work distribution maps for geometric operations
        model->threading.vertex_to_thread = (uint32_t*)calloc(model->geometry.vertices, sizeof(uint32_t));
        model->threading.edge_to_boundary = (uint32_t*)calloc(model->geometry.edges, sizeof(uint32_t));
        
        // Distribute vertices across threads (geometric distribution)
        for (uint32_t v = 0; v < model->geometry.vertices; v++) {
            model->threading.vertex_to_thread[v] = (v % 12) + 1;  // Workers 1-12
        }
        
        // Distribute edges across boundaries
        for (uint32_t e = 0; e < model->geometry.edges; e++) {
            model->threading.edge_to_boundary[e] = e % model->geometry.edges;
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
        
        // TODO: Optimizer state will be stored in thread-local storage
        // For now, we skip this allocation as parameters are in threads
        (void)total_params;  // Suppress unused warning
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
    model->header.kissing_spheres_enabled = (model->threads != NULL);
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
           model->ntt.enabled, (model->threads != NULL));
    
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