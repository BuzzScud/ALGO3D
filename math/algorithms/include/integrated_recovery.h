#ifndef INTEGRATED_RECOVERY_H
#define INTEGRATED_RECOVERY_H

#include "anchor_tracking.h"
#include "ecdsa_sample_loader.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * Integrated Recovery System
 * 
 * Combines:
 * - Geometric anchors (50 Platonic vertices)
 * - Anchor tracking (convergence analysis)
 * - Real ECDSA samples (ground truth)
 * - Recovery algorithm
 * 
 * Note: Uses simple geometric anchor approach to avoid
 * type conflicts between different recovery implementations.
 */

// Simple geometric anchor (avoiding conflicts)
typedef struct {
    double position[13];      // 13D coordinates
    int anchor_id;            // Anchor identifier
} SimpleGeometricAnchor;

typedef struct {
    // Geometric anchors (50 Platonic vertices)
    SimpleGeometricAnchor* geo_anchors;
    int num_geo_anchors;
    
    // Anchor tracking system
    AnchorTrackingSystem* tracking_system;
    
    // ECDSA samples
    ECDSASample** samples;
    int num_samples;
    
    // Statistics
    uint32_t total_attempts;
    uint32_t successful_recoveries;
    double success_rate;
    
    // Per bit-length statistics
    uint32_t attempts_per_bitlen[17];      // 8, 16, 24, ..., 256
    uint32_t successes_per_bitlen[17];
    double success_rate_per_bitlen[17];
    
} IntegratedRecoveryContext;

/**
 * Create integrated recovery context
 */
IntegratedRecoveryContext* create_integrated_recovery(
    const char* samples_directory,
    int bit_length  // 0 = all bit lengths, or specific (8, 16, 32, etc.)
);

/**
 * Free integrated recovery context
 */
void free_integrated_recovery(IntegratedRecoveryContext* ctx);

/**
 * Run recovery on single sample
 */
typedef struct {
    bool success;
    uint64_t recovered_k;
    uint64_t real_k;
    double error_magnitude;
    double convergence_rate;
    uint32_t iterations;
    double elapsed_time;
    AnchorTracking* anchor;
} RecoveryResult;

RecoveryResult* run_integrated_recovery_single(
    IntegratedRecoveryContext* ctx,
    ECDSASample* sample,
    int sample_index
);

/**
 * Run recovery on all samples
 */
void run_integrated_recovery_all(IntegratedRecoveryContext* ctx);

/**
 * Generate comprehensive report
 */
void generate_recovery_report(
    IntegratedRecoveryContext* ctx,
    const char* output_file
);

/**
 * Export results to CSV
 */
void export_recovery_results_csv(
    IntegratedRecoveryContext* ctx,
    const char* output_file
);

/**
 * Print summary statistics
 */
void print_recovery_summary(const IntegratedRecoveryContext* ctx);

#endif // INTEGRATED_RECOVERY_H
