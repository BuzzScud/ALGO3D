/**
 * @file message_passing.h
 * @brief Generic Lock-Free Message Passing System
 * 
 * A universal message passing system with:
 * - Extensible message types (user-defined start at 1000)
 * - 4 priority levels (CRITICAL, HIGH, NORMAL, LOW)
 * - Lock-free queue operations using CAS
 * - ABA prevention with tagged pointers
 * - Memory pool for efficient allocation
 * - Batch operations for performance
 * 
 * Design Philosophy:
 * - Lock-free for maximum concurrency
 * - Zero-copy where possible
 * - Predictable latency
 * - Bounded memory usage
 * - Type-safe message handling
 */

#ifndef MESSAGE_PASSING_H
#define MESSAGE_PASSING_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// MESSAGE TYPES
// ============================================================================

/**
 * Standard message types (0-999 reserved)
 * User-defined types should start at 1000
 */
typedef enum {
    // Control messages (0-99)
    MSG_TYPE_INIT = 0,
    MSG_TYPE_SHUTDOWN = 1,
    MSG_TYPE_PAUSE = 2,
    MSG_TYPE_RESUME = 3,
    MSG_TYPE_RESET = 4,
    
    // Work distribution (100-199)
    MSG_TYPE_WORK_REQUEST = 100,
    MSG_TYPE_WORK_ASSIGN = 101,
    MSG_TYPE_WORK_COMPLETE = 102,
    MSG_TYPE_WORK_STEAL = 103,
    MSG_TYPE_WORK_REJECT = 104,
    
    // Synchronization (200-299)
    MSG_TYPE_BARRIER_WAIT = 200,
    MSG_TYPE_BARRIER_RELEASE = 201,
    MSG_TYPE_LOCK_REQUEST = 202,
    MSG_TYPE_LOCK_GRANT = 203,
    MSG_TYPE_LOCK_RELEASE = 204,
    
    // Data transfer (300-399)
    MSG_TYPE_DATA_SEND = 300,
    MSG_TYPE_DATA_RECEIVE = 301,
    MSG_TYPE_DATA_ACK = 302,
    MSG_TYPE_DATA_NACK = 303,
    
    // State management (400-499)
    MSG_TYPE_STATE_CHANGE = 400,
    MSG_TYPE_STATE_QUERY = 401,
    MSG_TYPE_STATE_RESPONSE = 402,
    
    // Error handling (500-599)
    MSG_TYPE_ERROR = 500,
    MSG_TYPE_WARNING = 501,
    MSG_TYPE_EXCEPTION = 502,
    
    // Memory management (600-699)
    MSG_TYPE_ALLOC_REQUEST = 600,
    MSG_TYPE_ALLOC_GRANT = 601,
    MSG_TYPE_FREE_REQUEST = 602,
    MSG_TYPE_FREE_COMPLETE = 603,
    
    // Custom messages start here
    MSG_TYPE_USER_DEFINED = 1000
} MessageType;

/**
 * Message priority levels
 */
typedef enum {
    MSG_PRIORITY_CRITICAL = 0,  // Highest priority (system critical)
    MSG_PRIORITY_HIGH = 1,      // High priority (time-sensitive)
    MSG_PRIORITY_NORMAL = 2,    // Normal priority (default)
    MSG_PRIORITY_LOW = 3        // Low priority (background)
} MessagePriority;

/**
 * Message flags
 */
typedef enum {
    MSG_FLAG_NONE = 0,
    MSG_FLAG_URGENT = (1 << 0),      // Process immediately
    MSG_FLAG_BROADCAST = (1 << 1),   // Send to all threads
    MSG_FLAG_REPLY_NEEDED = (1 << 2), // Sender expects reply
    MSG_FLAG_NO_COPY = (1 << 3),     // Zero-copy message
    MSG_FLAG_PERSISTENT = (1 << 4)   // Don't free after delivery
} MessageFlags;

// ============================================================================
// MESSAGE STRUCTURE
// ============================================================================

/**
 * Generic message structure
 */
typedef struct Message {
    // Header (cache-line aligned)
    uint64_t id;                    // Unique message ID
    MessageType type;               // Message type
    MessagePriority priority;       // Priority level
    uint32_t flags;                 // Message flags
    
    // Routing
    uint32_t sender_id;             // Sender thread ID
    uint32_t receiver_id;           // Receiver thread ID (or BROADCAST)
    uint32_t reply_to;              // Message ID to reply to
    
    // Payload
    void* data;                     // Message data
    size_t data_size;               // Data size in bytes
    void (*free_fn)(void*);         // Custom free function (optional)
    
    // Metadata
    uint64_t timestamp;             // Creation timestamp
    uint64_t deadline;              // Delivery deadline (0 = no deadline)
    
    // Internal (for queue management)
    struct Message* next;           // Next message in queue
    uint64_t version;               // ABA prevention tag
} Message;

// ============================================================================
// MESSAGE QUEUE
// ============================================================================

/**
 * Lock-free message queue node
 */
typedef struct MessageQueueNode {
    Message* message;
    struct MessageQueueNode* next;
    uint64_t version;  // ABA prevention
} MessageQueueNode;

/**
 * Lock-free message queue
 */
typedef struct MessageQueue {
    MessageQueueNode* head;
    MessageQueueNode* tail;
    uint64_t version_counter;
    
    // Statistics
    uint64_t enqueue_count;
    uint64_t dequeue_count;
    uint64_t current_size;
    uint64_t peak_size;
} MessageQueue;

/**
 * Priority queue (4 queues, one per priority)
 */
typedef struct PriorityMessageQueue {
    MessageQueue queues[4];  // One per priority level
    uint64_t total_messages;
} PriorityMessageQueue;

// ============================================================================
// MESSAGE POOL
// ============================================================================

/**
 * Message memory pool for efficient allocation
 */
typedef struct MessagePool {
    Message* free_list;
    uint64_t pool_size;
    uint64_t allocated;
    uint64_t freed;
    uint64_t peak_usage;
} MessagePool;

// ============================================================================
// MESSAGE CHANNEL
// ============================================================================

/**
 * Message channel (bidirectional communication)
 */
typedef struct MessageChannel {
    uint32_t id;
    uint32_t thread_a;
    uint32_t thread_b;
    
    PriorityMessageQueue a_to_b;
    PriorityMessageQueue b_to_a;
    
    // Statistics
    uint64_t messages_sent;
    uint64_t messages_received;
    uint64_t bytes_transferred;
} MessageChannel;

// ============================================================================
// MESSAGE SYSTEM
// ============================================================================

/**
 * Global message passing system
 */
typedef struct MessageSystem {
    MessagePool* pool;
    MessageChannel** channels;
    uint32_t num_channels;
    uint32_t max_channels;
    
    // Global statistics
    uint64_t total_messages;
    uint64_t total_bytes;
    uint64_t dropped_messages;
} MessageSystem;

// ============================================================================
// CORE API
// ============================================================================

/**
 * Initialize message passing system
 */
MessageSystem* message_system_create(uint32_t max_channels, uint32_t pool_size);

/**
 * Destroy message passing system
 */
void message_system_destroy(MessageSystem* system);

/**
 * Create a message channel between two threads
 */
MessageChannel* message_channel_create(MessageSystem* system, uint32_t thread_a, uint32_t thread_b);

/**
 * Destroy a message channel
 */
void message_channel_destroy(MessageChannel* channel);

// ============================================================================
// MESSAGE OPERATIONS
// ============================================================================

/**
 * Allocate a message from the pool
 */
Message* message_alloc(MessageSystem* system);

/**
 * Free a message back to the pool
 */
void message_free(MessageSystem* system, Message* msg);

/**
 * Create a message with data
 */
Message* message_create(
    MessageSystem* system,
    MessageType type,
    MessagePriority priority,
    uint32_t sender_id,
    uint32_t receiver_id,
    void* data,
    size_t data_size
);

/**
 * Send a message through a channel
 */
bool message_send(MessageChannel* channel, Message* msg);

/**
 * Receive a message from a channel (non-blocking)
 */
Message* message_receive(MessageChannel* channel, uint32_t thread_id);

/**
 * Receive a message with timeout (blocking)
 */
Message* message_receive_timeout(MessageChannel* channel, uint32_t thread_id, uint64_t timeout_ns);

/**
 * Broadcast a message to all threads
 */
bool message_broadcast(MessageSystem* system, Message* msg);

// ============================================================================
// BATCH OPERATIONS
// ============================================================================

/**
 * Send multiple messages at once
 */
bool message_send_batch(MessageChannel* channel, Message** messages, uint32_t count);

/**
 * Receive multiple messages at once
 */
uint32_t message_receive_batch(MessageChannel* channel, uint32_t thread_id, Message** out_messages, uint32_t max_count);

// ============================================================================
// QUEUE OPERATIONS
// ============================================================================

/**
 * Initialize a message queue
 */
void message_queue_init(MessageQueue* queue);

/**
 * Enqueue a message (lock-free)
 */
bool message_queue_enqueue(MessageQueue* queue, Message* msg);

/**
 * Dequeue a message (lock-free)
 */
Message* message_queue_dequeue(MessageQueue* queue);

/**
 * Check if queue is empty
 */
bool message_queue_is_empty(MessageQueue* queue);

/**
 * Get queue size (approximate)
 */
uint64_t message_queue_size(MessageQueue* queue);

// ============================================================================
// PRIORITY QUEUE OPERATIONS
// ============================================================================

/**
 * Initialize a priority queue
 */
void priority_queue_init(PriorityMessageQueue* pqueue);

/**
 * Enqueue a message with priority
 */
bool priority_queue_enqueue(PriorityMessageQueue* pqueue, Message* msg);

/**
 * Dequeue highest priority message
 */
Message* priority_queue_dequeue(PriorityMessageQueue* pqueue);

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * Get current timestamp in nanoseconds
 */
uint64_t message_get_timestamp(void);

/**
 * Check if message has expired
 */
bool message_is_expired(Message* msg);

/**
 * Get message type name
 */
const char* message_type_name(MessageType type);

/**
 * Get priority name
 */
const char* message_priority_name(MessagePriority priority);

/**
 * Print message information
 */
void message_print(Message* msg);

/**
 * Print channel statistics
 */
void message_channel_print_stats(MessageChannel* channel);

/**
 * Print system statistics
 */
void message_system_print_stats(MessageSystem* system);

#ifdef __cplusplus
}
#endif

#endif // MESSAGE_PASSING_H