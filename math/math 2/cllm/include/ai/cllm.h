/*
 * CLLM - Crystalline Lattice Language Model
 * Core type definitions and structures
 * 
 * COMPLETE GEOMETRIC TRANSFORMATION
 * - Platonic solid foundation (all 5 solids)
 * - Clock lattice mapping (clock lattice)
 * - Blind recovery (25% corruption tolerance)
 * - Harmonic integration (cymatic frequencies)
 * - NTT attention (O(n log n))
 * - Kissing spheres threading (12-fold symmetry)
 * - GCD-based similarity
 * - Angular positions θ(n,k,λ,ω,ψ)
 */

#ifndef CLLM_H
#define CLLM_H

#include <stdint.h>
#include <stdbool.h>

// Include NEW math library types FIRST to prevent conflicts
#include "math/types.h"

// Include clock lattice for complete type definition
// (Will use NEW types if MATH_TYPES_H is defined)
#include "math/clock.h"

// Include CrystallineAbacus matrix utilities
#include "cllm_abacus_matrix.h"

// Include optimizer types from algorithms layer
#include "optimizers.h"

// Include hierarchical threading for 88D integration
#include "hierarchical_threading.h"

// Include vocabulary
#include "cllm_vocabulary.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// TYPE ALIASES
// ============================================================================

// Convenient aliases for commonly used types
// Note: ClockPosition is defined in math/clock.h
// ClockPosition is now defined in math/clock.h
typedef struct SphereThreadingModel SphereThreadingModel;
typedef struct Sphere SphereThread;

// ============================================================================
// MAGIC NUMBERS & CONSTANTS
// ============================================================================

#define CLLM_MAGIC 0x434C4C4D  // "CLLM"
#define CLLM_VERSION 2          // Version 2.0 - Complete geometric transformation

// Maximum sizes
#define MAX_MODEL_NAME 256
#define MAX_DESCRIPTION 1024
#define MAX_NEIGHBORS 12        // 12-fold symmetry (kissing spheres)

// Geometric constants
#define GOLDEN_RATIO 1.618033988749895
#define NUM_CYMATIC_FREQUENCIES 6
#define NUM_PLATONIC_PRIMES 5
#define NUM_TETRATION_BASES 3

// ============================================================================
// PLATONIC SOLID TYPES
// ============================================================================

/**
 * The five Platonic solids
 * Each defines a complete model architecture
 */
typedef enum {
    PLATONIC_TETRAHEDRON = 0,   // 4V, 6E, 4F  - Small, fast (48-dim)
    PLATONIC_CUBE = 1,          // 8V, 12E, 6F - Balanced (96-dim)
    PLATONIC_OCTAHEDRON = 2,    // 6V, 12E, 8F - Dual of cube (72-dim)
    PLATONIC_DODECAHEDRON = 3,  // 20V, 30E, 12F - Large, powerful (240-dim)
    PLATONIC_ICOSAHEDRON = 4    // 12V, 30E, 20F - Maximum symmetry (144-dim)
} PlatonicSolidType;

/**
 * Geometric properties of a Platonic solid
 * Satisfies Euler's formula: V - E + F = 2
 */
typedef struct {
    uint32_t vertices;          // Number of vertices (V)
    uint32_t edges;             // Number of edges (E)
    uint32_t faces;             // Number of faces (F)
    uint32_t symmetries;        // Size of symmetry group
    double edge_length;         // Normalized edge length
    bool has_golden_ratio;      // True for dodecahedron/icosahedron
} PlatonicGeometry;

// ============================================================================
// LAYER NORMALIZATION
// ============================================================================

/**
 * Layer Normalization structure
 */
typedef struct {
    uint32_t dim;           // Dimension to normalize
    float epsilon;          // Small constant for numerical stability
    double* gamma;          // Scale parameters [dim]
    double* beta;           // Shift parameters [dim]
} CLLMLayerNorm;

// ============================================================================
// OPTIMIZER TYPES
// ============================================================================
// Note: OptimizerType is now defined in algorithms/include/optimizers.h
// We include it here for compatibility

// ============================================================================
// FILE FORMAT STRUCTURES (for serialization)
// ============================================================================

/**
 * CLLM Header - File format header
 */
typedef struct {
    char magic[8];              // Magic string "CLLM\x02\x00\x00\x00"
    uint32_t version;           // Format version (2)
    char model_name[MAX_MODEL_NAME];
    char description[MAX_DESCRIPTION];
    
    // Basic dimensions
    uint64_t vocab_size;
    uint64_t embedding_dim;
    uint64_t hidden_dim;
    uint64_t num_layers;
    uint64_t max_seq_len;
    uint32_t num_heads;         // Always 12
    
    // Geometric configuration
    uint32_t platonic_solid_type; // PlatonicSolidType
    uint32_t vertices;
    uint32_t edges;
    uint32_t faces;
    
    // Feature flags
    uint8_t blind_recovery_enabled;
    uint8_t harmonic_enabled;
    uint8_t ntt_attention_enabled;
    uint8_t kissing_spheres_enabled;
    
    // Timestamps
    uint64_t created_timestamp;
    uint64_t modified_timestamp;
    
    // Metrics
    uint64_t total_params;
    double best_loss;
    uint64_t training_steps;
    
    uint8_t reserved[128];      // Reserved for future use
} CLLMHeader;

/**
 * CLLM Token - Token with prime encoding and clock position
 */
typedef struct {
    uint32_t token_id;
    uint64_t prime_encoding;    // Prime number encoding
    double lattice_coords[3];   // 3D lattice coordinates
    double angle;               // Angular position
    double radius;              // Radial distance
    char token_str[64];
    double frequency;
    uint32_t symmetry_group;    // 0 to 11 (12-fold)
    uint8_t reserved[20];
} CLLMToken;

// ============================================================================
// CORE MODEL STRUCTURE
// ============================================================================

/**
 * CLLMModel - Complete Crystalline Language Model
 * 
 * GEOMETRIC FOUNDATION:
 * - Based on Platonic solids (5 perfect forms)
 * - Dimensions derived from geometry: embedding_dim = V×12, hidden_dim = E×12, num_layers = F
 * - Clock lattice mapping for all tokens
 * - 12-fold symmetry throughout (kissing spheres)
 * 
 * REVOLUTIONARY FEATURES:
 * - Blind recovery: 25% corruption tolerance using Euler's formula
 * - Harmonic integration: Cymatic frequencies, Fourier transforms, prime resonance
 * - NTT attention: O(n log n) complexity for long sequences (10-100x speedup)
 * - Kissing spheres threading: Geometric work distribution
 * - GCD similarity: Prime-based token relationships
 * - Angular positions: θ(n,k,λ,ω,ψ) formula
 */
typedef struct {
    // ========================================================================
    // GEOMETRIC FOUNDATION
    // ========================================================================
    
    PlatonicSolidType solid_type;    // Which Platonic solid (CUBE default)
    PlatonicGeometry geometry;       // Complete geometric properties (legacy)
    
    // NEW: Full Platonic solid from math library (Phase 2)
    void* platonic_solid;            // PlatonicSolid* from math/platonic_generator.h
                                     // (void* to avoid circular dependency)
    
    // Dimensions DERIVED from geometry (automatic calculation)
    uint32_t embedding_dim;          // vertices × 12 (12-fold symmetry)
    uint32_t hidden_dim;             // edges × 12
    uint32_t num_layers;             // faces
    uint32_t num_heads;              // Always 12 (kissing spheres)
    
    // ========================================================================
    // CLOCK LATTICE MAPPING
    // ========================================================================
    
    // Map vertices to clock lattice
    ClockPosition* vertex_positions; // [vertices] - clock positions for vertices
    
    // Map tokens to clock lattice
    ClockPosition* token_positions;  // [vocab_size] - clock positions for tokens
    double* token_angular_positions; // [vocab_size] - θ(n,k,λ,ω,ψ) angular positions
    double (*token_positions_13d)[13]; // [vocab_size][13] - 13D clock lattice positions
    
    // ========================================================================
    // MODEL PARAMETERS
    // ========================================================================
    
    uint32_t vocab_size;
    uint32_t max_seq_len;
    
    // Vocabulary (integrated)
    CLLMVocabulary* vocabulary;      // Token vocabulary with save/load support
    
    // ========================================================================
    // THREAD-CENTRIC ARCHITECTURE (MANDATORY)
    // ========================================================================
    
    // Thread Pool (MANDATORY - not optional)
    // This is THE core of the model - everything else is organized around it
    HierarchicalThreadPool* threads;  // 96 threads (8 layers × 12 threads per layer)
    
    // Token → Thread Assignment (PERMANENT)
    // Every token is permanently assigned to a specific thread
    // The thread owns the token's embedding in its CrystallineAbacus
    struct {
        uint8_t layer;               // Layer (0-7)
        uint8_t dimension;           // Dimension (1-11, 0 is control)
        uint32_t thread_id;          // Absolute thread ID (0-95)
        HierarchicalThread* thread;  // Direct pointer to thread
    } *token_assignments;            // [vocab_size]
    
    // Thread Parameters (REPLACES flat arrays)
    // Each thread owns its parameters in CrystallineAbacus
    struct {
        // Token ownership
        uint32_t num_tokens_assigned;  // How many tokens this thread owns
        uint32_t* token_ids;           // Which tokens this thread owns [num_tokens_assigned]
        
        // Layer information
        uint8_t layer_id;              // Which transformer layer (0-7, 255 if not in layer)
        bool is_control_thread;        // Control thread (dimension 0)
        bool is_worker_thread;         // Worker thread (dimension 1-11)
        
    } *thread_params;                // [96 threads]
    
    // LEGACY REMOVED: All embeddings now stored in thread CrystallineAbacus
    // DELETED: double* embeddings;
    // DELETED: double* positional_encoding;
    // DELETED: AbacusMatrix* abacus_embeddings;
    // DELETED: AbacusMatrix* abacus_positional_encoding;
    // DELETED: bool use_abacus_embeddings;
    
    // ========================================================================
    // LAYER PARAMETERS (THREAD-ORGANIZED)
    // ========================================================================
    
    // Layer Control Threads
    // Each layer has a control thread (dimension 0) that coordinates the layer
    // Layer parameters are distributed across the layer's worker threads
    struct {
        HierarchicalThread* control_thread;  // Control thread for this layer
        HierarchicalThread** worker_threads; // Worker threads [11] (dimensions 1-11)
        
        // Layer-wide parameters (stored in control thread)
        // These are accessed via the control thread's CrystallineAbacus
        // - Query/Key/Value projection matrices distributed across workers
        // - FFN weights distributed across workers
        // - Layer norm parameters in control thread
        
    } *layer_info;                   // [num_layers]
    
    // LEGACY REMOVED: All layer parameters now in threads
    // DELETED: struct { double* query_weights; ... } *layers;
    // DELETED: double* output_weights;
    // DELETED: double* output_bias;
    // DELETED: double* embeddings_grad;
    // All parameters now stored in thread CrystallineAbacus
    // All gradients now stored in thread accumulators
    
    // ========================================================================
    // BLIND RECOVERY (OBJECTIVE 26)
    // ========================================================================
    
    struct {
        bool enabled;
        double corruption_tolerance;  // Max corruption % (up to 25%)
        uint32_t max_iterations;
        
        // Recovery state
        bool is_corrupted;
        double corruption_level;
        uint64_t last_recovery_time_ns;
        uint32_t recovery_count;
        
        // Backup for recovery
        // TODO Phase 3: Make thread-aware - each thread should backup its own parameters
        double* vertex_backup;        // TEMPORARY - Backup of vertex-related weights
        double* edge_backup;          // TEMPORARY - Backup of edge-related weights
        double* face_backup;          // TEMPORARY - Backup of face-related weights
        
        // Recovery methods (bit flags)
        // 0x01 = structural (Euler's formula)
        // 0x02 = symmetry-based
        // 0x04 = prime-based
        // 0x08 = tetration-based
        uint32_t recovery_methods;
        
    } recovery;
    
    // ========================================================================
    // HARMONIC INTEGRATION (OBJECTIVE 27)
    // ========================================================================
    
    struct {
        bool enabled;
        
        // Cymatic frequencies (Hz)
        double frequencies[NUM_CYMATIC_FREQUENCIES]; // 432, 528, 639, 741, 852, 963
        double primary_frequency;     // 432 Hz (universal)
        
        // Fourier coefficients
        // TODO Phase 3: Move to thread-local storage
        double* fourier_coefficients; // TEMPORARY - [embedding_dim]
        
        // Prime resonance (Platonic primes)
        uint32_t platonic_primes[NUM_PLATONIC_PRIMES]; // 5, 23, 29, 127, 241
        
        // Tetration attractors (bases 2, 3, 5)
        uint64_t tetration_attractors[NUM_TETRATION_BASES];
        
        // Feature flags
        bool use_fourier_transform;
        bool use_cymatic_modulation;
        bool use_prime_resonance;
        bool use_tetration_optimizer;
        
    } harmonic;
    
    // ========================================================================
    // NTT ATTENTION (OBJECTIVE 13D)
    // ========================================================================
    
    struct {
        bool enabled;
        uint32_t threshold_seq_len;   // Use NTT if seq_len > threshold (default: 512)
        bool auto_select;             // Automatically select NTT for long sequences
        
        // NTT workspace removed - computation now in thread-local storage
        // Each thread performs NTT in its own CrystallineAbacus temp storage
        
        // Statistics
        uint64_t ntt_calls;           // Number of times NTT was used
        uint64_t standard_calls;      // Number of times standard attention was used
        double ntt_time;              // Total time in NTT attention
        double standard_time;         // Total time in standard attention
        
    } ntt;
    
    // ========================================================================
    // TRAINING CONTEXT (for backward pass)
    // ========================================================================
    
    struct {
        bool enabled;                    // Training mode flag
        uint32_t max_batch_size;        // Maximum batch size
        uint32_t max_seq_len;           // Maximum sequence length
        
        // Layer cache removed - all activations now in thread-local storage
        // Each thread stores Q, K, V, attention_weights, attn_output in its own:
        // - thread->activation_buffer (forward pass activations)
        // - thread->cached_qkv (cached Q, K, V for backward pass)
        // - thread->value (current computation)
        // - thread->temp (temporary computation space)
        
        // Gradient accumulation
        uint32_t gradient_accumulation_steps;
        uint32_t current_accumulation_step;
        
        // Statistics
        uint64_t forward_passes;
        uint64_t backward_passes;
        
    } training;
    
    // ========================================================================
    // 88D THREADING STATISTICS & CONFIGURATION
    // ========================================================================
    
    struct {
        // Threading is ALWAYS enabled (not optional)
        // threads is now a direct field above, not in this struct
        
        // Geometric mapping (automatic from Platonic solid)
        uint32_t* vertex_to_thread;   // Map vertices to threads [vertices]
        uint32_t* edge_to_boundary;   // Map edges to shared boundaries [edges]
        uint32_t* face_to_layer;      // Map faces to layers [faces]
        
        // Work distribution statistics
        uint64_t total_work_units;    // Total work units processed
        uint64_t work_stolen;         // Work units stolen by other threads
        double parallel_efficiency;   // Actual speedup / ideal speedup
        double load_balance_score;    // Load balance quality (0-1)
        
        // Thread synchronization barriers
        pthread_barrier_t* forward_barrier;   // Forward pass synchronization
        pthread_barrier_t* backward_barrier;  // Backward pass synchronization
        pthread_barrier_t* optimizer_barrier; // Optimizer synchronization
        
    } threading;
    
    // ========================================================================
    // OPTIMIZER STATE
    // ========================================================================
    
    struct {
        OptimizerType type;           // SGD, ADAM, RMSPROP, ADAGRAD
        double learning_rate;
        double beta1, beta2;          // For Adam
        double epsilon;
        double weight_decay;
        
        // Momentum/velocity buffers (for Adam/RMSProp)
        // LEGACY REMOVED: Now stored in thread CrystallineAbacus
        // Each thread maintains its own momentum/velocity in temp storage
        // DELETED: double* m;
        // DELETED: double* v;
        uint64_t t;                   // Time step (global)
        
        // Tetration-based learning rate schedule (if harmonic enabled)
        bool use_tetration_schedule;
        double tetration_base;
        
    } optimizer;
    
    // ========================================================================
    // TRAINING STATE (GLOBAL COORDINATION ONLY)
    // ========================================================================
    
    struct {
        uint64_t step;                  // Global training step
        double learning_rate;           // Current learning rate
        OptimizerType optimizer_type;   // Optimizer type (duplicated from optimizer for convenience)
        double beta1, beta2;            // Adam parameters (duplicated from optimizer)
        double epsilon;                 // Numerical stability (duplicated from optimizer)
        double weight_decay;            // L2 regularization (duplicated from optimizer)
        
        // Learning rate schedule
        bool use_lr_schedule;           // Use learning rate scheduling
        double lr_decay_rate;           // Learning rate decay rate
        uint64_t lr_decay_steps;        // Steps between decay
        double min_lr;                  // Minimum learning rate
        
        // Gradient clipping
        bool use_grad_clip;             // Use gradient clipping
        double grad_clip_norm;          // Maximum gradient norm
        
    } training_state;
    
    // ========================================================================
    // TRAINING METRICS
    // ========================================================================
    
    struct {
        uint64_t total_steps;
        uint64_t epoch;
        double current_loss;
        double best_loss;
        uint64_t tokens_processed;
        double perplexity;
        
        // Geometric metrics
        double euler_validation;      // V - E + F (should be 2.0)
        double symmetry_score;        // Geometric symmetry preservation (0-1)
        double gcd_similarity_avg;    // Average GCD-based similarity
        
        // Performance metrics
        double tokens_per_second;
        double memory_usage_mb;
        double cache_hit_rate;
        
        // Recovery metrics
        uint32_t corruption_events;
        uint32_t successful_recoveries;
        double avg_recovery_time_ms;
        
    } metrics;
    
    // ========================================================================
    // FILE FORMAT COMPATIBILITY
    // ========================================================================
    
    CLLMHeader header;               // File format header
    CLLMToken* tokens;               // Token array (for serialization)
    
    // ========================================================================
    // GENERIC MODEL INTERFACE (for threading system)
    // ========================================================================
    
    void* generic_interface;         // GenericModel* (void* to avoid circular dependency)
                                     // This wraps the CLLMModel for use by worker threads
    
} CLLMModel;

// ============================================================================
// CONFIGURATION STRUCTURE
// ============================================================================

/**
 * CLLMConfig - Configuration for creating a model
 */
typedef struct {
    // Geometric foundation
    PlatonicSolidType solid_type;    // Which Platonic solid (CUBE default)
    
    // Basic parameters
    uint32_t vocab_size;
    uint32_t max_seq_len;
    
    // Dimensions (auto-calculated from solid_type if set to 0)
    uint32_t embedding_dim;          // 0 = auto (vertices × 12)
    uint32_t hidden_dim;             // 0 = auto (edges × 12)
    uint32_t num_layers;             // 0 = auto (faces)
    uint32_t num_heads;              // 0 = auto (always 12)
    
    // Feature flags
    bool enable_blind_recovery;
    bool enable_harmonic_integration;
    bool enable_ntt_attention;
    bool enable_kissing_spheres;
    
    // Threading
    int num_threads;                 // 0 = auto (13 for kissing spheres)
    
    // Optimizer
    OptimizerType optimizer_type;
    double learning_rate;
    double beta1, beta2;             // For Adam
    double epsilon;
    double weight_decay;
    
    // Recovery options
    double corruption_tolerance;     // Max corruption % (default 0.25)
    uint32_t max_recovery_iterations;
    
    // Harmonic options
    double primary_frequency;        // Default 432 Hz
    bool use_fourier_transform;
    bool use_cymatic_modulation;
    bool use_prime_resonance;
    bool use_tetration_optimizer;
    
    // NTT options
    uint32_t ntt_threshold_seq_len;  // Default 512
    bool ntt_auto_select;
    
} CLLMConfig;

// ============================================================================
// API FUNCTIONS (declarations only - implementations in src/)
// ============================================================================

/**
 * Create a CLLM model with geometric foundation
 */
CLLMModel* cllm_create_model(const CLLMConfig* config);

/**
 * Free a CLLM model
 */
void cllm_free_model(CLLMModel* model);

/**
 * Validate model integrity (Euler's formula, symmetry, etc.)
 */
bool cllm_validate_model(const CLLMModel* model);

/**
 * Save model to file
 */
bool cllm_save_model(const CLLMModel* model, const char* filename);

/**
 * Load model from file
 */
CLLMModel* cllm_load_model(const char* filename);

/**
 * Get default configuration for a Platonic solid
 */
CLLMConfig cllm_default_config(PlatonicSolidType solid_type, uint32_t vocab_size);

// ============================================================================
// VOCABULARY INTEGRATION
// ============================================================================

/**
 * Set vocabulary for model
 * Takes ownership of the vocabulary
 */
void cllm_set_vocabulary(CLLMModel* model, CLLMVocabulary* vocab);

/**
 * Get vocabulary from model
 * Returns pointer to internal vocabulary (do not free)
 */
CLLMVocabulary* cllm_get_vocabulary(CLLMModel* model);

/**
 * Build vocabulary from training file
 * Returns number of tokens added
 */
uint32_t cllm_build_vocabulary_from_file(CLLMModel* model, const char* filename);

/**
 * Build vocabulary from multiple files
 */
uint32_t cllm_build_vocabulary_from_files(CLLMModel* model, const char** filenames, uint32_t num_files);

/**
 * Save model vocabulary to file
 */
bool cllm_save_vocabulary(CLLMModel* model, const char* filename);

/**
 * Load vocabulary into model
 */
bool cllm_load_vocabulary(CLLMModel* model, const char* filename);

/**
 * Tokenize text using model's vocabulary
 * Returns allocated array of token IDs (caller must free)
 */
uint32_t* cllm_tokenize_text(CLLMModel* model, const char* text, uint32_t* num_tokens);

/**
 * Detokenize token IDs using model's vocabulary
 * Returns allocated string (caller must free)
 */
char* cllm_detokenize_text(CLLMModel* model, const uint32_t* token_ids, uint32_t num_tokens);

// ============================================================================
// BLIND RECOVERY SYSTEM (OBJECTIVE 26)
// ============================================================================

/**
 * Recovery statistics structure
 */
typedef struct {
    int total_corruptions;
    int successful_recoveries;
    int failed_recoveries;
    double recovery_rate;
    int best_method;
} RecoveryStats;

/**
 * Detect corruption in model parameters
 * @return Number of corrupted parameters detected
 */
int cllm_detect_corruption(CLLMModel* model);

/**
 * 26A: Structural Redundancy Recovery (Euler's formula)
 */
int cllm_recover_structural(CLLMModel* model);

/**
 * 26B: Symmetry-Based Reconstruction
 */
int cllm_recover_symmetry(CLLMModel* model);

/**
 * 26C: Prime-Based Validation
 */
int cllm_recover_prime(CLLMModel* model);

/**
 * 26D: Tetration Attractors
 */
int cllm_recover_tetration(CLLMModel* model);

/**
 * Comprehensive blind recovery (applies all methods)
 */
RecoveryStats cllm_blind_recovery(CLLMModel* model);

/**
 * Create backup of model parameters
 */
int cllm_create_backup(CLLMModel* model);

/**
 * Simulate corruption for testing (corrupts random parameters)
 */
int cllm_simulate_corruption(CLLMModel* model, double corruption_rate);

#ifdef __cplusplus
}
#endif

#endif /* CLLM_H */