/**
 * Continuous Training System
 * 
 * Monitors training queue and trains on new files as they arrive
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <time.h>
#include "cllm.h"
#include "cllm_batch.h"
#include "cllm_training.h"
#include "ai/cllm_training_system.h"
#include "cllm_format.h"
#include "cllm_utils.h"
#include "cllm_model_manager.h"

#define MAX_TOKENS_PER_FILE 100000

// Real-time stats update thread for crawler
static pthread_t crawler_stats_update_thread __attribute__((unused));
static volatile int crawler_stats_thread_running = 0;

/**
 * Get current timestamp string
 */
static void get_timestamp(char* buffer, size_t size) {
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    strftime(buffer, size, "[%H:%M:%S]", tm_info);
}

typedef struct {
    char data_dir[1024];
    char model_path[1024];
    CLLMModel* model;
    CLLMTraining* training;
    int running;
    int files_trained;
    int num_threads;
    pthread_mutex_t lock;
    void* app_state;  // NEW: AppState* for sphere stats updates (void* to avoid circular dependency)
} ContinuousTrainingState;

/**
 * Check if file is locked by another thread
 */
static int is_file_locked(const char* filepath) {
    char lockpath[2048];
    // Truncate filepath to prevent buffer overflow (max 2042 chars for ".lock")
    size_t len = strlen(filepath);
    if (len > 2042) {
        char truncated[2043];
        strncpy(truncated, filepath, 2042);
        truncated[2042] = '\0';
        snprintf(lockpath, sizeof(lockpath), "%s.lock", truncated);
    } else {
        snprintf(lockpath, sizeof(lockpath), "%s.lock", filepath);
    }
    return access(lockpath, F_OK) == 0;
}

/**
 * Create lock file
 */
static int create_lock(const char* filepath) {
    char lockpath[2048];
    // Truncate filepath to prevent buffer overflow (max 2042 chars for ".lock")
    size_t len = strlen(filepath);
    if (len > 2042) {
        char truncated[2043];
        strncpy(truncated, filepath, 2042);
        truncated[2042] = '\0';
        snprintf(lockpath, sizeof(lockpath), "%s.lock", truncated);
    } else {
        snprintf(lockpath, sizeof(lockpath), "%s.lock", filepath);
    }
    
    FILE* f = fopen(lockpath, "w");
    if (!f) return -1;
    
    fprintf(f, "%d\n", getpid());
    fclose(f);
    return 0;
}

/**
 * Remove lock file
 */
static void remove_lock(const char* filepath) {
    char lockpath[2048];
    // Truncate filepath to prevent buffer overflow (max 2042 chars for ".lock")
    size_t len = strlen(filepath);
    if (len > 2042) {
        char truncated[2043];
        strncpy(truncated, filepath, 2042);
        truncated[2042] = '\0';
        snprintf(lockpath, sizeof(lockpath), "%s.lock", truncated);
    } else {
        snprintf(lockpath, sizeof(lockpath), "%s.lock", filepath);
    }
    unlink(lockpath);
}

/**
 * Load tokens from file
 */
static int load_tokens_from_file(const char* filepath, uint32_t** tokens, size_t* num_tokens) {
    FILE* f = fopen(filepath, "r");
    if (!f) {
        char timestamp[32];
        get_timestamp(timestamp, sizeof(timestamp));
        fprintf(stderr, "%s Cannot open: %s\n", timestamp, filepath);
        return -1;
    }
    
    // Skip header lines and read token line
    char line[4096];
    int found = 0;
    while (fgets(line, sizeof(line), f)) {
        if (line[0] != '#') {
            found = 1;
            break;
        }
    }
    fclose(f);
    
    if (!found) {
        char timestamp[32];
        get_timestamp(timestamp, sizeof(timestamp));
        fprintf(stderr, "%s No tokens in: %s\n", timestamp, filepath);
        return -1;
    }
    
    // Count tokens
    int count = 0;
    char* p = line;
    while (*p) {
        while (*p && (*p == ' ' || *p == '\t' || *p == '\n')) p++;
        if (*p) {
            count++;
            while (*p && *p != ' ' && *p != '\t' && *p != '\n') p++;
        }
    }
    
    if (count == 0) {
        return -1;
    }
    
    // Allocate
    *tokens = (uint32_t*)malloc(count * sizeof(uint32_t));
    if (!*tokens) return -1;
    
    // Parse tokens
    int i = 0;
    p = line;
    char token[256];
    while (*p && i < count) {
        while (*p && (*p == ' ' || *p == '\t' || *p == '\n')) p++;
        if (!*p) break;
        
        int j = 0;
        while (*p && *p != ' ' && *p != '\t' && *p != '\n' && j < 255) {
            token[j++] = *p++;
        }
        token[j] = '\0';
        
        // Hash to ID
        unsigned long hash = 5381;
        for (char* tp = token; *tp; tp++) {
            hash = ((hash << 5) + hash) + *tp;
        }
        (*tokens)[i++] = (uint32_t)(hash % 10000);
    }
    
    *num_tokens = i;
    return 0;
}


/**
 * Update sphere stats for visualization (if AppState is available)
 */
static void update_crawler_sphere_stats(ContinuousTrainingState* state, CLLMTrainingSystem* system) {
    if (!state->app_state) return;  // No AppState available
    
    // Cast void* to AppState* (avoiding circular dependency)
    typedef struct {
        char _padding[1024];  // Skip to sphere_stats offset
        struct {
            int batches_processed[12];
            float avg_loss[12];
            int active_spheres;
            float total_gradient_norm;
            int total_batches;
        } sphere_stats;
        pthread_mutex_t sphere_stats_mutex;
    } AppStateForStats;
    
    AppStateForStats* app_state = (AppStateForStats*)state->app_state;
    
    // Lock mutex for thread-safe access
    pthread_mutex_lock(&app_state->sphere_stats_mutex);
    
    // Get number of workers
    // TODO: Implement get_num_workers for 88D system
    int num_workers = system ? system->num_threads : 0;
    app_state->sphere_stats.active_spheres = num_workers;
    app_state->sphere_stats.total_batches = 0;
    
    // Update per-sphere stats
    // TODO: Implement get_sphere_stats for 88D system
    for (int i = 0; i < num_workers && i < 12; i++) {
        int batches = 0;
        float loss = 0.0f;
        // Stub: would need to implement per-thread stats tracking
        if (system && i < (int)system->num_threads) {
            batches = (int)(system->batches_processed / system->num_threads);
            loss = (float)(system->epoch_loss / system->batches_processed);
            app_state->sphere_stats.batches_processed[i] = batches;
            app_state->sphere_stats.avg_loss[i] = loss;
            app_state->sphere_stats.total_batches += batches;
        }
    }
    
    // Update gradient norm
    // TODO: Implement get_gradient_norm for 88D system
    app_state->sphere_stats.total_gradient_norm = 0.0f; // Stub
    
    pthread_mutex_unlock(&app_state->sphere_stats_mutex);
}

/**
 * Real-time stats update thread for crawler - continuously polls sphere stats during training
 */
static void* crawler_stats_update_thread_func(void* arg) __attribute__((unused));
static void* crawler_stats_update_thread_func(void* arg) {
   ContinuousTrainingState* state = (ContinuousTrainingState*)arg;
   
   printf("✓ Crawler real-time stats update thread started\n");
   
   // Get the threaded system from the training context
   CLLMTrainingSystem* system = NULL;
   
   while (crawler_stats_thread_running && state->running) {
       // Get current threaded system (it's created per training session)
       // We'll update stats when system is available
       if (system) {
           update_crawler_sphere_stats(state, system);
       }
       
       // Sleep for 100ms (interruptible)
       for (int i = 0; i < 10 && crawler_stats_thread_running; i++) {
           usleep(10000);  // 10ms chunks for quick termination
       }
   }
   
   printf("✓ Crawler real-time stats update thread stopped\n");
   return NULL;
}

/**
 * Train on one file
 */
static int train_on_file(ContinuousTrainingState* state, const char* filepath) {
    printf("\n=== Training on file ===\n");
    printf("File: %s\n", filepath);
    
    // Load tokens
    uint32_t* tokens = NULL;
    size_t num_tokens = 0;
    
    if (load_tokens_from_file(filepath, &tokens, &num_tokens) != 0) {
        fprintf(stderr, "Failed to load tokens from: %s\n", filepath);
        return -1;
    }
    
    printf("Loaded %zu tokens\n", num_tokens);
    
    // Update training data
    if (state->training->tokens) {
        free(state->training->tokens);
    }
    state->training->tokens = tokens;
    state->training->num_tokens = num_tokens;
    
    // Train for N epochs using parallel system
    int epochs = 5;
    float total_loss = 0.0f;
    
    // Create parallel training system (use all available cores)
    int num_threads = sysconf(_SC_NPROCESSORS_ONLN);
    if (num_threads > 1) num_threads--;  // Reserve 1 for main thread
    if (num_threads < 1) num_threads = 1;
    
    // Create batch iterator
    CLLMBatchIterator* batch_iterator = cllm_batch_iterator_create(
        state->training->tokens,
        state->training->num_tokens,
        state->training->config.batch_size,
        state->training->config.sequence_length,
        0,  // shuffle = false
        0   // drop_last = false
    );
    
    if (!batch_iterator) {
        fprintf(stderr, "Failed to create batch iterator\n");
        return -1;
    }
    
    // Create parallel training system
    CLLMTrainingSystem* threaded_system = cllm_training_system_create(
        state->model,
        state->training,
        batch_iterator,
        num_threads
    );
    
    if (!threaded_system) {
        fprintf(stderr, "Failed to create parallel training system\n");
        cllm_batch_iterator_free(batch_iterator);
        return -1;
    }
    
    printf("Using %d parallel workers for training\n", num_threads);
    
    for (int epoch = 0; epoch < epochs && state->running; epoch++) {
        // Use parallel training (crystalline loss, multi-threaded)
        float loss = (float)cllm_train_epoch(state->training);
        total_loss += loss;
        printf("  Epoch %d/%d: loss = %.4f\n", epoch + 1, epochs, loss);
        
        // Update sphere stats for UI visualization
        update_crawler_sphere_stats(state, threaded_system);
    }
    
    // Cleanup parallel system
    cllm_training_system_free(threaded_system);
    cllm_batch_iterator_free(batch_iterator);
    
    float avg_loss = total_loss / epochs;
    printf("✓ Training complete: avg loss = %.4f\n", avg_loss);
    
    // Save model
    // cllm_write_model is declared in cllm_format.h
    if (cllm_write_model(state->model, state->model_path) == 0) {
        printf("✓ Model saved: %s\n", state->model_path);
    }
    
    return 0;
}

/**
 * Move file to trained directory
 */
static int move_to_trained(const char* data_dir, const char* filename) {
    char src[2048];
    char dst[2048];
    
    snprintf(src, sizeof(src), "%s/training_queue/%s", data_dir, filename);
    snprintf(dst, sizeof(dst), "%s/trained/%s", data_dir, filename);
    
    if (rename(src, dst) != 0) {
        fprintf(stderr, "Failed to move file: %s\n", filename);
        return -1;
    }
    
    printf("✓ Moved to trained: %s\n", filename);
    return 0;
}

/**
 * Training worker thread
 */
static void* training_worker_thread(void* arg) {
    ContinuousTrainingState* state = (ContinuousTrainingState*)arg;
    
    char queue_dir[2048];
    snprintf(queue_dir, sizeof(queue_dir), "%s/training_queue", state->data_dir);
    
    while (state->running) {
        DIR* dir = opendir(queue_dir);
        if (!dir) {
            // Interruptible sleep - check state->running every 100ms
            for (int i = 0; i < 50 && state->running; i++) {
                usleep(100000);  // 100ms
            }
            continue;
        }
        
        struct dirent* entry;
        int found_file = 0;
        
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_name[0] == '.') continue;
            if (strstr(entry->d_name, ".tok") == NULL) continue;
            
            char filepath[4096];  // Increased from 2048 to accommodate long paths
            // Truncate filename to prevent buffer overflow (max 2000 chars for 4096 buffer)
            char safe_name[2048];  // Increased from 256 to allow longer filenames
            strncpy(safe_name, entry->d_name, sizeof(safe_name) - 1);
            safe_name[sizeof(safe_name) - 1] = '\0';
            
            snprintf(filepath, sizeof(filepath), "%s/%s", queue_dir, safe_name);
            
            // Check if locked
            if (is_file_locked(filepath)) {
                continue;
            }
            
            // Try to lock
            if (create_lock(filepath) != 0) {
                continue;
            }
            
            // Train on file
            if (train_on_file(state, filepath) == 0) {
                // Move to trained
                move_to_trained(state->data_dir, entry->d_name);
                
                pthread_mutex_lock(&state->lock);
                state->files_trained++;
                pthread_mutex_unlock(&state->lock);
            }
            
            // Remove lock
            remove_lock(filepath);
            
            found_file = 1;
            break;  // Process one file at a time per thread
        }
        
        closedir(dir);
        
        if (!found_file) {
            // Interruptible sleep - check state->running every 100ms
            for (int i = 0; i < 50 && state->running; i++) {
                usleep(100000);  // 100ms
            }
        }
    }
    
    return NULL;
}

/**
 * Initialize continuous training
 */
ContinuousTrainingState* continuous_training_init(const char* data_dir, const char* model_path, 
                                                   CLLMModel* model, int num_threads, void* app_state) {
    ContinuousTrainingState* state = (ContinuousTrainingState*)calloc(1, sizeof(ContinuousTrainingState));
    if (!state) return NULL;
    
    strncpy(state->data_dir, data_dir, sizeof(state->data_dir) - 1);
    state->app_state = app_state;  // Store AppState for sphere stats updates
    
    // Only copy model_path if it's not NULL
    if (model_path) {
        strncpy(state->model_path, model_path, sizeof(state->model_path) - 1);
    } else {
        state->model_path[0] = '\0';  // Empty string if no path
    }
    
    state->running = 1;
    state->files_trained = 0;
    state->num_threads = num_threads;
    pthread_mutex_init(&state->lock, NULL);
    
    // NEW: Load or create model if not provided
    if (model) {
        // If model provided, use it directly (legacy support)
        state->model = model;
    } else {
        // Extract model name from path (e.g., "models/crawler.cllm" -> "crawler")
        char model_name[256] = "crawler_model";
        const char* last_slash = strrchr(model_path, '/');
        if (last_slash) {
            const char* name_start = last_slash + 1;
            const char* dot = strrchr(name_start, '.');
            if (dot) {
                size_t len = dot - name_start;
                if (len < sizeof(model_name)) {
                    strncpy(model_name, name_start, len);
                    model_name[len] = '\0';
                }
            }
        }
        
        char timestamp[32];
        get_timestamp(timestamp, sizeof(timestamp));
        
           
           // First, check if model exists and prepare it
           extern bool model_manager_exists(const char* name);
           extern bool model_manager_prepare(const char* name);
           extern bool model_manager_reload(const char* name);
           
           bool model_exists = model_manager_exists(model_name);
           
           if (model_exists) {
               // Model exists, prepare and load it
               printf("%s Model '%s' exists, preparing...\n", timestamp, model_name);
               if (!model_manager_prepare(model_name)) {
                   fprintf(stderr, "%s Failed to prepare model '%s'\n", timestamp, model_name);
                   free(state);
                   return NULL;
               }
               
               if (!model_manager_reload(model_name)) {
                   fprintf(stderr, "%s Failed to load model '%s' into memory\n", timestamp, model_name);
                   free(state);
                   return NULL;
               }
               
               printf("%s Model '%s' prepared and loaded\n", timestamp, model_name);
           } else {
               // Model doesn't exist, create it
               printf("%s Creating new model '%s' via model manager...\n", timestamp, model_name);
               
               // Create default config for crawler (using new geometric structure)
               CLLMConfig default_config = {
                   .solid_type = PLATONIC_CUBE,  // Use cube geometry
                   .vocab_size = 50000,          // Increased from 10K (supports larger vocabulary)
                   .max_seq_len = 1024,          // Increased from 512 (longer context)
                   .embedding_dim = 0,           // Auto-calculate from geometry (8 vertices × 12 = 96)
                   .hidden_dim = 0,              // Auto-calculate from geometry (12 edges × 12 = 144)
                   .num_layers = 0,              // Auto-calculate from geometry (6 faces)
                   .num_heads = 0,               // Auto-calculate (always 12)
                   .enable_blind_recovery = true,
                   .enable_harmonic_integration = true,
                   .enable_ntt_attention = true,
                   .enable_kissing_spheres = true,
                   .num_threads = 0,             // Auto (13 for kissing spheres)
                   .optimizer_type = OPTIMIZER_ADAM,
                   .learning_rate = 0.001,
                   .beta1 = 0.9,
                   .beta2 = 0.999,
                   .epsilon = 1e-8,
                   .weight_decay = 0.01,
                   .ntt_threshold_seq_len = 512,
                   .ntt_auto_select = true
               };
               
               // Create model through model manager (returns ManagedModel* not int!)
               ManagedModel* created = model_manager_create(model_name, &default_config);
               if (!created) {
                   fprintf(stderr, "%s Failed to create model via model manager\n", timestamp);
                   free(state);
                   return NULL;
               }
               
               printf("%s Model '%s' created successfully\n", timestamp, model_name);
           }
           
           // Now acquire the model for training (it should be accessible now)
           state->model = model_manager_acquire_write(model_name);
           if (!state->model) {
               fprintf(stderr, "%s Failed to acquire model '%s' for training\n", timestamp, model_name);
               free(state);
               return NULL;
           }
           
           printf("%s Model '%s' acquired for training\n", timestamp, model_name);
    }
    
    // Initialize training state
    // Use default configuration (model no longer stores training_config)
    CLLMTrainingConfig config = (CLLMTrainingConfig){
        .num_epochs = 10,
        .batch_size = 32,
        .sequence_length = 128,
        .learning_rate = 0.001f,
        .weight_decay = 0.01f,
        .gradient_clip = 1.0f,
    };
    strcpy(config.optimizer, "adam");
    strcpy(config.lr_scheduler, "cosine");  // Use cosine decay by default
    
    state->training = cllm_training_init(state->model, &config);
    if (!state->training) {
        free(state);
        return NULL;
    }
    
    return state;
}

/**
 * Start training threads
 */
int continuous_training_start(ContinuousTrainingState* state, pthread_t* threads) {
    char timestamp[32];
    get_timestamp(timestamp, sizeof(timestamp));
    
    printf("%s === CONTINUOUS TRAINING STARTED ===\n", timestamp);
    printf("%s Threads: %d\n", timestamp, state->num_threads);
    printf("%s Model: %s\n", timestamp, state->model_path);
    
    for (int i = 0; i < state->num_threads; i++) {
        if (pthread_create(&threads[i], NULL, training_worker_thread, state) != 0) {
            fprintf(stderr, "%s Failed to create training thread %d\n", timestamp, i);
            return -1;
        }
    }
    
    return 0;
}

/**
 * Stop training threads
 */
void continuous_training_stop(ContinuousTrainingState* state, pthread_t* threads) {
    state->running = 0;
    
    for (int i = 0; i < state->num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    char timestamp[32];
    get_timestamp(timestamp, sizeof(timestamp));
    printf("%s === CONTINUOUS TRAINING STOPPED ===\n", timestamp);
    printf("%s Total files trained: %d\n", timestamp, state->files_trained);
}

/**
 * Cleanup continuous training
 */
void continuous_training_cleanup(ContinuousTrainingState* state) {
    if (!state) return;
    
    if (state->training) {
        cllm_training_free(state->training);
    }
    
    // Release model back to model manager
    if (state->model) {
        // Extract model name from path to release it
        char model_name[256] = "crawler_model";
        const char* last_slash = strrchr(state->model_path, '/');
        if (last_slash) {
            const char* name_start = last_slash + 1;
            const char* dot = strrchr(name_start, '.');
            if (dot) {
                size_t len = dot - name_start;
                if (len < sizeof(model_name)) {
                    strncpy(model_name, name_start, len);
                    model_name[len] = '\0';
                }
            }
        }
        model_manager_release_write(model_name);
    }
    
    pthread_mutex_destroy(&state->lock);
    free(state);
}