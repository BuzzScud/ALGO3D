#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>

#include "ai/cllm_sphere_message.h"
#include "ai/cllm_message_queue.h"

// Test counter
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) \
    printf("\n=== Testing: %s ===\n", #name); \
    if (test_##name()) { \
        printf("✓ %s PASSED\n", #name); \
        tests_passed++; \
    } else { \
        printf("✗ %s FAILED\n", #name); \
        tests_failed++; \
    }

// ============================================================================
// SPHERE MESSAGE TESTS
// ============================================================================

int test_message_creation() {
    SphereMessage* msg = sphere_message_create(
        MSG_WORK_REQUEST,
        MSG_PRIORITY_NORMAL,
        0,  // sender
        1   // receiver
    );
    
    if (!msg) {
        printf("ERROR: Failed to create message\n");
        return 0;
    }
    
    if (msg->type != MSG_WORK_REQUEST) {
        printf("ERROR: Wrong message type\n");
        sphere_message_free(msg);
        return 0;
    }
    
    if (msg->priority != MSG_PRIORITY_NORMAL) {
        printf("ERROR: Wrong priority\n");
        sphere_message_free(msg);
        return 0;
    }
    
    if (msg->sender_id != 0 || msg->receiver_id != 1) {
        printf("ERROR: Wrong sender/receiver\n");
        sphere_message_free(msg);
        return 0;
    }
    
    sphere_message_free(msg);
    return 1;
}

int test_message_payload_work_request() {
    SphereMessage* msg = sphere_message_create(
        MSG_WORK_REQUEST,
        MSG_PRIORITY_HIGH,
        0, 1
    );
    
    sphere_message_set_work_request(msg, 100, 5, 50);
    
    if (msg->payload.work_request.requested_items != 100) {
        printf("ERROR: Wrong requested_items\n");
        sphere_message_free(msg);
        return 0;
    }
    
    if (msg->payload.work_request.symmetry_group != 5) {
        printf("ERROR: Wrong symmetry_group\n");
        sphere_message_free(msg);
        return 0;
    }
    
    if (msg->payload.work_request.current_load != 50) {
        printf("ERROR: Wrong current_load\n");
        sphere_message_free(msg);
        return 0;
    }
    
    sphere_message_free(msg);
    return 1;
}

int test_message_payload_boundary() {
    SphereMessage* msg = sphere_message_create(
        MSG_BOUNDARY_CROSSING,
        MSG_PRIORITY_CRITICAL,
        0, -1  // broadcast
    );
    
    sphere_message_set_boundary(msg, 144000, 11, 0.0, 0);
    
    if (msg->payload.boundary.prime != 144000) {
        printf("ERROR: Wrong prime\n");
        sphere_message_free(msg);
        return 0;
    }
    
    if (msg->payload.boundary.symmetry_group != 11) {
        printf("ERROR: Wrong symmetry_group\n");
        sphere_message_free(msg);
        return 0;
    }
    
    sphere_message_free(msg);
    return 1;
}

int test_message_clone() {
    SphereMessage* original = sphere_message_create(
        MSG_EPOCH_START,
        MSG_PRIORITY_HIGH,
        0, 1
    );
    
    sphere_message_set_epoch(original, 10, 1000, 0.001);
    
    SphereMessage* clone = sphere_message_clone(original);
    
    if (!clone) {
        printf("ERROR: Failed to clone message\n");
        sphere_message_free(original);
        return 0;
    }
    
    if (clone->type != original->type) {
        printf("ERROR: Clone has wrong type\n");
        sphere_message_free(original);
        sphere_message_free(clone);
        return 0;
    }
    
    if (clone->payload.epoch.epoch_number != 10) {
        printf("ERROR: Clone has wrong payload\n");
        sphere_message_free(original);
        sphere_message_free(clone);
        return 0;
    }
    
    // Message IDs should be different
    if (clone->message_id == original->message_id) {
        printf("ERROR: Clone has same message ID\n");
        sphere_message_free(original);
        sphere_message_free(clone);
        return 0;
    }
    
    sphere_message_free(original);
    sphere_message_free(clone);
    return 1;
}

int test_message_state() {
    SphereMessage* msg = sphere_message_create(
        MSG_GRADIENT_READY,
        MSG_PRIORITY_NORMAL,
        0, 1
    );
    
    if (sphere_message_is_processed(msg)) {
        printf("ERROR: New message should not be processed\n");
        sphere_message_free(msg);
        return 0;
    }
    
    sphere_message_mark_processed(msg);
    
    if (!sphere_message_is_processed(msg)) {
        printf("ERROR: Message should be processed\n");
        sphere_message_free(msg);
        return 0;
    }
    
    sphere_message_mark_acknowledged(msg);
    
    if (!sphere_message_is_acknowledged(msg)) {
        printf("ERROR: Message should be acknowledged\n");
        sphere_message_free(msg);
        return 0;
    }
    
    sphere_message_free(msg);
    return 1;
}

int test_message_validation() {
    SphereMessage* msg = sphere_message_create(
        MSG_STATS_REQUEST,
        MSG_PRIORITY_LOW,
        0, 1
    );
    
    if (!sphere_message_validate(msg)) {
        printf("ERROR: Valid message failed validation\n");
        sphere_message_free(msg);
        return 0;
    }
    
    // Test invalid symmetry group
    msg->sender_symmetry_group = 15;  // Invalid (should be 0-11)
    
    if (sphere_message_validate(msg)) {
        printf("ERROR: Invalid message passed validation\n");
        sphere_message_free(msg);
        return 0;
    }
    
    sphere_message_free(msg);
    return 1;
}

int test_message_print() {
    SphereMessage* msg = sphere_message_create(
        MSG_ERROR_REPORT,
        MSG_PRIORITY_CRITICAL,
        5, 0
    );
    
    sphere_message_set_error(msg, 42, "Test error message", 2);
    
    // Just verify it doesn't crash
    sphere_message_print(msg);
    
    sphere_message_free(msg);
    return 1;
}

// ============================================================================
// MESSAGE QUEUE TESTS
// ============================================================================

int test_queue_creation() {
    LockFreeMessageQueue* queue = message_queue_create(1000, false);
    
    if (!queue) {
        printf("ERROR: Failed to create queue\n");
        return 0;
    }
    
    if (!message_queue_is_empty(queue)) {
        printf("ERROR: New queue should be empty\n");
        message_queue_free(queue);
        return 0;
    }
    
    if (message_queue_size(queue) != 0) {
        printf("ERROR: New queue should have size 0\n");
        message_queue_free(queue);
        return 0;
    }
    
    message_queue_free(queue);
    return 1;
}

int test_queue_enqueue_dequeue() {
    LockFreeMessageQueue* queue = message_queue_create(0, false);
    
    // Create and enqueue message
    SphereMessage* msg1 = sphere_message_create(MSG_WORK_REQUEST, MSG_PRIORITY_NORMAL, 0, 1);
    sphere_message_set_work_request(msg1, 50, 3, 25);
    
    if (!message_queue_enqueue(queue, msg1)) {
        printf("ERROR: Failed to enqueue message\n");
        message_queue_free(queue);
        return 0;
    }
    
    if (message_queue_size(queue) != 1) {
        printf("ERROR: Queue size should be 1\n");
        message_queue_free(queue);
        return 0;
    }
    
    // Dequeue message
    SphereMessage* msg2 = message_queue_dequeue(queue);
    
    if (!msg2) {
        printf("ERROR: Failed to dequeue message\n");
        message_queue_free(queue);
        return 0;
    }
    
    if (msg2->type != MSG_WORK_REQUEST) {
        printf("ERROR: Dequeued wrong message type\n");
        sphere_message_free(msg2);
        message_queue_free(queue);
        return 0;
    }
    
    if (msg2->payload.work_request.requested_items != 50) {
        printf("ERROR: Dequeued message has wrong payload\n");
        sphere_message_free(msg2);
        message_queue_free(queue);
        return 0;
    }
    
    if (!message_queue_is_empty(queue)) {
        printf("ERROR: Queue should be empty after dequeue\n");
        sphere_message_free(msg2);
        message_queue_free(queue);
        return 0;
    }
    
    sphere_message_free(msg2);
    message_queue_free(queue);
    return 1;
}

int test_queue_priority() {
    LockFreeMessageQueue* queue = message_queue_create(0, false);
    
    // Enqueue messages with different priorities
    SphereMessage* low = sphere_message_create(MSG_WORK_REQUEST, MSG_PRIORITY_LOW, 0, 1);
    SphereMessage* normal = sphere_message_create(MSG_WORK_REQUEST, MSG_PRIORITY_NORMAL, 0, 1);
    SphereMessage* high = sphere_message_create(MSG_WORK_REQUEST, MSG_PRIORITY_HIGH, 0, 1);
    SphereMessage* critical = sphere_message_create(MSG_WORK_REQUEST, MSG_PRIORITY_CRITICAL, 0, 1);
    
    // Enqueue in random order
    message_queue_enqueue(queue, normal);
    message_queue_enqueue(queue, low);
    message_queue_enqueue(queue, critical);
    message_queue_enqueue(queue, high);
    
    if (message_queue_size(queue) != 4) {
        printf("ERROR: Queue should have 4 messages\n");
        message_queue_free(queue);
        return 0;
    }
    
    // Dequeue should return highest priority first
    SphereMessage* msg1 = message_queue_dequeue(queue);
    if (msg1->priority != MSG_PRIORITY_CRITICAL) {
        printf("ERROR: First dequeue should be CRITICAL, got %d\n", msg1->priority);
        sphere_message_free(msg1);
        message_queue_free(queue);
        return 0;
    }
    sphere_message_free(msg1);
    
    SphereMessage* msg2 = message_queue_dequeue(queue);
    if (msg2->priority != MSG_PRIORITY_HIGH) {
        printf("ERROR: Second dequeue should be HIGH, got %d\n", msg2->priority);
        sphere_message_free(msg2);
        message_queue_free(queue);
        return 0;
    }
    sphere_message_free(msg2);
    
    SphereMessage* msg3 = message_queue_dequeue(queue);
    if (msg3->priority != MSG_PRIORITY_NORMAL) {
        printf("ERROR: Third dequeue should be NORMAL, got %d\n", msg3->priority);
        sphere_message_free(msg3);
        message_queue_free(queue);
        return 0;
    }
    sphere_message_free(msg3);
    
    SphereMessage* msg4 = message_queue_dequeue(queue);
    if (msg4->priority != MSG_PRIORITY_LOW) {
        printf("ERROR: Fourth dequeue should be LOW, got %d\n", msg4->priority);
        sphere_message_free(msg4);
        message_queue_free(queue);
        return 0;
    }
    sphere_message_free(msg4);
    
    message_queue_free(queue);
    return 1;
}

int test_queue_max_size() {
    LockFreeMessageQueue* queue = message_queue_create(3, true);  // Max 3, drop on full
    
    // Enqueue 3 messages
    for (int i = 0; i < 3; i++) {
        SphereMessage* msg = sphere_message_create(MSG_WORK_REQUEST, MSG_PRIORITY_NORMAL, 0, 1);
        if (!message_queue_enqueue(queue, msg)) {
            printf("ERROR: Failed to enqueue message %d\n", i);
            message_queue_free(queue);
            return 0;
        }
    }
    
    if (!message_queue_is_full(queue)) {
        printf("ERROR: Queue should be full\n");
        message_queue_free(queue);
        return 0;
    }
    
    // Try to enqueue 4th message (should be dropped)
    SphereMessage* msg4 = sphere_message_create(MSG_WORK_REQUEST, MSG_PRIORITY_NORMAL, 0, 1);
    bool result = message_queue_enqueue(queue, msg4);
    
    // Message should be dropped (enqueue returns false and frees message)
    if (result) {
        printf("ERROR: Enqueue should fail when queue is full\n");
        message_queue_free(queue);
        return 0;
    }
    
    if (message_queue_size(queue) != 3) {
        printf("ERROR: Queue size should still be 3\n");
        message_queue_free(queue);
        return 0;
    }
    
    message_queue_free(queue);
    return 1;
}

int test_queue_peek() {
    LockFreeMessageQueue* queue = message_queue_create(0, false);
    
    SphereMessage* msg1 = sphere_message_create(MSG_EPOCH_START, MSG_PRIORITY_HIGH, 0, 1);
    sphere_message_set_epoch(msg1, 5, 100, 0.01);
    
    message_queue_enqueue(queue, msg1);
    
    // Peek should return message without removing it
    SphereMessage* peeked = message_queue_peek(queue);
    
    if (!peeked) {
        printf("ERROR: Peek returned NULL\n");
        message_queue_free(queue);
        return 0;
    }
    
    if (peeked->type != MSG_EPOCH_START) {
        printf("ERROR: Peeked wrong message type\n");
        message_queue_free(queue);
        return 0;
    }
    
    // Queue should still have the message
    if (message_queue_size(queue) != 1) {
        printf("ERROR: Peek should not remove message\n");
        message_queue_free(queue);
        return 0;
    }
    
    // Now dequeue it
    SphereMessage* dequeued = message_queue_dequeue(queue);
    sphere_message_free(dequeued);
    
    if (!message_queue_is_empty(queue)) {
        printf("ERROR: Queue should be empty after dequeue\n");
        message_queue_free(queue);
        return 0;
    }
    
    message_queue_free(queue);
    return 1;
}

int test_queue_statistics() {
    LockFreeMessageQueue* queue = message_queue_create(0, false);
    
    // Enqueue and dequeue some messages
    for (int i = 0; i < 10; i++) {
        SphereMessage* msg = sphere_message_create(MSG_WORK_REQUEST, MSG_PRIORITY_NORMAL, 0, 1);
        message_queue_enqueue(queue, msg);
    }
    
    for (int i = 0; i < 5; i++) {
        SphereMessage* msg = message_queue_dequeue(queue);
        sphere_message_free(msg);
    }
    
    MessageQueueStatistics stats;
    message_queue_get_statistics(queue, &stats);
    
    if (stats.total_enqueued != 10) {
        printf("ERROR: total_enqueued should be 10, got %lu\n", 
               (unsigned long)stats.total_enqueued);
        message_queue_free(queue);
        return 0;
    }
    
    if (stats.total_dequeued != 5) {
        printf("ERROR: total_dequeued should be 5, got %lu\n",
               (unsigned long)stats.total_dequeued);
        message_queue_free(queue);
        return 0;
    }
    
    if (stats.current_size != 5) {
        printf("ERROR: current_size should be 5, got %lu\n",
               (unsigned long)stats.current_size);
        message_queue_free(queue);
        return 0;
    }
    
    // Print statistics (just verify it doesn't crash)
    message_queue_print_statistics(queue);
    
    message_queue_free(queue);
    return 1;
}

int test_queue_batch_operations() {
    LockFreeMessageQueue* queue = message_queue_create(0, false);
    
    // Create batch of messages
    SphereMessage* messages[10];
    for (int i = 0; i < 10; i++) {
        messages[i] = sphere_message_create(MSG_WORK_REQUEST, MSG_PRIORITY_NORMAL, 0, 1);
    }
    
    // Enqueue batch
    uint64_t enqueued = message_queue_enqueue_batch(queue, messages, 10);
    
    if (enqueued != 10) {
        printf("ERROR: Should enqueue 10 messages, got %lu\n", (unsigned long)enqueued);
        message_queue_free(queue);
        return 0;
    }
    
    // Dequeue batch
    SphereMessage* dequeued[10];
    uint64_t dequeued_count = message_queue_dequeue_batch(queue, dequeued, 10);
    
    if (dequeued_count != 10) {
        printf("ERROR: Should dequeue 10 messages, got %lu\n", (unsigned long)dequeued_count);
        message_queue_free(queue);
        return 0;
    }
    
    // Free dequeued messages
    for (uint64_t i = 0; i < dequeued_count; i++) {
        sphere_message_free(dequeued[i]);
    }
    
    if (!message_queue_is_empty(queue)) {
        printf("ERROR: Queue should be empty\n");
        message_queue_free(queue);
        return 0;
    }
    
    message_queue_free(queue);
    return 1;
}

// ============================================================================
// CONCURRENT TESTS
// ============================================================================

typedef struct {
    LockFreeMessageQueue* queue;
    int thread_id;
    int num_messages;
} ThreadData;

void* producer_thread(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    
    for (int i = 0; i < data->num_messages; i++) {
        SphereMessage* msg = sphere_message_create(
            MSG_WORK_REQUEST,
            MSG_PRIORITY_NORMAL,
            data->thread_id,
            -1
        );
        
        message_queue_enqueue(data->queue, msg);
    }
    
    return NULL;
}

void* consumer_thread(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    int consumed = 0;
    
    while (consumed < data->num_messages) {
        SphereMessage* msg = message_queue_dequeue(data->queue);
        if (msg) {
            sphere_message_free(msg);
            consumed++;
        } else {
            // Queue empty, sleep briefly
            usleep(100);
        }
    }
    
    return NULL;
}

int test_queue_concurrent() {
    LockFreeMessageQueue* queue = message_queue_create(0, false);
    
    const int num_producers = 4;
    const int num_consumers = 4;
    const int messages_per_thread = 100;
    
    pthread_t producers[num_producers];
    pthread_t consumers[num_consumers];
    ThreadData producer_data[num_producers];
    ThreadData consumer_data[num_consumers];
    
    // Start producers
    for (int i = 0; i < num_producers; i++) {
        producer_data[i].queue = queue;
        producer_data[i].thread_id = i;
        producer_data[i].num_messages = messages_per_thread;
        pthread_create(&producers[i], NULL, producer_thread, &producer_data[i]);
    }
    
    // Start consumers
    for (int i = 0; i < num_consumers; i++) {
        consumer_data[i].queue = queue;
        consumer_data[i].thread_id = i;
        consumer_data[i].num_messages = messages_per_thread;
        pthread_create(&consumers[i], NULL, consumer_thread, &consumer_data[i]);
    }
    
    // Wait for producers
    for (int i = 0; i < num_producers; i++) {
        pthread_join(producers[i], NULL);
    }
    
    // Wait for consumers
    for (int i = 0; i < num_consumers; i++) {
        pthread_join(consumers[i], NULL);
    }
    
    // Queue should be empty
    if (!message_queue_is_empty(queue)) {
        printf("ERROR: Queue should be empty after concurrent test\n");
        printf("Remaining messages: %lu\n", (unsigned long)message_queue_size(queue));
        message_queue_free(queue);
        return 0;
    }
    
    MessageQueueStatistics stats;
    message_queue_get_statistics(queue, &stats);
    
    uint64_t expected_total = num_producers * messages_per_thread;
    if (stats.total_enqueued != expected_total) {
        printf("ERROR: Expected %lu enqueued, got %lu\n",
               (unsigned long)expected_total,
               (unsigned long)stats.total_enqueued);
        message_queue_free(queue);
        return 0;
    }
    
    if (stats.total_dequeued != expected_total) {
        printf("ERROR: Expected %lu dequeued, got %lu\n",
               (unsigned long)expected_total,
               (unsigned long)stats.total_dequeued);
        message_queue_free(queue);
        return 0;
    }
    
    message_queue_free(queue);
    return 1;
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main(void) {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║     MESSAGE PASSING SYSTEM - COMPREHENSIVE TESTS          ║\n");
    printf("║     Sphere Messages & Lock-Free Message Queue             ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    // Sphere Message Tests
    printf("\n--- SPHERE MESSAGE TESTS ---\n");
    TEST(message_creation);
    TEST(message_payload_work_request);
    TEST(message_payload_boundary);
    TEST(message_clone);
    TEST(message_state);
    TEST(message_validation);
    TEST(message_print);
    
    // Message Queue Tests
    printf("\n--- MESSAGE QUEUE TESTS ---\n");
    TEST(queue_creation);
    TEST(queue_enqueue_dequeue);
    TEST(queue_priority);
    TEST(queue_max_size);
    TEST(queue_peek);
    TEST(queue_statistics);
    TEST(queue_batch_operations);
    
    // Concurrent Tests
    printf("\n--- CONCURRENT TESTS ---\n");
    TEST(queue_concurrent);
    
    // Summary
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                      TEST SUMMARY                          ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║  Total Tests:  %3d                                         ║\n", 
           tests_passed + tests_failed);
    printf("║  Passed:       %3d ✓                                       ║\n", 
           tests_passed);
    printf("║  Failed:       %3d ✗                                       ║\n", 
           tests_failed);
    printf("║  Success Rate: %3d%%                                        ║\n",
           (tests_passed * 100) / (tests_passed + tests_failed));
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    return tests_failed > 0 ? 1 : 0;
}