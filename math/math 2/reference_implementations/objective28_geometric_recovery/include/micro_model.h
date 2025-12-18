/**
 * Trainable Micro-Model for Geometric Recovery
 * 
 * Phase 5: Captures complete geometric structure for k recovery
 * 
 * The micro-model encapsulates:
 * - G triangulation estimate
 * - 20 torus parameters (centers, amplitudes, periods)
 * - Clock lattice positions (p, q)
 * - Hierarchical relationships
 * - Oscillation patterns
 */

#ifndef MICRO_MODEL_H
#define MICRO_MODEL_H

#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

// Maximum number of tori
#define MAX_TORI 20

// Maximum number of training samples
#define MAX_TRAINING_SAMPLES 100

/**
 * Torus parameters
 */
typedef struct {
    int torus_id;
    double center;          // Estimated k center
    double amplitude;       // Oscillation amplitude
    double period;          // Oscillation period
    double phase;           // Phase offset
    double confidence;      // Confidence score (0-1)
} TorusParams;

/**
 * Clock lattice information
 */
typedef struct {
    uint64_t p;             // Prime factor p
    uint64_t q;             // Prime factor q
    int p_ring;             // Clock ring for p
    int p_position;         // Clock position for p
    double p_angle;         // Clock angle for p
    int q_ring;             // Clock ring for q
    int q_position;         // Clock position for q
    double q_angle;         // Clock angle for q
} ClockLatticeInfo;

/**
 * Micro-model structure
 */
typedef struct {
    // Model metadata
    char name[64];
    uint32_t version;
    uint64_t timestamp;
    
    // Curve parameters
    uint32_t bit_length;
    uint64_t n;             // Curve order (p * q)
    
    // G triangulation
    double g_estimate;
    double g_confidence;
    
    // Torus parameters
    uint32_t num_tori;
    TorusParams tori[MAX_TORI];
    
    // Clock lattice
    ClockLatticeInfo clock_info;
    
    // Training statistics
    uint32_t num_training_samples;
    double training_error;
    double validation_error;
    
    // Performance metrics
    double reduction_factor;    // Average reduction factor
    double best_reduction;      // Best reduction achieved
    double capture_rate;        // % of times true k is captured
    
} MicroModel;

/**
 * Training sample
 */
typedef struct {
    uint64_t k;             // Known k value
    uint64_t Q;             // Known Q value
    double error;           // Triangulation error
} TrainingSample;

/**
 * Validation sample (same as training but for validation)
 */
typedef struct {
    uint64_t k;             // Known k value
    uint64_t Q;             // Known Q value
} ValidationSample;

// ============================================================================
// MODEL CREATION & INITIALIZATION
// ============================================================================

/**
 * Create a new micro-model
 * 
 * @param name Model name
 * @param bit_length Bit length of the curve
 * @param n Curve order
 * @return Initialized micro-model, or NULL on error
 */
MicroModel* micro_model_create(const char* name, uint32_t bit_length, uint64_t n);

/**
 * Free micro-model
 * 
 * @param model Model to free
 */
void micro_model_free(MicroModel* model);

// ============================================================================
// TRAINING
// ============================================================================

/**
 * Train model on known (k, Q) pairs
 * 
 * @param model Model to train
 * @param samples Training samples
 * @param num_samples Number of samples
 * @return 0 on success, -1 on error
 */
int micro_model_train(MicroModel* model, TrainingSample* samples, uint32_t num_samples);

/**
 * Add torus parameters to model
 * 
 * @param model Model
 * @param torus_id Torus ID (1-20)
 * @param center Torus center
 * @param amplitude Oscillation amplitude
 * @param period Oscillation period
 * @param phase Phase offset
 * @param confidence Confidence score
 * @return 0 on success, -1 on error
 */
int micro_model_add_torus(
    MicroModel* model,
    int torus_id,
    double center,
    double amplitude,
    double period,
    double phase,
    double confidence
);

/**
 * Set clock lattice information
 * 
 * @param model Model
 * @param p Prime factor p
 * @param q Prime factor q
 * @return 0 on success, -1 on error
 */
int micro_model_set_clock_info(MicroModel* model, uint64_t p, uint64_t q);

/**
 * Set G estimate
 * 
 * @param model Model
 * @param g_estimate G estimate
 * @param g_confidence Confidence score
 * @return 0 on success, -1 on error
 */
int micro_model_set_g_estimate(MicroModel* model, double g_estimate, double g_confidence);

// ============================================================================
// RECOVERY
// ============================================================================

/**
 * Recover k from unknown Q using trained model
 * 
 * @param model Trained model
 * @param Q Unknown Q value
 * @param k_min Output: minimum k bound
 * @param k_max Output: maximum k bound
 * @return 0 on success, -1 on error
 */
int micro_model_recover(
    MicroModel* model,
    uint64_t Q,
    uint64_t* k_min,
    uint64_t* k_max
);

/**
 * Get reduction factor for a specific Q
 * 
 * @param model Model
 * @param Q Q value
 * @param true_k True k value (for validation)
 * @return Reduction factor (1.0 = no reduction, 2.0 = 50% reduction, etc.)
 */
double micro_model_get_reduction_factor(
    MicroModel* model,
    uint64_t Q,
    uint64_t true_k
);

// ============================================================================
// PERSISTENCE
// ============================================================================

/**
 * Save model to file
 * 
 * @param model Model to save
 * @param filename Output filename
 * @return 0 on success, -1 on error
 */
int micro_model_save(const MicroModel* model, const char* filename);

/**
 * Load model from file
 * 
 * @param filename Input filename
 * @return Loaded model, or NULL on error
 */
MicroModel* micro_model_load(const char* filename);

// ============================================================================
// VALIDATION
// ============================================================================

/**
 * Validate model on test samples
 * 
 * @param model Model to validate
 * @param samples Test samples
 * @param num_samples Number of samples
 * @return Average error, or -1.0 on error
 */
double micro_model_validate(
    MicroModel* model,
    TrainingSample* samples,
    uint32_t num_samples
);

/**
 * Print model summary
 * 
 * @param model Model
 * @param output Output file
 */
void micro_model_print_summary(const MicroModel* model, FILE* output);

/**
 * Get model statistics
 * 
 * @param model Model
 * @param avg_reduction Output: average reduction factor
 * @param best_reduction Output: best reduction factor
 * @param capture_rate Output: capture rate
 * @return 0 on success, -1 on error
 */
int micro_model_get_statistics(
    const MicroModel* model,
    double* avg_reduction,
    double* best_reduction,
    double* capture_rate
);

#ifdef __cplusplus
}
#endif

#endif /* MICRO_MODEL_H */