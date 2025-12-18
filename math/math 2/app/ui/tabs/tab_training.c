/**
 * Training Tab - Complete Redesign with Crystalline UI System
 * 
 * This is a COMPLETE REWRITE using the Crystalline UI system.
 * All functionality from the original is preserved but with a new interface.
 * 
 * Layout:
 * - Left Panel (61.8%): Visualization area with sphere and metrics
 * - Right Panel (38.2%): Controls with circular buttons and sliders
 * 
 * CRITICAL: Render content manually inside panels, don't rely entirely on Crystalline UI elements
 */

#include "../../app_common.h"
#include "../crystalline/elements.h"
#include "../crystalline/layout.h"
#include "../crystalline/draw.h"
#include "../crystalline/geometry.h"
#include "../sphere_visualization.h"
#include "../button_sizes.h"
#include "../../training_thread.h"
#include "../../time_format.h"
#include "cllm_training.h"
#include "cllm_training_threaded.h"
#include "cllm_model_registry.h"
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

// Training file structure
typedef struct {
    char filename[256];
    char filepath[512];
    bool selected;
    size_t size;
} TrainingFile;

// Training visualization data
typedef struct {
    float loss_history[100];
    int loss_count;
    int current_epoch;
    int total_epochs;
    float current_loss;
    float best_loss;
    float learning_rate;
    float batch_progress;
    
    // Entropy metrics
    float current_entropy;
    float target_entropy;
    float entropy_delta;
    
    // Hierarchy status
    bool hierarchy_active;
    int hierarchy_depth;
    int active_spheres;
    
    // Cymatic timing
    float cymatic_frequency;
    float resonance_strength;
    float phase_alignment;
} TrainingVisualization;

// Training Tab State - Owns its model independently
typedef struct {
    // Model ownership (OWNED BY THIS TAB)
    CLLMModel* model;                // NULL when not training
    char model_path[512];
    char model_name[256];
    bool model_loaded;
    
    // Training state (OWNED BY THIS TAB)
    CLLMTraining* training;          // NULL when not training
    pthread_t training_thread;
    bool is_training;
    bool should_stop;
    
    // Training statistics (UPDATED BY TRAINING THREAD)
    struct {
        int current_epoch;
        int total_epochs;
        float current_loss;
        float best_loss;
        uint64_t batches_processed;
        uint64_t tokens_processed;
        time_t training_start_time;
        time_t last_update_time;
        
        // Sphere statistics
        int active_spheres;
        uint64_t sphere_batches[12];
        float sphere_losses[12];
    } stats;
} TrainingTabState;

// UI State
static struct {
    // Panels
    CrystallinePanel* viz_panel;
    CrystallinePanel* control_panel;
    CrystallinePanel* metrics_panel;
    
    // Buttons (circular Crystalline UI style)
    CrystallineButton* btn_scan;
    CrystallineButton* btn_select;
    CrystallineButton* btn_start;
    CrystallineButton* btn_pause;
    CrystallineButton* btn_save;
    CrystallineButton* btn_load;
    CrystallineButton* btn_2d3d_toggle;
    
    // Sliders
    CrystallineSlider* slider_batch;
    CrystallineSlider* slider_sequence;
    CrystallineSlider* slider_epochs;
    CrystallineSlider* slider_lr;
    
    // Dropdown
    CrystallineDropdown* model_dropdown;
    
    // List
    CrystallineList* file_list;
    
    // Progress
    CrystallineProgress* training_progress;
    
    // Data
    TrainingFile files[100];
    int file_count;
    TrainingVisualization viz_data;
    
    // State
    bool initialized;
    char selected_model[256];
    
    // NEW: Training tab state (owns model independently)
    TrainingTabState tab_state;
    
} g_training_ui = {0};

/**
 * Scan training directory for files
 */
static void scan_training_directory(const char* dir_path) {
    printf("Scanning directory: %s\n", dir_path);
    DIR* dir = opendir(dir_path);
    if (!dir) {
        printf("ERROR: Could not open directory: %s\n", dir_path);
        return;
    }
    
    g_training_ui.file_count = 0;
    struct dirent* entry;
    
    while ((entry = readdir(dir)) != NULL && g_training_ui.file_count < 100) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        const char* ext = strrchr(entry->d_name, '.');
        if (ext && strcmp(ext, ".txt") == 0) {
            char full_path[512];
            snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
            
            struct stat st;
            if (stat(full_path, &st) == 0 && S_ISREG(st.st_mode)) {
                strncpy(g_training_ui.files[g_training_ui.file_count].filename, entry->d_name, 
                       sizeof(g_training_ui.files[g_training_ui.file_count].filename) - 1);
                strncpy(g_training_ui.files[g_training_ui.file_count].filepath, full_path,
                       sizeof(g_training_ui.files[g_training_ui.file_count].filepath) - 1);
                g_training_ui.files[g_training_ui.file_count].selected = false;
                g_training_ui.files[g_training_ui.file_count].size = st.st_size;
                
                g_training_ui.file_count++;
            }
        }
    }
    
    closedir(dir);
    printf("Found %d training files\n", g_training_ui.file_count);
    
    // Update file list UI
    if (g_training_ui.file_list) {
        char** items = malloc(g_training_ui.file_count * sizeof(char*));
        for (int i = 0; i < g_training_ui.file_count; i++) {
            items[i] = g_training_ui.files[i].filename;
        }
        crystalline_list_set_items(g_training_ui.file_list, items, g_training_ui.file_count);
        
        // Sync checkbox states
        for (int i = 0; i < g_training_ui.file_count; i++) {
            crystalline_list_set_item_checked(g_training_ui.file_list, i, g_training_ui.files[i].selected);
        }
        free(items);
    }
}

/**
 * Model Management Functions
 */

// Forward declarations
static void training_tab_unload_model(void);

/**
 * Load a model for training
 */
static bool training_tab_load_model(const char* model_name) {
    if (!model_name || !model_name[0]) {
        fprintf(stderr, "No model name provided\n");
        return false;
    }
    
    // Unload existing model if any
    training_tab_unload_model();
    
    // Build model path
    char model_path[512];
    extern bool model_registry_get_path(const char* name, char* path_out);
    if (!model_registry_get_path(model_name, model_path)) {
        fprintf(stderr, "Failed to get path for model: %s\n", model_name);
        return false;
    }
    
    // Load model
    printf("Loading model: %s\n", model_path);
    extern CLLMModel* cllm_read_model(const char* path);
    CLLMModel* model = cllm_read_model(model_path);
    if (!model) {
        fprintf(stderr, "Failed to load model: %s\n", model_path);
        return false;
    }
    
    // Store in tab state
    g_training_ui.tab_state.model = model;
    snprintf(g_training_ui.tab_state.model_path, 512, "%s", model_path);
    snprintf(g_training_ui.tab_state.model_name, 256, "%s", model_name);
    g_training_ui.tab_state.model_loaded = true;
    
    printf("✓ Model loaded: %s (%zu vocab, %zu dim, %u layers)\n",
           model_name, model->vocab_size, model->embedding_dim, model->num_layers);
    
    return true;
}

/**
 * Create a new model for training
 */
static bool training_tab_create_model(const char* model_name, const CLLMConfig* config) __attribute__((unused));
static bool training_tab_create_model(const char* model_name, const CLLMConfig* config) {
    if (!model_name || !config) {
        return false;
    }
    
    // Unload existing model if any
    training_tab_unload_model();
    
    // Create model
    printf("Creating model: %s\n", model_name);
    extern CLLMModel* cllm_create_model(const CLLMConfig* config);
    CLLMModel* model = cllm_create_model(config);
    if (!model) {
        fprintf(stderr, "Failed to create model\n");
        return false;
    }
    
    // Build model path
    char model_path[512];
    extern bool model_registry_get_path(const char* name, char* path_out);
    if (!model_registry_get_path(model_name, model_path)) {
        fprintf(stderr, "Failed to get path for model: %s\n", model_name);
        extern void cllm_free_model(CLLMModel* model);
        cllm_free_model(model);
        return false;
    }
    
    // Store in tab state
    g_training_ui.tab_state.model = model;
    snprintf(g_training_ui.tab_state.model_path, 512, "%s", model_path);
    snprintf(g_training_ui.tab_state.model_name, 256, "%s", model_name);
    g_training_ui.tab_state.model_loaded = true;
    
    // Save to disk
    extern int cllm_write_model(const CLLMModel* model, const char* path);
    if (cllm_write_model(model, model_path) != 0) {
        fprintf(stderr, "Warning: Failed to save model to disk\n");
    }
    
    // Register in registry
    extern bool model_registry_register(const char* name, const char* path);
    model_registry_register(model_name, model_path);
    
    printf("✓ Model created: %s\n", model_name);
    
    return true;
}

/**
 * Save the current model
 */
static bool training_tab_save_model(void) {
    if (!g_training_ui.tab_state.model_loaded || !g_training_ui.tab_state.model) {
        fprintf(stderr, "No model loaded\n");
        return false;
    }
    
    printf("Saving model: %s\n", g_training_ui.tab_state.model_path);
    
    extern int cllm_write_model(const CLLMModel* model, const char* path);
    if (cllm_write_model(g_training_ui.tab_state.model, g_training_ui.tab_state.model_path) != 0) {
        fprintf(stderr, "Failed to save model\n");
        return false;
    }
    
    // Refresh registry metadata
    extern bool model_registry_refresh(const char* name);
    model_registry_refresh(g_training_ui.tab_state.model_name);
    
    printf("✓ Model saved\n");
    return true;
}

// Forward declaration
static void training_tab_stop_training(void);

/**
 * Unload the current model
 */
static void training_tab_unload_model(void) {
    if (!g_training_ui.tab_state.model_loaded) {
        return;
    }
    
    // Stop training if active
    if (g_training_ui.tab_state.is_training) {
        training_tab_stop_training();
    }
    
    // Free model
    if (g_training_ui.tab_state.model) {
        printf("Unloading model: %s\n", g_training_ui.tab_state.model_name);
        extern void cllm_free_model(CLLMModel* model);
        cllm_free_model(g_training_ui.tab_state.model);
        g_training_ui.tab_state.model = NULL;
    }
    
    // Clear state
    g_training_ui.tab_state.model_loaded = false;
    g_training_ui.tab_state.model_name[0] = '\0';
    g_training_ui.tab_state.model_path[0] = '\0';
    
    printf("✓ Model unloaded\n");
}

/**
 * Training Functions
 */

/**
 * Training thread function
 */
static void* training_thread_func(void* arg) {
    TrainingTabState* tab = (TrainingTabState*)arg;
    
    printf("=== TRAINING THREAD STARTED ===\n");
    
    // Create batch iterator
    CLLMBatchIterator* batch_iterator = cllm_batch_iterator_create(
        tab->training->tokens,
        tab->training->num_tokens,
        tab->training->config.batch_size,
        tab->training->config.sequence_length,
        0,  // no shuffle
        0   // keep incomplete batches
    );
    
    if (!batch_iterator) {
        fprintf(stderr, "ERROR: Failed to create batch iterator\n");
        tab->is_training = false;
        return NULL;
    }
    
    // Create threaded training system
    ThreadedTrainingSystem* system = threaded_training_create(
        tab->training,
        batch_iterator,
        0  // auto-detect CPU cores
    );
    
    if (!system) {
        fprintf(stderr, "ERROR: Failed to create threaded training system\n");
        cllm_batch_iterator_free(batch_iterator);
        tab->is_training = false;
        return NULL;
    }
    
    printf("✓ Threaded training system created\n");
    
    // Training loop
    for (int epoch = 0; epoch < tab->stats.total_epochs && !tab->should_stop; epoch++) {
        tab->stats.current_epoch = epoch + 1;
        
        printf("Epoch %d/%d\n", epoch + 1, tab->stats.total_epochs);
        
        // Train one epoch using kissing spheres
        float epoch_loss = threaded_train_epoch_lockfree(system, epoch);
        
        // Update statistics
        tab->stats.current_loss = epoch_loss;
        if (epoch == 0 || epoch_loss < tab->stats.best_loss) {
            tab->stats.best_loss = epoch_loss;
        }
        tab->stats.last_update_time = time(NULL);
        
        printf("Epoch %d/%d - Loss: %.4f\n", 
               epoch + 1, tab->stats.total_epochs, epoch_loss);
        
        // Auto-save every 5 epochs
        if ((epoch + 1) % 5 == 0) {
            training_tab_save_model();
        }
    }
    
    // Final save
    training_tab_save_model();
    
    // Cleanup
    threaded_training_free(system);
    cllm_batch_iterator_free(batch_iterator);
    
    tab->is_training = false;
    printf("✓ Training thread completed\n");
    
    return NULL;
}

/**
 * Start training
 */
static bool training_tab_start_training(AppState* state) {
    if (!g_training_ui.tab_state.model_loaded) {
        fprintf(stderr, "No model loaded\n");
        return false;
    }
    
    if (g_training_ui.tab_state.is_training) {
        fprintf(stderr, "Training already in progress\n");
        return false;
    }
    
    // Create training configuration
    CLLMTrainingConfig config = {
        .num_epochs = state->training_epochs,
        .batch_size = state->training_batch_size,
        .sequence_length = state->training_sequence_length,
        .learning_rate = state->training_learning_rate,
        .weight_decay = 0.01f,
        .gradient_clip = 1.0f,
        .warmup_steps = 100,
        .save_every = 5,
        .eval_interval = 100,
        .max_steps = 10000,
        .gradient_accumulation_steps = 8,
        .use_mixed_precision = 1,
        .loss_scale = 1024.0f,
        .loss_scale_growth = 2.0f,
        .loss_scale_backoff = 0.5f,
        .loss_scale_window = 2000
    };
    strcpy(config.optimizer, "adam");
    
    // Initialize training
    g_training_ui.tab_state.training = cllm_training_init(
        g_training_ui.tab_state.model, 
        &config
    );
    
    if (!g_training_ui.tab_state.training) {
        fprintf(stderr, "Failed to initialize training\n");
        return false;
    }
    
    // Load training data
    extern int cllm_load_training_data(CLLMTraining* training, const char* filepath);
    for (int i = 0; i < g_training_ui.file_count; i++) {
        if (g_training_ui.files[i].selected) {
            cllm_load_training_data(
                g_training_ui.tab_state.training,
                g_training_ui.files[i].filepath
            );
        }
    }
    
    // Initialize statistics
    g_training_ui.tab_state.stats.current_epoch = 0;
    g_training_ui.tab_state.stats.total_epochs = config.num_epochs;
    g_training_ui.tab_state.stats.current_loss = 0.0f;
    g_training_ui.tab_state.stats.best_loss = 999999.0f;
    g_training_ui.tab_state.stats.batches_processed = 0;
    g_training_ui.tab_state.stats.tokens_processed = 0;
    g_training_ui.tab_state.stats.training_start_time = time(NULL);
    memset(g_training_ui.tab_state.stats.sphere_batches, 0, sizeof(g_training_ui.tab_state.stats.sphere_batches));
    memset(g_training_ui.tab_state.stats.sphere_losses, 0, sizeof(g_training_ui.tab_state.stats.sphere_losses));
    
    // Start training thread
    g_training_ui.tab_state.is_training = true;
    g_training_ui.tab_state.should_stop = false;
    
    if (pthread_create(&g_training_ui.tab_state.training_thread, NULL,
                      training_thread_func, &g_training_ui.tab_state) != 0) {
        fprintf(stderr, "Failed to create training thread\n");
        extern void cllm_training_free(CLLMTraining* training);
        cllm_training_free(g_training_ui.tab_state.training);
        g_training_ui.tab_state.training = NULL;
        g_training_ui.tab_state.is_training = false;
        return false;
    }
    
    printf("✓ Training started\n");
    return true;
}

/**
 * Stop training
 */
static void training_tab_stop_training(void) {
    if (!g_training_ui.tab_state.is_training) {
        return;
    }
    
    printf("Stopping training...\n");
    
    // Signal thread to stop
    g_training_ui.tab_state.should_stop = true;
    
    // Wait for thread to finish
    pthread_join(g_training_ui.tab_state.training_thread, NULL);
    
    // Cleanup training
    if (g_training_ui.tab_state.training) {
        extern void cllm_training_free(CLLMTraining* training);
        cllm_training_free(g_training_ui.tab_state.training);
        g_training_ui.tab_state.training = NULL;
    }
    
    g_training_ui.tab_state.is_training = false;
    
    printf("✓ Training stopped\n");
}

/**
 * Button callbacks
 */
static void on_scan_clicked(void* data) {
    AppState* state = (AppState*)data;
    if (!state) return;
    
    printf("=== SCAN BUTTON CLICKED ===\n");
    
    char training_dir[1024];
    extern void workspace_get_training_dir(AppState* state, char* output, size_t output_size);
    workspace_get_training_dir(state, training_dir, sizeof(training_dir));
    scan_training_directory(training_dir);
}

static void on_select_all_clicked(void* data) {
    (void)data;
    
    // Toggle: if all are selected, unselect all; otherwise select all
    bool all_selected = true;
    for (int i = 0; i < g_training_ui.file_count; i++) {
        if (!g_training_ui.files[i].selected) {
            all_selected = false;
            break;
        }
    }
    
    // Toggle state
    bool new_state = !all_selected;
    
    // Update internal array
    for (int i = 0; i < g_training_ui.file_count; i++) {
        g_training_ui.files[i].selected = new_state;
    }
    
    // Sync with CrystallineList checkboxes
    if (g_training_ui.file_list) {
        for (int i = 0; i < g_training_ui.file_count; i++) {
            crystalline_list_set_item_checked(g_training_ui.file_list, i, new_state);
        }
    }
    
    printf("SELECT button: %s all files\n", new_state ? "Selected" : "Unselected");
}

static void on_pause_clicked(void* data) {
    AppState* state = (AppState*)data;
    if (!state) return;
    
    // TODO: Implement pause functionality
    // For now, just log the click
    printf("PAUSE button clicked (functionality to be implemented)\n");
}

static void on_start_clicked(void* data) {
    AppState* state = (AppState*)data;
    if (!state) return;
    
    printf("=== START BUTTON CLICKED ===\n");
    
    if (g_training_ui.tab_state.is_training) {
        // Stop training
        printf("Stopping training...\n");
        training_tab_stop_training();
    } else {
        // Start training
        printf("Starting training...\n");
        
        // Load model if needed
        if (!g_training_ui.tab_state.model_loaded && g_training_ui.selected_model[0]) {
            printf("Loading selected model: '%s'\n", g_training_ui.selected_model);
            if (!training_tab_load_model(g_training_ui.selected_model)) {
                printf("ERROR: Failed to load model '%s'\n", g_training_ui.selected_model);
                return;
            }
        } else if (!g_training_ui.selected_model[0]) {
            printf("ERROR: No model selected! Please select a model from the dropdown first.\n");
            return;
        }
        
        // Start training
        if (!training_tab_start_training(state)) {
            printf("ERROR: Failed to start training\n");
        }
    }
}

static void on_save_clicked(void* data) {
    (void)data;  // Unused
    
    if (!g_training_ui.tab_state.model_loaded) {
        printf("ERROR: No model loaded to save\n");
        return;
    }
    
    printf("=== SAVE BUTTON CLICKED ===\n");
    
    if (training_tab_save_model()) {
        printf("✓ Model saved successfully\n");
    } else {
        printf("ERROR: Failed to save model\n");
    }
}

static void on_2d3d_toggle_clicked(void* data) {
    AppState* state = (AppState*)data;
    if (!state) return;
    
    printf("=== VISUALIZATION MODE TOGGLE CLICKED ===\n");
    printf("Current mode: %d\n", state->sphere_viz_mode);
    
    // Cycle through: 2D → 3D → CRYSTALLINE → 2D
    switch (state->sphere_viz_mode) {
        case SPHERE_VIZ_2D:
            state->sphere_viz_mode = SPHERE_VIZ_3D;
            break;
        case SPHERE_VIZ_3D:
            state->sphere_viz_mode = SPHERE_VIZ_CRYSTALLINE;
            break;
        case SPHERE_VIZ_CRYSTALLINE:
            state->sphere_viz_mode = SPHERE_VIZ_2D;
            break;
        default:
            state->sphere_viz_mode = SPHERE_VIZ_2D;
            break;
    }
    
    printf("New mode: %d\n", state->sphere_viz_mode);
}

/**
 * Slider callbacks
 */
static void on_batch_size_changed(float value, void* data) {
    AppState* state = (AppState*)data;
    if (!state) return;
    // PHASE 4: Batch size range 1-256 (was 1-32)
    state->training_batch_size = (int)(value * 255) + 1;  // 1-256
}

static void on_sequence_length_changed(float value, void* data) {
    AppState* state = (AppState*)data;
    if (!state) return;
    state->training_sequence_length = (int)(value * 480) + 32;  // 32-512
}

static void on_epochs_changed(float value, void* data) {
    AppState* state = (AppState*)data;
    if (!state) return;
    state->training_epochs = (int)(value * 99) + 1;  // 1-100
}

static void on_learning_rate_changed(float value, void* data) {
    AppState* state = (AppState*)data;
    if (!state) return;
    // Logarithmic scale: 0.0001 to 0.1
    float log_min = -4.0f;  // log10(0.0001)
    float log_max = -1.0f;  // log10(0.1)
    float log_lr = log_min + value * (log_max - log_min);
    state->training_learning_rate = powf(10.0f, log_lr);
}

/**
 * Model dropdown callback
 */
static void on_model_selected(int index, void* data) {
    printf("=== MODEL SELECTION CALLBACK CALLED: index=%d ===\n", index);
    AppState* state = (AppState*)data;
    if (!state || !g_training_ui.model_dropdown) return;
    
    // Get selected model name from model registry
    const ModelMetadata* metadata = model_registry_get_at_index((uint32_t)index);
    if (metadata) {
        strncpy(g_training_ui.selected_model, metadata->name, sizeof(g_training_ui.selected_model) - 1);
        g_training_ui.selected_model[sizeof(g_training_ui.selected_model) - 1] = '\0';
        printf("MODEL SELECTED: '%s' (index %d)\n", g_training_ui.selected_model, index);
        printf("  This model will be used when you click START\n");
    } else {
        printf("ERROR: Could not get model metadata for index %d\n", index);
    }
}

// File checkbox callback
static void on_file_checkbox_changed(int index, bool checked, void* user_data) {
    (void)user_data;
    if (index >= 0 && index < g_training_ui.file_count) {
        g_training_ui.files[index].selected = checked;
    }
}

/**
 * Initialize training tab
 */
void init_training_tab(AppState* state) {
    if (!state || g_training_ui.initialized) return;
    
    printf("Initializing Training Tab with Crystalline UI...\n");
    
    // Get font
    extern TTF_Font* get_global_font(void);
    TTF_Font* font = get_global_font();
    
    // Calculate layout using golden ratio
    // Use full available width INCLUDING control panel (training tab uses control panel space)
    int content_width = WINDOW_WIDTH - SIDEBAR_WIDTH;  // 1400px
    int viz_width = (int)(content_width * 0.618f);     // 865px (61.8%)
    int control_width = content_width - viz_width;      // 535px (38.2%)
    
    // Create visualization panel (left side)
    // IMPORTANT: Crystalline UI uses CENTER coordinates, not top-left
    float viz_panel_width = (float)(viz_width - 20);
    // PHASE 3: Reduce height by 130px to leave room for progress bar
    float viz_panel_height = (float)(WINDOW_HEIGHT - RENDER_OFFSET_Y - 150);  // Was -20, now -150
    g_training_ui.viz_panel = crystalline_panel_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        RENDER_OFFSET_X + 10 + viz_panel_width / 2.0f,  // CENTER X
        RENDER_OFFSET_Y + 10 + viz_panel_height / 2.0f, // CENTER Y
        viz_panel_width,
        viz_panel_height,
        "TRAINING",
        font
    );
    
    // Create control panel (right side)
    float control_panel_width = (float)(control_width - 30);
    float control_panel_height = (float)(WINDOW_HEIGHT - RENDER_OFFSET_Y - 20);
    g_training_ui.control_panel = crystalline_panel_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        RENDER_OFFSET_X + viz_width + 10 + control_panel_width / 2.0f,  // CENTER X
        RENDER_OFFSET_Y + 10 + control_panel_height / 2.0f,             // CENTER Y
        control_panel_width,
        control_panel_height,
        "CONTROLS",
        font
    );
    
    // Create metrics panel (inside viz panel, right side)
    // Using CENTER coordinates
    int metrics_x = RENDER_OFFSET_X + viz_width - 250;
    int metrics_y = RENDER_OFFSET_Y + 100;
    float metrics_panel_width = 230.0f;
    float metrics_panel_height = 400.0f;
    g_training_ui.metrics_panel = crystalline_panel_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        metrics_x + metrics_panel_width / 2.0f,  // CENTER X
        metrics_y + metrics_panel_height / 2.0f, // CENTER Y
        metrics_panel_width,
        metrics_panel_height,
        "STATUS",
        font
    );
    
    // Create circular buttons (Crystalline UI style)
    int btn_x = RENDER_OFFSET_X + viz_width + control_width / 2;  // MOVED to control panel
    int btn_y = WINDOW_HEIGHT - 80;  // Bottom of control panel
    
    printf("=== BUTTON POSITIONS ===\n");
    printf("btn_x = %d, btn_y = %d\n", btn_x, btn_y);
    printf("PAUSE: x=%d, y=%d\n", btn_x - 100, btn_y);
    printf("START: x=%d, y=%d\n", btn_x, btn_y);
    printf("SAVE: x=%d, y=%d\n", btn_x + 100, btn_y);
    
    g_training_ui.btn_pause = crystalline_button_create(
        CRYSTALLINE_STYLE_CIRCULAR,
        btn_x - 100, btn_y,
        BUTTON_RADIUS_TERTIARY, 0,
        "PAUSE",
        font
    );
    crystalline_button_set_callback(g_training_ui.btn_pause, on_pause_clicked, state);
    
    g_training_ui.btn_start = crystalline_button_create(
        CRYSTALLINE_STYLE_CIRCULAR,
        btn_x, btn_y,
        BUTTON_RADIUS_PRIMARY, 0,
        "START",
        font
    );
    crystalline_button_set_callback(g_training_ui.btn_start, on_start_clicked, state);
    
    g_training_ui.btn_save = crystalline_button_create(
        CRYSTALLINE_STYLE_CIRCULAR,
        btn_x + 80, btn_y,
        BUTTON_RADIUS_TERTIARY, 0,
        "SAVE",
        font
    );
    crystalline_button_set_callback(g_training_ui.btn_save, on_save_clicked, state);
    
    // Create buttons in control panel
    int ctrl_x = RENDER_OFFSET_X + viz_width + control_width / 2;
    int ctrl_y = RENDER_OFFSET_Y + 380;
    
    printf("SCAN: x=%d, y=%d\n", ctrl_x - 50, ctrl_y);
    printf("SELECT: x=%d, y=%d\n", ctrl_x + 50, ctrl_y);
    
    g_training_ui.btn_scan = crystalline_button_create(
        CRYSTALLINE_STYLE_CIRCULAR,
        ctrl_x - 50, ctrl_y,
        BUTTON_RADIUS_SECONDARY, 0,
        "SCAN",
        font
    );
    crystalline_button_set_callback(g_training_ui.btn_scan, on_scan_clicked, state);
    
    g_training_ui.btn_select = crystalline_button_create(
        CRYSTALLINE_STYLE_CIRCULAR,
        ctrl_x + 50, ctrl_y,
        BUTTON_RADIUS_SECONDARY, 0,
        "SELECT",
        font
    );
    crystalline_button_set_callback(g_training_ui.btn_select, on_select_all_clicked, state);
    
    // Create 2D/3D toggle button (small rectangular button, using CENTER coordinates)
    float toggle_width = (float)BUTTON_RECT_WIDTH_SMALL;
    float toggle_height = (float)BUTTON_RECT_HEIGHT;
    g_training_ui.btn_2d3d_toggle = crystalline_button_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        metrics_x + 115 + toggle_width / 2.0f,
        metrics_y - 40 + toggle_height / 2.0f,
        toggle_width, 
        toggle_height,
        "2D/3D",
        font
    );
    crystalline_button_set_callback(g_training_ui.btn_2d3d_toggle, on_2d3d_toggle_clicked, state);
    
    // Create sliders (rectangular style for better precision)
    // Using CENTER coordinates
    int slider_x = RENDER_OFFSET_X + viz_width + 20;
    int slider_w = control_width - 60;
    int slider_y = RENDER_OFFSET_Y + 150;
    float slider_center_x = slider_x + slider_w / 2.0f;
    float slider_height = (float)SLIDER_TRACK_HEIGHT;
    
    g_training_ui.slider_batch = crystalline_slider_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        slider_center_x, slider_y + slider_height / 2.0f,
        slider_w, slider_height,
        1, 256
    );
    crystalline_slider_set_callback(g_training_ui.slider_batch, on_batch_size_changed, state);
    
    g_training_ui.slider_sequence = crystalline_slider_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        slider_center_x, slider_y + 60 + slider_height / 2.0f,
        slider_w, slider_height,
        32, 512
    );
    crystalline_slider_set_callback(g_training_ui.slider_sequence, on_sequence_length_changed, state);
    
    g_training_ui.slider_epochs = crystalline_slider_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        slider_center_x, slider_y + 120 + slider_height / 2.0f,
        slider_w, slider_height,
        1, 100
    );
    crystalline_slider_set_callback(g_training_ui.slider_epochs, on_epochs_changed, state);
    
    g_training_ui.slider_lr = crystalline_slider_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        slider_center_x, slider_y + 180 + slider_height / 2.0f,
        slider_w, slider_height,
        0.0001f, 0.1f
    );
    crystalline_slider_set_callback(g_training_ui.slider_lr, on_learning_rate_changed, state);
    
    // Create model dropdown (using CENTER coordinates)
    float dropdown_height = 40.0f;
    g_training_ui.model_dropdown = crystalline_dropdown_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        slider_center_x, RENDER_OFFSET_Y + 60 + dropdown_height / 2.0f,
        slider_w, dropdown_height,
        font
    );
    crystalline_dropdown_set_callback(g_training_ui.model_dropdown, on_model_selected, state);
    
       // NOTE: Dropdown will be populated in draw_training_tab() after model_manager initializes
    
    // Create file list (using CENTER coordinates)
    // Position below buttons with proper spacing
    float list_height = 200.0f;
    float item_height = 25.0f;  // Height per item (NOT total list height)
    float list_y = RENDER_OFFSET_Y + 520 + list_height / 2.0f;  // Moved further down
    g_training_ui.file_list = crystalline_list_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        slider_center_x, list_y,
        slider_w, item_height,  // Pass item height, not total height
        font
    );
    crystalline_list_enable_checkboxes(g_training_ui.file_list, true);
    crystalline_list_set_check_callback(g_training_ui.file_list, on_file_checkbox_changed, state);
    
    // Create progress bar (using CENTER coordinates)
    float progress_width = (float)(viz_width - 40);
    float progress_height = 20.0f;  // Reduced from 30 to 20
    // PHASE 3: Position 20px below sphere visualization
    int viz_height = WINDOW_HEIGHT - RENDER_OFFSET_Y - 150;  // Match draw function
    float viz_bottom = RENDER_OFFSET_Y + viz_height;
    g_training_ui.training_progress = crystalline_progress_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        RENDER_OFFSET_X + 20 + progress_width / 2.0f,
        viz_bottom + 20.0f + progress_height / 2.0f,  // 20px below viz, centered
        progress_width,
        progress_height
    );
    
    // Scan initial directory
    char training_dir[1024];
    extern void workspace_get_training_dir(AppState* state, char* output, size_t output_size);
    workspace_get_training_dir(state, training_dir, sizeof(training_dir));
    scan_training_directory(training_dir);
    
    g_training_ui.initialized = true;
    printf("✓ Training Tab initialized with Crystalline UI\n");
}

/**
 * Update visualization data from training state
 */
void update_training_visualization(AppState* state) {
    if (!state) return;
    
    // CRITICAL: Check if training is still active before accessing metrics
    extern bool is_training_thread_active(void);
    if (!is_training_thread_active()) {
        // Training stopped - reset to clean inactive state
        // Keep visualization visible but show as inactive
        g_training_ui.viz_data.current_epoch = 0;
        g_training_ui.viz_data.total_epochs = 0;
        g_training_ui.viz_data.current_loss = 0.0f;
        g_training_ui.viz_data.learning_rate = 0.0f;
        g_training_ui.viz_data.batch_progress = 0.0f;
        
        // Reset entropy metrics
        g_training_ui.viz_data.current_entropy = 0.0f;
        g_training_ui.viz_data.target_entropy = 0.0f;
        g_training_ui.viz_data.entropy_delta = 0.0f;
        
        // Reset hierarchy status
        g_training_ui.viz_data.hierarchy_active = false;
        g_training_ui.viz_data.hierarchy_depth = 0;
        g_training_ui.viz_data.active_spheres = 0;
        
        // Reset cymatic timing
        g_training_ui.viz_data.cymatic_frequency = 0.0f;
        g_training_ui.viz_data.resonance_strength = 0.0f;
        g_training_ui.viz_data.phase_alignment = 0.0f;
        
        return;  // Don't access freed memory
    }
    
    // Update from training tab state
    if (g_training_ui.tab_state.is_training) {
        g_training_ui.viz_data.current_epoch = g_training_ui.tab_state.stats.current_epoch;
        g_training_ui.viz_data.total_epochs = g_training_ui.tab_state.stats.total_epochs;
        g_training_ui.viz_data.current_loss = g_training_ui.tab_state.stats.current_loss;
        g_training_ui.viz_data.best_loss = g_training_ui.tab_state.stats.best_loss;
        
        // Add to loss history
        if (g_training_ui.viz_data.loss_count < 100) {
            g_training_ui.viz_data.loss_history[g_training_ui.viz_data.loss_count++] = 
                g_training_ui.viz_data.current_loss;
        }
        
        // Update progress bar
        if (g_training_ui.training_progress && g_training_ui.viz_data.total_epochs > 0) {
            float progress = (float)g_training_ui.viz_data.current_epoch / 
                           g_training_ui.viz_data.total_epochs;
            crystalline_progress_set_value(g_training_ui.training_progress, progress);
        }
    }
}

/**
 * Draw training tab
 */
void draw_training_tab(SDL_Renderer* renderer, AppState* state) {
    if (!state) return;
    
    // Initialize on first draw
    if (!g_training_ui.initialized) {
        init_training_tab(state);
    }
    
    // Populate model dropdown once (registry already scanned at startup)
    static bool dropdown_populated = false;
    if (!dropdown_populated && g_training_ui.model_dropdown) {
        uint32_t model_count = model_registry_count();
        
        if (model_count > 0) {
            char** model_names = malloc(model_count * sizeof(char*));
            if (model_names) {
                for (uint32_t i = 0; i < model_count; i++) {
                    const ModelMetadata* metadata = model_registry_get_at_index(i);
                    model_names[i] = metadata ? (char*)metadata->name : "";
                }
                crystalline_dropdown_set_options(g_training_ui.model_dropdown, model_names, (int)model_count);
                printf("TRAINING MODEL DROPDOWN: Populated with %u models\n", model_count);
                free(model_names);
                dropdown_populated = true;
            }
        }
    }
    
    // Update visualization data
    update_training_visualization(state);
    
    // Render panels
    if (g_training_ui.viz_panel) {
        crystalline_panel_render(g_training_ui.viz_panel, renderer);
    }
    
    if (g_training_ui.control_panel) {
        crystalline_panel_render(g_training_ui.control_panel, renderer);
    }
    
    if (g_training_ui.metrics_panel) {
        crystalline_panel_render(g_training_ui.metrics_panel, renderer);
    }
    
    // Draw training metrics text at top
    extern void draw_text(SDL_Renderer* renderer, const char* text, int x, int y, SDL_Color color);
    SDL_Color text_color = {220, 220, 220, 255};
    
    (void)text_color;  // May be used later
    // REMOVED: Status bar at top was overlapped by sphere visualization
    // Epoch/loss metrics now displayed in Framework Status panel (right side)
    
    // Draw sphere visualization using layout helper
    // CRITICAL: Use same calculation as init_training_tab()
    int content_width = WINDOW_WIDTH - SIDEBAR_WIDTH;  // 1400px
    int viz_width = (int)(content_width * 0.618f);     // 865px (61.8%)
    // PHASE 3: Reduce height by 130px to leave room for progress bar below
    int viz_height = WINDOW_HEIGHT - RENDER_OFFSET_Y - 150;  // Was -20, now -150 (130px less)
    
    // Use layout helper to calculate proper visualization area
    SDL_Rect sphere_bounds = crystalline_layout_viz_area(
        RENDER_OFFSET_X,
        RENDER_OFFSET_Y,
        viz_width,
        viz_height,
        230,  // metrics panel width (matches metrics_panel_width)
        20    // spacing
    );
    
    draw_sphere_visualization(renderer, state, sphere_bounds);
    
    // Render progress bar
    if (g_training_ui.training_progress) {
        crystalline_progress_render(g_training_ui.training_progress, renderer);
    }
    
    // Update visualization mode toggle button label
    if (g_training_ui.btn_2d3d_toggle) {
        const char* toggle_label;
        switch (state->sphere_viz_mode) {
            case SPHERE_VIZ_2D:
                toggle_label = "3D";  // Shows what clicking will switch TO
                break;
            case SPHERE_VIZ_3D:
                toggle_label = "CRYS";  // Crystalline
                break;
            case SPHERE_VIZ_CRYSTALLINE:
                toggle_label = "2D";
                break;
            default:
                toggle_label = "2D";
                break;
        }
        crystalline_button_set_label(g_training_ui.btn_2d3d_toggle, toggle_label);
    }
    
    // Render sliders FIRST (background layer)
    // Note: Sliders use CENTER coordinates, but labels use LEFT edge
    // Calculate layout dimensions (same as init)
    int content_width_full = WINDOW_WIDTH - SIDEBAR_WIDTH;  // 1400px
    int viz_width_full = (int)(content_width_full * 0.618f);
    
    int slider_x = RENDER_OFFSET_X + viz_width_full + 20;
    int slider_y = RENDER_OFFSET_Y + 150;
    char label[64];
    
    // Batch Size (PHASE 4: Range 1-256)
    snprintf(label, sizeof(label), "Batch Size: %d (1-256)", state->training_batch_size);
    draw_text(renderer, label, slider_x, slider_y - SLIDER_LABEL_SPACING, text_color);
    if (g_training_ui.slider_batch) crystalline_slider_render(g_training_ui.slider_batch, renderer);
    
    // Sequence Length
    snprintf(label, sizeof(label), "Sequence Length: %d", state->training_sequence_length);
    draw_text(renderer, label, slider_x, slider_y + 60 - SLIDER_LABEL_SPACING, text_color);
    if (g_training_ui.slider_sequence) crystalline_slider_render(g_training_ui.slider_sequence, renderer);
    
    // Epochs
    snprintf(label, sizeof(label), "Epochs: %d", state->training_epochs);
    draw_text(renderer, label, slider_x, slider_y + 120 - SLIDER_LABEL_SPACING, text_color);
    if (g_training_ui.slider_epochs) crystalline_slider_render(g_training_ui.slider_epochs, renderer);
    
    // Learning Rate
    snprintf(label, sizeof(label), "Learning Rate: %.4f", state->training_learning_rate);
    draw_text(renderer, label, slider_x, slider_y + 180 - SLIDER_LABEL_SPACING, text_color);
    if (g_training_ui.slider_lr) crystalline_slider_render(g_training_ui.slider_lr, renderer);
    
    // Render model dropdown
    draw_text(renderer, "Model:", slider_x, RENDER_OFFSET_Y + 40, text_color);
    if (g_training_ui.model_dropdown) crystalline_dropdown_render(g_training_ui.model_dropdown, renderer);
    
    // Render file list using Crystalline UI
    char file_list_label[128];
    snprintf(file_list_label, sizeof(file_list_label), "Training Files (%d):", g_training_ui.file_count);
    draw_text(renderer, file_list_label, slider_x, RENDER_OFFSET_Y + 490, text_color);  // Position label above list
    
    if (g_training_ui.file_list) {
        crystalline_list_render(g_training_ui.file_list, renderer);
    }
    
    // NOW render buttons on TOP (so they're clickable and not covered)
    if (g_training_ui.btn_pause) crystalline_button_render(g_training_ui.btn_pause, renderer);
    if (g_training_ui.btn_start) crystalline_button_render(g_training_ui.btn_start, renderer);
    if (g_training_ui.btn_save) crystalline_button_render(g_training_ui.btn_save, renderer);
    if (g_training_ui.btn_scan) crystalline_button_render(g_training_ui.btn_scan, renderer);
    if (g_training_ui.btn_select) crystalline_button_render(g_training_ui.btn_select, renderer);
    if (g_training_ui.btn_2d3d_toggle) crystalline_button_render(g_training_ui.btn_2d3d_toggle, renderer);

    
    // Draw metrics panel content INSIDE the STATUS panel
    // Panel was created at: metrics_x = RENDER_OFFSET_X + viz_width - 250
    //                       metrics_y = RENDER_OFFSET_Y + 100
    // These are TOP-LEFT coordinates for text positioning
    int metrics_text_x = RENDER_OFFSET_X + viz_width_full - 250 + 10;  // Panel left + padding
    int metrics_text_y = RENDER_OFFSET_Y + 100 + 30;  // Panel top + padding for title
    
    draw_text(renderer, "FRAMEWORK STATUS", metrics_text_x, metrics_text_y, (SDL_Color){100, 150, 200, 255});
    metrics_text_y += 25;
    
    // PHASE 1: Add epoch/loss metrics here (moved from overlapping top status bar)
    char epoch_loss[128];
    snprintf(epoch_loss, sizeof(epoch_loss), "Epoch: %d / %d",
            g_training_ui.viz_data.current_epoch, g_training_ui.viz_data.total_epochs);
    draw_text(renderer, epoch_loss, metrics_text_x, metrics_text_y, text_color);
    metrics_text_y += 18;
    
    snprintf(epoch_loss, sizeof(epoch_loss), "Loss: %.4f", g_training_ui.viz_data.current_loss);
    draw_text(renderer, epoch_loss, metrics_text_x, metrics_text_y, text_color);
    metrics_text_y += 18;
    
    snprintf(epoch_loss, sizeof(epoch_loss), "Best: %.4f", g_training_ui.viz_data.best_loss);
    draw_text(renderer, epoch_loss, metrics_text_x, metrics_text_y, text_color);
    metrics_text_y += 25;
    
    draw_text(renderer, "Lattice Embeddings: ACTIVE", metrics_text_x, metrics_text_y, (SDL_Color){100, 255, 100, 255});
    metrics_text_y += 18;
    draw_text(renderer, "Angular Attention: ACTIVE", metrics_text_x, metrics_text_y, (SDL_Color){100, 255, 100, 255});
    metrics_text_y += 18;
    draw_text(renderer, "Crystalline Loss: ACTIVE", metrics_text_x, metrics_text_y, (SDL_Color){100, 255, 100, 255});
    metrics_text_y += 25;
    
    draw_text(renderer, "PERFORMANCE", metrics_text_x, metrics_text_y, (SDL_Color){100, 150, 200, 255});
    metrics_text_y += 25;
    
    char perf[128];
    snprintf(perf, sizeof(perf), "Active Threads: %d", g_training_ui.tab_state.stats.active_spheres);
    draw_text(renderer, perf, metrics_text_x, metrics_text_y, text_color);
    metrics_text_y += 18;
    
    snprintf(perf, sizeof(perf), "Total Batches: %llu", 
             (unsigned long long)g_training_ui.tab_state.stats.batches_processed);
    draw_text(renderer, perf, metrics_text_x, metrics_text_y, text_color);
    metrics_text_y += 18;
    
    snprintf(perf, sizeof(perf), "Tokens Processed: %llu", 
             (unsigned long long)g_training_ui.tab_state.stats.tokens_processed);
    draw_text(renderer, perf, metrics_text_x, metrics_text_y, text_color);
    metrics_text_y += 25;
    
    // PHASE 6: Display entropy metrics if available
    extern void* get_training_system(void);  // From training_thread.c
    void* training_system = get_training_system();
    
    if (training_system) {
        // Get entropy context
        void* entropy_ctx = threaded_training_get_entropy_context((ThreadedTrainingSystem*)training_system);
        
        if (entropy_ctx) {
            // Get model entropy stats
            extern const void* get_model_entropy_stats(const void* ctx);
            const void* model_stats = get_model_entropy_stats(entropy_ctx);
            
            if (model_stats) {
                draw_text(renderer, "ENTROPY METRICS", metrics_text_x, metrics_text_y, (SDL_Color){100, 150, 200, 255});
                metrics_text_y += 25;
                
                // Cast to access the structure
                typedef struct {
                    void* dimensions[12];
                    double total_entropy;
                    double normalized_entropy;
                    uint64_t update_count;
                    double entropy_trend;
                    double last_total_entropy;
                } ModelEntropyStats;
                
                const ModelEntropyStats* stats = (const ModelEntropyStats*)model_stats;
                
                char entropy_text[128];
                snprintf(entropy_text, sizeof(entropy_text), "Total: %.4f", stats->total_entropy);
                draw_text(renderer, entropy_text, metrics_text_x, metrics_text_y, text_color);
                metrics_text_y += 18;
                
                snprintf(entropy_text, sizeof(entropy_text), "Normalized: %.4f", stats->normalized_entropy);
                draw_text(renderer, entropy_text, metrics_text_x, metrics_text_y, text_color);
                metrics_text_y += 18;
                
                snprintf(entropy_text, sizeof(entropy_text), "Trend: %+.4f", stats->entropy_trend);
                SDL_Color trend_color = (stats->entropy_trend > 0) ? 
                    (SDL_Color){255, 150, 100, 255} : (SDL_Color){100, 255, 150, 255};
                draw_text(renderer, entropy_text, metrics_text_x, metrics_text_y, trend_color);
            }
        }
        
        // Get adaptive hierarchy context
        void* hierarchy_ctx = threaded_training_get_adaptive_hierarchy((ThreadedTrainingSystem*)training_system);
        
        if (hierarchy_ctx) {
            metrics_text_y += 25;
            draw_text(renderer, "ADAPTIVE HIERARCHY", metrics_text_x, metrics_text_y, (SDL_Color){100, 150, 200, 255});
            metrics_text_y += 25;
            
            // Display hierarchy status
            draw_text(renderer, "Status: ACTIVE", metrics_text_x, metrics_text_y, (SDL_Color){100, 255, 100, 255});
        }
        
        // Get cymatic timing stats
        uint64_t epoch_syncs = 0, batch_syncs = 0;
        
        if (threaded_training_get_cymatic_stats((ThreadedTrainingSystem*)training_system, &epoch_syncs, &batch_syncs) == 0) {
            metrics_text_y += 25;
            draw_text(renderer, "CYMATIC TIMING", metrics_text_x, metrics_text_y, (SDL_Color){100, 150, 200, 255});
            metrics_text_y += 25;
            
            char cymatic_text[128];
            snprintf(cymatic_text, sizeof(cymatic_text), "Epoch Syncs: %lu", (unsigned long)epoch_syncs);
            draw_text(renderer, cymatic_text, metrics_text_x, metrics_text_y, text_color);
            metrics_text_y += 18;
            
            snprintf(cymatic_text, sizeof(cymatic_text), "Batch Syncs: %lu", (unsigned long)batch_syncs);
            draw_text(renderer, cymatic_text, metrics_text_x, metrics_text_y, text_color);
        }
    }
}

/**
 * Handle mouse click
 */
void handle_training_tab_click(AppState* state, int x, int y) {
    if (!state) return;
    
    SDL_Event event = {0};
    event.type = SDL_MOUSEBUTTONDOWN;
    event.button.x = x;
    event.button.y = y;
    event.button.button = SDL_BUTTON_LEFT;
    
    // Handle button clicks
    if (g_training_ui.btn_pause) crystalline_button_handle_mouse(g_training_ui.btn_pause, &event);
    if (g_training_ui.btn_start) crystalline_button_handle_mouse(g_training_ui.btn_start, &event);
    if (g_training_ui.btn_save) crystalline_button_handle_mouse(g_training_ui.btn_save, &event);
    if (g_training_ui.btn_scan) crystalline_button_handle_mouse(g_training_ui.btn_scan, &event);
    if (g_training_ui.btn_select) crystalline_button_handle_mouse(g_training_ui.btn_select, &event);
    if (g_training_ui.btn_2d3d_toggle) crystalline_button_handle_mouse(g_training_ui.btn_2d3d_toggle, &event);
    
    // Handle slider clicks
    if (g_training_ui.slider_batch) crystalline_slider_handle_mouse(g_training_ui.slider_batch, &event);
    if (g_training_ui.slider_sequence) crystalline_slider_handle_mouse(g_training_ui.slider_sequence, &event);
    if (g_training_ui.slider_epochs) crystalline_slider_handle_mouse(g_training_ui.slider_epochs, &event);
    if (g_training_ui.slider_lr) crystalline_slider_handle_mouse(g_training_ui.slider_lr, &event);
    
    // Handle dropdown click
    if (g_training_ui.model_dropdown) crystalline_dropdown_handle_mouse(g_training_ui.model_dropdown, &event);
    
    // Handle file list clicks
    if (g_training_ui.file_list) {
        crystalline_list_handle_mouse(g_training_ui.file_list, &event);
    }
}



/**
 * Handle mouse press (BUTTONDOWN)
 */
void handle_training_tab_mouse_down(AppState* state, int x, int y) {
    if (!state) return;
    
    // CRITICAL: Ignore sidebar clicks (handled by main.c)
    if (x < SIDEBAR_WIDTH) return;
    
    SDL_Event event = {0};
    event.type = SDL_MOUSEBUTTONDOWN;
    event.button.x = x;
    event.button.y = y;
    event.button.button = SDL_BUTTON_LEFT;
    
    // Buttons need BUTTONDOWN to set ACTIVE state
    if (g_training_ui.btn_pause) crystalline_button_handle_mouse(g_training_ui.btn_pause, &event);
    if (g_training_ui.btn_start) crystalline_button_handle_mouse(g_training_ui.btn_start, &event);
    if (g_training_ui.btn_save) crystalline_button_handle_mouse(g_training_ui.btn_save, &event);
    if (g_training_ui.btn_scan) crystalline_button_handle_mouse(g_training_ui.btn_scan, &event);
    if (g_training_ui.btn_select) crystalline_button_handle_mouse(g_training_ui.btn_select, &event);
    if (g_training_ui.btn_2d3d_toggle) crystalline_button_handle_mouse(g_training_ui.btn_2d3d_toggle, &event);
    
    // Dropdown needs BUTTONDOWN for selection
    if (g_training_ui.model_dropdown) {
        crystalline_dropdown_handle_mouse(g_training_ui.model_dropdown, &event);
    }
    
    // File list needs BUTTONDOWN for checkbox clicks
    if (g_training_ui.file_list) {
        crystalline_list_handle_mouse(g_training_ui.file_list, &event);
    }
    
    // Sliders need BUTTONDOWN to start dragging
    if (g_training_ui.slider_batch) crystalline_slider_handle_mouse(g_training_ui.slider_batch, &event);
    if (g_training_ui.slider_sequence) crystalline_slider_handle_mouse(g_training_ui.slider_sequence, &event);
    if (g_training_ui.slider_epochs) crystalline_slider_handle_mouse(g_training_ui.slider_epochs, &event);
    if (g_training_ui.slider_lr) crystalline_slider_handle_mouse(g_training_ui.slider_lr, &event);
}

/**
 * Handle mouse release
 */
void handle_training_tab_mouse_up(AppState* state, int x, int y) {
    if (!state) return;
    
    // CRITICAL: Ignore sidebar clicks (handled by main.c)
    if (x < SIDEBAR_WIDTH) return;
    
    SDL_Event event = {0};
    event.type = SDL_MOUSEBUTTONUP;
    event.button.x = x;
    event.button.y = y;
    event.button.button = SDL_BUTTON_LEFT;
    
    // Handle button releases
    if (g_training_ui.btn_pause) crystalline_button_handle_mouse(g_training_ui.btn_pause, &event);
    if (g_training_ui.btn_start) crystalline_button_handle_mouse(g_training_ui.btn_start, &event);
    if (g_training_ui.btn_save) crystalline_button_handle_mouse(g_training_ui.btn_save, &event);
    if (g_training_ui.btn_scan) crystalline_button_handle_mouse(g_training_ui.btn_scan, &event);
    if (g_training_ui.btn_select) crystalline_button_handle_mouse(g_training_ui.btn_select, &event);
    if (g_training_ui.btn_2d3d_toggle) crystalline_button_handle_mouse(g_training_ui.btn_2d3d_toggle, &event);
    
    // Handle slider releases
    if (g_training_ui.slider_batch) crystalline_slider_handle_mouse(g_training_ui.slider_batch, &event);
    if (g_training_ui.slider_sequence) crystalline_slider_handle_mouse(g_training_ui.slider_sequence, &event);
    if (g_training_ui.slider_epochs) crystalline_slider_handle_mouse(g_training_ui.slider_epochs, &event);
    if (g_training_ui.slider_lr) crystalline_slider_handle_mouse(g_training_ui.slider_lr, &event);
}

/**
 * Handle mouse motion
 */
void handle_training_tab_mouse_motion(AppState* state, int x, int y) {
    if (!state) return;
    
    // CRITICAL: Ignore sidebar hover (handled by main.c)
    if (x < SIDEBAR_WIDTH) return;
    
    SDL_Event event = {0};
    event.type = SDL_MOUSEMOTION;
    event.motion.x = x;
    event.motion.y = y;
    
    // Handle button hover (CRITICAL for hover animations!)
    if (g_training_ui.btn_pause) crystalline_button_handle_mouse(g_training_ui.btn_pause, &event);
    if (g_training_ui.btn_start) crystalline_button_handle_mouse(g_training_ui.btn_start, &event);
    if (g_training_ui.btn_save) crystalline_button_handle_mouse(g_training_ui.btn_save, &event);
    if (g_training_ui.btn_scan) crystalline_button_handle_mouse(g_training_ui.btn_scan, &event);
    if (g_training_ui.btn_select) crystalline_button_handle_mouse(g_training_ui.btn_select, &event);
    if (g_training_ui.btn_2d3d_toggle) crystalline_button_handle_mouse(g_training_ui.btn_2d3d_toggle, &event);
    
    // Handle slider dragging
    if (g_training_ui.slider_batch) crystalline_slider_handle_mouse(g_training_ui.slider_batch, &event);
    if (g_training_ui.slider_sequence) crystalline_slider_handle_mouse(g_training_ui.slider_sequence, &event);
    if (g_training_ui.slider_epochs) crystalline_slider_handle_mouse(g_training_ui.slider_epochs, &event);
    if (g_training_ui.slider_lr) crystalline_slider_handle_mouse(g_training_ui.slider_lr, &event);
    
    // CRITICAL: Handle dropdown hover for option selection
    if (g_training_ui.model_dropdown) crystalline_dropdown_handle_mouse(g_training_ui.model_dropdown, &event);
    
    // Handle file list hover
    if (g_training_ui.file_list) crystalline_list_handle_mouse(g_training_ui.file_list, &event);
}

/**
 * Handle keyboard input
 */
void handle_training_tab_keydown(AppState* state, int key) {
    if (!state) return;
    
    // Handle 3D rotation with arrow keys
    if (state->sphere_viz_mode == SPHERE_VIZ_3D) {
        const double rotation_speed = 0.1;
        
        switch (key) {
            case SDLK_LEFT:
                state->rotation_y -= rotation_speed;
                break;
            case SDLK_RIGHT:
                state->rotation_y += rotation_speed;
                break;
            case SDLK_UP:
                state->rotation_x -= rotation_speed;
                if (state->rotation_x < -MATH_PI / 2.0) 
                    state->rotation_x = -MATH_PI / 2.0;
                break;
            case SDLK_DOWN:
                state->rotation_x += rotation_speed;
                if (state->rotation_x > MATH_PI / 2.0) 
                    state->rotation_x = MATH_PI / 2.0;
                break;
            case SDLK_r:
                // Reset rotation
                state->rotation_x = 0.0;
                state->rotation_y = 0.0;
                state->rotation_z = 0.0;
                printf("✓ Reset 3D rotation\n");
                break;
        }
    }
}

/**
 * Handle mouse wheel scrolling
 */
void handle_training_tab_scroll(AppState* state, int wheel_y) {
    (void)state;
    (void)wheel_y;
    // TODO: Implement scrolling for file list
}

/**
 * Handle text input
 */
void handle_training_tab_text_input(AppState* state, const char* text) {
    (void)state;
    (void)text;
}

/**
 * Cleanup
 */
void cleanup_training_tab(void) {
    // Cleanup Crystalline UI elements
    if (g_training_ui.viz_panel) crystalline_panel_destroy(g_training_ui.viz_panel);
    if (g_training_ui.control_panel) crystalline_panel_destroy(g_training_ui.control_panel);
    if (g_training_ui.metrics_panel) crystalline_panel_destroy(g_training_ui.metrics_panel);
    
    if (g_training_ui.btn_pause) crystalline_button_destroy(g_training_ui.btn_pause);
    if (g_training_ui.btn_start) crystalline_button_destroy(g_training_ui.btn_start);
    if (g_training_ui.btn_save) crystalline_button_destroy(g_training_ui.btn_save);
    if (g_training_ui.btn_scan) crystalline_button_destroy(g_training_ui.btn_scan);
    if (g_training_ui.btn_select) crystalline_button_destroy(g_training_ui.btn_select);
    if (g_training_ui.btn_2d3d_toggle) crystalline_button_destroy(g_training_ui.btn_2d3d_toggle);
    
    if (g_training_ui.slider_batch) crystalline_slider_destroy(g_training_ui.slider_batch);
    if (g_training_ui.slider_sequence) crystalline_slider_destroy(g_training_ui.slider_sequence);
    if (g_training_ui.slider_epochs) crystalline_slider_destroy(g_training_ui.slider_epochs);
    if (g_training_ui.slider_lr) crystalline_slider_destroy(g_training_ui.slider_lr);
    
    if (g_training_ui.model_dropdown) crystalline_dropdown_destroy(g_training_ui.model_dropdown);
    if (g_training_ui.file_list) crystalline_list_destroy(g_training_ui.file_list);
    if (g_training_ui.training_progress) crystalline_progress_destroy(g_training_ui.training_progress);
    
    g_training_ui.initialized = false;
}