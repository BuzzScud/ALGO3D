/**
 * cllm_model_registry.c - Model Registry Implementation
 * 
 * Lightweight registry for tracking model metadata.
 * Does NOT load models or coordinate access.
 */

#include "cllm_model_registry.h"
#include "cllm_format.h"
#include "cllm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>

// ============================================================================
// GLOBAL REGISTRY
// ============================================================================

static ModelRegistry g_registry = {0};
static bool g_initialized = false;

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * Find model metadata by name
 */
static ModelMetadata* find_model(const char* name) {
    for (uint32_t i = 0; i < g_registry.num_models; i++) {
        if (strcmp(g_registry.models[i]->name, name) == 0) {
            return g_registry.models[i];
        }
    }
    return NULL;
}

/**
 * Ensure models directory exists
 */
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

/**
 * Read model metadata from file
 */
static bool read_model_metadata(const char* path, ModelMetadata* metadata) {
    // Open file
    FILE* file = fopen(path, "rb");
    if (!file) {
        fprintf(stderr, "Failed to open model file: %s\n", path);
        return false;
    }
    
    // Read header
    CLLMHeader header;
    size_t read = fread(&header, sizeof(CLLMHeader), 1, file);
    fclose(file);
    
    if (read != 1) {
        fprintf(stderr, "Failed to read model header: %s\n", path);
        return false;
    }
    
    // Verify magic number (first 4 bytes should be "CLLM")
    if (memcmp(header.magic, "CLLM", 4) != 0) {
        fprintf(stderr, "Invalid model file (bad magic): %s\n", path);
        return false;
    }
    
    // Extract metadata from header
    metadata->vocab_size = (uint32_t)header.vocab_size;
    metadata->embedding_dim = (uint32_t)header.embedding_dim;
    metadata->num_layers = (uint32_t)header.num_layers;
    metadata->num_heads = header.num_heads;
    metadata->max_seq_len = header.max_seq_len;  // Use max_seq_len instead of context_length
    
    // Get file size
    struct stat st;
    if (stat(path, &st) == 0) {
        metadata->file_size = (uint64_t)st.st_size;
        metadata->created_time = st.st_ctime;
        metadata->modified_time = st.st_mtime;
    } else {
        metadata->file_size = 0;
        metadata->created_time = 0;
        metadata->modified_time = 0;
    }
    
    metadata->is_valid = true;
    
    return true;
}

/**
 * Add model to registry
 */
static bool add_model(ModelMetadata* metadata) {
    // Expand capacity if needed
    if (g_registry.num_models >= g_registry.capacity) {
        uint32_t new_capacity = g_registry.capacity == 0 ? 8 : g_registry.capacity * 2;
        ModelMetadata** new_models = (ModelMetadata**)realloc(
            g_registry.models,
            new_capacity * sizeof(ModelMetadata*)
        );
        if (!new_models) {
            fprintf(stderr, "Failed to expand registry capacity\n");
            return false;
        }
        g_registry.models = new_models;
        g_registry.capacity = new_capacity;
    }
    
    // Add model
    g_registry.models[g_registry.num_models++] = metadata;
    return true;
}

/**
 * Remove model from registry
 */
static bool remove_model(const char* name) {
    for (uint32_t i = 0; i < g_registry.num_models; i++) {
        if (strcmp(g_registry.models[i]->name, name) == 0) {
            // Free metadata
            free(g_registry.models[i]);
            
            // Shift remaining models
            for (uint32_t j = i; j < g_registry.num_models - 1; j++) {
                g_registry.models[j] = g_registry.models[j + 1];
            }
            
            g_registry.num_models--;
            return true;
        }
    }
    return false;
}

// ============================================================================
// REGISTRY LIFECYCLE
// ============================================================================

bool model_registry_init(const char* models_dir) {
    if (g_initialized) {
        return true;  // Already initialized
    }
    
    // Set models directory
    if (models_dir == NULL) {
        snprintf(g_registry.models_dir, MODEL_PATH_MAX, "./models");
    } else {
        snprintf(g_registry.models_dir, MODEL_PATH_MAX, "%s", models_dir);
    }
    
    // Create models directory if it doesn't exist
    if (!ensure_models_directory(g_registry.models_dir)) {
        return false;
    }
    
    // Initialize registry
    g_registry.models = NULL;
    g_registry.num_models = 0;
    g_registry.capacity = 0;
    
    // Initialize lock
    if (pthread_mutex_init(&g_registry.lock, NULL) != 0) {
        fprintf(stderr, "Failed to initialize registry lock\n");
        return false;
    }
    
    g_initialized = true;
    printf("Model registry initialized: %s\n", g_registry.models_dir);
    
    // Scan for existing models
    int found = model_registry_scan();
    if (found >= 0) {
        printf("Found %d model(s)\n", found);
    }
    
    return true;
}

void model_registry_cleanup(void) {
    if (!g_initialized) {
        return;
    }
    
    pthread_mutex_lock(&g_registry.lock);
    
    // Free all metadata
    for (uint32_t i = 0; i < g_registry.num_models; i++) {
        free(g_registry.models[i]);
    }
    free(g_registry.models);
    
    g_registry.models = NULL;
    g_registry.num_models = 0;
    g_registry.capacity = 0;
    
    pthread_mutex_unlock(&g_registry.lock);
    pthread_mutex_destroy(&g_registry.lock);
    
    g_initialized = false;
    printf("Model registry cleaned up\n");
}

bool model_registry_is_initialized(void) {
    return g_initialized;
}

// ============================================================================
// REGISTRY OPERATIONS
// ============================================================================

int model_registry_scan(void) {
    if (!g_initialized) {
        fprintf(stderr, "Registry not initialized\n");
        return -1;
    }
    
    pthread_mutex_lock(&g_registry.lock);
    
    // Clear existing models
    for (uint32_t i = 0; i < g_registry.num_models; i++) {
        free(g_registry.models[i]);
    }
    g_registry.num_models = 0;
    
    // Open directory
    DIR* dir = opendir(g_registry.models_dir);
    if (!dir) {
        fprintf(stderr, "Failed to open models directory: %s\n", strerror(errno));
        pthread_mutex_unlock(&g_registry.lock);
        return -1;
    }
    
    // Scan for .cllm files
    struct dirent* entry;
    int count = 0;
    
    while ((entry = readdir(dir)) != NULL) {
        // Check if file ends with .cllm
        size_t len = strlen(entry->d_name);
        if (len <= 5 || strcmp(entry->d_name + len - 5, ".cllm") != 0) {
            continue;
        }
        
        // Extract model name (remove .cllm extension)
        char model_name[MODEL_NAME_MAX];
        size_t name_len = (len - 5 < MODEL_NAME_MAX - 1) ? len - 5 : MODEL_NAME_MAX - 1;
        memcpy(model_name, entry->d_name, name_len);
        model_name[name_len] = '\0';
        
        // Build full path
        char model_path[MODEL_PATH_MAX];
        int path_len = snprintf(model_path, MODEL_PATH_MAX, "%s/%s",
                               g_registry.models_dir, entry->d_name);
        if (path_len >= MODEL_PATH_MAX) {
            fprintf(stderr, "Model path too long, skipping: %s\n", entry->d_name);
            continue;
        }
        
        // Create metadata
        ModelMetadata* metadata = (ModelMetadata*)calloc(1, sizeof(ModelMetadata));
        if (!metadata) {
            fprintf(stderr, "Failed to allocate metadata for %s\n", model_name);
            continue;
        }
        
        // Set name and path
        snprintf(metadata->name, MODEL_NAME_MAX, "%s", model_name);
        snprintf(metadata->path, MODEL_PATH_MAX, "%s", model_path);
        
        // Read metadata from file
        if (!read_model_metadata(model_path, metadata)) {
            fprintf(stderr, "Failed to read metadata for %s\n", model_name);
            free(metadata);
            continue;
        }
        
        // Add to registry
        if (!add_model(metadata)) {
            fprintf(stderr, "Failed to add model to registry: %s\n", model_name);
            free(metadata);
            continue;
        }
        
        count++;
        printf("  Registered: %s (%u vocab, %u dim, %u layers)\n",
               model_name, metadata->vocab_size, metadata->embedding_dim, metadata->num_layers);
    }
    
    closedir(dir);
    pthread_mutex_unlock(&g_registry.lock);
    
    return count;
}

bool model_registry_refresh(const char* name) {
    if (!g_initialized || !name) {
        return false;
    }
    
    pthread_mutex_lock(&g_registry.lock);
    
    ModelMetadata* metadata = find_model(name);
    if (!metadata) {
        pthread_mutex_unlock(&g_registry.lock);
        return false;
    }
    
    // Re-read metadata from file
    bool success = read_model_metadata(metadata->path, metadata);
    
    pthread_mutex_unlock(&g_registry.lock);
    
    return success;
}

const ModelMetadata* model_registry_get(const char* name) {
    if (!g_initialized || !name) {
        return NULL;
    }
    
    pthread_mutex_lock(&g_registry.lock);
    ModelMetadata* metadata = find_model(name);
    pthread_mutex_unlock(&g_registry.lock);
    
    return metadata;
}

const ModelMetadata* model_registry_get_at_index(uint32_t index) {
    if (!g_initialized || index >= g_registry.num_models) {
        return NULL;
    }
    
    pthread_mutex_lock(&g_registry.lock);
    ModelMetadata* metadata = g_registry.models[index];
    pthread_mutex_unlock(&g_registry.lock);
    
    return metadata;
}

uint32_t model_registry_count(void) {
    if (!g_initialized) {
        return 0;
    }
    
    pthread_mutex_lock(&g_registry.lock);
    uint32_t count = g_registry.num_models;
    pthread_mutex_unlock(&g_registry.lock);
    
    return count;
}

const char* model_registry_get_models_dir(void) {
    if (!g_initialized) {
        return NULL;
    }
    
    return g_registry.models_dir;
}

// ============================================================================
// MODEL MANAGEMENT
// ============================================================================

bool model_registry_register(const char* name, const char* path) {
    if (!g_initialized || !name || !path) {
        return false;
    }
    
    pthread_mutex_lock(&g_registry.lock);
    
    // Check if already exists
    if (find_model(name) != NULL) {
        fprintf(stderr, "Model already registered: %s\n", name);
        pthread_mutex_unlock(&g_registry.lock);
        return false;
    }
    
    // Create metadata
    ModelMetadata* metadata = (ModelMetadata*)calloc(1, sizeof(ModelMetadata));
    if (!metadata) {
        fprintf(stderr, "Failed to allocate metadata\n");
        pthread_mutex_unlock(&g_registry.lock);
        return false;
    }
    
    // Set name and path
    snprintf(metadata->name, MODEL_NAME_MAX, "%s", name);
    snprintf(metadata->path, MODEL_PATH_MAX, "%s", path);
    
    // Read metadata from file
    if (!read_model_metadata(path, metadata)) {
        free(metadata);
        pthread_mutex_unlock(&g_registry.lock);
        return false;
    }
    
    // Add to registry
    bool success = add_model(metadata);
    if (!success) {
        free(metadata);
    }
    
    pthread_mutex_unlock(&g_registry.lock);
    
    return success;
}

bool model_registry_unregister(const char* name) {
    if (!g_initialized || !name) {
        return false;
    }
    
    pthread_mutex_lock(&g_registry.lock);
    bool success = remove_model(name);
    pthread_mutex_unlock(&g_registry.lock);
    
    return success;
}

bool model_registry_rename(const char* old_name, const char* new_name) {
    if (!g_initialized || !old_name || !new_name) {
        return false;
    }
    
    pthread_mutex_lock(&g_registry.lock);
    
    // Find old model
    ModelMetadata* metadata = find_model(old_name);
    if (!metadata) {
        fprintf(stderr, "Model not found: %s\n", old_name);
        pthread_mutex_unlock(&g_registry.lock);
        return false;
    }
    
    // Check if new name already exists
    if (find_model(new_name) != NULL) {
        fprintf(stderr, "Model already exists: %s\n", new_name);
        pthread_mutex_unlock(&g_registry.lock);
        return false;
    }
    
    // Build new path
    char new_path[MODEL_PATH_MAX];
    int path_len = snprintf(new_path, MODEL_PATH_MAX, "%s/%s.cllm", g_registry.models_dir, new_name);
    if (path_len >= MODEL_PATH_MAX) {
        fprintf(stderr, "New path too long\n");
        pthread_mutex_unlock(&g_registry.lock);
        return false;
    }
    
    // Rename file
    if (rename(metadata->path, new_path) != 0) {
        fprintf(stderr, "Failed to rename file: %s\n", strerror(errno));
        pthread_mutex_unlock(&g_registry.lock);
        return false;
    }
    
    // Update metadata
    snprintf(metadata->name, MODEL_NAME_MAX, "%s", new_name);
    snprintf(metadata->path, MODEL_PATH_MAX, "%s", new_path);
    
    pthread_mutex_unlock(&g_registry.lock);
    
    printf("Renamed model: %s -> %s\n", old_name, new_name);
    return true;
}

bool model_registry_delete(const char* name) {
    if (!g_initialized || !name) {
        return false;
    }
    
    pthread_mutex_lock(&g_registry.lock);
    
    // Find model
    ModelMetadata* metadata = find_model(name);
    if (!metadata) {
        fprintf(stderr, "Model not found: %s\n", name);
        pthread_mutex_unlock(&g_registry.lock);
        return false;
    }
    
    // Delete file
    if (unlink(metadata->path) != 0) {
        fprintf(stderr, "Failed to delete file: %s\n", strerror(errno));
        pthread_mutex_unlock(&g_registry.lock);
        return false;
    }
    
    // Remove from registry
    remove_model(name);
    
    pthread_mutex_unlock(&g_registry.lock);
    
    printf("Deleted model: %s\n", name);
    return true;
}

bool model_registry_exists(const char* name) {
    if (!g_initialized || !name) {
        return false;
    }
    
    pthread_mutex_lock(&g_registry.lock);
    bool exists = (find_model(name) != NULL);
    pthread_mutex_unlock(&g_registry.lock);
    
    return exists;
}

// ============================================================================
// TRAINING DATA ASSOCIATION
// ============================================================================

bool model_registry_set_training_data(const char* name, const char* data_dir) {
    if (!g_initialized || !name || !data_dir) {
        return false;
    }
    
    pthread_mutex_lock(&g_registry.lock);
    
    ModelMetadata* metadata = find_model(name);
    if (!metadata) {
        pthread_mutex_unlock(&g_registry.lock);
        return false;
    }
    
    snprintf(metadata->training_data_dir, TRAINING_DATA_PATH_MAX, "%s", data_dir);
    
    pthread_mutex_unlock(&g_registry.lock);
    
    return true;
}

const char* model_registry_get_training_data(const char* name) {
    if (!g_initialized || !name) {
        return NULL;
    }
    
    pthread_mutex_lock(&g_registry.lock);
    
    ModelMetadata* metadata = find_model(name);
    if (!metadata || metadata->training_data_dir[0] == '\0') {
        pthread_mutex_unlock(&g_registry.lock);
        return NULL;
    }
    
    const char* data_dir = metadata->training_data_dir;
    pthread_mutex_unlock(&g_registry.lock);
    
    return data_dir;
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

bool model_registry_get_path(const char* name, char* path_out) {
    if (!g_initialized || !name || !path_out) {
        return false;
    }
    
    int len = snprintf(path_out, MODEL_PATH_MAX, "%s/%s.cllm",
                      g_registry.models_dir, name);
    
    return (len < MODEL_PATH_MAX);
}

bool model_registry_extract_name(const char* path, char* name_out) {
    if (!path || !name_out) {
        return false;
    }
    
    // Find last slash
    const char* filename = strrchr(path, '/');
    if (filename) {
        filename++;  // Skip the slash
    } else {
        filename = path;  // No slash, use whole path
    }
    
    // Find .cllm extension
    const char* ext = strstr(filename, ".cllm");
    if (!ext) {
        return false;
    }
    
    // Copy name (without extension)
    size_t name_len = ext - filename;
    if (name_len >= MODEL_NAME_MAX) {
        name_len = MODEL_NAME_MAX - 1;
    }
    
    memcpy(name_out, filename, name_len);
    name_out[name_len] = '\0';
    
    return true;
}

void model_registry_print(void) {
    if (!g_initialized) {
        printf("Registry not initialized\n");
        return;
    }
    
    pthread_mutex_lock(&g_registry.lock);
    
    printf("\n=== Model Registry ===\n");
    printf("Directory: %s\n", g_registry.models_dir);
    printf("Models: %u\n\n", g_registry.num_models);
    
    for (uint32_t i = 0; i < g_registry.num_models; i++) {
        ModelMetadata* m = g_registry.models[i];
        printf("%u. %s\n", i + 1, m->name);
        printf("   Path: %s\n", m->path);
        printf("   Vocab: %u, Dim: %u, Layers: %u, Heads: %u\n",
               m->vocab_size, m->embedding_dim, m->num_layers, m->num_heads);
        printf("   Size: %lu bytes\n", (unsigned long)m->file_size);
        if (m->training_data_dir[0] != '\0') {
            printf("   Training Data: %s\n", m->training_data_dir);
        }
        printf("\n");
    }
    
    pthread_mutex_unlock(&g_registry.lock);
}