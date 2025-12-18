#ifndef CLLM_MESSAGE_QUEUE_H
#define CLLM_MESSAGE_QUEUE_H

#include "ai/cllm_sphere_message.h"
#include <stdatomic.h>
#include <stdbool.h>
#include <stdlib.h>

// Use algorithms library message queue
#include "../../algorithms/include/message_passing.h"

/**
 * CLLM Message Queue
 * 
 * NOTE: This now uses MessageQueue from algorithms/message_passing.h
 * 
 * The algorithms library provides:
 * - MessageQueue struct (lock-free implementation)
 * - message_queue_create(), message_queue_free()
 * - message_queue_enqueue(), message_queue_dequeue()
 * - Priority queue support
 * - Thread-safe operations
 * 
 * CLLM-specific extensions are defined below.
 */

// ============================================================================
// TYPE ALIASES FOR COMPATIBILITY
// ============================================================================

/**
 * CLLM Message Queue (alias to algorithms MessageQueue)
 * 
 * For backward compatibility, we provide an alias.
 * New code should use MessageQueue directly.
 */
typedef MessageQueue LockFreeMessageQueue;

/**
 * NOTE: SphereMessage is defined in cllm_sphere_message.h
 * It already contains all necessary fields for CLLM messaging.
 */

// ============================================================================
// CLLM-SPECIFIC FUNCTIONS
// ============================================================================

/**
 * NOTE: sphere_message_create() and sphere_message_free() are defined
 * in cllm_sphere_message.h - no need to redeclare them here.
 */

/**
 * Create CLLM message queue
 * 
 * Allocates and initializes a message queue.
 * 
 * @param max_queue_size Maximum queue size (0 = unlimited)
 * @param drop_on_full Drop messages when queue is full
 * @return Message queue, or NULL on error
 */
static inline LockFreeMessageQueue* cllm_message_queue_create(
    uint64_t max_queue_size,
    bool drop_on_full
) {
    (void)max_queue_size;  // Not used in current implementation
    (void)drop_on_full;    // Not used in current implementation
    
    LockFreeMessageQueue* queue = (LockFreeMessageQueue*)calloc(1, sizeof(LockFreeMessageQueue));
    if (queue) {
        message_queue_init(queue);
    }
    return queue;
}

/**
 * Free CLLM message queue
 * 
 * Frees a message queue.
 * 
 * @param queue Queue to free
 */
static inline void cllm_message_queue_free(LockFreeMessageQueue* queue) {
    if (queue) {
        // Drain the queue first
        while (!message_queue_is_empty(queue)) {
            Message* msg = message_queue_dequeue(queue);
            if (msg) free(msg);
        }
        free(queue);
    }
}

/**
 * Enqueue sphere message
 * 
 * @param queue Queue to enqueue to
 * @param msg Sphere message to enqueue
 * @return true on success, false on failure
 */
bool cllm_message_queue_enqueue(LockFreeMessageQueue* queue, SphereMessage* msg);

/**
 * Dequeue sphere message
 * 
 * @param queue Queue to dequeue from
 * @return Sphere message, or NULL if queue is empty
 */
SphereMessage* cllm_message_queue_dequeue(LockFreeMessageQueue* queue);

/**
 * Check if queue is empty
 * 
 * @param queue Queue to check
 * @return true if empty, false otherwise
 */
static inline bool cllm_message_queue_is_empty(const LockFreeMessageQueue* queue) {
    return message_queue_is_empty((MessageQueue*)queue);
}

/**
 * Get queue size
 * 
 * @param queue Queue to check
 * @return Number of messages in queue
 */
static inline uint64_t cllm_message_queue_size(const LockFreeMessageQueue* queue) {
    return message_queue_size((MessageQueue*)queue);
}

// ============================================================================
// CLLM MESSAGE HELPERS
// ============================================================================

/**
 * Create gradient ready message
 */
SphereMessage* sphere_message_gradient_ready(
    int source_sphere_id,
    int dest_sphere_id,
    const float* gradient,
    size_t gradient_size
);

/**
 * Create weights updated message
 */
SphereMessage* sphere_message_weights_updated(
    int source_sphere_id,
    int dest_sphere_id,
    const float* weights,
    size_t weights_size
);

/**
 * Create boundary crossing message
 */
SphereMessage* sphere_message_boundary_crossing(
    int source_sphere_id,
    uint64_t boundary_value
);

/**
 * Create twin prime hit message
 */
SphereMessage* sphere_message_twin_prime_hit(
    int source_sphere_id,
    uint64_t prime_value
);

/**
 * Create work request message
 */
SphereMessage* sphere_message_work_request(
    int source_sphere_id,
    int dest_sphere_id,
    uint64_t requested_items
);

#endif // CLLM_MESSAGE_QUEUE_H