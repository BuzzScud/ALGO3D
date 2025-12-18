#ifndef CLLM_GLOBAL_PROGRESS_H
#define CLLM_GLOBAL_PROGRESS_H

#include <stddef.h>
#include <time.h>
#include <stdint.h>
#include <pthread.h>

// Phase definitions
typedef enum {
    CLLM_PHASE_INIT = 0,
    CLLM_PHASE_LOADING_DATA = 1,
    CLLM_PHASE_BUILDING_VOCAB = 2,
    CLLM_PHASE_TOKENIZING = 3,
    CLLM_PHASE_TRAINING = 4,
    CLLM_PHASE_SAVING = 5,
    CLLM_PHASE_COMPLETE = 6
} CLLMPhase;

// Phase weights (must sum to 100)
#define PHASE_WEIGHT_LOADING 5
#define PHASE_WEIGHT_VOCAB 20
#define PHASE_WEIGHT_TOKENIZING 25
#define PHASE_WEIGHT_TRAINING 45
#define PHASE_WEIGHT_SAVING 5

// Global progress tracker structure
typedef struct {
    // Current phase
    CLLMPhase current_phase;
    const char* phase_name;
    
    // Phase progress
    size_t phase_current;
    size_t phase_total;
    
    // Overall progress (0-100)
    double overall_progress;
    
    // Timing
    time_t program_start_time;
    time_t phase_start_time;
    time_t last_update_time;
    
    // Display control
    int update_interval_ms;
    int64_t last_update_ms;
    
    // Thread safety
    pthread_mutex_t lock;
    
    // Sub-phase information (optional)
    const char* subphase_name;
    size_t subphase_current;
    size_t subphase_total;
    
    // Training-specific
    int current_epoch;
    int total_epochs;
    double current_loss;
    
} CLLMGlobalProgress;

// Initialize global progress system
void cllm_global_progress_init(void);

// Start a new phase
void cllm_global_progress_start_phase(CLLMPhase phase, const char* phase_name, size_t total);

// Update phase progress
void cllm_global_progress_update(size_t current);

// Update with sub-phase information
void cllm_global_progress_update_subphase(const char* subphase_name, 
                                          size_t subphase_current, 
                                          size_t subphase_total);

// Update training-specific information
void cllm_global_progress_update_training(int epoch, int total_epochs, double loss);

// Complete current phase
void cllm_global_progress_complete_phase(void);

// Complete entire program
void cllm_global_progress_complete(void);

// Force display update (thread-safe)
void cllm_global_progress_display(void);

// Cleanup
void cllm_global_progress_cleanup(void);

// Get global progress instance (for direct access if needed)
CLLMGlobalProgress* cllm_global_progress_get(void);

#endif // CLLM_GLOBAL_PROGRESS_H