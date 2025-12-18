/**
 * cllm_model_manager.h - Global Model Manager
 * 
 * This provides a global model management system for the entire CLLM ecosystem.
 * Models are created once and shared across all components (UI tabs, CLI tools).
 * 
 * Features:
 * - Single source of truth for all models
 * - Concurrent access with read/write locks
 * - Model persistence and metadata
 * - Model registry for discovery
 * 
 * Usage:
 * - Training: Acquire write lock
 * - Inference: Acquire read lock (multiple concurrent readers allowed)
 * - Always release locks when done
 */

#ifndef CLLM_MODEL_MANAGER_H
#define CLLM_MODEL_MANAGER_H

#define _POSIX_C_SOURCE 200809L
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include "cllm.h"

#ifdef __cplusplus
extern "C" {
#endif

// Maximum lengths
#define MODEL_NAME_MAX 256
#define MODEL_PATH_MAX 1024

/**
 * Managed Model Structure
 * 
 * Represents a model managed by the global model manager.
 * Includes the model itself, metadata, and access control.
 */
typedef struct {
    char name[MODEL_NAME_MAX];          // Model name (unique identifier)
    char path[MODEL_PATH_MAX];          // Path to model file
    CLLMModel* model;                   // The actual model (may be NULL if not in memory)
    bool is_accessible;                 // Is model accessible (abacus has enough primes)?
    bool is_training;                   // Is model currently being trained?
    uint32_t read_count;                // Number of active readers
    pthread_rwlock_t lock;              // Read-write lock for concurrent access
    
    // Metadata (cached from file header)
    uint32_t vocab_size;
    uint32_t embedding_dim;
    uint32_t num_layers;
    uint32_t num_heads;
    uint64_t required_primes;           // Number of primes this model needs
    uint64_t created_time;              // Unix timestamp
    uint64_t modified_time;             // Unix timestamp
} ManagedModel;

/**
 * Model Manager - Global Singleton
 * 
 * Manages all models in the system.
 * Thread-safe with internal locking.
 */
typedef struct {
    ManagedModel** models;              // Array of managed models
    uint32_t num_models;                // Number of models
    uint32_t capacity;                  // Capacity of models array
    pthread_mutex_t manager_lock;       // Lock for manager operations
    char models_dir[MODEL_PATH_MAX];    // Directory for model storage
} ModelManager;

// ============================================================================
// INITIALIZATION AND CLEANUP
// ============================================================================

/**
 * Initialize the global model manager
 * 
 * Must be called before any other model manager functions.
 * Creates the models directory if it doesn't exist.
 * 
 * @param models_dir Directory to store models (NULL for default: ./models)
 * @return true on success, false on failure
 */
bool model_manager_init(const char* models_dir);

/**
 * Cleanup the global model manager
 * 
 * Releases all models and frees resources.
 * Should be called at program exit.
 */
void model_manager_cleanup(void);

// ============================================================================
// MODEL CREATION AND LOADING
// ============================================================================

/**
 * Create a new model
 * 
 * Creates a new model with the given configuration and adds it to the manager.
 * The model is immediately available for use.
 * 
 * @param name Unique name for the model
 * @param config Model configuration
 * @return Pointer to managed model on success, NULL on failure
 */
ManagedModel* model_manager_create(const char* name, const CLLMConfig* config);

/**
 * Load an existing model from disk
 * 
 * Loads a model from the specified path and adds it to the manager.
 * 
 * @param name Unique name for the model
 * @param path Path to model file
 * @return Pointer to managed model on success, NULL on failure
 */
ManagedModel* model_manager_load(const char* name, const char* path);

/**
 * Save a model to disk
 * 
 * Saves the model to its configured path.
 * 
 * @param name Name of the model to save
 * @return true on success, false on failure
 */
bool model_manager_save(const char* name);

/**
 * Unload a model from memory
 * 
 * Removes the model from memory but keeps it in the registry.
 * Can be reloaded later with model_manager_reload().
 * 
 * @param name Name of the model to unload
 * @return true on success, false on failure
 */
bool model_manager_unload(const char* name);

/**
 * Reload a model into memory
 * 
 * Loads a previously unloaded model back into memory.
 * 
 * @param name Name of the model to reload
 * @return true on success, false on failure
 */
bool model_manager_reload(const char* name);

/**
 * Delete a model
 * 
 * Removes the model from the manager and optionally deletes the file.
 * 
 * @param name Name of the model to delete
 * @param delete_file If true, also delete the model file from disk
 * @return true on success, false on failure
 */
bool model_manager_delete(const char* name, bool delete_file);

// ============================================================================
// MODEL ACCESS CONTROL
// ============================================================================

/**
 * Acquire read access to a model
 * 
 * Acquires a read lock on the model for inference.
 * Multiple readers can access the model concurrently.
 * Must call model_manager_release_read() when done.
 * 
 * @param name Name of the model
 * @return Pointer to the model on success, NULL on failure
 */
CLLMModel* model_manager_acquire_read(const char* name);

/**
 * Acquire write access to a model
 * 
 * Acquires a write lock on the model for training.
 * Only one writer can access the model at a time.
 * Blocks if there are active readers or writers.
 * Must call model_manager_release_write() when done.
 * 
 * @param name Name of the model
 * @return Pointer to the model on success, NULL on failure
 */
CLLMModel* model_manager_acquire_write(const char* name);

/**
 * Release read access to a model
 * 
 * Releases the read lock acquired with model_manager_acquire_read().
 * 
 * @param name Name of the model
 */
void model_manager_release_read(const char* name);

/**
 * Release write access to a model
 * 
 * Releases the write lock acquired with model_manager_acquire_write().
 * 
 * @param name Name of the model
 */
void model_manager_release_write(const char* name);

// ============================================================================
// MODEL QUERY AND DISCOVERY
// ============================================================================

/**
 * Get a managed model by name
 * 
 * Returns the managed model structure (not the model itself).
 * Use model_manager_acquire_read/write() to access the actual model.
 * 
 * @param name Name of the model
 * @return Pointer to managed model on success, NULL if not found
 */
ManagedModel* model_manager_get(const char* name);

/**
 * List all models
 * 
 * Returns an array of all managed models.
 * 
 * @param count Output: Number of models in the array
 * @return Array of managed models (caller must NOT free)
 */
ManagedModel** model_manager_list(uint32_t* count);

/**
 * Get count of registered models
 * 
 * @return Number of models registered in the manager
 */
uint32_t model_manager_count(void);

/**
 * Get model name at specific index (thread-safe)
 * Returns a copy of the model name that must be freed by caller
 * Returns NULL if index out of bounds
 */
char* model_manager_get_name_at_index(uint32_t index);

/**
 * Check if a model exists
 * 
 * @param name Name of the model
 * @return true if model exists, false otherwise
 */
bool model_manager_exists(const char* name);

/**
 * Get model status
 * 
 * @param name Name of the model
 * @param is_accessible Output: Is model accessible (abacus has enough primes)?
 * @param is_training Output: Is model currently being trained?
 * @param read_count Output: Number of active readers
 * @return true on success, false if model not found
 */
bool model_manager_get_status(const char* name, bool* is_accessible, 
                              bool* is_training, uint32_t* read_count);

/**
 * Get the first available model
 * 
 * Returns the first model in the manager's list.
 * Useful for getting a default model when no specific model is requested.
 * 
 * @return Pointer to first model on success, NULL if no models exist
 */
CLLMModel* model_manager_get_first(void);

/**
 * Get name of first registered model (for auto-selection)
 * Returns: Model name or NULL if no models
 */
const char* model_manager_get_first_name(void);

// ============================================================================
// DISK-BASED MODEL ACCESS (NEW - OBJECTIVE 26)
// ============================================================================

/**
 * Read model metadata without loading full model
 * 
 * Reads only the CLLMHeader from the model file.
 * This is fast (~2KB read) and doesn't load weights into memory.
 * Use this to check model properties before preparing/accessing.
 * 
 * @param path Path to model file
 * @return Pointer to CLLMHeader on success, NULL on failure (caller must free)
 */
CLLMHeader* model_manager_read_metadata(const char* path);

/**
 * Free metadata structure
 * 
 * @param header Metadata to free
 */
void model_manager_free_metadata(CLLMHeader* header);

/**
 * Check if abacus has enough primes for model
 * 
 * @param required_primes Number of primes needed
 * @return true if abacus has enough primes, false otherwise
 */
bool model_manager_check_abacus(uint64_t required_primes);

/**
 * Expand abacus to required prime count
 * 
 * Generates additional primes if needed.
 * This may take time for large prime counts.
 * 
 * @param required_primes Number of primes needed
 * @return true on success, false on failure
 */
bool model_manager_expand_abacus(uint64_t required_primes);

/**
 * Prepare model for use (ensure abacus has enough primes)
 * 
 * This function:
 * 1. Reads model metadata
 * 2. Checks num_primes_used
 * 3. Expands abacus if needed
 * 4. Marks model as "accessible"
 * 
 * After this, the model can be used for inference/training from disk.
 * 
 * @param name Name of the model
 * @return true on success, false on failure
 */
bool model_manager_prepare(const char* name);

/**
 * Rename a model
 * @param old_name Current model name
 * @param new_name New model name
 * @return true on success, false on failure
 * 
 * Note: Model must not be in use (read_count == 0 and not training)
 */
bool model_manager_rename(const char* old_name, const char* new_name);

#ifdef __cplusplus
}
#endif

#endif // CLLM_MODEL_MANAGER_H
