#include <stdio.h>
#include <math.h>
#include "math/abacus.h"
#include "math/abacus_geometric.h"
#include "math/clock.h"

int main() {
    // Test: 2 + 3 = 5
    CrystallineAbacus* a = abacus_from_double(2.0, 10, 5);
    CrystallineAbacus* b = abacus_from_double(3.0, 10, 5);
    CrystallineAbacus* result = abacus_new(10);
    
    printf("Testing 2 + 3 with geometric addition:\n");
    
    // Convert to clock positions
    ClockPosition pos_a, pos_b;
    abacus_to_clock_position(a, &pos_a);
    abacus_to_clock_position(b, &pos_b);
    
    printf("Input a (2.0):\n");
    printf("  angle: %f, ring: %u, position: %u, radius: %f\n",
           pos_a.angle, pos_a.ring, pos_a.position, pos_a.radius);
    printf("  quadrant: %d, polarity: %d\n", pos_a.quadrant, pos_a.polarity);
    
    printf("Input b (3.0):\n");
    printf("  angle: %f, ring: %u, position: %u, radius: %f\n",
           pos_b.angle, pos_b.ring, pos_b.position, pos_b.radius);
    printf("  quadrant: %d, polarity: %d\n", pos_b.quadrant, pos_b.polarity);
    
    // Perform geometric addition
    MathError err = abacus_add_geometric(a, b, result);
    printf("\nGeometric addition result: %s\n", 
           err == MATH_SUCCESS ? "SUCCESS" : "FAILED");
    
    // Convert result back
    double result_val;
    abacus_to_double(result, &result_val);
    printf("Result value: %f (expected 5.0)\n", result_val);
    
    // Test multiplication: 2 * 3 = 6
    CrystallineAbacus* result_mul = abacus_new(10);
    err = abacus_mul_geometric(a, b, result_mul);
    
    double result_mul_val;
    abacus_to_double(result_mul, &result_mul_val);
    printf("\nMultiplication result: %f (expected 6.0)\n", result_mul_val);
    
    abacus_free(a);
    abacus_free(b);
    abacus_free(result);
    abacus_free(result_mul);
    
    return 0;
}
