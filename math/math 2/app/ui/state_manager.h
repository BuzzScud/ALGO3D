/**
 * Central State Manager for Crystalline CLLM UI
 * 
 * Provides single source of truth for application state with
 * automatic synchronization across all tabs
 */

#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include "cllm.h"

// Forward declarations
typedef struct StateManager StateManager;
typedef struct ModelState ModelState;
typedef struct TrainingState TrainingState;
typedef struct AppUIState AppUIState;

// State change callback
typedef void (*StateChangeCallback)(void* user_data);

// ============================================================================
// MODEL STATE
// ============================================================================

struct ModelState {
    // Current model
    CLLMModel* model;
    char model_name[256];
    char model_path[512];
    bool model_loaded;
    
    // Model configuration
    uint32_t vocab_size;
    uint32_t embedding_dim;
    uint32_t hidden_dim;
    uint32_t num_layers;
    uint32_t num_heads;
    uint32_t sequence_length;
    
    // Model status
    bool is_training;
    bool is_inferencing;
    uint64_t total_parameters;
    
    // Change notification
    StateChangeCallback on_change;
    void* user_data;
};

// ============================================================================
// TRAINING STATE
// ============================================================================

typedef enum {
    TRAINING_STATUS_IDLE,
    TRAINING_STATUS_LOADING_DATA,
    TRAINING_STATUS_TRAINING,
    TRAINING_STATUS_PAUSED,
    TRAINING_STATUS_COMPLETED,
    TRAINING_STATUS_ERROR
} TrainingStatus;

struct TrainingState {
    // Status
    TrainingStatus status;
    char status_message[256];
    
    // Progress
    uint32_t current_epoch;
    uint32_t total_epochs;
    uint32_t current_batch;
    uint32_t total_batches;
    float progress_percentage;
    
    // Metrics
    float current_loss;
    float best_loss;
    float learning_rate;
    uint64_t tokens_processed;
    uint64_t batches_processed;
    
    // Performance
    float batches_per_second;
    float tokens_per_second;
    uint64_t time_elapsed_seconds;
    uint64_t time_remaining_seconds;
    
    // Configuration
    uint32_t batch_size;
    uint32_t num_batches;
    uint32_t sequence_length;
    float initial_learning_rate;
    
    // Thread info
    uint32_t num_threads;
    uint32_t active_threads;
    
    // Change notification
    StateChangeCallback on_change;
    void* user_data;
};

// ============================================================================
// UI STATE
// ============================================================================

struct AppUIState {
    // Active tab
    int active_tab;
    
    // Window state
    int window_width;
    int window_height;
    bool window_resized;
    
    // Input focus
    void* focused_component;
    
    // Dialogs
    bool dialog_open;
    void* active_dialog;
    
    // Change notification
    StateChangeCallback on_change;
    void* user_data;
};

// ============================================================================
// STATE MANAGER
// ============================================================================

struct StateManager {
    ModelState model;
    TrainingState training;
    AppUIState ui;
    
    // Global callbacks
    StateChangeCallback on_any_change;
    void* global_user_data;
};

// ============================================================================
// STATE MANAGER API
// ============================================================================

/**
 * Create state manager
 */
StateManager* state_manager_create(void);

/**
 * Destroy state manager
 */
void state_manager_destroy(StateManager* manager);

/**
 * Get global state manager instance (singleton)
 */
StateManager* state_manager_get_instance(void);

// ============================================================================
// MODEL STATE API
// ============================================================================

/**
 * Set current model
 */
void state_set_model(StateManager* manager, CLLMModel* model, const char* name, const char* path);

/**
 * Clear current model
 */
void state_clear_model(StateManager* manager);

/**
 * Update model configuration
 */
void state_update_model_config(StateManager* manager, uint32_t vocab_size, uint32_t embedding_dim,
                               uint32_t hidden_dim, uint32_t num_layers, uint32_t num_heads);

/**
 * Get model state
 */
const ModelState* state_get_model(const StateManager* manager);

/**
 * Register model state change callback
 */
void state_register_model_callback(StateManager* manager, StateChangeCallback callback, void* user_data);

// ============================================================================
// TRAINING STATE API
// ============================================================================

/**
 * Set training status
 */
void state_set_training_status(StateManager* manager, TrainingStatus status, const char* message);

/**
 * Update training progress
 */
void state_update_training_progress(StateManager* manager, uint32_t current_epoch, uint32_t total_epochs,
                                    uint32_t current_batch, uint32_t total_batches);

/**
 * Update training metrics
 */
void state_update_training_metrics(StateManager* manager, float current_loss, float best_loss,
                                   uint64_t tokens_processed, uint64_t batches_processed);

/**
 * Update training performance
 */
void state_update_training_performance(StateManager* manager, float batches_per_second,
                                       float tokens_per_second, uint64_t time_elapsed,
                                       uint64_t time_remaining);

/**
 * Set training configuration
 */
void state_set_training_config(StateManager* manager, uint32_t batch_size, uint32_t num_batches,
                               uint32_t sequence_length, float learning_rate);

/**
 * Update thread info
 */
void state_update_thread_info(StateManager* manager, uint32_t num_threads, uint32_t active_threads);

/**
 * Get training state
 */
const TrainingState* state_get_training(const StateManager* manager);

/**
 * Register training state change callback
 */
void state_register_training_callback(StateManager* manager, StateChangeCallback callback, void* user_data);

// ============================================================================
// UI STATE API
// ============================================================================

/**
 * Set active tab
 */
void state_set_active_tab(StateManager* manager, int tab_index);

/**
 * Update window size
 */
void state_update_window_size(StateManager* manager, int width, int height);

/**
 * Set focused component
 */
void state_set_focused_component(StateManager* manager, void* component);

/**
 * Set dialog state
 */
void state_set_dialog_open(StateManager* manager, bool open, void* dialog);

/**
 * Get UI state
 */
const AppUIState* state_get_ui(const StateManager* manager);

/**
 * Register UI state change callback
 */
void state_register_ui_callback(StateManager* manager, StateChangeCallback callback, void* user_data);

// ============================================================================
// GLOBAL CALLBACKS
// ============================================================================

/**
 * Register global state change callback (called on ANY state change)
 */
void state_register_global_callback(StateManager* manager, StateChangeCallback callback, void* user_data);

// ============================================================================
// PERSISTENCE
// ============================================================================

/**
 * Save state to file
 */
bool state_save(const StateManager* manager, const char* filename);

/**
 * Load state from file
 */
bool state_load(StateManager* manager, const char* filename);

#endif // STATE_MANAGER_H