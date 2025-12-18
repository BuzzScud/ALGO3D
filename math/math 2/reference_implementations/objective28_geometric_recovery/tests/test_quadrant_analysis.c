/**
 * test_quadrant_analysis.c - Analyze Quadrant Mirroring and Polarity Flipping
 * 
 * Based on user insight: "quadratic relationship with mirrored quadrants on the 
 * clock face which inherently suggests polarity flipping between quadrants"
 * 
 * This test analyzes:
 * 1. Which quadrant each k value falls into
 * 2. Polarity patterns between quadrants
 * 3. Right/Left vs Top/Bottom relationships
 * 4. Mirroring patterns in the clock sudoku
 * 5. Additional patterns in the 4% successful recoveries
 */

#include "clock_recovery.h"
#include "platonic_solids.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Test k values (same 50 primes)
static const uint64_t TEST_K_VALUES[] = {
    2, 3, 5, 7, 11, 13, 17, 19, 23, 29,
    31, 37, 41, 43, 47, 53, 59, 61, 67, 71,
    73, 79, 83, 89, 97, 101, 103, 107, 109, 113,
    127, 131, 137, 139, 149, 151, 157, 163, 167, 173,
    179, 181, 191, 193, 197, 199, 211, 223, 227, 229
};
#define NUM_TEST_K 50

// Quadrant definitions (based on 12-hour clock)
typedef enum {
    QUADRANT_1 = 0,  // 12-3  (0-90°)   - Top Right
    QUADRANT_2 = 1,  // 3-6   (90-180°) - Bottom Right
    QUADRANT_3 = 2,  // 6-9   (180-270°)- Bottom Left
    QUADRANT_4 = 3   // 9-12  (270-360°)- Top Left
} Quadrant;

// Polarity (based on quadrant position)
typedef enum {
    POLARITY_POSITIVE = 0,  // Quadrants 1 & 4 (Right side)
    POLARITY_NEGATIVE = 1   // Quadrants 2 & 3 (Left side)
} Polarity;

// Vertical position
typedef enum {
    VERTICAL_TOP = 0,     // Quadrants 1 & 4
    VERTICAL_BOTTOM = 1   // Quadrants 2 & 3
} VerticalPosition;

// Structure to hold quadrant analysis
typedef struct {
    uint64_t k;
    ClockPosition position;
    Quadrant quadrant;
    Polarity polarity;
    VerticalPosition vertical;
    int position_mod_3;   // Position mod 3 (for 12 = 4×3 structure)
    int position_mod_4;   // Position mod 4 (for 12 = 3×4 structure)
    double angle_normalized; // Angle normalized to [0, 2π)
} QuadrantInfo;

// Forward declarations
Quadrant get_quadrant(ClockPosition pos);
Polarity get_polarity(Quadrant q);
VerticalPosition get_vertical(Quadrant q);
void analyze_quadrant_patterns(QuadrantInfo* infos, int count);
void analyze_mirroring(QuadrantInfo* infos, int count);
void analyze_polarity_flips(QuadrantInfo* infos, int count);
void analyze_successful_recoveries(QuadrantInfo* infos, int count);

int main() {
    printf("\n=== Quadrant Analysis - Clock Sudoku Pattern ===\n\n");
    
    // Allocate array for all quadrant info
    QuadrantInfo* infos = calloc(NUM_TEST_K, sizeof(QuadrantInfo));
    if (!infos) {
        printf("FAILED to allocate memory\n");
        return 1;
    }
    
    // Analyze each k value
    printf("Mapping %d k values to quadrants...\n\n", NUM_TEST_K);
    for (int i = 0; i < NUM_TEST_K; i++) {
        uint64_t k = TEST_K_VALUES[i];
        infos[i].k = k;
        infos[i].position = map_k_to_clock_u64(k);
        infos[i].quadrant = get_quadrant(infos[i].position);
        infos[i].polarity = get_polarity(infos[i].quadrant);
        infos[i].vertical = get_vertical(infos[i].quadrant);
        infos[i].position_mod_3 = infos[i].position.position % 3;
        infos[i].position_mod_4 = infos[i].position.position % 4;
        infos[i].angle_normalized = fmod(infos[i].position.angle, 2.0 * M_PI);
        if (infos[i].angle_normalized < 0) infos[i].angle_normalized += 2.0 * M_PI;
    }
    
    // Print first 20 for inspection
    printf("First 20 k values:\n");
    printf("%-6s %-6s %-4s %-8s %-10s %-10s %-8s %-8s %-10s\n",
           "k", "Ring", "Pos", "Quadrant", "Polarity", "Vertical", "Mod3", "Mod4", "Angle");
    printf("------------------------------------------------------------------------------------\n");
    
    for (int i = 0; i < 20 && i < NUM_TEST_K; i++) {
        const char* quad_names[] = {"Q1(TR)", "Q2(BR)", "Q3(BL)", "Q4(TL)"};
        const char* pol_names[] = {"POS(+)", "NEG(-)"};
        const char* vert_names[] = {"TOP", "BOTTOM"};
        
        printf("%-6lu %-6d %-4d %-8s %-10s %-10s %-8d %-8d %-10.4f\n",
               infos[i].k,
               infos[i].position.ring,
               infos[i].position.position,
               quad_names[infos[i].quadrant],
               pol_names[infos[i].polarity],
               vert_names[infos[i].vertical],
               infos[i].position_mod_3,
               infos[i].position_mod_4,
               infos[i].angle_normalized);
    }
    
    // Analyze patterns
    printf("\n=== Quadrant Distribution ===\n\n");
    analyze_quadrant_patterns(infos, NUM_TEST_K);
    
    printf("\n=== Mirroring Analysis ===\n\n");
    analyze_mirroring(infos, NUM_TEST_K);
    
    printf("\n=== Polarity Flip Analysis ===\n\n");
    analyze_polarity_flips(infos, NUM_TEST_K);
    
    printf("\n=== Successful Recovery Analysis ===\n\n");
    analyze_successful_recoveries(infos, NUM_TEST_K);
    
    // Cleanup
    free(infos);
    
    printf("\n=== Analysis Complete ===\n\n");
    
    return 0;
}

Quadrant get_quadrant(ClockPosition pos) {
    // Normalize angle to [0, 2π)
    double angle = fmod(pos.angle, 2.0 * M_PI);
    if (angle < 0) angle += 2.0 * M_PI;
    
    // Determine quadrant based on angle
    if (angle < M_PI / 2.0) {
        return QUADRANT_1;  // 0-90° (12-3)
    } else if (angle < M_PI) {
        return QUADRANT_2;  // 90-180° (3-6)
    } else if (angle < 3.0 * M_PI / 2.0) {
        return QUADRANT_3;  // 180-270° (6-9)
    } else {
        return QUADRANT_4;  // 270-360° (9-12)
    }
}

Polarity get_polarity(Quadrant q) {
    // Right side (Q1, Q4) = Positive
    // Left side (Q2, Q3) = Negative
    if (q == QUADRANT_1 || q == QUADRANT_4) {
        return POLARITY_POSITIVE;
    } else {
        return POLARITY_NEGATIVE;
    }
}

VerticalPosition get_vertical(Quadrant q) {
    // Top (Q1, Q4) = Top
    // Bottom (Q2, Q3) = Bottom
    if (q == QUADRANT_1 || q == QUADRANT_4) {
        return VERTICAL_TOP;
    } else {
        return VERTICAL_BOTTOM;
    }
}

void analyze_quadrant_patterns(QuadrantInfo* infos, int count) {
    // Count k values in each quadrant
    int quad_counts[4] = {0};
    int pol_counts[2] = {0};
    int vert_counts[2] = {0};
    
    for (int i = 0; i < count; i++) {
        quad_counts[infos[i].quadrant]++;
        pol_counts[infos[i].polarity]++;
        vert_counts[infos[i].vertical]++;
    }
    
    printf("Quadrant Distribution:\n");
    printf("  Q1 (Top Right):    %d k values (%.1f%%)\n", quad_counts[0], 100.0 * quad_counts[0] / count);
    printf("  Q2 (Bottom Right): %d k values (%.1f%%)\n", quad_counts[1], 100.0 * quad_counts[1] / count);
    printf("  Q3 (Bottom Left):  %d k values (%.1f%%)\n", quad_counts[2], 100.0 * quad_counts[2] / count);
    printf("  Q4 (Top Left):     %d k values (%.1f%%)\n", quad_counts[3], 100.0 * quad_counts[3] / count);
    
    printf("\nPolarity Distribution:\n");
    printf("  Positive (Right): %d k values (%.1f%%)\n", pol_counts[0], 100.0 * pol_counts[0] / count);
    printf("  Negative (Left):  %d k values (%.1f%%)\n", pol_counts[1], 100.0 * pol_counts[1] / count);
    
    printf("\nVertical Distribution:\n");
    printf("  Top:    %d k values (%.1f%%)\n", vert_counts[0], 100.0 * vert_counts[0] / count);
    printf("  Bottom: %d k values (%.1f%%)\n", vert_counts[1], 100.0 * vert_counts[1] / count);
}

void analyze_mirroring(QuadrantInfo* infos, int count) {
    printf("Analyzing quadratic mirroring patterns...\n\n");
    
    // Check for mirrored pairs (Q1↔Q3, Q2↔Q4)
    printf("Diagonal Mirroring (Q1↔Q3, Q2↔Q4):\n");
    
    for (int i = 0; i < count; i++) {
        for (int j = i + 1; j < count; j++) {
            // Check if they're diagonal mirrors
            bool is_diagonal_mirror = 
                (infos[i].quadrant == QUADRANT_1 && infos[j].quadrant == QUADRANT_3) ||
                (infos[i].quadrant == QUADRANT_3 && infos[j].quadrant == QUADRANT_1) ||
                (infos[i].quadrant == QUADRANT_2 && infos[j].quadrant == QUADRANT_4) ||
                (infos[i].quadrant == QUADRANT_4 && infos[j].quadrant == QUADRANT_2);
            
            if (is_diagonal_mirror) {
                // Check if positions are also mirrored
                int pos_diff = abs(infos[i].position.position - infos[j].position.position);
                double angle_diff = fabs(infos[i].angle_normalized - infos[j].angle_normalized);
                
                // Normalize angle difference
                if (angle_diff > M_PI) angle_diff = 2.0 * M_PI - angle_diff;
                
                // Check if they're close to π radians apart (180°)
                if (fabs(angle_diff - M_PI) < 0.5) {
                    printf("  Mirror pair: k=%lu (Q%d, pos=%d) ↔ k=%lu (Q%d, pos=%d), angle_diff=%.2f°\n",
                           infos[i].k, infos[i].quadrant + 1, infos[i].position.position,
                           infos[j].k, infos[j].quadrant + 1, infos[j].position.position,
                           angle_diff * 180.0 / M_PI);
                }
            }
        }
    }
    
    // Check for horizontal mirroring (Q1↔Q4, Q2↔Q3)
    printf("\nHorizontal Mirroring (Q1↔Q4, Q2↔Q3):\n");
    
    for (int i = 0; i < count; i++) {
        for (int j = i + 1; j < count; j++) {
            bool is_horizontal_mirror = 
                (infos[i].quadrant == QUADRANT_1 && infos[j].quadrant == QUADRANT_4) ||
                (infos[i].quadrant == QUADRANT_4 && infos[j].quadrant == QUADRANT_1) ||
                (infos[i].quadrant == QUADRANT_2 && infos[j].quadrant == QUADRANT_3) ||
                (infos[i].quadrant == QUADRANT_3 && infos[j].quadrant == QUADRANT_2);
            
            if (is_horizontal_mirror) {
                double angle_sum = infos[i].angle_normalized + infos[j].angle_normalized;
                // Check if they're symmetric around vertical axis
                if (fabs(angle_sum - 2.0 * M_PI) < 0.5 || fabs(angle_sum) < 0.5) {
                    printf("  Mirror pair: k=%lu (Q%d) ↔ k=%lu (Q%d), angle_sum=%.2f°\n",
                           infos[i].k, infos[i].quadrant + 1,
                           infos[j].k, infos[j].quadrant + 1,
                           angle_sum * 180.0 / M_PI);
                }
            }
        }
    }
}

void analyze_polarity_flips(QuadrantInfo* infos, int count) {
    printf("Analyzing polarity flip patterns...\n\n");
    
    // Track polarity flips between consecutive k values
    int flip_count = 0;
    int same_count = 0;
    
    printf("Polarity transitions:\n");
    for (int i = 1; i < count; i++) {
        if (infos[i].polarity != infos[i-1].polarity) {
            flip_count++;
            printf("  k=%lu → k=%lu: %s → %s (FLIP)\n",
                   infos[i-1].k, infos[i].k,
                   infos[i-1].polarity == POLARITY_POSITIVE ? "POS" : "NEG",
                   infos[i].polarity == POLARITY_POSITIVE ? "POS" : "NEG");
        } else {
            same_count++;
        }
    }
    
    printf("\nPolarity Flip Statistics:\n");
    printf("  Flips: %d (%.1f%%)\n", flip_count, 100.0 * flip_count / (count - 1));
    printf("  Same:  %d (%.1f%%)\n", same_count, 100.0 * same_count / (count - 1));
    
    // Analyze vertical flips
    int vert_flip_count = 0;
    int vert_same_count = 0;
    
    for (int i = 1; i < count; i++) {
        if (infos[i].vertical != infos[i-1].vertical) {
            vert_flip_count++;
        } else {
            vert_same_count++;
        }
    }
    
    printf("\nVertical Flip Statistics:\n");
    printf("  Flips: %d (%.1f%%)\n", vert_flip_count, 100.0 * vert_flip_count / (count - 1));
    printf("  Same:  %d (%.1f%%)\n", vert_same_count, 100.0 * vert_same_count / (count - 1));
}

void analyze_successful_recoveries(QuadrantInfo* infos, int count) {
    printf("Analyzing the 2 successful recoveries (k=5 and k=199)...\n\n");
    
    // Find k=5 and k=199 (the 2 successful recoveries from deep analysis)
    QuadrantInfo* success1 = NULL;
    QuadrantInfo* success2 = NULL;
    
    for (int i = 0; i < count; i++) {
        if (infos[i].k == 5) success1 = &infos[i];
        if (infos[i].k == 199) success2 = &infos[i];
    }
    
    if (success1) {
        printf("Successful Recovery #1: k=5\n");
        printf("  Ring: %d\n", success1->position.ring);
        printf("  Position: %d\n", success1->position.position);
        printf("  Quadrant: Q%d\n", success1->quadrant + 1);
        printf("  Polarity: %s\n", success1->polarity == POLARITY_POSITIVE ? "POSITIVE" : "NEGATIVE");
        printf("  Vertical: %s\n", success1->vertical == VERTICAL_TOP ? "TOP" : "BOTTOM");
        printf("  Angle: %.4f rad (%.2f°)\n", success1->angle_normalized, success1->angle_normalized * 180.0 / M_PI);
        printf("  Position mod 3: %d\n", success1->position_mod_3);
        printf("  Position mod 4: %d\n", success1->position_mod_4);
        printf("\n");
    }
    
    if (success2) {
        printf("Successful Recovery #2: k=199\n");
        printf("  Ring: %d\n", success2->position.ring);
        printf("  Position: %d\n", success2->position.position);
        printf("  Quadrant: Q%d\n", success2->quadrant + 1);
        printf("  Polarity: %s\n", success2->polarity == POLARITY_POSITIVE ? "POSITIVE" : "NEGATIVE");
        printf("  Vertical: %s\n", success2->vertical == VERTICAL_TOP ? "TOP" : "BOTTOM");
        printf("  Angle: %.4f rad (%.2f°)\n", success2->angle_normalized, success2->angle_normalized * 180.0 / M_PI);
        printf("  Position mod 3: %d\n", success2->position_mod_3);
        printf("  Position mod 4: %d\n", success2->position_mod_4);
        printf("\n");
    }
    
    if (success1 && success2) {
        printf("Comparing the 2 successful recoveries:\n");
        printf("  Same ring: %s\n", success1->position.ring == success2->position.ring ? "YES" : "NO");
        printf("  Same quadrant: %s\n", success1->quadrant == success2->quadrant ? "YES" : "NO");
        printf("  Same polarity: %s\n", success1->polarity == success2->polarity ? "YES" : "NO");
        printf("  Same vertical: %s\n", success1->vertical == success2->vertical ? "YES" : "NO");
        printf("  Same mod 3: %s\n", success1->position_mod_3 == success2->position_mod_3 ? "YES" : "NO");
        printf("  Same mod 4: %s\n", success1->position_mod_4 == success2->position_mod_4 ? "YES" : "NO");
        
        // Check if they're in mirrored positions
        double angle_diff = fabs(success1->angle_normalized - success2->angle_normalized);
        if (angle_diff > M_PI) angle_diff = 2.0 * M_PI - angle_diff;
        
        printf("  Angle difference: %.2f° (%.4f rad)\n", 
               angle_diff * 180.0 / M_PI, angle_diff);
        
        if (fabs(angle_diff - M_PI) < 0.5) {
            printf("  → They are DIAGONAL MIRRORS! (180° apart)\n");
        }
    }
}