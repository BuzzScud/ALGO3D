#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/obj_mac.h>
#include "../include/geometric_recovery.h"

#define NUM_KNOWN_K 100
#define NUM_ANCHORS 100
#define PI 3.14159265358979323846
#define GOLDEN_RATIO 1.61803398874989484820
#define PI_PHI (PI * GOLDEN_RATIO)

// Compute π×φ distance
double pi_phi_distance(const double* p1, const double* p2, int dims) {
    double sum = 0.0;
    for (int d = 0; d < dims; d++) {
        double diff = p1[d] - p2[d];
        sum += diff * diff;
    }
    return sqrt(sum) / PI_PHI;
}

// Compute distance from origin
double distance_from_origin(const double* p, int dims) {
    double sum = 0.0;
    for (int d = 0; d < dims; d++) {
        sum += p[d] * p[d];
    }
    return sqrt(sum) / PI_PHI;
}

// Torus structure
typedef struct {
    double center[13];           // Torus center
    double major_radius;         // Distance from origin to torus center
    double minor_radius;         // Torus tube radius
    double* point_positions;     // All points on torus [NUM_KNOWN_K][13]
    int num_points;
    double entropy;              // Entropy measure
    bool is_stable;              // Oscillation stability
} TorusStructure;

// Compute torus from known k positions
TorusStructure* compute_torus(double positions[][13], int num_points) {
    TorusStructure* torus = calloc(1, sizeof(TorusStructure));
    torus->num_points = num_points;
    torus->point_positions = malloc(num_points * 13 * sizeof(double));
    
    // Copy positions
    for (int i = 0; i < num_points; i++) {
        for (int d = 0; d < 13; d++) {
            torus->point_positions[i * 13 + d] = positions[i][d];
        }
    }
    
    // Compute center (average position)
    for (int d = 0; d < 13; d++) {
        torus->center[d] = 0.0;
        for (int i = 0; i < num_points; i++) {
            torus->center[d] += positions[i][d];
        }
        torus->center[d] /= num_points;
    }
    
    // Compute major radius (distance from origin to center)
    torus->major_radius = distance_from_origin(torus->center, 13);
    
    // Compute minor radius (average distance from points to center)
    torus->minor_radius = 0.0;
    for (int i = 0; i < num_points; i++) {
        torus->minor_radius += pi_phi_distance(positions[i], torus->center, 13);
    }
    torus->minor_radius /= num_points;
    
    // Compute entropy (variance in distances)
    double variance = 0.0;
    for (int i = 0; i < num_points; i++) {
        double dist = pi_phi_distance(positions[i], torus->center, 13);
        double diff = dist - torus->minor_radius;
        variance += diff * diff;
    }
    variance /= num_points;
    torus->entropy = sqrt(variance);
    
    // Check stability (low entropy = stable)
    torus->is_stable = (torus->entropy < 0.05);
    
    return torus;
}

// Find intersection curves between tori
typedef struct {
    int torus1_idx;
    int torus2_idx;
    double intersection_points[100][13];
    int num_intersection_points;
    double curve_length;
} MyTorusIntersectionCurve;

MyTorusIntersectionCurve* find_torus_intersections(TorusStructure** tori, int num_tori) {
    // For now, just detect if tori intersect
    // Full implementation would compute actual intersection curves
    MyTorusIntersectionCurve* curves = malloc(num_tori * num_tori * sizeof(MyTorusIntersectionCurve));
    int curve_count = 0;
    
    for (int i = 0; i < num_tori; i++) {
        for (int j = i + 1; j < num_tori; j++) {
            // Check if tori intersect
            double center_dist = pi_phi_distance(tori[i]->center, tori[j]->center, 13);
            double sum_radii = tori[i]->minor_radius + tori[j]->minor_radius;
            double diff_radii = fabs(tori[i]->minor_radius - tori[j]->minor_radius);
            
            if (center_dist < sum_radii && center_dist > diff_radii) {
                // Tori intersect!
                curves[curve_count].torus1_idx = i;
                curves[curve_count].torus2_idx = j;
                curves[curve_count].num_intersection_points = 0;
                curves[curve_count].curve_length = 0.0;
                curve_count++;
            }
        }
    }
    
    return curves;
}

int main(void) {
    printf("=== RECURSIVE TORUS MAPPING & OSCILLATION ANALYSIS ===\n");
    printf("Generating %d known k values to map torus structure\n\n", NUM_KNOWN_K);
    
    // Initialize
    EC_GROUP* curve = EC_GROUP_new_by_curve_name(NID_secp128r1);
    BN_CTX* bn_ctx = BN_CTX_new();
    BIGNUM* order = BN_new();
    EC_GROUP_get_order(curve, order, bn_ctx);
    const EC_POINT* G = EC_GROUP_get0_generator(curve);
    
    // Generate 100 known k values
    printf("=== STEP 1: GENERATE KNOWN K VALUES ===\n");
    BIGNUM** known_k = malloc(NUM_KNOWN_K * sizeof(BIGNUM*));
    EC_POINT** known_Q = malloc(NUM_KNOWN_K * sizeof(EC_POINT*));
    double known_k_positions[NUM_KNOWN_K][13];
    
    for (int i = 0; i < NUM_KNOWN_K; i++) {
        known_k[i] = BN_new();
        BN_rand_range(known_k[i], order);
        known_Q[i] = EC_POINT_new(curve);
        EC_POINT_mul(curve, known_Q[i], known_k[i], NULL, NULL, bn_ctx);
        map_k_to_position(known_k[i], known_k_positions[i], 13);
        
        if ((i + 1) % 10 == 0) {
            printf("  Generated %d known k values...\n", i + 1);
        }
    }
    printf("✓ Generated %d known k values\n\n", NUM_KNOWN_K);
    
    // Generate anchors
    printf("=== STEP 2: GENERATE INITIAL ANCHORS ===\n");
    BIGNUM** anchor_k = malloc(NUM_ANCHORS * sizeof(BIGNUM*));
    EC_POINT** anchor_Q = malloc(NUM_ANCHORS * sizeof(EC_POINT*));
    
    for (int i = 0; i < NUM_ANCHORS; i++) {
        anchor_k[i] = BN_new();
        BN_rand_range(anchor_k[i], order);
        anchor_Q[i] = EC_POINT_new(curve);
        EC_POINT_mul(curve, anchor_Q[i], anchor_k[i], NULL, NULL, bn_ctx);
    }
    printf("✓ Generated %d anchors\n\n", NUM_ANCHORS);
    
    // Create recovery context
    printf("=== STEP 3: CREATE RECOVERY CONTEXT ===\n");
    GeometricRecoveryContext* ctx = geometric_recovery_create(curve, NUM_ANCHORS, 13);
    for (int i = 0; i < NUM_ANCHORS; i++) {
        geometric_recovery_add_anchor(ctx, anchor_k[i], anchor_Q[i]);
    }
    geometric_recovery_initialize(ctx);
    printf("✓ Recovery context initialized\n");
    printf("  Graph vertices: %u\n", ctx->model->num_vertices);
    printf("  Dimensions: %u\n\n", ctx->model->num_dimensions);
    
    // === STEP 4: MAP TORUS STRUCTURE ===
    printf("=== STEP 4: MAP TORUS STRUCTURE ===\n");
    TorusStructure* main_torus = compute_torus(known_k_positions, NUM_KNOWN_K);
    
    printf("Torus properties:\n");
    printf("  Center position:\n");
    for (int d = 0; d < 13; d++) {
        printf("    Dim %2d: %.6f\n", d, main_torus->center[d]);
    }
    printf("  Major radius (from origin): %.6f\n", main_torus->major_radius);
    printf("  Minor radius (tube): %.6f\n", main_torus->minor_radius);
    printf("  Entropy: %.6f\n", main_torus->entropy);
    printf("  Stable: %s\n\n", main_torus->is_stable ? "YES ✓" : "NO ✗");
    
    // === STEP 5: ANALYZE PARTITION BOUNDARIES ===
    printf("=== STEP 5: ANALYZE PARTITION BOUNDARIES ===\n");
    
    int num_uniform = 0;
    int num_inside = 0;
    int num_outside = 0;
    
    for (int t = 0; t < NUM_KNOWN_K; t++) {
        // Find 3 nearest anchors
        int nearest[3];
        double nearest_dists[3] = {1e100, 1e100, 1e100};
        
        for (int a = 0; a < NUM_ANCHORS; a++) {
            double dist = pi_phi_distance(known_k_positions[t], ctx->anchor_k_positions[a], 13);
            
            for (int i = 0; i < 3; i++) {
                if (dist < nearest_dists[i]) {
                    for (int j = 2; j > i; j--) {
                        nearest[j] = nearest[j-1];
                        nearest_dists[j] = nearest_dists[j-1];
                    }
                    nearest[i] = a;
                    nearest_dists[i] = dist;
                    break;
                }
            }
        }
        
        // Check uniformity
        double avg_dist = (nearest_dists[0] + nearest_dists[1] + nearest_dists[2]) / 3.0;
        double variance = 0.0;
        for (int i = 0; i < 3; i++) {
            double diff = nearest_dists[i] - avg_dist;
            variance += diff * diff;
        }
        variance /= 3.0;
        double stddev = sqrt(variance);
        
        if (stddev < 0.01) {
            num_uniform++;
        }
        
        // Check if inside partition
        double centroid[13];
        for (int d = 0; d < 13; d++) {
            centroid[d] = (ctx->anchor_k_positions[nearest[0]][d] +
                          ctx->anchor_k_positions[nearest[1]][d] +
                          ctx->anchor_k_positions[nearest[2]][d]) / 3.0;
        }
        
        double dist_to_centroid = pi_phi_distance(known_k_positions[t], centroid, 13);
        double avg_vertex_to_centroid = 0.0;
        for (int i = 0; i < 3; i++) {
            avg_vertex_to_centroid += pi_phi_distance(ctx->anchor_k_positions[nearest[i]], centroid, 13);
        }
        avg_vertex_to_centroid /= 3.0;
        
        if (dist_to_centroid < avg_vertex_to_centroid) {
            num_inside++;
        } else {
            num_outside++;
        }
    }
    
    printf("Partition analysis:\n");
    printf("  Uniform π×φ distances: %d/%d (%.1f%%)\n", num_uniform, NUM_KNOWN_K, (num_uniform * 100.0) / NUM_KNOWN_K);
    printf("  Inside partitions: %d/%d (%.1f%%)\n", num_inside, NUM_KNOWN_K, (num_inside * 100.0) / NUM_KNOWN_K);
    printf("  Outside/boundary: %d/%d (%.1f%%)\n\n", num_outside, NUM_KNOWN_K, (num_outside * 100.0) / NUM_KNOWN_K);
    
    // === STEP 6: DETECT ADDITIONAL OSCILLATIONS ===
    printf("=== STEP 6: DETECT ADDITIONAL OSCILLATIONS ===\n");
    
    // Analyze distance distribution
    double distances_from_torus_center[NUM_KNOWN_K];
    for (int i = 0; i < NUM_KNOWN_K; i++) {
        distances_from_torus_center[i] = pi_phi_distance(known_k_positions[i], main_torus->center, 13);
    }
    
    // Find peaks in distance distribution (indicates additional tori)
    printf("Distance distribution from torus center:\n");
    printf("  Min: %.6f\n", distances_from_torus_center[0]);
    printf("  Max: %.6f\n", distances_from_torus_center[0]);
    
    double min_dist = distances_from_torus_center[0];
    double max_dist = distances_from_torus_center[0];
    for (int i = 1; i < NUM_KNOWN_K; i++) {
        if (distances_from_torus_center[i] < min_dist) min_dist = distances_from_torus_center[i];
        if (distances_from_torus_center[i] > max_dist) max_dist = distances_from_torus_center[i];
    }
    printf("  Min: %.6f\n", min_dist);
    printf("  Max: %.6f\n", max_dist);
    printf("  Range: %.6f\n", max_dist - min_dist);
    
    // Check if range suggests multiple tori
    if ((max_dist - min_dist) > (2.0 * main_torus->minor_radius)) {
        printf("  ✓ MULTIPLE TORI DETECTED (range > 2×minor_radius)\n");
        printf("  → Need to recurse and identify additional tori\n\n");
    } else {
        printf("  ✗ Single torus (range ≤ 2×minor_radius)\n\n");
    }
    
    // === STEP 7: COMPUTE ENTROPY ===
    printf("=== STEP 7: COMPUTE ENTROPY ===\n");
    printf("Torus entropy: %.6f\n", main_torus->entropy);
    
    if (main_torus->entropy < 0.05) {
        printf("  ✓ LOW ENTROPY - Torus is STABLE\n");
        printf("  → Oscillations have converged\n");
        printf("  → Current resolution is sufficient\n\n");
    } else if (main_torus->entropy < 0.15) {
        printf("  ⚠ MEDIUM ENTROPY - Torus is CONVERGING\n");
        printf("  → Continue iteration to stabilize\n\n");
    } else {
        printf("  ✗ HIGH ENTROPY - Torus is UNSTABLE\n");
        printf("  → TRIGGER RECURSION: Scale up resolution\n");
        printf("  → Need more anchors or higher dimensional space\n\n");
    }
    
    // === STEP 8: RECURSION DECISION ===
    printf("=== STEP 8: RECURSION DECISION ===\n");
    
    bool needs_recursion = false;
    char* recursion_reason = NULL;
    
    if (!main_torus->is_stable) {
        needs_recursion = true;
        recursion_reason = "High entropy - oscillations not stable";
    } else if ((max_dist - min_dist) > (2.0 * main_torus->minor_radius)) {
        needs_recursion = true;
        recursion_reason = "Multiple tori detected - need finer resolution";
    } else if (num_uniform < (NUM_KNOWN_K * 0.5)) {
        needs_recursion = true;
        recursion_reason = "Less than 50% uniform distances - partition boundaries unclear";
    }
    
    if (needs_recursion) {
        printf("✗ RECURSION REQUIRED\n");
        printf("  Reason: %s\n", recursion_reason);
        printf("\n  Next steps:\n");
        printf("  1. Scale up graph resolution (2^24 → 2^26)\n");
        printf("  2. Add more anchors (100 → 1000)\n");
        printf("  3. Increase dimensions (13 → 26)\n");
        printf("  4. Re-run analysis\n");
        printf("  5. Repeat until entropy stabilizes\n\n");
    } else {
        printf("✓ NO RECURSION NEEDED\n");
        printf("  Torus is stable\n");
        printf("  Partition boundaries are clear\n");
        printf("  Ready for k recovery\n\n");
    }
    
    // === STEP 9: SUMMARY ===
    printf("=== STEP 9: ALGORITHM SUMMARY ===\n");
    printf("\nComplete recursive algorithm:\n");
    printf("1. Generate known k values → Map positions\n");
    printf("2. Generate initial anchors\n");
    printf("3. Create recovery context with initial resolution\n");
    printf("4. Map torus structure from known k\n");
    printf("5. Analyze partition boundaries (π×φ metric)\n");
    printf("6. Detect additional oscillations (multiple tori)\n");
    printf("7. Compute entropy (stability measure)\n");
    printf("8. Decision: Recurse or proceed?\n");
    printf("   - If unstable → Scale up and repeat\n");
    printf("   - If stable → Use for k recovery\n");
    printf("9. Iterate until convergence\n\n");
    
    printf("Key insights:\n");
    printf("• π×φ metric reveals circular/spherical structure\n");
    printf("• Uniform distances indicate partition boundaries\n");
    printf("• Entropy measures oscillation stability\n");
    printf("• Multiple tori = need higher resolution\n");
    printf("• Recursion continues until stable\n\n");
    
    // Cleanup
    free(main_torus->point_positions);
    free(main_torus);
    
    for (int i = 0; i < NUM_KNOWN_K; i++) {
        BN_free(known_k[i]);
        EC_POINT_free(known_Q[i]);
    }
    free(known_k);
    free(known_Q);
    
    for (int i = 0; i < NUM_ANCHORS; i++) {
        BN_free(anchor_k[i]);
        EC_POINT_free(anchor_Q[i]);
    }
    free(anchor_k);
    free(anchor_Q);
    
    geometric_recovery_free(ctx);
    BN_free(order);
    BN_CTX_free(bn_ctx);
    EC_GROUP_free(curve);
    
    printf("=== ANALYSIS COMPLETE ===\n");
    return 0;
}