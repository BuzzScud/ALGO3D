#ifndef MULTI_SCALAR_ANALYSIS_H
#define MULTI_SCALAR_ANALYSIS_H

#include <stdint.h>
#include <stdbool.h>
#include <openssl/bn.h>
#include <openssl/ec.h>

#include "ecdlp_integration.h"
#include "oscillation_detection.h"
#include "recursive_search.h"

/**
 * Multi-Scalar Analysis for ECDLP Recovery
 * 
 * Tests the recovery algorithm at multiple scales to verify:
 * 1. Stability across different scalars
 * 2. Cross-scalar correlation
 * 3. Variance analysis
 * 4. Recovery rate at different k ranges
 * 
 * RULE 1 COMPLIANCE: Uses ONLY prime_* functions from crystalline library
 */

// Forward declarations
typedef struct MultiScalarAnalysis MultiScalarAnalysis;
typedef struct ScalarTestResult ScalarTestResult;
typedef struct ScalarStatistics ScalarStatistics;

/**
 * Result of testing at a single scalar
 */
struct ScalarTestResult {
    double scalar;                    // Scalar value (e.g., 0.1, 1.0, 10.0)
    BIGNUM* target_k;                 // Target k value (scaled)
    BIGNUM* recovered_k;              // Recovered k value
    bool success;                     // Recovery successful?
    double final_score;               // Final score from recursive search
    uint32_t iterations;              // Number of iterations
    uint32_t final_depth;             // Final recursion depth
    double elapsed_time;              // Time in seconds
    
    // Oscillation metrics
    OscillationMap* oscillation_map;  // Final oscillation state
    uint32_t stable_dimensions;       // Number of stable dimensions
    uint32_t converging_dimensions;   // Number of converging dimensions
    double global_amplitude;          // Global oscillation amplitude
    
    // Variance metrics
    double* variance_per_dimension;   // Variance in each dimension
    double total_variance;            // Total variance across all dimensions
    double max_variance;              // Maximum variance in any dimension
    uint32_t high_variance_count;     // Number of high-variance dimensions
};

/**
 * Statistics across all scalars
 */
struct ScalarStatistics {
    uint32_t num_scalars;             // Number of scalars tested
    uint32_t num_successes;           // Number of successful recoveries
    double success_rate;              // Success rate (0.0 to 1.0)
    
    double avg_iterations;            // Average iterations
    double avg_time;                  // Average time in seconds
    double avg_score;                 // Average final score
    
    double min_time;                  // Minimum time
    double max_time;                  // Maximum time
    double min_score;                 // Minimum score
    double max_score;                 // Maximum score
    
    // Variance statistics
    double avg_variance;              // Average variance across scalars
    double max_variance;              // Maximum variance at any scalar
    double variance_trend;            // Variance trend (positive = increasing)
    
    // Correlation statistics
    double cross_scalar_correlation;  // Correlation between scalars
    double stability_score;           // Overall stability score (0.0 to 1.0)
};

/**
 * Multi-scalar analysis structure
 */
struct MultiScalarAnalysis {
    ECDLPInstance* instance;          // ECDLP instance
    ECLatticeEmbedding* Q_embedding;  // Target Q embedding
    
    uint32_t num_scalars;             // Number of scalars to test
    double* scalars;                  // Array of scalar values
    
    ScalarTestResult* results;        // Results for each scalar
    ScalarStatistics statistics;      // Overall statistics
    
    // Cross-scalar correlation matrix
    double** correlation_matrix;      // [num_scalars x num_scalars]
    
    // Variance analysis
    double* variance_per_scalar;      // Variance at each scalar
    double* variance_per_dimension;   // Variance in each dimension (averaged)
    
    // Configuration
    SearchParameters search_params;   // Search parameters
    uint32_t num_dimensions;          // Number of dimensions (15)
};

/**
 * Create multi-scalar analysis structure
 * 
 * @param instance ECDLP instance
 * @param Q_embedding Target Q embedding
 * @param scalars Array of scalar values to test
 * @param num_scalars Number of scalars
 * @return Multi-scalar analysis structure (NULL on error)
 */
MultiScalarAnalysis* create_multi_scalar_analysis(
    ECDLPInstance* instance,
    ECLatticeEmbedding* Q_embedding,
    const double* scalars,
    uint32_t num_scalars
);

/**
 * Test recovery at a single scalar
 * 
 * @param analysis Multi-scalar analysis structure
 * @param scalar_index Index of scalar to test
 * @return true if test completed successfully
 */
bool test_at_scalar(
    MultiScalarAnalysis* analysis,
    uint32_t scalar_index
);

/**
 * Test recovery at all scalars
 * 
 * @param analysis Multi-scalar analysis structure
 * @return true if all tests completed successfully
 */
bool analyze_all_scalars(MultiScalarAnalysis* analysis);

/**
 * Compute cross-scalar correlation
 * 
 * Computes correlation between oscillation patterns at different scalars
 * 
 * @param analysis Multi-scalar analysis structure
 * @return true if computation successful
 */
bool compute_cross_scalar_correlation(MultiScalarAnalysis* analysis);

/**
 * Compute variance per scalar and per dimension
 * 
 * @param analysis Multi-scalar analysis structure
 * @return true if computation successful
 */
bool compute_variance_analysis(MultiScalarAnalysis* analysis);

/**
 * Verify stability across all scalars
 * 
 * Checks if recovery is stable at all scalars:
 * - Success rate > 95%
 * - No catastrophic failures
 * - Consistent convergence
 * 
 * @param analysis Multi-scalar analysis structure
 * @return true if stable across all scalars
 */
bool verify_stability(const MultiScalarAnalysis* analysis);

/**
 * Compute overall statistics
 * 
 * @param analysis Multi-scalar analysis structure
 * @return true if computation successful
 */
bool compute_statistics(MultiScalarAnalysis* analysis);

/**
 * Get scalar test result
 * 
 * @param analysis Multi-scalar analysis structure
 * @param scalar_index Index of scalar
 * @return Scalar test result (NULL if invalid index)
 */
const ScalarTestResult* get_scalar_result(
    const MultiScalarAnalysis* analysis,
    uint32_t scalar_index
);

/**
 * Get overall statistics
 * 
 * @param analysis Multi-scalar analysis structure
 * @return Overall statistics
 */
const ScalarStatistics* get_statistics(const MultiScalarAnalysis* analysis);

/**
 * Print multi-scalar analysis results
 * 
 * @param analysis Multi-scalar analysis structure
 */
void print_multi_scalar_results(const MultiScalarAnalysis* analysis);

/**
 * Print scalar test result
 * 
 * @param result Scalar test result
 */
void print_scalar_result(const ScalarTestResult* result);

/**
 * Free scalar test result
 * 
 * @param result Scalar test result to free
 */
void free_scalar_result(ScalarTestResult* result);

/**
 * Free multi-scalar analysis structure
 * 
 * @param analysis Multi-scalar analysis structure to free
 */
void free_multi_scalar_analysis(MultiScalarAnalysis* analysis);

/**
 * Create default scalar set
 * 
 * Creates a standard set of scalars for testing:
 * [0.01, 0.1, 0.5, 1.0, 2.0, 10.0, 100.0]
 * 
 * @param num_scalars Output: number of scalars
 * @return Array of scalar values (caller must free)
 */
double* create_default_scalars(uint32_t* num_scalars);

/**
 * Scale BIGNUM by a scalar value
 * 
 * @param k Original k value
 * @param scalar Scalar multiplier
 * @return Scaled k value (caller must free with BN_free)
 */
BIGNUM* scale_k(const BIGNUM* k, double scalar);

/**
 * Compute correlation between two oscillation maps
 * 
 * @param map1 First oscillation map
 * @param map2 Second oscillation map
 * @return Correlation coefficient (-1.0 to 1.0)
 */
double compute_oscillation_correlation(
    const OscillationMap* map1,
    const OscillationMap* map2
);

/**
 * Compute variance of oscillation amplitudes
 * 
 * @param maps Array of oscillation maps
 * @param num_maps Number of maps
 * @param variance_per_dimension Output: variance per dimension
 * @return Total variance
 */
double compute_oscillation_variance(
    const OscillationMap** maps,
    uint32_t num_maps,
    double* variance_per_dimension
);

#endif // MULTI_SCALAR_ANALYSIS_H