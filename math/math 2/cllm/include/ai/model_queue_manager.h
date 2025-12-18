#ifndef MODEL_QUEUE_MANAGER_H
#define MODEL_QUEUE_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create queue directory for model
 * Creates models/<model_name>_queue/
 */
int create_model_queue(const char* model_name);

/**
 * Add file to model's queue
 * Copies tokenized file to model's queue directory
 */
int add_to_model_queue(const char* model_name, const char* tokenized_file);

/**
 * Get next file from model's queue
 * Returns path to next file, or NULL if queue is empty
 * Caller must free returned string
 */
char* get_next_from_queue(const char* model_name);

/**
 * Get queue size
 * Returns number of files in model's queue
 */
int get_queue_size(const char* model_name);

/**
 * Clear model queue
 * Deletes all files from model's queue
 */
int clear_model_queue(const char* model_name);

/**
 * Import queue from another model (cross-model training)
 * Copies all files from source_model's queue to target_model's queue
 * Returns number of files imported, or -1 on error
 */
int import_queue_from_model(const char* target_model, const char* source_model);

#ifdef __cplusplus
}
#endif

#endif /* MODEL_QUEUE_MANAGER_H */