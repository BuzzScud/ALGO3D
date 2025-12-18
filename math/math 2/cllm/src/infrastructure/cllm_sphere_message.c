#include "ai/cllm_sphere_message.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

// Global message ID counter (atomic)
static atomic_uint_fast64_t global_message_id = ATOMIC_VAR_INIT(0);

// ============================================================================
// MESSAGE CREATION AND DESTRUCTION
// ============================================================================

SphereMessage* sphere_message_create(MessageType type,
                                     MessagePriority priority,
                                     int sender_id,
                                     int receiver_id) {
    // Allocate aligned memory for cache line alignment
    SphereMessage* message = aligned_alloc(64, sizeof(SphereMessage));
    if (!message) {
        return NULL;
    }
    
    // Zero out the message
    memset(message, 0, sizeof(SphereMessage));
    
    // Set metadata
    message->type = type;
    message->priority = priority;
    message->message_id = atomic_fetch_add(&global_message_id, 1);
    
    // Get timestamp
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    message->timestamp_ns = (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
    
    // Set sender/receiver
    message->sender_id = sender_id;
    message->receiver_id = receiver_id;
    message->sender_symmetry_group = -1;
    message->receiver_symmetry_group = -1;
    
    // Initialize atomic state
    atomic_init(&message->processed, 0);
    atomic_init(&message->acknowledged, 0);
    
    // Initialize linked list
    message->next = NULL;
    
    return message;
}

void sphere_message_free(SphereMessage* message) {
    if (!message) return;
    
    // Note: We don't free payload pointers here as they may be shared
    // The caller is responsible for managing payload memory
    
    free(message);
}

SphereMessage* sphere_message_clone(const SphereMessage* message) {
    if (!message) return NULL;
    
    // Allocate new message
    SphereMessage* clone = aligned_alloc(64, sizeof(SphereMessage));
    if (!clone) {
        return NULL;
    }
    
    // Copy entire message
    memcpy(clone, message, sizeof(SphereMessage));
    
    // Reset atomic state
    atomic_init(&clone->processed, 0);
    atomic_init(&clone->acknowledged, 0);
    
    // Reset linked list
    clone->next = NULL;
    
    // Assign new message ID
    clone->message_id = atomic_fetch_add(&global_message_id, 1);
    
    // Update timestamp
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    clone->timestamp_ns = (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
    
    return clone;
}

// ============================================================================
// MESSAGE PAYLOAD SETTERS
// ============================================================================

void sphere_message_set_work_request(SphereMessage* message,
                                     uint64_t requested_items,
                                     int symmetry_group,
                                     uint64_t current_load) {
    if (!message) return;
    
    message->payload.work_request.requested_items = requested_items;
    message->payload.work_request.symmetry_group = symmetry_group;
    message->payload.work_request.current_load = current_load;
    message->sender_symmetry_group = symmetry_group;
}

void sphere_message_set_work_offer(SphereMessage* message,
                                   uint64_t offered_items,
                                   uint64_t batch_start,
                                   uint64_t batch_end,
                                   void* work_data) {
    if (!message) return;
    
    message->payload.work_offer.offered_items = offered_items;
    message->payload.work_offer.batch_start = batch_start;
    message->payload.work_offer.batch_end = batch_end;
    message->payload.work_offer.work_data = work_data;
}

void sphere_message_set_gradient(SphereMessage* message,
                                 uint64_t gradient_count,
                                 void* gradient_buffer,
                                 uint64_t buffer_size,
                                 int symmetry_group) {
    if (!message) return;
    
    message->payload.gradient.gradient_count = gradient_count;
    message->payload.gradient.gradient_buffer = gradient_buffer;
    message->payload.gradient.buffer_size = buffer_size;
    message->payload.gradient.symmetry_group = symmetry_group;
    message->sender_symmetry_group = symmetry_group;
}

void sphere_message_set_weight(SphereMessage* message,
                               uint64_t weight_count,
                               void* weight_buffer,
                               uint64_t buffer_size,
                               uint64_t version) {
    if (!message) return;
    
    message->payload.weight.weight_count = weight_count;
    message->payload.weight.weight_buffer = weight_buffer;
    message->payload.weight.buffer_size = buffer_size;
    message->payload.weight.version = version;
}

void sphere_message_set_boundary(SphereMessage* message,
                                 uint64_t prime,
                                 int symmetry_group,
                                 double distance_to_144000,
                                 int is_twin_prime) {
    if (!message) return;
    
    message->payload.boundary.prime = prime;
    message->payload.boundary.symmetry_group = symmetry_group;
    message->payload.boundary.distance_to_144000 = distance_to_144000;
    message->payload.boundary.is_twin_prime = is_twin_prime;
    message->sender_symmetry_group = symmetry_group;
}

void sphere_message_set_epoch(SphereMessage* message,
                              uint64_t epoch_number,
                              uint64_t total_batches,
                              double learning_rate) {
    if (!message) return;
    
    message->payload.epoch.epoch_number = epoch_number;
    message->payload.epoch.total_batches = total_batches;
    message->payload.epoch.learning_rate = learning_rate;
}

void sphere_message_set_error(SphereMessage* message,
                              int error_code,
                              const char* error_message,
                              int severity) {
    if (!message) return;
    
    message->payload.error.error_code = error_code;
    message->payload.error.severity = severity;
    
    // Get timestamp
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    message->payload.error.timestamp = (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
    
    // Copy error message (truncate if necessary)
    if (error_message) {
        strncpy(message->payload.error.error_message, error_message, 255);
        message->payload.error.error_message[255] = '\0';
    } else {
        message->payload.error.error_message[0] = '\0';
    }
}

void sphere_message_set_statistics(SphereMessage* message,
                                   uint64_t primes_processed,
                                   uint64_t batches_completed,
                                   double avg_processing_time,
                                   double cache_hit_rate,
                                   double utilization) {
    if (!message) return;
    
    message->payload.statistics.primes_processed = primes_processed;
    message->payload.statistics.batches_completed = batches_completed;
    message->payload.statistics.avg_processing_time = avg_processing_time;
    message->payload.statistics.cache_hit_rate = cache_hit_rate;
    message->payload.statistics.utilization = utilization;
}

// ============================================================================
// MESSAGE UTILITIES
// ============================================================================

const char* sphere_message_type_name(MessageType type) {
    // Suppress warnings for CLLM-specific message types (extensions of base MessageType)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wswitch"
    switch (type) {
        case MSG_TYPE_WORK_REQUEST: return "WORK_REQUEST";
        case MSG_TYPE_WORK_ASSIGN: return "WORK_ASSIGN";
        case MSG_TYPE_WORK_COMPLETE: return "WORK_COMPLETE";
        case MSG_TYPE_WORK_REJECT: return "WORK_REJECT";
        case MSG_CLLM_GRADIENT_READY: return "GRADIENT_READY";
        case MSG_CLLM_GRADIENT_ACCUMULATE: return "GRADIENT_ACCUMULATE";
        case MSG_CLLM_GRADIENT_COMPLETE: return "GRADIENT_COMPLETE";
        case MSG_CLLM_WEIGHTS_UPDATED: return "WEIGHTS_UPDATED";
        case MSG_CLLM_WEIGHTS_REQUEST: return "WEIGHTS_REQUEST";
        case MSG_CLLM_WEIGHTS_BROADCAST: return "WEIGHTS_BROADCAST";
        case MSG_CLLM_BOUNDARY_CROSSING: return "BOUNDARY_CROSSING";
        case MSG_CLLM_TWIN_PRIME_HIT: return "TWIN_PRIME_HIT";
        case MSG_CLLM_BOUNDARY_REGION_ENTER: return "BOUNDARY_REGION_ENTER";
        case MSG_CLLM_BOUNDARY_REGION_EXIT: return "BOUNDARY_REGION_EXIT";
        case MSG_CLLM_EPOCH_START: return "EPOCH_START";
        case MSG_CLLM_EPOCH_COMPLETE: return "EPOCH_COMPLETE";
        case MSG_CLLM_BATCH_START: return "BATCH_START";
        case MSG_CLLM_BATCH_COMPLETE: return "BATCH_COMPLETE";
        case MSG_CLLM_CHILD_SPAWN: return "CHILD_SPAWN";
        case MSG_CLLM_CHILD_TERMINATE: return "CHILD_TERMINATE";
        case MSG_CLLM_PARENT_SYNC: return "PARENT_SYNC";
        case MSG_CLLM_SIBLING_DISCOVER: return "SIBLING_DISCOVER";
        case MSG_CLLM_ERROR_REPORT: return "ERROR_REPORT";
        case MSG_CLLM_ERROR_RECOVERY: return "ERROR_RECOVERY";
        case MSG_CLLM_STATS_REQUEST: return "STATS_REQUEST";
        case MSG_CLLM_STATS_REPORT: return "STATS_REPORT";
        case MSG_CLLM_SHUTDOWN_REQUEST: return "SHUTDOWN_REQUEST";
        case MSG_CLLM_SHUTDOWN_ACK: return "SHUTDOWN_ACK";
        default: return "UNKNOWN";
    }
    #pragma GCC diagnostic pop
}

const char* sphere_message_priority_name(MessagePriority priority) {
    switch (priority) {
        case MSG_PRIORITY_LOW: return "LOW";
        case MSG_PRIORITY_NORMAL: return "NORMAL";
        case MSG_PRIORITY_HIGH: return "HIGH";
        case MSG_PRIORITY_CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

void sphere_message_mark_processed(SphereMessage* message) {
    if (!message) return;
    atomic_store(&message->processed, 1);
}

void sphere_message_mark_acknowledged(SphereMessage* message) {
    if (!message) return;
    atomic_store(&message->acknowledged, 1);
}

int sphere_message_is_processed(const SphereMessage* message) {
    if (!message) return 0;
    return atomic_load(&message->processed);
}

int sphere_message_is_acknowledged(const SphereMessage* message) {
    if (!message) return 0;
    return atomic_load(&message->acknowledged);
}

void sphere_message_print(const SphereMessage* message) {
    if (!message) {
        printf("NULL message\n");
        return;
    }
    
    printf("=== Sphere Message ===\n");
    printf("ID: %lu\n", (unsigned long)message->message_id);
    printf("Type: %s\n", sphere_message_type_name(message->type));
    printf("Priority: %s\n", sphere_message_priority_name(message->priority));
    printf("Sender: %d (group %d)\n", message->sender_id, message->sender_symmetry_group);
    printf("Receiver: %d (group %d)\n", message->receiver_id, message->receiver_symmetry_group);
    printf("Timestamp: %lu ns\n", (unsigned long)message->timestamp_ns);
    printf("Processed: %s\n", sphere_message_is_processed(message) ? "Yes" : "No");
    printf("Acknowledged: %s\n", sphere_message_is_acknowledged(message) ? "Yes" : "No");
    
    // Print payload based on type
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wswitch"
    switch (message->type) {
        case MSG_TYPE_WORK_REQUEST:
            printf("Payload: requested_items=%lu, symmetry_group=%d, current_load=%lu\n",
                   (unsigned long)message->payload.work_request.requested_items,
                   message->payload.work_request.symmetry_group,
                   (unsigned long)message->payload.work_request.current_load);
            break;
        case MSG_TYPE_WORK_ASSIGN:
            printf("Payload: offered_items=%lu, batch_start=%lu, batch_end=%lu\n",
                   (unsigned long)message->payload.work_offer.offered_items,
                   (unsigned long)message->payload.work_offer.batch_start,
                   (unsigned long)message->payload.work_offer.batch_end);
            break;
        case MSG_CLLM_BOUNDARY_CROSSING:
        case MSG_CLLM_TWIN_PRIME_HIT:
            printf("Payload: prime=%lu, symmetry_group=%d, distance=%.2f, is_twin=%d\n",
                   (unsigned long)message->payload.boundary.prime,
                   message->payload.boundary.symmetry_group,
                   message->payload.boundary.distance_to_144000,
                   message->payload.boundary.is_twin_prime);
            break;
        case MSG_CLLM_EPOCH_START:
        case MSG_CLLM_EPOCH_COMPLETE:
            printf("Payload: epoch=%lu, total_batches=%lu, learning_rate=%.6f\n",
                   (unsigned long)message->payload.epoch.epoch_number,
                   (unsigned long)message->payload.epoch.total_batches,
                   message->payload.epoch.learning_rate);
            break;
        case MSG_CLLM_ERROR_REPORT:
            printf("Payload: error_code=%d, severity=%d, message='%s'\n",
                   message->payload.error.error_code,
                   message->payload.error.severity,
                   message->payload.error.error_message);
            break;
        case MSG_CLLM_STATS_REPORT:
            printf("Payload: primes=%lu, batches=%lu, avg_time=%.2f, cache_hit=%.2f%%, util=%.2f%%\n",
                   (unsigned long)message->payload.statistics.primes_processed,
                   (unsigned long)message->payload.statistics.batches_completed,
                   message->payload.statistics.avg_processing_time,
                   message->payload.statistics.cache_hit_rate * 100.0,
                   message->payload.statistics.utilization * 100.0);
            break;
        default:
            printf("Payload: (type-specific)\n");
            break;
    }
    #pragma GCC diagnostic pop
    
    printf("=====================\n");
}

int sphere_message_validate(const SphereMessage* message) {
    if (!message) {
        return 0;
    }
    
    // Validate sender and receiver IDs
    if (message->sender_id < -1) {
        return 0;
    }
    
    if (message->receiver_id < -1) {
        return 0;
    }
    
    // Validate symmetry groups (if set)
    if (message->sender_symmetry_group != -1 && 
        (message->sender_symmetry_group < 0 || message->sender_symmetry_group >= 12)) {
        return 0;
    }
    
    if (message->receiver_symmetry_group != -1 && 
        (message->receiver_symmetry_group < 0 || message->receiver_symmetry_group >= 12)) {
        return 0;
    }
    
    // Validate priority
    if (message->priority < MSG_PRIORITY_LOW || message->priority > MSG_PRIORITY_CRITICAL) {
        return 0;
    }
    
    // Validate timestamp (should not be zero)
    if (message->timestamp_ns == 0) {
        return 0;
    }
    
    return 1;
}