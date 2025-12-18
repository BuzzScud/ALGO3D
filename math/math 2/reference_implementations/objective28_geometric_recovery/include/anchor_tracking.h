#ifndef ANCHOR_TRACKING_H
#define ANCHOR_TRACKING_H

#include <stdint.h>
#include <stdbool.h>
#include <openssl/bn.h>

/**
 * Anchor Tracking System
 * 
 * Tracks the relationship between real k values and estimated k values
 * for each anchor across multiple recursion levels.
 * 
 * This is CRITICAL for understanding how the algorithm converges.
 */

/**
 * Single anchor tracking data
 */
typedef struct {
    uint64_t anchor_id;
    
    // Known values
    BIGNUM* real_k;              // Real k value (known for testing)
    double real_angle;           // Real angle: θ = k·π·φ
    
    // Estimated values
    BIGNUM* estimated_k;         // Our current estimate
    double estimated_angle;      // Estimated angle
    
    // Error metrics
    BIGNUM* error_k;             // error_k = estimated_k - real_k
    double error_angle;          // error_angle = estimated_angle - real_angle
    double error_magnitude;      // |error_k|
    int error_direction;         // +1 if overestimate, -1 if underestimate
    
    // Position in 13D lattice
    double lattice_position[13]; // Position in 13D space
    double error_vector[13];     // Error vector in 13D space
    
    // Recursion tracking
    uint32_t num_recursion_levels;
    uint32_t max_recursion_levels;
    
    BIGNUM** estimates_per_level;    // Array of estimates at each level
    double* errors_per_level;        // Array of errors at each level
    double* convergence_rates;       // Convergence rate at each level
    
    // Convergence analysis
    bool is_converging;
    double convergence_rate;         // How fast is error decreasing?
    uint32_t iterations_to_convergence;  // Estimated iterations needed
    
} AnchorTracking;

/**
 * Global anchor tracking system
 */
typedef struct {
    uint32_t num_anchors;
    AnchorTracking** anchors;
    
    // Global statistics
    double avg_error;
    double max_error;
    double min_error;
    double error_std_dev;
    
    // Error distribution
    uint32_t num_overestimates;
    uint32_t num_underestimates;
    double avg_overestimate;
    double avg_underestimate;
    
    // Convergence analysis
    bool global_convergence;
    double global_convergence_rate;
    uint32_t estimated_iterations_to_convergence;
    
    // Lattice analysis
    double lattice_spacing;
    double avg_neighbor_distance;
    double lattice_regularity;
    
    // Error patterns
    double* error_direction_histogram;  // Histogram of error directions
    uint32_t histogram_bins;
    
} AnchorTrackingSystem;

/**
 * Create anchor tracking for single anchor
 */
AnchorTracking* create_anchor_tracking(
    uint64_t anchor_id,
    const BIGNUM* real_k,
    uint32_t max_recursion_levels
);

/**
 * Update anchor estimate at current recursion level
 */
void update_anchor_estimate(
    AnchorTracking* anchor,
    const BIGNUM* new_estimate,
    uint32_t recursion_level
);

/**
 * Compute error metrics for anchor
 */
void compute_anchor_errors(AnchorTracking* anchor);

/**
 * Analyze convergence for anchor
 */
void analyze_anchor_convergence(AnchorTracking* anchor);

/**
 * Free anchor tracking
 */
void free_anchor_tracking(AnchorTracking* anchor);

/**
 * Create anchor tracking system
 */
AnchorTrackingSystem* create_anchor_tracking_system(uint32_t num_anchors);

/**
 * Add anchor to tracking system
 */
void add_anchor_to_system(
    AnchorTrackingSystem* system,
    AnchorTracking* anchor
);

/**
 * Update all anchors in system
 */
void update_all_anchors(
    AnchorTrackingSystem* system,
    uint32_t recursion_level
);

/**
 * Compute global statistics
 */
void compute_global_statistics(AnchorTrackingSystem* system);

/**
 * Analyze lattice structure
 */
void analyze_lattice_structure(AnchorTrackingSystem* system);

/**
 * Analyze error patterns
 */
void analyze_error_patterns(AnchorTrackingSystem* system);

/**
 * Check global convergence
 */
bool check_global_convergence(const AnchorTrackingSystem* system);

/**
 * Print anchor tracking info
 */
void print_anchor_tracking(const AnchorTracking* anchor);

/**
 * Print system summary
 */
void print_tracking_system_summary(const AnchorTrackingSystem* system);

/**
 * Print detailed system report
 */
void print_detailed_tracking_report(const AnchorTrackingSystem* system);

/**
 * Export tracking data to CSV
 */
void export_tracking_to_csv(const AnchorTrackingSystem* system, const char* filename);

/**
 * Visualize error vectors (generates data for plotting)
 */
void visualize_error_vectors(const AnchorTrackingSystem* system, const char* filename);

/**
 * Visualize convergence patterns
 */
void visualize_convergence(const AnchorTrackingSystem* system, const char* filename);

/**
 * Free anchor tracking system
 */
void free_anchor_tracking_system(AnchorTrackingSystem* system);

#endif // ANCHOR_TRACKING_H