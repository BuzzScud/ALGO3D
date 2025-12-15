/**
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * @file stabilization_metrics.c
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * @brief Metrics and quality assessment for stabilization
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * 
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * Tracks stabilization quality, convergence rates, and provides
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * detailed metrics for multi-scale stabilization analysis.
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 */
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"

#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
#include "blind_recovery.h"
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
 * Stabilization metrics structure
 */
typedef struct {
    uint32_t total_scales;
    uint32_t stable_scales;
    uint32_t unstable_scales;
    double stability_ratio;
    double avg_corruption;
    double quality_score;
    bool all_stable;
    uint32_t iterations_to_stability;
} StabilizationMetrics;

/**
 * Compute comprehensive stabilization metrics
 */
StabilizationMetrics compute_stabilization_metrics(
    const MultiScaleAnalysis* analysis,
    const StructuralMap* structure
) {
    StabilizationMetrics metrics = {0};
    
    if (!analysis) {
        return metrics;
    }
    
    metrics.total_scales = analysis->num_scales;
    metrics.all_stable = analysis->all_scales_stable;
    
    // Count stable/unstable scales
    for (uint32_t i = 0; i < analysis->num_scales; i++) {
        if (analysis->scales[i].is_stable) {
            metrics.stable_scales++;
        } else {
            metrics.unstable_scales++;
        }
    }
    
    // Compute stability ratio
    if (metrics.total_scales > 0) {
        metrics.stability_ratio = (double)metrics.stable_scales / metrics.total_scales;
    }
    
    // Get corruption from structure
    if (structure) {
        metrics.avg_corruption = structure->corruption_percentage;
    }
    
    // Compute quality score
    metrics.quality_score = compute_stabilization_quality(analysis);
    
    return metrics;
}

/**
 * Print stabilization metrics
 */
void print_stabilization_metrics(const StabilizationMetrics* metrics) {
    if (!metrics) {
        return;
    }
    
    printf("Stabilization Metrics:\n");
    printf("  Total scales: %u\n", metrics->total_scales);
    printf("  Stable scales: %u\n", metrics->stable_scales);
    printf("  Unstable scales: %u\n", metrics->unstable_scales);
    printf("  Stability ratio: %.2f%%\n", metrics->stability_ratio * 100.0);
    printf("  Avg corruption: %.2f%%\n", metrics->avg_corruption * 100.0);
    printf("  Quality score: %.3f\n", metrics->quality_score);
    printf("  All stable: %s\n", metrics->all_stable ? "Yes" : "No");
}

/**
 * Validate stabilization quality
 */
bool validate_stabilization(
    const StabilizationMetrics* metrics,
    double min_stability_ratio,
    double max_corruption
) {
    if (!metrics) {
        return false;
    }
    
    // Check stability ratio
    if (metrics->stability_ratio < min_stability_ratio) {
        return false;
    }
    
    // Check corruption level
    if (metrics->avg_corruption > max_corruption) {
        return false;
    }
    
    // Check quality score
    if (metrics->quality_score < 0.7) {
        return false;
    }
    
    return true;
}

/**
 * Track stabilization progress over time
 */
typedef struct {
    uint32_t num_snapshots;
    uint32_t max_snapshots;
    StabilizationMetrics* snapshots;
    uint32_t current_index;
} StabilizationHistory;

/**
 * Create stabilization history
 */
StabilizationHistory* create_stabilization_history(uint32_t max_snapshots) {
    StabilizationHistory* history = (StabilizationHistory*)calloc(
        1, sizeof(StabilizationHistory)
    );
    
    if (!history) {
        return NULL;
    }
    
    history->snapshots = (StabilizationMetrics*)calloc(
        max_snapshots,
        sizeof(StabilizationMetrics)
    );
    
    if (!history->snapshots) {
        free(history);
        return NULL;
    }
    
    history->max_snapshots = max_snapshots;
    history->num_snapshots = 0;
    history->current_index = 0;
    
    return history;
}

/**
 * Add snapshot to history
 */
void add_stabilization_snapshot(
    StabilizationHistory* history,
    const StabilizationMetrics* metrics
) {
    if (!history || !metrics) {
        return;
    }
    
    history->snapshots[history->current_index] = *metrics;
    history->current_index = (history->current_index + 1) % history->max_snapshots;
    
    if (history->num_snapshots < history->max_snapshots) {
        history->num_snapshots++;
    }
}

/**
 * Compute stabilization trend
 */
typedef enum {
    TREND_IMPROVING,
    TREND_STABLE,
    TREND_DEGRADING,
    TREND_UNKNOWN
} StabilizationTrend;

StabilizationTrend compute_stabilization_trend(
    const StabilizationHistory* history
) {
    if (!history || history->num_snapshots < 2) {
        return TREND_UNKNOWN;
    }
    
    // Compare first and last snapshots
    uint32_t first_idx = (history->current_index + history->max_snapshots - 
                         history->num_snapshots) % history->max_snapshots;
    uint32_t last_idx = (history->current_index + history->max_snapshots - 1) % 
                        history->max_snapshots;
    
    const StabilizationMetrics* first = &history->snapshots[first_idx];
    const StabilizationMetrics* last = &history->snapshots[last_idx];
    
    double quality_change = last->quality_score - first->quality_score;
    
    if (quality_change > 0.05) {
        return TREND_IMPROVING;
    } else if (quality_change < -0.05) {
        return TREND_DEGRADING;
    } else {
        return TREND_STABLE;
    }
}

/**
 * Free stabilization history
 */
void free_stabilization_history(StabilizationHistory* history) {
    if (!history) return;
    
    if (history->snapshots) {
        free(history->snapshots);
    }
    
    free(history);
}

/**
 * Compute convergence rate
 */
double compute_convergence_rate(const StabilizationHistory* history) {
    if (!history || history->num_snapshots < 2) {
        return 0.0;
    }
    
    // Compute rate of quality improvement
    uint32_t first_idx = (history->current_index + history->max_snapshots - 
                         history->num_snapshots) % history->max_snapshots;
    uint32_t last_idx = (history->current_index + history->max_snapshots - 1) % 
                        history->max_snapshots;
    
    const StabilizationMetrics* first = &history->snapshots[first_idx];
    const StabilizationMetrics* last = &history->snapshots[last_idx];
    
    double quality_change = last->quality_score - first->quality_score;
    double time_span = (double)history->num_snapshots;
    
    return quality_change / time_span;
}

/**
 * Estimate iterations to full stability
 */
uint32_t estimate_iterations_to_stability(
    const StabilizationHistory* history,
    double target_quality
) {
    if (!history || history->num_snapshots < 2) {
        return 0;
    }
    
    double convergence_rate = compute_convergence_rate(history);
    
    if (convergence_rate <= 0.0) {
        return UINT32_MAX;  // Not converging
    }
    
    // Get current quality
    uint32_t last_idx = (history->current_index + history->max_snapshots - 1) % 
                        history->max_snapshots;
    double current_quality = history->snapshots[last_idx].quality_score;
    
    if (current_quality >= target_quality) {
        return 0;  // Already at target
    }
    
    double quality_gap = target_quality - current_quality;
    uint32_t estimated_iterations = (uint32_t)(quality_gap / convergence_rate);
    
    return estimated_iterations;
}
