#ifndef TRAINING_STATUS_H
#define TRAINING_STATUS_H

#include <stdbool.h>

// Training phases
typedef enum {
    TRAINING_PHASE_IDLE = 0,
    TRAINING_PHASE_LOADING_DATA,
    TRAINING_PHASE_BUILDING_VOCAB,
    TRAINING_PHASE_TOKENIZING,
    TRAINING_PHASE_CREATING_BATCHES,
    TRAINING_PHASE_INITIALIZING,
    TRAINING_PHASE_TRAINING,
    TRAINING_PHASE_COMPLETE,
    TRAINING_PHASE_ERROR
} TrainingPhase;

// Training status structure
typedef struct {
    TrainingPhase phase;
    char status_message[256];
    float progress;  // 0.0 to 1.0
    bool is_active;
    
    // Phase-specific details
    int files_loaded;
    int total_files;
    int tokens_processed;
    int total_tokens;
    int batches_created;
    int total_batches;
} TrainingStatus;

// Get phase name
static inline const char* training_phase_name(TrainingPhase phase) {
    switch (phase) {
        case TRAINING_PHASE_IDLE: return "Idle";
        case TRAINING_PHASE_LOADING_DATA: return "Loading Data";
        case TRAINING_PHASE_BUILDING_VOCAB: return "Building Vocabulary";
        case TRAINING_PHASE_TOKENIZING: return "Tokenizing";
        case TRAINING_PHASE_CREATING_BATCHES: return "Creating Batches";
        case TRAINING_PHASE_INITIALIZING: return "Initializing";
        case TRAINING_PHASE_TRAINING: return "Training";
        case TRAINING_PHASE_COMPLETE: return "Complete";
        case TRAINING_PHASE_ERROR: return "Error";
        default: return "Unknown";
    }
}

#endif // TRAINING_STATUS_H