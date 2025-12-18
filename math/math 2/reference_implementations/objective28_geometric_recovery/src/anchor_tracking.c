#include "../include/anchor_tracking.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Constants
#define PHI 1.618033988749895  // Golden ratio
#define PI 3.141592653589793

// 13 dimensional frequencies
static const uint32_t DIMENSIONAL_FREQS[13] = {
    3, 7, 31, 12, 19, 5, 11, 13, 17, 23, 29, 37, 41
};

/**
 * Compute angle from k: θ = k·π·φ
 */
static double compute_angle_from_k(const BIGNUM* k) {
    char* k_str = BN_bn2dec(k);
    double k_val = atof(k_str);
    OPENSSL_free(k_str);
    
    double angle = k_val * PI * PHI;
    // Normalize to [0, 360)
    while (angle >= 360.0) angle -= 360.0;
    while (angle < 0.0) angle += 360.0;
    
    return angle;
}

/**
 * Compute 13D lattice position from k
 */
static void compute_lattice_position(const BIGNUM* k, double* position) {
    char* k_str = BN_bn2dec(k);
    double k_val = atof(k_str);
    OPENSSL_free(k_str);
    
    for (uint32_t i = 0; i < 13; i++) {
        // Map k to dimension using frequency
        double freq = (double)DIMENSIONAL_FREQS[i];
        position[i] = math_fmod(k_val, freq);
    }
}

/**
 * Create anchor tracking for single anchor
 */
AnchorTracking* create_anchor_tracking(
    uint64_t anchor_id,
    const BIGNUM* real_k,
    uint32_t max_recursion_levels
) {
    AnchorTracking* anchor = (AnchorTracking*)calloc(1, sizeof(AnchorTracking));
    if (!anchor) return NULL;
    
    anchor->anchor_id = anchor_id;
    anchor->max_recursion_levels = max_recursion_levels;
    anchor->num_recursion_levels = 0;
    
    // Copy real_k
    anchor->real_k = BN_dup(real_k);
    if (!anchor->real_k) {
        free(anchor);
        return NULL;
    }
    
    // Compute real angle
    anchor->real_angle = compute_angle_from_k(real_k);
    
    // Compute lattice position
    compute_lattice_position(real_k, anchor->lattice_position);
    
    // Initialize estimated_k to NULL (will be set later)
    anchor->estimated_k = NULL;
    anchor->estimated_angle = 0.0;
    
    // Initialize error metrics
    anchor->error_k = BN_new();
    anchor->error_magnitude = 0.0;
    anchor->error_direction = 0;
    anchor->error_angle = 0.0;
    
    // Initialize error vector
    memset(anchor->error_vector, 0, sizeof(anchor->error_vector));
    
    // Allocate arrays for recursion tracking
    anchor->estimates_per_level = (BIGNUM**)calloc(max_recursion_levels, sizeof(BIGNUM*));
    anchor->errors_per_level = (double*)calloc(max_recursion_levels, sizeof(double));
    anchor->convergence_rates = (double*)calloc(max_recursion_levels, sizeof(double));
    
    if (!anchor->estimates_per_level || !anchor->errors_per_level || !anchor->convergence_rates) {
        free_anchor_tracking(anchor);
        return NULL;
    }
    
    // Initialize convergence analysis
    anchor->is_converging = false;
    anchor->convergence_rate = 0.0;
    anchor->iterations_to_convergence = 0;
    
    return anchor;
}

/**
 * Update anchor estimate at current recursion level
 */
void update_anchor_estimate(
    AnchorTracking* anchor,
    const BIGNUM* new_estimate,
    uint32_t recursion_level
) {
    if (!anchor || !new_estimate) return;
    if (recursion_level >= anchor->max_recursion_levels) return;
    
    // Update estimated_k
    if (anchor->estimated_k) {
        BN_free(anchor->estimated_k);
    }
    anchor->estimated_k = BN_dup(new_estimate);
    
    // Compute estimated angle
    anchor->estimated_angle = compute_angle_from_k(new_estimate);
    
    // Store estimate for this level
    if (anchor->estimates_per_level[recursion_level]) {
        BN_free(anchor->estimates_per_level[recursion_level]);
    }
    anchor->estimates_per_level[recursion_level] = BN_dup(new_estimate);
    
    // Update recursion level count
    if (recursion_level >= anchor->num_recursion_levels) {
        anchor->num_recursion_levels = recursion_level + 1;
    }
    
    // Compute errors
    compute_anchor_errors(anchor);
    
    // Store error for this level
    anchor->errors_per_level[recursion_level] = anchor->error_magnitude;
    
    // Analyze convergence
    analyze_anchor_convergence(anchor);
}

/**
 * Compute error metrics for anchor
 */
void compute_anchor_errors(AnchorTracking* anchor) {
    if (!anchor || !anchor->estimated_k) return;
    
    // Compute error_k = estimated_k - real_k
    BN_CTX* ctx = BN_CTX_new();
    BN_sub(anchor->error_k, anchor->estimated_k, anchor->real_k);
    
    // Compute error magnitude
    char* error_str = BN_bn2dec(anchor->error_k);
    double error_val = atof(error_str);
    OPENSSL_free(error_str);
    
    anchor->error_magnitude = math_abs(error_val);
    
    // Compute error direction
    if (BN_cmp(anchor->estimated_k, anchor->real_k) > 0) {
        anchor->error_direction = 1;  // Overestimate
    } else if (BN_cmp(anchor->estimated_k, anchor->real_k) < 0) {
        anchor->error_direction = -1;  // Underestimate
    } else {
        anchor->error_direction = 0;  // Exact match
    }
    
    // Compute angle error
    anchor->error_angle = anchor->estimated_angle - anchor->real_angle;
    
    // Normalize angle error to [-180, 180]
    while (anchor->error_angle > 180.0) anchor->error_angle -= 360.0;
    while (anchor->error_angle < -180.0) anchor->error_angle += 360.0;
    
    // Compute error vector in 13D space
    double estimated_position[13];
    compute_lattice_position(anchor->estimated_k, estimated_position);
    
    for (uint32_t i = 0; i < 13; i++) {
        anchor->error_vector[i] = estimated_position[i] - anchor->lattice_position[i];
    }
    
    BN_CTX_free(ctx);
}

/**
 * Analyze convergence for anchor
 */
void analyze_anchor_convergence(AnchorTracking* anchor) {
    if (!anchor || anchor->num_recursion_levels < 2) {
        anchor->is_converging = false;
        anchor->convergence_rate = 0.0;
        return;
    }
    
    // Compute convergence rate from last two levels
    uint32_t last_level = anchor->num_recursion_levels - 1;
    uint32_t prev_level = last_level - 1;
    
    double last_error = anchor->errors_per_level[last_level];
    double prev_error = anchor->errors_per_level[prev_level];
    
    // Avoid division by zero
    if (prev_error < 1e-10) {
        anchor->convergence_rate = 0.0;
        anchor->is_converging = (last_error < 1e-10);
        return;
    }
    
    // Convergence rate: (prev_error - last_error) / prev_error
    // Positive = converging, Negative = diverging
    anchor->convergence_rate = (prev_error - last_error) / prev_error;
    
    // Store convergence rate for this level
    anchor->convergence_rates[last_level] = anchor->convergence_rate;
    
    // Check if converging (error decreasing)
    anchor->is_converging = (anchor->convergence_rate > 0.0);
    
    // Estimate iterations to convergence
    if (anchor->is_converging && anchor->convergence_rate > 0.01) {
        // Simple linear extrapolation
        double remaining_error = last_error;
        double error_reduction_per_iteration = prev_error * anchor->convergence_rate;
        
        if (error_reduction_per_iteration > 1e-10) {
            anchor->iterations_to_convergence = (uint32_t)(remaining_error / error_reduction_per_iteration);
        } else {
            anchor->iterations_to_convergence = 0;
        }
    } else {
        anchor->iterations_to_convergence = 0;
    }
}

/**
 * Free anchor tracking
 */
void free_anchor_tracking(AnchorTracking* anchor) {
    if (!anchor) return;
    
    if (anchor->real_k) BN_free(anchor->real_k);
    if (anchor->estimated_k) BN_free(anchor->estimated_k);
    if (anchor->error_k) BN_free(anchor->error_k);
    
    if (anchor->estimates_per_level) {
        for (uint32_t i = 0; i < anchor->max_recursion_levels; i++) {
            if (anchor->estimates_per_level[i]) {
                BN_free(anchor->estimates_per_level[i]);
            }
        }
        free(anchor->estimates_per_level);
    }
    
    if (anchor->errors_per_level) free(anchor->errors_per_level);
    if (anchor->convergence_rates) free(anchor->convergence_rates);
    
    free(anchor);
}

/**
 * Create anchor tracking system
 */
AnchorTrackingSystem* create_anchor_tracking_system(uint32_t num_anchors) {
    AnchorTrackingSystem* system = (AnchorTrackingSystem*)calloc(1, sizeof(AnchorTrackingSystem));
    if (!system) return NULL;
    
    system->num_anchors = num_anchors;
    system->anchors = (AnchorTracking**)calloc(num_anchors, sizeof(AnchorTracking*));
    
    if (!system->anchors) {
        free(system);
        return NULL;
    }
    
    // Initialize statistics
    system->avg_error = 0.0;
    system->max_error = 0.0;
    system->min_error = 0.0;
    system->error_std_dev = 0.0;
    
    system->num_overestimates = 0;
    system->num_underestimates = 0;
    system->avg_overestimate = 0.0;
    system->avg_underestimate = 0.0;
    
    system->global_convergence = false;
    system->global_convergence_rate = 0.0;
    system->estimated_iterations_to_convergence = 0;
    
    system->lattice_spacing = 0.0;
    system->avg_neighbor_distance = 0.0;
    system->lattice_regularity = 0.0;
    
    // Initialize error direction histogram (36 bins for 10° each)
    system->histogram_bins = 36;
    system->error_direction_histogram = (double*)calloc(36, sizeof(double));
    
    return system;
}

/**
 * Add anchor to tracking system
 */
void add_anchor_to_system(
    AnchorTrackingSystem* system,
    AnchorTracking* anchor
) {
    if (!system || !anchor) return;
    
    // Find first empty slot
    for (uint32_t i = 0; i < system->num_anchors; i++) {
        if (system->anchors[i] == NULL) {
            system->anchors[i] = anchor;
            return;
        }
    }
}

/**
 * Update all anchors in system
 */
void update_all_anchors(
    AnchorTrackingSystem* system,
    uint32_t recursion_level
) {
    if (!system) return;
    
    (void)recursion_level;  // Unused for now, reserved for future use
    
    for (uint32_t i = 0; i < system->num_anchors; i++) {
        if (system->anchors[i]) {
            analyze_anchor_convergence(system->anchors[i]);
        }
    }
    
    // Recompute global statistics
    compute_global_statistics(system);
}

/**
 * Compute global statistics
 */
void compute_global_statistics(AnchorTrackingSystem* system) {
    if (!system) return;
    
    uint32_t count = 0;
    double sum_error = 0.0;
    double sum_sq_error = 0.0;
    double max_err = 0.0;
    double min_err = 1e100;
    
    uint32_t num_over = 0;
    uint32_t num_under = 0;
    double sum_over = 0.0;
    double sum_under = 0.0;
    
    for (uint32_t i = 0; i < system->num_anchors; i++) {
        AnchorTracking* anchor = system->anchors[i];
        if (!anchor || !anchor->estimated_k) continue;
        
        double err = anchor->error_magnitude;
        sum_error += err;
        sum_sq_error += err * err;
        
        if (err > max_err) max_err = err;
        if (err < min_err) min_err = err;
        
        if (anchor->error_direction > 0) {
            num_over++;
            sum_over += err;
        } else if (anchor->error_direction < 0) {
            num_under++;
            sum_under += err;
        }
        
        count++;
    }
    
    if (count > 0) {
        system->avg_error = sum_error / count;
        system->max_error = max_err;
        system->min_error = min_err;
        
        // Compute standard deviation
        double variance = (sum_sq_error / count) - (system->avg_error * system->avg_error);
        system->error_std_dev = math_sqrt(math_abs(variance));
        
        system->num_overestimates = num_over;
        system->num_underestimates = num_under;
        
        if (num_over > 0) system->avg_overestimate = sum_over / num_over;
        if (num_under > 0) system->avg_underestimate = sum_under / num_under;
    }
    
    // Check global convergence
    system->global_convergence = check_global_convergence(system);
}

/**
 * Analyze lattice structure
 */
void analyze_lattice_structure(AnchorTrackingSystem* system) {
    if (!system || system->num_anchors < 2) return;
    
    // Compute average distance between anchors
    double sum_distances = 0.0;
    uint32_t num_pairs = 0;
    
    for (uint32_t i = 0; i < system->num_anchors; i++) {
        AnchorTracking* anchor1 = system->anchors[i];
        if (!anchor1) continue;
        
        for (uint32_t j = i + 1; j < system->num_anchors; j++) {
            AnchorTracking* anchor2 = system->anchors[j];
            if (!anchor2) continue;
            
            // Compute Euclidean distance in 13D space
            double dist_sq = 0.0;
            for (uint32_t d = 0; d < 13; d++) {
                double diff = anchor1->lattice_position[d] - anchor2->lattice_position[d];
                dist_sq += diff * diff;
            }
            
            double dist = math_sqrt(dist_sq);
            sum_distances += dist;
            num_pairs++;
        }
    }
    
    if (num_pairs > 0) {
        system->avg_neighbor_distance = sum_distances / num_pairs;
        system->lattice_spacing = system->avg_neighbor_distance;
        
        // Compute lattice regularity (coefficient of variation)
        // Lower = more regular
        double sum_sq_dev = 0.0;
        for (uint32_t i = 0; i < system->num_anchors; i++) {
            AnchorTracking* anchor1 = system->anchors[i];
            if (!anchor1) continue;
            
            for (uint32_t j = i + 1; j < system->num_anchors; j++) {
                AnchorTracking* anchor2 = system->anchors[j];
                if (!anchor2) continue;
                
                double dist_sq = 0.0;
                for (uint32_t d = 0; d < 13; d++) {
                    double diff = anchor1->lattice_position[d] - anchor2->lattice_position[d];
                    dist_sq += diff * diff;
                }
                
                double dist = math_sqrt(dist_sq);
                double dev = dist - system->avg_neighbor_distance;
                sum_sq_dev += dev * dev;
            }
        }
        
        double variance = sum_sq_dev / num_pairs;
        double std_dev = math_sqrt(variance);
        system->lattice_regularity = std_dev / system->avg_neighbor_distance;
    }
}

/**
 * Analyze error patterns
 */
void analyze_error_patterns(AnchorTrackingSystem* system) {
    if (!system) return;
    
    // Reset histogram
    memset(system->error_direction_histogram, 0, system->histogram_bins * sizeof(double));
    
    // Build histogram of error angles
    for (uint32_t i = 0; i < system->num_anchors; i++) {
        AnchorTracking* anchor = system->anchors[i];
        if (!anchor || !anchor->estimated_k) continue;
        
        // Convert error angle to bin index
        double angle = anchor->error_angle + 180.0;  // Shift to [0, 360)
        uint32_t bin = (uint32_t)(angle / 10.0);  // 10° per bin
        if (bin >= system->histogram_bins) bin = system->histogram_bins - 1;
        
        system->error_direction_histogram[bin] += 1.0;
    }
    
    // Normalize histogram
    double total = 0.0;
    for (uint32_t i = 0; i < system->histogram_bins; i++) {
        total += system->error_direction_histogram[i];
    }
    
    if (total > 0.0) {
        for (uint32_t i = 0; i < system->histogram_bins; i++) {
            system->error_direction_histogram[i] /= total;
        }
    }
}

/**
 * Check global convergence
 */
bool check_global_convergence(const AnchorTrackingSystem* system) {
    if (!system) return false;
    
    uint32_t converging_count = 0;
    uint32_t total_count = 0;
    double sum_convergence_rate = 0.0;
    
    for (uint32_t i = 0; i < system->num_anchors; i++) {
        AnchorTracking* anchor = system->anchors[i];
        if (!anchor || !anchor->estimated_k) continue;
        
        if (anchor->is_converging) {
            converging_count++;
            sum_convergence_rate += anchor->convergence_rate;
        }
        total_count++;
    }
    
    if (total_count == 0) return false;
    
    // Global convergence if >80% of anchors are converging
    bool converging = (converging_count >= (total_count * 4 / 5));
    
    // Compute global convergence rate
    if (converging_count > 0) {
        ((AnchorTrackingSystem*)system)->global_convergence_rate = sum_convergence_rate / converging_count;
    }
    
    return converging;
}

/**
 * Print anchor tracking info
 */
void print_anchor_tracking(const AnchorTracking* anchor) {
    if (!anchor) return;
    
    printf("Anchor #%lu:\n", anchor->anchor_id);
    
    if (anchor->real_k) {
        char* real_k_str = BN_bn2dec(anchor->real_k);
        printf("  Real k: %s\n", real_k_str);
        OPENSSL_free(real_k_str);
    }
    
    if (anchor->estimated_k) {
        char* est_k_str = BN_bn2dec(anchor->estimated_k);
        printf("  Estimated k: %s\n", est_k_str);
        OPENSSL_free(est_k_str);
    }
    
    printf("  Error magnitude: %.6f\n", anchor->error_magnitude);
    printf("  Error direction: %s\n", 
           anchor->error_direction > 0 ? "Overestimate" : 
           anchor->error_direction < 0 ? "Underestimate" : "Exact");
    printf("  Converging: %s\n", anchor->is_converging ? "YES" : "NO");
    printf("  Convergence rate: %.6f\n", anchor->convergence_rate);
    printf("  Recursion levels: %u\n", anchor->num_recursion_levels);
}

/**
 * Print system summary
 */
void print_tracking_system_summary(const AnchorTrackingSystem* system) {
    if (!system) return;
    
    printf("\n=== Anchor Tracking System Summary ===\n");
    printf("Total anchors: %u\n", system->num_anchors);
    printf("Average error: %.6f\n", system->avg_error);
    printf("Max error: %.6f\n", system->max_error);
    printf("Min error: %.6f\n", system->min_error);
    printf("Error std dev: %.6f\n", system->error_std_dev);
    printf("Overestimates: %u (avg: %.6f)\n", system->num_overestimates, system->avg_overestimate);
    printf("Underestimates: %u (avg: %.6f)\n", system->num_underestimates, system->avg_underestimate);
    printf("Global convergence: %s\n", system->global_convergence ? "YES" : "NO");
    printf("Global convergence rate: %.6f\n", system->global_convergence_rate);
    printf("Lattice spacing: %.6f\n", system->lattice_spacing);
    printf("Lattice regularity: %.6f\n", system->lattice_regularity);
}

/**
 * Print detailed system report
 */
void print_detailed_tracking_report(const AnchorTrackingSystem* system) {
    if (!system) return;
    
    print_tracking_system_summary(system);
    
    printf("\n=== Individual Anchor Details ===\n");
    for (uint32_t i = 0; i < system->num_anchors; i++) {
        if (system->anchors[i]) {
            print_anchor_tracking(system->anchors[i]);
            printf("\n");
        }
    }
}

/**
 * Export tracking data to CSV
 */
void export_tracking_to_csv(const AnchorTrackingSystem* system, const char* filename) {
    if (!system || !filename) return;
    
    FILE* f = fopen(filename, "w");
    if (!f) return;
    
    // Header
    fprintf(f, "anchor_id,real_k,estimated_k,error_magnitude,error_direction,is_converging,convergence_rate,num_levels\n");
    
    // Data
    for (uint32_t i = 0; i < system->num_anchors; i++) {
        AnchorTracking* anchor = system->anchors[i];
        if (!anchor) continue;
        
        char* real_k_str = anchor->real_k ? BN_bn2dec(anchor->real_k) : strdup("N/A");
        char* est_k_str = anchor->estimated_k ? BN_bn2dec(anchor->estimated_k) : strdup("N/A");
        
        fprintf(f, "%lu,%s,%s,%.6f,%d,%d,%.6f,%u\n",
                anchor->anchor_id,
                real_k_str,
                est_k_str,
                anchor->error_magnitude,
                anchor->error_direction,
                anchor->is_converging ? 1 : 0,
                anchor->convergence_rate,
                anchor->num_recursion_levels);
        
        if (anchor->real_k) OPENSSL_free(real_k_str);
        else free(real_k_str);
        
        if (anchor->estimated_k) OPENSSL_free(est_k_str);
        else free(est_k_str);
    }
    
    fclose(f);
}

/**
 * Visualize error vectors (generates data for plotting)
 */
void visualize_error_vectors(const AnchorTrackingSystem* system, const char* filename) {
    if (!system || !filename) return;
    
    FILE* f = fopen(filename, "w");
    if (!f) return;
    
    // Header
    fprintf(f, "anchor_id");
    for (uint32_t d = 0; d < 13; d++) {
        fprintf(f, ",dim%u", d);
    }
    fprintf(f, "\n");
    
    // Data
    for (uint32_t i = 0; i < system->num_anchors; i++) {
        AnchorTracking* anchor = system->anchors[i];
        if (!anchor) continue;
        
        fprintf(f, "%lu", anchor->anchor_id);
        for (uint32_t d = 0; d < 13; d++) {
            fprintf(f, ",%.6f", anchor->error_vector[d]);
        }
        fprintf(f, "\n");
    }
    
    fclose(f);
}

/**
 * Visualize convergence patterns
 */
void visualize_convergence(const AnchorTrackingSystem* system, const char* filename) {
    if (!system || !filename) return;
    
    FILE* f = fopen(filename, "w");
    if (!f) return;
    
    // Header
    fprintf(f, "anchor_id,level,error,convergence_rate\n");
    
    // Data
    for (uint32_t i = 0; i < system->num_anchors; i++) {
        AnchorTracking* anchor = system->anchors[i];
        if (!anchor) continue;
        
        for (uint32_t level = 0; level < anchor->num_recursion_levels; level++) {
            fprintf(f, "%lu,%u,%.6f,%.6f\n",
                    anchor->anchor_id,
                    level,
                    anchor->errors_per_level[level],
                    anchor->convergence_rates[level]);
        }
    }
    
    fclose(f);
}

/**
 * Free anchor tracking system
 */
void free_anchor_tracking_system(AnchorTrackingSystem* system) {
    if (!system) return;
    
    if (system->anchors) {
        for (uint32_t i = 0; i < system->num_anchors; i++) {
            if (system->anchors[i]) {
                free_anchor_tracking(system->anchors[i]);
            }
        }
        free(system->anchors);
    }
    
    if (system->error_direction_histogram) {
        free(system->error_direction_histogram);
    }
    
    free(system);
}