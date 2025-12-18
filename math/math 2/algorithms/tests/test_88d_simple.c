/*
 * Simple 88D Constants Test
 */

#include <stdio.h>
#include <stdlib.h>
#include "hierarchical_threading.h"

int main() {
    printf("\n88D Constants Test\n");
    printf("==================\n\n");
    
    printf("HIERARCHICAL_88D_NUM_LAYERS = %d\n", HIERARCHICAL_88D_NUM_LAYERS);
    printf("HIERARCHICAL_88D_DIMS_PER_LAYER = %d\n", HIERARCHICAL_88D_DIMS_PER_LAYER);
    printf("HIERARCHICAL_88D_TOTAL_DIMENSIONS = %d\n", HIERARCHICAL_88D_TOTAL_DIMENSIONS);
    printf("HIERARCHICAL_88D_THREADS_PER_LAYER = %d\n", HIERARCHICAL_88D_THREADS_PER_LAYER);
    printf("HIERARCHICAL_88D_TOTAL_THREADS = %d\n", HIERARCHICAL_88D_TOTAL_THREADS);
    printf("HIERARCHICAL_88D_CLOCK_POSITIONS = %d\n", HIERARCHICAL_88D_CLOCK_POSITIONS);
    
    printf("\nTest: hierarchical_thread_get_88d with NULL pool\n");
    HierarchicalThread* thread = hierarchical_thread_get_88d(NULL, 0, 0);
    if (thread == NULL) {
        printf("✓ Returns NULL for NULL pool\n");
    } else {
        printf("✗ Should return NULL for NULL pool\n");
        return 1;
    }
    
    printf("\n✓ All basic tests passed!\n\n");
    return 0;
}