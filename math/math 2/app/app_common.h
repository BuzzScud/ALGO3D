// app/app_common.h - Application Common Header for Prime Spiral Visualization
#ifndef APP_COMMON_H
#define APP_COMMON_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include "prime_math_custom.h"

// Include the prime math library - CORRECTED INCLUDES
#include "prime_math.h"
#include "crystal_abacus.h"
#include "prime_hyperdim.h"
#include "prime_matrix.h"
#include "cllm.h"
#include "cllm_inference.h"
#include "cllm_training.h"
#include "cllm_metrics.h"
#include "lattice_cache.h"
#include "terminal_output.h"
#include "../math/include/math/types.h"
#include "../math/include/math/clock.h"

// Video-friendly dimensions (16:9 aspect ratio)
#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 900  
#define SIDEBAR_WIDTH 200
#define SUBMENU_HEIGHT 40
#define CONTROL_PANEL_WIDTH 320
#define RENDER_WIDTH (WINDOW_WIDTH - CONTROL_PANEL_WIDTH - SIDEBAR_WIDTH)
#define RENDER_OFFSET_X SIDEBAR_WIDTH
#define RENDER_OFFSET_Y SUBMENU_HEIGHT
#define VIDEO_WIDTH 1280   // Standard 720p width for video recording
#define VIDEO_HEIGHT 720   // Standard 720p height for video recording
#define INITIAL_PRIMES 1000
#define MAX_PRIMES 50000
#define PRIME_INCREMENT 1000
#define MAX_DEPTH 3
#define MIN_ZOOM 0.01
#define MAX_ZOOM 100.0
#define CLOCK_POSITIONS 12
#define QUADRANTS 4
#define MAX_INPUT_LENGTH 64
#define MAX_FACTORS 100

// Synesthetic frequency constants
#define SYN_BASE_FREQ 432.0
#define SYN_GOLD_FREQ 1000.0
#define SYN_GREEN_FREQ 750.0
#define SYN_BLUE_FREQ 500.0

// Main tab categories (left sidebar)
typedef enum {
    MAIN_TAB_VISUALIZATION,  // Prime visualization tabs
    MAIN_TAB_AI,             // AI/ML tabs (LLM, Training, Research, Crawler)
    MAIN_TAB_MODELS,         // Model management
    MAIN_TAB_SYSTEM,         // System tabs (Benchmark, Adapters)
    MAIN_TAB_DATA,           // Data management (URL Manager, Downloaded Files)
    MAIN_TAB_COUNT
} MainTab;

// Visualization sub-tabs
typedef enum {
    VIZ_SUB_PRIME_SPIRAL,
    VIZ_SUB_CALCULATOR,
    VIZ_SUB_SPHERES,
    VIZ_SUB_PRIME_FOLDING,
    VIZ_SUB_VIDEO_GENERATOR,
    VIZ_SUB_COUNT
} VisualizationSubTab;

// AI sub-tabs
typedef enum {
    AI_SUB_LLM,
    AI_SUB_TRAINING,
    AI_SUB_RESEARCH,
    AI_SUB_CRAWLER,
    AI_SUB_COUNT
} AISubTab;

// Models sub-tabs
typedef enum {
    MODELS_SUB_MANAGEMENT,
    MODELS_SUB_COUNT
} ModelsSubTab;

// System sub-tabs
typedef enum {
    SYSTEM_SUB_BENCHMARK,
    SYSTEM_SUB_ADAPTERS,
    SYSTEM_SUB_COUNT
} SystemSubTab;

// Data sub-tabs
typedef enum {
    DATA_SUB_URL_MANAGER,
    DATA_SUB_DOWNLOADED_FILES,
    DATA_SUB_COUNT
} DataSubTab;

// Legacy TabMode for backward compatibility (will be removed)
typedef enum {
    TAB_PRIME_SPIRAL,
    TAB_CALCULATOR,
    TAB_SPHERES,
    TAB_PRIME_FOLDING,
    TAB_VIDEO_GENERATOR,
    TAB_LLM,
    TAB_TRAINING,
    TAB_RESEARCH,
    TAB_CRAWLER,
    TAB_MODELS,
    TAB_URL_MANAGER,
    TAB_DOWNLOADED_FILES,
    TAB_COUNT
} TabMode;

// Visualization modes
typedef enum {
    MODE_ULAM_SPIRAL,
    MODE_RECURSIVE_FRACTAL,
    MODE_HARMONIC_RESONANCE,
    MODE_HYPERDIM_PROJECTION,
    MODE_SELF_SIMILAR_LAYERS,
    MODE_CLOCK_SUDOKU,
    MODE_NATURAL_PRIME_LATTICE,
    MODE_FOLDED_ORIGAMI,
    MODE_COUNT
} VisualizationMode;

// Folding modes
typedef enum {
    FOLD_NONE,
    FOLD_QUADRATIC_MIRROR,
    FOLD_CLOCK_COMPRESSION,
    FOLD_HYPERDIM_ORIGAMI,
    FOLD_SPIRAL_COLLAPSE,
    FOLD_COUNT
} FoldingMode;

// Sphere visualization modes (for Training Tab)
typedef enum {
    SPHERE_VIZ_2D,          // Flat circular arrangement (default)
    SPHERE_VIZ_3D,          // 3D arrangement with depth and perspective
    SPHERE_VIZ_CRYSTALLINE  // Crystalline geometric patterns and memory structure
} SphereVisualizationMode;

// Sphere structure for 3D visualization
typedef struct {
    double x, y, z;
    double radius;
    SDL_Color color;
    int connections[8];
    int conn_count;
} Sphere;

// Clock mapping for prime visualization
typedef struct {
    int quadrant;
    double angle;
    double radius;
    int clock_position;
    double fold_factor;
} ClockMapping;

// Number analysis structure
typedef struct {
    int number;
    bool is_prime;
    int factor_count;
    int factors[MAX_FACTORS];
    int prime_factor_count;
    int prime_factors[MAX_FACTORS];
    int prime_powers[MAX_FACTORS];
    int clock_position;
    int ring;  // Which ring (1, 2, or 3)
    int ring_number;
    double angle;  // Angle in degrees
    double angle_degrees;
    double radial_distance;
    double ring_distance;  // Distance from nearest ring boundary
    int quadrant;
    bool is_coprime_to_12;
} NumberAnalysis;

// Main application state
typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Surface* recording_surface;  // Surface for video recording (software)
    SDL_Renderer* recording_renderer; // Software renderer for recording
    
    // Current tab (legacy - will be replaced)
    TabMode current_tab;
    
    // NEW: Hierarchical tab system
    MainTab main_tab;
    union {
        VisualizationSubTab viz_sub;
        AISubTab ai_sub;
        ModelsSubTab models_sub;
        SystemSubTab system_sub;
        DataSubTab data_sub;
    } sub_tab;
    
    // Prime data - OLD SYSTEM (deprecated, kept for compatibility)
    int* primes;
    int prime_count;
    int prime_capacity;
    ClockMapping* clock_map;
    int visible_primes;
    
    // NEW: Lattice Cache System
    LatticeCache* lattice_cache;
    bool lattice_cache_initialized;
    bool auto_extend_on_zoom;
    
    // NEW: Clock Context for compact vector system
    ClockContext* clock_ctx;
    
    // Spheres data
    Sphere* spheres;
    int sphere_count;
    
    // Calculator state
    double calc_value1;
    double calc_value2;
    double calc_result;
    char calc_operation[32];
    char calc_input[64];  // Current input string
    bool calc_has_value1;  // Whether value1 is set
    
    // Calculator button positions (for click detection)
    int calc_button_x;
    int calc_button_y;
    int calc_button_width;
    int calc_button_height;
    int calc_button_spacing;
    int calc_button_pressed;  // Index of button being pressed (-1 = none)
    uint32_t calc_button_press_time;  // Time when button was pressed
    
    // Mode button positions (for click detection)
    int mode_button_x;
    int mode_button_y;
    int mode_button_width;
    int mode_button_height;
    int mode_button_pressed;  // Index of mode button being pressed (-1 = none)
    uint32_t mode_button_press_time;  // Time when mode button was pressed
    
    // Visualization settings
    VisualizationMode mode;
    FoldingMode fold_mode;
    double fold_amount;
    double zoom;
    double offset_x, offset_y;
    double rotation;
    double rotation_x, rotation_y, rotation_z;
    int current_dimension;
    double entropy;
    double entropy_visual_effect;
    double animation_phase;
    int recursion_depth;
    double self_similarity_score;
    
    // Display options
    bool show_connections;
    bool show_clock_grid;
    bool show_axes;
    bool animate;
    bool show_control_panel;
    bool clock_crystalline_mode;  // Toggle between natural (false) and crystalline (true)
    
    // Recording state
    bool is_recording;
    FILE* video_pipe;
    uint8_t* frame_buffer;
    int recording_frames;
    char video_path[512];
    
    // UI state
    int selected_control;
    bool dragging_slider;
    int mouse_x, mouse_y;
    
    // Performance
    uint32_t frame_count;
    uint32_t fps;
    uint32_t last_fps_update;
    
    // Folding parameters
    double fold_axes[10];  // Max 10 dimensions
    double quadratic_mirror[QUADRANTS][QUADRANTS];
    
    // Prime input and analysis
    char input_buffer[MAX_INPUT_LENGTH];
    bool input_active;
    int selected_prime;
    bool show_factors;
    bool show_all_factors;  // Expand/collapse factor list
    NumberAnalysis number_analysis;  // Current analysis (for compatibility)
    SDL_Rect input_field_rect;  // For click detection
    
    // Multi-number analysis history (unlimited)
    struct {
        int numbers[100];  // Support up to 100 analyzed numbers
        SDL_Color colors[100];
        int count;
        int selected_index;  // Which number is selected (-1 = none, show all)
    } analysis_history;
    
    // Factor cache for performance
    struct {
        int number;
        int* factors;
        int count;
        bool valid;
    } factor_cache[100];  // One cache entry per analyzed number
    int factor_cache_count;
    
    // Animation control
    double animation_speed;
    bool animation_paused;
    
    // CLLM Integration
    CLLMModel* cllm_model;
    CLLMInference* cllm_inference;
    CLLMTraining* cllm_training;
    char llm_input_text[1024];
    char llm_output_text[4096];
    bool llm_generating;
    float llm_temperature;
    int llm_max_tokens;
    
    // Training state
    char training_data_path[512];
    int training_epochs;
    float training_learning_rate;
    int training_batch_size;        // NEW: Batch size for training
    int training_sequence_length;   // NEW: Sequence length for training
    bool training_in_progress;
    float training_loss;
    int training_current_epoch;
    int training_thread_count;  // Number of threads for parallel training
    
    // Training status for UI feedback
    char training_status_message[256];  // Current status message
    float training_preprocessing_progress;  // 0.0 to 1.0 for preprocessing phases
    
    // Workspace management
    char workspace_path[512];  // Current workspace directory
    bool workspace_active;     // Whether using custom workspace
    
    // Research browser state
    char research_directory[512];
    int research_file_count;
    int research_selected_file;
    
    // Crawler state
    char crawler_start_url[2048];
    char crawler_current_url[2048];
    bool crawler_running;
    int crawler_pages_crawled;
    int crawler_queue_size;
    char crawler_data_dir[512];
    
    // Kissing spheres training statistics (for UI display)
    // PHASE 5: Expanded to support hierarchical visualization (up to 144 spheres)
    struct {
        int batches_processed[144];   // Per-sphere batch count (12^2 = 144 max)
        float avg_loss[144];           // Per-sphere average loss
        int parent_id[144];            // Parent sphere ID (-1 for root/orphan)
        int num_children[144];         // Number of children per sphere
        int is_control[144];           // 1 if control thread, 0 if worker
        int hierarchy_level[144];      // Level in hierarchy (0=root, 1=level-1, 2=level-2, -1=inactive)
        int symmetry_group[144];       // Symmetry group (0-11, -1=none)
        int active_spheres;            // Number of active spheres
        float total_gradient_norm;    // Total gradient magnitude
        int total_batches;            // Total batches across all spheres
    } sphere_stats;
    pthread_mutex_t sphere_stats_mutex;  // Protects sphere_stats from race conditions
    
    // Sphere visualization mode (2D or 3D)
    SphereVisualizationMode sphere_viz_mode;
    
    // UI Integration: Real-time metrics from training system
    CLLMMetrics* training_metrics;  // Forward declaration, defined in cllm_metrics.h
    
    // Terminal output buffer for training tab
    TerminalBuffer* terminal_buffer;  // Defined in terminal_output.h
    
    // Training tab panel states (collapsible panels)
    struct {
        bool framework_expanded;
        bool performance_expanded;
        bool legend_expanded;
        bool stats_table_expanded;
        bool terminal_expanded;
    } training_panels;
    
    // Control thread initialization status (MASTER_PLAN architecture)
    bool abacus_initializing;  // True while abacus is being initialized
    bool abacus_ready;         // True when abacus is ready for use
    bool model_loading;        // True while model is being loaded
    bool model_ready;          // True when model is ready for inference
} AppState;

// Utility functions - use library versions where available
// prime_min_int, prime_max_int are in the library
static inline int int_clamp(int val, int min, int max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

// Function declarations from main.c
AppState* init_app(void);
void cleanup(AppState* state);

// Function declarations from prime_input.c
int find_prime_index(int prime, AppState* state);
void handle_input(AppState* state, SDL_Event* event);
void render(AppState* state);
void handle_mouse_click(AppState* state, int x, int y);
void expand_primes(AppState* state);

// Function declarations from visualization.c
void draw_visualization(SDL_Renderer* renderer, AppState* state);
void draw_recursive_pattern(SDL_Renderer* renderer, double cx, double cy, double size, 
                           int depth, int prime_idx, AppState* state);
void draw_enhanced_ulam_spiral(SDL_Renderer* renderer, AppState* state);
void draw_self_similar_layers(SDL_Renderer* renderer, AppState* state);
void draw_clock_sudoku(SDL_Renderer* renderer, AppState* state);
void draw_hyperdim_prime(SDL_Renderer* renderer, int prime, int x, int y, AppState* state);
void apply_app_hyperdim_folding(double* x, double* y, AppState* state);
ClockMapping map_prime_to_clock(int prime, double fold_amount);
SDL_Color get_vibrational_color(double value, double prime);
void draw_clock_grid(SDL_Renderer* renderer);

// Function declarations from ui.c
int init_font_system(void);
void cleanup_font_system(void);
TTF_Font* get_global_font(void);
void draw_text(SDL_Renderer* renderer, const char* text, int x, int y, SDL_Color color);
void draw_tabs(SDL_Renderer* renderer, AppState* state);
void draw_control_panel(SDL_Renderer* renderer, AppState* state);

// Function declarations from tab_llm.c
void draw_llm_tab(SDL_Renderer* renderer, AppState* state);
void handle_llm_tab_click(AppState* state, int x, int y);
void handle_llm_tab_text_input(AppState* state, const char* text);
void handle_llm_tab_key(AppState* state, SDL_Keycode key);

// Function declarations from tab_training.c
void draw_training_tab(SDL_Renderer* renderer, AppState* state);
void handle_training_tab_click(AppState* state, int x, int y);
void draw_terminal_output(SDL_Renderer* renderer, AppState* state, SDL_Rect bounds);
void update_training_visualization(AppState* state);
void handle_training_tab_text_input(AppState* state, const char* text);
void handle_training_tab_keydown(AppState* state, SDL_Keycode key);
bool handle_training_tab_event(AppState* state, SDL_Event* event);

// Function declarations from tab_research.c
void draw_research_tab(SDL_Renderer* renderer, AppState* state);
void handle_research_tab_click(AppState* state, int x, int y);
void handle_research_tab_text_input(AppState* state, const char* text);
void handle_research_tab_keydown(AppState* state, SDL_Keycode key);
void handle_research_tab_scroll(AppState* state, int scroll_y);
bool handle_research_tab_event(AppState* state, SDL_Event* event);

// Function declarations from io.c
void start_recording(AppState* state);
void stop_recording(AppState* state);
void record_frame(AppState* state);

// Function declarations from enhanced_clock.c
int get_prime_ring(int prime);
int get_clock_position(int prime);
double get_exact_angle(int prime);
double get_radial_distance(int prime, double zoom);
int is_factor_of(int prime, int target);
void draw_enhanced_clock_sudoku(SDL_Renderer* renderer, AppState* state);
void draw_natural_prime_lattice(SDL_Renderer* renderer, AppState* state);
void draw_clock_face_grid(SDL_Renderer* renderer, int cx, int cy, AppState* state);
void draw_concentric_rings(SDL_Renderer* renderer, int cx, int cy, AppState* state);
void draw_radial_lines(SDL_Renderer* renderer, int cx, int cy, AppState* state);
void draw_primes_on_clock(SDL_Renderer* renderer, int cx, int cy, AppState* state);
void highlight_prime_and_factors(SDL_Renderer* renderer, int cx, int cy, AppState* state);
void draw_center_point(SDL_Renderer* renderer, int cx, int cy);
void draw_clock_labels(SDL_Renderer* renderer, int cx, int cy, AppState* state);

// Function declarations from clock_crystalline.c
void draw_clock_sudoku_crystalline(SDL_Renderer* renderer, AppState* state);
void get_crystalline_position(int prime, uint64_t prime_index, double *angle, double *radius, AppState* state);

// Function declarations from prime_input.c
void init_prime_input(AppState* state);
void handle_text_input(AppState* state, const char* text);
void handle_backspace(AppState* state);
void process_input(AppState* state);
void analyze_number(AppState* state, int num);
void add_to_analysis_history(AppState* state, int number);
void clear_analysis_history(AppState* state);
SDL_Color get_analysis_color(int index);
void draw_prime_input_panel(SDL_Renderer* renderer, AppState* state);
void draw_number_analysis(SDL_Renderer* renderer, AppState* state, int x_base, int y_pos);
void draw_factor_connections(SDL_Renderer* renderer, AppState* state, int cx, int cy);
void invalidate_factor_cache(AppState* state);
int* get_factors_cached(AppState* state, int number, int* out_count);
void draw_detailed_overlay(SDL_Renderer* renderer, AppState* state);
void draw_analyzed_numbers(SDL_Renderer* renderer, AppState* state, int cx, int cy);
void handle_input_field_click(AppState* state, int x, int y);
void handle_prime_input_key(AppState* state, SDL_Keycode key);
void save_snapshot(AppState* state);
char* get_save_path_with_extension(const char* title, const char* default_name, const char* extension);
void ensure_file_extension(char* path, const char* extension);

// Function declarations from spheres.c
void init_spheres(AppState* state);
void draw_spheres(SDL_Renderer* renderer, AppState* state);

// Function declarations from calculator.c
void draw_calculator(SDL_Renderer* renderer, AppState* state);
void draw_calculator_with_visualization(SDL_Renderer* renderer, AppState* state);
void draw_calculator_with_minimap(SDL_Renderer* renderer, AppState* state);
void handle_calculator_button(AppState* state, int button_idx);
void handle_mode_button(AppState* state, int mode_idx);
void handle_minimap_click(AppState* state, int click_x, int click_y, int map_x, int map_y, int map_width, int map_height);

// Lattice cache functions
#include "lattice_cache.h"
void init_lattice_cache(AppState* state, uint64_t max_prime);
void extend_lattice_cache(AppState* state, uint64_t new_max);

#endif // APP_COMMON_H