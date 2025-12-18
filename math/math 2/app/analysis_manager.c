// analysis_manager.c - Centralized analysis system
// Manages all number analysis, factor calculation, and visualization

#include "app_common.h"
#include <stdlib.h>
#include <string.h>

// Initialize analysis manager
void analysis_manager_init(AppState* state) {
    state->analysis_history.count = 0;
    state->analysis_history.selected_index = -1;
    state->factor_cache_count = 0;
    
    for (int i = 0; i < 100; i++) {
        state->factor_cache[i].number = 0;
        state->factor_cache[i].factors = NULL;
        state->factor_cache[i].count = 0;
        state->factor_cache[i].valid = false;
    }
}

// Cleanup analysis manager
void analysis_manager_cleanup(AppState* state) {
    // Free all cached factors
    for (int i = 0; i < state->factor_cache_count; i++) {
        if (state->factor_cache[i].factors) {
            free(state->factor_cache[i].factors);
            state->factor_cache[i].factors = NULL;
        }
    }
    state->factor_cache_count = 0;
    state->analysis_history.count = 0;
}

// Add number to analysis history
void analysis_manager_add(AppState* state, int number) {
    // Check if already exists
    for (int i = 0; i < state->analysis_history.count; i++) {
        if (state->analysis_history.numbers[i] == number) {
            state->analysis_history.selected_index = i;
            analyze_number(state, number);
            return;
        }
    }
    
    // Add new number
    if (state->analysis_history.count < 100) {
        int idx = state->analysis_history.count;
        state->analysis_history.numbers[idx] = number;
        state->analysis_history.colors[idx] = get_analysis_color(idx);
        state->analysis_history.selected_index = idx;
        state->analysis_history.count++;
        
        // Analyze the number
        analyze_number(state, number);
    }
}

// Select number by index
void analysis_manager_select(AppState* state, int index) {
    if (index >= 0 && index < state->analysis_history.count) {
        state->analysis_history.selected_index = index;
        int number = state->analysis_history.numbers[index];
        analyze_number(state, number);
    }
}

// Select number by value
void analysis_manager_select_number(AppState* state, int number) {
    for (int i = 0; i < state->analysis_history.count; i++) {
        if (state->analysis_history.numbers[i] == number) {
            analysis_manager_select(state, i);
            return;
        }
    }
}

// Toggle between all/selected factor display
void analysis_manager_toggle_mode(AppState* state) {
    if (state->analysis_history.selected_index == -1) {
        // Currently showing all - switch to selected
        if (state->analysis_history.count > 0) {
            state->analysis_history.selected_index = state->analysis_history.count - 1;
        }
    } else {
        // Currently showing selected - switch to all
        state->analysis_history.selected_index = -1;
    }
}

// Navigate to previous analyzed number
void analysis_manager_prev(AppState* state) {
    if (state->analysis_history.count == 0) return;
    
    if (state->analysis_history.selected_index <= 0) {
        state->analysis_history.selected_index = state->analysis_history.count - 1;
    } else {
        state->analysis_history.selected_index--;
    }
    
    int number = state->analysis_history.numbers[state->analysis_history.selected_index];
    analyze_number(state, number);
}

// Navigate to next analyzed number
void analysis_manager_next(AppState* state) {
    if (state->analysis_history.count == 0) return;
    
    if (state->analysis_history.selected_index < 0 || 
        state->analysis_history.selected_index >= state->analysis_history.count - 1) {
        state->analysis_history.selected_index = 0;
    } else {
        state->analysis_history.selected_index++;
    }
    
    int number = state->analysis_history.numbers[state->analysis_history.selected_index];
    analyze_number(state, number);
}

// Clear all analyzed numbers
void analysis_manager_clear(AppState* state) {
    state->analysis_history.count = 0;
    state->analysis_history.selected_index = -1;
    invalidate_factor_cache(state);
}

// Get factors for a number (with caching)
int* analysis_manager_get_factors(AppState* state, int number, int* out_count) {
    return get_factors_cached(state, number, out_count);
}

// Check if a number is analyzed
bool analysis_manager_contains(AppState* state, int number) {
    for (int i = 0; i < state->analysis_history.count; i++) {
        if (state->analysis_history.numbers[i] == number) {
            return true;
        }
    }
    return false;
}

// Get count of analyzed numbers
int analysis_manager_count(AppState* state) {
    return state->analysis_history.count;
}

// Get selected index
int analysis_manager_get_selected(AppState* state) {
    return state->analysis_history.selected_index;
}