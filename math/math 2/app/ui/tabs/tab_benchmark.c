/*
 * Benchmark Tab - Crystalline UI Version
 * 
 * Features:
 * - Radial layout with 12-fold symmetry
 * - Circular progress indicator
 * - Spiral layout for benchmark results
 * - Sacred geometry panels
 * - Frequency-based colors
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../../app_common.h"
#include "../crystalline/elements.h"
#include "../crystalline/geometry.h"
#include "../crystalline/layout.h"
#include "../crystalline/draw.h"
#include "../crystalline/color.h"

extern TTF_Font* get_global_font(void);

#define MAX_BENCHMARK_HISTORY 100
#define MAX_BENCHMARK_TESTS 20

typedef struct {
    char name[128];
    double duration_ms;
    int operations;
    double ops_per_second;
    time_t timestamp;
    bool success;
} BenchmarkResult;

typedef struct {
    BenchmarkResult results[MAX_BENCHMARK_HISTORY];
    int result_count;
    
    // Current benchmark state
    bool is_running;
    int current_test;
    int total_tests;
    float progress;
    
    // Performance metrics
    double avg_inference_time;
    double avg_tokens_per_second;
    double peak_tokens_per_second;
    double total_tokens_generated;
    double total_time_spent;
    
    // System metrics
    double cpu_usage;
    double memory_usage_mb;
    double gpu_usage;
    
    // UI state
    int scroll_offset;
    int selected_result;
    bool show_details;
} BenchmarkTabState;

static BenchmarkTabState benchmark_state = {
    .result_count = 0,
    .is_running = false,
    .current_test = 0,
    .total_tests = 0,
    .progress = 0.0f,
    .scroll_offset = 0,
    .selected_result = -1,
    .show_details = false
};

// Crystalline UI elements
static CrystallineButton* btn_run = NULL;
static CrystallineButton* btn_clear = NULL;
static CrystallinePanel* panel_metrics = NULL;
static CrystallinePanel* panel_results = NULL;
static CrystallineProgress* progress_benchmark = NULL;
static bool ui_initialized = false;

void benchmark_tab_init(void) {
    memset(&benchmark_state, 0, sizeof(BenchmarkTabState));
    benchmark_state.selected_result = -1;
}

void benchmark_tab_add_result(const char* name, double duration_ms, int operations, bool success) {
    if (benchmark_state.result_count >= MAX_BENCHMARK_HISTORY) {
        // Shift results
        memmove(&benchmark_state.results[0], &benchmark_state.results[1],
                sizeof(BenchmarkResult) * (MAX_BENCHMARK_HISTORY - 1));
        benchmark_state.result_count--;
    }
    
    BenchmarkResult* result = &benchmark_state.results[benchmark_state.result_count++];
    strncpy(result->name, name, sizeof(result->name) - 1);
    result->duration_ms = duration_ms;
    result->operations = operations;
    result->ops_per_second = (operations * 1000.0) / duration_ms;
    result->timestamp = time(NULL);
    result->success = success;
    
    // Update aggregate metrics
    benchmark_state.total_tokens_generated += operations;
    benchmark_state.total_time_spent += duration_ms / 1000.0;
    
    if (result->ops_per_second > benchmark_state.peak_tokens_per_second) {
        benchmark_state.peak_tokens_per_second = result->ops_per_second;
    }
    
    // Calculate averages
    double total_ops = 0;
    double total_time = 0;
    for (int i = 0; i < benchmark_state.result_count; i++) {
        total_ops += benchmark_state.results[i].operations;
        total_time += benchmark_state.results[i].duration_ms;
    }
    benchmark_state.avg_tokens_per_second = (total_ops * 1000.0) / total_time;
    benchmark_state.avg_inference_time = total_time / benchmark_state.result_count;
}

void benchmark_tab_run_tests(void) {
    benchmark_state.is_running = true;
    benchmark_state.current_test = 0;
    benchmark_state.total_tests = 10;
    
    // Simulate benchmark tests
    const char* test_names[] = {
        "Token Generation Speed",
        "Context Window Performance",
        "Batch Processing",
        "Memory Efficiency",
        "Lattice Lookup Speed",
        "Attention Computation",
        "Feed-Forward Speed",
        "Embedding Lookup",
        "Positional Encoding",
        "Full Pipeline"
    };
    
    for (int i = 0; i < benchmark_state.total_tests; i++) {
        benchmark_state.current_test = i;
        benchmark_state.progress = (float)(i + 1) / benchmark_state.total_tests * 100.0f;
        
        // Simulate test execution
        double duration = 10.0 + (rand() % 90);  // 10-100ms
        int operations = 50 + (rand() % 150);  // 50-200 operations
        
        benchmark_tab_add_result(test_names[i], duration, operations, true);
        
        // Small delay to show progress
        SDL_Delay(100);
    }
    
    benchmark_state.is_running = false;
}

void benchmark_tab_render(SDL_Renderer* renderer, TTF_Font* font, int x, int y, int w, int h) {
    (void)font; // Unused in crystalline version
    
    // Initialize UI elements if needed
    if (!ui_initialized) {
        float center_x = x + w / 2.0f;
        float center_y = y + h / 2.0f;
        
        // Create metrics panel (top)
        panel_metrics = crystalline_panel_create(
            CRYSTALLINE_STYLE_CIRCULAR,
            center_x,
            y + 100.0f,
            (float)(w - 40),
            140.0f,
            "PERFORMANCE METRICS",
            get_global_font()
        );
        
        // Create results panel (bottom)
        panel_results = crystalline_panel_create(
            CRYSTALLINE_STYLE_CIRCULAR,
            center_x,
            center_y + 150.0f,
            (float)(w - 40),
            (float)(h - 400),
            "BENCHMARK RESULTS",
            get_global_font()
        );
        
        // Create circular progress indicator
        progress_benchmark = crystalline_progress_create(
            CRYSTALLINE_STYLE_CIRCULAR,
            center_x,
            center_y - 50.0f,
            100.0f,  // radius
            0.0f
        );
        
        // Create buttons in radial layout
        btn_run = crystalline_button_create(
            CRYSTALLINE_STYLE_CIRCULAR,
            center_x - 120.0f,
            center_y - 50.0f,
            70.0f,   // radius
            0.0f,
            "RUN",
            get_global_font()
        );
        
        btn_clear = crystalline_button_create(
            CRYSTALLINE_STYLE_CIRCULAR,
            center_x + 120.0f,
            center_y - 50.0f,
            70.0f,   // radius
            0.0f,
            "CLEAR",
            get_global_font()
        );
        
        ui_initialized = true;
    }
    
    // Colors using crystalline color system
    SDL_Color text_color = crystalline_color_from_frequency(432.0f);  // Base frequency
    SDL_Color accent_color = crystalline_color_from_frequency(639.0f);  // Connection frequency
    SDL_Color success_color = crystalline_color_from_frequency(528.0f);  // Love frequency
    SDL_Color warning_color = crystalline_color_from_frequency(741.0f);  // Awakening frequency
    
    float center_x = x + w / 2.0f;
    
    // Draw title
    CrystallinePoint title_pos = crystalline_point_cartesian(center_x, y + 30.0f);
    crystalline_draw_text_centered(renderer, "PERFORMANCE BENCHMARKS", title_pos, text_color, NULL);
    
    // Draw metrics panel
    crystalline_panel_render(panel_metrics, renderer);
    
    // Draw metrics inside panel
    float metrics_y = y + 70.0f;
    
    char metrics_text[512];
    snprintf(metrics_text, sizeof(metrics_text),
             "Avg Inference: %.2f ms | Avg Tokens/sec: %.1f | Peak: %.1f tokens/sec",
             benchmark_state.avg_inference_time,
             benchmark_state.avg_tokens_per_second,
             benchmark_state.peak_tokens_per_second);
    
    CrystallinePoint metrics_pos = crystalline_point_cartesian(center_x, metrics_y);
    crystalline_draw_text_centered(renderer, metrics_text, metrics_pos, text_color, NULL);
    
    char totals_text[256];
    snprintf(totals_text, sizeof(totals_text),
             "Total Tokens: %.0f | Total Time: %.1f sec | Tests: %d",
             benchmark_state.total_tokens_generated,
             benchmark_state.total_time_spent,
             benchmark_state.result_count);
    
    CrystallinePoint totals_pos = crystalline_point_cartesian(center_x, metrics_y + 25.0f);
    crystalline_draw_text_centered(renderer, totals_text, totals_pos, accent_color, NULL);
    
    char system_text[256];
    snprintf(system_text, sizeof(system_text),
             "CPU: %.1f%% | Memory: %.1f MB | GPU: %.1f%%",
             benchmark_state.cpu_usage,
             benchmark_state.memory_usage_mb,
             benchmark_state.gpu_usage);
    
    CrystallinePoint system_pos = crystalline_point_cartesian(center_x, metrics_y + 50.0f);
    crystalline_draw_text_centered(renderer, system_text, system_pos, warning_color, NULL);
    
    // Update and draw progress
    if (benchmark_state.is_running) {
        crystalline_progress_set_value(progress_benchmark, benchmark_state.progress / 100.0f);
        
        char progress_text[256];
        snprintf(progress_text, sizeof(progress_text),
                 "Test %d/%d: %.1f%%",
                 benchmark_state.current_test + 1,
                 benchmark_state.total_tests,
                 benchmark_state.progress);
        
        CrystallinePoint progress_pos = crystalline_point_cartesian(center_x, y + h / 2.0f - 100.0f);
        crystalline_draw_text_centered(renderer, progress_text, progress_pos, text_color, NULL);
    } else {
        crystalline_progress_set_value(progress_benchmark, 0.0f);
    }
    
    crystalline_progress_render(progress_benchmark, renderer);
    
    // Draw buttons
    crystalline_button_render(btn_run, renderer);
    crystalline_button_render(btn_clear, renderer);
    
    // Draw results panel
    crystalline_panel_render(panel_results, renderer);
    
    // Draw results in spiral layout
    float results_center_x = center_x;
    float results_center_y = y + h / 2.0f + 150.0f;
    float spiral_start_radius = 30.0f;
    float spiral_growth = 5.0f;
    
    for (int i = benchmark_state.result_count - 1; i >= 0 && i >= benchmark_state.result_count - 10; i--) {
        BenchmarkResult* result = &benchmark_state.results[i];
        
        // Calculate spiral position
        int idx = benchmark_state.result_count - 1 - i;
        float angle = idx * 0.5f;  // Spiral angle
        float radius = spiral_start_radius + idx * spiral_growth;
        
        float result_x = results_center_x + radius * math_cos(angle);
        float result_y = results_center_y + radius * math_sin(angle);
        
        char result_text[256];
        snprintf(result_text, sizeof(result_text),
                 "%s: %.1f ms | %.0f ops/s",
                 result->name,
                 result->duration_ms,
                 result->ops_per_second);
        
        SDL_Color result_color = result->success ? success_color : warning_color;
        CrystallinePoint result_pos = crystalline_point_cartesian(result_x, result_y);
        crystalline_draw_text_centered(renderer, result_text, result_pos, result_color, NULL);
    }
    
    // Instructions
    const char* instructions = benchmark_state.is_running ? 
        "Benchmark in progress..." : 
        "Click RUN to start benchmarks, CLEAR to reset";
    
    CrystallinePoint inst_pos = crystalline_point_cartesian(center_x, y + h - 30.0f);
    crystalline_draw_text_centered(renderer, instructions, inst_pos, accent_color, NULL);
}

void handle_benchmark_tab_mouse_down(AppState* state, int x, int y) {
    if (!state || !ui_initialized) return;
    
    // Create SDL event for button handling
    SDL_Event event;
    event.type = SDL_MOUSEBUTTONDOWN;
    event.button.x = x;
    event.button.y = y;
    event.button.button = SDL_BUTTON_LEFT;
    
    // Check buttons
    crystalline_button_handle_mouse(btn_run, &event);
    crystalline_button_handle_mouse(btn_clear, &event);
}

/**
 * Handle benchmark tab mouse motion for hover states
 */
void handle_benchmark_tab_mouse_motion(AppState* state, int x, int y) {
    if (!state || !ui_initialized) return;
    
    // Create SDL event for hover handling
    SDL_Event event;
    event.type = SDL_MOUSEMOTION;
    event.motion.x = x;
    event.motion.y = y;
    
    // Update button hover states
    crystalline_button_handle_mouse(btn_run, &event);
    crystalline_button_handle_mouse(btn_clear, &event);
}

/**
 * Handle benchmark tab mouse up for click callbacks
 */
void handle_benchmark_tab_mouse_up(AppState* state, int x, int y) {
    if (!state || !ui_initialized) return;
    
    // Create SDL event for button release
    SDL_Event event;
    event.type = SDL_MOUSEBUTTONUP;
    event.button.x = x;
    event.button.y = y;
    event.button.button = SDL_BUTTON_LEFT;
    
    // Check Run button
    if (crystalline_button_handle_mouse(btn_run, &event)) {
        if (!benchmark_state.is_running) {
            printf("Running benchmark tests...\n");
            benchmark_tab_run_tests();
        } else {
            printf("Benchmark already running\n");
        }
        return;
    }
    
    // Check Clear button
    if (crystalline_button_handle_mouse(btn_clear, &event)) {
        printf("Clearing benchmark results\n");
        benchmark_state.result_count = 0;
        benchmark_state.total_tokens_generated = 0;
        benchmark_state.total_time_spent = 0;
        benchmark_state.avg_inference_time = 0;
        benchmark_state.avg_tokens_per_second = 0;
        benchmark_state.peak_tokens_per_second = 0;
        return;
    }
}

void benchmark_tab_cleanup(void) {
    if (btn_run) {
        crystalline_button_destroy(btn_run);
        btn_run = NULL;
    }
    if (btn_clear) {
        crystalline_button_destroy(btn_clear);
        btn_clear = NULL;
    }
    if (panel_metrics) {
        crystalline_panel_destroy(panel_metrics);
        panel_metrics = NULL;
    }
    if (panel_results) {
        crystalline_panel_destroy(panel_results);
        panel_results = NULL;
    }
    if (progress_benchmark) {
        crystalline_progress_destroy(progress_benchmark);
        progress_benchmark = NULL;
    }
    ui_initialized = false;
}// Legacy function for compatibility
void handle_benchmark_tab_click(AppState* state, int x, int y) {
    handle_benchmark_tab_mouse_down(state, x, y);
}
