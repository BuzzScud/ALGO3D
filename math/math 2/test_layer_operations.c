#include <stdio.h>
#include <stdlib.h>
#include "ai/cllm_lattice_hierarchy.h"
#include "ai/cllm_space88d_ops.h"

int main() {
    printf("========================================\n");
    printf("Testing Space88D Layer Operations\n");
    printf("========================================\n\n");
    
    // Create a sphere
    int symmetry_groups[] = {0};
    CLLMLatticeHierarchy* sphere = lattice_hierarchy_create(
        1, 0, symmetry_groups, 1, 0, NULL
    );
    
    if (!sphere) {
        fprintf(stderr, "ERROR: Failed to create sphere\n");
        return 1;
    }
    
    printf("✓ Sphere created\n\n");
    
    // Test 1: Layer selection based on magnitude
    printf("Test 1: Layer Selection\n");
    printf("------------------------\n");
    
    CrystallineAbacus* small_val = abacus_from_uint64(100, 60);
    CrystallineAbacus* medium_val = abacus_from_uint64(1000000, 60);
    CrystallineAbacus* large_val = abacus_from_uint64(1000000000, 60);
    
    int layer_small = cllm_space88d_select_layer(sphere, small_val);
    int layer_medium = cllm_space88d_select_layer(sphere, medium_val);
    int layer_large = cllm_space88d_select_layer(sphere, large_val);
    
    printf("  Value 100 → Layer %d (expected 0)\n", layer_small);
    printf("  Value 1M → Layer %d (expected 2)\n", layer_medium);
    printf("  Value 1B → Layer %d (expected 3)\n", layer_large);
    
    if (layer_small == 0 && layer_medium == 2 && layer_large == 3) {
        printf("✓ Layer selection working correctly\n\n");
    } else {
        fprintf(stderr, "ERROR: Layer selection incorrect\n");
        return 1;
    }
    
    // Test 2: Setting and getting values
    printf("Test 2: Set/Get Values\n");
    printf("----------------------\n");
    
    CrystallineAbacus* test_val = abacus_from_uint64(42, 60);
    if (cllm_space88d_set_value(sphere, 0, 5, test_val) != 0) {
        fprintf(stderr, "ERROR: Failed to set value\n");
        return 1;
    }
    
    const CrystallineAbacus* retrieved = cllm_space88d_get_value(sphere, 0, 5);
    if (!retrieved) {
        fprintf(stderr, "ERROR: Failed to get value\n");
        return 1;
    }
    
    uint64_t result;
    if (abacus_to_uint64(retrieved, &result) != MATH_SUCCESS || result != 42) {
        fprintf(stderr, "ERROR: Value mismatch\n");
        return 1;
    }
    
    printf("  Set value 42 at layer 0, dimension 5\n");
    printf("  Retrieved value: %lu\n", result);
    printf("✓ Set/Get operations working\n\n");
    
    // Test 3: Add to position
    printf("Test 3: Add to Position\n");
    printf("------------------------\n");
    
    CrystallineAbacus* add_val = abacus_from_uint64(10, 60);
    if (cllm_space88d_add_to_position(sphere, 0, 5, add_val) != 0) {
        fprintf(stderr, "ERROR: Failed to add to position\n");
        return 1;
    }
    
    retrieved = cllm_space88d_get_value(sphere, 0, 5);
    if (abacus_to_uint64(retrieved, &result) != MATH_SUCCESS || result != 52) {
        fprintf(stderr, "ERROR: Addition incorrect (expected 52, got %lu)\n", result);
        return 1;
    }
    
    printf("  Added 10 to position (42 + 10 = 52)\n");
    printf("  Result: %lu\n", result);
    printf("✓ Addition working\n\n");
    
    // Test 4: Multiply position
    printf("Test 4: Multiply Position\n");
    printf("-------------------------\n");
    
    CrystallineAbacus* mul_val = abacus_from_uint64(2, 60);
    if (cllm_space88d_multiply_position(sphere, 0, 5, mul_val) != 0) {
        fprintf(stderr, "ERROR: Failed to multiply position\n");
        return 1;
    }
    
    retrieved = cllm_space88d_get_value(sphere, 0, 5);
    if (abacus_to_uint64(retrieved, &result) != MATH_SUCCESS || result != 104) {
        fprintf(stderr, "ERROR: Multiplication incorrect (expected 104, got %lu)\n", result);
        return 1;
    }
    
    printf("  Multiplied position by 2 (52 × 2 = 104)\n");
    printf("  Result: %lu\n", result);
    printf("✓ Multiplication working\n\n");
    
    // Test 5: Copy layer
    printf("Test 5: Copy Layer\n");
    printf("------------------\n");
    
    // Set some values in layer 0
    for (uint8_t dim = 0; dim < 5; dim++) {
        CrystallineAbacus* val = abacus_from_uint64(dim * 10, 60);
        cllm_space88d_set_value(sphere, 0, dim, val);
        abacus_free(val);
    }
    
    // Copy layer 0 to layer 1
    if (cllm_space88d_copy_layer(sphere, 0, 1) != 0) {
        fprintf(stderr, "ERROR: Failed to copy layer\n");
        return 1;
    }
    
    // Verify copy
    bool copy_correct = true;
    for (uint8_t dim = 0; dim < 5; dim++) {
        const CrystallineAbacus* val0 = cllm_space88d_get_value(sphere, 0, dim);
        const CrystallineAbacus* val1 = cllm_space88d_get_value(sphere, 1, dim);
        
        if (val0 && val1) {
            if (abacus_compare(val0, val1) != 0) {
                copy_correct = false;
                break;
            }
        }
    }
    
    if (copy_correct) {
        printf("  Copied layer 0 to layer 1\n");
        printf("  All values match\n");
        printf("✓ Layer copy working\n\n");
    } else {
        fprintf(stderr, "ERROR: Layer copy values don't match\n");
        return 1;
    }
    
    // Test 6: Layer dot product
    printf("Test 6: Layer Dot Product\n");
    printf("-------------------------\n");
    
    CrystallineAbacus* dot_result = cllm_space88d_layer_dot_product(sphere, 0, 1);
    if (!dot_result) {
        fprintf(stderr, "ERROR: Failed to compute dot product\n");
        return 1;
    }
    
    uint64_t dot_val;
    if (abacus_to_uint64(dot_result, &dot_val) == MATH_SUCCESS) {
        printf("  Dot product of layer 0 and layer 1: %lu\n", dot_val);
        printf("✓ Dot product computed\n\n");
    }
    abacus_free(dot_result);
    
    // Test 7: Layer statistics
    printf("Test 7: Layer Statistics\n");
    printf("------------------------\n");
    
    CrystallineAbacus *min, *max, *sum;
    if (cllm_space88d_layer_stats(sphere, 0, &min, &max, &sum) == 0) {
        uint64_t min_val, max_val, sum_val;
        
        if (min && abacus_to_uint64(min, &min_val) == MATH_SUCCESS) {
            printf("  Min: %lu\n", min_val);
        }
        if (max && abacus_to_uint64(max, &max_val) == MATH_SUCCESS) {
            printf("  Max: %lu\n", max_val);
        }
        if (sum && abacus_to_uint64(sum, &sum_val) == MATH_SUCCESS) {
            printf("  Sum: %lu\n", sum_val);
        }
        
        if (min) abacus_free(min);
        if (max) abacus_free(max);
        if (sum) abacus_free(sum);
        
        printf("✓ Statistics computed\n\n");
    }
    
    // Test 8: Print layer
    printf("Test 8: Print Layer\n");
    printf("-------------------\n");
    cllm_space88d_print_layer(sphere, 0);
    printf("✓ Layer printed\n\n");
    
    // Cleanup
    abacus_free(small_val);
    abacus_free(medium_val);
    abacus_free(large_val);
    abacus_free(test_val);
    abacus_free(add_val);
    abacus_free(mul_val);
    lattice_hierarchy_free(sphere);
    
    printf("========================================\n");
    printf("✓ ALL LAYER OPERATION TESTS PASSED\n");
    printf("========================================\n");
    
    return 0;
}