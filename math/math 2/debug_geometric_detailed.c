#include <stdio.h>
#include <math.h>
#include "math/abacus.h"
#include "math/abacus_geometric.h"
#include "math/clock.h"
#include "math/clock_triangle.h"

int main() {
    // Test: 2 + 3 = 5
    CrystallineAbacus* a = abacus_from_double(2.0, 10, 5);
    CrystallineAbacus* b = abacus_from_double(3.0, 10, 5);
    
    printf("=== Detailed Trace of 2 + 3 ===\n\n");
    
    // Initialize clock triangle
    ClockTriangle3D triangle;
    clock_triangle_init(&triangle, 1.0, 1.0);
    
    // Step 1: MAP to clock positions
    ClockPosition pos_a, pos_b;
    abacus_to_clock_position(a, &pos_a);
    abacus_to_clock_position(b, &pos_b);
    
    printf("Step 1: MAP to clock positions\n");
    printf("  pos_a: position=%u, angle=%f, ring=%u\n", 
           pos_a.position, pos_a.angle, pos_a.ring);
    printf("  pos_b: position=%u, angle=%f, ring=%u\n\n", 
           pos_b.position, pos_b.angle, pos_b.ring);
    
    // Convert to 3D points
    ClockPoint3D point_a, point_b;
    clock_position_to_3d_point(&pos_a, &triangle, &point_a);
    clock_position_to_3d_point(&pos_b, &triangle, &point_b);
    
    printf("Step 2: Convert to 3D points\n");
    printf("  point_a: (%.3f, %.3f, %.3f)\n", 
           point_a.coords[0], point_a.coords[1], point_a.coords[2]);
    printf("  point_b: (%.3f, %.3f, %.3f)\n\n", 
           point_b.coords[0], point_b.coords[1], point_b.coords[2]);
    
    // Step 3: FOLD to Q1
    ClockPoint3D folded_a, folded_b;
    clock_triangle_fold_to_q1(&triangle, &point_a, &folded_a);
    clock_triangle_fold_to_q1(&triangle, &point_b, &folded_b);
    
    printf("Step 3: FOLD to Q1\n");
    printf("  folded_a: (%.3f, %.3f, %.3f)\n", 
           folded_a.coords[0], folded_a.coords[1], folded_a.coords[2]);
    printf("  folded_b: (%.3f, %.3f, %.3f)\n\n", 
           folded_b.coords[0], folded_b.coords[1], folded_b.coords[2]);
    
    // Step 4: OPERATE - Vector addition
    ClockPoint3D result_point;
    result_point.coords[0] = folded_a.coords[0] + folded_b.coords[0];
    result_point.coords[1] = folded_a.coords[1] + folded_b.coords[1];
    result_point.coords[2] = folded_a.coords[2] + folded_b.coords[2];
    result_point.polarity = point_a.polarity * point_b.polarity;
    result_point.quadrant = clock_triangle_get_quadrant(&result_point);
    
    printf("Step 4: OPERATE (vector addition)\n");
    printf("  result_point: (%.3f, %.3f, %.3f)\n", 
           result_point.coords[0], result_point.coords[1], result_point.coords[2]);
    printf("  quadrant: %d, polarity: %d\n\n", 
           result_point.quadrant, result_point.polarity);
    
    // Step 5: MAP BACK to clock position
    ClockPosition result_pos;
    point_3d_to_clock_position(&result_point, &triangle, &result_pos);
    
    printf("Step 5: MAP BACK to clock position\n");
    printf("  result_pos: position=%u, angle=%f, ring=%u\n\n", 
           result_pos.position, result_pos.angle, result_pos.ring);
    
    // Step 6: Convert to abacus
    CrystallineAbacus* result = abacus_new(10);
    clock_position_to_abacus(&result_pos, result);
    
    double result_val;
    abacus_to_double(result, &result_val);
    
    printf("Step 6: Convert to abacus\n");
    printf("  Final result: %.3f (expected 5.0)\n", result_val);
    
    abacus_free(a);
    abacus_free(b);
    abacus_free(result);
    clock_triangle_free(&triangle);
    
    return 0;
}
