/**
 * @file test_state_work_week4.c
 * @brief Comprehensive tests for Week 4: State Management + Work Distribution
 */

#define _GNU_SOURCE
#include "state_management.h"
#include "work_distribution.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

// Test framework
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) \
    printf("\n=== TEST: %s ===\n", #name); \
    fflush(stdout); \
    if (test_##name()) { \
        printf("✓ PASSED: %s\n", #name); \
        fflush(stdout); \
        tests_passed++; \
    } else { \
        printf("✗ FAILED: %s\n", #name); \
        fflush(stdout); \
        tests_failed++; \
    }

// ============================================================================
// STATE MANAGEMENT TESTS
// ============================================================================

bool test_state_manager_create() {
    StateManager* manager = state_manager_create(10, 100);
    assert(manager != NULL);
    assert(manager->max_machines == 10);
    assert(manager->max_registered_states == 100);
    
    state_manager_destroy(manager);
    return true;
}

bool test_state_machine_create() {
    StateManager* manager = state_manager_create(10, 100);
    
    StateMachine* machine = state_machine_create(manager, 1, STATE_UNINITIALIZED, 10, 5);
    assert(machine != NULL);
    assert(machine->id == 1);
    assert(machine->current_state == STATE_UNINITIALIZED);
    
    // Don't destroy machine explicitly - manager will do it
    state_manager_destroy(manager);
    return true;
}

bool test_state_transitions() {
    StateManager* manager = state_manager_create(10, 100);
    StateMachine* machine = state_machine_create(manager, 1, STATE_UNINITIALIZED, 10, 5);
    
    // Test basic transition
    TransitionResult result = state_machine_transition(machine, STATE_INITIALIZING);
    assert(result == TRANSITION_SUCCESS);
    assert(machine->current_state == STATE_INITIALIZING);
    assert(machine->previous_state == STATE_UNINITIALIZED);
    
    // Test another transition
    result = state_machine_transition(machine, STATE_READY);
    assert(result == TRANSITION_SUCCESS);
    assert(machine->current_state == STATE_READY);
    
    state_manager_destroy(manager);
    return true;
}

bool test_state_transition_rules() {
    StateManager* manager = state_manager_create(10, 100);
    StateMachine* machine = state_machine_create(manager, 1, STATE_READY, 10, 5);
    
    // Add transition rule: READY -> RUNNING
    bool added = state_machine_add_transition(machine, STATE_READY, STATE_RUNNING, NULL, NULL);
    assert(added);
    
    // Test valid transition
    assert(state_machine_can_transition(machine, STATE_RUNNING));
    
    TransitionResult result = state_machine_transition(machine, STATE_RUNNING);
    assert(result == TRANSITION_SUCCESS);
    
    state_manager_destroy(manager);
    return true;
}

static int callback_count = 0;
static void test_callback(const StateChangeEvent* event, void* user_data) {
    (void)event;
    (void)user_data;
    callback_count++;
}

bool test_state_callbacks() {
    StateManager* manager = state_manager_create(10, 100);
    StateMachine* machine = state_machine_create(manager, 1, STATE_READY, 10, 5);
    
    callback_count = 0;
    
    // Register callback
    bool registered = state_machine_register_callback(machine, test_callback, NULL);
    assert(registered);
    
    // Trigger transition
    state_machine_transition(machine, STATE_RUNNING);
    assert(callback_count == 1);
    
    // Another transition
    state_machine_transition(machine, STATE_PAUSED);
    assert(callback_count == 2);
    
    state_manager_destroy(manager);
    return true;
}

bool test_state_statistics() {
    StateManager* manager = state_manager_create(10, 100);
    StateMachine* machine = state_machine_create(manager, 1, STATE_READY, 10, 5);
    
    // Make some transitions
    state_machine_transition(machine, STATE_RUNNING);
    usleep(1000);  // 1ms
    state_machine_transition(machine, STATE_PAUSED);
    usleep(1000);  // 1ms
    state_machine_transition(machine, STATE_RUNNING);
    
    // Check statistics
    uint64_t count = state_machine_get_state_count(machine, STATE_RUNNING);
    assert(count == 2);  // Entered RUNNING twice
    
    uint64_t transitions = state_machine_get_transition_count(machine);
    assert(transitions == 3);
    
    state_manager_destroy(manager);
    return true;
}

bool test_state_names() {
    const char* name = state_get_name(STATE_RUNNING);
    assert(strcmp(name, "RUNNING") == 0);
    
    name = state_get_name(STATE_ERROR);
    assert(strcmp(name, "ERROR") == 0);
    
    return true;
}

// ============================================================================
// WORK DISTRIBUTION TESTS
// ============================================================================

static void simple_work(void* data) {
    int* value = (int*)data;
    (*value)++;
}

bool test_work_distributor_create() {
    WorkDistributor* dist = work_distributor_create(4, 100);
    assert(dist != NULL);
    assert(dist->num_workers == 4);
    
    work_distributor_destroy(dist);
    return true;
}

bool test_work_submit_get() {
    WorkDistributor* dist = work_distributor_create(4, 100);
    
    int* value = (int*)malloc(sizeof(int));
    *value = 0;
    bool submitted = work_submit(dist, 0, simple_work, value, sizeof(int), WORK_PRIORITY_NORMAL);
    assert(submitted);
    
    WorkItem* item = work_get(dist, 0);
    assert(item != NULL);
    assert(item->work_fn == simple_work);
    
    work_complete(dist, item);
    free(value);
    
    work_distributor_destroy(dist);
    return true;
}

bool test_work_execution() {
    WorkDistributor* dist = work_distributor_create(4, 100);
    
    int* value = (int*)malloc(sizeof(int));
    *value = 10;
    work_submit(dist, 0, simple_work, value, sizeof(int), WORK_PRIORITY_NORMAL);
    
    WorkItem* item = work_get(dist, 0);
    assert(item != NULL);
    
    // Execute work
    item->start_time = work_get_timestamp();
    item->work_fn(item->data);
    
    assert(*value == 11);  // Work was executed
    
    work_complete(dist, item);
    free(value);
    
    work_distributor_destroy(dist);
    return true;
}

bool test_work_global_queue() {
    WorkDistributor* dist = work_distributor_create(4, 100);
    
    int* value = (int*)malloc(sizeof(int));
    *value = 0;
    bool submitted = work_submit_global(dist, simple_work, value, sizeof(int), WORK_PRIORITY_NORMAL);
    assert(submitted);
    
    // Any worker can get it
    WorkItem* item = work_get(dist, 2);
    assert(item != NULL);
    
    work_complete(dist, item);
    free(value);
    
    work_distributor_destroy(dist);
    return true;
}

bool test_work_stealing() {
    WorkDistributor* dist = work_distributor_create(4, 100);
    
    // Submit work to worker 0
    for (int i = 0; i < 10; i++) {
        int* value = (int*)malloc(sizeof(int));
        *value = i;
        work_submit(dist, 0, simple_work, value, sizeof(int), WORK_PRIORITY_NORMAL);
    }
    
    // Worker 1 steals from worker 0
    WorkItem* stolen = work_steal(dist, 1);
    assert(stolen != NULL);
    assert(stolen->worker_id == 1);  // Now owned by worker 1
    
    work_complete(dist, stolen);
    
    work_distributor_destroy(dist);
    return true;
}

bool test_work_batch_operations() {
    WorkDistributor* dist = work_distributor_create(4, 100);
    
    const int BATCH_SIZE = 5;
    void (*work_fns[BATCH_SIZE])(void*);
    void* data[BATCH_SIZE];
    size_t data_sizes[BATCH_SIZE];
    WorkPriority priorities[BATCH_SIZE];
    
    for (int i = 0; i < BATCH_SIZE; i++) {
        work_fns[i] = simple_work;
        data[i] = malloc(sizeof(int));
        *(int*)data[i] = i;
        data_sizes[i] = sizeof(int);
        priorities[i] = WORK_PRIORITY_NORMAL;
    }
    
    uint32_t submitted = work_submit_batch(dist, 0, work_fns, data, data_sizes, priorities, BATCH_SIZE);
    assert(submitted == BATCH_SIZE);
    
    // Get batch
    WorkItem* items[BATCH_SIZE];
    uint32_t got = work_get_batch(dist, 0, items, BATCH_SIZE);
    assert(got == BATCH_SIZE);
    
    // Complete all and free data
    for (uint32_t i = 0; i < got; i++) {
        free(items[i]->data);  // Free the data before completing
        work_complete(dist, items[i]);
    }
    
    work_distributor_destroy(dist);
    return true;
}

bool test_work_load_balancing() {
    WorkDistributor* dist = work_distributor_create(4, 100);
    
    // Submit work to worker 0
    for (int i = 0; i < 20; i++) {
        int* value = (int*)malloc(sizeof(int));
        *value = i;
        work_submit(dist, 0, simple_work, value, sizeof(int), WORK_PRIORITY_NORMAL);
    }
    
    uint64_t load0 = work_get_worker_load(dist, 0);
    assert(load0 == 20);
    
    uint64_t load1 = work_get_worker_load(dist, 1);
    assert(load1 == 0);
    
    // Find most loaded
    uint32_t most = work_find_most_loaded_worker(dist);
    assert(most == 0);
    
    // Find least loaded
    uint32_t least = work_find_least_loaded_worker(dist);
    assert(least != 0);
    
    work_distributor_destroy(dist);
    return true;
}

bool test_work_statistics() {
    WorkDistributor* dist = work_distributor_create(4, 100);
    
    // Submit and complete some work
    for (int i = 0; i < 5; i++) {
        int* value = (int*)malloc(sizeof(int));
        *value = i;
        work_submit(dist, 0, simple_work, value, sizeof(int), WORK_PRIORITY_NORMAL);
    }
    
    // Process work
    for (int i = 0; i < 5; i++) {
        WorkItem* item = work_get(dist, 0);
        if (item) {
            item->start_time = work_get_timestamp();
            item->work_fn(item->data);
            work_complete(dist, item);
        }
    }
    
    uint64_t completed, stolen, received, avg_time;
    work_get_worker_stats(dist, 0, &completed, &stolen, &received, &avg_time);
    
    assert(completed == 5);
    assert(received == 5);
    
    work_distributor_destroy(dist);
    return true;
}

bool test_work_priority_names() {
    const char* name = work_priority_name(WORK_PRIORITY_HIGH);
    assert(strcmp(name, "HIGH") == 0);
    
    name = work_status_name(WORK_STATUS_COMPLETED);
    assert(strcmp(name, "COMPLETED") == 0);
    
    return true;
}

// ============================================================================
// INTEGRATION TESTS
// ============================================================================

bool test_state_work_integration() {
    // Create state manager and work distributor
    StateManager* state_mgr = state_manager_create(10, 100);
    WorkDistributor* work_dist = work_distributor_create(4, 100);
    
    // Create state machine for worker 0
    StateMachine* worker_state = state_machine_create(state_mgr, 0, STATE_IDLE, 10, 5);
    
    // Transition to WORKING
    state_machine_transition(worker_state, STATE_WORKING);
    assert(worker_state->current_state == STATE_WORKING);
    
    // Submit work
    int* value = (int*)malloc(sizeof(int));
    *value = 0;
    work_submit(work_dist, 0, simple_work, value, sizeof(int), WORK_PRIORITY_NORMAL);
    
    // Get and execute work
    WorkItem* item = work_get(work_dist, 0);
    assert(item != NULL);
    
    item->start_time = work_get_timestamp();
    item->work_fn(item->data);
    work_complete(work_dist, item);
    free(value);
    
    // Transition back to IDLE
    state_machine_transition(worker_state, STATE_IDLE);
    assert(worker_state->current_state == STATE_IDLE);
    
    state_manager_destroy(state_mgr);
    work_distributor_destroy(work_dist);
    
    return true;
}

bool test_stress_combined() {
    StateManager* state_mgr = state_manager_create(10, 100);
    WorkDistributor* work_dist = work_distributor_create(4, 1000);
    
    // Create state machines for all workers
    for (int i = 0; i < 4; i++) {
        state_machine_create(state_mgr, i, STATE_IDLE, 10, 5);
    }
    
    // Submit 100 work items
    for (int i = 0; i < 100; i++) {
        int* value = (int*)malloc(sizeof(int));
        *value = i;
        work_submit(work_dist, i % 4, simple_work, value, sizeof(int), WORK_PRIORITY_NORMAL);
    }
    
    // Process all work
    int processed = 0;
    for (int worker = 0; worker < 4; worker++) {
        StateMachine* machine = state_mgr->machines[worker];
        state_machine_transition(machine, STATE_WORKING);
        
        while (true) {
            WorkItem* item = work_get(work_dist, worker);
            if (!item) {
                item = work_steal(work_dist, worker);
            }
            if (!item) break;
            
            item->start_time = work_get_timestamp();
            item->work_fn(item->data);
            free(item->data);  // Free the data
            work_complete(work_dist, item);
            processed++;
        }
        
        state_machine_transition(machine, STATE_IDLE);
    }
    
    printf("Processed %d work items\n", processed);
    assert(processed == 100);
    
    state_manager_destroy(state_mgr);
    work_distributor_destroy(work_dist);
    
    return true;
}

// ============================================================================
// MAIN
// ============================================================================

int main(void) {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║   WEEK 4: STATE MANAGEMENT + WORK DISTRIBUTION TESTS      ║\n");
    printf("║   Hierarchical State Machine + Lock-Free Work Queues      ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    printf("\n--- State Management Tests ---\n");
    TEST(state_manager_create);
    TEST(state_machine_create);
    TEST(state_transitions);
    TEST(state_transition_rules);
    TEST(state_callbacks);
    TEST(state_statistics);
    TEST(state_names);
    
    printf("\n--- Work Distribution Tests ---\n");
    TEST(work_distributor_create);
    TEST(work_submit_get);
    TEST(work_execution);
    TEST(work_global_queue);
    TEST(work_stealing);
    TEST(work_batch_operations);
    TEST(work_load_balancing);
    TEST(work_statistics);
    TEST(work_priority_names);
    
    printf("\n--- Integration Tests ---\n");
    TEST(state_work_integration);
    TEST(stress_combined);
    
    // Summary
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                      TEST SUMMARY                          ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║  Total Tests:   %-2d                                         ║\n", tests_passed + tests_failed);
    printf("║  Passed:        %-2d  ✓                                      ║\n", tests_passed);
    printf("║  Failed:         %-2d  ✗                                      ║\n", tests_failed);
    printf("║  Success Rate: %3d%%                                        ║\n", 
           (tests_passed * 100) / (tests_passed + tests_failed));
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    return tests_failed > 0 ? 1 : 0;
}