/**
 * @file state_management.c
 * @brief Implementation of Generic Hierarchical State Management System
 */

#include "state_management.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdatomic.h>

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

uint64_t state_get_timestamp(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

const char* state_get_name(StateType state) {
    switch (state) {
        case STATE_UNINITIALIZED: return "UNINITIALIZED";
        case STATE_INITIALIZING: return "INITIALIZING";
        case STATE_INITIALIZED: return "INITIALIZED";
        case STATE_READY: return "READY";
        case STATE_RUNNING: return "RUNNING";
        case STATE_PAUSED: return "PAUSED";
        case STATE_STOPPING: return "STOPPING";
        case STATE_STOPPED: return "STOPPED";
        case STATE_ERROR: return "ERROR";
        case STATE_TERMINATED: return "TERMINATED";
        case STATE_IDLE: return "IDLE";
        case STATE_WAITING: return "WAITING";
        case STATE_WORKING: return "WORKING";
        case STATE_BLOCKED: return "BLOCKED";
        case STATE_YIELDING: return "YIELDING";
        case STATE_BARRIER_WAIT: return "BARRIER_WAIT";
        case STATE_BARRIER_READY: return "BARRIER_READY";
        case STATE_LOCK_WAIT: return "LOCK_WAIT";
        case STATE_LOCK_ACQUIRED: return "LOCK_ACQUIRED";
        default: return "UNKNOWN";
    }
}

const char* transition_result_name(TransitionResult result) {
    switch (result) {
        case TRANSITION_SUCCESS: return "SUCCESS";
        case TRANSITION_INVALID: return "INVALID";
        case TRANSITION_BLOCKED: return "BLOCKED";
        case TRANSITION_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

// ============================================================================
// STATE MANAGER IMPLEMENTATION
// ============================================================================

StateManager* state_manager_create(uint32_t max_machines, uint32_t max_states) {
    StateManager* manager = (StateManager*)calloc(1, sizeof(StateManager));
    if (!manager) return NULL;
    
    manager->machines = (StateMachine**)calloc(max_machines, sizeof(StateMachine*));
    if (!manager->machines) {
        free(manager);
        return NULL;
    }
    
    manager->state_registry = (StateInfo*)calloc(max_states, sizeof(StateInfo));
    if (!manager->state_registry) {
        free(manager->machines);
        free(manager);
        return NULL;
    }
    
    manager->max_machines = max_machines;
    manager->max_registered_states = max_states;
    
    // Register standard states
    state_manager_register_state(manager, STATE_UNINITIALIZED, "UNINITIALIZED", "Not initialized", STATE_FLAG_NONE);
    state_manager_register_state(manager, STATE_INITIALIZING, "INITIALIZING", "Initializing", STATE_FLAG_TRANSIENT);
    state_manager_register_state(manager, STATE_INITIALIZED, "INITIALIZED", "Initialized", STATE_FLAG_NONE);
    state_manager_register_state(manager, STATE_READY, "READY", "Ready to run", STATE_FLAG_NONE);
    state_manager_register_state(manager, STATE_RUNNING, "RUNNING", "Running", STATE_FLAG_NONE);
    state_manager_register_state(manager, STATE_PAUSED, "PAUSED", "Paused", STATE_FLAG_NONE);
    state_manager_register_state(manager, STATE_STOPPING, "STOPPING", "Stopping", STATE_FLAG_TRANSIENT);
    state_manager_register_state(manager, STATE_STOPPED, "STOPPED", "Stopped", STATE_FLAG_NONE);
    state_manager_register_state(manager, STATE_ERROR, "ERROR", "Error state", STATE_FLAG_CRITICAL);
    state_manager_register_state(manager, STATE_TERMINATED, "TERMINATED", "Terminated", STATE_FLAG_TERMINAL);
    
    return manager;
}

void state_manager_destroy(StateManager* manager) {
    if (!manager) return;
    
    // Destroy all state machines
    for (uint32_t i = 0; i < manager->num_machines; i++) {
        state_machine_destroy(manager->machines[i]);
    }
    
    free(manager->machines);
    free(manager->state_registry);
    free(manager);
}

bool state_manager_register_state(
    StateManager* manager,
    StateType type,
    const char* name,
    const char* description,
    uint32_t flags
) {
    if (!manager || manager->num_registered_states >= manager->max_registered_states) {
        return false;
    }
    
    StateInfo* info = &manager->state_registry[manager->num_registered_states++];
    info->type = type;
    info->flags = flags;
    info->name = name;
    info->description = description;
    
    return true;
}

const StateInfo* state_manager_get_state_info(StateManager* manager, StateType type) {
    if (!manager) return NULL;
    
    for (uint32_t i = 0; i < manager->num_registered_states; i++) {
        if (manager->state_registry[i].type == type) {
            return &manager->state_registry[i];
        }
    }
    
    return NULL;
}

// ============================================================================
// STATE MACHINE IMPLEMENTATION
// ============================================================================

StateMachine* state_machine_create(
    StateManager* manager,
    uint32_t id,
    StateType initial_state,
    uint32_t max_transitions,
    uint32_t max_callbacks
) {
    if (!manager || manager->num_machines >= manager->max_machines) {
        return NULL;
    }
    
    StateMachine* machine = (StateMachine*)calloc(1, sizeof(StateMachine));
    if (!machine) return NULL;
    
    machine->id = id;
    machine->current_state = initial_state;
    machine->previous_state = initial_state;
    machine->state_entry_time = state_get_timestamp();
    
    // Allocate transitions
    machine->transitions = (StateTransition*)calloc(max_transitions, sizeof(StateTransition));
    if (!machine->transitions) {
        free(machine);
        return NULL;
    }
    machine->max_transitions = max_transitions;
    
    // Allocate callbacks
    machine->callbacks = (StateChangeCallback*)calloc(max_callbacks, sizeof(StateChangeCallback));
    machine->callback_data = (void**)calloc(max_callbacks, sizeof(void*));
    if (!machine->callbacks || !machine->callback_data) {
        free(machine->transitions);
        free(machine->callbacks);
        free(machine->callback_data);
        free(machine);
        return NULL;
    }
    machine->max_callbacks = max_callbacks;
    
    // Allocate statistics arrays (support up to 1000 states)
    machine->num_states = 1000;
    machine->state_durations = (uint64_t*)calloc(machine->num_states, sizeof(uint64_t));
    machine->state_counts = (uint64_t*)calloc(machine->num_states, sizeof(uint64_t));
    if (!machine->state_durations || !machine->state_counts) {
        free(machine->transitions);
        free(machine->callbacks);
        free(machine->callback_data);
        free(machine->state_durations);
        free(machine->state_counts);
        free(machine);
        return NULL;
    }
    
    // Initialize statistics for initial state
    if (initial_state < machine->num_states) {
        machine->state_counts[initial_state] = 1;
    }
    
    // Add to manager
    manager->machines[manager->num_machines++] = machine;
    
    return machine;
}

void state_machine_destroy(StateMachine* machine) {
    if (!machine) return;
    
    free(machine->transitions);
    free(machine->callbacks);
    free(machine->callback_data);
    free(machine->state_durations);
    free(machine->state_counts);
    free(machine);
}

StateType state_machine_get_state(const StateMachine* machine) {
    if (!machine) return STATE_UNINITIALIZED;
    return machine->current_state;
}

StateType state_machine_get_previous_state(const StateMachine* machine) {
    if (!machine) return STATE_UNINITIALIZED;
    return machine->previous_state;
}

bool state_machine_can_transition(
    const StateMachine* machine,
    StateType new_state
) {
    if (!machine) return false;
    
    // Check if there's a transition rule
    for (uint32_t i = 0; i < machine->num_transitions; i++) {
        const StateTransition* trans = &machine->transitions[i];
        if (trans->from_state == machine->current_state && trans->to_state == new_state) {
            // If there's a validator, check it
            if (trans->validator) {
                return trans->validator(machine->context);
            }
            return true;
        }
    }
    
    // If no explicit rule, allow transition (permissive by default)
    return true;
}

TransitionResult state_machine_transition(
    StateMachine* machine,
    StateType new_state
) {
    if (!machine) return TRANSITION_ERROR;
    
    // Check if transition is valid
    if (!state_machine_can_transition(machine, new_state)) {
        return TRANSITION_INVALID;
    }
    
    // Update statistics for old state
    uint64_t now = state_get_timestamp();
    uint64_t duration = now - machine->state_entry_time;
    
    if (machine->current_state < machine->num_states) {
        machine->state_durations[machine->current_state] += duration;
    }
    
    // Perform transition
    StateType old_state = machine->current_state;
    machine->previous_state = old_state;
    machine->current_state = new_state;
    machine->state_entry_time = now;
    machine->total_transitions++;
    
    // Update statistics for new state
    if (new_state < machine->num_states) {
        machine->state_counts[new_state]++;
    }
    
    // Execute transition callback if exists
    for (uint32_t i = 0; i < machine->num_transitions; i++) {
        StateTransition* trans = &machine->transitions[i];
        if (trans->from_state == old_state && trans->to_state == new_state) {
            if (trans->callback) {
                trans->callback(machine->context);
            }
            break;
        }
    }
    
    // Notify all registered callbacks
    StateChangeEvent event = {
        .entity_id = machine->id,
        .old_state = old_state,
        .new_state = new_state,
        .timestamp = now,
        .context = machine->context
    };
    
    for (uint32_t i = 0; i < machine->num_callbacks; i++) {
        if (machine->callbacks[i]) {
            machine->callbacks[i](&event, machine->callback_data[i]);
        }
    }
    
    return TRANSITION_SUCCESS;
}

bool state_machine_force_transition(
    StateMachine* machine,
    StateType new_state
) {
    if (!machine) return false;
    
    // Update statistics for old state
    uint64_t now = state_get_timestamp();
    uint64_t duration = now - machine->state_entry_time;
    
    if (machine->current_state < machine->num_states) {
        machine->state_durations[machine->current_state] += duration;
    }
    
    // Perform transition
    machine->previous_state = machine->current_state;
    machine->current_state = new_state;
    machine->state_entry_time = now;
    machine->total_transitions++;
    
    // Update statistics for new state
    if (new_state < machine->num_states) {
        machine->state_counts[new_state]++;
    }
    
    return true;
}

// ============================================================================
// TRANSITION RULES IMPLEMENTATION
// ============================================================================

bool state_machine_add_transition(
    StateMachine* machine,
    StateType from_state,
    StateType to_state,
    bool (*validator)(void* context),
    void (*callback)(void* context)
) {
    if (!machine || machine->num_transitions >= machine->max_transitions) {
        return false;
    }
    
    StateTransition* trans = &machine->transitions[machine->num_transitions++];
    trans->from_state = from_state;
    trans->to_state = to_state;
    trans->validator = validator;
    trans->callback = callback;
    
    return true;
}

bool state_machine_remove_transition(
    StateMachine* machine,
    StateType from_state,
    StateType to_state
) {
    if (!machine) return false;
    
    for (uint32_t i = 0; i < machine->num_transitions; i++) {
        StateTransition* trans = &machine->transitions[i];
        if (trans->from_state == from_state && trans->to_state == to_state) {
            // Shift remaining transitions
            for (uint32_t j = i; j < machine->num_transitions - 1; j++) {
                machine->transitions[j] = machine->transitions[j + 1];
            }
            machine->num_transitions--;
            return true;
        }
    }
    
    return false;
}

void state_machine_clear_transitions(StateMachine* machine) {
    if (!machine) return;
    machine->num_transitions = 0;
}

// ============================================================================
// CALLBACK IMPLEMENTATION
// ============================================================================

bool state_machine_register_callback(
    StateMachine* machine,
    StateChangeCallback callback,
    void* user_data
) {
    if (!machine || !callback || machine->num_callbacks >= machine->max_callbacks) {
        return false;
    }
    
    machine->callbacks[machine->num_callbacks] = callback;
    machine->callback_data[machine->num_callbacks] = user_data;
    machine->num_callbacks++;
    
    return true;
}

bool state_machine_unregister_callback(
    StateMachine* machine,
    StateChangeCallback callback
) {
    if (!machine || !callback) return false;
    
    for (uint32_t i = 0; i < machine->num_callbacks; i++) {
        if (machine->callbacks[i] == callback) {
            // Shift remaining callbacks
            for (uint32_t j = i; j < machine->num_callbacks - 1; j++) {
                machine->callbacks[j] = machine->callbacks[j + 1];
                machine->callback_data[j] = machine->callback_data[j + 1];
            }
            machine->num_callbacks--;
            return true;
        }
    }
    
    return false;
}

// ============================================================================
// STATISTICS IMPLEMENTATION
// ============================================================================

uint64_t state_machine_get_state_duration(const StateMachine* machine) {
    if (!machine) return 0;
    
    uint64_t now = state_get_timestamp();
    return now - machine->state_entry_time;
}

uint64_t state_machine_get_total_state_duration(
    const StateMachine* machine,
    StateType state
) {
    if (!machine || state >= machine->num_states) return 0;
    
    uint64_t total = machine->state_durations[state];
    
    // Add current duration if in this state
    if (machine->current_state == state) {
        total += state_machine_get_state_duration(machine);
    }
    
    return total;
}

uint64_t state_machine_get_state_count(
    const StateMachine* machine,
    StateType state
) {
    if (!machine || state >= machine->num_states) return 0;
    return machine->state_counts[state];
}

uint64_t state_machine_get_transition_count(const StateMachine* machine) {
    if (!machine) return 0;
    return machine->total_transitions;
}

void state_machine_reset_statistics(StateMachine* machine) {
    if (!machine) return;
    
    memset(machine->state_durations, 0, machine->num_states * sizeof(uint64_t));
    memset(machine->state_counts, 0, machine->num_states * sizeof(uint64_t));
    machine->total_transitions = 0;
    machine->state_entry_time = state_get_timestamp();
}

// ============================================================================
// UTILITY IMPLEMENTATION
// ============================================================================

void state_machine_print(const StateMachine* machine) {
    if (!machine) return;
    
    printf("\n=== State Machine %u ===\n", machine->id);
    printf("Current State: %s (%d)\n", state_get_name(machine->current_state), machine->current_state);
    printf("Previous State: %s (%d)\n", state_get_name(machine->previous_state), machine->previous_state);
    printf("State Duration: %lu ns\n", state_machine_get_state_duration(machine));
    printf("Total Transitions: %lu\n", machine->total_transitions);
    printf("Registered Transitions: %u\n", machine->num_transitions);
    printf("Registered Callbacks: %u\n", machine->num_callbacks);
    printf("========================\n\n");
}

void state_manager_print_stats(const StateManager* manager) {
    if (!manager) return;
    
    printf("\n=== State Manager Statistics ===\n");
    printf("State Machines: %u / %u\n", manager->num_machines, manager->max_machines);
    printf("Registered States: %u / %u\n", manager->num_registered_states, manager->max_registered_states);
    printf("Total Transitions: %lu\n", manager->total_transitions);
    printf("Failed Transitions: %lu\n", manager->failed_transitions);
    printf("================================\n\n");
}