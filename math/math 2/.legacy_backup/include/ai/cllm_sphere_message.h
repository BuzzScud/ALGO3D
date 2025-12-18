#ifndef CLLM_SPHERE_MESSAGE_H
#define CLLM_SPHERE_MESSAGE_H

#include <stdint.h>
#include <stdatomic.h>
#include <time.h>

// Use algorithms library types
#include "../../algorithms/include/message_passing.h"

/**
 * Sphere Message System
 * 
 * Lock-free message passing between spheres in the hierarchy.
 * Messages are used for coordination, work stealing, gradient synchronization,
 * and boundary notifications.
 * 
 * NOTE: This now uses MessageType and MessagePriority from algorithms/message_passing.h
 * CLLM-specific message types are defined as custom messages (>= MSG_TYPE_USER_DEFINED)
 */

// ============================================================================
// CLLM-SPECIFIC MESSAGE TYPES (Custom Messages)
// ============================================================================

/**
 * CLLM Message Type Extensions
 * 
 * Defines CLLM-specific message types as extensions to the base MessageType enum.
 * These start at MSG_TYPE_USER_DEFINED (1000) to avoid conflicts.
 */
typedef enum {
    // Gradient Synchronization Messages (1000-1099)
    MSG_CLLM_GRADIENT_READY = MSG_TYPE_USER_DEFINED,  // 1000
    MSG_CLLM_GRADIENT_ACCUMULATE,                      // 1001
    MSG_CLLM_GRADIENT_COMPLETE,                        // 1002
    
    // Weight Broadcasting Messages (1100-1199)
    MSG_CLLM_WEIGHTS_UPDATED = 1100,                   // Weights have been updated
    MSG_CLLM_WEIGHTS_REQUEST,                          // Request latest weights
    MSG_CLLM_WEIGHTS_BROADCAST,                        // Broadcasting weights to children
    
    // Boundary Notifications (1200-1299)
    MSG_CLLM_BOUNDARY_CROSSING = 1200,                 // Crossed 144000 boundary
    MSG_CLLM_TWIN_PRIME_HIT,                           // Hit twin prime (143999 or 144001)
    MSG_CLLM_BOUNDARY_REGION_ENTER,                    // Entered boundary region
    MSG_CLLM_BOUNDARY_REGION_EXIT,                     // Exited boundary region
    
    // Coordination Messages (1300-1399)
    MSG_CLLM_EPOCH_START = 1300,                       // Start new epoch
    MSG_CLLM_EPOCH_COMPLETE,                           // Epoch complete
    MSG_CLLM_BATCH_START,                              // Start new batch
    MSG_CLLM_BATCH_COMPLETE,                           // Batch complete
    
    // Hierarchy Management (1400-1499)
    MSG_CLLM_CHILD_SPAWN = 1400,                       // Spawn child sphere
    MSG_CLLM_CHILD_TERMINATE,                          // Terminate child sphere
    MSG_CLLM_PARENT_SYNC,                              // Synchronize with parent
    MSG_CLLM_SIBLING_DISCOVER,                         // Discover sibling sphere
    
    // Error Handling (1500-1599)
    MSG_CLLM_ERROR_REPORT = 1500,                      // Report error
    MSG_CLLM_ERROR_RECOVERY,                           // Error recovery
    
    // Statistics (1600-1699)
    MSG_CLLM_STATS_REQUEST = 1600,                     // Request statistics
    MSG_CLLM_STATS_REPORT,                             // Report statistics
    
    // Shutdown (1700-1799)
    MSG_CLLM_SHUTDOWN_REQUEST = 1700,                  // Request shutdown
    MSG_CLLM_SHUTDOWN_ACK,                             // Acknowledge shutdown
    
} CLLMMessageType;

// Note: Use MessagePriority from algorithms/message_passing.h directly
// No need to redefine it here

// ============================================================================
// MESSAGE PAYLOAD STRUCTURES
// ============================================================================

/**
 * Work Request Payload
 */
typedef struct {
    uint64_t requested_items;   // Number of work items requested
    int symmetry_group;         // Requesting symmetry group
    uint64_t current_load;      // Current workload of requester
} WorkRequestPayload;

/**
 * Work Offer Payload
 */
typedef struct {
    uint64_t offered_items;     // Number of work items offered
    uint64_t batch_start;       // Start index of offered batch
    uint64_t batch_end;         // End index of offered batch
    void* work_data;            // Pointer to work data (if applicable)
} WorkOfferPayload;

/**
 * Gradient Payload
 */
typedef struct {
    uint64_t gradient_count;    // Number of gradients
    void* gradient_buffer;      // Pointer to gradient buffer
    uint64_t buffer_size;       // Size of gradient buffer
    int symmetry_group;         // Symmetry group
} GradientPayload;

/**
 * Weight Payload
 */
typedef struct {
    uint64_t weight_count;      // Number of weights
    void* weight_buffer;        // Pointer to weight buffer
    uint64_t buffer_size;       // Size of weight buffer
    uint64_t version;           // Weight version number
} WeightPayload;

/**
 * Boundary Payload
 */
typedef struct {
    uint64_t prime;             // Prime that triggered boundary event
    int symmetry_group;         // Symmetry group
    double distance_to_144000;  // Distance to 144000
    int is_twin_prime;          // 1 if twin prime, 0 otherwise
} BoundaryPayload;

/**
 * Epoch Payload
 */
typedef struct {
    uint64_t epoch_number;      // Epoch number
    uint64_t total_batches;     // Total batches in epoch
    double learning_rate;       // Learning rate for epoch
} EpochPayload;

/**
 * Error Payload
 */
typedef struct {
    int error_code;             // Error code
    char error_message[256];    // Error message
    uint64_t timestamp;         // Error timestamp
    int severity;               // Error severity (0-3)
} ErrorPayload;

/**
 * Statistics Payload
 */
typedef struct {
    uint64_t primes_processed;  // Primes processed
    uint64_t batches_completed; // Batches completed
    double avg_processing_time; // Average processing time
    double cache_hit_rate;      // Cache hit rate
    double utilization;         // Utilization rate
} StatisticsPayload;

/**
 * Message Payload Union
 * 
 * Contains all possible payload types.
 */
typedef union {
    WorkRequestPayload work_request;
    WorkOfferPayload work_offer;
    GradientPayload gradient;
    WeightPayload weight;
    BoundaryPayload boundary;
    EpochPayload epoch;
    ErrorPayload error;
    StatisticsPayload statistics;
    uint64_t generic_data[8];   // Generic 64-byte payload
} MessagePayload;

// ============================================================================
// SPHERE MESSAGE STRUCTURE
// ============================================================================

/**
 * Sphere Message
 * 
 * Complete message structure for inter-sphere communication.
 * Aligned to cache line (64 bytes) to prevent false sharing.
 */
typedef struct SphereMessage {
    // Message metadata (cache line 1)
    MessageType type;           // Message type
    MessagePriority priority;   // Message priority
    uint64_t message_id;        // Unique message ID
    uint64_t timestamp_ns;      // Timestamp (nanoseconds)
    
    // Sender/Receiver information
    int sender_id;              // Sender sphere ID
    int receiver_id;            // Receiver sphere ID (-1 for broadcast)
    int sender_symmetry_group;  // Sender symmetry group
    int receiver_symmetry_group;// Receiver symmetry group
    
    // Message payload
    MessagePayload payload;     // Message payload
    
    // Message state
    atomic_int processed;       // 1 if processed, 0 otherwise
    atomic_int acknowledged;    // 1 if acknowledged, 0 otherwise
    
    // Linked list for queue (cache line aligned)
    struct SphereMessage* next; // Next message in queue
    
    // Padding to cache line boundary (64 bytes)
    char padding[64 - ((sizeof(MessageType) + 
                        sizeof(MessagePriority) + 
                        sizeof(uint64_t) * 2 +
                        sizeof(int) * 4 +
                        sizeof(MessagePayload) +
                        sizeof(atomic_int) * 2 +
                        sizeof(void*)) % 64)];
    
} __attribute__((aligned(64))) SphereMessage;

// ============================================================================
// MESSAGE CREATION AND DESTRUCTION
// ============================================================================

/**
 * Create a new sphere message
 * 
 * @param type Message type
 * @param priority Message priority
 * @param sender_id Sender sphere ID
 * @param receiver_id Receiver sphere ID (-1 for broadcast)
 * @return Pointer to new message, or NULL on failure
 */
SphereMessage* sphere_message_create(MessageType type,
                                     MessagePriority priority,
                                     int sender_id,
                                     int receiver_id);

/**
 * Free a sphere message
 * 
 * @param message Message to free
 */
void sphere_message_free(SphereMessage* message);

/**
 * Clone a sphere message
 * 
 * @param message Message to clone
 * @return Pointer to cloned message, or NULL on failure
 */
SphereMessage* sphere_message_clone(const SphereMessage* message);

// ============================================================================
// MESSAGE PAYLOAD SETTERS
// ============================================================================

/**
 * Set work request payload
 */
void sphere_message_set_work_request(SphereMessage* message,
                                     uint64_t requested_items,
                                     int symmetry_group,
                                     uint64_t current_load);

/**
 * Set work offer payload
 */
void sphere_message_set_work_offer(SphereMessage* message,
                                   uint64_t offered_items,
                                   uint64_t batch_start,
                                   uint64_t batch_end,
                                   void* work_data);

/**
 * Set gradient payload
 */
void sphere_message_set_gradient(SphereMessage* message,
                                 uint64_t gradient_count,
                                 void* gradient_buffer,
                                 uint64_t buffer_size,
                                 int symmetry_group);

/**
 * Set weight payload
 */
void sphere_message_set_weight(SphereMessage* message,
                               uint64_t weight_count,
                               void* weight_buffer,
                               uint64_t buffer_size,
                               uint64_t version);

/**
 * Set boundary payload
 */
void sphere_message_set_boundary(SphereMessage* message,
                                 uint64_t prime,
                                 int symmetry_group,
                                 double distance_to_144000,
                                 int is_twin_prime);

/**
 * Set epoch payload
 */
void sphere_message_set_epoch(SphereMessage* message,
                              uint64_t epoch_number,
                              uint64_t total_batches,
                              double learning_rate);

/**
 * Set error payload
 */
void sphere_message_set_error(SphereMessage* message,
                              int error_code,
                              const char* error_message,
                              int severity);

/**
 * Set statistics payload
 */
void sphere_message_set_statistics(SphereMessage* message,
                                   uint64_t primes_processed,
                                   uint64_t batches_completed,
                                   double avg_processing_time,
                                   double cache_hit_rate,
                                   double utilization);

// ============================================================================
// MESSAGE UTILITIES
// ============================================================================

/**
 * Get message type name
 * 
 * @param type Message type
 * @return String representation of message type
 */
const char* sphere_message_type_name(MessageType type);

/**
 * Get message priority name
 * 
 * @param priority Message priority
 * @return String representation of message priority
 */
const char* sphere_message_priority_name(MessagePriority priority);

/**
 * Mark message as processed
 * 
 * @param message Message to mark
 */
void sphere_message_mark_processed(SphereMessage* message);

/**
 * Mark message as acknowledged
 * 
 * @param message Message to mark
 */
void sphere_message_mark_acknowledged(SphereMessage* message);

/**
 * Check if message is processed
 * 
 * @param message Message to check
 * @return 1 if processed, 0 otherwise
 */
int sphere_message_is_processed(const SphereMessage* message);

/**
 * Check if message is acknowledged
 * 
 * @param message Message to check
 * @return 1 if acknowledged, 0 otherwise
 */
int sphere_message_is_acknowledged(const SphereMessage* message);

/**
 * Print message information
 * 
 * @param message Message to print
 */
void sphere_message_print(const SphereMessage* message);

/**
 * Validate message
 * 
 * @param message Message to validate
 * @return 1 if valid, 0 otherwise
 */
int sphere_message_validate(const SphereMessage* message);

#endif // CLLM_SPHERE_MESSAGE_H