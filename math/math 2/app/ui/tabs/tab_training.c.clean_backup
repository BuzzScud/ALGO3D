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
#include "cllm_model_manager.h"
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
} TrainingVisualization;

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
        free(items);
    }
}

/**
 * Button callbacks
 */
static void on_scan_clicked(void* data) {
    AppState* state = (AppState*)data;
    if (!state) return;
    
    char training_dir[1024];
    extern void workspace_get_training_dir(AppState* state, char* output, size_t output_size);
    workspace_get_training_dir(state, training_dir, sizeof(training_dir));
    scan_training_directory(training_dir);
}

static void on_select_all_clicked(void* data) {
    (void)data;
    for (int i = 0; i < g_training_ui.file_count; i++) {
        g_training_ui.files[i].selected = true;
    }
}

static void on_start_clicked(void* data) {
    AppState* state = (AppState*)data;
    if (!state) return;
    
    if (state->training_in_progress) {
        // Stop training
        printf("Stopping training...\n");
        extern void stop_training_thread(AppState* state);
        stop_training_thread(state);
    } else {
        // Start training
        printf("Starting training...\n");
        
        // Load model if needed
        if (!state->cllm_model && g_training_ui.selected_model[0]) {
            state->cllm_model = model_manager_acquire_write(g_training_ui.selected_model);
        }
        
        // Initialize training if needed
        if (!state->cllm_training && state->cllm_model) {
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
            
            state->cllm_training = cllm_training_init(state->cllm_model, &config);
            
            // Load training data
            if (state->cllm_training) {
                for (int i = 0; i < g_training_ui.file_count; i++) {
                    if (g_training_ui.files[i].selected) {
                        cllm_load_training_data(state->cllm_training, g_training_ui.files[i].filepath);
                    }
                }
            }
        }
        
        // Start training thread
        extern int start_training_thread(AppState* state);
        start_training_thread(state);
    }
}

static void on_save_clicked(void* data) {
    AppState* state = (AppState*)data;
    if (!state || !state->cllm_model) return;
    
    system("mkdir -p checkpoints");
    char checkpoint_path[512];
    snprintf(checkpoint_path, sizeof(checkpoint_path), 
            "checkpoints/checkpoint_epoch_%d.cllm", state->training_current_epoch);
    extern int cllm_write_model(CLLMModel* model, const char* path);
    if (cllm_write_model(state->cllm_model, checkpoint_path) == 0) {
        printf("✓ Checkpoint saved: %s\n", checkpoint_path);
    }
}

static void on_2d3d_toggle_clicked(void* data) {
    AppState* state = (AppState*)data;
    if (!state) return;
    
    // Toggle between 2D and 3D
    state->sphere_viz_mode = (state->sphere_viz_mode == SPHERE_VIZ_2D) ? 
                             SPHERE_VIZ_3D : SPHERE_VIZ_2D;
}

/**
 * Slider callbacks
 */
static void on_batch_size_changed(float value, void* data) {
    AppState* state = (AppState*)data;
    if (!state) return;
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
    AppState* state = (AppState*)data;
    if (!state || !g_training_ui.model_dropdown) return;
    
    // Get selected model name
    // TODO: Implement model list retrieval
    snprintf(g_training_ui.selected_model, sizeof(g_training_ui.selected_model), 
            "model_%d", index);
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
    int viz_width = (int)(RENDER_WIDTH * 0.618f);  // 61.8%
    int control_width = RENDER_WIDTH - viz_width;   // 38.2%
    
    // Create visualization panel (left side)
    g_training_ui.viz_panel = crystalline_panel_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        RENDER_OFFSET_X + 10,
        RENDER_OFFSET_Y + 10,
        viz_width - 20,
        WINDOW_HEIGHT - RENDER_OFFSET_Y - 20,
        "TRAINING",
        font
    );
    
    // Create control panel (right side)
    g_training_ui.control_panel = crystalline_panel_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        RENDER_OFFSET_X + viz_width + 10,
        RENDER_OFFSET_Y + 10,
        control_width - 30,
        WINDOW_HEIGHT - RENDER_OFFSET_Y - 20,
        "CONTROLS",
        font
    );
    
    // Create metrics panel (inside viz panel, right side)
    int metrics_x = RENDER_OFFSET_X + viz_width - 250;
    int metrics_y = RENDER_OFFSET_Y + 100;
    g_training_ui.metrics_panel = crystalline_panel_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        metrics_x,
        metrics_y,
        230,
        400,
        "STATUS",
        font
    );
    
    // Create circular buttons (Crystalline UI style)
    int btn_x = RENDER_OFFSET_X + viz_width / 2;
    int btn_y = WINDOW_HEIGHT - 100;
    
    g_training_ui.btn_pause = crystalline_button_create(
        CRYSTALLINE_STYLE_CIRCULAR,
        btn_x - 100, btn_y,
        BUTTON_RADIUS_TERTIARY, 0,
        "PAUSE",
        font
    );
    
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
        btn_x + 100, btn_y,
        BUTTON_RADIUS_TERTIARY, 0,
        "SAVE",
        font
    );
    crystalline_button_set_callback(g_training_ui.btn_save, on_save_clicked, state);
    
    // Create buttons in control panel
    int ctrl_x = RENDER_OFFSET_X + viz_width + control_width / 2;
    int ctrl_y = RENDER_OFFSET_Y + 250;
    
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
    
    // Create 2D/3D toggle button (small rectangular button)
    g_training_ui.btn_2d3d_toggle = crystalline_button_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        metrics_x + 115,
        metrics_y - 40,
        BUTTON_RECT_WIDTH_SMALL, 
        BUTTON_RECT_HEIGHT,
        "2D/3D",
        font
    );
    crystalline_button_set_callback(g_training_ui.btn_2d3d_toggle, on_2d3d_toggle_clicked, state);
    
    // Create sliders (rectangular style for better precision)
    int slider_x = RENDER_OFFSET_X + viz_width + 20;
    int slider_y = RENDER_OFFSET_Y + 150;
    int slider_w = control_width - 60;
    
    g_training_ui.slider_batch = crystalline_slider_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        slider_x, slider_y,
        slider_w, SLIDER_TRACK_HEIGHT,
        1, 256
    );
    crystalline_slider_set_callback(g_training_ui.slider_batch, on_batch_size_changed, state);
    
    g_training_ui.slider_sequence = crystalline_slider_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        slider_x, slider_y + 60,
        slider_w, SLIDER_TRACK_HEIGHT,
        32, 512
    );
    crystalline_slider_set_callback(g_training_ui.slider_sequence, on_sequence_length_changed, state);
    
    g_training_ui.slider_epochs = crystalline_slider_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        slider_x, slider_y + 120,
        slider_w, SLIDER_TRACK_HEIGHT,
        1, 100
    );
    crystalline_slider_set_callback(g_training_ui.slider_epochs, on_epochs_changed, state);
    
    g_training_ui.slider_lr = crystalline_slider_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        slider_x, slider_y + 180,
        slider_w, SLIDER_TRACK_HEIGHT,
        0.0001f, 0.1f
    );
    crystalline_slider_set_callback(g_training_ui.slider_lr, on_learning_rate_changed, state);
    
    // Create model dropdown
    g_training_ui.model_dropdown = crystalline_dropdown_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        slider_x, RENDER_OFFSET_Y + 60,
        slider_w, 40,
        font
    );
    crystalline_dropdown_set_callback(g_training_ui.model_dropdown, on_model_selected, state);
    
    // Create file list
    g_training_ui.file_list = crystalline_list_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        slider_x, ctrl_y + 80,
        slider_w, 200,
        font
    );
    
    // Create progress bar
    g_training_ui.training_progress = crystalline_progress_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        RENDER_OFFSET_X + 20,
        RENDER_OFFSET_Y + 200,
        viz_width - 40,
        30
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
    
    // Update from training metrics
    if (state->training_metrics) {
        g_training_ui.viz_data.current_epoch = state->training_metrics->training.current_epoch;
        g_training_ui.viz_data.total_epochs = state->training_metrics->training.total_epochs;
        g_training_ui.viz_data.current_loss = state->training_metrics->training.current_loss;
        
        // Update best loss
        if (g_training_ui.viz_data.current_loss < g_training_ui.viz_data.best_loss) {
            g_training_ui.viz_data.best_loss = g_training_ui.viz_data.current_loss;
        }
        
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
    
    // Draw training metrics text
    extern void draw_text(SDL_Renderer* renderer, const char* text, int x, int y, SDL_Color color);
    SDL_Color text_color = {220, 220, 220, 255};
    
    int text_x = RENDER_OFFSET_X + 30;
    int text_y = RENDER_OFFSET_Y + 50;
    
    char metrics[256];
    snprintf(metrics, sizeof(metrics), "Epoch: %d / %d  |  Loss: %.4f  |  Best: %.4f",
            g_training_ui.viz_data.current_epoch, g_training_ui.viz_data.total_epochs,
            g_training_ui.viz_data.current_loss, g_training_ui.viz_data.best_loss);
    draw_text(renderer, metrics, text_x, text_y, text_color);
    
    // Draw sphere visualization (call existing function - DO NOT MODIFY)
    int viz_width = (int)(RENDER_WIDTH * 0.618f);
    SDL_Rect sphere_bounds = {
        RENDER_OFFSET_X + 30,
        RENDER_OFFSET_Y + 250,
        viz_width - 280,
        400
    };
    draw_sphere_visualization(renderer, state, sphere_bounds);
    
    // Render progress bar
    if (g_training_ui.training_progress) {
        crystalline_progress_render(g_training_ui.training_progress, renderer);
    }
    
    // Update 2D/3D toggle button label
    if (g_training_ui.btn_2d3d_toggle) {
        const char* toggle_label = (state->sphere_viz_mode == SPHERE_VIZ_2D) ? "3D" : "2D";
        crystalline_button_set_label(g_training_ui.btn_2d3d_toggle, toggle_label);
    }
    
    // Render buttons
    if (g_training_ui.btn_pause) crystalline_button_render(g_training_ui.btn_pause, renderer);
    if (g_training_ui.btn_start) crystalline_button_render(g_training_ui.btn_start, renderer);
    if (g_training_ui.btn_save) crystalline_button_render(g_training_ui.btn_save, renderer);
    if (g_training_ui.btn_scan) crystalline_button_render(g_training_ui.btn_scan, renderer);
    if (g_training_ui.btn_select) crystalline_button_render(g_training_ui.btn_select, renderer);
    if (g_training_ui.btn_2d3d_toggle) crystalline_button_render(g_training_ui.btn_2d3d_toggle, renderer);
    
    // Render sliders with labels (proper spacing)
    int control_width = RENDER_WIDTH - viz_width;
    int slider_x = RENDER_OFFSET_X + viz_width + 20;
    int slider_y = RENDER_OFFSET_Y + 150;
    int slider_w = control_width - 60;
    
    char label[64];
    
    // Batch Size
    snprintf(label, sizeof(label), "Batch Size: %d", state->training_batch_size);
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
    
    // Render file list with checkboxes (manual rendering)
    int ctrl_y = RENDER_OFFSET_Y + 250;
    int file_list_y = ctrl_y + 80;
    int file_list_height = 200;
    
    // File list header
    char file_header[64];
    int selected_count = 0;
    for (int i = 0; i < g_training_ui.file_count; i++) {
        if (g_training_ui.files[i].selected) selected_count++;
    }
    snprintf(file_header, sizeof(file_header), "Training Files: %d (%d selected)", 
             g_training_ui.file_count, selected_count);
    draw_text(renderer, file_header, slider_x, ctrl_y + 60, text_color);
    
    // File list background
    SDL_Rect file_list_rect = {slider_x, file_list_y, slider_w, file_list_height};
    SDL_SetRenderDrawColor(renderer, 30, 30, 40, 255);
    SDL_RenderFillRect(renderer, &file_list_rect);
    SDL_SetRenderDrawColor(renderer, 60, 60, 70, 255);
    SDL_RenderDrawRect(renderer, &file_list_rect);
    
    // Render files with checkboxes
    int file_y = file_list_y + 5;
    int max_visible = (file_list_height - 10) / 18;
    for (int i = 0; i < g_training_ui.file_count && i < max_visible; i++) {
        SDL_Color file_color = g_training_ui.files[i].selected ? 
            (SDL_Color){100, 200, 255, 255} : text_color;
        
        // Checkbox
        SDL_Rect checkbox = {slider_x + 5, file_y, 12, 12};
        SDL_SetRenderDrawColor(renderer, 50, 50, 60, 255);
        SDL_RenderFillRect(renderer, &checkbox);
        SDL_SetRenderDrawColor(renderer, 80, 80, 90, 255);
        SDL_RenderDrawRect(renderer, &checkbox);
        
        if (g_training_ui.files[i].selected) {
            SDL_SetRenderDrawColor(renderer, 100, 200, 255, 255);
            SDL_Rect check = {checkbox.x + 2, checkbox.y + 2, 8, 8};
            SDL_RenderFillRect(renderer, &check);
        }
        
        // Filename (truncated)
        char display_name[50];
        snprintf(display_name, sizeof(display_name), "%.45s", g_training_ui.files[i].filename);
        draw_text(renderer, display_name, slider_x + 22, file_y, file_color);
        
        file_y += 18;
    }
    
    // Draw metrics panel content
    int metrics_x = (int)(RENDER_OFFSET_X + viz_width * 0.618f) + 10;
    int metrics_y = RENDER_OFFSET_Y + 120;
    
    draw_text(renderer, "FRAMEWORK STATUS", metrics_x, metrics_y, (SDL_Color){100, 150, 200, 255});
    metrics_y += 25;
    draw_text(renderer, "Lattice Embeddings: ACTIVE", metrics_x, metrics_y, (SDL_Color){100, 255, 100, 255});
    metrics_y += 18;
    draw_text(renderer, "Angular Attention: ACTIVE", metrics_x, metrics_y, (SDL_Color){100, 255, 100, 255});
    metrics_y += 18;
    draw_text(renderer, "Crystalline Loss: ACTIVE", metrics_x, metrics_y, (SDL_Color){100, 255, 100, 255});
    metrics_y += 25;
    
    draw_text(renderer, "PERFORMANCE", metrics_x, metrics_y, (SDL_Color){100, 150, 200, 255});
    metrics_y += 25;
    
    char perf[128];
    snprintf(perf, sizeof(perf), "Active Threads: %d", state->sphere_stats.active_spheres);
    draw_text(renderer, perf, metrics_x, metrics_y, text_color);
    metrics_y += 18;
    
    snprintf(perf, sizeof(perf), "Total Batches: %d", state->sphere_stats.total_batches);
    draw_text(renderer, perf, metrics_x, metrics_y, text_color);
    metrics_y += 18;
    
    snprintf(perf, sizeof(perf), "Gradient Norm: %.4f", state->sphere_stats.total_gradient_norm);
    draw_text(renderer, perf, metrics_x, metrics_y, text_color);
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
    
    // Handle file list checkbox clicks
    int ctrl_y = RENDER_OFFSET_Y + 250;
    int file_list_y = ctrl_y + 80;
    int file_list_height = 200;
    int viz_width = (int)(RENDER_WIDTH * 0.618f);
    int control_width = RENDER_WIDTH - viz_width;
    int slider_x = RENDER_OFFSET_X + viz_width + 20;
    int slider_w = control_width - 60;
    
    SDL_Rect file_list_rect = {slider_x, file_list_y, slider_w, file_list_height};
    
    if (x >= file_list_rect.x && x <= file_list_rect.x + file_list_rect.w &&
        y >= file_list_rect.y && y <= file_list_rect.y + file_list_rect.h) {
        
        // Calculate which file was clicked
        int relative_y = y - file_list_rect.y - 5;
        int file_index = relative_y / 18;
        
        if (file_index >= 0 && file_index < g_training_ui.file_count) {
            // Toggle selection
            g_training_ui.files[file_index].selected = !g_training_ui.files[file_index].selected;
            printf("Toggled file %d: %s -> %s\n", file_index, 
                   g_training_ui.files[file_index].filename,
                   g_training_ui.files[file_index].selected ? "selected" : "unselected");
        }
    }
}

/**
 * Handle mouse release
 */
void handle_training_tab_mouse_up(AppState* state, int x, int y) {
    if (!state) return;
    
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
    
    SDL_Event event = {0};
    event.type = SDL_MOUSEMOTION;
    event.motion.x = x;
    event.motion.y = y;
    
    // Handle slider dragging
    if (g_training_ui.slider_batch) crystalline_slider_handle_mouse(g_training_ui.slider_batch, &event);
    if (g_training_ui.slider_sequence) crystalline_slider_handle_mouse(g_training_ui.slider_sequence, &event);
    if (g_training_ui.slider_epochs) crystalline_slider_handle_mouse(g_training_ui.slider_epochs, &event);
    if (g_training_ui.slider_lr) crystalline_slider_handle_mouse(g_training_ui.slider_lr, &event);
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
                if (state->rotation_x < -PRIME_PI / 2.0) 
                    state->rotation_x = -PRIME_PI / 2.0;
                break;
            case SDLK_DOWN:
                state->rotation_x += rotation_speed;
                if (state->rotation_x > PRIME_PI / 2.0) 
                    state->rotation_x = PRIME_PI / 2.0;
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