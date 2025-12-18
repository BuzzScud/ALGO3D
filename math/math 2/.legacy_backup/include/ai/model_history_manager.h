#ifndef MODEL_HISTORY_MANAGER_H
#define MODEL_HISTORY_MANAGER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Training history entry
 */
typedef struct {
    int epoch;
    float loss;
    float learning_rate;
    int batch_size;
    int sequence_length;
    uint64_t timestamp;  // Unix timestamp
} TrainingHistoryEntry;

/**
 * Initialize history directory for a model
 * Creates models/<model_name>_history/ if it doesn't exist
 * 
 * Returns: 0 on success, -1 on error
 */
int model_history_init(const char* model_name);

/**
 * Save training history entry
 * Appends entry to models/<model_name>_history/training_log.csv
 * 
 * Returns: 0 on success, -1 on error
 */
int model_history_save_entry(const char* model_name, const TrainingHistoryEntry* entry);

/**
 * Load all training history entries
 * Reads from models/<model_name>_history/training_log.csv
 * 
 * Returns: Array of entries (caller must free), NULL on error
 * Sets *num_entries to number of entries loaded
 */
TrainingHistoryEntry* model_history_load_all(const char* model_name, int* num_entries);

/**
 * Get latest training history entry
 * 
 * Returns: 0 on success, -1 on error
 */
int model_history_get_latest(const char* model_name, TrainingHistoryEntry* entry);

/**
 * Get total number of epochs trained
 * 
 * Returns: Number of epochs, -1 on error
 */
int model_history_get_total_epochs(const char* model_name);

/**
 * Clear all training history for a model
 * Deletes models/<model_name>_history/training_log.csv
 * 
 * Returns: 0 on success, -1 on error
 */
int model_history_clear(const char* model_name);

/**
 * Export training history to JSON
 * Creates models/<model_name>_history/training_log.json
 * 
 * Returns: 0 on success, -1 on error
 */
int model_history_export_json(const char* model_name);

#ifdef __cplusplus
}
#endif

#endif /* MODEL_HISTORY_MANAGER_H */