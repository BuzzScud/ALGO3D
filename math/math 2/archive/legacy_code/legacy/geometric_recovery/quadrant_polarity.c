/**
 * @file quadrant_polarity.c
 * @brief Quadrant polarity system with mirrored sudoku folding
 * 
 * MIGRATED: Uses NEW math library
 * - Replaced math_cos with math_cos (2 calls)
 * - Replaced math_sin with math_sin (2 calls)
 * Total: 4 function calls migrated to NEW math library
 */

#include "geometric_recovery.h"
#include "../../../math/include/math/types.h"            // NEW math library constants
#include "../../../math/include/math/transcendental.h"  // NEW math library
#include <string.h>

/**
 * Create quadrant polarity system
 * 
 * Each quadrant has polarity and mirror relationships:
 * Q1: (+,+) polarity +1, mirrors Q2/Q3/Q4
 * Q2: (-,+) polarity -1, mirrors Q1/Q3/Q4
 * Q3: (-,-) polarity +1, mirrors Q1/Q2/Q4
 * Q4: (+,-) polarity -1, mirrors Q1/Q2/Q3
 */
QuadrantPolarity create_quadrant_polarity(int quadrant) {
    QuadrantPolarity pol;
    memset(&pol, 0, sizeof(QuadrantPolarity));
    
    pol.quadrant = quadrant;
    
    switch (quadrant) {
        case 1:  // (+, +)
            pol.polarity = +1;
            pol.x_flip = false;
            pol.y_flip = false;
            pol.mirror_quadrants[0] = 2;
            pol.mirror_quadrants[1] = 3;
            pol.mirror_quadrants[2] = 4;
            pol.fold_angle = 0.0;
            break;
            
        case 2:  // (-, +)
            pol.polarity = -1;
            pol.x_flip = true;
            pol.y_flip = false;
            pol.mirror_quadrants[0] = 1;
            pol.mirror_quadrants[1] = 3;
            pol.mirror_quadrants[2] = 4;
            pol.fold_angle = MATH_PI / 2.0;
            break;
            
        case 3:  // (-, -)
            pol.polarity = +1;
            pol.x_flip = true;
            pol.y_flip = true;
            pol.mirror_quadrants[0] = 1;
            pol.mirror_quadrants[1] = 2;
            pol.mirror_quadrants[2] = 4;
            pol.fold_angle = MATH_PI;
            break;
            
        case 4:  // (+, -)
            pol.polarity = -1;
            pol.x_flip = false;
            pol.y_flip = true;
            pol.mirror_quadrants[0] = 1;
            pol.mirror_quadrants[1] = 2;
            pol.mirror_quadrants[2] = 3;
            pol.fold_angle = 3.0 * MATH_PI / 2.0;
            break;
            
        default:
            pol.quadrant = 1;
            pol.polarity = +1;
            break;
    }
    
    return pol;
}

/**
 * Apply quadrant polarity correction to position
 * 
 * Adjusts position based on quadrant polarity and mirroring.
 */
void apply_quadrant_polarity(
    double* position,
    const QuadrantPolarity* polarity,
    uint32_t num_dimensions
) {
    // Apply polarity scaling
    for (uint32_t d = 0; d < num_dimensions; d++) {
        position[d] *= polarity->polarity;
    }
    
    // Apply axis flips
    if (polarity->x_flip && num_dimensions > 0) {
        position[0] = -position[0];
    }
    if (polarity->y_flip && num_dimensions > 1) {
        position[1] = -position[1];
    }
    
    // Apply Möbius fold angle correction
    // This handles the 2π folding with polarity flip
    if (polarity->fold_angle > 1e-6) {
        double cos_fold = math_cos(polarity->fold_angle);
        double sin_fold = math_sin(polarity->fold_angle);
        
        if (num_dimensions >= 2) {
            double x = position[0];
            double y = position[1];
            position[0] = x * cos_fold - y * sin_fold;
            position[1] = x * sin_fold + y * cos_fold;
        }
    }
}

/**
 * Fold position using mirrored sudoku (Q2/Q3/Q4 → Q1)
 * 
 * This implements the key insight: "Each quadrant is a mirrored sudoku
 * of its adjacent quadrants". All operations can be folded into Q1.
 */
void fold_to_first_quadrant(
    double* position,
    QuadrantPolarity* polarity_out,
    uint32_t num_dimensions
) {
    // Determine current quadrant
    int quadrant = determine_quadrant(position, num_dimensions);
    
    // Create polarity for current quadrant
    *polarity_out = create_quadrant_polarity(quadrant);
    
    // If already in Q1, nothing to do
    if (quadrant == 1) {
        return;
    }
    
    // Fold to Q1 by reversing the polarity transformations
    // This is the inverse of apply_quadrant_polarity
    
    // Reverse Möbius fold
    if (polarity_out->fold_angle > 1e-6) {
        double cos_fold = math_cos(-polarity_out->fold_angle);
        double sin_fold = math_sin(-polarity_out->fold_angle);
        
        if (num_dimensions >= 2) {
            double x = position[0];
            double y = position[1];
            position[0] = x * cos_fold - y * sin_fold;
            position[1] = x * sin_fold + y * cos_fold;
        }
    }
    
    // Reverse axis flips
    if (polarity_out->x_flip && num_dimensions > 0) {
        position[0] = -position[0];
    }
    if (polarity_out->y_flip && num_dimensions > 1) {
        position[1] = -position[1];
    }
    
    // Reverse polarity scaling
    for (uint32_t d = 0; d < num_dimensions; d++) {
        position[d] *= polarity_out->polarity;
    }
    
    // Now position is in Q1
    // Update polarity to reflect Q1
    *polarity_out = create_quadrant_polarity(1);
}
