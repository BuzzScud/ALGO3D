#include "multi_scalar_analysis.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Crystalline library includes
extern double math_sqrt(double x);
extern double math_abs(double x);

// Inline helper for absolute value
static inline double prime_fabs_inline(double x) {
    return (x < 0.0) ? -x : x;
}

/**
 * Create multi-scalar analysis structure
 */
MultiScalarAnalysis* create_multi_scalar_analysis(
    ECDLPInstance* instance,
    ECLatticeEmbedding* Q_embedding,
    const double* scalars,
    uint32_t num_scalars
) {
    if (!instance || !Q_embedding || !scalars || num_scalars == 0) {
        return NULL;
    }
    
    MultiScalarAnalysis* analysis = calloc(1, sizeof(MultiScalarAnalysis));
    if (!analysis) return NULL;
    
    analysis->instance = instance;
    analysis->Q_embedding = Q_embedding;
    analysis->num_scalars = num_scalars;
    analysis->num_dimensions = 15;  // Fixed for our implementation
    
    // Allocate scalars array
    analysis->scalars = malloc(num_scalars * sizeof(double));
    if (!analysis->scalars) {
        free(analysis);
        return NULL;
    }
    memcpy(analysis->scalars, scalars, num_scalars * sizeof(double));
    
    // Allocate results array
    analysis->results = calloc(num_scalars, sizeof(ScalarTestResult));
    if (!analysis->results) {
        free(analysis->scalars);
        free(analysis);
        return NULL;
    }
    
    // Allocate correlation matrix
    analysis->correlation_matrix = malloc(num_scalars * sizeof(double*));
    if (!analysis->correlation_matrix) {
        free(analysis->results);
        free(analysis->scalars);
        free(analysis);
        return NULL;
    }
    for (uint32_t i = 0; i < num_scalars; i++) {
        analysis->correlation_matrix[i] = calloc(num_scalars, sizeof(double));
        if (!analysis->correlation_matrix[i]) {
            for (uint32_t j = 0; j < i; j++) {
                free(analysis->correlation_matrix[j]);
            }
            free(analysis->correlation_matrix);
            free(analysis->results);
            free(analysis->scalars);
            free(analysis);
            return NULL;
        }
    }
    
    // Allocate variance arrays
    analysis->variance_per_scalar = calloc(num_scalars, sizeof(double));
    analysis->variance_per_dimension = calloc(15, sizeof(double));
    if (!analysis->variance_per_scalar || !analysis->variance_per_dimension) {
        for (uint32_t i = 0; i < num_scalars; i++) {
            free(analysis->correlation_matrix[i]);
        }
        free(analysis->correlation_matrix);
        free(analysis->results);
        free(analysis->scalars);
        free(analysis->variance_per_scalar);
        free(analysis->variance_per_dimension);
        free(analysis);
        return NULL;
    }
    
    // Initialize default search parameters
    init_default_search_parameters(&analysis->search_params);
    
    return analysis;
}

/**
 * Scale BIGNUM by a scalar value
 */
BIGNUM* scale_k(const BIGNUM* k, double scalar) {
    if (!k || scalar <= 0.0) return NULL;
    
    BN_CTX* ctx = BN_CTX_new();
    if (!ctx) return NULL;
    
    BIGNUM* result = BN_new();
    if (!result) {
        BN_CTX_free(ctx);
        return NULL;
    }
    
    // Convert scalar to BIGNUM
    BIGNUM* scalar_bn = BN_new();
    if (!scalar_bn) {
        BN_free(result);
        BN_CTX_free(ctx);
        return NULL;
    }
    
    // Scale: result = k * scalar
    // For simplicity, we'll use integer scaling
    uint64_t scale_int = (uint64_t)(scalar * 1000000.0);  // Scale to avoid precision loss
    BN_set_word(scalar_bn, scale_int);
    
    BN_mul(result, k, scalar_bn, ctx);
    
    // Divide by 1000000 to get actual scaled value
    BIGNUM* divisor = BN_new();
    BN_set_word(divisor, 1000000);
    BN_div(result, NULL, result, divisor, ctx);
    
    BN_free(divisor);
    BN_free(scalar_bn);
    BN_CTX_free(ctx);
    
    return result;
}

/**
 * Test recovery at a single scalar
 */
bool test_at_scalar(
    MultiScalarAnalysis* analysis,
    uint32_t scalar_index
) {
    if (!analysis || scalar_index >= analysis->num_scalars) {
        return false;
    }
    
    ScalarTestResult* result = &analysis->results[scalar_index];
    result->scalar = analysis->scalars[scalar_index];
    
    // Scale the target k
    result->target_k = scale_k(analysis->instance->k, result->scalar);
    if (!result->target_k) {
        return false;
    }
    
    // Create search state
    SearchState* state = create_search_state(&analysis->search_params);
    if (!state) {
        BN_free(result->target_k);
        return false;
    }
    
    // Start timing
    clock_t start = clock();
    
    // Run recursive search
    result->recovered_k = recursive_search(
        analysis->instance,
        analysis->Q_embedding,
        &analysis->search_params,
        state,
        result->target_k
    );
    
    // End timing
    clock_t end = clock();
    result->elapsed_time = (double)(end - start) / CLOCKS_PER_SEC;
    
    // Check success
    if (result->recovered_k) {
        result->success = (BN_cmp(result->recovered_k, result->target_k) == 0);
        result->final_score = state->best_score;
        result->iterations = (uint32_t)state->iterations;
        result->final_depth = state->depth;
    } else {
        result->success = false;
        result->final_score = 0.0;
        result->iterations = 0;
        result->final_depth = 0;
    }
    
    // Sample trajectory and detect oscillations for final state
    // Convert BIGNUM to uint64_t for trajectory sampling
    uint64_t k_val = result->recovered_k ? BN_get_word(result->recovered_k) : BN_get_word(result->target_k);
    ECTrajectory* trajectory = sample_ec_trajectory(
        analysis->instance,
        k_val,
        64,  // num_samples
        1.0  // sampling_rate
    );
    
    if (trajectory) {
        result->oscillation_map = detect_oscillations(trajectory);
        free_ec_trajectory(trajectory);
        
        if (result->oscillation_map) {
            // Count stable and converging dimensions
            result->stable_dimensions = 0;
            result->converging_dimensions = 0;
            result->global_amplitude = result->oscillation_map->global_amplitude;
            
            for (uint32_t i = 0; i < result->oscillation_map->num_dimensions; i++) {
                if (result->oscillation_map->signatures[i].is_stable) {
                    result->stable_dimensions++;
                }
                if (result->oscillation_map->is_converging) {
                    result->converging_dimensions++;
                }
            }
            
            // Compute variance per dimension
            result->variance_per_dimension = calloc(15, sizeof(double));
            if (result->variance_per_dimension) {
                result->total_variance = 0.0;
                result->max_variance = 0.0;
                result->high_variance_count = 0;
                
                for (uint32_t i = 0; i < 15; i++) {
                    double amp = result->oscillation_map->signatures[i].amplitude;
                    double variance = amp * amp;  // Variance = amplitude^2
                    result->variance_per_dimension[i] = variance;
                    result->total_variance += variance;
                    
                    if (variance > result->max_variance) {
                        result->max_variance = variance;
                    }
                    
                    if (variance > 1.0) {  // Threshold for high variance
                        result->high_variance_count++;
                    }
                }
            }
        }
    }
    
    free_search_state(state);
    
    return true;
}

/**
 * Test recovery at all scalars
 */
bool analyze_all_scalars(MultiScalarAnalysis* analysis) {
    if (!analysis) return false;
    
    printf("\n=== Multi-Scalar Analysis ===\n");
    printf("Testing at %u scalars...\n\n", analysis->num_scalars);
    
    for (uint32_t i = 0; i < analysis->num_scalars; i++) {
        printf("Testing scalar %.2fx... ", analysis->scalars[i]);
        fflush(stdout);
        
        if (!test_at_scalar(analysis, i)) {
            printf("FAILED\n");
            return false;
        }
        
        printf("%s (%.3f seconds)\n",
               analysis->results[i].success ? "SUCCESS" : "FAILED",
               analysis->results[i].elapsed_time);
    }
    
    printf("\n");
    return true;
}

/**
 * Compute correlation between two oscillation maps
 */
double compute_oscillation_correlation(
    const OscillationMap* map1,
    const OscillationMap* map2
) {
    if (!map1 || !map2) return 0.0;
    if (map1->num_dimensions != map2->num_dimensions) return 0.0;
    
    uint32_t n = map1->num_dimensions;
    
    // Compute mean amplitudes
    double mean1 = 0.0, mean2 = 0.0;
    for (uint32_t i = 0; i < n; i++) {
        mean1 += map1->signatures[i].amplitude;
        mean2 += map2->signatures[i].amplitude;
    }
    mean1 /= n;
    mean2 /= n;
    
    // Compute correlation
    double numerator = 0.0;
    double denom1 = 0.0, denom2 = 0.0;
    
    for (uint32_t i = 0; i < n; i++) {
        double diff1 = map1->signatures[i].amplitude - mean1;
        double diff2 = map2->signatures[i].amplitude - mean2;
        
        numerator += diff1 * diff2;
        denom1 += diff1 * diff1;
        denom2 += diff2 * diff2;
    }
    
    double denominator = math_sqrt(denom1 * denom2);
    if (denominator < 1e-10) return 0.0;
    
    return numerator / denominator;
}

/**
 * Compute cross-scalar correlation
 */
bool compute_cross_scalar_correlation(MultiScalarAnalysis* analysis) {
    if (!analysis) return false;
    
    for (uint32_t i = 0; i < analysis->num_scalars; i++) {
        for (uint32_t j = 0; j < analysis->num_scalars; j++) {
            if (i == j) {
                analysis->correlation_matrix[i][j] = 1.0;
            } else {
                const ScalarTestResult* r1 = &analysis->results[i];
                const ScalarTestResult* r2 = &analysis->results[j];
                
                if (r1->oscillation_map && r2->oscillation_map) {
                    analysis->correlation_matrix[i][j] = compute_oscillation_correlation(
                        r1->oscillation_map,
                        r2->oscillation_map
                    );
                } else {
                    analysis->correlation_matrix[i][j] = 0.0;
                }
            }
        }
    }
    
    return true;
}

/**
 * Compute variance of oscillation amplitudes
 */
double compute_oscillation_variance(
    const OscillationMap** maps,
    uint32_t num_maps,
    double* variance_per_dimension
) {
    if (!maps || num_maps == 0) return 0.0;
    
    uint32_t num_dims = maps[0]->num_dimensions;
    
    // Compute mean amplitude per dimension
    double* means = calloc(num_dims, sizeof(double));
    if (!means) return 0.0;
    
    for (uint32_t i = 0; i < num_maps; i++) {
        if (!maps[i]) continue;
        for (uint32_t d = 0; d < num_dims; d++) {
            means[d] += maps[i]->signatures[d].amplitude;
        }
    }
    
    for (uint32_t d = 0; d < num_dims; d++) {
        means[d] /= num_maps;
    }
    
    // Compute variance per dimension
    double total_variance = 0.0;
    
    for (uint32_t d = 0; d < num_dims; d++) {
        double variance = 0.0;
        for (uint32_t i = 0; i < num_maps; i++) {
            if (!maps[i]) continue;
            double diff = maps[i]->signatures[d].amplitude - means[d];
            variance += diff * diff;
        }
        variance /= num_maps;
        
        if (variance_per_dimension) {
            variance_per_dimension[d] = variance;
        }
        total_variance += variance;
    }
    
    free(means);
    return total_variance;
}

/**
 * Compute variance analysis
 */
bool compute_variance_analysis(MultiScalarAnalysis* analysis) {
    if (!analysis) return false;
    
    // Collect oscillation maps
    const OscillationMap** maps = malloc(analysis->num_scalars * sizeof(OscillationMap*));
    if (!maps) return false;
    
    for (uint32_t i = 0; i < analysis->num_scalars; i++) {
        maps[i] = analysis->results[i].oscillation_map;
    }
    
    // Compute variance per dimension (averaged across scalars)
    compute_oscillation_variance(maps, analysis->num_scalars, analysis->variance_per_dimension);
    
    // Compute variance per scalar
    for (uint32_t i = 0; i < analysis->num_scalars; i++) {
        analysis->variance_per_scalar[i] = analysis->results[i].total_variance;
    }
    
    free(maps);
    return true;
}

/**
 * Compute overall statistics
 */
bool compute_statistics(MultiScalarAnalysis* analysis) {
    if (!analysis) return false;
    
    ScalarStatistics* stats = &analysis->statistics;
    stats->num_scalars = analysis->num_scalars;
    stats->num_successes = 0;
    
    stats->avg_iterations = 0.0;
    stats->avg_time = 0.0;
    stats->avg_score = 0.0;
    stats->avg_variance = 0.0;
    
    stats->min_time = 1e9;
    stats->max_time = 0.0;
    stats->min_score = 1.0;
    stats->max_score = 0.0;
    stats->max_variance = 0.0;
    
    for (uint32_t i = 0; i < analysis->num_scalars; i++) {
        const ScalarTestResult* r = &analysis->results[i];
        
        if (r->success) {
            stats->num_successes++;
        }
        
        stats->avg_iterations += r->iterations;
        stats->avg_time += r->elapsed_time;
        stats->avg_score += r->final_score;
        stats->avg_variance += r->total_variance;
        
        if (r->elapsed_time < stats->min_time) stats->min_time = r->elapsed_time;
        if (r->elapsed_time > stats->max_time) stats->max_time = r->elapsed_time;
        if (r->final_score < stats->min_score) stats->min_score = r->final_score;
        if (r->final_score > stats->max_score) stats->max_score = r->final_score;
        if (r->total_variance > stats->max_variance) stats->max_variance = r->total_variance;
    }
    
    stats->avg_iterations /= analysis->num_scalars;
    stats->avg_time /= analysis->num_scalars;
    stats->avg_score /= analysis->num_scalars;
    stats->avg_variance /= analysis->num_scalars;
    
    stats->success_rate = (double)stats->num_successes / analysis->num_scalars;
    
    // Compute variance trend (linear regression)
    double sum_x = 0.0, sum_y = 0.0, sum_xy = 0.0, sum_x2 = 0.0;
    for (uint32_t i = 0; i < analysis->num_scalars; i++) {
        double x = analysis->scalars[i];
        double y = analysis->variance_per_scalar[i];
        sum_x += x;
        sum_y += y;
        sum_xy += x * y;
        sum_x2 += x * x;
    }
    
    uint32_t n = analysis->num_scalars;
    stats->variance_trend = (n * sum_xy - sum_x * sum_y) / (n * sum_x2 - sum_x * sum_x);
    
    // Compute cross-scalar correlation (average of off-diagonal elements)
    double sum_corr = 0.0;
    uint32_t count = 0;
    for (uint32_t i = 0; i < analysis->num_scalars; i++) {
        for (uint32_t j = i + 1; j < analysis->num_scalars; j++) {
            sum_corr += analysis->correlation_matrix[i][j];
            count++;
        }
    }
    stats->cross_scalar_correlation = (count > 0) ? (sum_corr / count) : 0.0;
    
    // Compute stability score (based on success rate and variance)
    stats->stability_score = stats->success_rate * (1.0 - stats->avg_variance / 10.0);
    if (stats->stability_score < 0.0) stats->stability_score = 0.0;
    if (stats->stability_score > 1.0) stats->stability_score = 1.0;
    
    return true;
}

/**
 * Verify stability across all scalars
 */
bool verify_stability(const MultiScalarAnalysis* analysis) {
    if (!analysis) return false;
    
    const ScalarStatistics* stats = &analysis->statistics;
    
    // Check success rate > 95%
    if (stats->success_rate < 0.95) {
        return false;
    }
    
    // Check no catastrophic failures (score < 0.5)
    for (uint32_t i = 0; i < analysis->num_scalars; i++) {
        if (analysis->results[i].final_score < 0.5) {
            return false;
        }
    }
    
    // Check consistent convergence (variance not too high)
    if (stats->avg_variance > 5.0) {
        return false;
    }
    
    return true;
}

/**
 * Get scalar test result
 */
const ScalarTestResult* get_scalar_result(
    const MultiScalarAnalysis* analysis,
    uint32_t scalar_index
) {
    if (!analysis || scalar_index >= analysis->num_scalars) {
        return NULL;
    }
    return &analysis->results[scalar_index];
}

/**
 * Get overall statistics
 */
const ScalarStatistics* get_statistics(const MultiScalarAnalysis* analysis) {
    if (!analysis) return NULL;
    return &analysis->statistics;
}

/**
 * Print scalar test result
 */
void print_scalar_result(const ScalarTestResult* result) {
    if (!result) return;
    
    printf("\n--- Scalar %.2fx Result ---\n", result->scalar);
    printf("  Success: %s\n", result->success ? "YES" : "NO");
    
    if (result->target_k) {
        char* k_str = BN_bn2hex(result->target_k);
        printf("  Target k: %s\n", k_str);
        OPENSSL_free(k_str);
    }
    
    if (result->recovered_k) {
        char* k_str = BN_bn2hex(result->recovered_k);
        printf("  Recovered k: %s\n", k_str);
        OPENSSL_free(k_str);
    }
    
    printf("  Final score: %.6f\n", result->final_score);
    printf("  Iterations: %u\n", result->iterations);
    printf("  Final depth: %u\n", result->final_depth);
    printf("  Elapsed time: %.3f seconds\n", result->elapsed_time);
    
    if (result->oscillation_map) {
        printf("  Stable dimensions: %u / %u\n",
               result->stable_dimensions,
               result->oscillation_map->num_dimensions);
        printf("  Global amplitude: %.6f\n", result->global_amplitude);
    }
    
    printf("  Total variance: %.6f\n", result->total_variance);
    printf("  Max variance: %.6f\n", result->max_variance);
    printf("  High variance dimensions: %u\n", result->high_variance_count);
}

/**
 * Print multi-scalar analysis results
 */
void print_multi_scalar_results(const MultiScalarAnalysis* analysis) {
    if (!analysis) return;
    
    printf("\n========================================\n");
    printf("MULTI-SCALAR ANALYSIS RESULTS\n");
    printf("========================================\n");
    
    const ScalarStatistics* stats = &analysis->statistics;
    
    printf("\nOverall Statistics:\n");
    printf("  Scalars tested: %u\n", stats->num_scalars);
    printf("  Successes: %u / %u\n", stats->num_successes, stats->num_scalars);
    printf("  Success rate: %.1f%%\n", stats->success_rate * 100.0);
    printf("  Avg iterations: %.1f\n", stats->avg_iterations);
    printf("  Avg time: %.3f seconds\n", stats->avg_time);
    printf("  Avg score: %.6f\n", stats->avg_score);
    printf("  Time range: [%.3f, %.3f] seconds\n", stats->min_time, stats->max_time);
    printf("  Score range: [%.6f, %.6f]\n", stats->min_score, stats->max_score);
    
    printf("\nVariance Analysis:\n");
    printf("  Avg variance: %.6f\n", stats->avg_variance);
    printf("  Max variance: %.6f\n", stats->max_variance);
    printf("  Variance trend: %.6f\n", stats->variance_trend);
    
    printf("\nCorrelation Analysis:\n");
    printf("  Cross-scalar correlation: %.6f\n", stats->cross_scalar_correlation);
    printf("  Stability score: %.6f\n", stats->stability_score);
    
    printf("\nStability: %s\n", verify_stability(analysis) ? "STABLE" : "UNSTABLE");
    
    printf("\n========================================\n");
}

/**
 * Create default scalar set
 */
double* create_default_scalars(uint32_t* num_scalars) {
    if (!num_scalars) return NULL;
    
    *num_scalars = 7;
    double* scalars = malloc(7 * sizeof(double));
    if (!scalars) return NULL;
    
    scalars[0] = 0.01;
    scalars[1] = 0.1;
    scalars[2] = 0.5;
    scalars[3] = 1.0;
    scalars[4] = 2.0;
    scalars[5] = 10.0;
    scalars[6] = 100.0;
    
    return scalars;
}

/**
 * Free scalar test result
 */
void free_scalar_result(ScalarTestResult* result) {
    if (!result) return;
    
    if (result->target_k) {
        BN_free(result->target_k);
        result->target_k = NULL;
    }
    
    if (result->recovered_k) {
        BN_free(result->recovered_k);
        result->recovered_k = NULL;
    }
    
    if (result->oscillation_map) {
        free_oscillation_map(result->oscillation_map);
        result->oscillation_map = NULL;
    }
    
    if (result->variance_per_dimension) {
        free(result->variance_per_dimension);
        result->variance_per_dimension = NULL;
    }
}

/**
 * Free multi-scalar analysis structure
 */
void free_multi_scalar_analysis(MultiScalarAnalysis* analysis) {
    if (!analysis) return;
    
    if (analysis->scalars) {
        free(analysis->scalars);
    }
    
    if (analysis->results) {
        for (uint32_t i = 0; i < analysis->num_scalars; i++) {
            free_scalar_result(&analysis->results[i]);
        }
        free(analysis->results);
    }
    
    if (analysis->correlation_matrix) {
        for (uint32_t i = 0; i < analysis->num_scalars; i++) {
            if (analysis->correlation_matrix[i]) {
                free(analysis->correlation_matrix[i]);
            }
        }
        free(analysis->correlation_matrix);
    }
    
    if (analysis->variance_per_scalar) {
        free(analysis->variance_per_scalar);
    }
    
    if (analysis->variance_per_dimension) {
        free(analysis->variance_per_dimension);
    }
    
    free(analysis);
}
