/**
 * Crawler High-Level API
 * 
 * Provides a clean, thread-safe API for both CLI and GUI applications.
 * This is the ONLY interface applications should use.
 */

#include "../../include/crawler.h"
#include "../../include/cllm_model_manager.h"
#include "content_filter.h"
#include "preprocessor.h"
#include "crawler_url_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>


// Helper to detect CPU cores
static int detect_cpu_cores() {
    int cores = (int)sysconf(_SC_NPROCESSORS_ONLN);
    if (cores <= 0) cores = 2; // Fallback to 2 if detection fails
    // Use cores-1 to leave one core for system/control
    int worker_cores = cores - 1;
    if (worker_cores < 1) worker_cores = 1; // Minimum 1 thread
    return worker_cores;
}

// Main state structure
struct CrawlerState {
    // Configuration
    char data_dir[1024];
    char start_url[512];
    int max_pages;
    int num_threads;  // Number of threads per stage (preprocessor, tokenizer, training)
    char model_name[256];  // NEW: Selected model name for training
    
    ExtractionMode extraction_mode;  // Content extraction mode
    
    // Thread handles
    pthread_t crawler_thread;
    pthread_t* preprocessor_threads;  // Array of preprocessor threads
    pthread_t* tokenizer_threads;     // Array of tokenizer threads
    pthread_t monitor_thread;
    pthread_t* training_threads;      // Array of training threads
    
    // Component states (opaque pointers to internal implementations)
    void* crawler_internal;
    void* preprocessor_internal;
    void* tokenizer_internal;
    void* training_internal;  // NEW: Training state
    void* url_manager;        // CrawlerURLManager* for database integration
    
    // Status tracking
    int running;
    int pages_crawled;
    int pages_preprocessed;
    int pages_tokenized;
    int pages_trained;
    char current_url[512];
    char last_error[512];
    
    // Callback
    CrawlerCallback callback;
    void* callback_user_data;
    
    // Thread safety
    pthread_mutex_t status_lock;
};

// Note: We use the actual function declarations from crawler.h
// The internal state structures are opaque (void*)

// Helper to count files in directory
static int count_files_in_dir(const char* dir_path) {
    DIR* dir = opendir(dir_path);
    if (!dir) return 0;
    
    int count = 0;
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] != '.') {
            count++;
        }
    }
    closedir(dir);
    return count;
}

// Helper to trigger callback
static void trigger_callback(CrawlerState* state, CrawlerEventType type, const char* message) {
    if (!state || !state->callback) return;
    
    CrawlerEvent event;
    event.type = type;
    strncpy(event.message, message ? message : "", sizeof(event.message) - 1);
    event.message[sizeof(event.message) - 1] = '\0';
    event.pages_crawled = state->pages_crawled;
    
    state->callback(&event, state->callback_user_data);
}

// Status monitoring thread
static void* status_monitor_thread_func(void* arg) {
    CrawlerState* state = (CrawlerState*)arg;
    
    char raw_dir[2048], preprocessed_dir[2048], queue_dir[2048], trained_dir[2048];
    snprintf(raw_dir, sizeof(raw_dir), "%s/raw_pages", state->data_dir);
    snprintf(preprocessed_dir, sizeof(preprocessed_dir), "%s/preprocessed", state->data_dir);
    snprintf(queue_dir, sizeof(queue_dir), "%s/training_queue", state->data_dir);
    snprintf(trained_dir, sizeof(trained_dir), "%s/trained", state->data_dir);
    
    int last_crawled = 0, last_preprocessed = 0, last_tokenized = 0, last_trained = 0;
    
    while (state->running) {
        // Count files
        int crawled = count_files_in_dir(raw_dir);
        int preprocessed = count_files_in_dir(preprocessed_dir);
        int tokenized = count_files_in_dir(queue_dir);
        int trained = count_files_in_dir(trained_dir);
        
        // Update status
        pthread_mutex_lock(&state->status_lock);
        state->pages_crawled = crawled;
        state->pages_preprocessed = preprocessed;
        state->pages_tokenized = tokenized;
        state->pages_trained = trained;
        pthread_mutex_unlock(&state->status_lock);
        
        // Trigger callbacks for changes
        if (crawled > last_crawled) {
            char msg[256];
            snprintf(msg, sizeof(msg), "Downloaded %d new page(s)", crawled - last_crawled);
            trigger_callback(state, CRAWLER_EVENT_PAGE_DOWNLOADED, msg);
            last_crawled = crawled;
        }
        
        if (preprocessed > last_preprocessed) {
            char msg[256];
            snprintf(msg, sizeof(msg), "Preprocessed %d new page(s)", preprocessed - last_preprocessed);
            trigger_callback(state, CRAWLER_EVENT_PAGE_PREPROCESSED, msg);
            last_preprocessed = preprocessed;
        }
        
        if (tokenized > last_tokenized) {
            char msg[256];
            snprintf(msg, sizeof(msg), "Tokenized %d new page(s)", tokenized - last_tokenized);
            trigger_callback(state, CRAWLER_EVENT_PAGE_TOKENIZED, msg);
            last_tokenized = tokenized;
        }
        
        if (trained > last_trained) {
            char msg[256];
            snprintf(msg, sizeof(msg), "Trained on %d new page(s)", trained - last_trained);
            trigger_callback(state, CRAWLER_EVENT_PAGE_TRAINED, msg);
            last_trained = trained;
        }
        
        sleep(2);
    }
    
    return NULL;
}

// ============================================================================
// PUBLIC API IMPLEMENTATION
// ============================================================================

CrawlerState* crawler_state_init(const char* data_dir, const char* start_url, int max_pages) {
    return crawler_state_init_threaded(data_dir, start_url, max_pages, 0);
}

CrawlerState* crawler_state_init_threaded(const char* data_dir, const char* start_url, 
                                          int max_pages, int num_threads) {
    CrawlerState* state = (CrawlerState*)calloc(1, sizeof(CrawlerState));
    if (!state) return NULL;
    
    strncpy(state->data_dir, data_dir, sizeof(state->data_dir) - 1);
    
    // start_url is now optional - if NULL, crawler uses database URLs only
    if (start_url) {
        strncpy(state->start_url, start_url, sizeof(state->start_url) - 1);
    } else {
        state->start_url[0] = '\0';
    }
    
    state->max_pages = max_pages;
    state->running = 0;
    
    // Auto-detect CPU cores if num_threads is 0
    if (num_threads <= 0) {
        state->num_threads = detect_cpu_cores();
    } else {
        state->num_threads = num_threads;
    }
    
    // Set default extraction mode
    state->extraction_mode = EXTRACT_ALL;
    
    // Initialize thread arrays to NULL
    state->preprocessor_threads = NULL;
    state->tokenizer_threads = NULL;
    state->training_threads = NULL;
    state->training_internal = NULL;
    
    pthread_mutex_init(&state->status_lock, NULL);
    
    // Create directory structure
    mkdir(data_dir, 0755);
    
    char path[1024];
    snprintf(path, sizeof(path), "%s/raw_pages", data_dir);
    mkdir(path, 0755);
    
    snprintf(path, sizeof(path), "%s/preprocessed", data_dir);
    mkdir(path, 0755);
    
    snprintf(path, sizeof(path), "%s/training_queue", data_dir);
    mkdir(path, 0755);
    
    snprintf(path, sizeof(path), "%s/trained", data_dir);
    mkdir(path, 0755);
    
    return state;
}

int crawler_start(CrawlerState* state) {
    if (!state) return -1;
    if (state->running) return -1;
    
    state->running = 1;
    
    printf("Starting crawler with %d threads per stage\n", state->num_threads);
    
    // Initialize components
    state->crawler_internal = crawler_internal_init(state->data_dir, state->start_url, state->max_pages);
    if (!state->crawler_internal) {
        state->running = 0;
        return -1;
    }
    
    // Pass URL manager to internal crawler
    if (state->url_manager) {
        crawler_internal_set_url_manager((CrawlerStateInternal*)state->crawler_internal, state->url_manager);
        
        // Set global URL manager for preprocessor link extraction
        extern CrawlerURLManager* g_crawler_url_manager;
        g_crawler_url_manager = (CrawlerURLManager*)state->url_manager;
    }
    
    state->preprocessor_internal = preprocessor_init(state->data_dir);
    state->tokenizer_internal = tokenizer_init(state->data_dir);
    
    // Initialize training component
    // Use specified model from model manager
    char model_path[2048];
    CLLMModel* existing_model = NULL;
    
    // Try to use the specified model name
    if (state->model_name[0] != '\0') {
        // Model name was specified - try to load it
        printf("✓ Loading specified model: %s\n", state->model_name);
        
        // First reload the model to ensure it's registered
        extern bool model_manager_reload(const char* name);
        model_manager_reload(state->model_name);
        
        // Prepare model (expands abacus if needed, marks accessible)
        extern bool model_manager_prepare(const char* name);
        extern CLLMModel* model_manager_acquire_read(const char* name);
        
        printf("Preparing model '%s' for training...\n", state->model_name);
        if (model_manager_prepare(state->model_name)) {
            printf("✓ Model prepared successfully\n");
            
            // Load the model into memory
            if (model_manager_reload(state->model_name)) {
                printf("✓ Model loaded into memory\n");
            } else {
                fprintf(stderr, "⚠ Warning: Failed to load model into memory\n");
            }
            
            // Now acquire it for training
            existing_model = model_manager_acquire_read(state->model_name);
            
            if (existing_model) {
                printf("✓ Successfully loaded model: %s\n", state->model_name);
                state->training_internal = continuous_training_init(state->data_dir, NULL, existing_model, state->num_threads, state->callback_user_data);
            } else {
                fprintf(stderr, "⚠ Warning: Could not acquire model after preparation\n");
                snprintf(model_path, sizeof(model_path), "%s/model.cllm", state->data_dir);
                state->training_internal = continuous_training_init(state->data_dir, model_path, NULL, state->num_threads, state->callback_user_data);
            }
        } else {
            fprintf(stderr, "⚠ Warning: Could not prepare model '%s', will create new one\n", state->model_name);
            snprintf(model_path, sizeof(model_path), "%s/model.cllm", state->data_dir);
            state->training_internal = continuous_training_init(state->data_dir, model_path, NULL, state->num_threads, state->callback_user_data);
        }
    } else {
        // No model name specified - try to use first available
        extern CLLMModel* model_manager_get_first(void);
        extern const char* model_manager_get_first_name(void);
        extern bool model_manager_prepare(const char* name);
        
        const char* first_name = model_manager_get_first_name();
        if (first_name) {
            printf("Preparing first available model: %s\n", first_name);
            if (model_manager_prepare(first_name)) {
                   // Load the model into memory
                   if (model_manager_reload(first_name)) {
                       printf("✓ Model loaded into memory\n");
                   } else {
                       fprintf(stderr, "⚠ Warning: Failed to load model into memory\n");
                   }
                existing_model = model_manager_get_first();
                if (existing_model) {
                    printf("✓ Using first available model from model manager\n");
                    state->training_internal = continuous_training_init(state->data_dir, NULL, existing_model, state->num_threads, state->callback_user_data);
                } else {
                    fprintf(stderr, "⚠ Warning: Could not acquire first model after preparation\n");
                    snprintf(model_path, sizeof(model_path), "%s/model.cllm", state->data_dir);
                    state->training_internal = continuous_training_init(state->data_dir, model_path, NULL, state->num_threads, state->callback_user_data);
                }
            } else {
                fprintf(stderr, "⚠ Warning: Could not prepare first model\n");
                snprintf(model_path, sizeof(model_path), "%s/model.cllm", state->data_dir);
                state->training_internal = continuous_training_init(state->data_dir, model_path, NULL, state->num_threads, state->callback_user_data);
            }
        } else {
            // No existing model, create new one
            snprintf(model_path, sizeof(model_path), "%s/model.cllm", state->data_dir);
            printf("⚠ No existing model found, will create new one\n");
            state->training_internal = continuous_training_init(state->data_dir, model_path, NULL, state->num_threads, state->callback_user_data);
        }
    }
    
    // Start crawler thread (single thread for sequential downloading)
    if (pthread_create(&state->crawler_thread, NULL, crawler_thread_func, state->crawler_internal) != 0) {
        state->running = 0;
        crawler_internal_cleanup(state->crawler_internal);
        return -1;
    }
    
    // Start multiple preprocessor threads
    state->preprocessor_threads = (pthread_t*)malloc(state->num_threads * sizeof(pthread_t));
    if (!state->preprocessor_threads) {
        state->running = 0;
        pthread_cancel(state->crawler_thread);
        crawler_internal_cleanup(state->crawler_internal);
        return -1;
    }
    
    for (int i = 0; i < state->num_threads; i++) {
        if (pthread_create(&state->preprocessor_threads[i], NULL, preprocessor_thread_func, state->preprocessor_internal) != 0) {
            fprintf(stderr, "Warning: Failed to start preprocessor thread %d\n", i);
        }
    }
    
    // Start multiple tokenizer threads
    state->tokenizer_threads = (pthread_t*)malloc(state->num_threads * sizeof(pthread_t));
    if (!state->tokenizer_threads) {
        state->running = 0;
        pthread_cancel(state->crawler_thread);
        for (int i = 0; i < state->num_threads; i++) {
            pthread_cancel(state->preprocessor_threads[i]);
        }
        free(state->preprocessor_threads);
        crawler_internal_cleanup(state->crawler_internal);
        preprocessor_cleanup(state->preprocessor_internal);
        return -1;
    }
    
    for (int i = 0; i < state->num_threads; i++) {
        if (pthread_create(&state->tokenizer_threads[i], NULL, tokenizer_thread_func, state->tokenizer_internal) != 0) {
            fprintf(stderr, "Warning: Failed to start tokenizer thread %d\n", i);
        }
    }
    
    // Start training threads
    if (state->training_internal) {
        state->training_threads = (pthread_t*)malloc(state->num_threads * sizeof(pthread_t));
        if (state->training_threads) {
            if (continuous_training_start(state->training_internal, state->training_threads) != 0) {
                fprintf(stderr, "Warning: Failed to start training threads\n");
                free(state->training_threads);
                state->training_threads = NULL;
            }
        }
    }
    
    // Start monitor thread
    pthread_create(&state->monitor_thread, NULL, status_monitor_thread_func, state);
    
    trigger_callback(state, CRAWLER_EVENT_PAGE_DOWNLOADED, "Crawler started");
    
    return 0;
}

void crawler_stop(CrawlerState* state) {
    if (!state || !state->running) return;
    
    state->running = 0;
    
    printf("Stopping crawler threads...\n");
    
    // Stop training threads first
    if (state->training_internal && state->training_threads) {
        continuous_training_stop(state->training_internal, state->training_threads);
        free(state->training_threads);
        state->training_threads = NULL;
    }
    
    // Wait for crawler thread
    if (state->crawler_thread) pthread_join(state->crawler_thread, NULL);
    
    // Wait for all preprocessor threads
    if (state->preprocessor_threads) {
        for (int i = 0; i < state->num_threads; i++) {
            pthread_join(state->preprocessor_threads[i], NULL);
        }
        free(state->preprocessor_threads);
        state->preprocessor_threads = NULL;
    }
    
    // Wait for all tokenizer threads
    if (state->tokenizer_threads) {
        for (int i = 0; i < state->num_threads; i++) {
            pthread_join(state->tokenizer_threads[i], NULL);
        }
        free(state->tokenizer_threads);
        state->tokenizer_threads = NULL;
    }
    
    // Wait for monitor thread
    if (state->monitor_thread) pthread_join(state->monitor_thread, NULL);
    
    trigger_callback(state, CRAWLER_EVENT_STOPPED, "Crawler stopped");
}

void crawler_get_status(CrawlerState* state, CrawlerStatus* status) {
    if (!state || !status) return;
    
    pthread_mutex_lock(&state->status_lock);
    status->running = state->running;
    status->pages_crawled = state->pages_crawled;
    status->pages_preprocessed = state->pages_preprocessed;
    status->pages_tokenized = state->pages_tokenized;
    status->pages_trained = state->pages_trained;
    strncpy(status->current_url, state->current_url, sizeof(status->current_url) - 1);
    status->current_url[sizeof(status->current_url) - 1] = '\0';
    strncpy(status->last_error, state->last_error, sizeof(status->last_error) - 1);
    status->last_error[sizeof(status->last_error) - 1] = '\0';
    pthread_mutex_unlock(&state->status_lock);
}

void crawler_set_callback(CrawlerState* state, CrawlerCallback callback, void* user_data) {
    if (!state) return;
    
    pthread_mutex_lock(&state->status_lock);
    state->callback = callback;
    state->callback_user_data = user_data;
    pthread_mutex_unlock(&state->status_lock);
}

void crawler_set_extraction_mode(CrawlerState* state, ExtractionMode mode) {
    if (!state) return;
    
    pthread_mutex_lock(&state->status_lock);
    state->extraction_mode = mode;
    pthread_mutex_unlock(&state->status_lock);
}

void crawler_set_model_name(CrawlerState* state, const char* model_name) {
    if (!state) return;
    
    pthread_mutex_lock(&state->status_lock);
    if (model_name && model_name[0] != '\0') {
        strncpy(state->model_name, model_name, sizeof(state->model_name) - 1);
        state->model_name[sizeof(state->model_name) - 1] = '\0';
    } else {
        state->model_name[0] = '\0';
    }
    pthread_mutex_unlock(&state->status_lock);
}

/**
 * Set URL manager for database integration
 */
void crawler_set_url_manager(CrawlerState* state, void* url_manager) {
    if (!state) return;
    
    pthread_mutex_lock(&state->status_lock);
    state->url_manager = url_manager;
    
    // Pass to internal crawler if already initialized
    if (state->crawler_internal) {
        crawler_internal_set_url_manager((CrawlerStateInternal*)state->crawler_internal, url_manager);
    }
    pthread_mutex_unlock(&state->status_lock);
}


void crawler_state_cleanup(CrawlerState* state) {
    if (!state) return;
    
    crawler_stop(state);
    
    if (state->crawler_internal) crawler_internal_cleanup(state->crawler_internal);
    if (state->preprocessor_internal) preprocessor_cleanup(state->preprocessor_internal);
    if (state->tokenizer_internal) tokenizer_cleanup(state->tokenizer_internal);
    
    // NEW: Cleanup training
    if (state->training_internal) {
        continuous_training_cleanup(state->training_internal);
        state->training_internal = NULL;
    }
    
    pthread_mutex_destroy(&state->status_lock);
    free(state);
}
