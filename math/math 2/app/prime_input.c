// prime_input.c - Prime number input and analysis
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "app_common.h"
#include "clock_abacus.h"
#include "prime_math.h"

// Helper to find prime index
int find_prime_index(int prime, AppState* state) {
    // Binary search (primes are sorted) - O(log n) instead of O(n)
    int left = 0;
    int right = state->prime_count - 1;
    
    while (left <= right) {
        int mid = left + (right - left) / 2;
        
        if (state->primes[mid] == prime) {
            return mid + 1;  // Babylonian (1-based)
        } else if (state->primes[mid] < prime) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    
    return -1;  // Not found
}


// Calculate distance from nearest ring boundary
double calculate_ring_distance(int number, AppState* state) {
    int prime_index = find_prime_index(number, state);
    if (prime_index < 0) return 0.0;
    
    int ring = get_prime_ring(prime_index);
    
    // Ring boundaries in prime indices
    int ring_boundaries[] = {0, 12, 72, 132, 232};
    int lower = ring_boundaries[ring];
    int upper = ring_boundaries[ring + 1];
    
    int dist_from_lower = prime_index - lower;
    int dist_from_upper = upper - prime_index;
    
    return (dist_from_lower < dist_from_upper) ? (double)dist_from_lower : (double)dist_from_upper;
}

// Analyze a number and populate the analysis structure
void analyze_number(AppState* state, int number) {
    NumberAnalysis* analysis = &state->number_analysis;
    
    analysis->number = number;
    
    // Find prime index
    int prime_index = find_prime_index(number, state);
    
    if (prime_index > 0) {
        int ring = get_prime_ring(prime_index);
        int position = get_position_on_ring(prime_index, ring);
        double angle = get_clock_angle(position, ring);
        
        analysis->ring = ring;
        analysis->angle = angle * 180.0 / MATH_PI;
        analysis->angle_degrees = analysis->angle;
        analysis->ring_distance = calculate_ring_distance(number, state);
        analysis->clock_position = position;
        
        // Calculate radial distance (distance from center)
        analysis->radial_distance = get_ring_radius(ring, 350.0);
        
        // Calculate quadrant (0=E, 1=N, 2=W, 3=S)
        double angle_deg = analysis->angle;
        if (angle_deg < 0) angle_deg += 360;
        analysis->quadrant = (int)(angle_deg / 90.0);
    } else {
        // Not a prime or not in our list
        analysis->ring = -1;
        analysis->angle = 0.0;
        analysis->angle_degrees = 0.0;
        analysis->ring_distance = 0.0;
        analysis->radial_distance = 0.0;
        analysis->clock_position = 0;
        analysis->quadrant = 0;
    }
    
    // Check if prime
    analysis->is_prime = is_prime((uint64_t)number);
    
    // Factor the number
    analysis->factor_count = 0;
    if (number > 1) {
        for (int i = 2; i <= number && analysis->factor_count < 100; i++) {
            if (number % i == 0) {
                analysis->factors[analysis->factor_count++] = i;
            }
        }
    }
}

// Add number to analysis history
void add_to_analysis_history(AppState* state, int number) {
    // Check if already in history
    for (int i = 0; i < state->analysis_history.count; i++) {
        if (state->analysis_history.numbers[i] == number) {
            state->analysis_history.selected_index = i;
            return;  // Already exists
        }
    }
    
    // Add new number
    if (state->analysis_history.count < 100) {
        int idx = state->analysis_history.count;
        state->analysis_history.numbers[idx] = number;
        state->analysis_history.colors[idx] = get_analysis_color(idx);
        state->analysis_history.selected_index = idx;
        state->analysis_history.count++;
    }
}

// Get color for analyzed number
SDL_Color get_analysis_color(int index) {
    SDL_Color colors[] = {
        {255, 255, 0, 255},    // Yellow
        {0, 255, 255, 255},    // Cyan
        {0, 255, 0, 255},      // Green
        {255, 0, 255, 255},    // Magenta
        {255, 128, 0, 255},    // Orange
        {255, 100, 100, 255},  // Light red
        {100, 100, 255, 255},  // Light blue
        {255, 255, 100, 255},  // Light yellow
        {100, 255, 100, 255},  // Light green
        {255, 100, 255, 255}   // Light magenta
    };
    return colors[index % 10];
}

// Process input and analyze number
void process_input(AppState* state) {
    if (strlen(state->input_buffer) == 0) return;
    
    int number = atoi(state->input_buffer);
    if (number <= 0) {
        printf("Invalid number: %s\n", state->input_buffer);
        state->input_buffer[0] = '\0';
        state->input_active = false;
        return;
    }
    
    printf("Analyzing number: %d\n", number);
    
    // Analyze the number
    analyze_number(state, number);
    
    // Add to history
    add_to_analysis_history(state, number);
    
    // Clear input and exit input mode
    state->input_buffer[0] = '\0';
    state->input_active = false;
    
    printf("Analysis complete. Prime: %s, Ring: %d, Position: %d, Angle: %.1f°\n",
           state->number_analysis.is_prime ? "YES" : "NO",
           state->number_analysis.ring,
           state->number_analysis.clock_position,
           state->number_analysis.angle_degrees);
}

// Handle text input
void handle_text_input(AppState* state, const char* text) {
    if (!state->input_active) return;
    
    // Only allow digits
    for (int i = 0; text[i] != '\0'; i++) {
        if (text[i] >= '0' && text[i] <= '9') {
            int len = strlen(state->input_buffer);
            if (len < 63) {  // Leave room for null terminator
                state->input_buffer[len] = text[i];
                state->input_buffer[len + 1] = '\0';
            }
        }
    }
}

// Handle backspace
void handle_backspace(AppState* state) {
    if (!state->input_active) return;
    
    int len = strlen(state->input_buffer);
    if (len > 0) {
        state->input_buffer[len - 1] = '\0';
    }
}

// Draw detailed overlay
void draw_detailed_overlay(SDL_Renderer* renderer, AppState* state) {
    // Draw analysis overlay on the visualization
    if (state->number_analysis.number <= 0) return;
    
    NumberAnalysis* analysis = &state->number_analysis;
    
    // Draw a semi-transparent panel at the top
    SDL_Rect panel = {10, 10, 400, 150};
    SDL_SetRenderDrawColor(renderer, 20, 20, 40, 200);
    SDL_RenderFillRect(renderer, &panel);
    SDL_SetRenderDrawColor(renderer, 100, 100, 150, 255);
    SDL_RenderDrawRect(renderer, &panel);
    
    // Colors for text
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color green = {0, 255, 0, 255};
    SDL_Color red = {255, 0, 0, 255};
    SDL_Color yellow = {255, 255, 0, 255};
    SDL_Color cyan = {0, 255, 255, 255};
    
    // Buffer for text
    char text_buffer[256];
    
    // Line 1: Number and prime status
    snprintf(text_buffer, sizeof(text_buffer), "Number: %d", analysis->number);
    draw_text(renderer, text_buffer, 20, 20, white);
    
    SDL_Color prime_color = analysis->is_prime ? green : red;
    const char* prime_text = analysis->is_prime ? "PRIME" : "COMPOSITE";
    draw_text(renderer, prime_text, 200, 20, prime_color);
    
    // Line 2: Ring and position
    snprintf(text_buffer, sizeof(text_buffer), "Ring: %d  Position: %d", 
             analysis->ring, analysis->clock_position);
    draw_text(renderer, text_buffer, 20, 45, cyan);
    
    // Line 3: Angle and quadrant
    snprintf(text_buffer, sizeof(text_buffer), "Angle: %.1f°  Quadrant: %d", 
             analysis->angle, analysis->quadrant);
    draw_text(renderer, text_buffer, 20, 70, cyan);
    
    // Line 4: Factor count
    snprintf(text_buffer, sizeof(text_buffer), "Factors: %d", analysis->factor_count);
    draw_text(renderer, text_buffer, 20, 95, yellow);
    
    // Line 5: Radius info
    snprintf(text_buffer, sizeof(text_buffer), "Radial: %.1f  Ring Dist: %.1f", 
             analysis->radial_distance, analysis->ring_distance);
    draw_text(renderer, text_buffer, 20, 120, white);
}

// Get or calculate factors for a number (with caching)
int* get_factors_cached(AppState* state, int number, int* out_count) {
    // Check if already in cache
    for (int i = 0; i < state->factor_cache_count; i++) {
        if (state->factor_cache[i].number == number && state->factor_cache[i].valid) {
            *out_count = state->factor_cache[i].count;
            return state->factor_cache[i].factors;
        }
    }
    
    // Not in cache - calculate and store
    if (state->factor_cache_count >= 100) {
        // Cache full - reuse oldest entry
        state->factor_cache_count = 0;
    }
    
    int cache_idx = state->factor_cache_count++;
    state->factor_cache[cache_idx].number = number;
    state->factor_cache[cache_idx].factors = (int*)malloc(100 * sizeof(int));
    state->factor_cache[cache_idx].count = 0;
    
    // Calculate factors - OPTIMIZED: Only check up to sqrt(n)
    if (number > 1) {
        // Check divisors up to sqrt(number)
        int sqrt_n = (int)prime_sqrt_int(number);
        for (int i = 2; i <= sqrt_n && state->factor_cache[cache_idx].count < 98; i++) {
            if (number % i == 0) {
                state->factor_cache[cache_idx].factors[state->factor_cache[cache_idx].count++] = i;
                // Add the corresponding divisor (number/i) if it's different
                int complement = number / i;
                if (complement != i && state->factor_cache[cache_idx].count < 98) {
                    state->factor_cache[cache_idx].factors[state->factor_cache[cache_idx].count++] = complement;
                }
            }
        }
        
        // Always include the number itself as a factor
        if (state->factor_cache[cache_idx].count < 100) {
            state->factor_cache[cache_idx].factors[state->factor_cache[cache_idx].count++] = number;
        }
        
        // Sort factors in ascending order for consistency
        for (int i = 0; i < state->factor_cache[cache_idx].count - 1; i++) {
            for (int j = i + 1; j < state->factor_cache[cache_idx].count; j++) {
                if (state->factor_cache[cache_idx].factors[i] > state->factor_cache[cache_idx].factors[j]) {
                    int temp = state->factor_cache[cache_idx].factors[i];
                    state->factor_cache[cache_idx].factors[i] = state->factor_cache[cache_idx].factors[j];
                    state->factor_cache[cache_idx].factors[j] = temp;
                }
            }
        }
    }
    
    state->factor_cache[cache_idx].valid = true;
    *out_count = state->factor_cache[cache_idx].count;
    return state->factor_cache[cache_idx].factors;
}

// Invalidate factor cache (call when analysis history changes)
void invalidate_factor_cache(AppState* state) {
    for (int i = 0; i < state->factor_cache_count; i++) {
        state->factor_cache[i].valid = false;
        if (state->factor_cache[i].factors) {
            free(state->factor_cache[i].factors);
            state->factor_cache[i].factors = NULL;
        }
    }
    state->factor_cache_count = 0;
}

// Draw factor connections
void draw_factor_connections(SDL_Renderer* renderer, AppState* state, int cx, int cy) {
    if (!state->show_factors) return;
    
    // Determine which numbers to show factors for
    int start_idx = 0;
    int end_idx = state->analysis_history.count;
    
    if (state->analysis_history.selected_index >= 0 && 
        state->analysis_history.selected_index < state->analysis_history.count) {
        // Show only selected number's factors
        start_idx = state->analysis_history.selected_index;
        end_idx = start_idx + 1;
    }
    
    // Draw factor lines for each analyzed number
    for (int h = start_idx; h < end_idx; h++) {
        int number = state->analysis_history.numbers[h];
        SDL_Color color = state->analysis_history.colors[h];
        
        // Get number position
        int num_prime_idx = find_prime_index(number, state);
        if (num_prime_idx < 0) continue;
        
        int num_ring = get_prime_ring(num_prime_idx);
        int num_position = get_position_on_ring(num_prime_idx, num_ring);
        double num_angle = get_clock_angle(num_position, num_ring);
        double num_radius = get_ring_radius(num_ring, 350.0 * state->zoom);
        
        int num_x = cx + (int)(num_radius * math_cos(num_angle)) + (int)state->offset_x;
        int num_y = cy + (int)(num_radius * math_sin(num_angle)) + (int)state->offset_y;
        
        // Get factors from cache (or calculate if not cached)
        int factor_count = 0;
        int* factors = get_factors_cached(state, number, &factor_count);
        
        // Draw lines to all factors
        for (int i = 0; i < factor_count; i++) {
            int factor = factors[i];
            if (factor == number) continue;  // Skip self
            
            int factor_prime_idx = find_prime_index(factor, state);
            if (factor_prime_idx < 0) continue;
            
            int factor_ring = get_prime_ring(factor_prime_idx);
            int factor_position = get_position_on_ring(factor_prime_idx, factor_ring);
            double factor_angle = get_clock_angle(factor_position, factor_ring);
            double factor_radius = get_ring_radius(factor_ring, 350.0 * state->zoom);
            
            int factor_x = cx + (int)(factor_radius * math_cos(factor_angle)) + (int)state->offset_x;
            int factor_y = cy + (int)(factor_radius * math_sin(factor_angle)) + (int)state->offset_y;
            
            // Draw thick line
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 150);
            for (int t = 0; t < 5; t++) {
                SDL_RenderDrawLine(renderer, num_x + t - 2, num_y, factor_x + t - 2, factor_y);
            }
        }
    }
}

// Draw analyzed numbers on visualization (for ALL modes)
void draw_analyzed_numbers(SDL_Renderer* renderer, AppState* state, int cx, int cy) {
    if (state->analysis_history.count == 0) return;
    
    for (int i = 0; i < state->analysis_history.count; i++) {
        int analyzed_num = state->analysis_history.numbers[i];
        SDL_Color color = state->analysis_history.colors[i];
        bool is_selected = (state->analysis_history.selected_index == i);
        
        // Find this number's position
        int prime_index = find_prime_index(analyzed_num, state);
        if (prime_index < 0) continue;
        
        int ring = get_prime_ring(prime_index);
        int position = get_position_on_ring(prime_index, ring);
        double angle = get_clock_angle(position, ring);
        double radius = get_ring_radius(ring, 350.0 * state->zoom);
        
        int x = cx + (int)(radius * math_cos(angle)) + (int)state->offset_x;
        int y = cy + (int)(radius * math_sin(angle)) + (int)state->offset_y;
        
        // Draw larger dot
        int size = is_selected ? 10 : 8;
        
        // White outline if selected
        if (is_selected) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            for (int dy = -size-2; dy <= size+2; dy++) {
                for (int dx = -size-2; dx <= size+2; dx++) {
                    int dist_sq = dx*dx + dy*dy;
                    if (dist_sq <= (size+2)*(size+2) && dist_sq > size*size) {
                        SDL_RenderDrawPoint(renderer, x + dx, y + dy);
                    }
                }
            }
        }
        
        // Colored dot
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
        for (int dy = -size; dy <= size; dy++) {
            for (int dx = -size; dx <= size; dx++) {
                if (dx*dx + dy*dy <= size*size) {
                    SDL_RenderDrawPoint(renderer, x + dx, y + dy);
                }
            }
        }
    }
}