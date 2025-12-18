/**
 * Event System for Crystalline CLLM UI
 * 
 * Provides pub/sub pattern for cross-component and cross-tab communication
 */

#ifndef EVENT_SYSTEM_H
#define EVENT_SYSTEM_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Forward declarations
typedef struct EventSystem EventSystem;
typedef struct Event Event;

// Event types
typedef enum {
    // Model events
    EVENT_MODEL_LOADED,
    EVENT_MODEL_UNLOADED,
    EVENT_MODEL_CREATED,
    EVENT_MODEL_SAVED,
    EVENT_MODEL_DELETED,
    EVENT_MODEL_CONFIG_CHANGED,
    
    // Training events
    EVENT_TRAINING_STARTED,
    EVENT_TRAINING_STOPPED,
    EVENT_TRAINING_PAUSED,
    EVENT_TRAINING_RESUMED,
    EVENT_TRAINING_COMPLETED,
    EVENT_TRAINING_ERROR,
    EVENT_TRAINING_PROGRESS_UPDATED,
    EVENT_TRAINING_METRICS_UPDATED,
    
    // UI events
    EVENT_TAB_CHANGED,
    EVENT_WINDOW_RESIZED,
    EVENT_DIALOG_OPENED,
    EVENT_DIALOG_CLOSED,
    EVENT_COMPONENT_FOCUSED,
    EVENT_COMPONENT_UNFOCUSED,
    
    // Data events
    EVENT_DATA_LOADED,
    EVENT_DATA_SAVED,
    EVENT_DATA_DELETED,
    EVENT_DATA_UPDATED,
    
    // Custom events
    EVENT_CUSTOM,
    
    EVENT_TYPE_COUNT
} EventType;

// Event priority
typedef enum {
    EVENT_PRIORITY_LOW,
    EVENT_PRIORITY_NORMAL,
    EVENT_PRIORITY_HIGH,
    EVENT_PRIORITY_CRITICAL
} EventPriority;

// Event callback
typedef void (*EventCallback)(const Event* event, void* user_data);

// Event structure
struct Event {
    EventType type;
    EventPriority priority;
    uint64_t timestamp;
    
    // Event data (flexible)
    void* data;
    size_t data_size;
    
    // Source information
    const char* source;
    
    // Custom event name (for EVENT_CUSTOM)
    char custom_name[64];
};

// Event listener
typedef struct EventListener {
    EventType type;
    EventCallback callback;
    void* user_data;
    bool once;  // Remove after first trigger
    struct EventListener* next;
} EventListener;

// Event system
struct EventSystem {
    EventListener* listeners[EVENT_TYPE_COUNT];
    Event* event_queue;
    int queue_size;
    int queue_capacity;
    bool processing;
};

// ============================================================================
// EVENT SYSTEM API
// ============================================================================

/**
 * Create event system
 */
EventSystem* event_system_create(void);

/**
 * Destroy event system
 */
void event_system_destroy(EventSystem* system);

/**
 * Get global event system instance (singleton)
 */
EventSystem* event_system_get_instance(void);

// ============================================================================
// EVENT REGISTRATION
// ============================================================================

/**
 * Register event listener
 * Returns listener ID for later removal
 */
int event_register(EventSystem* system, EventType type, EventCallback callback, void* user_data);

/**
 * Register one-time event listener (auto-removes after first trigger)
 */
int event_register_once(EventSystem* system, EventType type, EventCallback callback, void* user_data);

/**
 * Unregister event listener
 */
void event_unregister(EventSystem* system, EventType type, EventCallback callback);

/**
 * Unregister all listeners for a type
 */
void event_unregister_all(EventSystem* system, EventType type);

// ============================================================================
// EVENT DISPATCH
// ============================================================================

/**
 * Dispatch event immediately (synchronous)
 */
void event_dispatch(EventSystem* system, EventType type, void* data, size_t data_size, const char* source);

/**
 * Queue event for later processing (asynchronous)
 */
void event_queue(EventSystem* system, EventType type, EventPriority priority, 
                void* data, size_t data_size, const char* source);

/**
 * Dispatch custom event
 */
void event_dispatch_custom(EventSystem* system, const char* name, void* data, 
                           size_t data_size, const char* source);

/**
 * Process queued events
 */
void event_process_queue(EventSystem* system);

/**
 * Clear event queue
 */
void event_clear_queue(EventSystem* system);

// ============================================================================
// CONVENIENCE FUNCTIONS
// ============================================================================

/**
 * Dispatch model events
 */
void event_model_loaded(EventSystem* system, const char* model_name);
void event_model_created(EventSystem* system, const char* model_name);
void event_model_saved(EventSystem* system, const char* model_path);
void event_model_deleted(EventSystem* system, const char* model_name);

/**
 * Dispatch training events
 */
void event_training_started(EventSystem* system);
void event_training_stopped(EventSystem* system);
void event_training_progress(EventSystem* system, float progress);

/**
 * Dispatch UI events
 */
void event_tab_changed(EventSystem* system, int tab_index);
void event_window_resized(EventSystem* system, int width, int height);

// ============================================================================
// DEBUGGING
// ============================================================================

/**
 * Get event type name
 */
const char* event_type_name(EventType type);

/**
 * Print event system statistics
 */
void event_print_stats(const EventSystem* system);

#endif // EVENT_SYSTEM_H