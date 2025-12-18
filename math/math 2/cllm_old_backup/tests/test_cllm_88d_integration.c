/**
 * @file test_cllm_88d_integration.c
 * @brief Test suite for CLLM 88D Threading Integration
 */

#include "ai/cllm_88d_integration.h"
#include "../include/cllm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
// HELPER FUNCTIONS
// ============================================================================

/**
 * Create a minimal CLLM model for testing
 */
CLLMModel* create_test_model(PlatonicSolidType solid_type) {
    CLLMModel* model = calloc(1, sizeof(CLLMModel));
    if (!model) return NULL;
    
    model->solid_type = solid_type;
    
    // Set geometry based on solid type
    switch (solid_type) {
        case PLATONIC_CUBE:
            model->geometry.vertices = 8;
            model->geometry.edges = 12;
            model->geometry.faces = 6;
            model->embedding_dim = 96;   // 8 * 12
            model->hidden_dim = 144;     // 12 * 12
            model->num_layers = 6;
            break;
        case PLATONIC_TETRAHEDRON:
            model->geometry.vertices = 4;
            model->geometry.edges = 6;
            model->geometry.faces = 4;
            model->embedding_dim = 48;   // 4 * 12
            model->hidden_dim = 72;      // 6 * 12
            model->num_layers = 4;
            break;
        case PLATONIC_OCTAHEDRON:
            model->geometry.vertices = 6;
            model->geometry.edges = 12;
            model->geometry.faces = 8;
            model->embedding_dim = 72;   // 6 * 12
            model->hidden_dim = 144;     // 12 * 12
            model->num_layers = 8;
            break;
        case PLATONIC_DODECAHEDRON:
            model->geometry.vertices = 20;
            model->geometry.edges = 30;
            model->geometry.faces = 12;
            model->embedding_dim = 240;  // 20 * 12
            model->hidden_dim = 360;     // 30 * 12
            model->num_layers = 12;
            break;
        case PLATONIC_ICOSAHEDRON:
            model->geometry.vertices = 12;
            model->geometry.edges = 30;
            model->geometry.faces = 20;
            model->embedding_dim = 144;  // 12 * 12
            model->hidden_dim = 360;     // 30 * 12
            model->num_layers = 20;
            break;
    }
    
    model->num_heads = 12;
    model->vocab_size = 1000;
    model->max_seq_len = 128;
    
    return model;
}

/**
 * Destroy test model
 */
void destroy_test_model(CLLMModel* model) {
    if (!model) return;
    
    // Cleanup threading if initialized
    if (model->threading.enabled) {
        cllm_cleanup_88d_threading(model);
    }
    
    free(model);
}

// ============================================================================
// TEST CASES
// ============================================================================

/**
 * Test 1: Initialize and cleanup 88D threading
 */
bool test_initialize_cleanup() {
    CLLMModel* model = create_test_model(PLATONIC_CUBE);
    assert(model != NULL);
    
    // Initialize threading
    bool success = cllm_initialize_88d_threading(model);
    if (!success) {
        printf("Failed to initialize 88D threading\n");
        destroy_test_model(model);
        return false;
    }
    
    // Verify initialization
    assert(model->threading.enabled == true);
    assert(model->threading.pool_88d != NULL);
    assert(model->threading.work_queue != NULL);
    assert(model->threading.steal_pool != NULL);
    assert(model->threading.vertex_to_thread != NULL);
    assert(model->threading.edge_to_boundary != NULL);
    assert(model->threading.face_to_layer != NULL);
    assert(model->threading.token_to_thread != NULL);
    
    // Cleanup
    cllm_cleanup_88d_threading(model);
    
    // Verify cleanup
    assert(model->threading.enabled == false);
    assert(model->threading.pool_88d == NULL);
    assert(model->threading.work_queue == NULL);
    assert(model->threading.steal_pool == NULL);
    assert(model->threading.vertex_to_thread == NULL);
    assert(model->threading.edge_to_boundary == NULL);
    assert(model->threading.face_to_layer == NULL);
    assert(model->threading.token_to_thread == NULL);
    
    destroy_test_model(model);
    return true;
}

/**
 * Test 2: Geometry mapping
 */
bool test_geometry_mapping() {
    CLLMModel* model = create_test_model(PLATONIC_CUBE);
    assert(model != NULL);
    
    // Initialize threading
    bool success = cllm_initialize_88d_threading(model);
    if (!success) {
        destroy_test_model(model);
        return false;
    }
    
    // Test vertex mapping
    for (uint32_t i = 0; i < model->geometry.vertices; i++) {
        uint32_t thread_id = cllm_get_thread_for_vertex(model, i);
        assert(thread_id < 88);  // Should be in range [0, 87]
        printf("  Vertex %u -> Thread %u\n", i, thread_id);
    }
    
    // Test token mapping
    for (uint32_t i = 0; i < 10; i++) {  // Test first 10 tokens
        uint32_t thread_id = cllm_get_thread_for_token(model, i);
        assert(thread_id < 88);
        printf("  Token %u -> Thread %u\n", i, thread_id);
    }
    
    // Test face mapping
    for (uint32_t i = 0; i < model->geometry.faces; i++) {
        uint32_t layer_id = cllm_get_layer_for_face(model, i);
        assert(layer_id < 8);  // Should be in range [0, 7]
        printf("  Face %u -> Layer %u\n", i, layer_id);
    }
    
    destroy_test_model(model);
    return true;
}

/**
 * Test 3: Work distribution
 */
bool test_work_distribution() {
    CLLMModel* model = create_test_model(PLATONIC_CUBE);
    assert(model != NULL);
    
    // Initialize threading
    bool success = cllm_initialize_88d_threading(model);
    if (!success) {
        destroy_test_model(model);
        return false;
    }
    
    // Distribute work
    uint32_t num_items = 1000;
    success = cllm_distribute_work_88d(model, CLLM_WORK_EMBEDDING_LOOKUP, NULL, num_items);
    if (!success) {
        printf("Failed to distribute work\n");
        destroy_test_model(model);
        return false;
    }
    
    // Verify work was distributed
    assert(model->threading.total_work_units == num_items);
    printf("  Distributed %u work items\n", num_items);
    
    destroy_test_model(model);
    return true;
}

/**
 * Test 4: Synchronization
 */
bool test_synchronization() {
    CLLMModel* model = create_test_model(PLATONIC_CUBE);
    assert(model != NULL);
    
    // Initialize threading
    bool success = cllm_initialize_88d_threading(model);
    if (!success) {
        destroy_test_model(model);
        return false;
    }
    
    // Test global synchronization
    printf("  Testing global synchronization...\n");
    cllm_synchronize_threads(model);
    printf("  ✓ Global synchronization successful\n");
    
    // Test layer synchronization
    printf("  Testing layer synchronization...\n");
    for (uint32_t layer = 0; layer < 8; layer++) {
        cllm_synchronize_layer(model, layer);
        printf("  ✓ Layer %u synchronization successful\n", layer);
    }
    
    destroy_test_model(model);
    return true;
}

/**
 * Test 5: Statistics
 */
bool test_statistics() {
    CLLMModel* model = create_test_model(PLATONIC_CUBE);
    assert(model != NULL);
    
    // Initialize threading
    bool success = cllm_initialize_88d_threading(model);
    if (!success) {
        destroy_test_model(model);
        return false;
    }
    
    // Get statistics
    HierarchicalThreadPoolStats stats;
    cllm_get_threading_stats(model, &stats);
    
    printf("  Number of threads: %u\n", stats.num_threads);
    printf("  Number of levels: %u\n", stats.num_levels);
    
    // Print statistics
    cllm_print_threading_stats(model);
    
    // Reset statistics
    cllm_reset_threading_stats(model);
    assert(model->threading.total_work_units == 0);
    
    destroy_test_model(model);
    return true;
}

/**
 * Test 6: Multiple solid types
 */
bool test_multiple_solids() {
    PlatonicSolidType solids[] = {
        PLATONIC_TETRAHEDRON,
        PLATONIC_CUBE,
        PLATONIC_OCTAHEDRON,
        PLATONIC_DODECAHEDRON,
        PLATONIC_ICOSAHEDRON
    };
    
    const char* names[] = {
        "Tetrahedron",
        "Cube",
        "Octahedron",
        "Dodecahedron",
        "Icosahedron"
    };
    
    for (int i = 0; i < 5; i++) {
        printf("  Testing %s...\n", names[i]);
        
        CLLMModel* model = create_test_model(solids[i]);
        assert(model != NULL);
        
        bool success = cllm_initialize_88d_threading(model);
        if (!success) {
            printf("  Failed to initialize threading for %s\n", names[i]);
            destroy_test_model(model);
            return false;
        }
        
        printf("    Vertices: %u, Edges: %u, Faces: %u\n",
               model->geometry.vertices, model->geometry.edges, model->geometry.faces);
        
        destroy_test_model(model);
        printf("  ✓ %s passed\n", names[i]);
    }
    
    return true;
}

/**
 * Test 7: Message broadcasting
 */
bool test_message_broadcasting() {
    CLLMModel* model = create_test_model(PLATONIC_CUBE);
    assert(model != NULL);
    
    // Initialize threading
    bool success = cllm_initialize_88d_threading(model);
    if (!success) {
        destroy_test_model(model);
        return false;
    }
    
    // Broadcast a message
    uint32_t test_data = 42;
    printf("  Broadcasting message with data: %u\n", test_data);
    cllm_broadcast_message(model, 1, &test_data, sizeof(test_data));
    printf("  ✓ Message broadcast successful\n");
    
    destroy_test_model(model);
    return true;
}

/**
 * Test 8: Work item submission
 */
bool test_work_item_submission() {
    CLLMModel* model = create_test_model(PLATONIC_CUBE);
    assert(model != NULL);
    
    // Initialize threading
    bool success = cllm_initialize_88d_threading(model);
    if (!success) {
        destroy_test_model(model);
        return false;
    }
    
    // Submit individual work items
    for (int i = 0; i < 10; i++) {
        CLLMWorkItem item;
        item.type = CLLM_WORK_EMBEDDING_LOOKUP;
        item.data = NULL;
        item.start_idx = i * 10;
        item.end_idx = (i + 1) * 10;
        item.batch_idx = 0;
        item.layer_idx = 0;
        
        success = cllm_submit_work_item(model, &item);
        if (!success) {
            printf("  Failed to submit work item %d\n", i);
            destroy_test_model(model);
            return false;
        }
    }
    
    printf("  Submitted 10 work items\n");
    assert(model->threading.total_work_units == 10);
    
    destroy_test_model(model);
    return true;
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main(void) {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║         CLLM 88D Threading Integration Test Suite             ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    TEST(initialize_cleanup);
    TEST(geometry_mapping);
    TEST(work_distribution);
    TEST(synchronization);
    TEST(statistics);
    TEST(multiple_solids);
    TEST(message_broadcasting);
    TEST(work_item_submission);
    
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