#include "ai/cllm_control_process.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

// Test counters
static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) \
    do { \
        printf("Running test: %s...", name); \
        tests_run++; \
    } while(0)

#define PASS() \
    do { \
        printf(" PASSED\n"); \
        tests_passed++; \
    } while(0)

#define FAIL(msg) \
    do { \
        printf(" FAILED: %s\n", msg); \
        return; \
    } while(0)

// ============================================================================
// TEST: Control Process Creation
// ============================================================================

void test_control_process_create(void) {
    TEST("control_process_create");
    
    SystemConfiguration config = {
        .max_hierarchy_depth = 3,
        .max_spheres_per_level = 12,
        .initial_sphere_count = 4,
        .batch_size = 32,
        .max_epochs = 100,
        .learning_rate = 0.001,
        .max_threads = 8,
        .max_memory_bytes = 1024 * 1024 * 1024,
        .sync_interval_batches = 10,
        .checkpoint_interval_epochs = 5,
        .health_check_interval_ms = 1000,
        .sphere_timeout_seconds = 30.0,
        .enable_boundary_awareness = true,
        .enable_twin_prime_tracking = true
    };
    
    ControlProcess* cp = control_process_create(&config);
    if (!cp) FAIL("Failed to create control process");
    
    // Verify initial state
    if (cp->state != CONTROL_STATE_INITIALIZING) {
        control_process_free(cp);
        FAIL("Initial state should be INITIALIZING");
    }
    
    // Verify configuration copied
    if (cp->config.max_threads != 8) {
        control_process_free(cp);
        FAIL("Configuration not copied correctly");
    }
    
    // Verify no spheres initially
    if (cp->total_sphere_count != 0) {
        control_process_free(cp);
        FAIL("Should have no spheres initially");
    }
    
    control_process_free(cp);
    PASS();
}

// ============================================================================
// TEST: Control Process Start/Stop
// ============================================================================

void test_control_process_start_stop(void) {
    TEST("control_process_start_stop");
    
    SystemConfiguration config = {
        .max_hierarchy_depth = 3,
        .max_spheres_per_level = 12,
        .initial_sphere_count = 1,
        .batch_size = 32,
        .max_epochs = 100,
        .learning_rate = 0.001,
        .max_threads = 4,
        .max_memory_bytes = 1024 * 1024 * 1024,
        .sync_interval_batches = 10,
        .checkpoint_interval_epochs = 5,
        .health_check_interval_ms = 1000,
        .sphere_timeout_seconds = 30.0,
        .enable_boundary_awareness = true,
        .enable_twin_prime_tracking = true
    };
    
    ControlProcess* cp = control_process_create(&config);
    if (!cp) FAIL("Failed to create control process");
    
    // Start control process
    if (!control_process_start(cp)) {
        control_process_free(cp);
        FAIL("Failed to start control process");
    }
    
    // Verify state changed to RUNNING
    if (cp->state != CONTROL_STATE_RUNNING) {
        control_process_free(cp);
        FAIL("State should be RUNNING after start");
    }
    
    // Verify root sphere created
    if (!cp->root_sphere) {
        control_process_free(cp);
        FAIL("Root sphere should be created");
    }
    
    if (cp->total_sphere_count != 1) {
        control_process_free(cp);
        FAIL("Should have 1 sphere after start");
    }
    
    // Stop control process
    if (!control_process_stop(cp)) {
        control_process_free(cp);
        FAIL("Failed to stop control process");
    }
    
    // Verify state changed to STOPPED
    if (cp->state != CONTROL_STATE_STOPPED) {
        control_process_free(cp);
        FAIL("State should be STOPPED after stop");
    }
    
    control_process_free(cp);
    PASS();
}

// ============================================================================
// TEST: Control Process Pause/Resume
// ============================================================================

void test_control_process_pause_resume(void) {
    TEST("control_process_pause_resume");
    
    SystemConfiguration config = {
        .max_hierarchy_depth = 3,
        .max_spheres_per_level = 12,
        .initial_sphere_count = 1,
        .batch_size = 32,
        .max_epochs = 100,
        .learning_rate = 0.001,
        .max_threads = 4,
        .max_memory_bytes = 1024 * 1024 * 1024,
        .sync_interval_batches = 10,
        .checkpoint_interval_epochs = 5,
        .health_check_interval_ms = 1000,
        .sphere_timeout_seconds = 30.0,
        .enable_boundary_awareness = true,
        .enable_twin_prime_tracking = true
    };
    
    ControlProcess* cp = control_process_create(&config);
    if (!cp) FAIL("Failed to create control process");
    
    if (!control_process_start(cp)) {
        control_process_free(cp);
        FAIL("Failed to start control process");
    }
    
    // Pause
    if (!control_process_pause(cp)) {
        control_process_free(cp);
        FAIL("Failed to pause control process");
    }
    
    if (cp->state != CONTROL_STATE_PAUSED) {
        control_process_free(cp);
        FAIL("State should be PAUSED");
    }
    
    // Resume
    if (!control_process_resume(cp)) {
        control_process_free(cp);
        FAIL("Failed to resume control process");
    }
    
    if (cp->state != CONTROL_STATE_RUNNING) {
        control_process_free(cp);
        FAIL("State should be RUNNING after resume");
    }
    
    control_process_stop(cp);
    control_process_free(cp);
    PASS();
}

// ============================================================================
// TEST: Epoch Management
// ============================================================================

void test_epoch_management(void) {
    TEST("epoch_management");
    
    SystemConfiguration config = {
        .max_hierarchy_depth = 3,
        .max_spheres_per_level = 12,
        .initial_sphere_count = 1,
        .batch_size = 32,
        .max_epochs = 100,
        .learning_rate = 0.001,
        .max_threads = 4,
        .max_memory_bytes = 1024 * 1024 * 1024,
        .sync_interval_batches = 10,
        .checkpoint_interval_epochs = 5,
        .health_check_interval_ms = 1000,
        .sphere_timeout_seconds = 30.0,
        .enable_boundary_awareness = true,
        .enable_twin_prime_tracking = true
    };
    
    ControlProcess* cp = control_process_create(&config);
    if (!cp) FAIL("Failed to create control process");
    
    if (!control_process_start(cp)) {
        control_process_free(cp);
        FAIL("Failed to start control process");
    }
    
    // Start epoch
    if (!control_process_start_epoch(cp, 100)) {
        control_process_stop(cp);
        control_process_free(cp);
        FAIL("Failed to start epoch");
    }
    
    // Verify epoch state
    if (!cp->epoch_state.epoch_in_progress) {
        control_process_stop(cp);
        control_process_free(cp);
        FAIL("Epoch should be in progress");
    }
    
    if (cp->epoch_state.current_epoch != 1) {
        control_process_stop(cp);
        control_process_free(cp);
        FAIL("Current epoch should be 1");
    }
    
    if (cp->epoch_state.total_batches != 100) {
        control_process_stop(cp);
        control_process_free(cp);
        FAIL("Total batches should be 100");
    }
    
    // Get epoch stats
    EpochState epoch_state;
    if (!control_process_get_epoch_stats(cp, &epoch_state)) {
        control_process_stop(cp);
        control_process_free(cp);
        FAIL("Failed to get epoch stats");
    }
    
    if (epoch_state.current_epoch != 1) {
        control_process_stop(cp);
        control_process_free(cp);
        FAIL("Epoch stats incorrect");
    }
    
    // End epoch
    if (!control_process_end_epoch(cp)) {
        control_process_stop(cp);
        control_process_free(cp);
        FAIL("Failed to end epoch");
    }
    
    if (cp->epoch_state.epoch_in_progress) {
        control_process_stop(cp);
        control_process_free(cp);
        FAIL("Epoch should not be in progress after end");
    }
    
    control_process_stop(cp);
    control_process_free(cp);
    PASS();
}

// ============================================================================
// TEST: Sphere Spawning
// ============================================================================

void test_sphere_spawning(void) {
    TEST("sphere_spawning");
    
    SystemConfiguration config = {
        .max_hierarchy_depth = 3,
        .max_spheres_per_level = 12,
        .initial_sphere_count = 1,
        .batch_size = 32,
        .max_epochs = 100,
        .learning_rate = 0.001,
        .max_threads = 4,
        .max_memory_bytes = 1024 * 1024 * 1024,
        .sync_interval_batches = 10,
        .checkpoint_interval_epochs = 5,
        .health_check_interval_ms = 1000,
        .sphere_timeout_seconds = 30.0,
        .enable_boundary_awareness = true,
        .enable_twin_prime_tracking = true
    };
    
    ControlProcess* cp = control_process_create(&config);
    if (!cp) FAIL("Failed to create control process");
    
    if (!control_process_start(cp)) {
        control_process_free(cp);
        FAIL("Failed to start control process");
    }
    
    // Spawn child sphere
    uint32_t child_id = control_process_spawn_sphere(cp, 1, 0);
    if (child_id == 0) {
        control_process_stop(cp);
        control_process_free(cp);
        FAIL("Failed to spawn child sphere");
    }
    
    if (cp->total_sphere_count != 2) {
        control_process_stop(cp);
        control_process_free(cp);
        FAIL("Should have 2 spheres after spawning");
    }
    
    // Verify child added to root
    if (cp->root_sphere->num_children != 1) {
        control_process_stop(cp);
        control_process_free(cp);
        FAIL("Root should have 1 child");
    }
    
    control_process_stop(cp);
    control_process_free(cp);
    PASS();
}

// ============================================================================
// TEST: Sphere Termination
// ============================================================================

void test_sphere_termination(void) {
    TEST("sphere_termination");
    
    SystemConfiguration config = {
        .max_hierarchy_depth = 3,
        .max_spheres_per_level = 12,
        .initial_sphere_count = 1,
        .batch_size = 32,
        .max_epochs = 100,
        .learning_rate = 0.001,
        .max_threads = 4,
        .max_memory_bytes = 1024 * 1024 * 1024,
        .sync_interval_batches = 10,
        .checkpoint_interval_epochs = 5,
        .health_check_interval_ms = 1000,
        .sphere_timeout_seconds = 30.0,
        .enable_boundary_awareness = true,
        .enable_twin_prime_tracking = true
    };
    
    ControlProcess* cp = control_process_create(&config);
    if (!cp) FAIL("Failed to create control process");
    
    if (!control_process_start(cp)) {
        control_process_free(cp);
        FAIL("Failed to start control process");
    }
    
    // Spawn child sphere
    uint32_t child_id = control_process_spawn_sphere(cp, 1, 0);
    if (child_id == 0) {
        control_process_stop(cp);
        control_process_free(cp);
        FAIL("Failed to spawn child sphere");
    }
    
    // Terminate child sphere
    if (!control_process_terminate_sphere(cp, child_id)) {
        control_process_stop(cp);
        control_process_free(cp);
        FAIL("Failed to terminate child sphere");
    }
    
    if (cp->total_sphere_count != 1) {
        control_process_stop(cp);
        control_process_free(cp);
        FAIL("Should have 1 sphere after termination");
    }
    
    // Verify child removed from root
    if (cp->root_sphere->num_children != 0) {
        control_process_stop(cp);
        control_process_free(cp);
        FAIL("Root should have 0 children");
    }
    
    control_process_stop(cp);
    control_process_free(cp);
    PASS();
}

// ============================================================================
// TEST: Sphere Finding
// ============================================================================

void test_sphere_finding(void) {
    TEST("sphere_finding");
    
    SystemConfiguration config = {
        .max_hierarchy_depth = 3,
        .max_spheres_per_level = 12,
        .initial_sphere_count = 1,
        .batch_size = 32,
        .max_epochs = 100,
        .learning_rate = 0.001,
        .max_threads = 4,
        .max_memory_bytes = 1024 * 1024 * 1024,
        .sync_interval_batches = 10,
        .checkpoint_interval_epochs = 5,
        .health_check_interval_ms = 1000,
        .sphere_timeout_seconds = 30.0,
        .enable_boundary_awareness = true,
        .enable_twin_prime_tracking = true
    };
    
    ControlProcess* cp = control_process_create(&config);
    if (!cp) FAIL("Failed to create control process");
    
    if (!control_process_start(cp)) {
        control_process_free(cp);
        FAIL("Failed to start control process");
    }
    
    // Find root sphere
    CLLMLatticeHierarchy* root = control_process_find_sphere(cp, 1);
    if (!root) {
        control_process_stop(cp);
        control_process_free(cp);
        FAIL("Failed to find root sphere");
    }
    
    if (root != cp->root_sphere) {
        control_process_stop(cp);
        control_process_free(cp);
        FAIL("Found sphere should be root");
    }
    
    // Try to find non-existent sphere
    CLLMLatticeHierarchy* not_found = control_process_find_sphere(cp, 999);
    if (not_found) {
        control_process_stop(cp);
        control_process_free(cp);
        FAIL("Should not find non-existent sphere");
    }
    
    control_process_stop(cp);
    control_process_free(cp);
    PASS();
}

// ============================================================================
// TEST: Sphere Counting
// ============================================================================

void test_sphere_counting(void) {
    TEST("sphere_counting");
    
    SystemConfiguration config = {
        .max_hierarchy_depth = 3,
        .max_spheres_per_level = 12,
        .initial_sphere_count = 1,
        .batch_size = 32,
        .max_epochs = 100,
        .learning_rate = 0.001,
        .max_threads = 4,
        .max_memory_bytes = 1024 * 1024 * 1024,
        .sync_interval_batches = 10,
        .checkpoint_interval_epochs = 5,
        .health_check_interval_ms = 1000,
        .sphere_timeout_seconds = 30.0,
        .enable_boundary_awareness = true,
        .enable_twin_prime_tracking = true
    };
    
    ControlProcess* cp = control_process_create(&config);
    if (!cp) FAIL("Failed to create control process");
    
    if (!control_process_start(cp)) {
        control_process_free(cp);
        FAIL("Failed to start control process");
    }
    
    // Initial count
    uint32_t count = control_process_count_spheres(cp);
    if (count != 1) {
        control_process_stop(cp);
        control_process_free(cp);
        FAIL("Initial count should be 1");
    }
    
    // Spawn 3 children
    for (int i = 0; i < 3; i++) {
        control_process_spawn_sphere(cp, 1, i);
    }
    
    count = control_process_count_spheres(cp);
    if (count != 4) {
        control_process_stop(cp);
        control_process_free(cp);
        FAIL("Count should be 4 after spawning 3 children");
    }
    
    control_process_stop(cp);
    control_process_free(cp);
    PASS();
}

// ============================================================================
// TEST: System Statistics
// ============================================================================

void test_system_statistics(void) {
    TEST("system_statistics");
    
    SystemConfiguration config = {
        .max_hierarchy_depth = 3,
        .max_spheres_per_level = 12,
        .initial_sphere_count = 1,
        .batch_size = 32,
        .max_epochs = 100,
        .learning_rate = 0.001,
        .max_threads = 4,
        .max_memory_bytes = 1024 * 1024 * 1024,
        .sync_interval_batches = 10,
        .checkpoint_interval_epochs = 5,
        .health_check_interval_ms = 1000,
        .sphere_timeout_seconds = 30.0,
        .enable_boundary_awareness = true,
        .enable_twin_prime_tracking = true
    };
    
    ControlProcess* cp = control_process_create(&config);
    if (!cp) FAIL("Failed to create control process");
    
    if (!control_process_start(cp)) {
        control_process_free(cp);
        FAIL("Failed to start control process");
    }
    
    // Get system stats
    SphereStatistics stats;
    if (!control_process_get_system_stats(cp, &stats)) {
        control_process_stop(cp);
        control_process_free(cp);
        FAIL("Failed to get system stats");
    }
    
    // Get sphere stats
    SphereStatistics sphere_stats;
    if (!control_process_get_sphere_stats(cp, 1, &sphere_stats)) {
        control_process_stop(cp);
        control_process_free(cp);
        FAIL("Failed to get sphere stats");
    }
    
    control_process_stop(cp);
    control_process_free(cp);
    PASS();
}

// ============================================================================
// TEST: System Health
// ============================================================================

void test_system_health(void) {
    TEST("system_health");
    
    SystemConfiguration config = {
        .max_hierarchy_depth = 3,
        .max_spheres_per_level = 12,
        .initial_sphere_count = 1,
        .batch_size = 32,
        .max_epochs = 100,
        .learning_rate = 0.001,
        .max_threads = 4,
        .max_memory_bytes = 1024 * 1024 * 1024,
        .sync_interval_batches = 10,
        .checkpoint_interval_epochs = 5,
        .health_check_interval_ms = 1000,
        .sphere_timeout_seconds = 30.0,
        .enable_boundary_awareness = true,
        .enable_twin_prime_tracking = true
    };
    
    ControlProcess* cp = control_process_create(&config);
    if (!cp) FAIL("Failed to create control process");
    
    if (!control_process_start(cp)) {
        control_process_free(cp);
        FAIL("Failed to start control process");
    }
    
    // Get system health
    SystemHealth health;
    if (!control_process_get_system_health(cp, &health)) {
        control_process_stop(cp);
        control_process_free(cp);
        FAIL("Failed to get system health");
    }
    
    // Check sphere health
    if (!control_process_check_sphere_health(cp, 1)) {
        control_process_stop(cp);
        control_process_free(cp);
        FAIL("Root sphere should be healthy");
    }
    
    control_process_stop(cp);
    control_process_free(cp);
    PASS();
}

// ============================================================================
// TEST: State Transitions
// ============================================================================

void test_state_transitions(void) {
    TEST("state_transitions");
    
    SystemConfiguration config = {
        .max_hierarchy_depth = 3,
        .max_spheres_per_level = 12,
        .initial_sphere_count = 1,
        .batch_size = 32,
        .max_epochs = 100,
        .learning_rate = 0.001,
        .max_threads = 4,
        .max_memory_bytes = 1024 * 1024 * 1024,
        .sync_interval_batches = 10,
        .checkpoint_interval_epochs = 5,
        .health_check_interval_ms = 1000,
        .sphere_timeout_seconds = 30.0,
        .enable_boundary_awareness = true,
        .enable_twin_prime_tracking = true
    };
    
    ControlProcess* cp = control_process_create(&config);
    if (!cp) FAIL("Failed to create control process");
    
    // INITIALIZING -> RUNNING
    if (control_process_get_state(cp) != CONTROL_STATE_INITIALIZING) {
        control_process_free(cp);
        FAIL("Should start in INITIALIZING state");
    }
    
    control_process_start(cp);
    if (control_process_get_state(cp) != CONTROL_STATE_RUNNING) {
        control_process_free(cp);
        FAIL("Should transition to RUNNING");
    }
    
    // RUNNING -> PAUSED
    control_process_pause(cp);
    if (control_process_get_state(cp) != CONTROL_STATE_PAUSED) {
        control_process_free(cp);
        FAIL("Should transition to PAUSED");
    }
    
    // PAUSED -> RUNNING
    control_process_resume(cp);
    if (control_process_get_state(cp) != CONTROL_STATE_RUNNING) {
        control_process_free(cp);
        FAIL("Should transition back to RUNNING");
    }
    
    // RUNNING -> STOPPED
    control_process_stop(cp);
    if (control_process_get_state(cp) != CONTROL_STATE_STOPPED) {
        control_process_free(cp);
        FAIL("Should transition to STOPPED");
    }
    
    control_process_free(cp);
    PASS();
}

// ============================================================================
// TEST: Validation
// ============================================================================

void test_validation(void) {
    TEST("validation");
    
    SystemConfiguration config = {
        .max_hierarchy_depth = 3,
        .max_spheres_per_level = 12,
        .initial_sphere_count = 1,
        .batch_size = 32,
        .max_epochs = 100,
        .learning_rate = 0.001,
        .max_threads = 4,
        .max_memory_bytes = 1024 * 1024 * 1024,
        .sync_interval_batches = 10,
        .checkpoint_interval_epochs = 5,
        .health_check_interval_ms = 1000,
        .sphere_timeout_seconds = 30.0,
        .enable_boundary_awareness = true,
        .enable_twin_prime_tracking = true
    };
    
    ControlProcess* cp = control_process_create(&config);
    if (!cp) FAIL("Failed to create control process");
    
    // Should be valid after creation
    if (!control_process_validate(cp)) {
        control_process_free(cp);
        FAIL("Should be valid after creation");
    }
    
    control_process_start(cp);
    
    // Should be valid after start
    if (!control_process_validate(cp)) {
        control_process_stop(cp);
        control_process_free(cp);
        FAIL("Should be valid after start");
    }
    
    control_process_stop(cp);
    control_process_free(cp);
    PASS();
}

// ============================================================================
// TEST: Print Functions
// ============================================================================

void test_print_functions(void) {
    TEST("print_functions");
    
    SystemConfiguration config = {
        .max_hierarchy_depth = 3,
        .max_spheres_per_level = 12,
        .initial_sphere_count = 1,
        .batch_size = 32,
        .max_epochs = 100,
        .learning_rate = 0.001,
        .max_threads = 4,
        .max_memory_bytes = 1024 * 1024 * 1024,
        .sync_interval_batches = 10,
        .checkpoint_interval_epochs = 5,
        .health_check_interval_ms = 1000,
        .sphere_timeout_seconds = 30.0,
        .enable_boundary_awareness = true,
        .enable_twin_prime_tracking = true
    };
    
    ControlProcess* cp = control_process_create(&config);
    if (!cp) FAIL("Failed to create control process");
    
    control_process_start(cp);
    
    // These should not crash
    control_process_print_hierarchy(cp);
    control_process_print_stats(cp);
    
    control_process_stop(cp);
    control_process_free(cp);
    PASS();
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main(void) {
    printf("\n");
    printf("========================================\n");
    printf("  Control Process Tests\n");
    printf("========================================\n\n");
    
    // Run all tests
    test_control_process_create();
    test_control_process_start_stop();
    test_control_process_pause_resume();
    test_epoch_management();
    test_sphere_spawning();
    test_sphere_termination();
    test_sphere_finding();
    test_sphere_counting();
    test_system_statistics();
    test_system_health();
    test_state_transitions();
    test_validation();
    test_print_functions();
    
    // Print summary
    printf("\n");
    printf("========================================\n");
    printf("  Test Summary\n");
    printf("========================================\n");
    printf("Tests run:    %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);
    printf("Success rate: %.1f%%\n", 
           tests_run > 0 ? (100.0 * tests_passed / tests_run) : 0.0);
    printf("========================================\n\n");
    
    return (tests_passed == tests_run) ? 0 : 1;
}