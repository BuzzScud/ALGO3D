// app/ui_enhanced.c - Enhanced UI with better layout and controls
// Replaces ui.c with improved organization

#include "app_common.h"
#include <SDL2/SDL_ttf.h>

static TTF_Font* g_font = NULL;

int init_font_system(void) {
    if (TTF_Init() == -1) {
        fprintf(stderr, "TTF_Init failed: %s\n", TTF_GetError());
        return 0;
    }
    
    // Try to load a system font (supports Ubuntu, Debian, CentOS, Fedora, Arch)
    // Check for environment variable override first
    const char* env_font = getenv("CLLM_FONT_PATH");
    if (env_font) {
        g_font = TTF_OpenFont(env_font, 14);
        if (g_font) {
            printf("Loaded font from CLLM_FONT_PATH: %s\n", env_font);
            return 1;
        }
        fprintf(stderr, "Warning: CLLM_FONT_PATH set but font not found: %s\n", env_font);
    }
    
    const char* font_paths[] = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",              // Ubuntu/Debian
        "/usr/share/fonts/dejavu/DejaVuSans.ttf",                       // CentOS/RHEL/Fedora
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf", // Ubuntu/Debian
        "/usr/share/fonts/liberation/LiberationSans-Regular.ttf",       // CentOS/RHEL/Fedora
        "/usr/share/fonts/truetype/freefont/FreeSans.ttf",              // Ubuntu/Debian
        "/usr/share/fonts/gnu-free/FreeSans.ttf",                       // CentOS/RHEL/Fedora
        "/usr/share/fonts/TTF/DejaVuSans.ttf",                          // Arch Linux
        "/System/Library/Fonts/Helvetica.ttc",                          // macOS
        "C:\\Windows\\Fonts\\arial.ttf",                                 // Windows
        NULL
    };
    
    for (int i = 0; font_paths[i] != NULL; i++) {
        g_font = TTF_OpenFont(font_paths[i], 14);
        if (g_font) {
            printf("Loaded font: %s\n", font_paths[i]);
            return 1;
        }
    }
    
    fprintf(stderr, "Failed to load any font\n");
    return 0;
}

void cleanup_font_system(void) {
    if (g_font) {
        TTF_CloseFont(g_font);
        g_font = NULL;
    }
    TTF_Quit();
}

TTF_Font* get_global_font(void) {
    return g_font;
}

void draw_text(SDL_Renderer* renderer, const char* text, int x, int y, SDL_Color color) {
    if (!g_font || !text || text[0] == '\0') {
        return;
    }
    
    SDL_Surface* surface = TTF_RenderText_Blended(g_font, text, color);
    if (!surface) {
        return;
    }
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture) {
        SDL_Rect dest = {x, y, surface->w, surface->h};
        SDL_RenderCopy(renderer, texture, NULL, &dest);
        SDL_DestroyTexture(texture);
    }
    
    SDL_FreeSurface(surface);
}

// Draw tabs at the top
void draw_tabs(SDL_Renderer* renderer, AppState* state) {
    const char* tab_names[] = {
        "Visualization",
        "Calculator", 
        "Spheres",
        "Folding",
        "Video",
        "LLM",
        "Training",
        "Research",
        "Crawler",
        "Models"
    };
    
    int tab_width = RENDER_WIDTH / TAB_COUNT;
    
    for (int i = 0; i < (int)TAB_COUNT; i++) {
        SDL_Rect tab_rect = {i * tab_width, 0, tab_width, 40};
        
        if (i == (int)state->current_tab) {
            SDL_SetRenderDrawColor(renderer, 60, 80, 120, 255);
        } else {
            SDL_SetRenderDrawColor(renderer, 30, 40, 60, 255);
        }
        
        SDL_RenderFillRect(renderer, &tab_rect);
        SDL_SetRenderDrawColor(renderer, 100, 150, 200, 255);
        SDL_RenderDrawRect(renderer, &tab_rect);
        
        SDL_Color text_color = {255, 255, 255, 255};
        draw_text(renderer, tab_names[i], i * tab_width + 10, 12, text_color);
    }
}

// Draw enhanced control panel with better organization
void draw_control_panel(SDL_Renderer* renderer, AppState* state) {
    // Background (control panel is on the right side, after visualization area)
    int panel_x = RENDER_OFFSET_X + RENDER_WIDTH;
    SDL_Rect panel_rect = {panel_x, RENDER_OFFSET_Y, CONTROL_PANEL_WIDTH, WINDOW_HEIGHT - RENDER_OFFSET_Y};
    SDL_SetRenderDrawColor(renderer, 20, 30, 40, 255);
    SDL_RenderFillRect(renderer, &panel_rect);
    
    SDL_SetRenderDrawColor(renderer, 100, 150, 200, 255);
    SDL_RenderDrawRect(renderer, &panel_rect);
    
    // Colors
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color green = {100, 255, 100, 255};
    SDL_Color yellow = {255, 255, 100, 255};
    SDL_Color cyan = {100, 255, 255, 255};
    SDL_Color red = {255, 100, 100, 255};
    
    int x_base = panel_x + 10;
    int y_pos = RENDER_OFFSET_Y + 10;
    
    // ===== SECTION 1: MODE INFO =====
    draw_text(renderer, "MODE", x_base, y_pos, green);
    y_pos += 20;
    
    const char* mode_names[] = {
        "Ulam Spiral",
        "Recursive Fractal",
        "Harmonic Resonance",
        "Hyperdim Projection",
        "Self-Similar",
        "Clock Sudoku",
        "Natural Prime Lattice",
        "Folded Origami"
    };
    char mode_text[64];
    snprintf(mode_text, sizeof(mode_text), "%s", mode_names[state->mode]);
    draw_text(renderer, mode_text, x_base, y_pos, white);
    y_pos += 20;
    
    // Show clock mode if in Clock Sudoku
    if (state->mode == MODE_CLOCK_SUDOKU) {
        const char* clock_mode = state->clock_crystalline_mode ? "Crystalline (12-fold)" : "Natural (4-fold X)";
        SDL_Color mode_color = state->clock_crystalline_mode ? (SDL_Color){100, 255, 100, 255} : (SDL_Color){255, 200, 100, 255};
        draw_text(renderer, clock_mode, x_base + 5, y_pos, mode_color);
        y_pos += 20;
        draw_text(renderer, "Press C to toggle", x_base + 5, y_pos, cyan);
    }
    y_pos += 5;
    
    // ===== SECTION 2: PRIME COUNT CONTROL =====
    SDL_SetRenderDrawColor(renderer, 50, 70, 90, 255);
    SDL_Rect prime_section = {x_base, y_pos, 300, 80};
    SDL_RenderFillRect(renderer, &prime_section);
    SDL_SetRenderDrawColor(renderer, 100, 150, 200, 255);
    SDL_RenderDrawRect(renderer, &prime_section);
    y_pos += 5;
    
    draw_text(renderer, "PRIME COUNT", x_base + 5, y_pos, green);
    y_pos += 20;
    
    char prime_count_text[64];
    snprintf(prime_count_text, sizeof(prime_count_text), "Visible: %d / %d", 
             (int)state->visible_primes, (int)state->prime_count);
    draw_text(renderer, prime_count_text, x_base + 5, y_pos, white);
    y_pos += 20;
    
    // +/- buttons for prime count
    SDL_Rect minus_btn = {x_base + 5, y_pos, 40, 25};
    SDL_Rect plus_btn = {x_base + 255, y_pos, 40, 25};
    
    SDL_SetRenderDrawColor(renderer, 80, 100, 120, 255);
    SDL_RenderFillRect(renderer, &minus_btn);
    SDL_RenderFillRect(renderer, &plus_btn);
    SDL_SetRenderDrawColor(renderer, 150, 180, 200, 255);
    SDL_RenderDrawRect(renderer, &minus_btn);
    SDL_RenderDrawRect(renderer, &plus_btn);
    
    draw_text(renderer, "-", x_base + 18, y_pos + 5, white);
    draw_text(renderer, "+", x_base + 268, y_pos + 5, white);
    
    // Slider for prime count
    int slider_x = x_base + 50;
    int slider_width = 200;
    SDL_Rect slider_bg = {slider_x, y_pos + 8, slider_width, 10};
    SDL_SetRenderDrawColor(renderer, 40, 50, 60, 255);
    SDL_RenderFillRect(renderer, &slider_bg);
    
    int slider_pos = (int)((state->visible_primes * slider_width) / state->prime_count);
    SDL_Rect slider_handle = {slider_x + slider_pos - 5, y_pos + 3, 10, 20};
    SDL_SetRenderDrawColor(renderer, 100, 200, 100, 255);
    SDL_RenderFillRect(renderer, &slider_handle);
    
    y_pos += 35;
    
    // ===== SECTION 3: PRIME INPUT =====
    SDL_SetRenderDrawColor(renderer, 50, 70, 90, 255);
    SDL_Rect input_section = {x_base, y_pos, 300, 120};
    SDL_RenderFillRect(renderer, &input_section);
    SDL_SetRenderDrawColor(renderer, 100, 150, 200, 255);
    SDL_RenderDrawRect(renderer, &input_section);
    
    // Highlight if input is active
    if (state->input_active) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 100, 255);
        SDL_RenderDrawRect(renderer, &input_section);
    }
    
    y_pos += 5;
    
    draw_text(renderer, "PRIME INPUT", x_base + 5, y_pos, green);
    y_pos += 20;
    
    // Input field (clickable area)
    SDL_Rect input_field = {x_base + 5, y_pos, 290, 30};
    if (state->input_active) {
        SDL_SetRenderDrawColor(renderer, 60, 80, 100, 255);
    } else {
        SDL_SetRenderDrawColor(renderer, 40, 50, 60, 255);
    }
    SDL_RenderFillRect(renderer, &input_field);
    SDL_SetRenderDrawColor(renderer, 120, 150, 180, 255);
    SDL_RenderDrawRect(renderer, &input_field);
    
    // Store input field rect for click detection
    state->input_field_rect = input_field;
    
    // Display input buffer or placeholder
    if (state->input_buffer[0] != '\0') {
        draw_text(renderer, state->input_buffer, x_base + 10, y_pos + 8, yellow);
    } else {
        draw_text(renderer, "Click to enter number...", x_base + 10, y_pos + 8, cyan);
    }
    y_pos += 35;
    
    // Instructions
    if (state->input_active) {
        draw_text(renderer, "Type number, press ENTER", x_base + 5, y_pos, cyan);
    } else {
        draw_text(renderer, "Press 'I' or click above", x_base + 5, y_pos, cyan);
    }
    y_pos += 20;
    
    // Analyze button
    SDL_Rect analyze_btn = {x_base + 5, y_pos, 290, 25};
    SDL_SetRenderDrawColor(renderer, 80, 120, 80, 255);
    SDL_RenderFillRect(renderer, &analyze_btn);
    SDL_SetRenderDrawColor(renderer, 120, 180, 120, 255);
    SDL_RenderDrawRect(renderer, &analyze_btn);
    draw_text(renderer, "ANALYZE (Enter)", x_base + 90, y_pos + 5, white);
    
    y_pos += 35;
    
    // ===== SECTION 4: NUMBER ANALYSIS =====
    if (state->number_analysis.number > 0) {
        SDL_SetRenderDrawColor(renderer, 50, 70, 90, 255);
        SDL_Rect analysis_section = {x_base, y_pos, 300, 280};
        SDL_RenderFillRect(renderer, &analysis_section);
        SDL_SetRenderDrawColor(renderer, 100, 150, 200, 255);
        SDL_RenderDrawRect(renderer, &analysis_section);
        y_pos += 5;
        
        draw_text(renderer, "ANALYSIS", x_base + 5, y_pos, green);
        y_pos += 20;
        
        NumberAnalysis* analysis = &state->number_analysis;
        char text[256];
        
        // Number
        snprintf(text, sizeof(text), "Number: %d", analysis->number);
        draw_text(renderer, text, x_base + 5, y_pos, yellow);
        y_pos += 18;
        
        // Prime status
        snprintf(text, sizeof(text), "Prime: %s", analysis->is_prime ? "Yes" : "No");
        SDL_Color prime_color = analysis->is_prime ? green : red;
        draw_text(renderer, text, x_base + 5, y_pos, prime_color);
        y_pos += 18;
        
        // Ring
        snprintf(text, sizeof(text), "Ring: %d", analysis->ring);
        draw_text(renderer, text, x_base + 5, y_pos, white);
        y_pos += 18;
        
        // Angle
        snprintf(text, sizeof(text), "Angle: %.2f degrees", analysis->angle);
        draw_text(renderer, text, x_base + 5, y_pos, white);
        y_pos += 18;
        
        // Clock position
        snprintf(text, sizeof(text), "Position: %d o'clock", analysis->clock_position);
        draw_text(renderer, text, x_base + 5, y_pos, white);
        y_pos += 18;
        
        // Quadrant
        snprintf(text, sizeof(text), "Quadrant: %d", analysis->quadrant);
        draw_text(renderer, text, x_base + 5, y_pos, white);
        y_pos += 18;
        
        // Hop count (distance from center in rings)
        snprintf(text, sizeof(text), "Hops from center: %d", analysis->ring);
        draw_text(renderer, text, x_base + 5, y_pos, cyan);
        y_pos += 18;
        
        // Distance from nearest ring
        snprintf(text, sizeof(text), "Ring distance: %.1f", analysis->ring_distance);
        draw_text(renderer, text, x_base + 5, y_pos, cyan);
        y_pos += 20;
        
        // Factors
        if (analysis->factor_count > 0) {
            draw_text(renderer, "Factors:", x_base + 5, y_pos, green);
            y_pos += 18;
            
            for (int i = 0; i < analysis->factor_count && i < 10; i++) {
                snprintf(text, sizeof(text), "  %d", analysis->factors[i]);
                draw_text(renderer, text, x_base + 5, y_pos, white);
                y_pos += 15;
            }
            
            if (analysis->factor_count > 10) {
                snprintf(text, sizeof(text), "  ... and %d more", analysis->factor_count - 10);
                draw_text(renderer, text, x_base + 5, y_pos, white);
                y_pos += 15;
            }
        }
        
        // Toggle factors button
        y_pos += 5;
        SDL_Rect factors_btn = {x_base + 5, y_pos, 290, 25};
        SDL_SetRenderDrawColor(renderer, 80, 80, 120, 255);
        SDL_RenderFillRect(renderer, &factors_btn);
        SDL_SetRenderDrawColor(renderer, 120, 120, 180, 255);
        SDL_RenderDrawRect(renderer, &factors_btn);
        const char* factor_text = state->show_factors ? "Hide Factors (F)" : "Show Factors (F)";
        draw_text(renderer, factor_text, x_base + 70, y_pos + 5, white);
        
        y_pos += 35;
    }
    
    // ===== SECTION 4.5: LATTICE CACHE CONTROLS =====
    if (state->lattice_cache_initialized) {
        SDL_SetRenderDrawColor(renderer, 50, 70, 90, 255);
        SDL_Rect cache_section = {x_base, y_pos, 300, 100};
        SDL_RenderFillRect(renderer, &cache_section);
        SDL_SetRenderDrawColor(renderer, 100, 150, 200, 255);
        SDL_RenderDrawRect(renderer, &cache_section);
        y_pos += 5;
        
        draw_text(renderer, "LATTICE CACHE", x_base + 5, y_pos, green);
        y_pos += 20;
        
        // Cache stats
        size_t count = 0;
        uint64_t max_prime = 0;
        size_t memory = 0;
        lattice_cache_get_stats(state->lattice_cache, &count, &max_prime, &memory);
        
        char stats[128];
        snprintf(stats, sizeof(stats), "Primes: %lu (max: %lu)", count, max_prime);
        draw_text(renderer, stats, x_base + 5, y_pos, white);
        y_pos += 18;
        
        snprintf(stats, sizeof(stats), "Memory: %.2f MB", memory / (1024.0 * 1024.0));
        draw_text(renderer, stats, x_base + 5, y_pos, white);
        y_pos += 20;
        
        // Generate More button
        SDL_Rect gen_btn = {x_base + 5, y_pos, 140, 25};
        SDL_SetRenderDrawColor(renderer, 80, 120, 80, 255);
        SDL_RenderFillRect(renderer, &gen_btn);
        SDL_SetRenderDrawColor(renderer, 150, 200, 150, 255);
        SDL_RenderDrawRect(renderer, &gen_btn);
        draw_text(renderer, "Generate More", x_base + 15, y_pos + 5, white);
        
        // Auto-extend toggle
        SDL_Rect auto_btn = {x_base + 155, y_pos, 140, 25};
        if (state->auto_extend_on_zoom) {
            SDL_SetRenderDrawColor(renderer, 80, 120, 80, 255);
        } else {
            SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
        }
        SDL_RenderFillRect(renderer, &auto_btn);
        SDL_SetRenderDrawColor(renderer, 150, 180, 200, 255);
        SDL_RenderDrawRect(renderer, &auto_btn);
        draw_text(renderer, state->auto_extend_on_zoom ? "Auto: ON" : "Auto: OFF", 
                 x_base + 165, y_pos + 5, white);
        
        y_pos += 35;
    }
    
    // ===== SECTION 5: CONTROLS =====
    y_pos = WINDOW_HEIGHT - 200;
    
    draw_text(renderer, "CONTROLS", x_base, y_pos, green);
    y_pos += 18;
    draw_text(renderer, "M - Mode", x_base, y_pos, white);
    y_pos += 15;
    draw_text(renderer, "I - Input", x_base, y_pos, white);
    y_pos += 15;
    draw_text(renderer, "F - Factors", x_base, y_pos, white);
    y_pos += 15;
    draw_text(renderer, "G - Generate More", x_base, y_pos, white);
    y_pos += 15;
    draw_text(renderer, "Space - Animate", x_base, y_pos, white);
    y_pos += 15;
    draw_text(renderer, "+/- Zoom", x_base, y_pos, white);
    y_pos += 15;
    draw_text(renderer, "0 - Reset", x_base, y_pos, white);
    y_pos += 15;
    draw_text(renderer, "E - More primes", x_base, y_pos, white);
    y_pos += 15;
    draw_text(renderer, "Q - Quit", x_base, y_pos, white);
    
    // Recording status
    if (state->is_recording) {
        y_pos = WINDOW_HEIGHT - 60;
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_Rect rec_box = {x_base, y_pos, 280, 40};
        SDL_RenderDrawRect(renderer, &rec_box);
        
        char rec_text[64];
            snprintf(rec_text, sizeof(rec_text), "RECORDING: %d frames", state->recording_frames);
            draw_text(renderer, rec_text, x_base + 10, y_pos + 10, red);
        }
    }
