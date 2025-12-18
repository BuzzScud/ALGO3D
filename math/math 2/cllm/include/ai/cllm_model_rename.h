#ifndef CLLM_MODEL_RENAME_H
#define CLLM_MODEL_RENAME_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Rename model (complete operation)
 * 
 * This function:
 * 1. Validates new name is unique
 * 2. Loads the model
 * 3. Updates model name and queue directory
 * 4. Saves model with new name
 * 5. Renames queue directory
 * 6. Deletes old model file
 * 
 * Returns 0 on success, -1 on error
 */
int cllm_model_rename(const char* old_name, const char* new_name);

/**
 * Get model name from file path
 * Extracts name from path like "models/my_model.cllm" -> "my_model"
 * Caller must free returned string
 */
char* cllm_model_get_name_from_path(const char* filepath);

#ifdef __cplusplus
}
#endif

#endif /* CLLM_MODEL_RENAME_H */