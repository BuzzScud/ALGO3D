#include "ai/model_history_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <errno.h>

#define MAX_PATH 1024
#define MAX_LINE 512

/**
 * Get history directory path for a model
 */
static void get_history_dir(const char* model_name, char* path, size_t path_size) {
    snprintf(path, path_size, "models/%s_history", model_name);
}

/**
 * Get history file path for a model
 */
static void get_history_file(const char* model_name, char* path, size_t path_size) {
    snprintf(path, path_size, "models/%s_history/training_log.csv", model_name);
}

/**
 * Initialize history directory for a model
 */
int model_history_init(const char* model_name) {
    if (!model_name) return -1;
    
    char dir_path[MAX_PATH];
    get_history_dir(model_name, dir_path, sizeof(dir_path));
    
    // Create directory if it doesn't exist
    struct stat st = {0};
    if (stat(dir_path, &st) == -1) {
        if (mkdir(dir_path, 0755) != 0) {
            fprintf(stderr, "Failed to create history directory: %s\n", dir_path);
            return -1;
        }
    }
    
    // Create CSV file with header if it doesn't exist
    char file_path[MAX_PATH];
    get_history_file(model_name, file_path, sizeof(file_path));
    
    if (stat(file_path, &st) == -1) {
        FILE* f = fopen(file_path, "w");
        if (!f) {
            fprintf(stderr, "Failed to create history file: %s\n", file_path);
            return -1;
        }
        fprintf(f, "epoch,loss,learning_rate,batch_size,sequence_length,timestamp\n");
        fclose(f);
    }
    
    return 0;
}

/**
 * Save training history entry
 */
int model_history_save_entry(const char* model_name, const TrainingHistoryEntry* entry) {
    if (!model_name || !entry) return -1;
    
    // Ensure directory exists
    if (model_history_init(model_name) != 0) {
        return -1;
    }
    
    char file_path[MAX_PATH];
    get_history_file(model_name, file_path, sizeof(file_path));
    
    FILE* f = fopen(file_path, "a");
    if (!f) {
        fprintf(stderr, "Failed to open history file: %s\n", file_path);
        return -1;
    }
    
    fprintf(f, "%d,%.6f,%.6f,%d,%d,%lu\n",
            entry->epoch,
            entry->loss,
            entry->learning_rate,
            entry->batch_size,
            entry->sequence_length,
            entry->timestamp);
    
    fclose(f);
    return 0;
}

/**
 * Load all training history entries
 */
TrainingHistoryEntry* model_history_load_all(const char* model_name, int* num_entries) {
    if (!model_name || !num_entries) return NULL;
    
    *num_entries = 0;
    
    char file_path[MAX_PATH];
    get_history_file(model_name, file_path, sizeof(file_path));
    
    FILE* f = fopen(file_path, "r");
    if (!f) {
        // File doesn't exist yet - not an error
        return NULL;
    }
    
    // Count lines (excluding header)
    char line[MAX_LINE];
    int count = 0;
    if (fgets(line, sizeof(line), f) == NULL) {
        fclose(f);
        return NULL;
    }
    while (fgets(line, sizeof(line), f)) {
        count++;
    }
    
    if (count == 0) {
        fclose(f);
        return NULL;
    }
    
    // Allocate array
    TrainingHistoryEntry* entries = malloc(count * sizeof(TrainingHistoryEntry));
    if (!entries) {
        fclose(f);
        return NULL;
    }
    
    // Read entries
    rewind(f);
    if (fgets(line, sizeof(line), f) == NULL) {
        free(entries);
        fclose(f);
        return NULL;
    }
    
    int i = 0;
    while (fgets(line, sizeof(line), f) && i < count) {
        if (sscanf(line, "%d,%f,%f,%d,%d,%lu",
                   &entries[i].epoch,
                   &entries[i].loss,
                   &entries[i].learning_rate,
                   &entries[i].batch_size,
                   &entries[i].sequence_length,
                   &entries[i].timestamp) == 6) {
            i++;
        }
    }
    
    fclose(f);
    *num_entries = i;
    return entries;
}

/**
 * Get latest training history entry
 */
int model_history_get_latest(const char* model_name, TrainingHistoryEntry* entry) {
    if (!model_name || !entry) return -1;
    
    int num_entries = 0;
    TrainingHistoryEntry* entries = model_history_load_all(model_name, &num_entries);
    
    if (!entries || num_entries == 0) {
        return -1;
    }
    
    // Copy last entry
    *entry = entries[num_entries - 1];
    free(entries);
    
    return 0;
}

/**
 * Get total number of epochs trained
 */
int model_history_get_total_epochs(const char* model_name) {
    if (!model_name) return -1;
    
    int num_entries = 0;
    TrainingHistoryEntry* entries = model_history_load_all(model_name, &num_entries);
    
    if (!entries || num_entries == 0) {
        return 0;
    }
    
    // Find maximum epoch number
    int max_epoch = 0;
    for (int i = 0; i < num_entries; i++) {
        if (entries[i].epoch > max_epoch) {
            max_epoch = entries[i].epoch;
        }
    }
    
    free(entries);
    return max_epoch;
}

/**
 * Clear all training history for a model
 */
int model_history_clear(const char* model_name) {
    if (!model_name) return -1;
    
    char file_path[MAX_PATH];
    get_history_file(model_name, file_path, sizeof(file_path));
    
    if (remove(file_path) != 0 && errno != ENOENT) {
        fprintf(stderr, "Failed to remove history file: %s\n", file_path);
        return -1;
    }
    
    // Recreate with header
    return model_history_init(model_name);
}

/**
 * Export training history to JSON
 */
int model_history_export_json(const char* model_name) {
    if (!model_name) return -1;
    
    int num_entries = 0;
    TrainingHistoryEntry* entries = model_history_load_all(model_name, &num_entries);
    
    if (!entries || num_entries == 0) {
        return -1;
    }
    
    char json_path[MAX_PATH];
    snprintf(json_path, sizeof(json_path), "models/%s_history/training_log.json", model_name);
    
    FILE* f = fopen(json_path, "w");
    if (!f) {
        free(entries);
        return -1;
    }
    
    fprintf(f, "{\n");
    fprintf(f, "  &quot;model_name&quot;: &quot;%s&quot;,\n", model_name);
    fprintf(f, "  &quot;total_epochs&quot;: %d,\n", entries[num_entries - 1].epoch);
    fprintf(f, "  &quot;entries&quot;: [\n");
    
    for (int i = 0; i < num_entries; i++) {
        fprintf(f, "    {\n");
        fprintf(f, "      &quot;epoch&quot;: %d,\n", entries[i].epoch);
        fprintf(f, "      &quot;loss&quot;: %.6f,\n", entries[i].loss);
        fprintf(f, "      &quot;learning_rate&quot;: %.6f,\n", entries[i].learning_rate);
        fprintf(f, "      &quot;batch_size&quot;: %d,\n", entries[i].batch_size);
        fprintf(f, "      &quot;sequence_length&quot;: %d,\n", entries[i].sequence_length);
        fprintf(f, "      &quot;timestamp&quot;: %lu\n", entries[i].timestamp);
        fprintf(f, "    }%s\n", (i < num_entries - 1) ? "," : "");
    }
    
    fprintf(f, "  ]\n");
    fprintf(f, "}\n");
    
    fclose(f);
    free(entries);
    
    return 0;
}