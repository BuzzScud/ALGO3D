#include "../include/integrated_recovery.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define PHI 1.618033988749895
#define PI 3.141592653589793
#define TWO_PI (2.0 * PI)

// 13 dimensional frequencies
static const uint64_t DIMENSIONAL_FREQUENCIES[13] = {
    3, 7, 31, 12, 19, 5, 11, 13, 17, 23, 29, 37, 41
};

// Bit length index mapping
static int get_bitlen_index(int bit_length) {
    int bit_lengths[] = {8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256};
    for (int i = 0; i < 16; i++) {
        if (bit_lengths[i] == bit_length) return i;
    }
    return -1;
}

/**
 * Generate simple geometric anchors (50 Platonic vertices in 13D)
 */
static SimpleGeometricAnchor* generate_simple_anchors(int* num_anchors) {
    // Total: 4 + 8 + 6 + 20 + 12 = 50 vertices
    SimpleGeometricAnchor* anchors = (SimpleGeometricAnchor*)calloc(50, sizeof(SimpleGeometricAnchor));
    if (!anchors) return NULL;
    
    int idx = 0;
    
    // Tetrahedron: 4 vertices
    for (int v = 0; v < 4; v++) {
        anchors[idx].anchor_id = idx;
        double angle = v * TWO_PI / 4.0;
        for (int d = 0; d < 13; d++) {
            double phi_d = (double)DIMENSIONAL_FREQUENCIES[d];
            anchors[idx].position[d] = math_cos(angle * phi_d) * math_pow(PHI, d % 3);
        }
        idx++;
    }
    
    // Cube: 8 vertices
    for (int v = 0; v < 8; v++) {
        anchors[idx].anchor_id = idx;
        int x = (v & 1) ? 1 : -1;
        int y = (v & 2) ? 1 : -1;
        int z = (v & 4) ? 1 : -1;
        for (int d = 0; d < 13; d++) {
            double phi_d = (double)DIMENSIONAL_FREQUENCIES[d];
            anchors[idx].position[d] = 
                (x * math_cos(phi_d) + y * math_sin(phi_d) + z * math_cos(2.0 * phi_d)) / math_sqrt(3.0);
        }
        idx++;
    }
    
    // Octahedron: 6 vertices
    for (int v = 0; v < 6; v++) {
        anchors[idx].anchor_id = idx;
        double angle = v * TWO_PI / 6.0;
        for (int d = 0; d < 13; d++) {
            double phi_d = (double)DIMENSIONAL_FREQUENCIES[d];
            anchors[idx].position[d] = math_cos(angle * phi_d) * math_pow(PHI, d % 2);
        }
        idx++;
    }
    
    // Dodecahedron: 20 vertices
    for (int v = 0; v < 20; v++) {
        anchors[idx].anchor_id = idx;
        double angle = v * TWO_PI / 20.0;
        for (int d = 0; d < 13; d++) {
            double phi_d = (double)DIMENSIONAL_FREQUENCIES[d];
            anchors[idx].position[d] = math_cos(angle * phi_d * PHI) * math_pow(PHI, d % 5);
        }
        idx++;
    }
    
    // Icosahedron: 12 vertices
    for (int v = 0; v < 12; v++) {
        anchors[idx].anchor_id = idx;
        double angle = v * TWO_PI / 12.0;
        for (int d = 0; d < 13; d++) {
            double phi_d = (double)DIMENSIONAL_FREQUENCIES[d];
            anchors[idx].position[d] = math_cos(angle * phi_d) * math_pow(PHI, d % 4);
        }
        idx++;
    }
    
    *num_anchors = 50;
    return anchors;
}

/**
 * Create integrated recovery context
 */
IntegratedRecoveryContext* create_integrated_recovery(
    const char* samples_directory,
    int bit_length
) {
    IntegratedRecoveryContext* ctx = (IntegratedRecoveryContext*)calloc(1, sizeof(IntegratedRecoveryContext));
    if (!ctx) return NULL;
    
    printf("Creating integrated recovery context...\n");
    
    // Load ECDSA samples
    if (bit_length == 0) {
        ctx->samples = load_all_samples(samples_directory, &ctx->num_samples);
        printf("  ✓ Loaded %d samples (all bit lengths)\n", ctx->num_samples);
    } else {
        ctx->samples = load_samples_by_bit_length(samples_directory, bit_length, &ctx->num_samples);
        printf("  ✓ Loaded %d samples (%d-bit)\n", ctx->num_samples, bit_length);
    }
    
    if (!ctx->samples || ctx->num_samples == 0) {
        fprintf(stderr, "  ✗ Failed to load samples\n");
        free(ctx);
        return NULL;
    }
    
    // Generate geometric anchors (50 Platonic vertices)
    ctx->geo_anchors = generate_simple_anchors(&ctx->num_geo_anchors);
    if (!ctx->geo_anchors) {
        fprintf(stderr, "  ✗ Failed to generate geometric anchors\n");
        free_ecdsa_samples(ctx->samples, ctx->num_samples);
        free(ctx);
        return NULL;
    }
    printf("  ✓ Generated %d geometric anchors (Platonic solids)\n", ctx->num_geo_anchors);
    
    // Create anchor tracking system
    ctx->tracking_system = create_anchor_tracking_system(ctx->num_samples);
    if (!ctx->tracking_system) {
        fprintf(stderr, "  ✗ Failed to create tracking system\n");
        free(ctx->geo_anchors);
        free_ecdsa_samples(ctx->samples, ctx->num_samples);
        free(ctx);
        return NULL;
    }
    printf("  ✓ Created anchor tracking system (%d anchors)\n", ctx->num_samples);
    
    // Initialize statistics
    ctx->total_attempts = 0;
    ctx->successful_recoveries = 0;
    ctx->success_rate = 0.0;
    
    memset(ctx->attempts_per_bitlen, 0, sizeof(ctx->attempts_per_bitlen));
    memset(ctx->successes_per_bitlen, 0, sizeof(ctx->successes_per_bitlen));
    memset(ctx->success_rate_per_bitlen, 0, sizeof(ctx->success_rate_per_bitlen));
    
    printf("  ✓ Integrated recovery context created\n\n");
    
    return ctx;
}

/**
 * Free integrated recovery context
 */
void free_integrated_recovery(IntegratedRecoveryContext* ctx) {
    if (!ctx) return;
    
    if (ctx->geo_anchors) free(ctx->geo_anchors);
    if (ctx->tracking_system) free_anchor_tracking_system(ctx->tracking_system);
    if (ctx->samples) free_ecdsa_samples(ctx->samples, ctx->num_samples);
    
    free(ctx);
}

/**
 * Map k to 13D position using dimensional frequencies
 */
static void map_k_to_13d(uint64_t k, double* position) {
    for (int d = 0; d < 13; d++) {
        position[d] = (double)(k % DIMENSIONAL_FREQUENCIES[d]);
    }
}

/**
 * Find nearest geometric anchor
 */
static int find_nearest_geometric_anchor(
    const double* target_position,
    SimpleGeometricAnchor* anchors,
    int num_anchors
) {
    int nearest_idx = 0;
    double min_distance = 1e100;
    
    for (int i = 0; i < num_anchors; i++) {
        // Compute Euclidean distance in 13D
        double dist_sq = 0.0;
        for (int d = 0; d < 13; d++) {
            double diff = target_position[d] - anchors[i].position[d];
            dist_sq += diff * diff;
        }
        
        double dist = math_sqrt(dist_sq);
        
        if (dist < min_distance) {
            min_distance = dist;
            nearest_idx = i;
        }
    }
    
    return nearest_idx;
}

/**
 * Estimate k from geometric anchor
 */
static uint64_t estimate_k_from_anchor(
    int anchor_idx,
    int num_anchors,
    uint64_t max_k
) {
    // Map anchor index to k range
    // Distribute anchors evenly across [0, max_k]
    return (uint64_t)((double)anchor_idx * (double)max_k / (double)num_anchors);
}

/**
 * Simple recovery using geometric anchors
 */
/**
 * Multi-layer search implementation (from search_recovery_v2.c)
 */
static uint64_t multi_layer_search(
    double target_angle,
    uint64_t center_k,
    int num_layers,
    double* final_error
) {
    // Search layers: ±100 (coarse), ±25 (medium), ±10 (fine)
    typedef struct {
        int range;
        int step;
    } Layer;
    
    Layer layers[3] = {
        {100, 10},  // Coarse: ±100, step 10
        {25, 2},    // Medium: ±25, step 2
        {10, 1}     // Fine: ±10, step 1
    };
    
    uint64_t best_k = center_k;
    double best_error = 1e9;
    
    // Normalize target angle to [0, 2π)
    double normalized_target = target_angle;
    while (normalized_target < 0) normalized_target += TWO_PI;
    while (normalized_target >= TWO_PI) normalized_target -= TWO_PI;
    
    // Iterate through layers
    for (int layer = 0; layer < num_layers && layer < 3; layer++) {
        int64_t min_offset = -layers[layer].range;
        int64_t max_offset = layers[layer].range;
        
        // Search with current step size
        for (int64_t offset = min_offset; offset <= max_offset; offset += layers[layer].step) {
            int64_t candidate_k = (int64_t)best_k + offset;
            if (candidate_k < 0) continue;
            
            uint64_t k = (uint64_t)candidate_k;
            
            // Forward mapping: θ = k·π·φ
            double computed_angle = (double)k * PI * PHI;
            
            // Normalize to [0, 2π)
            while (computed_angle < 0) computed_angle += TWO_PI;
            while (computed_angle >= TWO_PI) computed_angle -= TWO_PI;
            
            // Compute error (handle wraparound)
            double error = math_abs(computed_angle - normalized_target);
            if (error > PI) error = TWO_PI - error;
            
            // Update best
            if (error < best_error) {
                best_error = error;
                best_k = k;
            }
        }
        
        // Center next layer on current best
        center_k = best_k;
    }
    
    if (final_error) *final_error = best_error;
    return best_k;
}

static uint64_t recover_k_simple(
    IntegratedRecoveryContext* ctx,
    ECDSASample* sample
) {
    // Compute Q position in 13D space
    double q_position[13];
    for (int d = 0; d < 13; d++) {
        // Use dimensional frequencies to compute position
        double freq = (double)DIMENSIONAL_FREQUENCIES[d];
        
        // Extract x, y coordinates from Q point (public key)
        char* qx_str = BN_bn2hex(sample->pubkey_x);
        char* qy_str = BN_bn2hex(sample->pubkey_y);
        
        // Convert to double (simplified - use first few bytes)
        uint64_t qx_val = strtoull(qx_str, NULL, 16);
        uint64_t qy_val = strtoull(qy_str, NULL, 16);
        
        // Compute position using π×φ metric
        double angle = (double)(qx_val % 360) * PI / 180.0;
        q_position[d] = math_cos(angle * freq) * math_pow(PHI, d % 5);
        
        OPENSSL_free(qx_str);
        OPENSSL_free(qy_str);
    }
    
    // Find 3 nearest geometric anchors
    typedef struct {
        int index;
        double distance;
        uint64_t k_estimate;
    } AnchorInfo;
    
    AnchorInfo nearest[3] = {{0, 1e9, 0}, {0, 1e9, 0}, {0, 1e9, 0}};
    
    for (int i = 0; i < ctx->num_geo_anchors; i++) {
        double distance = 0.0;
        for (int d = 0; d < 13; d++) {
            double diff = q_position[d] - ctx->geo_anchors[i].position[d];
            distance += diff * diff;
        }
        distance = math_sqrt(distance);
        
        // Update nearest 3
        for (int j = 0; j < 3; j++) {
            if (distance < nearest[j].distance) {
                // Shift down
                for (int k = 2; k > j; k--) {
                    nearest[k] = nearest[k-1];
                }
                nearest[j].index = i;
                nearest[j].distance = distance;
                // Map anchor index to k estimate (50 anchors → [0, 300])
                nearest[j].k_estimate = (uint64_t)((double)i * 300.0 / 50.0);
                break;
            }
        }
    }
    
    // Compute weighted k estimate (inverse distance weighting)
    double total_weight = 0.0;
    double weighted_sum = 0.0;
    
    for (int i = 0; i < 3; i++) {
        if (nearest[i].distance < 0.001) {
            // Very close to anchor, use it directly
            weighted_sum = (double)nearest[i].k_estimate;
            total_weight = 1.0;
            break;
        }
        double weight = 1.0 / nearest[i].distance;
        weighted_sum += weight * nearest[i].k_estimate;
        total_weight += weight;
    }
    
    uint64_t center_k = (uint64_t)(weighted_sum / total_weight);
    
    // Compute target angle from Q point (public key)
    char* qx_str = BN_bn2hex(sample->pubkey_x);
    uint64_t qx_val = strtoull(qx_str, NULL, 16);
    OPENSSL_free(qx_str);
    
    double target_angle = (double)(qx_val % 360) * PI / 180.0;
    
    // Perform multi-layer search
    double final_error = 0.0;
    uint64_t recovered_k = multi_layer_search(target_angle, center_k, 3, &final_error);
    
    return recovered_k;
}

/**
 * Run recovery on single sample
 */
RecoveryResult* run_integrated_recovery_single(
    IntegratedRecoveryContext* ctx,
    ECDSASample* sample,
    int sample_index
) {
    RecoveryResult* result = (RecoveryResult*)calloc(1, sizeof(RecoveryResult));
    if (!result) return NULL;
    
    // Start timer
    clock_t start = clock();
    
    // Create anchor tracking for this sample
    AnchorTracking* anchor = create_anchor_tracking(sample_index + 1, sample->k, 10);
    if (!anchor) {
        free(result);
        return NULL;
    }
    
    // Run recovery
    uint64_t recovered_k = recover_k_simple(ctx, sample);
    
    // Convert to BIGNUM for tracking
    BIGNUM* recovered_bn = BN_new();
    BN_set_word(recovered_bn, recovered_k);
    
    // Update anchor tracking
    update_anchor_estimate(anchor, recovered_bn, 0);
    
    // Get real k value
    char* real_k_str = BN_bn2dec(sample->k);
    uint64_t real_k = strtoull(real_k_str, NULL, 10);
    OPENSSL_free(real_k_str);
    
    // Compute error
    double error = math_abs((double)recovered_k - (double)real_k);
    
    // Check success (within 10% of real k)
    double tolerance = (double)real_k * 0.1;
    result->success = (error <= tolerance);
    
    result->recovered_k = recovered_k;
    result->real_k = real_k;
    result->error_magnitude = error;
    result->convergence_rate = anchor->convergence_rate;
    result->iterations = 1;
    
    // End timer
    clock_t end = clock();
    result->elapsed_time = (double)(end - start) / CLOCKS_PER_SEC;
    
    // Store anchor
    result->anchor = anchor;
    
    // Add to tracking system
    add_anchor_to_system(ctx->tracking_system, anchor);
    
    // Cleanup
    BN_free(recovered_bn);
    
    return result;
}

/**
 * Run recovery on all samples
 */
void run_integrated_recovery_all(IntegratedRecoveryContext* ctx) {
    if (!ctx) return;
    
    printf("\n========================================\n");
    printf("RUNNING INTEGRATED RECOVERY\n");
    printf("========================================\n\n");
    
    printf("Total samples: %d\n", ctx->num_samples);
    printf("Geometric anchors: %d\n", ctx->num_geo_anchors);
    printf("\n");
    
    // Process each sample
    for (int i = 0; i < ctx->num_samples; i++) {
        ECDSASample* sample = ctx->samples[i];
        
        printf("Sample %d/%d: %d-bit k... ", i + 1, ctx->num_samples, sample->bit_length);
        fflush(stdout);
        
        // Run recovery
        RecoveryResult* result = run_integrated_recovery_single(ctx, sample, i);
        
        if (result) {
            // Update statistics
            ctx->total_attempts++;
            
            int bitlen_idx = get_bitlen_index(sample->bit_length);
            if (bitlen_idx >= 0) {
                ctx->attempts_per_bitlen[bitlen_idx]++;
            }
            
            if (result->success) {
                ctx->successful_recoveries++;
                if (bitlen_idx >= 0) {
                    ctx->successes_per_bitlen[bitlen_idx]++;
                }
                printf("✓ SUCCESS (error: %.2f, time: %.4fs)\n", 
                       result->error_magnitude, result->elapsed_time);
            } else {
                printf("✗ FAILED (error: %.2f, time: %.4fs)\n",
                       result->error_magnitude, result->elapsed_time);
            }
            
            free(result);
        } else {
            printf("✗ ERROR\n");
        }
    }
    
    // Compute success rates
    if (ctx->total_attempts > 0) {
        ctx->success_rate = (double)ctx->successful_recoveries / (double)ctx->total_attempts;
    }
    
    for (int i = 0; i < 17; i++) {
        if (ctx->attempts_per_bitlen[i] > 0) {
            ctx->success_rate_per_bitlen[i] = 
                (double)ctx->successes_per_bitlen[i] / (double)ctx->attempts_per_bitlen[i];
        }
    }
    
    // Compute global statistics
    compute_global_statistics(ctx->tracking_system);
    
    printf("\n========================================\n");
    printf("RECOVERY COMPLETE\n");
    printf("========================================\n\n");
}

/**
 * Print summary statistics
 */
void print_recovery_summary(const IntegratedRecoveryContext* ctx) {
    if (!ctx) return;
    
    printf("\n=== Recovery Summary ===\n");
    printf("Total attempts: %u\n", ctx->total_attempts);
    printf("Successful recoveries: %u\n", ctx->successful_recoveries);
    printf("Overall success rate: %.2f%%\n", ctx->success_rate * 100.0);
    printf("\n");
    
    printf("Success rate by bit length:\n");
    int bit_lengths[] = {8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256};
    for (int i = 0; i < 16; i++) {
        if (ctx->attempts_per_bitlen[i] > 0) {
            printf("  %3d-bit: %u/%u (%.2f%%)\n",
                   bit_lengths[i],
                   ctx->successes_per_bitlen[i],
                   ctx->attempts_per_bitlen[i],
                   ctx->success_rate_per_bitlen[i] * 100.0);
        }
    }
    printf("\n");
    
    // Print tracking system summary
    print_tracking_system_summary(ctx->tracking_system);
}

/**
 * Generate comprehensive report
 */
void generate_recovery_report(
    IntegratedRecoveryContext* ctx,
    const char* output_file
) {
    if (!ctx || !output_file) return;
    
    FILE* f = fopen(output_file, "w");
    if (!f) return;
    
    fprintf(f, "# Integrated Recovery Report\n\n");
    fprintf(f, "## Overall Statistics\n\n");
    fprintf(f, "- Total attempts: %u\n", ctx->total_attempts);
    fprintf(f, "- Successful recoveries: %u\n", ctx->successful_recoveries);
    fprintf(f, "- Overall success rate: %.2f%%\n\n", ctx->success_rate * 100.0);
    
    fprintf(f, "## Success Rate by Bit Length\n\n");
    fprintf(f, "| Bit Length | Attempts | Successes | Success Rate |\n");
    fprintf(f, "|------------|----------|-----------|-------------|\n");
    
    int bit_lengths[] = {8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256};
    for (int i = 0; i < 16; i++) {
        if (ctx->attempts_per_bitlen[i] > 0) {
            fprintf(f, "| %d-bit | %u | %u | %.2f%% |\n",
                    bit_lengths[i],
                    ctx->attempts_per_bitlen[i],
                    ctx->successes_per_bitlen[i],
                    ctx->success_rate_per_bitlen[i] * 100.0);
        }
    }
    
    fprintf(f, "\n## Anchor Tracking Statistics\n\n");
    fprintf(f, "- Average error: %.6f\n", ctx->tracking_system->avg_error);
    fprintf(f, "- Max error: %.6f\n", ctx->tracking_system->max_error);
    fprintf(f, "- Min error: %.6f\n", ctx->tracking_system->min_error);
    fprintf(f, "- Error std dev: %.6f\n", ctx->tracking_system->error_std_dev);
    fprintf(f, "- Global convergence: %s\n", 
            ctx->tracking_system->global_convergence ? "YES" : "NO");
    fprintf(f, "- Global convergence rate: %.6f\n", 
            ctx->tracking_system->global_convergence_rate);
    
    fclose(f);
}

/**
 * Export results to CSV
 */
void export_recovery_results_csv(
    IntegratedRecoveryContext* ctx,
    const char* output_file
) {
    if (!ctx || !output_file) return;
    
    // Export tracking system to CSV
    export_tracking_to_csv(ctx->tracking_system, output_file);
}