/**
 * @file micro_model.h
 * @brief Trainable Micro-Model for Geometric Recovery
 * 
 * Lightweight neural network for learned k recovery from clock positions.
 * Uses ONLY the NEW math library - no legacy dependencies.
 * 
 * Features:
 * - Lightweight architecture (< 1000 parameters)
 * - Fast inference (< 1ms)
 * - Online learning support
 * - Torus-based geometric constraints
 * - Clock lattice integration
 * 
 * Thesis Reference: Chapter 16 - Geometric Recovery
 */

#ifndef ALGORITHMS_GEOMETRIC_RECOVERY_MICRO_MODEL_H
#define ALGORITHMS_GEOMETRIC_RECOVERY_MICRO_MODEL_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// OPAQUE TYPE
// ============================================================================

/**
 * Micro-model structure (opaque)
 */
typedef struct MicroModel MicroModel;

/**
 * Training sample
 */
typedef struct {
    uint64_t k;             // Known k value
    uint64_t Q;             // Known Q value
    double error;           // Triangulation error
} TrainingSample;

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
 * Uses gradient descent to learn the mapping from clock positions to k values.
 * Converges when RMS error falls below threshold or max epochs reached.
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
 * Torus parameters provide geometric constraints for k recovery.
 * Multiple tori can be added to capture complex oscillation patterns.
 * 
 * @param model Model
 * @param torus_id Torus ID (1-20)
 * @param center Torus center (estimated k value)
 * @param amplitude Oscillation amplitude
 * @param period Oscillation period
 * @param phase Phase offset
 * @param confidence Confidence score (0.0-1.0)
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
 * Provides prime factorization information (p, q) for the curve order n = p * q.
 * Automatically maps p and q to clock lattice positions.
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
 * Provides the generator point estimate from triangulation.
 * 
 * @param model Model
 * @param g_estimate G estimate
 * @param g_confidence Confidence score (0.0-1.0)
 * @return 0 on success, -1 on error
 */
int micro_model_set_g_estimate(MicroModel* model, double g_estimate, double g_confidence);

// ============================================================================
// RECOVERY
// ============================================================================

/**
 * Recover k from unknown Q using trained model
 * 
 * Uses the trained neural network and torus constraints to predict
 * a tight bound [k_min, k_max] for the unknown k value.
 * 
 * Algorithm:
 * 1. Map Q to clock position
 * 2. Extract geometric features
 * 3. Predict k using neural network
 * 4. Apply torus constraints
 * 5. Return bounded range
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
 * Computes how much the search space is reduced by the model's prediction.
 * Reduction factor = n / (k_max - k_min)
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
 * Saves the entire model state including weights, torus parameters,
 * and training statistics to a binary file.
 * 
 * @param model Model to save
 * @param filename Output filename
 * @return 0 on success, -1 on error
 */
int micro_model_save(const MicroModel* model, const char* filename);

/**
 * Load model from file
 * 
 * Loads a previously saved model from a binary file.
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
 * Tests the model on a validation set and computes:
 * - Validation error (RMS)
 * - Capture rate (% of times true k is within predicted bounds)
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
 * Prints a comprehensive summary of the model including:
 * - Metadata (name, version, timestamp)
 * - Curve parameters
 * - G estimate
 * - Torus parameters
 * - Clock lattice information
 * - Training statistics
 * - Performance metrics
 * 
 * @param model Model
 * @param output Output file (e.g., stdout)
 */
void micro_model_print_summary(const MicroModel* model, FILE* output);

/**
 * Get model statistics
 * 
 * Retrieves performance metrics from the model.
 * 
 * @param model Model
 * @param avg_reduction Output: average reduction factor
 * @param best_reduction Output: best reduction factor
 * @param capture_rate Output: capture rate (0.0-1.0)
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

#endif /* ALGORITHMS_GEOMETRIC_RECOVERY_MICRO_MODEL_H */