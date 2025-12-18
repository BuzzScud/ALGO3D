/**
 * test_k_angle_distribution.c - Analyze K→Angle Distribution
 * 
 * This test analyzes how k values map to angles using the π·φ metric.
 * Goal: Understand the distribution to place anchors optimally.
 */

#include "clock_recovery.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define PI M_PI
#define PHI ((1.0 + sqrt(5.0)) / 2.0)

int main() {
    printf("\n=== K→Angle Distribution Analysis ===\n\n");
    
    // Analyze k = 0 to 300
    printf("Analyzing k values from 0 to 300...\n\n");
    
    printf("%-6s %-12s %-12s %-12s\n", "k", "Angle(rad)", "Angle(deg)", "Quadrant");
    printf("------------------------------------------------------------\n");
    
    // Count k values per quadrant
    int q1_count = 0, q2_count = 0, q3_count = 0, q4_count = 0;
    
    // Sample every 10th k value for display
    for (uint64_t k = 0; k <= 300; k += 10) {
        ClockPosition pos = map_k_to_clock_u64(k);
        double angle_deg = pos.angle * 180.0 / PI;
        
        // Determine quadrant
        const char* quadrant;
        if (angle_deg < 90.0) {
            quadrant = "Q1";
            q1_count++;
        } else if (angle_deg < 180.0) {
            quadrant = "Q2";
            q2_count++;
        } else if (angle_deg < 270.0) {
            quadrant = "Q3";
            q3_count++;
        } else {
            quadrant = "Q4";
            q4_count++;
        }
        
        printf("%-6lu %-12.6f %-12.2f %-12s\n", k, pos.angle, angle_deg, quadrant);
    }
    
    printf("\n=== Distribution Analysis ===\n\n");
    
    // Analyze density in different regions
    printf("Analyzing k density in 30° bins...\n\n");
    
    int bins[12] = {0}; // 12 bins of 30° each
    
    for (uint64_t k = 0; k <= 300; k++) {
        ClockPosition pos = map_k_to_clock_u64(k);
        double angle_deg = pos.angle * 180.0 / PI;
        
        // Determine bin (0-11)
        int bin = (int)(angle_deg / 30.0);
        if (bin >= 12) bin = 11;
        
        bins[bin]++;
    }
    
    printf("%-12s %-12s %-12s\n", "Angle Range", "K Count", "Density");
    printf("--------------------------------------------\n");
    
    for (int i = 0; i < 12; i++) {
        printf("%-12s %-12d %-12.2f%%\n",
               i == 0 ? "0-30°" :
               i == 1 ? "30-60°" :
               i == 2 ? "60-90°" :
               i == 3 ? "90-120°" :
               i == 4 ? "120-150°" :
               i == 5 ? "150-180°" :
               i == 6 ? "180-210°" :
               i == 7 ? "210-240°" :
               i == 8 ? "240-270°" :
               i == 9 ? "270-300°" :
               i == 10 ? "300-330°" : "330-360°",
               bins[i],
               100.0 * bins[i] / 301);
    }
    
    // Find optimal anchor positions
    printf("\n=== Optimal Anchor Positions ===\n\n");
    printf("Based on k density, optimal 12 anchor positions:\n\n");
    
    // For each 30° bin, find the median k value
    printf("%-12s %-12s %-12s\n", "Bin", "Median K", "Angle(deg)");
    printf("--------------------------------------------\n");
    
    for (int bin = 0; bin < 12; bin++) {
        // Find all k values in this bin
        uint64_t k_values[301];
        int count = 0;
        
        for (uint64_t k = 0; k <= 300; k++) {
            ClockPosition pos = map_k_to_clock_u64(k);
            double angle_deg = pos.angle * 180.0 / PI;
            int k_bin = (int)(angle_deg / 30.0);
            if (k_bin >= 12) k_bin = 11;
            
            if (k_bin == bin) {
                k_values[count++] = k;
            }
        }
        
        if (count > 0) {
            // Find median
            uint64_t median_k = k_values[count / 2];
            ClockPosition median_pos = map_k_to_clock_u64(median_k);
            double median_angle_deg = median_pos.angle * 180.0 / PI;
            
            printf("%-12d %-12lu %-12.2f\n", bin, median_k, median_angle_deg);
        }
    }
    
    // Analyze wraparound
    printf("\n=== Wraparound Analysis ===\n\n");
    printf("Checking how many times k wraps around 360°...\n\n");
    
    int wraps = 0;
    double prev_angle = 0.0;
    
    for (uint64_t k = 0; k <= 300; k++) {
        ClockPosition pos = map_k_to_clock_u64(k);
        double angle_deg = pos.angle * 180.0 / PI;
        
        if (angle_deg < prev_angle) {
            wraps++;
            printf("Wrap %d at k=%lu: %.2f° → %.2f°\n", wraps, k, prev_angle, angle_deg);
        }
        
        prev_angle = angle_deg;
    }
    
    printf("\nTotal wraps: %d\n", wraps);
    printf("This means k values cycle through 360° approximately %d times in range [0,300]\n", wraps + 1);
    
    return 0;
}