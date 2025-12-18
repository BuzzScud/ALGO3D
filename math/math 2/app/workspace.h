// workspace.h - Workspace management header
#ifndef WORKSPACE_H
#define WORKSPACE_H

#include "app_common.h"
#include <stdbool.h>

/**
 * Initialize workspace system
 * @param state Application state
 * @param workspace_path Path to workspace directory (NULL for default)
 */
void workspace_init(AppState* state, const char* workspace_path);

/**
 * Create workspace directory structure
 * @param state Application state
 * @return 0 on success, -1 on failure
 */
int workspace_create_directories(AppState* state);

/**
 * Get full path for a file in the workspace
 * @param state Application state
 * @param relative_path Relative path within workspace
 * @param output Output buffer for full path
 * @param output_size Size of output buffer
 */
void workspace_get_path(AppState* state, const char* relative_path, char* output, size_t output_size);

/**
 * Get models directory path
 */
void workspace_get_models_dir(AppState* state, char* output, size_t output_size);

/**
 * Get training data directory path
 */
void workspace_get_training_dir(AppState* state, char* output, size_t output_size);

/**
 * Get checkpoints directory path
 */
void workspace_get_checkpoints_dir(AppState* state, char* output, size_t output_size);

/**
 * Get research directory path
 */
void workspace_get_research_dir(AppState* state, char* output, size_t output_size);

/**
 * Check if workspace exists
 */
bool workspace_exists(const char* workspace_path);

/**
 * List available workspaces
 */
int workspace_list(const char* base_dir, char workspaces[][512], int max_workspaces);

/**
 * Get current workspace path
 */
char* workspace_get_current_path(AppState* state);

/**
 * Switch to a different workspace
 */
bool workspace_switch(AppState* state, const char* new_workspace_path);

#endif // WORKSPACE_H