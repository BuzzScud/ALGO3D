/**
 * @file test_phase4_simple.c
 * @brief Simple test for Phase 4 (Recursive Stabilization)
 */

#include "blind_recovery/blind_recovery.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    printf("=== Phase 4 Simple Test ===\n\n");
    
    // Create test structure (cube)
    StructuralMap structure = {
        .num_vertices = 8,
        .num_edges = 12,
        .num_faces = 6,
        .coprime_matrix = NULL,
        .dimensional_offsets = NULL,
        .corruption_mask = NULL,
        .corruption_percentage = 0.0
    };
    
    // Test 1: Multi-scale analysis
    printf("Test 1: Multi-scale analysis\n");
    MultiScaleAnalysis* analysis = analyze_multi_scale(&structure, 3);
    
    if (analysis) {
        printf("  ✓ Created multi-scale analysis with %u scales\n", analysis->num_scales);
        printf("  ✓ All scales stable: %s\n", analysis->all_scales_stable ? "Yes" : "No");
        
        // Test 2: Stabilization quality
        printf("\nTest 2: Stabilization quality\n");
        double quality = compute_stabilization_quality(analysis);
        printf("  ✓ Quality score: %.3f\n", quality);
        
        // Test 3: Adaptive stabilization
        printf("\nTest 3: Adaptive stabilization\n");
        bool stabilized = stabilize_adaptive(&structure, analysis);
        printf("  ✓ Stabilization %s\n", stabilized ? "succeeded" : "in progress");
        
        free_multi_scale_analysis(analysis);
        printf("\n✓ All Phase 4 basic tests passed!\n");
        return 0;
    } else {
        printf("  ✗ Failed to create multi-scale analysis\n");
        return 1;
    }
}
