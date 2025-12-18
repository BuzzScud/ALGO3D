/**
 * State Manager Implementation
 */

#include "state_manager.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Global singleton instance
static StateManager* g_state_manager = NULL;

// ============================================================================
// STATE MANAGER CREATION
// ============================================================================

StateManager* state_manager_create(void) {
    StateManager* manager = calloc(1, sizeof(StateManager));
    if (!manager) return NULL;
    
    // Initialize model state
    manager->model.model = NULL;
    manager->model.model_loaded = false;
    manager->model.is_training = false;
    manager->model.is_inferencing = false;
    
    // Initialize training state
    manager->training.status = TRAINING_STATUS_IDLE;
    manager->training.current_epoch = 0;
    manager->training.total_epochs = 0;
    manager->training.current_batch = 0;
    manager->training.total_batches = 0;
    manager->training.progress_percentage = 0.0f;
    manager->training.current_loss = 0.0f;
    manager->training.best_loss = 1e9f;
    manager->training.batch_size = 32;  // Default batch size
    
    // Initialize UI state
    manager->ui.active_tab = 0;
    manager->ui.window_width = 1920;
    manager->ui.window_height = 1080;
    manager->ui.window_resized = false;
    manager->ui.dialog_open = false;
    
    return manager;
}

void state_manager_destroy(StateManager* manager) {
    if (!manager) return;
    free(manager);
}

StateManager* state_manager_get_instance(void) {
    if (!g_state_manager) {
        g_state_manager = state_manager_create();
    }
    return g_state_manager;
}

// ============================================================================
// MODEL STATE API
// ============================================================================

void state_set_model(StateManager* manager, CLLMModel* model, const char* name, const char* path) {
    if (!manager) return;
    
    manager->model.model = model;
    manager->model.model_loaded = (model != NULL);
    
    if (name) {
        strncpy(manager->model.model_name, name, sizeof(manager->model.model_name) - 1);
    }
    if (path) {
        strncpy(manager->model.model_path, path, sizeof(manager->model.model_path) - 1);
    }
    
    // Update configuration from model
    if (model) {
        manager->model.vocab_size = model->vocab_size;
        manager->model.embedding_dim = model->embedding_dim;
        manager->model.num_layers = model->num_layers;
        
        // Calculate total parameters (approximate)
        manager->model.total_parameters = 
            (uint64_t)model->vocab_size * model->embedding_dim +
            (uint64_t)model->num_weights;
    }
    
    // Notify callbacks
    if (manager->model.on_change) {
        manager->model.on_change(manager->model.user_data);
    }
    if (manager->on_any_change) {
        manager->on_any_change(manager->global_user_data);
    }
}

void state_clear_model(StateManager* manager) {
    if (!manager) return;
    
    manager->model.model = NULL;
    manager->model.model_loaded = false;
    manager->model.model_name[0] = '\0';
    manager->model.model_path[0] = '\0';
    
    // Notify callbacks
    if (manager->model.on_change) {
        manager->model.on_change(manager->model.user_data);
    }
    if (manager->on_any_change) {
        manager->on_any_change(manager->global_user_data);
    }
}

void state_update_model_config(StateManager* manager, uint32_t vocab_size, uint32_t embedding_dim,
                               uint32_t hidden_dim, uint32_t num_layers, uint32_t num_heads) {
    if (!manager) return;
    
    manager->model.vocab_size = vocab_size;
    manager->model.embedding_dim = embedding_dim;
    manager->model.hidden_dim = hidden_dim;
    manager->model.num_layers = num_layers;
    manager->model.num_heads = num_heads;
    
    // Notify callbacks
    if (manager->model.on_change) {
        manager->model.on_change(manager->model.user_data);
    }
    if (manager->on_any_change) {
        manager->on_any_change(manager->global_user_data);
    }
}

const ModelState* state_get_model(const StateManager* manager) {
    return manager ? &manager->model : NULL;
}

void state_register_model_callback(StateManager* manager, StateChangeCallback callback, void* user_data) {
    if (!manager) return;
    manager->model.on_change = callback;
    manager->model.user_data = user_data;
}

// ============================================================================
// TRAINING STATE API
// ============================================================================

void state_set_training_status(StateManager* manager, TrainingStatus status, const char* message) {
    if (!manager) return;
    
    manager->training.status = status;
    if (message) {
        strncpy(manager->training.status_message, message, sizeof(manager->training.status_message) - 1);
    }
    
    // Update model training flag
    manager->model.is_training = (status == TRAINING_STATUS_TRAINING);
    
    // Notify callbacks
    if (manager->training.on_change) {
        manager->training.on_change(manager->training.user_data);
    }
    if (manager->on_any_change) {
        manager->on_any_change(manager->global_user_data);
    }
}

void state_update_training_progress(StateManager* manager, uint32_t current_epoch, uint32_t total_epochs,
                                    uint32_t current_batch, uint32_t total_batches) {
    if (!manager) return;
    
    manager->training.current_epoch = current_epoch;
    manager->training.total_epochs = total_epochs;
    manager->training.current_batch = current_batch;
    manager->training.total_batches = total_batches;
    
    // Calculate progress percentage
    if (total_epochs > 0 && total_batches > 0) {
        float epoch_progress = (float)current_epoch / total_epochs;
        float batch_progress = (float)current_batch / total_batches;
        manager->training.progress_percentage = (epoch_progress + batch_progress / total_epochs) * 100.0f;
    }
    
    // Notify callbacks
    if (manager->training.on_change) {
        manager->training.on_change(manager->training.user_data);
    }
    if (manager->on_any_change) {
        manager->on_any_change(manager->global_user_data);
    }
}

void state_update_training_metrics(StateManager* manager, float current_loss, float best_loss,
                                   uint64_t tokens_processed, uint64_t batches_processed) {
    if (!manager) return;
    
    manager->training.current_loss = current_loss;
    manager->training.best_loss = best_loss;
    manager->training.tokens_processed = tokens_processed;
    manager->training.batches_processed = batches_processed;
    
    // Notify callbacks
    if (manager->training.on_change) {
        manager->training.on_change(manager->training.user_data);
    }
    if (manager->on_any_change) {
        manager->on_any_change(manager->global_user_data);
    }
}

void state_update_training_performance(StateManager* manager, float batches_per_second,
                                       float tokens_per_second, uint64_t time_elapsed,
                                       uint64_t time_remaining) {
    if (!manager) return;
    
    manager->training.batches_per_second = batches_per_second;
    manager->training.tokens_per_second = tokens_per_second;
    manager->training.time_elapsed_seconds = time_elapsed;
    manager->training.time_remaining_seconds = time_remaining;
    
    // Notify callbacks
    if (manager->training.on_change) {
        manager->training.on_change(manager->training.user_data);
    }
    if (manager->on_any_change) {
        manager->on_any_change(manager->global_user_data);
    }
}

void state_set_training_config(StateManager* manager, uint32_t batch_size, uint32_t num_batches,
                               uint32_t sequence_length, float learning_rate) {
    if (!manager) return;
    
    manager->training.batch_size = batch_size;
    manager->training.num_batches = num_batches;
    manager->training.sequence_length = sequence_length;
    manager->training.learning_rate = learning_rate;
    manager->training.initial_learning_rate = learning_rate;
    
    // Notify callbacks
    if (manager->training.on_change) {
        manager->training.on_change(manager->training.user_data);
    }
    if (manager->on_any_change) {
        manager->on_any_change(manager->global_user_data);
    }
}

void state_update_thread_info(StateManager* manager, uint32_t num_threads, uint32_t active_threads) {
    if (!manager) return;
    
    manager->training.num_threads = num_threads;
    manager->training.active_threads = active_threads;
    
    // Notify callbacks
    if (manager->training.on_change) {
        manager->training.on_change(manager->training.user_data);
    }
    if (manager->on_any_change) {
        manager->on_any_change(manager->global_user_data);
    }
}

const TrainingState* state_get_training(const StateManager* manager) {
    return manager ? &manager->training : NULL;
}

void state_register_training_callback(StateManager* manager, StateChangeCallback callback, void* user_data) {
    if (!manager) return;
    manager->training.on_change = callback;
    manager->training.user_data = user_data;
}

// ============================================================================
// UI STATE API
// ============================================================================

void state_set_active_tab(StateManager* manager, int tab_index) {
    if (!manager) return;
    
    manager->ui.active_tab = tab_index;
    
    // Notify callbacks
    if (manager->ui.on_change) {
        manager->ui.on_change(manager->ui.user_data);
    }
    if (manager->on_any_change) {
        manager->on_any_change(manager->global_user_data);
    }
}

void state_update_window_size(StateManager* manager, int width, int height) {
    if (!manager) return;
    
    manager->ui.window_width = width;
    manager->ui.window_height = height;
    manager->ui.window_resized = true;
    
    // Notify callbacks
    if (manager->ui.on_change) {
        manager->ui.on_change(manager->ui.user_data);
    }
    if (manager->on_any_change) {
        manager->on_any_change(manager->global_user_data);
    }
}

void state_set_focused_component(StateManager* manager, void* component) {
    if (!manager) return;
    
    manager->ui.focused_component = component;
    
    // Notify callbacks
    if (manager->ui.on_change) {
        manager->ui.on_change(manager->ui.user_data);
    }
    if (manager->on_any_change) {
        manager->on_any_change(manager->global_user_data);
    }
}

void state_set_dialog_open(StateManager* manager, bool open, void* dialog) {
    if (!manager) return;
    
    manager->ui.dialog_open = open;
    manager->ui.active_dialog = dialog;
    
    // Notify callbacks
    if (manager->ui.on_change) {
        manager->ui.on_change(manager->ui.user_data);
    }
    if (manager->on_any_change) {
        manager->on_any_change(manager->global_user_data);
    }
}

const AppUIState* state_get_ui(const StateManager* manager) {
    return manager ? &manager->ui : NULL;
}

void state_register_ui_callback(StateManager* manager, StateChangeCallback callback, void* user_data) {
    if (!manager) return;
    manager->ui.on_change = callback;
    manager->ui.user_data = user_data;
}

// ============================================================================
// GLOBAL CALLBACKS
// ============================================================================

void state_register_global_callback(StateManager* manager, StateChangeCallback callback, void* user_data) {
    if (!manager) return;
    manager->on_any_change = callback;
    manager->global_user_data = user_data;
}

// ============================================================================
// PERSISTENCE
// ============================================================================

bool state_save(const StateManager* manager, const char* filename) {
    if (!manager || !filename) return false;
    
    FILE* f = fopen(filename, "w");
    if (!f) return false;
    
    // Save model state
    fprintf(f, "[Model]\n");
    fprintf(f, "name=%s\n", manager->model.model_name);
    fprintf(f, "path=%s\n", manager->model.model_path);
    fprintf(f, "loaded=%d\n", manager->model.model_loaded);
    fprintf(f, "vocab_size=%u\n", manager->model.vocab_size);
    fprintf(f, "embedding_dim=%u\n", manager->model.embedding_dim);
    fprintf(f, "hidden_dim=%u\n", manager->model.hidden_dim);
    fprintf(f, "num_layers=%u\n", manager->model.num_layers);
    fprintf(f, "num_heads=%u\n", manager->model.num_heads);
    fprintf(f, "\n");
    
    // Save training config
    fprintf(f, "[Training]\n");
    fprintf(f, "batch_size=%u\n", manager->training.batch_size);
    fprintf(f, "num_batches=%u\n", manager->training.num_batches);
    fprintf(f, "sequence_length=%u\n", manager->training.sequence_length);
    fprintf(f, "learning_rate=%f\n", manager->training.learning_rate);
    fprintf(f, "\n");
    
    // Save UI state
    fprintf(f, "[UI]\n");
    fprintf(f, "active_tab=%d\n", manager->ui.active_tab);
    fprintf(f, "window_width=%d\n", manager->ui.window_width);
    fprintf(f, "window_height=%d\n", manager->ui.window_height);
    fprintf(f, "\n");
    
    fclose(f);
    return true;
}

bool state_load(StateManager* manager, const char* filename) {
    if (!manager || !filename) return false;
    
    FILE* f = fopen(filename, "r");
    if (!f) return false;
    
    char line[512];
    char section[64] = "";
    
    while (fgets(line, sizeof(line), f)) {
        // Remove newline
        line[strcspn(line, "\n")] = 0;
        
        // Skip empty lines
        if (line[0] == '\0') continue;
        
        // Check for section header
        if (line[0] == '[') {
            sscanf(line, "[%63[^]]]", section);
            continue;
        }
        
        // Parse key=value
        char key[64], value[256];
        if (sscanf(line, "%63[^=]=%255[^\n]", key, value) == 2) {
            if (strcmp(section, "Model") == 0) {
                if (strcmp(key, "name") == 0) {
                    strncpy(manager->model.model_name, value, sizeof(manager->model.model_name) - 1);
                } else if (strcmp(key, "path") == 0) {
                    strncpy(manager->model.model_path, value, sizeof(manager->model.model_path) - 1);
                } else if (strcmp(key, "vocab_size") == 0) {
                    manager->model.vocab_size = atoi(value);
                } else if (strcmp(key, "embedding_dim") == 0) {
                    manager->model.embedding_dim = atoi(value);
                } else if (strcmp(key, "hidden_dim") == 0) {
                    manager->model.hidden_dim = atoi(value);
                } else if (strcmp(key, "num_layers") == 0) {
                    manager->model.num_layers = atoi(value);
                } else if (strcmp(key, "num_heads") == 0) {
                    manager->model.num_heads = atoi(value);
                }
            } else if (strcmp(section, "Training") == 0) {
                if (strcmp(key, "batch_size") == 0) {
                    manager->training.batch_size = atoi(value);
                } else if (strcmp(key, "num_batches") == 0) {
                    manager->training.num_batches = atoi(value);
                } else if (strcmp(key, "sequence_length") == 0) {
                    manager->training.sequence_length = atoi(value);
                } else if (strcmp(key, "learning_rate") == 0) {
                    manager->training.learning_rate = atof(value);
                }
            } else if (strcmp(section, "UI") == 0) {
                if (strcmp(key, "active_tab") == 0) {
                    manager->ui.active_tab = atoi(value);
                } else if (strcmp(key, "window_width") == 0) {
                    manager->ui.window_width = atoi(value);
                } else if (strcmp(key, "window_height") == 0) {
                    manager->ui.window_height = atoi(value);
                }
            }
        }
    }
    
    fclose(f);
    return true;
}