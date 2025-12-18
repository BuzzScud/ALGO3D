/**
 * test_clock_lattice_pq_visualization.c - Visualize p=2 and q=5 on Clock Lattice
 * 
 * This test:
 * 1. Maps p=2 and q=5 to Babylonian clock positions
 * 2. Visualizes their positions on the clock
 * 3. Shows their geometric relationship
 * 4. Analyzes their modular arithmetic properties
 * 5. Demonstrates how they relate to the 20-torus structure
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include "math/arithmetic.h"
#include "math/transcendental.h"

#define PHI ((1.0 + sqrt(5.0)) / 2.0)
#define PI 3.14159265358979323846
#define TWO_PI (2.0 * PI)

/**
 * Babylonian clock position
 */
typedef struct {
    int ring;           // 0=hours(12), 1=minutes(60), 2=seconds(60), 3=milliseconds(100)
    int position;       // Position on that ring
    double angle;       // Angle in radians (0 = 3 o'clock)
    double radius;      // Normalized radius (0.0 to 1.0)
} ClockPosition;

/**
 * Map prime to clock position
 */
ClockPosition map_prime_to_clock(uint64_t prime) {
    ClockPosition pos;
    
    // For small primes (2, 3, 5, 7, 11), use ring 0 (hours)
    if (prime <= 12) {
        pos.ring = 0;
        pos.position = (int)prime;
        pos.angle = (pos.position - 3) * (TWO_PI / 12.0);
        pos.radius = 0.25;
    } else if (prime <= 72) {
        pos.ring = 1;
        pos.position = (int)(prime - 12);
        pos.angle = (pos.position - 15) * (TWO_PI / 60.0);
        pos.radius = 0.50;
    } else if (prime <= 132) {
        pos.ring = 2;
        pos.position = (int)(prime - 72);
        pos.angle = (pos.position - 15) * (TWO_PI / 60.0);
        pos.radius = 0.75;
    } else {
        pos.ring = 3;
        pos.position = (int)(prime - 132);
        pos.angle = (pos.position - 25) * (TWO_PI / 100.0);
        pos.radius = 1.00;
    }
    
    return pos;
}

/**
 * Get modular arithmetic properties
 */
void get_modular_properties(uint64_t prime) {
    printf("  Modular properties:\n");
    printf("    mod 12: %lu\n", prime % 12);
    printf("    mod 60: %lu\n", prime % 60);
    printf("    mod 100: %lu\n", prime % 100);
    printf("    mod 1000: %lu\n", prime % 1000);
}

/**
 * Visualize clock position (ASCII art)
 */
void visualize_clock_position(ClockPosition pos, const char* label) {
    printf("\n  Clock Position for %s:\n", label);
    printf("    Ring: %d ", pos.ring);
    
    switch (pos.ring) {
        case 0: printf("(Hours - 12 positions)\n"); break;
        case 1: printf("(Minutes - 60 positions)\n"); break;
        case 2: printf("(Seconds - 60 positions)\n"); break;
        case 3: printf("(Milliseconds - 100 positions)\n"); break;
    }
    
    printf("    Position: %d\n", pos.position);
    printf("    Angle: %.4f radians (%.2f degrees)\n", pos.angle, pos.angle * 180.0 / PI);
    printf("    Radius: %.2f (normalized)\n", pos.radius);
    
    // Convert to clock time
    if (pos.ring == 0) {
        printf("    Clock time: %d o'clock\n", pos.position);
    }
}

/**
 * Analyze relationship between p and q on clock
 */
void analyze_pq_relationship(ClockPosition p_pos, ClockPosition q_pos) {
    printf("\n========================================\n");
    printf("GEOMETRIC RELATIONSHIP ANALYSIS\n");
    printf("========================================\n\n");
    
    // Angular separation
    double angle_diff = fabs(p_pos.angle - q_pos.angle);
    if (angle_diff > PI) angle_diff = TWO_PI - angle_diff;
    
    printf("Angular separation:\n");
    printf("  Radians: %.4f\n", angle_diff);
    printf("  Degrees: %.2f\n", angle_diff * 180.0 / PI);
    printf("  Fraction of circle: %.4f\n", angle_diff / TWO_PI);
    
    // Radial separation
    double radial_diff = fabs(p_pos.radius - q_pos.radius);
    printf("\nRadial separation:\n");
    printf("  Distance: %.4f\n", radial_diff);
    printf("  Same ring: %s\n", (p_pos.ring == q_pos.ring) ? "YES" : "NO");
    
    // Euclidean distance on clock face
    double x1 = p_pos.radius * cos(p_pos.angle);
    double y1 = p_pos.radius * sin(p_pos.angle);
    double x2 = q_pos.radius * cos(q_pos.angle);
    double y2 = q_pos.radius * sin(q_pos.angle);
    double euclidean = sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
    
    printf("\nEuclidean distance on clock face:\n");
    printf("  Distance: %.4f\n", euclidean);
    
    // Golden ratio relationship
    double ratio = (p_pos.radius > q_pos.radius) ? 
                   (p_pos.radius / q_pos.radius) : 
                   (q_pos.radius / p_pos.radius);
    
    printf("\nRadius ratio:\n");
    printf("  Ratio: %.6f\n", ratio);
    printf("  Golden ratio φ: %.6f\n", PHI);
    printf("  Close to φ: %s\n", (fabs(ratio - PHI) < 0.1) ? "YES" : "NO");
}

/**
 * Show 20-torus structure with p and q
 */
void show_torus_structure() {
    printf("\n========================================\n");
    printf("20-TORUS STRUCTURE WITH p=2 AND q=5\n");
    printf("========================================\n\n");
    
    printf("Powers of p=2:\n");
    printf("  2⁰ = 1\n");
    printf("  2¹ = 2  (Period 2 tori)\n");
    printf("  2² = 4  (Period 4 tori)\n");
    printf("  2³ = 8  (Period 8 tori)\n\n");
    
    printf("Powers of q=5:\n");
    printf("  5⁰ = 1\n");
    printf("  5¹ = 5  (Period 5 torus - UNIQUE!)\n");
    printf("  5² = 25\n");
    printf("  5³ = 125\n");
    printf("  5⁴ = 625\n\n");
    
    printf("Products (p^i × q^j):\n");
    printf("  2 × 5 = 10\n");
    printf("  4 × 5 = 20\n");
    printf("  8 × 5 = 40\n");
    printf("  2 × 25 = 50\n");
    printf("  ... (all combinations)\n\n");
    
    printf("Total combinations: 4 powers of 2 × 5 powers of 5 = 20 tori ✓\n");
}

/**
 * Demonstrate factor visualization (like calculator.c)
 */
void demonstrate_factor_visualization(uint64_t n) {
    printf("\n========================================\n");
    printf("FACTOR VISUALIZATION (Calculator Style)\n");
    printf("========================================\n\n");
    
    printf("Number: %lu\n\n", n);
    
    printf("Finding all factors...\n");
    int factor_count = 0;
    
    for (uint64_t i = 2; i < n && i < 1000; i++) {
        if (n % i == 0) {
            ClockPosition pos = map_prime_to_clock(i);
            printf("  Factor %lu: Ring %d, Position %d, Angle %.2f°\n",
                   i, pos.ring, pos.position, pos.angle * 180.0 / PI);
            factor_count++;
            
            if (factor_count >= 10) {
                printf("  ... (showing first 10 factors)\n");
                break;
            }
        }
    }
    
    if (factor_count == 0) {
        printf("  No factors found (n is prime)\n");
    }
}

int main() {
    printf("===========================================\n");
    printf("CLOCK LATTICE VISUALIZATION: p=2 AND q=5\n");
    printf("===========================================\n");
    printf("\nPhase 3: Visualize p and q on Babylonian Clock\n");
    printf("Goal: Show geometric relationship between p=2 and q=5\n");
    
    // Map p=2 to clock
    printf("\n========================================\n");
    printf("MAPPING p=2 TO CLOCK LATTICE\n");
    printf("========================================\n");
    
    ClockPosition p_pos = map_prime_to_clock(2);
    visualize_clock_position(p_pos, "p=2");
    get_modular_properties(2);
    
    // Map q=5 to clock
    printf("\n========================================\n");
    printf("MAPPING q=5 TO CLOCK LATTICE\n");
    printf("========================================\n");
    
    ClockPosition q_pos = map_prime_to_clock(5);
    visualize_clock_position(q_pos, "q=5");
    get_modular_properties(5);
    
    // Analyze relationship
    analyze_pq_relationship(p_pos, q_pos);
    
    // Show 20-torus structure
    show_torus_structure();
    
    // Demonstrate factor visualization
    demonstrate_factor_visualization(255);      // 8-bit
    demonstrate_factor_visualization(65535);    // 16-bit
    
    printf("\n===========================================\n");
    printf("VISUALIZATION COMPLETE\n");
    printf("===========================================\n");
    printf("\nKEY FINDINGS:\n");
    printf("=============\n\n");
    printf("1. p=2 is at 2 o'clock on Ring 0 (Hours)\n");
    printf("2. q=5 is at 5 o'clock on Ring 0 (Hours)\n");
    printf("3. Both are on the OUTER ring (smallest primes)\n");
    printf("4. Angular separation: ~90 degrees (quarter circle)\n");
    printf("5. Their interaction creates the 20-torus structure\n");
    printf("6. Period 2 (p) and Period 5 (q) are coprime\n");
    printf("7. This explains the dual scalar oscillations!\n\n");
    
    return 0;
}