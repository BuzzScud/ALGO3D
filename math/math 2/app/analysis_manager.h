// analysis_manager.h - Centralized analysis system interface

#ifndef ANALYSIS_MANAGER_H
#define ANALYSIS_MANAGER_H

#include "app_common.h"

// Initialize analysis manager
void analysis_manager_init(AppState* state);

// Cleanup analysis manager
void analysis_manager_cleanup(AppState* state);

// Add number to analysis history
void analysis_manager_add(AppState* state, int number);

// Select number by index
void analysis_manager_select(AppState* state, int index);

// Select number by value
void analysis_manager_select_number(AppState* state, int number);

// Toggle between all/selected factor display
void analysis_manager_toggle_mode(AppState* state);

// Navigate to previous analyzed number
void analysis_manager_prev(AppState* state);

// Navigate to next analyzed number
void analysis_manager_next(AppState* state);

// Clear all analyzed numbers
void analysis_manager_clear(AppState* state);

// Get factors for a number (with caching)
int* analysis_manager_get_factors(AppState* state, int number, int* out_count);

// Check if a number is analyzed
bool analysis_manager_contains(AppState* state, int number);

// Get count of analyzed numbers
int analysis_manager_count(AppState* state);

// Get selected index
int analysis_manager_get_selected(AppState* state);

#endif // ANALYSIS_MANAGER_H