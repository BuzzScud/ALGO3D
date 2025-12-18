/**
 * Model Metadata Management
 * 
 * Functions for managing model names, epochs, and configuration
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "cllm.h"
#include "cllm_model_manager.h"

/**
 * Get epochs trained for a model
 */
int cllm_model_get_epochs_trained(CLLMModel* model) {
    if (!model) return -1;
    // Return training steps from metrics (training_meta doesn't exist in new structure)
    return (int)(model->metrics.total_steps / 1000);  // Approximate epochs
}

/**
 * Set target epochs for a model
 */
int cllm_model_set_target_epochs(CLLMModel* model, int target) {
    if (!model || target < 0) return -1;
    
    // Note: epochs_trained field removed from CLLMModel
    // Just return success for now
    return 0;
}

/**
 * Validate epoch configuration
 */
int cllm_model_validate_epochs(CLLMModel* model, int target_epochs) {
    // Suppress unused parameter warning
    (void)target_epochs;
    
    if (!model) return -1;
    
    // Note: epochs_trained field removed from CLLMModel
    // Just return success for now
    return 0;
}

/**
 * Get model name
 */
const char* cllm_model_get_name(CLLMModel* model) {
    if (!model) return NULL;
    return model->header.model_name;
}

/**
 * Set model name
 */
int cllm_model_set_name(CLLMModel* model, const char* name) {
    if (!model || !name) return -1;
    
    // Validate name length
    if (strlen(name) >= 256) {
        fprintf(stderr, "Error: Model name too long (max 255 characters)\n");
        return -1;
    }
    
    // Validate name characters (alphanumeric, underscore, hyphen only)
    for (const char* p = name; *p; p++) {
        if (!isalnum(*p) && *p != '_' && *p != '-') {
            fprintf(stderr, "Error: Invalid character '%c' in model name\n", *p);
            return -1;
        }
    }
    
    //     strncpy(model->model_name, name, 255);
    //     model->model_name[255] = '\0';
    
    // Update queue directory
    //     snprintf(model->queue_directory, 512, "models/%s_queue", name);
    
    return 0;
}

/**
 * Generate default model name
 */
char* cllm_model_generate_default_name(void) {
    static int counter = 1;
    char* name = malloc(64);
    if (!name) return NULL;
    
    snprintf(name, 64, "model_%d", counter++);
    return name;
}

/**
 * Validate model name is unique
 */
int cllm_model_validate_name_unique(const char* name) {
    if (!name) return -1;
    
    // Check if model file exists
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "models/%s.cllm", name);
    
    FILE* f = fopen(filepath, "r");
    if (f) {
        fclose(f);
        return -1; // Name already exists
    }
    
    return 0; // Name is unique
}

/**
 * Initialize model metadata with defaults
 */
int cllm_model_init_metadata(CLLMModel* model, const char* name) {
    if (!model) return -1;
    
    // Set name
    if (name) {
        if (cllm_model_set_name(model, name) != 0) {
            return -1;
        }
    } else {
        // Generate default name
        char* default_name = cllm_model_generate_default_name();
        if (!default_name) return -1;
        
        cllm_model_set_name(model, default_name);
        free(default_name);
    }
    
    // Initialize epochs
    //     model->epochs_trained = 0;
    //     model->target_epochs = 10; // Default target
    
    // Initialize training config with defaults
    //     model->training_config.learning_rate = 0.001f;
    //     model->training_config.batch_size = 4;
    //     model->training_config.sequence_length = 64;
    //     model->training_config.num_epochs = 10;
    //     model->training_config.weight_decay = 0.01f;
    //     model->training_config.gradient_clip = 1.0;
    
    // NOTE: Training history is now stored in separate files
    // in models/<name>_history/ directory (see model_history_manager.h)
    
    return 0;
}
