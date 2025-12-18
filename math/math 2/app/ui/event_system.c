/**
 * Event System Implementation
 */

#include "event_system.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

// Global singleton instance
static EventSystem* g_event_system = NULL;

// ============================================================================
// EVENT SYSTEM CREATION
// ============================================================================

EventSystem* event_system_create(void) {
    EventSystem* system = calloc(1, sizeof(EventSystem));
    if (!system) return NULL;
    
    // Initialize listener lists
    for (int i = 0; i < EVENT_TYPE_COUNT; i++) {
        system->listeners[i] = NULL;
    }
    
    // Initialize event queue
    system->queue_capacity = 256;
    system->event_queue = calloc(system->queue_capacity, sizeof(Event));
    system->queue_size = 0;
    system->processing = false;
    
    return system;
}

void event_system_destroy(EventSystem* system) {
    if (!system) return;
    
    // Free all listeners
    for (int i = 0; i < EVENT_TYPE_COUNT; i++) {
        EventListener* listener = system->listeners[i];
        while (listener) {
            EventListener* next = listener->next;
            free(listener);
            listener = next;
        }
    }
    
    // Free event queue
    for (int i = 0; i < system->queue_size; i++) {
        if (system->event_queue[i].data) {
            free(system->event_queue[i].data);
        }
    }
    free(system->event_queue);
    
    free(system);
}

EventSystem* event_system_get_instance(void) {
    if (!g_event_system) {
        g_event_system = event_system_create();
    }
    return g_event_system;
}

// ============================================================================
// EVENT REGISTRATION
// ============================================================================

int event_register(EventSystem* system, EventType type, EventCallback callback, void* user_data) {
    if (!system || !callback || type >= EVENT_TYPE_COUNT) return -1;
    
    // Create new listener
    EventListener* listener = calloc(1, sizeof(EventListener));
    if (!listener) return -1;
    
    listener->type = type;
    listener->callback = callback;
    listener->user_data = user_data;
    listener->once = false;
    
    // Add to front of list
    listener->next = system->listeners[type];
    system->listeners[type] = listener;
    
    return 0;
}

int event_register_once(EventSystem* system, EventType type, EventCallback callback, void* user_data) {
    if (!system || !callback || type >= EVENT_TYPE_COUNT) return -1;
    
    // Create new listener
    EventListener* listener = calloc(1, sizeof(EventListener));
    if (!listener) return -1;
    
    listener->type = type;
    listener->callback = callback;
    listener->user_data = user_data;
    listener->once = true;
    
    // Add to front of list
    listener->next = system->listeners[type];
    system->listeners[type] = listener;
    
    return 0;
}

void event_unregister(EventSystem* system, EventType type, EventCallback callback) {
    if (!system || !callback || type >= EVENT_TYPE_COUNT) return;
    
    EventListener** current = &system->listeners[type];
    
    while (*current) {
        if ((*current)->callback == callback) {
            EventListener* to_remove = *current;
            *current = (*current)->next;
            free(to_remove);
            return;
        }
        current = &(*current)->next;
    }
}

void event_unregister_all(EventSystem* system, EventType type) {
    if (!system || type >= EVENT_TYPE_COUNT) return;
    
    EventListener* listener = system->listeners[type];
    while (listener) {
        EventListener* next = listener->next;
        free(listener);
        listener = next;
    }
    
    system->listeners[type] = NULL;
}

// ============================================================================
// EVENT DISPATCH
// ============================================================================

void event_dispatch(EventSystem* system, EventType type, void* data, size_t data_size, const char* source) {
    if (!system || type >= EVENT_TYPE_COUNT) return;
    
    // Create event
    Event event = {
        .type = type,
        .priority = EVENT_PRIORITY_NORMAL,
        .timestamp = (uint64_t)time(NULL),
        .data = data,
        .data_size = data_size,
        .source = source
    };
    
    // Dispatch to all listeners
    EventListener** current = &system->listeners[type];
    
    while (*current) {
        EventListener* listener = *current;
        
        // Call callback
        listener->callback(&event, listener->user_data);
        
        // Remove if once
        if (listener->once) {
            *current = listener->next;
            free(listener);
        } else {
            current = &listener->next;
        }
    }
}

void event_queue(EventSystem* system, EventType type, EventPriority priority,
                void* data, size_t data_size, const char* source) {
    if (!system || type >= EVENT_TYPE_COUNT) return;
    
    // Resize queue if needed
    if (system->queue_size >= system->queue_capacity) {
        system->queue_capacity *= 2;
        system->event_queue = realloc(system->event_queue, 
                                     system->queue_capacity * sizeof(Event));
    }
    
    // Copy data if provided
    void* data_copy = NULL;
    if (data && data_size > 0) {
        data_copy = malloc(data_size);
        memcpy(data_copy, data, data_size);
    }
    
    // Add event to queue
    Event event = {
        .type = type,
        .priority = priority,
        .timestamp = (uint64_t)time(NULL),
        .data = data_copy,
        .data_size = data_size,
        .source = source
    };
    
    system->event_queue[system->queue_size++] = event;
}

void event_dispatch_custom(EventSystem* system, const char* name, void* data,
                           size_t data_size, const char* source) {
    if (!system || !name) return;
    
    Event event = {
        .type = EVENT_CUSTOM,
        .priority = EVENT_PRIORITY_NORMAL,
        .timestamp = (uint64_t)time(NULL),
        .data = data,
        .data_size = data_size,
        .source = source
    };
    
    strncpy(event.custom_name, name, sizeof(event.custom_name) - 1);
    
    // Dispatch to custom event listeners
    EventListener* listener = system->listeners[EVENT_CUSTOM];
    while (listener) {
        listener->callback(&event, listener->user_data);
        listener = listener->next;
    }
}

void event_process_queue(EventSystem* system) {
    if (!system || system->processing) return;
    
    system->processing = true;
    
    // Sort by priority (simple bubble sort for now)
    for (int i = 0; i < system->queue_size - 1; i++) {
        for (int j = 0; j < system->queue_size - i - 1; j++) {
            if (system->event_queue[j].priority < system->event_queue[j + 1].priority) {
                Event temp = system->event_queue[j];
                system->event_queue[j] = system->event_queue[j + 1];
                system->event_queue[j + 1] = temp;
            }
        }
    }
    
    // Process all events
    for (int i = 0; i < system->queue_size; i++) {
        Event* event = &system->event_queue[i];
        
        // Dispatch to listeners
        EventListener** current = &system->listeners[event->type];
        
        while (*current) {
            EventListener* listener = *current;
            
            // Call callback
            listener->callback(event, listener->user_data);
            
            // Remove if once
            if (listener->once) {
                *current = listener->next;
                free(listener);
            } else {
                current = &listener->next;
            }
        }
        
        // Free event data
        if (event->data) {
            free(event->data);
        }
    }
    
    // Clear queue
    system->queue_size = 0;
    system->processing = false;
}

void event_clear_queue(EventSystem* system) {
    if (!system) return;
    
    // Free all event data
    for (int i = 0; i < system->queue_size; i++) {
        if (system->event_queue[i].data) {
            free(system->event_queue[i].data);
        }
    }
    
    system->queue_size = 0;
}

// ============================================================================
// CONVENIENCE FUNCTIONS
// ============================================================================

void event_model_loaded(EventSystem* system, const char* model_name) {
    event_dispatch(system, EVENT_MODEL_LOADED, (void*)model_name, 
                  model_name ? strlen(model_name) + 1 : 0, "model_registry");
}

void event_model_created(EventSystem* system, const char* model_name) {
    event_dispatch(system, EVENT_MODEL_CREATED, (void*)model_name,
                  model_name ? strlen(model_name) + 1 : 0, "model_registry");
}

void event_model_saved(EventSystem* system, const char* model_path) {
    event_dispatch(system, EVENT_MODEL_SAVED, (void*)model_path,
                  model_path ? strlen(model_path) + 1 : 0, "model_registry");
}

void event_model_deleted(EventSystem* system, const char* model_name) {
    event_dispatch(system, EVENT_MODEL_DELETED, (void*)model_name,
                  model_name ? strlen(model_name) + 1 : 0, "model_registry");
}

void event_training_started(EventSystem* system) {
    event_dispatch(system, EVENT_TRAINING_STARTED, NULL, 0, "training_thread");
}

void event_training_stopped(EventSystem* system) {
    event_dispatch(system, EVENT_TRAINING_STOPPED, NULL, 0, "training_thread");
}

void event_training_progress(EventSystem* system, float progress) {
    event_dispatch(system, EVENT_TRAINING_PROGRESS_UPDATED, &progress, sizeof(float), "training_thread");
}

void event_tab_changed(EventSystem* system, int tab_index) {
    event_dispatch(system, EVENT_TAB_CHANGED, &tab_index, sizeof(int), "ui");
}

void event_window_resized(EventSystem* system, int width, int height) {
    int size[2] = {width, height};
    event_dispatch(system, EVENT_WINDOW_RESIZED, size, sizeof(size), "ui");
}

// ============================================================================
// DEBUGGING
// ============================================================================

const char* event_type_name(EventType type) {
    static const char* names[] = {
        "MODEL_LOADED",
        "MODEL_UNLOADED",
        "MODEL_CREATED",
        "MODEL_SAVED",
        "MODEL_DELETED",
        "MODEL_CONFIG_CHANGED",
        "TRAINING_STARTED",
        "TRAINING_STOPPED",
        "TRAINING_PAUSED",
        "TRAINING_RESUMED",
        "TRAINING_COMPLETED",
        "TRAINING_ERROR",
        "TRAINING_PROGRESS_UPDATED",
        "TRAINING_METRICS_UPDATED",
        "TAB_CHANGED",
        "WINDOW_RESIZED",
        "DIALOG_OPENED",
        "DIALOG_CLOSED",
        "COMPONENT_FOCUSED",
        "COMPONENT_UNFOCUSED",
        "DATA_LOADED",
        "DATA_SAVED",
        "DATA_DELETED",
        "DATA_UPDATED",
        "CUSTOM"
    };
    
    if (type >= 0 && type < EVENT_TYPE_COUNT) {
        return names[type];
    }
    return "UNKNOWN";
}

void event_print_stats(const EventSystem* system) {
    if (!system) return;
    
    printf("Event System Statistics:\n");
    printf("  Queue size: %d / %d\n", system->queue_size, system->queue_capacity);
    printf("  Processing: %s\n", system->processing ? "yes" : "no");
    printf("\nListeners by type:\n");
    
    for (int i = 0; i < EVENT_TYPE_COUNT; i++) {
        int count = 0;
        EventListener* listener = system->listeners[i];
        while (listener) {
            count++;
            listener = listener->next;
        }
        
        if (count > 0) {
            printf("  %s: %d listeners\n", event_type_name(i), count);
        }
    }
}