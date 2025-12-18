/**
 * @file statistics.c
 * @brief Implementation of statistical analysis functions
 * 
 * PHASE 1 INTEGRATION: Migrated to NEW math library
 * - Replaced math_sqrt with math_sqrt
 * - Replaced math_abs with math_abs
 * - Replaced prime_floor with math_floor
 * - Replaced prime_ceil with math_ceil
 * - Replaced prime_log2 with math_log2
 */

#include "statistics.h"
#include "math/constants.h"
#include "math/transcendental.h"
#include "math/constants.h"
#include "math/arithmetic.h"
#include "math/constants.h"
#include "math/types.h"
#include "math/constants.h"
#include <stdlib.h>
#include <string.h>

/* ============================================================================
 * Helper Functions
 * ============================================================================ */

static int compare_doubles(const void* a, const void* b) {
    double diff = (*(double*)a - *(double*)b);
    return (diff > 0) ? 1 : ((diff < 0) ? -1 : 0);
}

/* ============================================================================
 * Descriptive Statistics
 * ============================================================================ */

double stats_mean(const double* values, size_t size) {
    if (!values || size == 0) return 0.0;
    
    double sum = 0.0;
    for (size_t i = 0; i < size; i++) {
        sum += values[i];
    }
    return sum / (double)size;
}

double stats_variance(const double* values, size_t size, bool sample_variance) {
    if (!values || size == 0) return 0.0;
    
    double mean = stats_mean(values, size);
    double var = 0.0;
    
    for (size_t i = 0; i < size; i++) {
        double diff = values[i] - mean;
        var += diff * diff;
    }
    
    // Use n-1 for sample variance, n for population variance
    double denominator = sample_variance ? (double)(size - 1) : (double)size;
    return var / denominator;
}

double stats_std_dev(const double* values, size_t size, bool sample_variance) {
    return math_sqrt(stats_variance(values, size, sample_variance));  // PHASE 1: NEW math library
}

double stats_median(double* values, size_t size) {
    if (!values || size == 0) return 0.0;
    
    // Sort the array
    qsort(values, size, sizeof(double), compare_doubles);
    
    if (size % 2 == 0) {
        // Even number of elements: average of middle two
        return (values[size/2 - 1] + values[size/2]) / 2.0;
    } else {
        // Odd number of elements: middle element
        return values[size/2];
    }
}

double stats_mode(const double* values, size_t size, double tolerance) {
    if (!values || size == 0) return 0.0;
    
    // Simple implementation: find most frequent value within tolerance
    double mode = values[0];
    size_t max_count = 1;
    
    for (size_t i = 0; i < size; i++) {
        size_t count = 0;
        for (size_t j = 0; j < size; j++) {
            if (math_abs(values[i] - values[j]) <= tolerance) {  // PHASE 1: NEW math library
                count++;
            }
        }
        if (count > max_count) {
            max_count = count;
            mode = values[i];
        }
    }
    
    return mode;
}

double stats_percentile(double* values, size_t size, double percentile) {
    if (!values || size == 0) return 0.0;
    if (percentile < 0.0) percentile = 0.0;
    if (percentile > 100.0) percentile = 100.0;
    
    // Sort the array
    qsort(values, size, sizeof(double), compare_doubles);
    
    double index = (percentile / 100.0) * (double)(size - 1);
    size_t lower = (size_t)math_floor(index);  // PHASE 1: NEW math library
    size_t upper = (size_t)math_ceil(index);   // PHASE 1: NEW math library
    
    if (lower == upper) {
        return values[lower];
    } else {
        double weight = index - (double)lower;
        return values[lower] * (1.0 - weight) + values[upper] * weight;
    }
}

/* ============================================================================
 * Correlation and Covariance
 * ============================================================================ */

double stats_covariance(
    const double* x,
    const double* y,
    size_t size,
    bool sample_covariance
) {
    if (!x || !y || size == 0) return 0.0;
    
    double mean_x = stats_mean(x, size);
    double mean_y = stats_mean(y, size);
    
    double cov = 0.0;
    for (size_t i = 0; i < size; i++) {
        cov += (x[i] - mean_x) * (y[i] - mean_y);
    }
    
    double denominator = sample_covariance ? (double)(size - 1) : (double)size;
    return cov / denominator;
}

double stats_correlation(const double* x, const double* y, size_t size) {
    if (!x || !y || size == 0) return 0.0;
    
    double cov = stats_covariance(x, y, size, true);
    double std_x = stats_std_dev(x, size, true);
    double std_y = stats_std_dev(y, size, true);
    
    if (std_x < 1e-10 || std_y < 1e-10) return 0.0;
    
    return cov / (std_x * std_y);
}

double stats_spearman_correlation(const double* x, const double* y, size_t size) {
    if (!x || !y || size == 0) return 0.0;
    
    // Convert to ranks
    double* rank_x = (double*)malloc(size * sizeof(double));
    double* rank_y = (double*)malloc(size * sizeof(double));
    
    if (!rank_x || !rank_y) {
        free(rank_x);
        free(rank_y);
        return 0.0;
    }
    
    // Simple ranking (could be optimized)
    for (size_t i = 0; i < size; i++) {
        rank_x[i] = 1.0;
        rank_y[i] = 1.0;
        
        for (size_t j = 0; j < size; j++) {
            if (x[j] < x[i]) rank_x[i] += 1.0;
            if (y[j] < y[i]) rank_y[i] += 1.0;
        }
    }
    
    // Compute Pearson correlation of ranks
    double corr = stats_correlation(rank_x, rank_y, size);
    
    free(rank_x);
    free(rank_y);
    
    return corr;
}

/* ============================================================================
 * Distribution Functions
 * ============================================================================ */

void stats_histogram(
    const double* values,
    size_t size,
    size_t* bins,
    size_t num_bins,
    double min_value,
    double max_value
) {
    if (!values || !bins || size == 0 || num_bins == 0) return;
    
    // Initialize bins to zero
    memset(bins, 0, num_bins * sizeof(size_t));
    
    double range = max_value - min_value;
    if (range < 1e-10) return;
    
    for (size_t i = 0; i < size; i++) {
        double val = values[i];
        
        if (val < min_value || val > max_value) continue;
        
        size_t bin = (size_t)((val - min_value) / range * (double)num_bins);
        if (bin >= num_bins) bin = num_bins - 1;
        
        bins[bin]++;
    }
}

double stats_empirical_cdf(double* values, size_t size, double x) {
    if (!values || size == 0) return 0.0;
    
    // Sort the array
    qsort(values, size, sizeof(double), compare_doubles);
    
    // Count values <= x
    size_t count = 0;
    for (size_t i = 0; i < size; i++) {
        if (values[i] <= x) {
            count++;
        } else {
            break;
        }
    }
    
    return (double)count / (double)size;
}

/* ============================================================================
 * Normalization and Standardization
 * ============================================================================ */

void stats_z_score_normalize(double* values, size_t size) {
    if (!values || size == 0) return;
    
    double mean = stats_mean(values, size);
    double std = stats_std_dev(values, size, false);
    
    if (std < 1e-10) return;
    
    for (size_t i = 0; i < size; i++) {
        values[i] = (values[i] - mean) / std;
    }
}

void stats_min_max_normalize(double* values, size_t size) {
    if (!values || size == 0) return;
    
    double min = stats_min(values, size);
    double max = stats_max(values, size);
    double range = max - min;
    
    if (range < 1e-10) return;
    
    for (size_t i = 0; i < size; i++) {
        values[i] = (values[i] - min) / range;
    }
}

void stats_robust_scale(double* values, size_t size) {
    if (!values || size == 0) return;
    
    // Make a copy for computing median and IQR
    double* sorted = (double*)malloc(size * sizeof(double));
    if (!sorted) return;
    
    memcpy(sorted, values, size * sizeof(double));
    
    double median = stats_median(sorted, size);
    double q1 = stats_percentile(sorted, size, 25.0);
    double q3 = stats_percentile(sorted, size, 75.0);
    double iqr = q3 - q1;
    
    free(sorted);
    
    if (iqr < 1e-10) return;
    
    for (size_t i = 0; i < size; i++) {
        values[i] = (values[i] - median) / iqr;
    }
}

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

double stats_min(const double* values, size_t size) {
    if (!values || size == 0) return 0.0;
    
    double min = values[0];
    for (size_t i = 1; i < size; i++) {
        if (values[i] < min) min = values[i];
    }
    return min;
}

double stats_max(const double* values, size_t size) {
    if (!values || size == 0) return 0.0;
    
    double max = values[0];
    for (size_t i = 1; i < size; i++) {
        if (values[i] > max) max = values[i];
    }
    return max;
}

double stats_range(const double* values, size_t size) {
    return stats_max(values, size) - stats_min(values, size);
}

double stats_sum(const double* values, size_t size) {
    if (!values || size == 0) return 0.0;
    
    double sum = 0.0;
    for (size_t i = 0; i < size; i++) {
        sum += values[i];
    }
    return sum;
}

double stats_sum_of_squares(const double* values, size_t size) {
    if (!values || size == 0) return 0.0;
    
    double sum = 0.0;
    for (size_t i = 0; i < size; i++) {
        sum += values[i] * values[i];
    }
    return sum;
}

/* ============================================================================
 * Entropy and Information Theory
 * ============================================================================ */

double stats_shannon_entropy(const double* signal, size_t size) {
    if (!signal || size == 0) return 0.0;
    
    // Normalize to probabilities
    double sum = 0.0;
    for (size_t i = 0; i < size; i++) {
        double val = signal[i];
        if (val < 0.0) val = -val;  // Absolute value
        sum += val;
    }
    
    if (sum == 0.0) return 0.0;
    
    // Calculate Shannon entropy: H = -Σ p(i) * log₂(p(i))
    double entropy = 0.0;
    for (size_t i = 0; i < size; i++) {
        double val = signal[i];
        if (val < 0.0) val = -val;
        double p = val / sum;
        
        if (p > 0.0) {
            // Use math_log2 for log base 2
            double log2_p = math_log2(p);  // PHASE 1: NEW math library
            entropy -= p * log2_p;
        }
    }
    
    return entropy;
}

double stats_entropy_reduction(double initial_bits, uint32_t steps,
                                double cut_min, double cut_max) {
    if (steps == 0) return initial_bits;
    if (cut_min < 0.0) cut_min = 0.0;
    if (cut_max > 1.0) cut_max = 1.0;
    if (cut_min >= cut_max) return initial_bits;
    
    double current_bits = initial_bits;
    
    for (uint32_t step = 0; step < steps; step++) {
        // Deterministic cut using golden ratio (from mathematical_constants.h)
        // This provides reproducible, testable results
        double t = (double)(step + 1) * MATH_PHI;
        t = t - (uint64_t)t;  // Fractional part
        
        double cut_fraction = cut_min + t * (cut_max - cut_min);
        
        // Reduce entropy: new_bits = old_bits + log₂(1 - cut_fraction)
        double reduction = math_log2(1.0 - cut_fraction);  // PHASE 1: NEW math library
        current_bits += reduction;  // reduction is negative
        
        if (current_bits < 0.0) current_bits = 0.0;
    }
    
    return current_bits;
}

double stats_modular_probability(uint64_t value, const uint64_t* primes,
                                  size_t num_primes, double* prob_dist) {
    if (!primes || !prob_dist || num_primes == 0) return 0.0;
    
    double sum = 0.0;
    
    for (size_t i = 0; i < num_primes; i++) {
        if (primes[i] == 0) {
            prob_dist[i] = 0.0;
            continue;
        }
        
        uint64_t remainder = value % primes[i];
        double p = (double)remainder / (double)primes[i];
        prob_dist[i] = p;
        sum += p;
    }
    
    return sum;
}

double stats_entropy_residuals(const uint64_t* layers, size_t num_layers,
                                const uint64_t* primes, size_t num_primes,
                                double phi_scale) {
    if (!layers || !primes || num_layers == 0 || num_primes == 0) {
        return 0.0;
    }
    
    double total_residual = 0.0;
    
    // Allocate temporary probability distribution
    double* prob_dist = (double*)malloc(num_primes * sizeof(double));
    if (!prob_dist) return 0.0;
    
    for (size_t layer = 0; layer < num_layers; layer++) {
        // Calculate modular probability distribution
        double sum = stats_modular_probability(layers[layer], primes, 
                                               num_primes, prob_dist);
        
        // Calculate entropy for this layer
        double layer_entropy = 0.0;
        for (size_t i = 0; i < num_primes; i++) {
            if (sum > 0.0 && prob_dist[i] > 0.0) {
                double p = prob_dist[i] / sum;
                double log2_p = math_log2(p);  // PHASE 1: NEW math library
                layer_entropy -= p * log2_p;
            }
        }
        
        // Fold into golden ratio bounds
        // residual = entropy mod (φ * scale)
        double bound = MATH_PHI * phi_scale;
        if (bound > 0.0) {
            // Use prime_fmod for modular arithmetic
            while (layer_entropy >= bound) {
                layer_entropy -= bound;
            }
            while (layer_entropy < 0.0) {
                layer_entropy += bound;
            }
        }
        
        total_residual += layer_entropy;
    }
    
    free(prob_dist);
    
    // Final folding
    double final_bound = MATH_PHI * phi_scale * (double)num_layers;
    if (final_bound > 0.0) {
        while (total_residual >= final_bound) {
            total_residual -= final_bound;
        }
        while (total_residual < 0.0) {
            total_residual += final_bound;
        }
    }
    
    return total_residual;
}
