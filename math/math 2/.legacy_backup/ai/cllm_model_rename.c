/**
 * Model Renaming System
 * 
 * Handles renaming models including files and directories
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "cllm.h"
#include "cllm_format.h"
#include "cllm_utils.h"
#include "ai/cllm_model_metadata.h"

/**
 * Rename queue directory
 */
static int rename_queue_directory(const char* old_name, const char* new_name) {
    char old_dir[512];
    char new_dir[512];
    
    snprintf(old_dir, sizeof(old_dir), "models/%s_queue", old_name);
    snprintf(new_dir, sizeof(new_dir), "models/%s_queue", new_name);
    
    // Check if old directory exists
    struct stat st;
    if (stat(old_dir, &st) == 0) {
        if (rename(old_dir, new_dir) != 0) {
            fprintf(stderr, "Error: Failed to rename queue directory from %s to %s\n", old_dir, new_dir);
            return -1;
        }
    }
    
    return 0;
}

/**
 * Rename model (complete operation)
 */
int cllm_model_rename(const char* old_name, const char* new_name) {
    if (!old_name || !new_name) return -1;
    
    // Validate new name
    if (cllm_model_validate_name_unique(new_name) != 0) {
        fprintf(stderr, "Error: Model name '%s' already exists\n", new_name);
        return -1;
    }
    
    // Load model
    char old_path[512];
    snprintf(old_path, sizeof(old_path), "models/%s.cllm", old_name);
    
    CLLMModel* model = cllm_read_model(old_path);
    if (!model) {
        fprintf(stderr, "Error: Failed to load model '%s'\n", old_name);
        return -1;
    }
    
    // Update model name and queue directory
    if (cllm_model_set_name(model, new_name) != 0) {
        fprintf(stderr, "Error: Failed to set new model name\n");
        cllm_free_model(model);
        return -1;
    }
    
    // Save model with new name
    char new_path[512];
    snprintf(new_path, sizeof(new_path), "models/%s.cllm", new_name);
    
    if (cllm_write_model(model, new_path) != 0) {
        fprintf(stderr, "Error: Failed to save model with new name\n");
        cllm_free_model(model);
        return -1;
    }
    
    cllm_free_model(model);
    
    // Rename queue directory
    if (rename_queue_directory(old_name, new_name) != 0) {
        fprintf(stderr, "Warning: Failed to rename queue directory\n");
        // Continue anyway - model file is renamed
    }
    
    // Delete old model file
    if (unlink(old_path) != 0) {
        fprintf(stderr, "Warning: Failed to delete old model file: %s\n", old_path);
        // Continue anyway - new model exists
    }
    
    printf("Successfully renamed model '%s' to '%s'\n", old_name, new_name);
    return 0;
}

/**
 * Get model name from file path
 */
char* cllm_model_get_name_from_path(const char* filepath) {
    if (!filepath) return NULL;
    
    // Find last '/' or '\'
    const char* filename = strrchr(filepath, '/');
    if (!filename) {
        filename = strrchr(filepath, '\\');
    }
    if (filename) {
        filename++; // Skip the separator
    } else {
        filename = filepath;
    }
    
    // Remove .cllm extension
    char* name = strdup(filename);
    if (!name) return NULL;
    
    char* ext = strstr(name, ".cllm");
    if (ext) {
        *ext = '\0';
    }
    
    return name;
}