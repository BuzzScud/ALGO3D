/**
 * cllm_model_manager.c - Global Model Manager Implementation
 * 
 * Provides global model management with concurrent access control.
 */

#include "cllm_model_manager.h"
#include "cllm_utils.h"
#include "cllm_format.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <dirent.h>

// Forward declarations for save/load (will implement later)
static bool cllm_save_model_internal(CLLMModel* model, const char* path);
static CLLMModel* cllm_load_model_internal(const char* path);

// Global model manager instance
static ModelManager g_model_manager = {0};
static bool g_manager_initialized = false;

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

static ManagedModel* find_model_by_name(const char* name) {
    for (uint32_t i = 0; i < g_model_manager.num_models; i++) {
        if (strcmp(g_model_manager.models[i]->name, name) == 0) {
            return g_model_manager.models[i];
        }
    }
    return NULL;
}

static bool ensure_models_directory(const char* dir) {
    struct stat st = {0};
    if (stat(dir, &st) == -1) {
        if (mkdir(dir, 0755) == -1) {
            fprintf(stderr, "Failed to create models directory: %s\n", strerror(errno));
            return false;
        }
    }
    return true;
}

// ============================================================================
// INITIALIZATION AND CLEANUP
// ============================================================================

bool model_manager_init(const char* models_dir) {
    if (g_manager_initialized) {
        return true;  // Already initialized
    }
    
    // Set models directory
    if (models_dir == NULL) {
        snprintf(g_model_manager.models_dir, MODEL_PATH_MAX, "./models");
    } else {
        snprintf(g_model_manager.models_dir, MODEL_PATH_MAX, "%s", models_dir);
    }
    
    // Create models directory if it doesn't exist
    if (!ensure_models_directory(g_model_manager.models_dir)) {
        return false;
    }
    
    // Initialize manager
    g_model_manager.models = NULL;
    g_model_manager.num_models = 0;
    g_model_manager.capacity = 0;
    
    // Initialize manager lock
    if (pthread_mutex_init(&g_model_manager.manager_lock, NULL) != 0) {
        fprintf(stderr, "Failed to initialize manager lock\n");
        return false;
    }
    
    g_manager_initialized = true;
    printf("Model manager initialized: %s\n", g_model_manager.models_dir);
    
    // CRITICAL FIX: Do NOT auto-load models during initialization
    // Models can be very large (12GB+ for 50K vocab) and cause OOM
    // Instead, scan directory and register model names only
    printf("Scanning for available models (not loading yet)...\n");
    DIR* dir = opendir(g_model_manager.models_dir);
    if (dir) {
        struct dirent* entry;
        int found_count = 0;
        
        while ((entry = readdir(dir)) != NULL) {
            // Check if file ends with .cllm
            size_t len = strlen(entry->d_name);
            if (len > 5 && strcmp(entry->d_name + len - 5, ".cllm") == 0) {
                // Extract model name (remove .cllm extension)
                char model_name[MODEL_NAME_MAX];
                size_t name_len = (len - 5 < MODEL_NAME_MAX - 1) ? len - 5 : MODEL_NAME_MAX - 1;
                memcpy(model_name, entry->d_name, name_len);
                model_name[name_len] = '\0';
                
                // Build full path (with bounds checking to prevent truncation)
                char model_path[MODEL_PATH_MAX];
                int path_len = snprintf(model_path, MODEL_PATH_MAX, "%s/%s", 
                                       g_model_manager.models_dir, entry->d_name);
                if (path_len >= MODEL_PATH_MAX) {
                    fprintf(stderr, "Model path too long, skipping: %s/%s\n", 
                           g_model_manager.models_dir, entry->d_name);
                    continue;
                }
                
                // Register the model (without loading it)
                // Create managed model entry
                ManagedModel* managed = (ManagedModel*)calloc(1, sizeof(ManagedModel));
                if (!managed) {
                    fprintf(stderr, "Failed to allocate managed model for %s\n", model_name);
                    continue;
                }
                
                // Set name and path
                snprintf(managed->name, MODEL_NAME_MAX, "%s", model_name);
                snprintf(managed->path, MODEL_PATH_MAX, "%s", model_path);
                
                // Model is NOT loaded yet (lazy loading)
                managed->model = NULL;
                managed->is_accessible = false;  // Will be set to true when loaded
                managed->is_training = false;
                managed->read_count = 0;
                
                // Initialize lock
                if (pthread_rwlock_init(&managed->lock, NULL) != 0) {
                    fprintf(stderr, "Failed to initialize model lock for %s\n", model_name);
                    free(managed);
                    continue;
                }
                
                // Add to manager array
                if (g_model_manager.num_models >= g_model_manager.capacity) {
                    uint32_t new_capacity = g_model_manager.capacity == 0 ? 4 : g_model_manager.capacity * 2;
                    ManagedModel** new_models = (ManagedModel**)realloc(
                        g_model_manager.models, 
                        new_capacity * sizeof(ManagedModel*)
                    );
                    if (!new_models) {
                        fprintf(stderr, "Failed to expand models array\n");
                        pthread_rwlock_destroy(&managed->lock);
                        free(managed);
                        continue;
                    }
                    g_model_manager.models = new_models;
                    g_model_manager.capacity = new_capacity;
                }
                
                g_model_manager.models[g_model_manager.num_models++] = managed;
                
                printf("  Registered model: %s (will load on-demand)\n", model_name);
                found_count++;
            }
        }
        closedir(dir);
        
        printf("Registered %d model(s) - will load on-demand to save memory\n", found_count);
    } else {
        printf("No existing models found (directory empty or inaccessible)\n");
    }
    
    return true;
}

void model_manager_cleanup(void) {
    if (!g_manager_initialized) {
        return;
    }
    
    pthread_mutex_lock(&g_model_manager.manager_lock);
    
    // Free all models
    for (uint32_t i = 0; i < g_model_manager.num_models; i++) {
        ManagedModel* managed = g_model_manager.models[i];
        if (managed) {
            // Destroy lock
            pthread_rwlock_destroy(&managed->lock);
            
            // Free model if loaded
            if (managed->model) {
                cllm_free_model(managed->model);
            }
            
            free(managed);
        }
    }
    
    free(g_model_manager.models);
    g_model_manager.models = NULL;
    g_model_manager.num_models = 0;
    g_model_manager.capacity = 0;
    
    pthread_mutex_unlock(&g_model_manager.manager_lock);
    pthread_mutex_destroy(&g_model_manager.manager_lock);
    
    g_manager_initialized = false;
    printf("Model manager cleaned up\n");
}

// ============================================================================
// MODEL CREATION AND LOADING
// ============================================================================

ManagedModel* model_manager_create(const char* name, const CLLMConfig* config) {
    if (!g_manager_initialized || !name || !config) {
        return NULL;
    }
    
    pthread_mutex_lock(&g_model_manager.manager_lock);
    
    // Check if model already exists
    if (find_model_by_name(name) != NULL) {
        fprintf(stderr, "Model '%s' already exists\n", name);
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return NULL;
    }
    
    // Create managed model
    ManagedModel* managed = (ManagedModel*)calloc(1, sizeof(ManagedModel));
    if (!managed) {
        fprintf(stderr, "Failed to allocate managed model\n");
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return NULL;
    }
    
    // Set name and path
    snprintf(managed->name, MODEL_NAME_MAX, "%s", name);
    int path_len = snprintf(managed->path, MODEL_PATH_MAX, "%s/%s.cllm", 
                           g_model_manager.models_dir, name);
    if (path_len >= MODEL_PATH_MAX) {
        fprintf(stderr, "Model path too long\n");
        free(managed);
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return NULL;
    }
    
    // Create the model
    managed->model = cllm_create_model(config);
    if (!managed->model) {
        fprintf(stderr, "Failed to create model\n");
        free(managed);
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return NULL;
    }
    
    // Initialize lock
    if (pthread_rwlock_init(&managed->lock, NULL) != 0) {
        fprintf(stderr, "Failed to initialize model lock\n");
        cllm_free_model(managed->model);
        free(managed);
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return NULL;
    }
    
    // Set metadata
    managed->is_accessible = true;
    managed->is_training = false;
    managed->read_count = 0;
    managed->vocab_size = config->vocab_size;
    managed->embedding_dim = config->embedding_dim;
    managed->num_layers = config->num_layers;
    managed->num_heads = config->num_heads;
    managed->created_time = (uint64_t)time(NULL);
    managed->modified_time = managed->created_time;
    
    // Add to manager
    if (g_model_manager.num_models >= g_model_manager.capacity) {
        uint32_t new_capacity = g_model_manager.capacity == 0 ? 4 : g_model_manager.capacity * 2;
        ManagedModel** new_models = (ManagedModel**)realloc(
            g_model_manager.models, 
            new_capacity * sizeof(ManagedModel*)
        );
        if (!new_models) {
            fprintf(stderr, "Failed to expand models array\n");
            pthread_rwlock_destroy(&managed->lock);
            cllm_free_model(managed->model);
            free(managed);
            pthread_mutex_unlock(&g_model_manager.manager_lock);
            return NULL;
        }
        g_model_manager.models = new_models;
        g_model_manager.capacity = new_capacity;
    }
    
    g_model_manager.models[g_model_manager.num_models++] = managed;
    
    pthread_mutex_unlock(&g_model_manager.manager_lock);
    
    printf("Created model '%s' (%u vocab, %u dim, %u layers)\n", 
           name, config->vocab_size, config->embedding_dim, config->num_layers);
    
    // Auto-save the model to disk
    printf("Auto-saving model to disk: %s\n", managed->path);
    if (cllm_save_model_internal(managed->model, managed->path)) {
        printf("  ✓ Model saved successfully\n");
    } else {
        printf("  ✗ Warning: Failed to save model to disk\n");
    }
    
    return managed;
}

ManagedModel* model_manager_load(const char* name, const char* path) {
    if (!g_manager_initialized || !name || !path) {
        return NULL;
    }
    
    pthread_mutex_lock(&g_model_manager.manager_lock);
    
    // Check if model already exists
    if (find_model_by_name(name) != NULL) {
        fprintf(stderr, "Model '%s' already exists\n", name);
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return NULL;
    }
    
    // Load the model
    CLLMModel* model = cllm_load_model_internal(path);
    if (!model) {
        fprintf(stderr, "Failed to load model from '%s'\n", path);
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return NULL;
    }
    
    // Create managed model
    ManagedModel* managed = (ManagedModel*)calloc(1, sizeof(ManagedModel));
    if (!managed) {
        fprintf(stderr, "Failed to allocate managed model\n");
        cllm_free_model(model);
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return NULL;
    }
    
    // Set name and path
    snprintf(managed->name, MODEL_NAME_MAX, "%s", name);
    snprintf(managed->path, MODEL_PATH_MAX, "%s", path);
    
    managed->model = model;
    
    // Initialize lock
    if (pthread_rwlock_init(&managed->lock, NULL) != 0) {
        fprintf(stderr, "Failed to initialize model lock\n");
        cllm_free_model(model);
        free(managed);
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return NULL;
    }
    
    // Set metadata from model
    managed->is_accessible = true;  // Changed from is_loaded
    managed->is_training = false;
    managed->read_count = 0;
    managed->vocab_size = (uint32_t)model->vocab_size;
    managed->embedding_dim = (uint32_t)model->embedding_dim;
    managed->num_layers = model->num_layers;
    managed->num_heads = model->header.num_heads;
    //     managed->required_primes = model->header.num_primes_used > 0 ? 
    //                                model->header.num_primes_used : model->vocab_size;
    managed->created_time = (uint64_t)time(NULL);
    managed->modified_time = managed->created_time;
    
    // Add to manager
    if (g_model_manager.num_models >= g_model_manager.capacity) {
        uint32_t new_capacity = g_model_manager.capacity == 0 ? 4 : g_model_manager.capacity * 2;
        ManagedModel** new_models = (ManagedModel**)realloc(
            g_model_manager.models, 
            new_capacity * sizeof(ManagedModel*)
        );
        if (!new_models) {
            fprintf(stderr, "Failed to expand models array\n");
            pthread_rwlock_destroy(&managed->lock);
            cllm_free_model(model);
            free(managed);
            pthread_mutex_unlock(&g_model_manager.manager_lock);
            return NULL;
        }
        g_model_manager.models = new_models;
        g_model_manager.capacity = new_capacity;
    }
    
    g_model_manager.models[g_model_manager.num_models++] = managed;
    
    pthread_mutex_unlock(&g_model_manager.manager_lock);
    
    printf("Loaded model '%s' from '%s'\n", name, path);
    
    return managed;
}

bool model_manager_save(const char* name) {
    if (!g_manager_initialized || !name) {
        return false;
    }
    
    pthread_mutex_lock(&g_model_manager.manager_lock);
    
    ManagedModel* managed = find_model_by_name(name);
    if (!managed) {
        fprintf(stderr, "Model '%s' not found\n", name);
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return false;
    }
    
    if (!managed->is_accessible || !managed->model) {
        fprintf(stderr, "Model '%s' is not loaded\n", name);
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return false;
    }
    
    // Save the model
    bool success = cllm_save_model_internal(managed->model, managed->path);
    if (success) {
        managed->modified_time = (uint64_t)time(NULL);
        printf("Saved model '%s' to '%s'\n", name, managed->path);
    } else {
        fprintf(stderr, "Failed to save model '%s'\n", name);
    }
    
    pthread_mutex_unlock(&g_model_manager.manager_lock);
    
    return success;
}

// Continued in next message due to length...

bool model_manager_unload(const char* name) {
    if (!g_manager_initialized || !name) {
        return false;
    }
    
    pthread_mutex_lock(&g_model_manager.manager_lock);
    
    ManagedModel* managed = find_model_by_name(name);
    if (!managed) {
        fprintf(stderr, "Model '%s' not found\n", name);
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return false;
    }
    
    if (!managed->is_accessible) {
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return true;  // Already unloaded
    }
    
    // Free the model
    if (managed->model) {
        cllm_free_model(managed->model);
        managed->model = NULL;
    }
    
    managed->is_accessible = false;
    
    pthread_mutex_unlock(&g_model_manager.manager_lock);
    
    printf("Unloaded model '%s'\n", name);
    return true;
}

bool model_manager_reload(const char* name) {
    if (!g_manager_initialized || !name) {
        return false;
    }
    
    pthread_mutex_lock(&g_model_manager.manager_lock);
    
    ManagedModel* managed = find_model_by_name(name);
    if (!managed) {
        fprintf(stderr, "Model '%s' not found\n", name);
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return false;
    }
    
    if (managed->is_accessible) {
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return true;  // Already loaded
    }
    
    // Load the model
    managed->model = cllm_load_model_internal(managed->path);
    if (!managed->model) {
        fprintf(stderr, "Failed to reload model '%s'\n", name);
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return false;
    }
    
    managed->is_accessible = true;
    
    pthread_mutex_unlock(&g_model_manager.manager_lock);
    
    printf("Reloaded model '%s'\n", name);
    return true;
}

bool model_manager_delete(const char* name, bool delete_file) {
    if (!g_manager_initialized || !name) {
        return false;
    }
    
    pthread_mutex_lock(&g_model_manager.manager_lock);
    
    // Find model index
    int index = -1;
    for (uint32_t i = 0; i < g_model_manager.num_models; i++) {
        if (strcmp(g_model_manager.models[i]->name, name) == 0) {
            index = (int)i;
            break;
        }
    }
    
    if (index == -1) {
        fprintf(stderr, "Model '%s' not found\n", name);
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return false;
    }
    
    ManagedModel* managed = g_model_manager.models[index];
    
    // Delete file if requested
    if (delete_file) {
        if (remove(managed->path) != 0) {
            fprintf(stderr, "Failed to delete model file '%s'\n", managed->path);
        }
    }
    
    // Free model
    if (managed->model) {
        cllm_free_model(managed->model);
    }
    
    // Destroy lock
    pthread_rwlock_destroy(&managed->lock);
    
    // Free managed model
    free(managed);
    
    // Remove from array
    for (uint32_t i = (uint32_t)index; i < g_model_manager.num_models - 1; i++) {
        g_model_manager.models[i] = g_model_manager.models[i + 1];
    }
    g_model_manager.num_models--;
    
    pthread_mutex_unlock(&g_model_manager.manager_lock);
    
    printf("Deleted model '%s'\n", name);
    return true;
}

// ============================================================================
// MODEL ACCESS CONTROL
// ============================================================================

CLLMModel* model_manager_acquire_read(const char* name) {
    if (!g_manager_initialized || !name) {
        return NULL;
    }
    
    pthread_mutex_lock(&g_model_manager.manager_lock);
    
    ManagedModel* managed = find_model_by_name(name);
    if (!managed) {
        fprintf(stderr, "Model '%s' not found\n", name);
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return NULL;
    }
    
    // NO LAZY LOADING: Model must be explicitly loaded first
    if (!managed->is_accessible || !managed->model) {
        fprintf(stderr, "Model '%s' is not accessible\n", name);
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return NULL;
    }
    
    pthread_mutex_unlock(&g_model_manager.manager_lock);
    
    // Acquire read lock
    pthread_rwlock_rdlock(&managed->lock);
    
    // Increment read count
    pthread_mutex_lock(&g_model_manager.manager_lock);
    managed->read_count++;
    pthread_mutex_unlock(&g_model_manager.manager_lock);
    
    return managed->model;
}

CLLMModel* model_manager_acquire_write(const char* name) {
    if (!g_manager_initialized || !name) {
        return NULL;
    }
    
    pthread_mutex_lock(&g_model_manager.manager_lock);
    
    ManagedModel* managed = find_model_by_name(name);
    if (!managed) {
        fprintf(stderr, "Model '%s' not found\n", name);
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return NULL;
    }
    
    // NO LAZY LOADING: Model must be explicitly loaded first
    if (!managed->is_accessible || !managed->model) {
        fprintf(stderr, "Model '%s' is not accessible\n", name);
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return NULL;
    }
    
    pthread_mutex_unlock(&g_model_manager.manager_lock);
    
    // Acquire write lock
    pthread_rwlock_wrlock(&managed->lock);
    
    // Set training flag
    pthread_mutex_lock(&g_model_manager.manager_lock);
    managed->is_training = true;
    pthread_mutex_unlock(&g_model_manager.manager_lock);
    
    return managed->model;

}

void model_manager_release_read(const char* name) {
    if (!g_manager_initialized || !name) {
        return;
    }
    
    pthread_mutex_lock(&g_model_manager.manager_lock);
    
    ManagedModel* managed = find_model_by_name(name);
    if (!managed) {
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return;
    }
    
    // Decrement read count
    if (managed->read_count > 0) {
        managed->read_count--;
    }
    
    pthread_mutex_unlock(&g_model_manager.manager_lock);
    
    // Release read lock
    pthread_rwlock_unlock(&managed->lock);
}

void model_manager_release_write(const char* name) {
    if (!g_manager_initialized || !name) {
        return;
    }
    
    pthread_mutex_lock(&g_model_manager.manager_lock);
    
    ManagedModel* managed = find_model_by_name(name);
    if (!managed) {
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return;
    }
    
    // Clear training flag
    managed->is_training = false;
    
    pthread_mutex_unlock(&g_model_manager.manager_lock);
    
    // Release write lock
    pthread_rwlock_unlock(&managed->lock);
}

// ============================================================================
// MODEL QUERY AND DISCOVERY
// ============================================================================

ManagedModel* model_manager_get(const char* name) {
    if (!g_manager_initialized || !name) {
        return NULL;
    }
    
    pthread_mutex_lock(&g_model_manager.manager_lock);
    ManagedModel* managed = find_model_by_name(name);
    pthread_mutex_unlock(&g_model_manager.manager_lock);
    
    return managed;
}

ManagedModel** model_manager_list(uint32_t* count) {
    printf("DEBUG: model_manager_list called\n");
    fflush(stdout);
    
    if (!g_manager_initialized || !count) {
        printf("DEBUG: model_manager_list: not initialized or null count\n");
        fflush(stdout);
        return NULL;
    }
    
    printf("DEBUG: model_manager_list: acquiring lock...\n");
    fflush(stdout);
    
    pthread_mutex_lock(&g_model_manager.manager_lock);
    
    printf("DEBUG: model_manager_list: lock acquired, num_models=%u\n", g_model_manager.num_models);
    fflush(stdout);
    
    *count = g_model_manager.num_models;
    ManagedModel** models = g_model_manager.models;
    
    pthread_mutex_unlock(&g_model_manager.manager_lock);
    
    printf("DEBUG: model_manager_list: returning\n");
    fflush(stdout);
    
    return models;
}

uint32_t model_manager_count(void) {
    if (!g_manager_initialized) {
        return 0;
    }
    
    pthread_mutex_lock(&g_model_manager.manager_lock);
    uint32_t count = g_model_manager.num_models;
    pthread_mutex_unlock(&g_model_manager.manager_lock);
    
    return count;
}

// Thread-safe function to get model name at index
// Returns NULL if index out of bounds or model doesn't exist
// Caller must free the returned string
char* model_manager_get_name_at_index(uint32_t index) {
    if (!g_manager_initialized) {
        return NULL;
    }
    
    pthread_mutex_lock(&g_model_manager.manager_lock);
    
    if (index >= g_model_manager.num_models || !g_model_manager.models[index]) {
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return NULL;
    }
    
    char* name = strdup(g_model_manager.models[index]->name);
    
    pthread_mutex_unlock(&g_model_manager.manager_lock);
    
    return name;
}

bool model_manager_exists(const char* name) {
    if (!g_manager_initialized || !name) {
        return false;
    }
    
    pthread_mutex_lock(&g_model_manager.manager_lock);
    bool exists = (find_model_by_name(name) != NULL);
    pthread_mutex_unlock(&g_model_manager.manager_lock);
    
    return exists;
}

bool model_manager_get_status(const char* name, bool* is_accessible, 
                              bool* is_training, uint32_t* read_count) {
    if (!g_manager_initialized || !name) {
        return false;
    }
    
    pthread_mutex_lock(&g_model_manager.manager_lock);
    
    ManagedModel* managed = find_model_by_name(name);
    if (!managed) {
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return false;
    }
    
    if (is_accessible) *is_accessible = managed->is_accessible;
    if (is_training) *is_training = managed->is_training;
    if (read_count) *read_count = managed->read_count;
    
    pthread_mutex_unlock(&g_model_manager.manager_lock);
    
    return true;
}

CLLMModel* model_manager_get_first(void) {
    if (!g_manager_initialized) {
        return NULL;
    }
    
    pthread_mutex_lock(&g_model_manager.manager_lock);
    
    // Return first model if any exist
    CLLMModel* model = NULL;
    if (g_model_manager.num_models > 0 && g_model_manager.models[0]) {
        model = g_model_manager.models[0]->model;
    }
    
    pthread_mutex_unlock(&g_model_manager.manager_lock);
    
    return model;
}

const char* model_manager_get_first_name(void) {
    if (!g_manager_initialized) {
        return NULL;
    }
    
    pthread_mutex_lock(&g_model_manager.manager_lock);
    
    // Return first model name if any exist
    const char* name = NULL;
    if (g_model_manager.num_models > 0 && g_model_manager.models[0]) {
        name = g_model_manager.models[0]->name;
    }
    
    pthread_mutex_unlock(&g_model_manager.manager_lock);
    
    return name;
}

// ============================================================================
// INTERNAL SAVE/LOAD FUNCTIONS (Stubs for now)
// ============================================================================

static bool cllm_save_model_internal(CLLMModel* model, const char* path) {
    if (!model || !path) {
        return false;
    }
    
    // Use proper cllm_write_model function
    int result = cllm_write_model(model, path);
    if (result != 0) {
        fprintf(stderr, "Failed to save model to: %s\n", path);
        return false;
    }
    
    return true;
}

static CLLMModel* cllm_load_model_internal(const char* path) {
    if (!path) {
        return NULL;
    }
    
    // Use proper cllm_read_model function
    CLLMModel* model = cllm_read_model(path);
    if (!model) {
        fprintf(stderr, "Failed to load model from: %s\n", path);
        return NULL;
    }
    
    return model;
}

// ============================================================================
// DISK-BASED MODEL ACCESS IMPLEMENTATION (OBJECTIVE 26)
// ============================================================================

CLLMHeader* model_manager_read_metadata(const char* path) {
    if (!path) {
        fprintf(stderr, "model_manager_read_metadata: NULL path\n");
        return NULL;
    }
    
    FILE* file = fopen(path, "rb");
    if (!file) {
        fprintf(stderr, "Failed to open model file for metadata: %s\n", path);
        return NULL;
    }
    
    // Allocate header
    CLLMHeader* header = (CLLMHeader*)malloc(sizeof(CLLMHeader));
    if (!header) {
        fprintf(stderr, "Failed to allocate memory for header\n");
        fclose(file);
        return NULL;
    }
    
    // Read header only (fast - just ~2KB)
    if (fread(header, sizeof(CLLMHeader), 1, file) != 1) {
        fprintf(stderr, "Failed to read model header from: %s\n", path);
        free(header);
        fclose(file);
        return NULL;
    }
    
    fclose(file);
    
    // Validate magic number
    if (memcmp(header->magic, "CLLM\x01\x00\x00\x00", 8) != 0) {
        fprintf(stderr, "Invalid model file (bad magic number): %s\n", path);
        free(header);
        return NULL;
    }
    
    return header;
}

void model_manager_free_metadata(CLLMHeader* header) {
    if (header) {
        free(header);
    }
}

bool model_manager_check_abacus(uint64_t required_primes) {
    // Get current prime count from rainbow table
    extern int rainbow_table_get_count(void);
    int available = rainbow_table_get_count();
    
    return (uint64_t)available >= required_primes;
}

bool model_manager_expand_abacus(uint64_t required_primes) {
    // Get current prime count
    extern int rainbow_table_get_count(void);
    extern int rainbow_table_generate_primes(int target_count);
    
    int available = rainbow_table_get_count();
    
    if ((uint64_t)available >= required_primes) {
        // Already have enough primes
        return true;
    }
    
    printf("Expanding abacus: %d -> %lu primes\n", 
           available, (unsigned long)required_primes);
    
    // Generate additional primes
    int result = rainbow_table_generate_primes((int)required_primes);
    
    if (result < 0) {
        fprintf(stderr, "Failed to expand abacus to %lu primes\n", 
                (unsigned long)required_primes);
        return false;
    }
    
    printf("✓ Abacus expanded successfully to %d primes\n", result);
    
    return true;
}

bool model_manager_prepare(const char* name) {
    if (!g_manager_initialized || !name) {
        fprintf(stderr, "model_manager_prepare: Invalid parameters\n");
        return false;
    }
    
    pthread_mutex_lock(&g_model_manager.manager_lock);
    
    // Find the managed model
    ManagedModel* managed = find_model_by_name(name);
    if (!managed) {
        fprintf(stderr, "Model not found: %s\n", name);
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return false;
    }
    
    // Read metadata to get required prime count
    CLLMHeader* header = model_manager_read_metadata(managed->path);
    if (!header) {
        fprintf(stderr, "Failed to read metadata for model: %s\n", name);
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return false;
    }
    
    // Get required primes (use vocab_size if num_primes_used is 0)
    uint64_t required = header->vocab_size;
    if (required == 0) {
        // Fallback: estimate from vocab_size
        required = header->vocab_size;
        printf("Note: Model doesn't specify prime count, estimating from vocab_size: %lu\n",
               (unsigned long)required);
    }
    
    // Cache metadata
    managed->required_primes = required;
    managed->vocab_size = (uint32_t)header->vocab_size;
    managed->embedding_dim = (uint32_t)header->embedding_dim;
    managed->num_layers = (uint32_t)header->num_layers;
    managed->num_heads = header->num_heads;
    
    model_manager_free_metadata(header);
    
    // Check if abacus has enough primes
    if (!model_manager_check_abacus(required)) {
        printf("Model '%s' requires %lu primes, expanding abacus...\n",
               name, (unsigned long)required);
        
        if (!model_manager_expand_abacus(required)) {
            fprintf(stderr, "Failed to expand abacus for model: %s\n", name);
            pthread_mutex_unlock(&g_model_manager.manager_lock);
            return false;
        }
    }
    
    // Mark model as accessible
    managed->is_accessible = true;
    
    pthread_mutex_unlock(&g_model_manager.manager_lock);
    
    printf("✓ Model '%s' is now accessible (requires %lu primes)\n",
           name, (unsigned long)required);
    
    return true;
}

bool model_manager_rename(const char* old_name, const char* new_name) {
    if (!g_manager_initialized || !old_name || !new_name) {
        fprintf(stderr, "model_manager_rename: Invalid parameters\n");
        return false;
    }
    
    pthread_mutex_lock(&g_model_manager.manager_lock);
    
    // Find the model to rename
    ManagedModel* managed = find_model_by_name(old_name);
    if (!managed) {
        fprintf(stderr, "Model not found: %s\n", old_name);
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return false;
    }
    
    // Check if new name already exists
    if (find_model_by_name(new_name)) {
        fprintf(stderr, "Model '%s' already exists\n", new_name);
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return false;
    }
    
    // Check if model is currently in use
    if (managed->read_count > 0 || managed->is_training) {
        fprintf(stderr, "Cannot rename model '%s' - currently in use\n", old_name);
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return false;
    }
    
    // Construct new path
    char new_path[1024];
    int path_len = snprintf(new_path, sizeof(new_path), "%s/%s.cllm", 
                            g_model_manager.models_dir, new_name);
    if (path_len < 0 || path_len >= (int)sizeof(new_path)) {
        fprintf(stderr, "Model path too long for rename\n");
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return false;
    }
    
    // Rename file on disk
    if (rename(managed->path, new_path) != 0) {
        perror("Failed to rename model file");
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return false;
    }
    
    // Update managed model structure
    strncpy(managed->name, new_name, sizeof(managed->name) - 1);
    managed->name[sizeof(managed->name) - 1] = '\0';
    strncpy(managed->path, new_path, sizeof(managed->path) - 1);
    managed->path[sizeof(managed->path) - 1] = '\0';
    
    pthread_mutex_unlock(&g_model_manager.manager_lock);
    
    printf("✓ Model renamed: %s → %s\n", old_name, new_name);
    return true;
}
