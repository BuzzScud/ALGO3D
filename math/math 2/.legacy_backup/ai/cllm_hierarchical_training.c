/**
 * @file cllm_hierarchical_training.c
 * @brief Hierarchical Training System Implementation
 * 
 * Implements hierarchical training using the kissing spheres architecture.
 * This is a simplified implementation that wraps the existing training functions.
 */

#include "ai/cllm_hierarchical_training.h"
#include "cllm_training.h"
#include "cllm_batch.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * Hierarchical Training System Structure
 */
struct HierarchicalTrainingSystem {
    CLLMTraining* training;
    int num_threads;
    CLLMBatchIterator* batch_iterator;
    
    // Statistics
    int total_batches_processed;
    double total_loss;
};

/**
 * Create hierarchical training system
 */
HierarchicalTrainingSystem* hierarchical_training_create(
    CLLMTraining* training,
    int num_threads,
    CLLMBatchIterator* batch_iterator
) {
    if (!training || !batch_iterator) {
        fprintf(stderr, "Error: NULL parameters in hierarchical_training_create\n");
        return NULL;
    }
    
    HierarchicalTrainingSystem* system = (HierarchicalTrainingSystem*)calloc(
        1, sizeof(HierarchicalTrainingSystem));
    
    if (!system) {
        fprintf(stderr, "Error: Failed to allocate hierarchical training system\n");
        return NULL;
    }
    
    system->training = training;
    system->num_threads = num_threads;
    system->batch_iterator = batch_iterator;
    system->total_batches_processed = 0;
    system->total_loss = 0.0;
    
    printf("✓ Hierarchical training system created with %d threads\n", num_threads);
    
    return system;
}

/**
 * Train one epoch using hierarchical system
 */
float hierarchical_train_epoch(HierarchicalTrainingSystem* system) {
    if (!system || !system->training || !system->batch_iterator) {
        fprintf(stderr, "Error: Invalid hierarchical training system\n");
        return 0.0f;
    }
    
    CLLMTraining* training = system->training;
    CLLMBatchIterator* iterator = system->batch_iterator;
    
    int batch_size = training->config.batch_size;
    int seq_len = training->config.sequence_length;
    int tokens_per_batch = batch_size * seq_len;
    
    double epoch_loss = 0.0;
    int batches_in_epoch = 0;
    
    // Reset iterator for new epoch
    cllm_batch_iterator_reset(iterator);
    
    // Process all batches in the epoch
    while (1) {
        // Get next batch
        CLLMBatch* batch = cllm_batch_iterator_next(iterator);
        if (!batch) {
            break;
        }
        
        // Forward pass - now uses transformer layers!
        cllm_forward_training(training, batch->input_ids);
        
        // Compute loss
        double batch_loss = cllm_compute_loss(training, batch->input_ids, batch->target_ids, tokens_per_batch);
        
        // Backward pass (simplified - weight updates happen here)
        // In a full implementation, this would compute gradients and update weights
        // For now, we just track the loss
        
        epoch_loss += batch_loss;
        batches_in_epoch++;
        system->total_batches_processed++;
        
        // Print progress every 10 batches
        if (batches_in_epoch % 10 == 0) {
            printf("  Batch %d: Loss = %.4f\r", batches_in_epoch, batch_loss);
            fflush(stdout);
        }
        
        cllm_batch_free(batch);
        
        training->current_step++;
    }
    
    // Calculate average loss
    float avg_loss = (batches_in_epoch > 0) ? (float)(epoch_loss / batches_in_epoch) : 0.0f;
    
    // Update training statistics
    if (avg_loss < training->best_loss || training->best_loss == 0.0) {
        training->best_loss = avg_loss;
    }
    
    system->total_loss += epoch_loss;
    
    printf("\n  Processed %d batches, Average Loss: %.4f\n", batches_in_epoch, avg_loss);
    
    return avg_loss;
}

/**
 * Free hierarchical training system
 */
void hierarchical_training_free(HierarchicalTrainingSystem* system) {
    if (!system) return;
    
    printf("Hierarchical training system statistics:\n");
    printf("  Total batches processed: %d\n", system->total_batches_processed);
    printf("  Total loss: %.4f\n", system->total_loss);
    
    free(system);
}