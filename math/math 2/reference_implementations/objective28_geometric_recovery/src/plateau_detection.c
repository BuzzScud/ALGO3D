/**
 * plateau_detection.c - Automatic Plateau Detection for Iterative Algorithms
 */

#include "../include/plateau_detection.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"
#include <stdlib.h>
#include <string.h>

PlateauDetector* create_plateau_detector(
    int history_size,
    double threshold,
    int window_size
) {
    PlateauDetector* detector = (PlateauDetector*)calloc(1, sizeof(PlateauDetector));
    if (!detector) return NULL;
    
    detector->history = (double*)calloc(history_size, sizeof(double));
    if (!detector->history) {
        free(detector);
        return NULL;
    }
    
    detector->history_size = history_size;
    detector->current_index = 0;
    detector->samples_collected = 0;
    detector->threshold = threshold;
    detector->window_size = window_size;
    detector->plateaued = false;
    detector->plateau_iteration = -1;
    detector->plateau_value = 0.0;
    
    return detector;
}

void free_plateau_detector(PlateauDetector* detector) {
    if (!detector) return;
    if (detector->history) free(detector->history);
    free(detector);
}

bool plateau_detector_add_sample(
    PlateauDetector* detector,
    double value,
    int iteration
) {
    if (!detector) return false;
    
    // Add to circular buffer
    detector->history[detector->current_index] = value;
    detector->current_index = (detector->current_index + 1) % detector->history_size;
    detector->samples_collected++;
    
    // Need at least window_size samples to detect plateau
    if (detector->samples_collected < detector->window_size) {
        return false;
    }
    
    // Already plateaued
    if (detector->plateaued) {
        return true;
    }
    
    // Check for plateau: compute variance over window
    int start_idx = (detector->current_index - detector->window_size + detector->history_size) % detector->history_size;
    
    // Compute mean
    double sum = 0.0;
    for (int i = 0; i < detector->window_size; i++) {
        int idx = (start_idx + i) % detector->history_size;
        sum += detector->history[idx];
    }
    double mean = sum / detector->window_size;
    
    // Compute max deviation from mean
    double max_deviation = 0.0;
    for (int i = 0; i < detector->window_size; i++) {
        int idx = (start_idx + i) % detector->history_size;
        double deviation = math_abs(detector->history[idx] - mean);
        if (deviation > max_deviation) {
            max_deviation = deviation;
        }
    }
    
    // Check if max deviation is below threshold
    double relative_deviation = (mean > 0.0) ? (max_deviation / mean) : max_deviation;
    
    if (relative_deviation < detector->threshold) {
        detector->plateaued = true;
        detector->plateau_iteration = iteration - detector->window_size / 2;
        detector->plateau_value = mean;
        return true;
    }
    
    return false;
}

bool is_plateaued(const PlateauDetector* detector) {
    return detector ? detector->plateaued : false;
}

double get_plateau_value(const PlateauDetector* detector) {
    return detector ? detector->plateau_value : 0.0;
}

int get_plateau_iteration(const PlateauDetector* detector) {
    return detector ? detector->plateau_iteration : -1;
}

void reset_plateau_detector(PlateauDetector* detector) {
    if (!detector) return;
    
    memset(detector->history, 0, detector->history_size * sizeof(double));
    detector->current_index = 0;
    detector->samples_collected = 0;
    detector->plateaued = false;
    detector->plateau_iteration = -1;
    detector->plateau_value = 0.0;
}