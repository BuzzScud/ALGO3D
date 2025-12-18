// workspace.c - Workspace management for project isolation
#include "app_common.h"
#include "workspace.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * Initialize workspace system
 * Sets up default paths or custom workspace
 */
void workspace_init(AppState* state, const char* workspace_path) {
    if (!state) return;
    
    if (workspace_path && strlen(workspace_path) > 0) {
        // Use custom workspace
        strncpy(state->workspace_path, workspace_path, sizeof(state->workspace_path) - 1);
        state->workspace_path[sizeof(state->workspace_path) - 1] = '\0';
        state->workspace_active = true;
        
        printf("=== WORKSPACE MODE ===\n");
        printf("Using workspace: %s\n", state->workspace_path);
        
        // Create workspace directories
        workspace_create_directories(state);
    } else {
        // Use default paths (current behavior)
        strncpy(state->workspace_path, ".", sizeof(state->workspace_path) - 1);
        state->workspace_active = false;
        
        printf("=== DEFAULT MODE ===\n");
        printf("Using default directories\n");
    }
}

/**
 * Create workspace directory structure
 */
int workspace_create_directories(AppState* state) {
    if (!state) return -1;
    
    char path[1024];
    
    // Create main workspace directory
    if (mkdir(state->workspace_path, 0755) == -1) {
        // Directory might already exist, that's okay
    }
    
    // Create subdirectories
    snprintf(path, sizeof(path), "%s/models", state->workspace_path);
    mkdir(path, 0755);
    
    snprintf(path, sizeof(path), "%s/data", state->workspace_path);
    mkdir(path, 0755);
    
    snprintf(path, sizeof(path), "%s/data/training", state->workspace_path);
    mkdir(path, 0755);
    
    snprintf(path, sizeof(path), "%s/checkpoints", state->workspace_path);
    mkdir(path, 0755);
    
    snprintf(path, sizeof(path), "%s/docs", state->workspace_path);
    mkdir(path, 0755);
    
    snprintf(path, sizeof(path), "%s/docs/research", state->workspace_path);
    mkdir(path, 0755);
    
    printf("✓ Workspace directories created\n");
    printf("  - %s/models/\n", state->workspace_path);
    printf("  - %s/data/training/\n", state->workspace_path);
    printf("  - %s/checkpoints/\n", state->workspace_path);
    printf("  - %s/docs/research/\n", state->workspace_path);
    
    return 0;
}

/**
 * Get full path for a file in the workspace
 */
void workspace_get_path(AppState* state, const char* relative_path, char* output, size_t output_size) {
    if (!state || !relative_path || !output) return;
    
    if (state->workspace_active) {
        snprintf(output, output_size, "%s/%s", state->workspace_path, relative_path);
    } else {
        strncpy(output, relative_path, output_size - 1);
        output[output_size - 1] = '\0';
    }
}

/**
 * Get models directory path
 */
void workspace_get_models_dir(AppState* state, char* output, size_t output_size) {
    workspace_get_path(state, "models", output, output_size);
}

/**
 * Get training data directory path
 */
void workspace_get_training_dir(AppState* state, char* output, size_t output_size) {
    workspace_get_path(state, "data/training", output, output_size);
}

/**
 * Get checkpoints directory path
 */
void workspace_get_checkpoints_dir(AppState* state, char* output, size_t output_size) {
    workspace_get_path(state, "checkpoints", output, output_size);
}

/**
 * Get research directory path
 */
void workspace_get_research_dir(AppState* state, char* output, size_t output_size) {
    workspace_get_path(state, "docs/research", output, output_size);
}

/**
 * Check if workspace exists
 */
bool workspace_exists(const char* workspace_path) {
    struct stat st;
    return (stat(workspace_path, &st) == 0 && S_ISDIR(st.st_mode));
}

/**
 * List available workspaces in a directory
 */
int workspace_list(const char* base_dir, char workspaces[][512], int max_workspaces) {
    // TODO: Implement directory listing
    // For now, just return 0
    (void)base_dir;
    (void)workspaces;
    (void)max_workspaces;
    return 0;
}

/**
 * Get current workspace path
 */
char* workspace_get_current_path(AppState* state) {
    if (!state) return NULL;
    return state->workspace_path;
}

/**
 * Switch to a different workspace
 */
bool workspace_switch(AppState* state, const char* new_workspace_path) {
    if (!state || !new_workspace_path) return false;
    
    // Check if workspace exists
    if (!workspace_exists(new_workspace_path)) {
        printf("Workspace does not exist: %s\n", new_workspace_path);
        printf("Creating new workspace...\n");
        
        // Try to create it
        if (mkdir(new_workspace_path, 0755) == -1) {
            printf("Failed to create workspace directory\n");
            return false;
        }
    }
    
    // Update workspace path
    strncpy(state->workspace_path, new_workspace_path, sizeof(state->workspace_path) - 1);
    state->workspace_path[sizeof(state->workspace_path) - 1] = '\0';
    state->workspace_active = true;
    
    // Create subdirectories
    workspace_create_directories(state);
    
    printf("✓ Switched to workspace: %s\n", new_workspace_path);
    return true;
}