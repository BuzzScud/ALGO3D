#ifndef CLLM_HIERARCHICAL_TRAINING_H
#define CLLM_HIERARCHICAL_TRAINING_H

#include "../cllm_training.h"
#include "../cllm_batch.h"

/**
 * CLLM Hierarchical Training System
 * 
 * Implements the correct recursive kissing spheres hierarchy for training.
 * 
 * Architecture:
 * - Level 0: 1 root (Node Zero) - control only, NEVER processes batches
 * - Level 1: Up to 12 spheres - can be control or workers
 * - Level 2+: Recursive structure - each sphere can have up to 12 children
 * 
 * For 64 cores:
 * - Level 0: 1 root control
 * - Level 1: 12 control spheres
 * - Level 2: 51 worker spheres
 */

// Forward declaration
typedef struct HierarchicalTrainingSystem HierarchicalTrainingSystem;

/**
 * Create hierarchical training system
 * 
 * @param training Training configuration
 * @param num_threads Total number of threads (including control)
 * @param batch_iterator Batch iterator for training data
 * @return New hierarchical training system, or NULL on error
 */
HierarchicalTrainingSystem* hierarchical_training_create(
    CLLMTraining* training,
    int num_threads,
    CLLMBatchIterator* batch_iterator
);

/**
 * Train one epoch using hierarchical system
 * 
 * @param system Hierarchical training system
 * @return Average loss for the epoch
 */
float hierarchical_train_epoch(HierarchicalTrainingSystem* system);

/**
 * Free hierarchical training system
 * 
 * @param system Hierarchical training system to free
 */
void hierarchical_training_free(HierarchicalTrainingSystem* system);

#endif // CLLM_HIERARCHICAL_TRAINING_H