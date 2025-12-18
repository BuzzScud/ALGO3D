/**
 * @file confidence_scoring.c
 * @brief Confidence Scoring for Geometric Recovery
 * 
 * Implements centralized confidence score computation and management.
 * 
 * Key Features:
 * - Multi-metric confidence scoring
 * - Weighted combination using geometric mean
 * - Historical tracking and trend analysis
 * - Adaptive thresholds
 * 
 * Thesis Reference: Chapter 16 - Geometric Recovery Quality Assessment
 */

#include "geometric_recovery/confidence_scoring.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * INTERNAL HELPERS
 * ============================================================================ */

/**
 * @brief Find metric by name
 */
static int find_metric_index(const ConfidenceScorer* scorer, const char* name) {
    for (uint32_t i = 0; i < scorer->num_metrics; i++) {
        if (strcmp(scorer->metrics[i].name, name) == 0) {
            return (int)i;
        }
    }
    return -1;
}

/**
 * @brief Update statistics
 */
static void update_statistics(ConfidenceScorer* scorer) {
    if (scorer->history_size == 0 || scorer->history_position == 0) {
        return;
    }
    
    // Compute mean
    double sum = 0.0;
    uint32_t count = (scorer->history_position < scorer->history_size) 
                     ? scorer->history_position 
                     : scorer->history_size;
    
    for (uint32_t i = 0; i < count; i++) {
        sum += scorer->history[i];
    }
    scorer->mean_confidence = sum / count;
    
    // Compute standard deviation
    double var_sum = 0.0;
    for (uint32_t i = 0; i < count; i++) {
        double diff = scorer->history[i] - scorer->mean_confidence;
        var_sum += diff * diff;
    }
    scorer->std_confidence = math_sqrt(var_sum / count);
    
    // Compute trend (linear regression slope)
    if (count >= 2) {
        double sum_x = 0.0, sum_y = 0.0, sum_xy = 0.0, sum_xx = 0.0;
        
        for (uint32_t i = 0; i < count; i++) {
            double x = (double)i;
            double y = scorer->history[i];
            sum_x += x;
            sum_y += y;
            sum_xy += x * y;
            sum_xx += x * x;
        }
        
        double n = (double)count;
        double denominator = n * sum_xx - sum_x * sum_x;
        
        if (math_abs(denominator) > 1e-10) {
            scorer->trend = (n * sum_xy - sum_x * sum_y) / denominator;
        } else {
            scorer->trend = 0.0;
        }
    }
}

/* ============================================================================
 * PUBLIC API
 * ============================================================================ */

ConfidenceScorer* confidence_scorer_create(
    uint32_t initial_capacity,
    uint32_t history_size
) {
    if (initial_capacity == 0) initial_capacity = 10;
    
    ConfidenceScorer* scorer = calloc(1, sizeof(ConfidenceScorer));
    if (!scorer) return NULL;
    
    scorer->metrics = calloc(initial_capacity, sizeof(ConfidenceMetric));
    if (!scorer->metrics) {
        free(scorer);
        return NULL;
    }
    
    scorer->capacity = initial_capacity;
    scorer->num_metrics = 0;
    scorer->overall_confidence = 0.0;
    scorer->min_threshold = 0.5;  // Default: 50%
    scorer->max_threshold = 1.0;
    
    // Initialize history
    if (history_size > 0) {
        scorer->history = calloc(history_size, sizeof(double));
        if (!scorer->history) {
            free(scorer->metrics);
            free(scorer);
            return NULL;
        }
        scorer->history_size = history_size;
    }
    
    scorer->mean_confidence = 0.0;
    scorer->std_confidence = 0.0;
    scorer->trend = 0.0;
    
    return scorer;
}

void confidence_scorer_free(ConfidenceScorer* scorer) {
    if (!scorer) return;
    
    free(scorer->metrics);
    free(scorer->history);
    free(scorer);
}

int confidence_scorer_add_metric(
    ConfidenceScorer* scorer,
    const char* name,
    double value,
    double weight
) {
    if (!scorer || !name) return -1;
    
    // Check if metric already exists
    if (find_metric_index(scorer, name) >= 0) {
        return confidence_scorer_update_metric(scorer, name, value);
    }
    
    // Expand capacity if needed
    if (scorer->num_metrics >= scorer->capacity) {
        uint32_t new_capacity = scorer->capacity * 2;
        ConfidenceMetric* new_metrics = realloc(
            scorer->metrics,
            new_capacity * sizeof(ConfidenceMetric)
        );
        if (!new_metrics) return -1;
        
        scorer->metrics = new_metrics;
        scorer->capacity = new_capacity;
    }
    
    // Add metric
    ConfidenceMetric* metric = &scorer->metrics[scorer->num_metrics];
    strncpy(metric->name, name, sizeof(metric->name) - 1);
    metric->name[sizeof(metric->name) - 1] = '\0';
    metric->value = confidence_normalize(value);
    metric->weight = weight > 0.0 ? weight : 1.0;
    metric->is_valid = true;
    
    scorer->num_metrics++;
    
    return 0;
}

int confidence_scorer_update_metric(
    ConfidenceScorer* scorer,
    const char* name,
    double value
) {
    if (!scorer || !name) return -1;
    
    int idx = find_metric_index(scorer, name);
    if (idx < 0) return -1;
    
    scorer->metrics[idx].value = confidence_normalize(value);
    scorer->metrics[idx].is_valid = true;
    
    return 0;
}

int confidence_scorer_remove_metric(
    ConfidenceScorer* scorer,
    const char* name
) {
    if (!scorer || !name) return -1;
    
    int idx = find_metric_index(scorer, name);
    if (idx < 0) return -1;
    
    // Shift remaining metrics
    for (uint32_t i = idx; i < scorer->num_metrics - 1; i++) {
        scorer->metrics[i] = scorer->metrics[i + 1];
    }
    
    scorer->num_metrics--;
    
    return 0;
}

double confidence_scorer_compute(ConfidenceScorer* scorer) {
    if (!scorer || scorer->num_metrics == 0) {
        return 0.0;
    }
    
    // Compute weighted average
    double weighted_sum = 0.0;
    double total_weight = 0.0;
    
    for (uint32_t i = 0; i < scorer->num_metrics; i++) {
        if (scorer->metrics[i].is_valid) {
            weighted_sum += scorer->metrics[i].value * scorer->metrics[i].weight;
            total_weight += scorer->metrics[i].weight;
        }
    }
    
    if (total_weight == 0.0) {
        scorer->overall_confidence = 0.0;
    } else {
        scorer->overall_confidence = weighted_sum / total_weight;
    }
    
    // Add to history
    if (scorer->history_size > 0) {
        uint32_t pos = scorer->history_position % scorer->history_size;
        scorer->history[pos] = scorer->overall_confidence;
        scorer->history_position++;
        
        update_statistics(scorer);
    }
    
    return scorer->overall_confidence;
}

int confidence_scorer_get_assessment(
    const ConfidenceScorer* scorer,
    ConfidenceAssessment* assessment
) {
    if (!scorer || !assessment) return -1;
    
    memset(assessment, 0, sizeof(ConfidenceAssessment));
    
    if (scorer->num_metrics == 0) {
        return -1;
    }
    
    double min_val = 1.0;
    double max_val = 0.0;
    uint32_t valid_count = 0;
    double weighted_sum = 0.0;
    double total_weight = 0.0;
    
    for (uint32_t i = 0; i < scorer->num_metrics; i++) {
        if (scorer->metrics[i].is_valid) {
            double val = scorer->metrics[i].value;
            
            if (val < min_val) min_val = val;
            if (val > max_val) max_val = val;
            
            weighted_sum += val * scorer->metrics[i].weight;
            total_weight += scorer->metrics[i].weight;
            valid_count++;
        }
    }
    
    assessment->overall_confidence = scorer->overall_confidence;
    assessment->weighted_score = (total_weight > 0.0) ? weighted_sum / total_weight : 0.0;
    assessment->min_metric = min_val;
    assessment->max_metric = max_val;
    assessment->num_valid_metrics = valid_count;
    assessment->meets_threshold = (scorer->overall_confidence >= scorer->min_threshold);
    assessment->confidence_trend = scorer->trend;
    
    return 0;
}

bool confidence_scorer_meets_threshold(const ConfidenceScorer* scorer) {
    if (!scorer) return false;
    
    return scorer->overall_confidence >= scorer->min_threshold;
}

void confidence_scorer_set_min_threshold(
    ConfidenceScorer* scorer,
    double threshold
) {
    if (scorer) {
        scorer->min_threshold = confidence_normalize(threshold);
    }
}

const ConfidenceMetric* confidence_scorer_get_metric(
    const ConfidenceScorer* scorer,
    const char* name
) {
    if (!scorer || !name) return NULL;
    
    int idx = find_metric_index(scorer, name);
    if (idx < 0) return NULL;
    
    return &scorer->metrics[idx];
}

void confidence_scorer_reset_metrics(ConfidenceScorer* scorer) {
    if (!scorer) return;
    
    scorer->num_metrics = 0;
    scorer->overall_confidence = 0.0;
}

void confidence_scorer_clear_history(ConfidenceScorer* scorer) {
    if (!scorer || !scorer->history) return;
    
    memset(scorer->history, 0, scorer->history_size * sizeof(double));
    scorer->history_position = 0;
    scorer->mean_confidence = 0.0;
    scorer->std_confidence = 0.0;
    scorer->trend = 0.0;
}

double confidence_scorer_get_trend(const ConfidenceScorer* scorer) {
    if (!scorer) return 0.0;
    
    return scorer->trend;
}

void confidence_scorer_print_stats(const ConfidenceScorer* scorer) {
    if (!scorer) return;
    
    printf("=== Confidence Scorer Statistics ===\n");
    printf("Number of metrics: %u\n", scorer->num_metrics);
    printf("Overall confidence: %.2f%%\n", scorer->overall_confidence * 100.0);
    printf("Minimum threshold: %.2f%%\n", scorer->min_threshold * 100.0);
    printf("Meets threshold: %s\n", 
           confidence_scorer_meets_threshold(scorer) ? "YES" : "NO");
    
    if (scorer->history_size > 0 && scorer->history_position > 0) {
        printf("\nHistorical Statistics:\n");
        printf("  Mean: %.2f%%\n", scorer->mean_confidence * 100.0);
        printf("  Std Dev: %.2f%%\n", scorer->std_confidence * 100.0);
        printf("  Trend: %.4f (%s)\n", scorer->trend,
               scorer->trend > 0 ? "improving" : 
               scorer->trend < 0 ? "degrading" : "stable");
    }
    
    if (scorer->num_metrics > 0) {
        printf("\nMetrics:\n");
        for (uint32_t i = 0; i < scorer->num_metrics; i++) {
            const ConfidenceMetric* m = &scorer->metrics[i];
            printf("  %s: %.2f%% (weight: %.2f) %s\n",
                   m->name, m->value * 100.0, m->weight,
                   m->is_valid ? "" : "[INVALID]");
        }
    }
    
    printf("====================================\n");
}

/* ============================================================================
 * UTILITY FUNCTIONS
 * ============================================================================ */

double confidence_normalize(double value) {
    if (value < 0.0) return 0.0;
    if (value > 1.0) return 1.0;
    return value;
}

double confidence_combine(
    double conf1,
    double conf2,
    double weight1,
    double weight2
) {
    // Weighted geometric mean for conservative combination
    double total_weight = weight1 + weight2;
    if (total_weight == 0.0) return 0.0;
    
    double w1 = weight1 / total_weight;
    double w2 = weight2 / total_weight;
    
    // Geometric mean: (conf1^w1 * conf2^w2)
    double result = math_pow(conf1, w1) * math_pow(conf2, w2);
    
    return confidence_normalize(result);
}

double confidence_from_error(double error, double max_error) {
    if (max_error <= 0.0) return 0.0;
    
    // Exponential decay: conf = exp(-k * error)
    // where k is chosen so that conf = 0.1 at max_error
    double k = 2.3 / max_error;  // ln(10) ≈ 2.3
    double confidence = math_exp(-k * error);
    
    return confidence_normalize(confidence);
}

double confidence_from_distance(double distance, double max_distance) {
    if (max_distance <= 0.0) return 0.0;
    
    double normalized_dist = distance / max_distance;
    
    // Linear decay with threshold
    double confidence = 1.0 - normalized_dist;
    
    return confidence_normalize(confidence);
}