#ifndef MULTI_TORUS_TRACKER_H
#define MULTI_TORUS_TRACKER_H

#include <stdbool.h>
#include <stdint.h>
#include "oscillation_decomposition.h"

/**
 * Multi-Torus Tracker
 * 
 * Tracks multiple tori independently and computes their intersection
 * to determine the actual reduced search space.
 */

typedef struct {
    int torus_id;              // Torus identifier (1, 2, 3, ...)
    
    // Oscillation parameters
    double frequency;          // Frequency of this torus
    double amplitude;          // Amplitude of oscillation
    double phase;              // Phase offset
    int period;                // Period in iterations
    
    // Torus geometry
    double major_radius;       // Major radius (R)
    double minor_radius;       // Minor radius (r)
    double center_k;           // Center k value
    
    // Bounds
    double k_min;              // Minimum k in this torus
    double k_max;              // Maximum k in this torus
    
    // Confidence
    double confidence;         // Confidence in this torus (0-1)
    int samples_used;          // Number of samples used to identify
} TorusDescriptor;

typedef struct {
    TorusDescriptor* tori;     // Array of identified tori
    int num_tori;              // Number of tori identified
    int max_tori;              // Maximum tori to track
    
    // Intersection bounds
    double intersection_k_min; // Minimum k in intersection
    double intersection_k_max; // Maximum k in intersection
    bool intersection_valid;   // Is intersection non-empty?
    
    // Search space reduction
    uint64_t original_space;   // Original search space size
    uint64_t reduced_space;    // Reduced search space size
    double reduction_factor;   // Reduction factor
    
    // History for tracking
    double* k_history;         // History of k estimates
    int history_size;
    int history_index;
} MultiTorusTracker;

/**
 * Create multi-torus tracker
 */
MultiTorusTracker* create_multi_torus_tracker(
    int max_tori,
    int history_size,
    uint64_t original_space
);

/**
 * Free multi-torus tracker
 */
void free_multi_torus_tracker(MultiTorusTracker* tracker);

/**
 * Add k estimate sample
 */
void multi_torus_add_sample(
    MultiTorusTracker* tracker,
    double k_estimate
);

/**
 * Identify tori from collected samples
 * 
 * Uses oscillation decomposition to identify multiple tori
 */
int identify_tori(MultiTorusTracker* tracker);

/**
 * Compute intersection of all tori
 * 
 * Finds the region where all tori overlap
 */
bool compute_torus_intersection(MultiTorusTracker* tracker);

/**
 * Get specific torus descriptor
 */
const TorusDescriptor* get_torus(
    const MultiTorusTracker* tracker,
    int index
);

/**
 * Check if k value is within intersection
 */
bool is_k_in_intersection(
    const MultiTorusTracker* tracker,
    double k
);

/**
 * Get reduction factor
 */
double get_multi_torus_reduction_factor(const MultiTorusTracker* tracker);

/**
 * Print multi-torus analysis
 */
void print_multi_torus_analysis(const MultiTorusTracker* tracker);

/**
 * Export multi-torus data to CSV
 */
void export_multi_torus_csv(
    const MultiTorusTracker* tracker,
    const char* filename
);

/**
 * Multi-sample intersection result
 */
typedef struct {
    uint32_t num_samples;           // Number of samples combined
    uint32_t* sample_ids;           // Which samples were combined
    double intersection_k_min;      // Combined intersection min
    double intersection_k_max;      // Combined intersection max
    double intersection_size;       // Size of intersection
    double reduction_factor;        // Reduction factor
    bool contains_true_k;           // Does intersection contain true k?
    double original_space;          // Original search space
} MultiSampleIntersection;

/**
 * Compute intersection of multiple samples
 */
MultiSampleIntersection* compute_multi_sample_intersection(
    MultiTorusTracker** trackers,
    uint32_t num_samples,
    uint64_t true_k,
    uint64_t max_k
);

/**
 * Free multi-sample intersection
 */
void free_multi_sample_intersection(MultiSampleIntersection* intersection);

#endif // MULTI_TORUS_TRACKER_H