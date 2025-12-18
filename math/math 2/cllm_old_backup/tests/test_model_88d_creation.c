/**
 * @file test_model_88d_creation.c
 * @brief Test CLLM Model Creation with 88D Threading Integration
 */

#include "cllm.h"
#include "ai/cllm_88d_integration.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// Test counter
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) \
    printf("\n=== TEST: %s ===\n", #name); \
    if (test_##name()) { \
        printf("✓ PASSED: %s\n", #name); \
        tests_passed++; \
    } else { \
        printf("✗ FAILED: %s\n", #name); \
        tests_failed++; \
    }

// ============================================================================
// TEST CASES
// ============================================================================

/**
 * Test 1: Create model with 88D threading (Cube)
 */
bool test_create_cube_with_88d() {
    printf("Creating CLLM model with Cube geometry and 88D threading...\n");
    
    CLLMConfig config = cllm_default_config(PLATONIC_CUBE, 1000);
    config.enable_kissing_spheres = true;  // Enable 88D threading
    
    CLLMModel* model = cllm_create_model(&config);
    if (!model) {
        printf("Failed to create model\n");
        return false;
    }
    
    // Verify model was created
    assert(model != NULL);
    assert(model->solid_type == PLATONIC_CUBE);
    assert(model->geometry.vertices == 8);
    assert(model->geometry.edges == 12);
    assert(model->geometry.faces == 6);
    
    // Verify 88D threading was initialized
    assert(model->threading.enabled == true);
    assert(model->threading.pool_88d != NULL);
    assert(model->threading.vertex_to_thread != NULL);
    assert(model->threading.edge_to_boundary != NULL);
    assert(model->threading.face_to_layer != NULL);
    assert(model->threading.token_to_thread != NULL);
    
    printf("  ✓ Model created successfully\n");
    printf("  ✓ 88D threading initialized\n");
    printf("  ✓ Geometry: 8V, 12E, 6F\n");
    
    // Get threading statistics
    HierarchicalThreadPoolStats stats;
    cllm_get_threading_stats(model, &stats);
    printf("  ✓ Thread pool: %u threads, %u levels\n", stats.num_threads, stats.num_levels);
    
    // Free model
    cllm_free_model(model);
    printf("  ✓ Model freed successfully\n");
    
    return true;
}

/**
 * Test 2: Create model with 88D threading (Tetrahedron)
 */
bool test_create_tetrahedron_with_88d() {
    printf("Creating CLLM model with Tetrahedron geometry and 88D threading...\n");
    
    CLLMConfig config = cllm_default_config(PLATONIC_TETRAHEDRON, 500);
    config.enable_kissing_spheres = true;
    
    CLLMModel* model = cllm_create_model(&config);
    if (!model) {
        printf("Failed to create model\n");
        return false;
    }
    
    assert(model->solid_type == PLATONIC_TETRAHEDRON);
    assert(model->geometry.vertices == 4);
    assert(model->geometry.edges == 6);
    assert(model->geometry.faces == 4);
    assert(model->threading.enabled == true);
    
    printf("  ✓ Tetrahedron model created\n");
    printf("  ✓ Geometry: 4V, 6E, 4F\n");
    
    cllm_free_model(model);
    return true;
}

/**
 * Test 3: Create model with 88D threading (Dodecahedron)
 */
bool test_create_dodecahedron_with_88d() {
    printf("Creating CLLM model with Dodecahedron geometry and 88D threading...\n");
    
    CLLMConfig config = cllm_default_config(PLATONIC_DODECAHEDRON, 2000);
    config.enable_kissing_spheres = true;
    
    CLLMModel* model = cllm_create_model(&config);
    if (!model) {
        printf("Failed to create model\n");
        return false;
    }
    
    assert(model->solid_type == PLATONIC_DODECAHEDRON);
    assert(model->geometry.vertices == 20);
    assert(model->geometry.edges == 30);
    assert(model->geometry.faces == 12);
    assert(model->threading.enabled == true);
    
    printf("  ✓ Dodecahedron model created\n");
    printf("  ✓ Geometry: 20V, 30E, 12F\n");
    
    cllm_free_model(model);
    return true;
}

/**
 * Test 4: Create model without 88D threading
 */
bool test_create_without_88d() {
    printf("Creating CLLM model without 88D threading...\n");
    
    CLLMConfig config = cllm_default_config(PLATONIC_CUBE, 1000);
    config.enable_kissing_spheres = false;  // Disable 88D threading
    
    CLLMModel* model = cllm_create_model(&config);
    if (!model) {
        printf("Failed to create model\n");
        return false;
    }
    
    assert(model->threading.enabled == false);
    assert(model->threading.pool_88d == NULL);
    
    printf("  ✓ Model created without threading\n");
    
    cllm_free_model(model);
    return true;
}

/**
 * Test 5: Verify geometry mapping
 */
bool test_geometry_mapping() {
    printf("Testing geometry mapping to threads...\n");
    
    CLLMConfig config = cllm_default_config(PLATONIC_CUBE, 100);
    config.enable_kissing_spheres = true;
    
    CLLMModel* model = cllm_create_model(&config);
    if (!model) return false;
    
    // Test vertex mapping
    for (uint32_t v = 0; v < model->geometry.vertices; v++) {
        uint32_t thread_id = cllm_get_thread_for_vertex(model, v);
        assert(thread_id < 88);  // Should be in worker thread range
        printf("  Vertex %u -> Thread %u\n", v, thread_id);
    }
    
    // Test token mapping
    for (uint32_t t = 0; t < 10; t++) {
        uint32_t thread_id = cllm_get_thread_for_token(model, t);
        assert(thread_id < 88);
        printf("  Token %u -> Thread %u\n", t, thread_id);
    }
    
    // Test face mapping
    for (uint32_t f = 0; f < model->geometry.faces; f++) {
        uint32_t layer_id = cllm_get_layer_for_face(model, f);
        assert(layer_id < 8);  // Should be in layer range
        printf("  Face %u -> Layer %u\n", f, layer_id);
    }
    
    printf("  ✓ All geometry mappings valid\n");
    
    cllm_free_model(model);
    return true;
}

/**
 * Test 6: Multiple model creation/destruction
 */
bool test_multiple_models() {
    printf("Testing multiple model creation/destruction cycles...\n");
    
    for (int i = 0; i < 3; i++) {
        printf("  Cycle %d...\n", i + 1);
        
        CLLMConfig config = cllm_default_config(PLATONIC_CUBE, 500);
        config.enable_kissing_spheres = true;
        
        CLLMModel* model = cllm_create_model(&config);
        if (!model) {
            printf("  Failed to create model in cycle %d\n", i + 1);
            return false;
        }
        
        assert(model->threading.enabled == true);
        
        cllm_free_model(model);
        printf("  ✓ Cycle %d complete\n", i + 1);
    }
    
    printf("  ✓ All cycles successful\n");
    return true;
}

/**
 * Test 7: Threading statistics
 */
bool test_threading_stats() {
    printf("Testing threading statistics...\n");
    
    CLLMConfig config = cllm_default_config(PLATONIC_CUBE, 1000);
    config.enable_kissing_spheres = true;
    
    CLLMModel* model = cllm_create_model(&config);
    if (!model) return false;
    
    // Get statistics
    HierarchicalThreadPoolStats stats;
    cllm_get_threading_stats(model, &stats);
    
    printf("  Number of threads: %u\n", stats.num_threads);
    printf("  Number of levels: %u\n", stats.num_levels);
    printf("  Total messages: %lu\n", stats.total_messages);
    printf("  Total work items: %lu\n", stats.total_work_items);
    
    assert(stats.num_threads == 96);
    assert(stats.num_levels == 8);
    
    // Print full statistics
    cllm_print_threading_stats(model);
    
    printf("  ✓ Statistics retrieved successfully\n");
    
    cllm_free_model(model);
    return true;
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main(void) {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║      CLLM Model Creation with 88D Threading Test Suite        ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    TEST(create_cube_with_88d);
    TEST(create_tetrahedron_with_88d);
    TEST(create_dodecahedron_with_88d);
    TEST(create_without_88d);
    TEST(geometry_mapping);
    TEST(multiple_models);
    TEST(threading_stats);
    
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║                        TEST SUMMARY                            ║\n");
    printf("╠════════════════════════════════════════════════════════════════╣\n");
    printf("║  Total Tests:  %3d                                             ║\n", tests_passed + tests_failed);
    printf("║  Passed:       %3d ✓                                           ║\n", tests_passed);
    printf("║  Failed:       %3d ✗                                           ║\n", tests_failed);
    printf("║  Success Rate: %3d%%                                            ║\n", 
           (tests_passed * 100) / (tests_passed + tests_failed));
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    return tests_failed == 0 ? 0 : 1;
}