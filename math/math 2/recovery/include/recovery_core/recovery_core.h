/**
 * @file recovery_core.h
 * @brief Universal Recovery Core Library
 * 
 * Production-grade recovery using samples-as-anchors with unified OBJECTIVE 28 algorithms.
 * 
 * Features:
 * - Unified algorithm (Phases 1-6 integrated)
 * - Configurable sample count
 * - Thread-safe
 * - Zero-copy where possible
 * - Production error handling
 */

#ifndef RECOVERY_CORE_H
#define RECOVERY_CORE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Version
#define RECOVERY_VERSION_MAJOR 1
#define RECOVERY_VERSION_MINOR 0
#define RECOVERY_VERSION_PATCH 0

// Error codes
typedef enum {
    RECOVERY_OK = 0,
    RECOVERY_ERROR_INVALID_PARAM = -1,
    RECOVERY_ERROR_OUT_OF_MEMORY = -2,
    RECOVERY_ERROR_NOT_CONVERGED = -3,
    RECOVERY_ERROR_NO_SAMPLES = -4,
    RECOVERY_ERROR_INTERNAL = -5
} recovery_error_t;

// Recovery methods
typedef enum {
    RECOVERY_METHOD_AUTO,           // Automatic selection
    RECOVERY_METHOD_CRYPTO,         // Optimized for cryptographic data
    RECOVERY_METHOD_SIGNAL,         // Optimized for signal processing
    RECOVERY_METHOD_BINARY,         // Generic binary data
    RECOVERY_METHOD_FAST,           // Fast convergence (may sacrifice quality)
    RECOVERY_METHOD_ACCURATE        // Accurate convergence (slower)
} recovery_method_t;

// Configuration
typedef struct {
    int max_iterations;             // Maximum iterations (default: 10000)
    double convergence_threshold;   // Convergence threshold (default: 0.001)
    int num_samples;                // Number of samples to use (0 = all)
    recovery_method_t method;       // Recovery method
    int verbose;                    // Verbose output (0 = quiet, 1 = normal, 2 = debug)
    int num_threads;                // Number of threads (0 = auto)
    bool use_gpu;                   // Use GPU acceleration if available
} recovery_config_t;

// Recovery context (opaque)
typedef struct recovery_context recovery_context_t;

// Recovery result
typedef struct {
    uint8_t* data;                  // Recovered data
    size_t length;                  // Length of recovered data
    int iterations;                 // Iterations taken
    double final_oscillation;       // Final oscillation value
    double convergence_rate;        // Convergence rate
    double quality_score;           // Quality score (0-1)
    bool converged;                 // Whether it converged
    double time_seconds;            // Time taken
} recovery_result_t;

// Sample structure
typedef struct {
    uint8_t* data;                  // Sample data
    size_t length;                  // Sample length
    size_t offset;                  // Offset in result (0 = beginning)
    double confidence;              // Confidence in this sample (0-1)
} recovery_sample_t;

/**
 * Get default configuration
 */
recovery_config_t recovery_default_config(void);

/**
 * Initialize recovery context
 * 
 * @param config Configuration (NULL for defaults)
 * @return Context or NULL on error
 */
recovery_context_t* recovery_init(const recovery_config_t* config);

/**
 * Set Q data (target/public/corrupted data)
 * 
 * @param ctx Context
 * @param q Q data
 * @param len Length of Q
 * @return Error code
 */
recovery_error_t recovery_set_q(recovery_context_t* ctx, const uint8_t* q, size_t len);

/**
 * Add sample (known good data as anchor)
 * 
 * @param ctx Context
 * @param sample Sample data
 * @param len Sample length
 * @param offset Offset in result (0 = beginning)
 * @param confidence Confidence (0-1, use 1.0 if certain)
 * @return Error code
 */
recovery_error_t recovery_add_sample(recovery_context_t* ctx, 
                                    const uint8_t* sample, 
                                    size_t len,
                                    size_t offset,
                                    double confidence);

/**
 * Add multiple samples at once
 * 
 * @param ctx Context
 * @param samples Array of samples
 * @param num_samples Number of samples
 * @return Error code
 */
recovery_error_t recovery_add_samples(recovery_context_t* ctx,
                                     const recovery_sample_t* samples,
                                     size_t num_samples);

/**
 * Run recovery
 * 
 * @param ctx Context
 * @return Error code
 */
recovery_error_t recovery_run(recovery_context_t* ctx);

/**
 * Get result
 * 
 * @param ctx Context
 * @return Result (caller must free with recovery_free_result)
 */
recovery_result_t* recovery_get_result(recovery_context_t* ctx);

/**
 * Get current oscillation value (during recovery)
 * 
 * @param ctx Context
 * @return Current oscillation
 */
double recovery_get_oscillation(recovery_context_t* ctx);

/**
 * Get progress (0-1)
 * 
 * @param ctx Context
 * @return Progress
 */
double recovery_get_progress(recovery_context_t* ctx);

/**
 * Cancel recovery (from another thread)
 * 
 * @param ctx Context
 */
void recovery_cancel(recovery_context_t* ctx);

/**
 * Free result
 * 
 * @param result Result to free
 */
void recovery_free_result(recovery_result_t* result);

/**
 * Free context
 * 
 * @param ctx Context to free
 */
void recovery_free(recovery_context_t* ctx);

/**
 * Get error message
 * 
 * @param error Error code
 * @return Error message
 */
const char* recovery_error_string(recovery_error_t error);

/**
 * Get version string
 * 
 * @return Version string
 */
const char* recovery_version(void);

#ifdef __cplusplus
}
#endif

#endif // RECOVERY_CORE_H
