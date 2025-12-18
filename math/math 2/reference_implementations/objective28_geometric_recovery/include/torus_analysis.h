#ifndef TORUS_ANALYSIS_H
#define TORUS_ANALYSIS_H

#include <stdbool.h>
#include <stdint.h>
#include <openssl/ec.h>

/**
 * Torus Analysis Framework
 * 
 * Analyzes oscillations within the plateau to identify torus structures.
 * Tracks multiple scalar oscillations and reduces search space iteratively.
 */

typedef struct {
    double min_value;          // Minimum value in oscillation
    double max_value;          // Maximum value in oscillation
    double amplitude;          // Amplitude of oscillation
    double frequency;          // Estimated frequency
    double phase;              // Estimated phase
    int period;                // Period in iterations
} OscillationMetrics;

typedef struct {
    // 3D space for torus analysis
    double* estimated_k_history;      // Dimension 1: Estimated k over time
    double* error_min_history;        // Dimension 2: Error minimum over time
    double* error_max_history;        // Dimension 3: Error maximum over time
    
    int history_size;
    int current_index;
    int samples_collected;
    
    // Oscillation metrics for each dimension
    OscillationMetrics k_oscillation;
    OscillationMetrics error_min_oscillation;
    OscillationMetrics error_max_oscillation;
    
    // Torus parameters
    double major_radius;       // Major radius of torus
    double minor_radius;       // Minor radius of torus
    bool torus_identified;     // Has torus been identified?
    
    // Search space reduction
    uint64_t original_space_size;
    uint64_t reduced_space_size;
    double reduction_factor;
} TorusAnalyzer;

/**
 * Create torus analyzer
 */
TorusAnalyzer* create_torus_analyzer(int history_size, uint64_t original_space_size);

/**
 * Free torus analyzer
 */
void free_torus_analyzer(TorusAnalyzer* analyzer);

/**
 * Add sample to analyzer
 * 
 * @param analyzer Torus analyzer
 * @param estimated_k Current estimated k
 * @param error_min Minimum error across all training pairs
 * @param error_max Maximum error across all training pairs
 */
void torus_analyzer_add_sample(
    TorusAnalyzer* analyzer,
    double estimated_k,
    double error_min,
    double error_max
);

/**
 * Analyze oscillations and identify torus
 * 
 * @param analyzer Torus analyzer
 * @return true if torus identified, false otherwise
 */
bool analyze_torus(TorusAnalyzer* analyzer);

/**
 * Compute oscillation metrics for a dimension
 */
OscillationMetrics compute_oscillation_metrics(
    const double* history,
    int size
);

/**
 * Get search space reduction factor
 */
double get_reduction_factor(const TorusAnalyzer* analyzer);

/**
 * Print torus analysis results
 */
void print_torus_analysis(const TorusAnalyzer* analyzer);

/**
 * Export torus data to CSV
 */
void export_torus_data(const TorusAnalyzer* analyzer, const char* filename);

#endif // TORUS_ANALYSIS_H