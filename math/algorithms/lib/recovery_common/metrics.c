#include "recovery_common/recovery_common.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"

/*
 * Metrics Calculation Module
 * 
 * Calculates quality metrics for recovery validation.
 */

// ============================================================================
// METRICS CALCULATION
// ============================================================================

double calculate_rmse(double* data1, double* data2, size_t size) {
    if (!data1 || !data2 || size == 0) {
        return -1.0;
    }
    
    double sum_squared_error = 0.0;
    
    for (size_t i = 0; i < size; i++) {
        double diff = data1[i] - data2[i];
        sum_squared_error += diff * diff;
    }
    
    return math_sqrt(sum_squared_error / size);
}

double calculate_max_error(double* data1, double* data2, size_t size) {
    if (!data1 || !data2 || size == 0) {
        return -1.0;
    }
    
    double max_error = 0.0;
    
    for (size_t i = 0; i < size; i++) {
        double diff = math_abs(data1[i] - data2[i]);
        if (diff > max_error) {
            max_error = diff;
        }
    }
    
    return max_error;
}

double calculate_recovery_rate(size_t recovered, size_t total) {
    if (total == 0) {
        return 0.0;
    }
    
    return (double)recovered / total;
}

double calculate_quality_score(double rmse, double max_error, double recovery_rate) {
    // Quality score is a weighted combination of metrics
    // Lower RMSE and max_error are better, higher recovery_rate is better
    
    // Normalize RMSE and max_error (assuming they're in range 0-1)
    double rmse_score = 1.0 - math_min(rmse, 1.0);
    double max_error_score = 1.0 - math_min(max_error, 1.0);
    
    // Weighted average
    double quality = 0.4 * rmse_score + 0.3 * max_error_score + 0.3 * recovery_rate;
    
    return math_min(math_max(quality, 0.0), 1.0); // Clamp to [0, 1]
}
