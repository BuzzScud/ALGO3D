/**
 * @file confidence_scoring.h
 * @brief Confidence Scoring for Geometric Recovery
 * 
 * Provides centralized confidence score computation and management
 * for geometric recovery algorithms.
 * 
 * Key Features:
 * - Multi-metric confidence scoring
 * - Weighted combination of metrics
 * - Confidence thresholds and validation
 * - Historical confidence tracking
 * - Adaptive confidence adjustment
 * 
 * Thesis Reference: Chapter 16 - Geometric Recovery Quality Assessment
 * 
 * Usage:
 *   ConfidenceScorer* scorer = confidence_scorer_create();
 *   confidence_scorer_add_metric(scorer, "geometric", 0.85, 1.0);
 *   double score = confidence_scorer_compute(scorer);
 *   confidence_scorer_free(scorer);
 */

#ifndef CONFIDENCE_SCORING_H
#define CONFIDENCE_SCORING_H

#include <stdint.h>
#include <stdbool.h>

/* ============================================================================
 * TYPES
 * ============================================================================ */

/**
 * @brief Confidence metric
 * 
 * Represents a single confidence metric with its value and weight.
 */
typedef struct {
    char name[64];              // Metric name
    double value;               // Metric value [0.0, 1.0]
    double weight;              // Metric weight (importance)
    bool is_valid;              // Validity flag
} ConfidenceMetric;

/**
 * @brief Confidence scorer
 * 
 * Manages multiple confidence metrics and computes overall confidence.
 */
typedef struct {
    ConfidenceMetric* metrics;  // Array of metrics
    uint32_t num_metrics;       // Number of metrics
    uint32_t capacity;          // Allocated capacity
    
    double overall_confidence;  // Computed overall confidence
    double min_threshold;       // Minimum acceptable confidence
    double max_threshold;       // Maximum confidence (for normalization)
    
    // Historical tracking
    double* history;            // Confidence history
    uint32_t history_size;      // Size of history buffer
    uint32_t history_position;  // Current position in history
    
    // Statistics
    double mean_confidence;     // Mean confidence over history
    double std_confidence;      // Standard deviation
    double trend;               // Trend (positive = improving)
} ConfidenceScorer;

/**
 * @brief Confidence assessment result
 */
typedef struct {
    double overall_confidence;  // Overall confidence score
    double weighted_score;      // Weighted combination
    double min_metric;          // Minimum metric value
    double max_metric;          // Maximum metric value
    uint32_t num_valid_metrics; // Number of valid metrics
    bool meets_threshold;       // Meets minimum threshold
    double confidence_trend;    // Historical trend
} ConfidenceAssessment;

/* ============================================================================
 * API FUNCTIONS
 * ============================================================================ */

/**
 * @brief Create confidence scorer
 * 
 * @param initial_capacity Initial capacity for metrics
 * @param history_size Size of history buffer (0 = no history)
 * @return Scorer instance or NULL on failure
 */
ConfidenceScorer* confidence_scorer_create(
    uint32_t initial_capacity,
    uint32_t history_size
);

/**
 * @brief Free confidence scorer
 * 
 * @param scorer Scorer to free
 */
void confidence_scorer_free(ConfidenceScorer* scorer);

/**
 * @brief Add confidence metric
 * 
 * @param scorer Scorer instance
 * @param name Metric name
 * @param value Metric value [0.0, 1.0]
 * @param weight Metric weight (importance)
 * @return 0 on success, -1 on failure
 */
int confidence_scorer_add_metric(
    ConfidenceScorer* scorer,
    const char* name,
    double value,
    double weight
);

/**
 * @brief Update existing metric
 * 
 * @param scorer Scorer instance
 * @param name Metric name
 * @param value New value
 * @return 0 on success, -1 if metric not found
 */
int confidence_scorer_update_metric(
    ConfidenceScorer* scorer,
    const char* name,
    double value
);

/**
 * @brief Remove metric
 * 
 * @param scorer Scorer instance
 * @param name Metric name
 * @return 0 on success, -1 if metric not found
 */
int confidence_scorer_remove_metric(
    ConfidenceScorer* scorer,
    const char* name
);

/**
 * @brief Compute overall confidence
 * 
 * Computes weighted average of all valid metrics.
 * 
 * @param scorer Scorer instance
 * @return Overall confidence score [0.0, 1.0]
 */
double confidence_scorer_compute(ConfidenceScorer* scorer);

/**
 * @brief Get confidence assessment
 * 
 * @param scorer Scorer instance
 * @param assessment Output assessment result
 * @return 0 on success, -1 on failure
 */
int confidence_scorer_get_assessment(
    const ConfidenceScorer* scorer,
    ConfidenceAssessment* assessment
);

/**
 * @brief Check if confidence meets threshold
 * 
 * @param scorer Scorer instance
 * @return true if meets threshold, false otherwise
 */
bool confidence_scorer_meets_threshold(const ConfidenceScorer* scorer);

/**
 * @brief Set minimum confidence threshold
 * 
 * @param scorer Scorer instance
 * @param threshold Minimum threshold [0.0, 1.0]
 */
void confidence_scorer_set_min_threshold(
    ConfidenceScorer* scorer,
    double threshold
);

/**
 * @brief Get metric by name
 * 
 * @param scorer Scorer instance
 * @param name Metric name
 * @return Metric or NULL if not found
 */
const ConfidenceMetric* confidence_scorer_get_metric(
    const ConfidenceScorer* scorer,
    const char* name
);

/**
 * @brief Reset all metrics
 * 
 * Clears all metrics but preserves history.
 * 
 * @param scorer Scorer instance
 */
void confidence_scorer_reset_metrics(ConfidenceScorer* scorer);

/**
 * @brief Clear history
 * 
 * @param scorer Scorer instance
 */
void confidence_scorer_clear_history(ConfidenceScorer* scorer);

/**
 * @brief Get confidence trend
 * 
 * Computes trend from historical data.
 * Positive = improving, negative = degrading.
 * 
 * @param scorer Scorer instance
 * @return Trend value
 */
double confidence_scorer_get_trend(const ConfidenceScorer* scorer);

/**
 * @brief Print scorer statistics
 * 
 * @param scorer Scorer instance
 */
void confidence_scorer_print_stats(const ConfidenceScorer* scorer);

/* ============================================================================
 * UTILITY FUNCTIONS
 * ============================================================================ */

/**
 * @brief Normalize confidence value
 * 
 * Clamps value to [0.0, 1.0] range.
 * 
 * @param value Input value
 * @return Normalized value
 */
double confidence_normalize(double value);

/**
 * @brief Combine two confidence scores
 * 
 * Uses weighted geometric mean for conservative combination.
 * 
 * @param conf1 First confidence
 * @param conf2 Second confidence
 * @param weight1 Weight for first confidence
 * @param weight2 Weight for second confidence
 * @return Combined confidence
 */
double confidence_combine(
    double conf1,
    double conf2,
    double weight1,
    double weight2
);

/**
 * @brief Compute confidence from error
 * 
 * Converts error metric to confidence score.
 * 
 * @param error Error value (lower is better)
 * @param max_error Maximum acceptable error
 * @return Confidence score [0.0, 1.0]
 */
double confidence_from_error(double error, double max_error);

/**
 * @brief Compute confidence from distance
 * 
 * Converts distance metric to confidence score.
 * 
 * @param distance Distance value
 * @param max_distance Maximum acceptable distance
 * @return Confidence score [0.0, 1.0]
 */
double confidence_from_distance(double distance, double max_distance);

#endif // CONFIDENCE_SCORING_H