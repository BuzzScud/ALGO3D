/**
 * cllm_model_registry.h - Model Registry Interface
 * 
 * Lightweight registry for tracking model metadata.
 * Does NOT load models or coordinate access - just tracks files and metadata.
 * 
 * Key Principles:
 * - Metadata only (no model loading)
 * - No coordination (no locks between tabs)
 * - Simple operations (scan, rename, delete)
 * - Fast and lightweight
 */

#ifndef CLLM_MODEL_REGISTRY_H
#define CLLM_MODEL_REGISTRY_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// CONSTANTS
// ============================================================================

#define MODEL_NAME_MAX 256
#define MODEL_PATH_MAX 512
#define TRAINING_DATA_PATH_MAX 512

// ============================================================================
// MODEL METADATA
// ============================================================================

/**
 * Model Metadata - Information about a model file
 * 
 * This structure contains ONLY metadata read from the model file.
 * It does NOT contain the actual model data.
 */
typedef struct {
    // Identification
    char name[MODEL_NAME_MAX];              // Model name (without .cllm extension)
    char path[MODEL_PATH_MAX];              // Full path to .cllm file
    
    // Model properties (read from file header)
    uint32_t vocab_size;                    // Vocabulary size
    uint32_t embedding_dim;                 // Embedding dimension
    uint32_t num_layers;                    // Number of layers
    uint32_t num_heads;                     // Number of attention heads
    uint32_t max_seq_len;                   // Maximum sequence length
    
    // File information
    uint64_t file_size;                     // File size in bytes
    time_t created_time;                    // File creation time
    time_t modified_time;                   // File modification time
    
    // Training data association
    char training_data_dir[TRAINING_DATA_PATH_MAX];  // Associated training data directory
    
    // Validation
    bool is_valid;                          // True if file exists and is readable
    
} ModelMetadata;

// ============================================================================
// MODEL REGISTRY
// ============================================================================

/**
 * Model Registry - Tracks all available models
 * 
 * The registry scans the models directory and maintains metadata for all .cllm files.
 * It does NOT load models into memory - it only tracks file information.
 */
typedef struct {
    ModelMetadata** models;                 // Array of model metadata
    uint32_t num_models;                    // Number of models
    uint32_t capacity;                      // Array capacity
    char models_dir[MODEL_PATH_MAX];        // Models directory path
    pthread_mutex_t lock;                   // Lock for registry operations only
} ModelRegistry;

// ============================================================================
// REGISTRY LIFECYCLE
// ============================================================================

/**
 * Initialize the model registry
 * 
 * @param models_dir Directory containing .cllm files (NULL = "./models")
 * @return true on success, false on failure
 */
bool model_registry_init(const char* models_dir);

/**
 * Cleanup the model registry
 * 
 * Frees all metadata and resources.
 * Does NOT delete model files.
 */
void model_registry_cleanup(void);

/**
 * Check if registry is initialized
 * 
 * @return true if initialized, false otherwise
 */
bool model_registry_is_initialized(void);

// ============================================================================
// REGISTRY OPERATIONS
// ============================================================================

/**
 * Scan models directory for .cllm files
 * 
 * Reads all .cllm files in the models directory and extracts metadata.
 * Existing metadata is cleared and rebuilt from scratch.
 * 
 * @return Number of models found, or -1 on error
 */
int model_registry_scan(void);

/**
 * Refresh a single model's metadata
 * 
 * Re-reads the model file and updates metadata.
 * Useful after a model has been modified.
 * 
 * @param name Model name
 * @return true on success, false if model not found or error
 */
bool model_registry_refresh(const char* name);

/**
 * Get model metadata by name
 * 
 * @param name Model name (without .cllm extension)
 * @return Pointer to metadata, or NULL if not found
 * 
 * NOTE: Returned pointer is valid until next registry operation.
 *       Do not free or modify.
 */
const ModelMetadata* model_registry_get(const char* name);

/**
 * Get model metadata by index
 * 
 * @param index Model index (0 to num_models-1)
 * @return Pointer to metadata, or NULL if index out of range
 * 
 * NOTE: Returned pointer is valid until next registry operation.
 *       Do not free or modify.
 */
const ModelMetadata* model_registry_get_at_index(uint32_t index);

/**
 * Get number of models in registry
 * 
 * @return Number of models
 */
uint32_t model_registry_count(void);

/**
 * Get models directory path
 * 
 * @return Path to models directory
 */
const char* model_registry_get_models_dir(void);

// ============================================================================
// MODEL MANAGEMENT
// ============================================================================

/**
 * Register a new model
 * 
 * Adds a model to the registry without scanning the entire directory.
 * Useful after creating a new model.
 * 
 * @param name Model name
 * @param path Full path to .cllm file
 * @return true on success, false on failure
 */
bool model_registry_register(const char* name, const char* path);

/**
 * Unregister a model
 * 
 * Removes a model from the registry.
 * Does NOT delete the file - use model_registry_delete() for that.
 * 
 * @param name Model name
 * @return true on success, false if not found
 */
bool model_registry_unregister(const char* name);

/**
 * Rename a model
 * 
 * Renames the model file and updates registry.
 * 
 * @param old_name Current model name
 * @param new_name New model name
 * @return true on success, false on failure
 */
bool model_registry_rename(const char* old_name, const char* new_name);

/**
 * Delete a model
 * 
 * Deletes the model file from disk and removes from registry.
 * 
 * @param name Model name
 * @return true on success, false on failure
 */
bool model_registry_delete(const char* name);

/**
 * Check if a model exists
 * 
 * @param name Model name
 * @return true if model exists in registry, false otherwise
 */
bool model_registry_exists(const char* name);

// ============================================================================
// TRAINING DATA ASSOCIATION
// ============================================================================

/**
 * Associate training data directory with a model
 * 
 * @param name Model name
 * @param data_dir Path to training data directory
 * @return true on success, false on failure
 */
bool model_registry_set_training_data(const char* name, const char* data_dir);

/**
 * Get training data directory for a model
 * 
 * @param name Model name
 * @return Path to training data directory, or NULL if not set or model not found
 */
const char* model_registry_get_training_data(const char* name);

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * Get full path for a model name
 * 
 * Constructs the full path: models_dir/name.cllm
 * 
 * @param name Model name
 * @param path_out Buffer to store path (must be at least MODEL_PATH_MAX)
 * @return true on success, false on failure
 */
bool model_registry_get_path(const char* name, char* path_out);

/**
 * Extract model name from path
 * 
 * Extracts the model name from a full path by removing directory and .cllm extension.
 * 
 * @param path Full path to .cllm file
 * @param name_out Buffer to store name (must be at least MODEL_NAME_MAX)
 * @return true on success, false on failure
 */
bool model_registry_extract_name(const char* path, char* name_out);

/**
 * Print registry contents (for debugging)
 */
void model_registry_print(void);

#ifdef __cplusplus
}
#endif

#endif // CLLM_MODEL_REGISTRY_H