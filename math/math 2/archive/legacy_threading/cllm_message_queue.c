/**
 * @file cllm_message_queue.c
 * @brief CLLM Message Queue Implementation - Wrapper for Algorithm Library
 * 
 * This file now uses the algorithm library's MessageSystem implementation.
 * Most functionality is delegated to the algorithm library.
 * 
 * Note: The algorithm library provides all the lock-free queue operations.
 * This file only provides compatibility wrappers if needed.
 */

#include "ai/cllm_message_queue.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ============================================================================
// COMPATIBILITY WRAPPERS
// ============================================================================

/**
 * Create message queue (compatibility wrapper)
 */
LockFreeMessageQueue* message_queue_create(uint64_t max_queue_size, bool drop_on_full) {
    return cllm_message_queue_create(max_queue_size, drop_on_full);
}

/**
 * Free message queue (compatibility wrapper)
 */
void message_queue_free(LockFreeMessageQueue* queue) {
    cllm_message_queue_free(queue);
}

/**
 * Print message queue statistics (compatibility wrapper)
 */
void message_queue_print_statistics(const LockFreeMessageQueue* queue) {
    if (!queue) {
        printf("Queue: NULL\n");
        return;
    }
    
    printf("Queue Statistics:\n");
    printf("  Current size: %lu\n", (unsigned long)queue->current_size);
    printf("  Peak size: %lu\n", (unsigned long)queue->peak_size);
    printf("  Enqueue count: %lu\n", (unsigned long)queue->enqueue_count);
    printf("  Dequeue count: %lu\n", (unsigned long)queue->dequeue_count);
}