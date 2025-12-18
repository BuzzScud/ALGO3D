/**
 * @file test_message_passing.c
 * @brief Comprehensive tests for message passing system
 */

#define _GNU_SOURCE
#include "message_passing.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>

// Test framework
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) \
    printf("\n=== TEST: %s ===\n", #name); \
    if (test_##name()) { \
        printf("✓ PASSED: %s\n", #name); \
        tests_passed++; \
    } else { \
        printf("✗ FAILED: %s\n", #name); \
        tests_failed++; \
    }

// ============================================================================
// BASIC TESTS
// ============================================================================

bool test_system_create_destroy() {
    MessageSystem* system = message_system_create(10, 100);
    assert(system != NULL);
    assert(system->max_channels == 10);
    assert(system->pool != NULL);
    
    message_system_destroy(system);
    return true;
}

bool test_message_alloc_free() {
    MessageSystem* system = message_system_create(10, 100);
    assert(system != NULL);
    
    Message* msg = message_alloc(system);
    assert(msg != NULL);
    assert(msg->id > 0);
    assert(msg->timestamp > 0);
    
    message_free(system, msg);
    
    message_system_destroy(system);
    return true;
}

bool test_message_create() {
    MessageSystem* system = message_system_create(10, 100);
    assert(system != NULL);
    
    char* data = strdup("Hello, World!");
    Message* msg = message_create(
        system,
        MSG_TYPE_DATA_SEND,
        MSG_PRIORITY_NORMAL,
        1,  // sender
        2,  // receiver
        data,
        strlen(data) + 1
    );
    
    assert(msg != NULL);
    assert(msg->type == MSG_TYPE_DATA_SEND);
    assert(msg->priority == MSG_PRIORITY_NORMAL);
    assert(msg->sender_id == 1);
    assert(msg->receiver_id == 2);
    assert(msg->data == data);
    assert(msg->data_size == strlen(data) + 1);
    
    message_free(system, msg);
    message_system_destroy(system);
    return true;
}

// ============================================================================
// QUEUE TESTS
// ============================================================================

bool test_queue_basic() {
    MessageSystem* system = message_system_create(10, 100);
    MessageQueue queue;
    message_queue_init(&queue);
    
    assert(message_queue_is_empty(&queue));
    assert(message_queue_size(&queue) == 0);
    
    // Enqueue messages
    for (int i = 0; i < 5; i++) {
        Message* msg = message_alloc(system);
        msg->type = MSG_TYPE_DATA_SEND;
        
        // Store identifier in data
        int* data = (int*)malloc(sizeof(int));
        *data = i;
        msg->data = data;
        msg->data_size = sizeof(int);
        
        bool result = message_queue_enqueue(&queue, msg);
        assert(result);
    }
    
    assert(!message_queue_is_empty(&queue));
    assert(message_queue_size(&queue) == 5);
    
    // Dequeue messages
    for (int i = 0; i < 5; i++) {
        Message* msg = message_queue_dequeue(&queue);
        assert(msg != NULL);
        int* data = (int*)msg->data;
        assert(*data == i);
        message_free(system, msg);
    }
    
    assert(message_queue_is_empty(&queue));
    
    message_system_destroy(system);
    return true;
}

bool test_priority_queue() {
    MessageSystem* system = message_system_create(10, 100);
    PriorityMessageQueue pqueue;
    priority_queue_init(&pqueue);
    
    // Enqueue messages with different priorities
    for (int i = 0; i < 4; i++) {
        for (int p = 0; p < 4; p++) {
            Message* msg = message_alloc(system);
            msg->type = MSG_TYPE_DATA_SEND;
            msg->priority = p;
            msg->sender_id = i * 4 + p;
            
            bool result = priority_queue_enqueue(&pqueue, msg);
            assert(result);
        }
    }
    
    // Dequeue should return highest priority first
    Message* msg1 = priority_queue_dequeue(&pqueue);
    assert(msg1 != NULL);
    assert(msg1->priority == MSG_PRIORITY_CRITICAL);
    message_free(system, msg1);
    
    Message* msg2 = priority_queue_dequeue(&pqueue);
    assert(msg2 != NULL);
    assert(msg2->priority == MSG_PRIORITY_CRITICAL);
    message_free(system, msg2);
    
    // Drain remaining messages
    while (true) {
        Message* msg = priority_queue_dequeue(&pqueue);
        if (!msg) break;
        message_free(system, msg);
    }
    
    message_system_destroy(system);
    return true;
}

// ============================================================================
// CHANNEL TESTS
// ============================================================================

bool test_channel_create() {
    MessageSystem* system = message_system_create(10, 100);
    
    MessageChannel* channel = message_channel_create(system, 1, 2);
    assert(channel != NULL);
    assert(channel->thread_a == 1);
    assert(channel->thread_b == 2);
    
    message_channel_destroy(channel);
    message_system_destroy(system);
    return true;
}

bool test_channel_send_receive() {
    MessageSystem* system = message_system_create(10, 100);
    MessageChannel* channel = message_channel_create(system, 1, 2);
    
    // Thread 1 sends to Thread 2
    char* data = strdup("Hello from Thread 1");
    Message* msg = message_create(
        system,
        MSG_TYPE_DATA_SEND,
        MSG_PRIORITY_NORMAL,
        1,  // sender
        2,  // receiver
        data,
        strlen(data) + 1
    );
    
    bool sent = message_send(channel, msg);
    assert(sent);
    
    // Thread 2 receives
    Message* received = message_receive(channel, 2);
    assert(received != NULL);
    assert(received->sender_id == 1);
    assert(received->receiver_id == 2);
    assert(strcmp((char*)received->data, "Hello from Thread 1") == 0);
    
    message_free(system, received);
    message_channel_destroy(channel);
    message_system_destroy(system);
    return true;
}

bool test_channel_bidirectional() {
    MessageSystem* system = message_system_create(10, 100);
    MessageChannel* channel = message_channel_create(system, 1, 2);
    
    // Thread 1 -> Thread 2
    Message* msg1 = message_create(system, MSG_TYPE_DATA_SEND, MSG_PRIORITY_NORMAL, 1, 2, NULL, 0);
    message_send(channel, msg1);
    
    // Thread 2 -> Thread 1
    Message* msg2 = message_create(system, MSG_TYPE_DATA_SEND, MSG_PRIORITY_NORMAL, 2, 1, NULL, 0);
    message_send(channel, msg2);
    
    // Thread 2 receives from Thread 1
    Message* recv1 = message_receive(channel, 2);
    assert(recv1 != NULL);
    assert(recv1->sender_id == 1);
    message_free(system, recv1);
    
    // Thread 1 receives from Thread 2
    Message* recv2 = message_receive(channel, 1);
    assert(recv2 != NULL);
    assert(recv2->sender_id == 2);
    message_free(system, recv2);
    
    message_channel_destroy(channel);
    message_system_destroy(system);
    return true;
}

// ============================================================================
// BATCH OPERATIONS
// ============================================================================

bool test_batch_send_receive() {
    MessageSystem* system = message_system_create(10, 100);
    MessageChannel* channel = message_channel_create(system, 1, 2);
    
    // Create batch of messages
    const int BATCH_SIZE = 10;
    Message* messages[BATCH_SIZE];
    
    for (int i = 0; i < BATCH_SIZE; i++) {
        // Allocate data to use as identifier
        int* data = (int*)malloc(sizeof(int));
        *data = i;
        
        messages[i] = message_create(
            system,
            MSG_TYPE_DATA_SEND,
            MSG_PRIORITY_NORMAL,
            1,  // sender
            2,  // receiver
            data,
            sizeof(int)
        );
    }
    
    // Send batch
    bool sent = message_send_batch(channel, messages, BATCH_SIZE);
    if (!sent) {
        printf("ERROR: Batch send failed\n");
        for (int i = 0; i < BATCH_SIZE; i++) {
            printf("  Message %d: %p, sender_id=%u\n", i, (void*)messages[i], messages[i] ? messages[i]->sender_id : 0);
        }
    }
    assert(sent);
    
    // Receive batch
    Message* received[BATCH_SIZE];
    uint32_t count = message_receive_batch(channel, 2, received, BATCH_SIZE);
    assert(count == BATCH_SIZE);
    
    // Verify order
    for (int i = 0; i < BATCH_SIZE; i++) {
        int* data = (int*)received[i]->data;
        assert(*data == i);
        message_free(system, received[i]);
    }
    
    message_channel_destroy(channel);
    message_system_destroy(system);
    return true;
}

// ============================================================================
// PRIORITY TESTS
// ============================================================================

bool test_priority_ordering() {
    MessageSystem* system = message_system_create(10, 100);
    MessageChannel* channel = message_channel_create(system, 1, 2);
    
    // Send messages with different priorities (in reverse order)
    Message* low = message_create(system, MSG_TYPE_DATA_SEND, MSG_PRIORITY_LOW, 1, 2, NULL, 0);
    message_send(channel, low);
    
    Message* normal = message_create(system, MSG_TYPE_DATA_SEND, MSG_PRIORITY_NORMAL, 1, 2, NULL, 0);
    message_send(channel, normal);
    
    Message* high = message_create(system, MSG_TYPE_DATA_SEND, MSG_PRIORITY_HIGH, 1, 2, NULL, 0);
    message_send(channel, high);
    
    Message* critical = message_create(system, MSG_TYPE_DATA_SEND, MSG_PRIORITY_CRITICAL, 1, 2, NULL, 0);
    message_send(channel, critical);
    
    // Receive should get highest priority first
    Message* recv1 = message_receive(channel, 2);
    assert(recv1->priority == MSG_PRIORITY_CRITICAL);
    message_free(system, recv1);
    
    Message* recv2 = message_receive(channel, 2);
    assert(recv2->priority == MSG_PRIORITY_HIGH);
    message_free(system, recv2);
    
    Message* recv3 = message_receive(channel, 2);
    assert(recv3->priority == MSG_PRIORITY_NORMAL);
    message_free(system, recv3);
    
    Message* recv4 = message_receive(channel, 2);
    assert(recv4->priority == MSG_PRIORITY_LOW);
    message_free(system, recv4);
    
    message_channel_destroy(channel);
    message_system_destroy(system);
    return true;
}

// ============================================================================
// UTILITY TESTS
// ============================================================================

bool test_message_types() {
    const char* name = message_type_name(MSG_TYPE_INIT);
    assert(strcmp(name, "INIT") == 0);
    
    name = message_type_name(MSG_TYPE_WORK_REQUEST);
    assert(strcmp(name, "WORK_REQUEST") == 0);
    
    name = message_type_name(MSG_TYPE_ERROR);
    assert(strcmp(name, "ERROR") == 0);
    
    return true;
}

bool test_priority_names() {
    const char* name = message_priority_name(MSG_PRIORITY_CRITICAL);
    assert(strcmp(name, "CRITICAL") == 0);
    
    name = message_priority_name(MSG_PRIORITY_NORMAL);
    assert(strcmp(name, "NORMAL") == 0);
    
    return true;
}

bool test_timestamp() {
    uint64_t t1 = message_get_timestamp();
    usleep(1000);  // 1ms
    uint64_t t2 = message_get_timestamp();
    
    assert(t2 > t1);
    assert((t2 - t1) >= 1000000);  // At least 1ms difference
    
    return true;
}

bool test_message_print() {
    MessageSystem* system = message_system_create(10, 100);
    
    Message* msg = message_create(
        system,
        MSG_TYPE_DATA_SEND,
        MSG_PRIORITY_HIGH,
        1,
        2,
        NULL,
        0
    );
    
    printf("\n");
    message_print(msg);
    
    message_free(system, msg);
    message_system_destroy(system);
    return true;
}

// ============================================================================
// STATISTICS TESTS
// ============================================================================

bool test_channel_statistics() {
    MessageSystem* system = message_system_create(10, 100);
    MessageChannel* channel = message_channel_create(system, 1, 2);
    
    // Send some messages
    for (int i = 0; i < 5; i++) {
        Message* msg = message_create(system, MSG_TYPE_DATA_SEND, MSG_PRIORITY_NORMAL, 1, 2, NULL, 0);
        message_send(channel, msg);
    }
    
    // Receive some messages
    for (int i = 0; i < 3; i++) {
        Message* msg = message_receive(channel, 2);
        message_free(system, msg);
    }
    
    message_channel_print_stats(channel);
    
    assert(channel->messages_sent == 5);
    assert(channel->messages_received == 3);
    
    message_channel_destroy(channel);
    message_system_destroy(system);
    return true;
}

bool test_system_statistics() {
    MessageSystem* system = message_system_create(10, 100);
    
    // Create some channels
    MessageChannel* ch1 = message_channel_create(system, 1, 2);
    MessageChannel* ch2 = message_channel_create(system, 3, 4);
    
    // Send some messages
    for (int i = 0; i < 10; i++) {
        Message* msg = message_create(system, MSG_TYPE_DATA_SEND, MSG_PRIORITY_NORMAL, 1, 2, NULL, 0);
        message_send(ch1, msg);
    }
    
    message_system_print_stats(system);
    
    message_channel_destroy(ch1);
    message_channel_destroy(ch2);
    message_system_destroy(system);
    return true;
}

// ============================================================================
// STRESS TESTS
// ============================================================================

bool test_stress_many_messages() {
    MessageSystem* system = message_system_create(10, 1000);
    MessageChannel* channel = message_channel_create(system, 1, 2);
    
    const int NUM_MESSAGES = 1000;
    
    printf("Sending %d messages...\n", NUM_MESSAGES);
    
    // Send many messages
    for (int i = 0; i < NUM_MESSAGES; i++) {
        int* data = (int*)malloc(sizeof(int));
        *data = i;
        
        Message* msg = message_create(
            system,
            MSG_TYPE_DATA_SEND,
            MSG_PRIORITY_NORMAL,
            1,
            2,
            data,
            sizeof(int)
        );
        
        bool sent = message_send(channel, msg);
        assert(sent);
    }
    
    printf("Receiving %d messages...\n", NUM_MESSAGES);
    
    // Receive all messages
    for (int i = 0; i < NUM_MESSAGES; i++) {
        Message* msg = message_receive(channel, 2);
        assert(msg != NULL);
        int* data = (int*)msg->data;
        assert(*data == i);
        message_free(system, msg);
    }
    
    printf("All %d messages verified\n", NUM_MESSAGES);
    
    message_channel_destroy(channel);
    message_system_destroy(system);
    return true;
}

// ============================================================================
// MAIN
// ============================================================================

int main(void) {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║   WEEK 3: MESSAGE PASSING SYSTEM - COMPREHENSIVE TESTS    ║\n");
    printf("║   Lock-Free Message Passing with Priority Queues          ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    printf("\n--- Basic Tests ---\n");
    TEST(system_create_destroy);
    TEST(message_alloc_free);
    TEST(message_create);
    
    printf("\n--- Queue Tests ---\n");
    TEST(queue_basic);
    TEST(priority_queue);
    
    printf("\n--- Channel Tests ---\n");
    TEST(channel_create);
    TEST(channel_send_receive);
    TEST(channel_bidirectional);
    
    printf("\n--- Batch Operations ---\n");
    TEST(batch_send_receive);
    
    printf("\n--- Priority Tests ---\n");
    TEST(priority_ordering);
    
    printf("\n--- Utility Tests ---\n");
    TEST(message_types);
    TEST(priority_names);
    TEST(timestamp);
    TEST(message_print);
    
    printf("\n--- Statistics Tests ---\n");
    TEST(channel_statistics);
    TEST(system_statistics);
    
    printf("\n--- Stress Tests ---\n");
    TEST(stress_many_messages);
    
    // Summary
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                      TEST SUMMARY                          ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║  Total Tests:   %-2d                                         ║\n", tests_passed + tests_failed);
    printf("║  Passed:        %-2d  ✓                                      ║\n", tests_passed);
    printf("║  Failed:         %-2d  ✗                                      ║\n", tests_failed);
    printf("║  Success Rate: %3d%%                                        ║\n", 
           (tests_passed * 100) / (tests_passed + tests_failed));
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    return tests_failed > 0 ? 1 : 0;
}