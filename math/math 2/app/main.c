// app/main.c - Main application entry point
#define _POSIX_C_SOURCE 200809L
#include "app_common.h"
#include "cllm_integration.h"
#include "cllm_format.h"
#include "cllm_model_registry.h"
#include "cllm_model_registry.h"
#include "input_manager.h"
#include "ui/tabs/tab_video.h"
#include "ui/tabs/tab_crawler.h"
#include "ui/tabs/tab_models.h"
#include "ui/tabs/tab_url_manager.h"
#include "ui/tabs/tab_downloaded_files.h"
#include "ui/layout_manager.h"
#include "ui/left_sidebar.h"
#include "ui/loading_screen.h"

// Global pointer for lattice cache access from helper functions
AppState* app_state_global = NULL;

// Global loading screen state
static LoadingScreenState g_loading_screen;

AppState* init_app(void) {
    AppState* state = calloc(1, sizeof(AppState));
    if (!state) return NULL;
    
    // Initialize sphere_stats mutex
    pthread_mutex_init(&state->sphere_stats_mutex, NULL);
    
    // Initialize sphere visualization mode to 2D (default)
    state->sphere_viz_mode = SPHERE_VIZ_2D;
    
    // Initialize loading screen state
    loading_screen_init(&g_loading_screen);
    
    // Stage 1: Initialize SDL
    loading_screen_update(&g_loading_screen, LOAD_STAGE_SDL, 0.0f);
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL init failed: %s\n", SDL_GetError());
        loading_screen_set_error(&g_loading_screen, "SDL initialization failed");
        return NULL;
    }
    loading_screen_update(&g_loading_screen, LOAD_STAGE_SDL, 1.0f);
    
    // Stage 2: Create window
    loading_screen_update(&g_loading_screen, LOAD_STAGE_WINDOW, 0.0f);
    state->window = SDL_CreateWindow("Crystalline CLLM",
                                    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                    WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!state->window) {
        loading_screen_set_error(&g_loading_screen, "Window creation failed");
        SDL_Quit();
        return NULL;
    }
    loading_screen_update(&g_loading_screen, LOAD_STAGE_WINDOW, 1.0f);
    
    // Stage 3: Create renderer
    loading_screen_update(&g_loading_screen, LOAD_STAGE_RENDERER, 0.0f);
    state->renderer = SDL_CreateRenderer(state->window, -1, 
                                        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!state->renderer) {
        loading_screen_set_error(&g_loading_screen, "Renderer creation failed");
        SDL_DestroyWindow(state->window);
        SDL_Quit();
        return NULL;
    }
    
    // Render initial loading screen
    loading_screen_render(state->renderer, &g_loading_screen, 255);
    loading_screen_update(&g_loading_screen, LOAD_STAGE_RENDERER, 1.0f);
    loading_screen_render(state->renderer, &g_loading_screen, 255);
    
    // Stage 4: Load fonts
    loading_screen_update(&g_loading_screen, LOAD_STAGE_FONTS, 0.0f);
    loading_screen_render(state->renderer, &g_loading_screen, 255);
    if (!init_font_system()) {
        printf("Font system init failed\n");
        loading_screen_set_error(&g_loading_screen, "Font system initialization failed");
        loading_screen_render(state->renderer, &g_loading_screen, 255);
        SDL_Delay(2000); // Show error for 2 seconds
        SDL_DestroyRenderer(state->renderer);
        SDL_DestroyWindow(state->window);
        SDL_Quit();
        return NULL;
    }
    loading_screen_update(&g_loading_screen, LOAD_STAGE_FONTS, 1.0f);
    loading_screen_render(state->renderer, &g_loading_screen, 255);

    // Stage 5: Initialize input system
    loading_screen_update(&g_loading_screen, LOAD_STAGE_INPUT, 0.0f);
    loading_screen_render(state->renderer, &g_loading_screen, 255);
    
    g_input_manager = input_manager_create();
    if (!g_input_manager) {
        printf("Failed to create input manager\n");
        loading_screen_set_error(&g_loading_screen, "Input manager creation failed");
        loading_screen_render(state->renderer, &g_loading_screen, 255);
        SDL_Delay(2000);
        SDL_DestroyRenderer(state->renderer);
        SDL_DestroyWindow(state->window);
        SDL_Quit();
        return NULL;
    }
    
    loading_screen_update(&g_loading_screen, LOAD_STAGE_INPUT, 0.5f);
    loading_screen_render(state->renderer, &g_loading_screen, 255);
    
    // CRITICAL: Register ALL tab inputs at startup
    // This ensures inputs are available immediately, regardless of which tab is visited first
    extern void init_all_inputs(InputManager* manager, void* state);
    init_all_inputs(g_input_manager, state);
    
    loading_screen_update(&g_loading_screen, LOAD_STAGE_INPUT, 1.0f);
    loading_screen_render(state->renderer, &g_loading_screen, 255);
    
    // Stage 6: Initialize tabs
    loading_screen_update(&g_loading_screen, LOAD_STAGE_TABS, 0.0f);
    loading_screen_render(state->renderer, &g_loading_screen, 255);
    
    // CRITICAL: Initialize ALL tab UI components
    // This ensures panels, buttons, and other UI elements are created
    extern void init_all_tabs(AppState* state);
    init_all_tabs(state);
    
    loading_screen_update(&g_loading_screen, LOAD_STAGE_TABS, 1.0f);
    loading_screen_render(state->renderer, &g_loading_screen, 255);
    
    // Stage 7: Start background services
    loading_screen_update(&g_loading_screen, LOAD_STAGE_SERVICES, 0.0f);
    loading_screen_render(state->renderer, &g_loading_screen, 255);
    
    // Initialize model registry
    printf("\n=== Initializing Model Registry ===\n");
    if (!model_registry_init("./models")) {
        printf("WARNING: Failed to initialize model registry\n");
    } else {
        printf("Model registry initialized successfully\n");
    }
    printf("=== Model Registry Ready ===\n\n");
    
    // Initialize model registry
    printf("=== Initializing Model Registry ===\n");
    if (!model_registry_init("./models")) {
        printf("WARNING: Failed to initialize model registry\n");
    } else {
        printf("Model registry initialized successfully\n");
        // Scan for models
        int model_count = model_registry_scan();
        printf("Found %d models in registry\n", model_count);
    }
    printf("=== Model Registry Ready ===\n\n");
    
    loading_screen_update(&g_loading_screen, LOAD_STAGE_SERVICES, 0.5f);
    loading_screen_render(state->renderer, &g_loading_screen, 255);
    
    // Initialize state
    state->mode = MODE_ULAM_SPIRAL;
    state->fold_mode = FOLD_NONE;
    state->current_tab = TAB_PRIME_SPIRAL;
    
    // Initialize hierarchical tab system
    state->main_tab = MAIN_TAB_VISUALIZATION;
    state->sub_tab.viz_sub = VIZ_SUB_PRIME_SPIRAL;
    
    state->zoom = 1.0;
    state->animate = false;
    state->show_control_panel = true;
    state->clock_crystalline_mode = false;  // Start with natural X pattern
    
    // Set InputManager's initial tab
    if (g_input_manager) {
        input_manager_set_tab(g_input_manager, TAB_PRIME_SPIRAL);
    }
    
    // Generate primes - OLD SYSTEM (kept for compatibility)
    state->prime_count = 1000;
    state->primes = generate_n_primes(state->prime_count);
    state->visible_primes = state->prime_count;
    printf("Generated %d primes (old system)\n", state->prime_count);
    
    // OPTIMIZATION: Defer lattice cache initialization to improve startup time
    // Cache will be initialized on first use (when needed for visualization)
    // printf("\n=== Initializing Crystalline Lattice Cache ===\n");
    // state->lattice_cache = lattice_cache_create(200000);
    state->lattice_cache = NULL;  // Initialize lazily
    state->lattice_cache_initialized = false;
    state->auto_extend_on_zoom = false;  // Disabled by default
    // printf("=== Lattice Cache Ready ===\n\n");
    
    // Initialize clock map
    state->clock_map = calloc(state->prime_count, sizeof(ClockMapping));
    for (int i = 0; i < state->prime_count; i++) {
        state->clock_map[i].quadrant = state->primes[i] % 4;
        state->clock_map[i].angle = (state->primes[i] % 360) * 3.14159 / 180.0;
        state->clock_map[i].radius = 1.0;
        state->clock_map[i].clock_position = (state->primes[i] - 1) % 12;
        state->clock_map[i].fold_factor = 0.0;
    }
    
    // Initialize spheres
    state->sphere_count = 50;
    state->spheres = calloc(state->sphere_count, sizeof(Sphere));
    for (int i = 0; i < state->sphere_count; i++) {
        state->spheres[i].x = (rand() % 200) - 100;
        state->spheres[i].y = (rand() % 200) - 100;
        state->spheres[i].z = (rand() % 200) - 100;
        state->spheres[i].radius = 5 + (rand() % 15);
    }
    
    strcpy(state->calc_operation, "+");
    state->calc_input[0] = '\0';
    state->show_all_factors = false;
    
    // Initialize button press tracking
    state->calc_button_pressed = -1;
    state->calc_button_press_time = 0;
    state->mode_button_pressed = -1;
    state->mode_button_press_time = 0;
    
    // Initialize analysis history
    state->analysis_history.count = 0;
    state->analysis_history.selected_index = -1;  // -1 = show all
    
    // Initialize factor cache
    state->factor_cache_count = 0;
    for (int i = 0; i < 100; i++) {
        state->factor_cache[i].number = 0;
        state->factor_cache[i].factors = NULL;
        state->factor_cache[i].count = 0;
        state->factor_cache[i].valid = false;
    }
    
    // Initialize recording system (software renderer for video capture)
    state->recording_surface = SDL_CreateRGBSurface(0, VIDEO_WIDTH, VIDEO_HEIGHT, 32,
                                                     0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    if (state->recording_surface) {
        state->recording_renderer = SDL_CreateSoftwareRenderer(state->recording_surface);
        if (state->recording_renderer) {
            printf("Recording system initialized (720p)\n");
        } else {
            printf("Warning: Could not create recording renderer\n");
            SDL_FreeSurface(state->recording_surface);
            state->recording_surface = NULL;
        }
    } else {
        printf("Warning: Could not create recording surface\n");
    }
    
    // Initialize terminal output buffer
    state->terminal_buffer = calloc(1, sizeof(TerminalBuffer));
    if (state->terminal_buffer) {
        terminal_buffer_init(state->terminal_buffer);
        printf("✓ Terminal output buffer initialized\n");
    }
    
    // Initialize training panel states (all expanded by default)
    state->training_panels.framework_expanded = true;
    
    // Complete loading
    loading_screen_update(&g_loading_screen, LOAD_STAGE_SERVICES, 1.0f);
    loading_screen_render(state->renderer, &g_loading_screen, 255);
    
    loading_screen_update(&g_loading_screen, LOAD_STAGE_COMPLETE, 1.0f);
    loading_screen_render(state->renderer, &g_loading_screen, 255);
    state->training_panels.performance_expanded = true;
    state->training_panels.legend_expanded = true;
    state->training_panels.stats_table_expanded = true;
    state->training_panels.terminal_expanded = true;
    
    // Initialize CLLM system
    printf("\n=== Initializing CLLM System ===\n");
    state->cllm_model = NULL;
    state->cllm_inference = NULL;
    state->cllm_training = NULL;
    state->llm_input_text[0] = '\0';
    state->llm_output_text[0] = '\0';
    state->llm_generating = false;
    // LLM parameters - optimized for coherent output
    state->llm_temperature = 0.8f;     // Balanced (was 0.7, user had 1.29)
    state->llm_max_tokens = 512;       // Longer responses (was 100)
    state->training_data_path[0] = '\0';
    state->training_epochs = 10;
    state->training_batch_size = 1;           // NEW: Default batch size
    state->training_sequence_length = 64;     // NEW: Default sequence length
    state->training_thread_count = 0;  // 0 = auto-detect optimal
    state->training_learning_rate = 0.001f;
    state->training_in_progress = false;
    state->training_loss = 0.0f;
    state->training_current_epoch = 0;
    strcpy(state->research_directory, "docs/research");
    state->research_file_count = 0;
    state->research_selected_file = -1;
    
    // Initialize crawler state
    state->crawler_start_url[0] = '\0'; // No default URL - user must provide
    state->crawler_current_url[0] = '\0';
    state->crawler_running = false;
    state->crawler_pages_crawled = 0;
    state->crawler_queue_size = 0;
    
    // Set crawler data directory (use workspace if available, otherwise default)
    if (state->workspace_active) {
        snprintf(state->crawler_data_dir, sizeof(state->crawler_data_dir), 
                 "%.498s/crawler_data", state->workspace_path);
    } else {
        strcpy(state->crawler_data_dir, "crawler_data");
    }
    
    // Model loading moved to control thread - happens in background
    // Set initial message
    strcpy(state->llm_output_text, "Initializing system... Please wait.");
    
    return state;
}

void cleanup(AppState* state) {
    if (!state) return;
    
    // Cleanup all tab UI components
    extern void cleanup_all_tabs(AppState* state);
    cleanup_all_tabs(state);
    if (state->is_recording) stop_recording(state);
    
    // CRITICAL: Stop control thread before cleanup
    extern void stop_control_thread(void);
    stop_control_thread();
    
    // CRITICAL: Stop crawler thread before cleanup
    extern void stop_crawler_thread(void);
    stop_crawler_thread();
    
    // CRITICAL: Stop training thread before cleanup (prevents use-after-free)
    extern void stop_training_thread(AppState* state);
    stop_training_thread(state);
    
    if (state->primes) free(state->primes);
    if (state->clock_map) free(state->clock_map);
    if (state->spheres) free(state->spheres);
    
    // NEW: Free lattice cache
    if (state->lattice_cache) {
        lattice_cache_free(state->lattice_cache);
    }
    
    // Free terminal buffer
    if (state->terminal_buffer) {
        free(state->terminal_buffer);
    }
    
    // PHASE 1: Cleanup global abacus
    extern void app_cleanup_global_abacus(void);
    app_cleanup_global_abacus();
    
    // CRITICAL: Cleanup model manager (fixes 395MB leak)
    model_registry_cleanup();
    
    // Cleanup model registry
    model_registry_cleanup();
    
    if (state->renderer) SDL_DestroyRenderer(state->renderer);
    if (state->window) SDL_DestroyWindow(state->window);
    cleanup_font_system();
    
    // Destroy sphere_stats mutex
    pthread_mutex_destroy(&state->sphere_stats_mutex);
    
    free(state);
    SDL_Quit();
}

void expand_primes(AppState* state) {
    int new_count = state->prime_count + 1000;
    int* new_primes = generate_n_primes(new_count);
    free(state->primes);
    state->primes = new_primes;
    state->prime_count = new_count;
    state->visible_primes = new_count;
    
    state->clock_map = realloc(state->clock_map, new_count * sizeof(ClockMapping));
    for (int i = state->prime_count - 1000; i < new_count; i++) {
        state->clock_map[i].quadrant = state->primes[i] % 4;
        state->clock_map[i].angle = (state->primes[i] % 360) * 3.14159 / 180.0;
        state->clock_map[i].radius = 1.0;
        state->clock_map[i].clock_position = (state->primes[i] - 1) % 12;
        state->clock_map[i].fold_factor = 0.0;
    }
    printf("Expanded to %d primes\n", new_count);
}

void handle_mouse_click(AppState* state, int x, int y) {
    // Silent mouse clicks (no terminal spam)
    
    // Check if clicking on left sidebar (200px width)
    if (x < 200) {
        handle_sidebar_click(state, x, y);
        return;
    }
    
    // Check if clicking on submenu bar (40px height, after sidebar)
    if (y < 40 && x >= 200) {
        handle_submenu_click(state, x, y);
        return;
    }
    
    // Check if clicking on analyzed numbers in visualization area
    if (x < RENDER_WIDTH && state->analysis_history.count > 0) {
        int center_x = RENDER_WIDTH / 2;
        int center_y = WINDOW_HEIGHT / 2;
        
        // Check each analyzed number
        for (int i = 0; i < state->analysis_history.count; i++) {
            int num = state->analysis_history.numbers[i];
            
            // Get position using same functions as drawing
            double angle = get_exact_angle(num);
            double radius = get_radial_distance(num, state->zoom);
            
            int screen_x = center_x + (int)(radius * math_cos(angle)) + (int)state->offset_x;
            int screen_y = center_y + (int)(radius * math_sin(angle)) + (int)state->offset_y;
            
            // Check if click is within 20 pixels of this number
            int dx = x - screen_x;
            int dy = y - screen_y;
            int dist_sq = dx * dx + dy * dy;
            
            if (dist_sq <= 400) {  // 20 pixel radius
                // Select this number
                state->analysis_history.selected_index = i;
                // Re-analyze this number to update the overlay
                analyze_number(state, num);
                printf("Selected analyzed number: %d (index %d)\n", num, i);
                return;
            }
        }
    }
    
    // Check if clicking on input field or analyze button (in control panel)
    if (state->current_tab == TAB_PRIME_SPIRAL && x >= RENDER_WIDTH) {
        // Input field
        SDL_Rect* input_rect = &state->input_field_rect;
        if (x >= input_rect->x && x < input_rect->x + input_rect->w &&
            y >= input_rect->y && y < input_rect->y + input_rect->h) {
            state->input_active = true;
            SDL_StartTextInput();
            printf("Input field clicked - activated\n");
        }
        
        // Analyze button (approximately 35 pixels below input field)
        int analyze_btn_y = input_rect->y + 65;  // Approximate position
        SDL_Rect analyze_rect = {input_rect->x, analyze_btn_y, 290, 25};
        if (x >= analyze_rect.x && x < analyze_rect.x + analyze_rect.w &&
            y >= analyze_rect.y && y < analyze_rect.y + analyze_rect.h) {
            printf("=== ANALYZE BUTTON CLICKED ===\n");
            process_input(state);
            printf("=== ANALYSIS COMPLETE ===\n");
        }
        
        // NEW: Lattice cache control buttons
        if (state->lattice_cache_initialized) {
            int x_base = RENDER_WIDTH + 10;
            
            // Get current y position (after analysis section, before controls)
            // Approximate position based on UI layout
            int cache_y = WINDOW_HEIGHT - 320;  // Above controls section
            
            // Generate More button (140x25)
            SDL_Rect gen_btn = {x_base + 5, cache_y + 60, 140, 25};
            if (x >= gen_btn.x && x < gen_btn.x + gen_btn.w &&
                y >= gen_btn.y && y < gen_btn.y + gen_btn.h) {
                // Generate More clicked
                uint64_t current_max = 0;
                lattice_cache_get_stats(state->lattice_cache, NULL, &current_max, NULL);
                uint64_t new_max = current_max + 50000;
                printf("Generating more primes up to %lu...\n", new_max);
                size_t added = lattice_cache_extend(state->lattice_cache, new_max);
                printf("Added %lu new primes\n", added);
                return;
            }
            
            // Auto-extend toggle button (140x25)
            SDL_Rect auto_btn = {x_base + 155, cache_y + 60, 140, 25};
            if (x >= auto_btn.x && x < auto_btn.x + auto_btn.w &&
                y >= auto_btn.y && y < auto_btn.y + auto_btn.h) {
                // Toggle auto-extend
                state->auto_extend_on_zoom = !state->auto_extend_on_zoom;
                lattice_cache_set_auto_extend(state->lattice_cache, state->auto_extend_on_zoom);
                return;
            }
        }
    }
    
    // Route clicks to tab-specific handlers
    if (state->current_tab == TAB_LLM) {
        handle_llm_tab_click(state, x, y);
        return;
    }
    
    if (state->current_tab == TAB_TRAINING) {
        handle_training_tab_click(state, x, y);
        return;
    }
    
    if (state->current_tab == TAB_RESEARCH) {
        handle_research_tab_click(state, x, y);
        return;
    }
    
    if (state->current_tab == TAB_CRAWLER) {
        // Crawler tab uses new event handlers (mouse_down/up/motion)
        // No legacy click handler needed
        return;
    }
    
    if (state->current_tab == TAB_VIDEO_GENERATOR) {
        handle_video_tab_click(state, x, y);
        return;
    }
    
    if (state->current_tab == TAB_URL_MANAGER) {
        extern void handle_url_manager_tab_click(AppState* state, int x, int y);
        handle_url_manager_tab_click(state, x, y);
        return;
    }
    
    if (state->current_tab == TAB_DOWNLOADED_FILES) {
        extern void handle_downloaded_files_tab_click(AppState* state, int x, int y);
        handle_downloaded_files_tab_click(state, x, y);
        return;
    }
    
    if (state->current_tab == TAB_CALCULATOR) {
              // Check mini-map clicks first (top area, 50-350 pixels from top)
              int minimap_x = 50;
              int minimap_y = 50;
              int minimap_w = RENDER_WIDTH - 100;
              int minimap_h = 300;
              
              if (x >= minimap_x && x < minimap_x + minimap_w && 
                  y >= minimap_y && y < minimap_y + minimap_h) {
                  // Click is in mini-map area
                  handle_minimap_click(state, x, y, minimap_x, minimap_y, minimap_w, minimap_h);
                  return;
              }
              
              // Check calculator buttons (4 columns, 5 rows)
              int btn_x = state->calc_button_x;
              int btn_y = state->calc_button_y;
              int btn_w = state->calc_button_width;
              int btn_h = state->calc_button_height;
              int spacing = state->calc_button_spacing;
              
              // Safety check: only process clicks if buttons have been drawn (positions initialized)
              if (btn_w > 0 && btn_h > 0 && x >= btn_x && y >= btn_y) {
                  int col = (x - btn_x) / (btn_w + spacing);
                  int row = (y - btn_y) / (btn_h + spacing);
                  
                  if (col >= 0 && col < 4 && row >= 0 && row < 5) {
                      // Check if click is within button bounds (not in spacing)
                      int local_x = (x - btn_x) % (btn_w + spacing);
                      int local_y = (y - btn_y) % (btn_h + spacing);
                      
                      if (local_x < btn_w && local_y < btn_h) {
                          int idx = row * 4 + col;
                          if (idx < 19) {
                              handle_calculator_button(state, idx);
                          }
                      }
                  }
              }
              
              // Check mode selector buttons
              int mode_x = state->mode_button_x;
              int mode_y = state->mode_button_y;
              int mode_w = state->mode_button_width;
              int mode_h = state->mode_button_height;
              
              // Safety check for mode buttons
              if (mode_w > 0 && mode_h > 0 && x >= mode_x && x < mode_x + mode_w && y >= mode_y) {
                  int mode_idx = (y - mode_y) / (mode_h + 5);
                  if (mode_idx >= 0 && mode_idx < MODE_COUNT) {
                      // Check if within button bounds
                      int local_y = (y - mode_y) % (mode_h + 5);
                      if (local_y < mode_h) {
                          handle_mode_button(state, mode_idx);
                      }
                  }
              }
          }
}

void handle_input(AppState* state, SDL_Event* event) {
    // Silent event handling (no debug spam)
    
    if (event->type == SDL_KEYDOWN) {
        // Silent key presses (no terminal spam)
    }
    
    // Handle input events through global input manager
    if (g_input_manager && input_manager_handle_event(g_input_manager, event)) {
        printf("DEBUG: InputManager handled event\n");
        return; // Event was handled by input manager
    }
    
    switch (event->type) {
        case SDL_KEYDOWN:
            // Handle LLM tab keyboard input FIRST (before hotkeys)
            if (state->current_tab == TAB_LLM) {
                handle_llm_tab_key(state, event->key.keysym.sym);
                break;
            }
            
            // Handle input mode toggle (only if NOT in LLM tab)
            if (event->key.keysym.sym == SDLK_i) {
                state->input_active = !state->input_active;
                if (state->input_active) {
                    SDL_StartTextInput();
                    printf("Input activated\n");
                } else {
                    SDL_StopTextInput();
                    printf("Input deactivated\n");
                }
                break;
            }
            
            // Handle Training tab keyboard input
            if (state->current_tab == TAB_TRAINING) {
                handle_training_tab_keydown(state, event->key.keysym.sym);
                break;
            }
            
            // Handle Research tab keyboard input
            if (state->current_tab == TAB_RESEARCH) {
                handle_research_tab_keydown(state, event->key.keysym.sym);
                break;
            }
            
            
            
            // If input is active, only handle input-related keys
            if (state->input_active) {
                switch (event->key.keysym.sym) {
                    case SDLK_RETURN:
                        printf("=== ENTER PRESSED - PROCESSING INPUT ===\n");
                        process_input(state);
                        printf("=== INPUT PROCESSED ===\n");
                        break;
                    case SDLK_BACKSPACE:
                        handle_backspace(state);
                        break;
                    case SDLK_ESCAPE:
                        state->input_active = false;
                        SDL_StopTextInput();
                        printf("Input cancelled\n");
                        break;
                }
                break;  // Don't process other commands when input is active
            }
            
            // Normal command processing (only when input is NOT active)
            switch (event->key.keysym.sym) {
                case SDLK_m:
                    state->mode = (state->mode + 1) % MODE_COUNT;
                    printf("Mode: %d\n", state->mode);
                    break;
                case SDLK_f:
                    // Toggle factor highlighting
                    state->show_factors = !state->show_factors;
                    printf("Factor highlighting: %s\n", state->show_factors ? "ON" : "OFF");
                    break;
                case SDLK_a:
                    // Toggle between all factors and selected only
                    if (state->analysis_history.selected_index == -1) {
                        state->analysis_history.selected_index = state->analysis_history.count - 1;  // Select last
                        printf("Showing factors for selected number only\n");
                    } else {
                        state->analysis_history.selected_index = -1;  // Show all
                        printf("Showing factors for ALL numbers\n");
                    }
                    break;
                case SDLK_SPACE:
                    state->animate = !state->animate;
                    break;
                case SDLK_c:
                    // Toggle crystalline mode for clock sudoku
                    if (state->mode == MODE_CLOCK_SUDOKU) {
                        state->clock_crystalline_mode = !state->clock_crystalline_mode;
                        printf("Clock mode: %s\n", state->clock_crystalline_mode ? "CRYSTALLINE (12-fold)" : "NATURAL (4-fold X)");
                    } else {
                        state->show_connections = !state->show_connections;
                    }
                    break;
                case SDLK_g:
                    state->show_clock_grid = !state->show_clock_grid;
                    break;
                case SDLK_p:
                    state->show_control_panel = !state->show_control_panel;
                    break;
                case SDLK_r:
                    if (state->is_recording) stop_recording(state);
                    else start_recording(state);
                    break;
                case SDLK_s:
                    save_snapshot(state);
                    break;
                case SDLK_e:
                    printf("Expanding primes (old system)...\n");
                    expand_primes(state);
                    break;
                case SDLK_TAB:
                    state->current_tab = (state->current_tab + 1) % TAB_COUNT;
                    // Update InputManager's current tab
                    if (g_input_manager) {
                        input_manager_set_tab(g_input_manager, state->current_tab);
                    }
                    break;
                case SDLK_PLUS:
                case SDLK_EQUALS:
                case SDLK_KP_PLUS:
                    state->zoom = math_min(state->zoom * 1.2, MAX_ZOOM);
                    printf("Zoom in: %.2f\n", state->zoom);
                    break;
                case SDLK_MINUS:
                case SDLK_UNDERSCORE:
                case SDLK_KP_MINUS:
                    state->zoom = math_max(state->zoom / 1.2, MIN_ZOOM);
                    printf("Zoom out: %.2f\n", state->zoom);
                    break;
                case SDLK_LEFT:
                case SDLK_KP_4:
                    state->offset_x -= 20;
                    printf("Pan left: %.0f\n", state->offset_x);
                    break;
                case SDLK_RIGHT:
                case SDLK_KP_6:
                    state->offset_x += 20;
                    printf("Pan right: %.0f\n", state->offset_x);
                    break;
                case SDLK_UP:
                case SDLK_KP_8:
                    state->offset_y -= 20;
                    printf("Pan up: %.0f\n", state->offset_y);
                    break;
                case SDLK_DOWN:
                case SDLK_KP_2:
                    state->offset_y += 20;
                    printf("Pan down: %.0f\n", state->offset_y);
                    break;
                case SDLK_0:
                    state->zoom = 1.0;
                    state->offset_x = 0;
                    state->offset_y = 0;
                    state->rotation = 0;
                    printf("Reset view\n");
                    break;
                case SDLK_ESCAPE:
                case SDLK_q:
                    {
                        SDL_Event quit_event;
                        quit_event.type = SDL_QUIT;
                        SDL_PushEvent(&quit_event);
                    }
                    break;
            }
            break;
        
        case SDL_TEXTINPUT:
            if (state->current_tab == TAB_LLM) {
                handle_llm_tab_text_input(state, event->text.text);
            } else if (state->current_tab == TAB_TRAINING) {
                handle_training_tab_text_input(state, event->text.text);
            } else if (state->current_tab == TAB_RESEARCH) {
                handle_research_tab_text_input(state, event->text.text);
            
            } else if (state->input_active) {
                // Filter out command keys (F, A, M, etc.) from text input
                char c = event->text.text[0];
                if (c >= '0' && c <= '9') {
                    // Only allow digits in input mode
                    printf("TEXT INPUT: '%s'\n", event->text.text);
                    handle_text_input(state, event->text.text);
                    printf("Buffer now: '%s'\n", state->input_buffer);
                }
            }
            break;
            
        case SDL_MOUSEBUTTONDOWN:
            state->dragging_slider = true;
            
            // CRITICAL: Check submenu FIRST (Y < 40, X >= 200)
            if (event->button.y < SUBMENU_HEIGHT && event->button.x >= SIDEBAR_WIDTH) {
                // Submenu click - handle tab switching
                handle_mouse_click(state, event->button.x, event->button.y);
            } else if (event->button.x < SIDEBAR_WIDTH) {
                // Sidebar click - handle tab switching
                handle_mouse_click(state, event->button.x, event->button.y);
            } else {
                // Content area click - route to appropriate tab
                if (state->current_tab == TAB_TRAINING) {
                    extern void handle_training_tab_mouse_down(AppState* state, int x, int y);
                    handle_training_tab_mouse_down(state, event->button.x, event->button.y);
                } else if (state->current_tab == TAB_LLM) {
                    extern void handle_llm_tab_mouse_down(AppState* state, int x, int y);
                    handle_llm_tab_mouse_down(state, event->button.x, event->button.y);
                } else if (state->current_tab == TAB_VIDEO_GENERATOR) {
                    extern void handle_video_tab_mouse_down(AppState* state, int x, int y);
                    handle_video_tab_mouse_down(state, event->button.x, event->button.y);
                } else if (state->current_tab == TAB_RESEARCH) {
                    extern void handle_research_tab_mouse_down(AppState* state, int x, int y);
                    handle_research_tab_mouse_down(state, event->button.x, event->button.y);
                } else if (state->current_tab == TAB_URL_MANAGER) {
                    extern void handle_url_manager_tab_mouse_down(AppState* state, int x, int y);
                    handle_url_manager_tab_mouse_down(state, event->button.x, event->button.y);
                } else if (state->current_tab == TAB_DOWNLOADED_FILES) {
                    extern void handle_downloaded_files_tab_mouse_down(AppState* state, int x, int y);
                    handle_downloaded_files_tab_mouse_down(state, event->button.x, event->button.y);
                } else if (state->current_tab == TAB_MODELS) {
                    extern void handle_models_tab_mouse_down(SDL_MouseButtonEvent* event, AppState* state);
                    handle_models_tab_mouse_down(&event->button, state);
                } else if (state->current_tab == TAB_CRAWLER) {
                    extern void handle_crawler_tab_mouse_down(SDL_MouseButtonEvent* event, AppState* state);
                    handle_crawler_tab_mouse_down(&event->button, state);
                } else {
                    // For non-Crystalline tabs, use old handler
                    handle_mouse_click(state, event->button.x, event->button.y);
                }
            }
            break;
            
        case SDL_MOUSEBUTTONUP:
            state->dragging_slider = false;
            
            // CRITICAL: Check submenu FIRST (Y < 40, X >= 200)
            if (event->button.y < SUBMENU_HEIGHT && event->button.x >= SIDEBAR_WIDTH) {
                // Submenu click - handle tab switching
                handle_mouse_click(state, event->button.x, event->button.y);
            } else if (event->button.x < SIDEBAR_WIDTH) {
                // Sidebar click - already handled in BUTTONDOWN, but process for consistency
                handle_mouse_click(state, event->button.x, event->button.y);
            } else {
                // Content area click - route to appropriate tab
                if (state->current_tab == TAB_TRAINING) {
                    extern void handle_training_tab_mouse_up(AppState* state, int x, int y);
                    handle_training_tab_mouse_up(state, event->button.x, event->button.y);
                } else if (state->current_tab == TAB_LLM) {
                    extern void handle_llm_tab_mouse_up(AppState* state, int x, int y);
                    handle_llm_tab_mouse_up(state, event->button.x, event->button.y);
                } else if (state->current_tab == TAB_VIDEO_GENERATOR) {
                    extern void handle_video_tab_mouse_up(AppState* state, int x, int y);
                    handle_video_tab_mouse_up(state, event->button.x, event->button.y);
                } else if (state->current_tab == TAB_RESEARCH) {
                    extern void handle_research_tab_mouse_up(AppState* state, int x, int y);
                    handle_research_tab_mouse_up(state, event->button.x, event->button.y);
                } else if (state->current_tab == TAB_URL_MANAGER) {
                    extern void handle_url_manager_tab_mouse_up(AppState* state, int x, int y);
                    handle_url_manager_tab_mouse_up(state, event->button.x, event->button.y);
                } else if (state->current_tab == TAB_DOWNLOADED_FILES) {
                    extern void handle_downloaded_files_tab_mouse_up(AppState* state, int x, int y);
                    handle_downloaded_files_tab_mouse_up(state, event->button.x, event->button.y);
                } else if (state->current_tab == TAB_MODELS) {
                    extern void handle_models_tab_mouse_up(SDL_MouseButtonEvent* event, AppState* state);
                    handle_models_tab_mouse_up(&event->button, state);
                } else if (state->current_tab == TAB_CRAWLER) {
                    extern void handle_crawler_tab_mouse_up(SDL_MouseButtonEvent* event, AppState* state);
                    handle_crawler_tab_mouse_up(&event->button, state);
                }
            }
            break;
            
        case SDL_MOUSEMOTION:
            state->mouse_x = event->motion.x;
            state->mouse_y = event->motion.y;
            
            // Route motion events to crystalline UI tabs for hover states
            if (state->current_tab == TAB_TRAINING) {
                extern void handle_training_tab_mouse_motion(AppState* state, int x, int y);
                handle_training_tab_mouse_motion(state, event->motion.x, event->motion.y);
            } else if (state->current_tab == TAB_LLM) {
                extern void handle_llm_tab_mouse_motion(AppState* state, int x, int y);
                handle_llm_tab_mouse_motion(state, event->motion.x, event->motion.y);
            } else if (state->current_tab == TAB_VIDEO_GENERATOR) {
                extern void handle_video_tab_mouse_motion(AppState* state, int x, int y);
                handle_video_tab_mouse_motion(state, event->motion.x, event->motion.y);
            } else if (state->current_tab == TAB_RESEARCH) {
                extern void handle_research_tab_mouse_motion(AppState* state, int x, int y);
                handle_research_tab_mouse_motion(state, event->motion.x, event->motion.y);
            } else if (state->current_tab == TAB_URL_MANAGER) {
                extern void handle_url_manager_tab_mouse_motion(AppState* state, int x, int y);
                handle_url_manager_tab_mouse_motion(state, event->motion.x, event->motion.y);
            } else if (state->current_tab == TAB_DOWNLOADED_FILES) {
                extern void handle_downloaded_files_tab_mouse_motion(AppState* state, int x, int y);
                handle_downloaded_files_tab_mouse_motion(state, event->motion.x, event->motion.y);
            } else if (state->current_tab == TAB_MODELS) {
                extern void handle_models_tab_mouse_motion(SDL_MouseMotionEvent* event, AppState* state);
                handle_models_tab_mouse_motion(&event->motion, state);
            } else if (state->current_tab == TAB_CRAWLER) {
                extern void handle_crawler_tab_mouse_motion(SDL_MouseMotionEvent* event, AppState* state);
                handle_crawler_tab_mouse_motion(&event->motion, state);
            }
            
            if (event->motion.state & SDL_BUTTON_LMASK) {
                if (!state->dragging_slider) {
                    state->offset_x += event->motion.xrel;
                    state->offset_y += event->motion.yrel;
                }
            }
            if (event->motion.state & SDL_BUTTON_RMASK) {
                state->rotation += event->motion.xrel * 0.01;
            }
            break;
            
        case SDL_MOUSEWHEEL:
            if (state->current_tab == TAB_RESEARCH) {
                handle_research_tab_scroll(state, event->wheel.y);
            } else if (state->current_tab == TAB_TRAINING) {
                extern void handle_training_tab_scroll(AppState* state, int wheel_y);
                handle_training_tab_scroll(state, event->wheel.y);
            } else {
                if (event->wheel.y > 0) {
                    state->zoom = math_min(state->zoom * 1.2, MAX_ZOOM);
                    // Silent zoom
                } else if (event->wheel.y < 0) {
                    state->zoom = math_max(state->zoom / 1.2, MIN_ZOOM);
                    // Silent zoom
                }
            }
            break;
    }
}

// Helper function to sync hierarchical tabs with legacy tab system
static void sync_hierarchical_to_legacy_tab(AppState* state) {
    switch (state->main_tab) {
        case MAIN_TAB_VISUALIZATION:
            switch (state->sub_tab.viz_sub) {
                case VIZ_SUB_PRIME_SPIRAL:
                    state->current_tab = TAB_PRIME_SPIRAL;
                    break;
                case VIZ_SUB_CALCULATOR:
                    state->current_tab = TAB_CALCULATOR;
                    break;
                case VIZ_SUB_SPHERES:
                    state->current_tab = TAB_SPHERES;
                    break;
                case VIZ_SUB_PRIME_FOLDING:
                    state->current_tab = TAB_PRIME_FOLDING;
                    break;
                case VIZ_SUB_VIDEO_GENERATOR:
                    state->current_tab = TAB_VIDEO_GENERATOR;
                    break;
                default:
                    break;
            }
            break;
        case MAIN_TAB_AI:
            switch (state->sub_tab.ai_sub) {
                case AI_SUB_LLM:
                    state->current_tab = TAB_LLM;
                    break;
                case AI_SUB_TRAINING:
                    state->current_tab = TAB_TRAINING;
                    break;
                case AI_SUB_RESEARCH:
                    state->current_tab = TAB_RESEARCH;
                    break;
                case AI_SUB_CRAWLER:
                    state->current_tab = TAB_CRAWLER;
                    break;
                default:
                    break;
            }
            break;
        case MAIN_TAB_MODELS:
            state->current_tab = TAB_MODELS;
            break;
        case MAIN_TAB_SYSTEM:
            // Benchmark tab doesn't exist in legacy system yet
            state->current_tab = TAB_PRIME_SPIRAL; // Default fallback
            break;
        case MAIN_TAB_DATA:
            switch (state->sub_tab.data_sub) {
                case DATA_SUB_URL_MANAGER:
                    state->current_tab = TAB_URL_MANAGER;
                    break;
                case DATA_SUB_DOWNLOADED_FILES:
                    state->current_tab = TAB_DOWNLOADED_FILES;
                    break;
                default:
                    state->current_tab = TAB_URL_MANAGER;
                    break;
            }
            break;
        default:
            break;
    }
}

void render(AppState* state) {
    SDL_SetRenderDrawColor(state->renderer, 10, 15, 20, 255);
    SDL_RenderClear(state->renderer);
    
    // Sync hierarchical tabs to legacy tab for backward compatibility
    sync_hierarchical_to_legacy_tab(state);
    
    // CRITICAL FIX: Sync input manager's current tab to match AppState
    // Without this, input manager thinks we're still on the old tab
    // and all inputs are skipped due to tab_id mismatch
    if (g_input_manager) {
        input_manager_set_tab(g_input_manager, state->current_tab);
    }
    
    // Render new UI: left sidebar and submenu
    render_left_sidebar(state->renderer, state);
    render_submenu(state->renderer, state);
    
    // Get layout for current tab (reserved for future layout-based rendering)
    // TabLayout layout = get_tab_layout(state->current_tab, WINDOW_WIDTH, WINDOW_HEIGHT);
    
    switch (state->current_tab) {
        case TAB_PRIME_SPIRAL:
            draw_visualization(state->renderer, state);
            if (state->show_control_panel) {
                draw_control_panel(state->renderer, state);
            }
            break;
        case TAB_CALCULATOR:
            // Calculator with mini-map visualization
            draw_calculator_with_minimap(state->renderer, state);
            if (state->show_control_panel) {
                draw_control_panel(state->renderer, state);
            }
            break;
        case TAB_SPHERES:
            draw_spheres(state->renderer, state);
            if (state->show_control_panel) {
                draw_control_panel(state->renderer, state);
            }
            break;
        case TAB_PRIME_FOLDING:
            // Folding visualization - use main visualization for now
            draw_visualization(state->renderer, state);
            if (state->show_control_panel) {
                draw_control_panel(state->renderer, state);
            }
            break;
        case TAB_VIDEO_GENERATOR:
            draw_video_tab(state->renderer, state);
            break;
        case TAB_LLM:
            draw_llm_tab(state->renderer, state);
            break;
        case TAB_TRAINING:
            draw_training_tab(state->renderer, state);
            break;
        case TAB_RESEARCH:
            draw_research_tab(state->renderer, state);
            break;
        case TAB_CRAWLER:
            // Render crawler tab with Crystalline UI
            render_crawler_tab(state->renderer, state);
            break;
        case TAB_MODELS:
            // Render models tab with Crystalline UI
            render_models_tab(state->renderer, state);
            break;
        case TAB_URL_MANAGER:
            // Draw URL manager tab
            draw_url_manager_tab(state->renderer, state);
            break;
        case TAB_DOWNLOADED_FILES:
            // Draw downloaded files viewer tab
            draw_downloaded_files_tab(state->renderer, state);
            break;
        case TAB_COUNT:
            break;
    }
       
       if (state->animate) {
        state->animation_phase += 0.016;
    }
    
    static uint32_t last_time = 0;
    static int frame_count = 0;
    uint32_t current_time = SDL_GetTicks();
    frame_count++;
    if (current_time - last_time >= 1000) {
        state->fps = frame_count;
        frame_count = 0;
        last_time = current_time;
    }
    
    if (state->is_recording) {
        record_frame(state);
    }
    
    SDL_RenderPresent(state->renderer);
}

int main(int argc, char* argv[]) {
    printf("\n═══════════════════════════════════════════════════════════════\n");
    printf("        HYPER PRIME SPIRAL - PRIME MATHEMATICS EDITION\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("Controls:\n");
    printf("  M - Cycle modes | F - Cycle folding | Tab - Switch tabs\n");
    printf("  Space - Animate | +/- - Zoom | Arrows - Pan | 0 - Reset\n");
    printf("  E - Expand primes | R - Record | S - Snapshot | Q - Quit\n");
    printf("═══════════════════════════════════════════════════════════════\n\n");
    
    // Parse command line arguments
    const char* workspace_path = NULL;
    bool create_workspace = false;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--workspace") == 0 && i + 1 < argc) {
            workspace_path = argv[i + 1];
            i++;
        } else if (strcmp(argv[i], "--create") == 0) {
            create_workspace = true;
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("Usage: %s [OPTIONS]\n", argv[0]);
            printf("\nOptions:\n");
            printf("  --workspace PATH    Use custom workspace directory\n");
            printf("  --create            Create workspace if it doesn't exist\n");
            printf("  --help, -h          Show this help message\n");
            printf("\nExamples:\n");
            printf("  %s                                    # Use default directories\n", argv[0]);
            printf("  %s --workspace ~/my_project           # Use custom workspace\n", argv[0]);
            printf("  %s --workspace ~/new_project --create # Create new workspace\n", argv[0]);
            printf("\nEnvironment Variables:\n");
            printf("  CRYSTALLINE_WORKSPACE    Default workspace directory\n");
            return 0;
        }
    }
    
    // Check environment variable if no command line argument
    if (!workspace_path) {
        workspace_path = getenv("CRYSTALLINE_WORKSPACE");
    }
    
    AppState* state = init_app();
    if (!state) {
        fprintf(stderr, "Failed to initialize\n");
        return 1;
    }
    
    // Smooth fade-out transition from loading screen to main UI
    printf("Loading complete - transitioning to main UI...\n");
    for (int alpha = 255; alpha >= 0; alpha -= 8) {
        loading_screen_render(state->renderer, &g_loading_screen, (uint8_t)alpha);
        SDL_Delay(16); // ~60fps
    }
    
    // Set global pointer for lattice cache access
    app_state_global = state;
    
    // Initialize status flags
    state->abacus_initializing = false;
    state->abacus_ready = false;
    state->model_loading = false;
    state->model_ready = false;
    
    // CRITICAL: Start control thread for background initialization
    // This implements MASTER_PLAN architecture - main loop continues immediately
    // Heavy initialization (abacus, model loading) happens in background
    extern void start_control_thread(AppState* state);
    start_control_thread(state);
    
    // Initialize workspace system
    extern void workspace_init(AppState* state, const char* workspace_path);
    workspace_init(state, workspace_path);
    
    // NOTE: Model loading now happens in control thread (background)
    // The code below has been moved to control_thread.c
    // Main loop continues immediately - UI is responsive during initialization
    
    if (create_workspace && workspace_path) {
        extern int workspace_create_directories(AppState* state);
        workspace_create_directories(state);
    }
    
    SDL_Event event;
    bool running = true;
    
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else {
                handle_input(state, &event);
            }
        }
        
        // Training now runs in separate thread (see training_thread.c)
        // UI remains responsive during training
        
        render(state);
        SDL_Delay(16);
    }
    
    cleanup(state);
    return 0;
}
