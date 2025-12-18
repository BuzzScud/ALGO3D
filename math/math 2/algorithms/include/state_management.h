/**
 * @file state_management.h
 * @brief Generic Hierarchical State Management System
 * 
 * A universal state management system with:
 * - Hierarchical state machine (10 standard states + user-defined)
 * - State transitions with validation
 * - Atomic state operations
 * - State change notifications
 * - Thread-safe state tracking
 * 
 * Design Philosophy:
 * - Atomic operations for thread safety
 * - Hierarchical state organization
 * - Extensible state definitions
 * - Event-driven notifications
 * - Minimal overhead
 */

#ifndef STATE_MANAGEMENT_H
#define STATE_MANAGEMENT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// STATE DEFINITIONS
// ============================================================================

/**
 * Standard state types (0-999 reserved)
 * User-defined states should start at 1000
 */
typedef enum {
    // Lifecycle states (0-99)
    STATE_UNINITIALIZED = 0,
    STATE_INITIALIZING = 1,
    STATE_INITIALIZED = 2,
    STATE_READY = 3,
    STATE_RUNNING = 4,
    STATE_PAUSED = 5,
    STATE_STOPPING = 6,
    STATE_STOPPED = 7,
    STATE_ERROR = 8,
    STATE_TERMINATED = 9,
    
    // Work states (100-199)
    STATE_IDLE = 100,
    STATE_WAITING = 101,
    STATE_WORKING = 102,
    STATE_BLOCKED = 103,
    STATE_YIELDING = 104,
    
    // Synchronization states (200-299)
    STATE_BARRIER_WAIT = 200,
    STATE_BARRIER_READY = 201,
    STATE_LOCK_WAIT = 202,
    STATE_LOCK_ACQUIRED = 203,
    
    // Custom states start here
    STATE_USER_DEFINED = 1000
} StateType;

/**
 * State transition result
 */
typedef enum {
    TRANSITION_SUCCESS = 0,
    TRANSITION_INVALID = 1,
    TRANSITION_BLOCKED = 2,
    TRANSITION_ERROR = 3
} TransitionResult;

/**
 * State flags
 */
typedef enum {
    STATE_FLAG_NONE = 0,
    STATE_FLAG_TERMINAL = (1 << 0),      // Cannot transition from this state
    STATE_FLAG_TRANSIENT = (1 << 1),     // Automatically transitions
    STATE_FLAG_CRITICAL = (1 << 2),      // Critical state (errors fatal)
    STATE_FLAG_PERSISTENT = (1 << 3)     // State persists across resets
} StateFlags;

// ============================================================================
// STATE STRUCTURES
// ============================================================================

/**
 * State information
 */
typedef struct StateInfo {
    StateType type;
    uint32_t flags;
    const char* name;
    const char* description;
} StateInfo;

/**
 * State transition rule
 */
typedef struct StateTransition {
    StateType from_state;
    StateType to_state;
    bool (*validator)(void* context);  // Optional validation function
    void (*callback)(void* context);   // Optional callback on transition
} StateTransition;

/**
 * State change event
 */
typedef struct StateChangeEvent {
    uint32_t entity_id;
    StateType old_state;
    StateType new_state;
    uint64_t timestamp;
    void* context;
} StateChangeEvent;

/**
 * State change callback
 */
typedef void (*StateChangeCallback)(const StateChangeEvent* event, void* user_data);

/**
 * State machine entity
 */
typedef struct StateMachine {
    uint32_t id;
    StateType current_state;
    StateType previous_state;
    uint64_t state_entry_time;
    uint64_t total_transitions;
    
    // Transition rules
    StateTransition* transitions;
    uint32_t num_transitions;
    uint32_t max_transitions;
    
    // Callbacks
    StateChangeCallback* callbacks;
    void** callback_data;
    uint32_t num_callbacks;
    uint32_t max_callbacks;
    
    // Statistics
    uint64_t* state_durations;  // Time spent in each state
    uint64_t* state_counts;     // Number of times in each state
    uint32_t num_states;
    
    // Context
    void* context;
} StateMachine;

/**
 * State manager (manages multiple state machines)
 */
typedef struct StateManager {
    StateMachine** machines;
    uint32_t num_machines;
    uint32_t max_machines;
    
    // Global state registry
    StateInfo* state_registry;
    uint32_t num_registered_states;
    uint32_t max_registered_states;
    
    // Statistics
    uint64_t total_transitions;
    uint64_t failed_transitions;
} StateManager;

// ============================================================================
// STATE MANAGER API
// ============================================================================

/**
 * Create a state manager
 */
StateManager* state_manager_create(uint32_t max_machines, uint32_t max_states);

/**
 * Destroy a state manager
 */
void state_manager_destroy(StateManager* manager);

/**
 * Register a state type
 */
bool state_manager_register_state(
    StateManager* manager,
    StateType type,
    const char* name,
    const char* description,
    uint32_t flags
);

/**
 * Get state information
 */
const StateInfo* state_manager_get_state_info(StateManager* manager, StateType type);

// ============================================================================
// STATE MACHINE API
// ============================================================================

/**
 * Create a state machine
 */
StateMachine* state_machine_create(
    StateManager* manager,
    uint32_t id,
    StateType initial_state,
    uint32_t max_transitions,
    uint32_t max_callbacks
);

/**
 * Destroy a state machine
 */
void state_machine_destroy(StateMachine* machine);

/**
 * Get current state
 */
StateType state_machine_get_state(const StateMachine* machine);

/**
 * Get previous state
 */
StateType state_machine_get_previous_state(const StateMachine* machine);

/**
 * Transition to a new state (with validation)
 */
TransitionResult state_machine_transition(
    StateMachine* machine,
    StateType new_state
);

/**
 * Force transition (bypass validation)
 */
bool state_machine_force_transition(
    StateMachine* machine,
    StateType new_state
);

/**
 * Check if transition is valid
 */
bool state_machine_can_transition(
    const StateMachine* machine,
    StateType new_state
);

// ============================================================================
// TRANSITION RULES API
// ============================================================================

/**
 * Add a transition rule
 */
bool state_machine_add_transition(
    StateMachine* machine,
    StateType from_state,
    StateType to_state,
    bool (*validator)(void* context),
    void (*callback)(void* context)
);

/**
 * Remove a transition rule
 */
bool state_machine_remove_transition(
    StateMachine* machine,
    StateType from_state,
    StateType to_state
);

/**
 * Clear all transition rules
 */
void state_machine_clear_transitions(StateMachine* machine);

// ============================================================================
// CALLBACK API
// ============================================================================

/**
 * Register a state change callback
 */
bool state_machine_register_callback(
    StateMachine* machine,
    StateChangeCallback callback,
    void* user_data
);

/**
 * Unregister a state change callback
 */
bool state_machine_unregister_callback(
    StateMachine* machine,
    StateChangeCallback callback
);

// ============================================================================
// STATISTICS API
// ============================================================================

/**
 * Get time spent in current state
 */
uint64_t state_machine_get_state_duration(const StateMachine* machine);

/**
 * Get total time spent in a specific state
 */
uint64_t state_machine_get_total_state_duration(
    const StateMachine* machine,
    StateType state
);

/**
 * Get number of times a state was entered
 */
uint64_t state_machine_get_state_count(
    const StateMachine* machine,
    StateType state
);

/**
 * Get total number of transitions
 */
uint64_t state_machine_get_transition_count(const StateMachine* machine);

/**
 * Reset statistics
 */
void state_machine_reset_statistics(StateMachine* machine);

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * Get state name
 */
const char* state_get_name(StateType state);

/**
 * Get transition result name
 */
const char* transition_result_name(TransitionResult result);

/**
 * Get current timestamp
 */
uint64_t state_get_timestamp(void);

/**
 * Print state machine information
 */
void state_machine_print(const StateMachine* machine);

/**
 * Print state manager statistics
 */
void state_manager_print_stats(const StateManager* manager);

#ifdef __cplusplus
}
#endif

#endif // STATE_MANAGEMENT_H