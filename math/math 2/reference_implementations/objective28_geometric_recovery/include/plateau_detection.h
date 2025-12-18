#ifndef PLATEAU_DETECTION_H
#define PLATEAU_DETECTION_H

#include <stdbool.h>
#include <stdint.h>

/**
 * Plateau Detection Library
 * 
 * Automatically detects when an iterative algorithm has plateaued
 * and should stop iterating.
 */

typedef struct {
    double* history;           // History of metric values
    int history_size;          // Size of history buffer
    int current_index;         // Current position in circular buffer
    int samples_collected;     // Number of samples collected
    
    double threshold;          // Plateau threshold (% change)
    int window_size;           // Window size for plateau detection
    
    bool plateaued;            // Has plateau been detected?
    int plateau_iteration;     // Iteration where plateau was detected
    double plateau_value;      // Value at plateau
} PlateauDetector;

/**
 * Create plateau detector
 * 
 * @param history_size Maximum history size
 * @param threshold Plateau threshold (e.g., 0.01 for 1% change)
 * @param window_size Window size for detection (e.g., 50 iterations)
 * @return Plateau detector or NULL on error
 */
PlateauDetector* create_plateau_detector(
    int history_size,
    double threshold,
    int window_size
);

/**
 * Free plateau detector
 */
void free_plateau_detector(PlateauDetector* detector);

/**
 * Add sample to detector
 * 
 * @param detector Plateau detector
 * @param value Current metric value
 * @param iteration Current iteration number
 * @return true if plateau detected, false otherwise
 */
bool plateau_detector_add_sample(
    PlateauDetector* detector,
    double value,
    int iteration
);

/**
 * Check if plateaued
 */
bool is_plateaued(const PlateauDetector* detector);

/**
 * Get plateau value
 */
double get_plateau_value(const PlateauDetector* detector);

/**
 * Get plateau iteration
 */
int get_plateau_iteration(const PlateauDetector* detector);

/**
 * Reset detector
 */
void reset_plateau_detector(PlateauDetector* detector);

#endif // PLATEAU_DETECTION_H