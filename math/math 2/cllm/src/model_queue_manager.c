/**
 * Model Queue Manager
 * 
 * Manages per-model training queues
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include "cllm.h"

/**
 * Create queue directory for model
 */
int create_model_queue(const char* model_name) {
    if (!model_name) return -1;
    
    char queue_dir[512];
    snprintf(queue_dir, sizeof(queue_dir), "models/%s_queue", model_name);
    
    // Create models directory if it doesn't exist
    mkdir("models", 0755);
    
    // Create queue directory
    if (mkdir(queue_dir, 0755) != 0 && errno != EEXIST) {
        fprintf(stderr, "Error: Failed to create queue directory: %s\n", queue_dir);
        return -1;
    }
    
    return 0;
}

/**
 * Add file to model's queue
 */
int add_to_model_queue(const char* model_name, const char* tokenized_file) {
    if (!model_name || !tokenized_file) return -1;
    
    // Ensure queue exists
    if (create_model_queue(model_name) != 0) {
        return -1;
    }
    
    // Get filename from path
    const char* filename = strrchr(tokenized_file, '/');
    if (filename) {
        filename++; // Skip the '/'
    } else {
        filename = tokenized_file;
    }
    
    // Create destination path
    char dest_path[1024];
    snprintf(dest_path, sizeof(dest_path), "models/%s_queue/%s", model_name, filename);
    
    // Copy file to queue
    FILE* src = fopen(tokenized_file, "r");
    if (!src) {
        fprintf(stderr, "Error: Cannot open source file: %s\n", tokenized_file);
        return -1;
    }
    
    FILE* dst = fopen(dest_path, "w");
    if (!dst) {
        fprintf(stderr, "Error: Cannot create destination file: %s\n", dest_path);
        fclose(src);
        return -1;
    }
    
    // Copy contents
    char buffer[4096];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        if (fwrite(buffer, 1, bytes, dst) != bytes) {
            fprintf(stderr, "Error: Failed to write to destination\n");
            fclose(src);
            fclose(dst);
            return -1;
        }
    }
    
    fclose(src);
    fclose(dst);
    
    return 0;
}

/**
 * Get next file from model's queue
 */
char* get_next_from_queue(const char* model_name) {
    if (!model_name) return NULL;
    
    char queue_dir[512];
    snprintf(queue_dir, sizeof(queue_dir), "models/%s_queue", model_name);
    
    DIR* dir = opendir(queue_dir);
    if (!dir) {
        return NULL;
    }
    
    struct dirent* entry;
    char* result = NULL;
    
    while ((entry = readdir(dir)) != NULL) {
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        // Skip lock files
        if (strstr(entry->d_name, ".lock")) {
            continue;
        }
        
        // Found a file
        result = malloc(1024);
        if (result) {
            snprintf(result, 1024, "%s/%s", queue_dir, entry->d_name);
        }
        break;
    }
    
    closedir(dir);
    return result;
}

/**
 * Get queue size
 */
int get_queue_size(const char* model_name) {
    if (!model_name) return -1;
    
    char queue_dir[512];
    snprintf(queue_dir, sizeof(queue_dir), "models/%s_queue", model_name);
    
    DIR* dir = opendir(queue_dir);
    if (!dir) {
        return 0;
    }
    
    int count = 0;
    struct dirent* entry;
    
    while ((entry = readdir(dir)) != NULL) {
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        // Skip lock files
        if (strstr(entry->d_name, ".lock")) {
            continue;
        }
        
        count++;
    }
    
    closedir(dir);
    return count;
}

/**
 * Clear model queue
 */
int clear_model_queue(const char* model_name) {
    if (!model_name) return -1;
    
    char queue_dir[512];
    snprintf(queue_dir, sizeof(queue_dir), "models/%s_queue", model_name);
    
    DIR* dir = opendir(queue_dir);
    if (!dir) {
        return 0; // Queue doesn't exist, nothing to clear
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        // Delete file
        char filepath[1024];
        snprintf(filepath, sizeof(filepath), "%s/%s", queue_dir, entry->d_name);
        unlink(filepath);
    }
    
    closedir(dir);
    return 0;
}

/**
 * Import queue from another model (cross-model training)
 */
int import_queue_from_model(const char* target_model, const char* source_model) {
    if (!target_model || !source_model) return -1;
    
    char source_dir[512];
    snprintf(source_dir, sizeof(source_dir), "models/%s_queue", source_model);
    
    DIR* dir = opendir(source_dir);
    if (!dir) {
        fprintf(stderr, "Error: Source queue does not exist: %s\n", source_dir);
        return -1;
    }
    
    // Ensure target queue exists
    if (create_model_queue(target_model) != 0) {
        closedir(dir);
        return -1;
    }
    
    int count = 0;
    struct dirent* entry;
    
    while ((entry = readdir(dir)) != NULL) {
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        // Skip lock files
        if (strstr(entry->d_name, ".lock")) {
            continue;
        }
        
        // Copy file to target queue
        char source_path[1024];
        snprintf(source_path, sizeof(source_path), "%s/%s", source_dir, entry->d_name);
        
        if (add_to_model_queue(target_model, source_path) == 0) {
            count++;
        }
    }
    
    closedir(dir);
    
    printf("Imported %d files from %s to %s\n", count, source_model, target_model);
    return count;
}
