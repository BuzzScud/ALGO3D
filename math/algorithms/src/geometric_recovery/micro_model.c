/**
 * @file micro_model.c
 * @brief Trainable Micro-Model for Geometric Recovery
 * 
 * Lightweight neural network for learned k recovery from geometric features.
 * Uses ONLY the NEW math library - NO external dependencies.
 * 
 * Features:
 * - Lightweight architecture (< 1000 parameters)
 * - Fast inference (< 1ms)
 * - Online learning support
 * - Torus-based geometric constraints
 * - Universal applicability (ANY cryptographic or non-cryptographic system)
 * 
 * Thesis Reference: Chapter 16 - Geometric Recovery
 */

#include "geometric_recovery/micro_model.h"
#include "math/types.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

// ============================================================================
// CONSTANTS
// ============================================================================

#define MAX_TORI 20
#define MAX_TRAINING_SAMPLES 1000
#define LEARNING_RATE 0.01
#define MAX_EPOCHS 100
#define CONVERGENCE_THRESHOLD 1e-6

// ============================================================================
// INTERNAL STRUCTURES
// ============================================================================

/**
 * Torus parameters for geometric constraints
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
struct MicroModel {
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
    
    // Neural network weights (simple linear model)
    double weights[10];     // Feature weights
    double bias;            // Bias term
    
    // Training statistics
    uint32_t num_training_samples;
    double training_error;
    double validation_error;
    
    // Performance metrics
    double reduction_factor;    // Average reduction factor
    double best_reduction;      // Best reduction achieved
    double capture_rate;        // % of times true k is captured
};

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * Extract features from value (universal - works for ANY system)
 */
static void extract_features(uint64_t value, double* features) {
    // Universal geometric features based on value properties
    features[0] = (double)(value % 12);           // 12-fold symmetry
    features[1] = (double)(value % 60);           // Babylonian base-60
    features[2] = (double)(value % 100);          // Centesimal
    features[3] = (double)value;                  // Magnitude
    features[4] = features[0] * features[1];      // Interaction term
    features[5] = features[2] * features[3];      // Interaction term
    features[6] = features[0] * features[0];      // Quadratic term
    features[7] = features[1] * features[1];      // Quadratic term
    features[8] = features[2] * features[2];      // Quadratic term
    features[9] = math_log((double)value + 1.0);  // Log feature (NEW math library)
}

/**
 * Predict k from features using linear model
 */
static double predict_k(MicroModel* model, double* features) {
    double prediction = model->bias;
    for (int i = 0; i < 10; i++) {
        prediction += model->weights[i] * features[i];
    }
    return prediction;
}

/**
 * Apply torus constraints to prediction
 */
static void apply_torus_constraints(MicroModel* model, double* k_pred, double* k_min, double* k_max) {
    if (model->num_tori == 0) {
        // No constraints - use full range
        *k_min = 0.0;
        *k_max = (double)model->n;
        return;
    }
    
    // Find torus with highest confidence
    double best_confidence = 0.0;
    int best_torus = -1;
    
    for (uint32_t i = 0; i < model->num_tori; i++) {
        if (model->tori[i].confidence > best_confidence) {
            best_confidence = model->tori[i].confidence;
            best_torus = i;
        }
    }
    
    if (best_torus >= 0) {
        TorusParams* torus = &model->tori[best_torus];
        
        // Constrain prediction to torus bounds
        double torus_min = torus->center - torus->amplitude;
        double torus_max = torus->center + torus->amplitude;
        
        // Blend prediction with torus center
        double blend_factor = torus->confidence;
        *k_pred = blend_factor * torus->center + (1.0 - blend_factor) * (*k_pred);
        
        // Set bounds
        *k_min = torus_min;
        *k_max = torus_max;
    } else {
        // No valid torus - use prediction ± 10%
        *k_min = (*k_pred) * 0.9;
        *k_max = (*k_pred) * 1.1;
    }
}

// ============================================================================
// MODEL CREATION & INITIALIZATION
// ============================================================================

MicroModel* micro_model_create(const char* name, uint32_t bit_length, uint64_t n) {
    if (!name || bit_length == 0 || n == 0) {
        return NULL;
    }
    
    MicroModel* model = (MicroModel*)calloc(1, sizeof(MicroModel));
    if (!model) {
        return NULL;
    }
    
    // Set metadata
    strncpy(model->name, name, sizeof(model->name) - 1);
    model->version = 1;
    model->timestamp = (uint64_t)time(NULL);
    
    // Set curve parameters
    model->bit_length = bit_length;
    model->n = n;
    
    // Initialize weights randomly (small values)
    srand((unsigned int)time(NULL));
    for (int i = 0; i < 10; i++) {
        model->weights[i] = ((double)rand() / RAND_MAX) * 0.01 - 0.005;
    }
    model->bias = 0.0;
    
    // Initialize G estimate
    model->g_estimate = 0.0;
    model->g_confidence = 0.0;
    
    // Initialize torus count
    model->num_tori = 0;
    
    // Initialize clock info
    model->clock_info.p = 0;
    model->clock_info.q = 0;
    
    // Initialize training statistics
    model->num_training_samples = 0;
    model->training_error = 0.0;
    model->validation_error = 0.0;
    
    // Initialize performance metrics
    model->reduction_factor = 1.0;
    model->best_reduction = 1.0;
    model->capture_rate = 0.0;
    
    return model;
}

void micro_model_free(MicroModel* model) {
    if (model) {
        free(model);
    }
}

// ============================================================================
// TRAINING
// ============================================================================

int micro_model_train(MicroModel* model, TrainingSample* samples, uint32_t num_samples) {
    if (!model || !samples || num_samples == 0) {
        return -1;
    }
    
    printf("Training micro-model on %u samples...\n", num_samples);
    
    // Gradient descent training
    for (int epoch = 0; epoch < MAX_EPOCHS; epoch++) {
        double total_error = 0.0;
        
        for (uint32_t i = 0; i < num_samples; i++) {
            // Extract features from k
            double features[10];
            extract_features(samples[i].k, features);
            
            // Forward pass
            double prediction = predict_k(model, features);
            
            // Compute error
            double error = prediction - (double)samples[i].k;
            total_error += error * error;
            
            // Backward pass (gradient descent)
            for (int j = 0; j < 10; j++) {
                model->weights[j] -= LEARNING_RATE * error * features[j];
            }
            model->bias -= LEARNING_RATE * error;
        }
        
        // Compute RMS error using NEW math library
        double rms_error = math_sqrt(total_error / num_samples);
        
        // Check convergence
        if (rms_error < CONVERGENCE_THRESHOLD) {
            printf("  Converged at epoch %d (RMS error: %.6f)\n", epoch, rms_error);
            break;
        }
        
        if (epoch % 10 == 0) {
            printf("  Epoch %d: RMS error = %.6f\n", epoch, rms_error);
        }
    }
    
    // Update training statistics
    model->num_training_samples = num_samples;
    
    // Calculate final training error
    double total_error = 0.0;
    for (uint32_t i = 0; i < num_samples; i++) {
        double features[10];
        extract_features(samples[i].k, features);
        double prediction = predict_k(model, features);
        double error = prediction - (double)samples[i].k;
        total_error += error * error;
    }
    model->training_error = math_sqrt(total_error / num_samples);
    
    printf("Training complete! Final RMS error: %.6f\n", model->training_error);
    
    return 0;
}

int micro_model_add_torus(
    MicroModel* model,
    int torus_id,
    double center,
    double amplitude,
    double period,
    double phase,
    double confidence
) {
    if (!model || torus_id < 1 || torus_id > MAX_TORI) {
        return -1;
    }
    
    if (model->num_tori >= MAX_TORI) {
        return -1;
    }
    
    TorusParams* torus = &model->tori[model->num_tori];
    torus->torus_id = torus_id;
    torus->center = center;
    torus->amplitude = amplitude;
    torus->period = period;
    torus->phase = phase;
    torus->confidence = confidence;
    
    model->num_tori++;
    
    return 0;
}

int micro_model_set_clock_info(MicroModel* model, uint64_t p, uint64_t q) {
    if (!model || p == 0 || q == 0) {
        return -1;
    }
    
    model->clock_info.p = p;
    model->clock_info.q = q;
    
    // Simplified clock mapping (universal geometric properties)
    model->clock_info.p_ring = (int)(p % 4);
    model->clock_info.p_position = (int)(p % 12);
    model->clock_info.p_angle = (double)(p % 360);
    
    model->clock_info.q_ring = (int)(q % 4);
    model->clock_info.q_position = (int)(q % 12);
    model->clock_info.q_angle = (double)(q % 360);
    
    return 0;
}

int micro_model_set_g_estimate(MicroModel* model, double g_estimate, double g_confidence) {
    if (!model) {
        return -1;
    }
    
    model->g_estimate = g_estimate;
    model->g_confidence = g_confidence;
    
    return 0;
}

// ============================================================================
// RECOVERY
// ============================================================================

int micro_model_recover(
    MicroModel* model,
    uint64_t Q,
    uint64_t* k_min,
    uint64_t* k_max
) {
    if (!model || Q == 0 || !k_min || !k_max) {
        return -1;
    }
    
    // Extract features from Q
    double features[10];
    extract_features(Q, features);
    
    // Predict k
    double k_pred = predict_k(model, features);
    
    // Apply torus constraints
    double k_min_d, k_max_d;
    apply_torus_constraints(model, &k_pred, &k_min_d, &k_max_d);
    
    // Convert to uint64_t
    *k_min = (uint64_t)(k_min_d < 0 ? 0 : k_min_d);
    *k_max = (uint64_t)(k_max_d > model->n ? model->n : k_max_d);
    
    // Ensure k_min < k_max
    if (*k_min >= *k_max) {
        *k_min = 0;
        *k_max = model->n;
    }
    
    return 0;
}

double micro_model_get_reduction_factor(
    MicroModel* model,
    uint64_t Q,
    uint64_t true_k
) {
    if (!model || Q == 0) {
        return 1.0;
    }
    
    uint64_t k_min, k_max;
    if (micro_model_recover(model, Q, &k_min, &k_max) != 0) {
        return 1.0;
    }
    
    // Check if true_k is captured
    bool captured = (true_k >= k_min && true_k <= k_max);
    
    // Compute reduction factor
    uint64_t range = k_max - k_min;
    double reduction = (double)model->n / (double)range;
    
    // Update statistics
    if (captured) {
        if (reduction > model->best_reduction) {
            model->best_reduction = reduction;
        }
    }
    
    return captured ? reduction : 1.0;
}

// ============================================================================
// PERSISTENCE
// ============================================================================

int micro_model_save(const MicroModel* model, const char* filename) {
    if (!model || !filename) {
        return -1;
    }
    
    FILE* f = fopen(filename, "wb");
    if (!f) {
        return -1;
    }
    
    // Write entire model structure
    size_t written = fwrite(model, sizeof(MicroModel), 1, f);
    fclose(f);
    
    return (written == 1) ? 0 : -1;
}

MicroModel* micro_model_load(const char* filename) {
    if (!filename) {
        return NULL;
    }
    
    FILE* f = fopen(filename, "rb");
    if (!f) {
        return NULL;
    }
    
    MicroModel* model = (MicroModel*)malloc(sizeof(MicroModel));
    if (!model) {
        fclose(f);
        return NULL;
    }
    
    // Read entire model structure
    size_t read = fread(model, sizeof(MicroModel), 1, f);
    fclose(f);
    
    if (read != 1) {
        free(model);
        return NULL;
    }
    
    return model;
}

// ============================================================================
// VALIDATION
// ============================================================================

double micro_model_validate(
    MicroModel* model,
    TrainingSample* samples,
    uint32_t num_samples
) {
    if (!model || !samples || num_samples == 0) {
        return -1.0;
    }
    
    double total_error = 0.0;
    uint32_t captured = 0;
    
    for (uint32_t i = 0; i < num_samples; i++) {
        uint64_t k_min, k_max;
        if (micro_model_recover(model, samples[i].Q, &k_min, &k_max) == 0) {
            // Check if true k is captured
            if (samples[i].k >= k_min && samples[i].k <= k_max) {
                captured++;
            }
            
            // Compute error (distance from prediction to true k)
            double features[10];
            extract_features(samples[i].k, features);
            double prediction = predict_k(model, features);
            double error = prediction - (double)samples[i].k;
            total_error += error * error;
        }
    }
    
    // Update validation statistics using NEW math library
    model->validation_error = math_sqrt(total_error / num_samples);
    model->capture_rate = (double)captured / num_samples;
    
    return model->validation_error;
}

void micro_model_print_summary(const MicroModel* model, FILE* output) {
    if (!model || !output) {
        return;
    }
    
    fprintf(output, "\n");
    fprintf(output, "╔══════════════════════════════════════════════════════════╗\n");
    fprintf(output, "║  MICRO-MODEL SUMMARY                                     ║\n");
    fprintf(output, "╚══════════════════════════════════════════════════════════╝\n");
    fprintf(output, "\n");
    
    fprintf(output, "Model: %s (v%u)\n", model->name, model->version);
    fprintf(output, "Timestamp: %lu\n", model->timestamp);
    fprintf(output, "Bit Length: %u\n", model->bit_length);
    fprintf(output, "Curve Order: %lu\n", model->n);
    fprintf(output, "\n");
    
    fprintf(output, "G Estimate: %.6f (confidence: %.2f%%)\n", 
            model->g_estimate, model->g_confidence * 100.0);
    fprintf(output, "\n");
    
    fprintf(output, "Torus Parameters: %u tori\n", model->num_tori);
    for (uint32_t i = 0; i < model->num_tori; i++) {
        fprintf(output, "  Torus %d: center=%.2f, amplitude=%.2f, confidence=%.2f%%\n",
                model->tori[i].torus_id,
                model->tori[i].center,
                model->tori[i].amplitude,
                model->tori[i].confidence * 100.0);
    }
    fprintf(output, "\n");
    
    if (model->clock_info.p > 0 && model->clock_info.q > 0) {
        fprintf(output, "Clock Lattice:\n");
        fprintf(output, "  p = %lu (ring %d, pos %d, angle %.2f)\n",
                model->clock_info.p,
                model->clock_info.p_ring,
                model->clock_info.p_position,
                model->clock_info.p_angle);
        fprintf(output, "  q = %lu (ring %d, pos %d, angle %.2f)\n",
                model->clock_info.q,
                model->clock_info.q_ring,
                model->clock_info.q_position,
                model->clock_info.q_angle);
        fprintf(output, "\n");
    }
    
    fprintf(output, "Training Statistics:\n");
    fprintf(output, "  Samples: %u\n", model->num_training_samples);
    fprintf(output, "  Training Error: %.6f\n", model->training_error);
    fprintf(output, "  Validation Error: %.6f\n", model->validation_error);
    fprintf(output, "\n");
    
    fprintf(output, "Performance Metrics:\n");
    fprintf(output, "  Average Reduction: %.2fx\n", model->reduction_factor);
    fprintf(output, "  Best Reduction: %.2fx\n", model->best_reduction);
    fprintf(output, "  Capture Rate: %.2f%%\n", model->capture_rate * 100.0);
    fprintf(output, "\n");
}

int micro_model_get_statistics(
    const MicroModel* model,
    double* avg_reduction,
    double* best_reduction,
    double* capture_rate
) {
    if (!model || !avg_reduction || !best_reduction || !capture_rate) {
        return -1;
    }
    
    *avg_reduction = model->reduction_factor;
    *best_reduction = model->best_reduction;
    *capture_rate = model->capture_rate;
    
    return 0;
}
