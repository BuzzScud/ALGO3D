#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include "ai/cllm_thread_pool.h"

// Test thread pool creation
void test_thread_pool_create() {
    printf("Testing thread_pool_create...\n");
    
    // Test with valid capacity
    ThreadPool* pool = thread_pool_create(1000, false);
    assert(pool != NULL);
    assert(pool->capacity == 1000);
    assert(thread_pool_get_active_count(pool) == 0);
    thread_pool_destroy(pool);
    
    // Test with MAX_THREADS
    pool = thread_pool_create(MAX_THREADS, false);
    assert(pool != NULL);
    assert(pool->capacity == MAX_THREADS);
    thread_pool_destroy(pool);
    
    // Test with capacity exceeding MAX_THREADS (should cap)
    pool = thread_pool_create(MAX_THREADS + 1000, false);
    assert(pool != NULL);
    assert(pool->capacity == MAX_THREADS);
    thread_pool_destroy(pool);
    
    // Test with zero capacity (should fail)
    pool = thread_pool_create(0, false);
    assert(pool == NULL);
    
    printf("✓ thread_pool_create passed\n");
}

// Test thread pool with reuse enabled
void test_thread_pool_create_with_reuse() {
    printf("Testing thread_pool_create with reuse...\n");
    
    ThreadPool* pool = thread_pool_create(1000, true);
    assert(pool != NULL);
    assert(pool->enable_reuse == true);
    thread_pool_destroy(pool);
    
    printf("✓ thread_pool_create with reuse passed\n");
}

// Test thread registration
void test_thread_registration() {
    printf("Testing thread registration...\n");
    
    ThreadPool* pool = thread_pool_create(100, false);
    assert(pool != NULL);
    
    // Register a thread
    pthread_t tid = pthread_self();
    ThreadEntry* entry = thread_pool_register(pool, tid, 1, 0, 0);
    assert(entry != NULL);
    assert(entry->sphere_id == 1);
    assert(entry->hierarchy_level == 0);
    assert(entry->symmetry_group == 0);
    assert(entry->active == true);
    assert(thread_pool_get_active_count(pool) == 1);
    
    // Unregister the thread
    int result = thread_pool_unregister(pool, tid);
    assert(result == 1);
    assert(thread_pool_get_active_count(pool) == 0);
    
    thread_pool_destroy(pool);
    
    printf("✓ thread registration passed\n");
}

// Test multiple thread registrations
void test_multiple_registrations() {
    printf("Testing multiple thread registrations...\n");
    
    ThreadPool* pool = thread_pool_create(100, false);
    assert(pool != NULL);
    
    // Register multiple threads (using dummy thread IDs)
    pthread_t tids[10];
    for (int i = 0; i < 10; i++) {
        tids[i] = (pthread_t)(uintptr_t)(i + 1);
        ThreadEntry* entry = thread_pool_register(pool, tids[i], i, i % 5, i % 12);
        assert(entry != NULL);
    }
    
    assert(thread_pool_get_active_count(pool) == 10);
    
    // Unregister all threads
    for (int i = 0; i < 10; i++) {
        int result = thread_pool_unregister(pool, tids[i]);
        assert(result == 1);
    }
    
    assert(thread_pool_get_active_count(pool) == 0);
    
    thread_pool_destroy(pool);
    
    printf("✓ multiple registrations passed\n");
}

// Test pool capacity limit
void test_pool_capacity_limit() {
    printf("Testing pool capacity limit...\n");
    
    ThreadPool* pool = thread_pool_create(10, false);
    assert(pool != NULL);
    
    // Fill the pool
    pthread_t tids[10];
    for (int i = 0; i < 10; i++) {
        tids[i] = (pthread_t)(uintptr_t)(i + 1);
        ThreadEntry* entry = thread_pool_register(pool, tids[i], i, 0, 0);
        assert(entry != NULL);
    }
    
    assert(thread_pool_get_active_count(pool) == 10);
    
    // Try to register one more (should fail)
    pthread_t extra_tid = (pthread_t)(uintptr_t)100;
    ThreadEntry* entry = thread_pool_register(pool, extra_tid, 100, 0, 0);
    assert(entry == NULL);
    assert(thread_pool_get_active_count(pool) == 10);
    
    // Unregister all
    for (int i = 0; i < 10; i++) {
        thread_pool_unregister(pool, tids[i]);
    }
    
    thread_pool_destroy(pool);
    
    printf("✓ pool capacity limit passed\n");
}

// Test can_spawn checking
void test_can_spawn() {
    printf("Testing thread_pool_can_spawn...\n");
    
    ThreadPool* pool = thread_pool_create(100, false);
    assert(pool != NULL);
    
    // Initially should be able to spawn
    assert(thread_pool_can_spawn(pool, 12) == true);
    assert(thread_pool_can_spawn(pool, 100) == true);
    assert(thread_pool_can_spawn(pool, 101) == false);
    
    // Register 90 threads
    pthread_t tids[90];
    for (int i = 0; i < 90; i++) {
        tids[i] = (pthread_t)(uintptr_t)(i + 1);
        thread_pool_register(pool, tids[i], i, 0, 0);
    }
    
    // Now we have 10 slots left
    assert(thread_pool_can_spawn(pool, 10) == true);
    assert(thread_pool_can_spawn(pool, 11) == false);
    assert(thread_pool_can_spawn(pool, 12) == false);
    
    // Unregister all
    for (int i = 0; i < 90; i++) {
        thread_pool_unregister(pool, tids[i]);
    }
    
    thread_pool_destroy(pool);
    
    printf("✓ thread_pool_can_spawn passed\n");
}

// Test reserve and release
void test_reserve_release() {
    printf("Testing thread_pool_reserve and release...\n");
    
    ThreadPool* pool = thread_pool_create(100, false);
    assert(pool != NULL);
    
    // Reserve 12 slots
    bool result = thread_pool_reserve(pool, 12);
    assert(result == true);
    assert(thread_pool_get_active_count(pool) == 12);
    
    // Try to reserve 90 more (should succeed)
    result = thread_pool_reserve(pool, 88);
    assert(result == true);
    assert(thread_pool_get_active_count(pool) == 100);
    
    // Try to reserve 1 more (should fail - pool full)
    result = thread_pool_reserve(pool, 1);
    assert(result == false);
    assert(thread_pool_get_active_count(pool) == 100);
    
    // Release 50 slots
    thread_pool_release(pool, 50);
    assert(thread_pool_get_active_count(pool) == 50);
    
    // Now we can reserve again
    result = thread_pool_reserve(pool, 50);
    assert(result == true);
    assert(thread_pool_get_active_count(pool) == 100);
    
    thread_pool_destroy(pool);
    
    printf("✓ thread_pool_reserve and release passed\n");
}

// Test thread finding
void test_thread_find() {
    printf("Testing thread_pool_find...\n");
    
    ThreadPool* pool = thread_pool_create(100, false);
    assert(pool != NULL);
    
    // Register a thread
    pthread_t tid = (pthread_t)(uintptr_t)42;
    ThreadEntry* entry = thread_pool_register(pool, tid, 1, 2, 3);
    assert(entry != NULL);
    
    // Find the thread
    ThreadEntry* found = thread_pool_find(pool, tid);
    assert(found != NULL);
    assert(found == entry);
    assert(found->sphere_id == 1);
    assert(found->hierarchy_level == 2);
    assert(found->symmetry_group == 3);
    
    // Try to find non-existent thread
    pthread_t fake_tid = (pthread_t)(uintptr_t)999;
    found = thread_pool_find(pool, fake_tid);
    assert(found == NULL);
    
    thread_pool_unregister(pool, tid);
    thread_pool_destroy(pool);
    
    printf("✓ thread_pool_find passed\n");
}

// Test statistics
void test_statistics() {
    printf("Testing thread pool statistics...\n");
    
    ThreadPool* pool = thread_pool_create(100, false);
    assert(pool != NULL);
    
    // Initial stats should be zero
    ThreadPoolStats stats;
    thread_pool_get_stats(pool, &stats);
    assert(stats.threads_created == 0);
    assert(stats.threads_destroyed == 0);
    assert(stats.peak_thread_count == 0);
    
    // Register some threads
    pthread_t tids[10];
    for (int i = 0; i < 10; i++) {
        tids[i] = (pthread_t)(uintptr_t)(i + 1);
        thread_pool_register(pool, tids[i], i, 0, 0);
    }
    
    // Check stats
    thread_pool_get_stats(pool, &stats);
    assert(stats.threads_created == 10);
    assert(stats.threads_destroyed == 0);
    assert(stats.peak_thread_count == 10);
    
    // Unregister some threads
    for (int i = 0; i < 5; i++) {
        thread_pool_unregister(pool, tids[i]);
    }
    
    // Check stats again
    thread_pool_get_stats(pool, &stats);
    assert(stats.threads_created == 10);
    assert(stats.threads_destroyed == 5);
    assert(stats.peak_thread_count == 10);
    
    // Reset stats
    thread_pool_reset_stats(pool);
    thread_pool_get_stats(pool, &stats);
    assert(stats.threads_created == 0);
    assert(stats.threads_destroyed == 0);
    assert(stats.peak_thread_count == 0);
    
    // Unregister remaining threads
    for (int i = 5; i < 10; i++) {
        thread_pool_unregister(pool, tids[i]);
    }
    
    thread_pool_destroy(pool);
    
    printf("✓ thread pool statistics passed\n");
}

// Test spawn request tracking
void test_spawn_request_tracking() {
    printf("Testing spawn request tracking...\n");
    
    ThreadPool* pool = thread_pool_create(100, false);
    assert(pool != NULL);
    
    // Make some successful reservations
    thread_pool_reserve(pool, 10);
    thread_pool_reserve(pool, 20);
    
    ThreadPoolStats stats;
    thread_pool_get_stats(pool, &stats);
    assert(stats.spawn_requests == 2);
    assert(stats.spawn_rejections == 0);
    
    // Make a reservation that will fail
    thread_pool_reserve(pool, 100);  // Only 70 slots left
    
    thread_pool_get_stats(pool, &stats);
    assert(stats.spawn_requests == 3);
    assert(stats.spawn_rejections == 1);
    
    thread_pool_destroy(pool);
    
    printf("✓ spawn request tracking passed\n");
}

// Test thread reuse
void test_thread_reuse() {
    printf("Testing thread reuse...\n");
    
    ThreadPool* pool = thread_pool_create(100, true);
    assert(pool != NULL);
    assert(pool->enable_reuse == true);
    
    // Register and unregister a thread
    pthread_t tid = (pthread_t)(uintptr_t)1;
    ThreadEntry* entry1 = thread_pool_register(pool, tid, 1, 0, 0);
    assert(entry1 != NULL);
    
    thread_pool_unregister(pool, tid);
    
    // Free list should have 1 entry
    size_t free_count = atomic_load(&pool->free_count);
    assert(free_count == 1);
    
    // Register another thread (should reuse)
    tid = (pthread_t)(uintptr_t)2;
    ThreadEntry* entry2 = thread_pool_register(pool, tid, 2, 0, 0);
    assert(entry2 != NULL);
    
    // Check reuse stats
    ThreadPoolStats stats;
    thread_pool_get_stats(pool, &stats);
    assert(stats.reuse_count == 1);
    
    thread_pool_unregister(pool, tid);
    thread_pool_destroy(pool);
    
    printf("✓ thread reuse passed\n");
}

// Test available slots calculation
void test_available_slots() {
    printf("Testing thread_pool_get_available...\n");
    
    ThreadPool* pool = thread_pool_create(100, false);
    assert(pool != NULL);
    
    // Initially all slots available
    assert(thread_pool_get_available(pool) == 100);
    
    // Register 30 threads
    pthread_t tids[30];
    for (int i = 0; i < 30; i++) {
        tids[i] = (pthread_t)(uintptr_t)(i + 1);
        thread_pool_register(pool, tids[i], i, 0, 0);
    }
    
    // 70 slots should be available
    assert(thread_pool_get_available(pool) == 70);
    
    // Fill the pool
    for (int i = 30; i < 100; i++) {
        tids[i % 30] = (pthread_t)(uintptr_t)(i + 1);
        thread_pool_register(pool, tids[i % 30], i, 0, 0);
    }
    
    // No slots available
    assert(thread_pool_get_available(pool) == 0);
    
    thread_pool_destroy(pool);
    
    printf("✓ thread_pool_get_available passed\n");
}

// Test validation
void test_validation() {
    printf("Testing thread_pool_validate...\n");
    
    ThreadPool* pool = thread_pool_create(100, false);
    assert(pool != NULL);
    
    // Valid pool should pass validation
    assert(thread_pool_validate(pool) == 1);
    
    // NULL pool should fail
    assert(thread_pool_validate(NULL) == 0);
    
    thread_pool_destroy(pool);
    
    printf("✓ thread_pool_validate passed\n");
}

// Test MAX_THREADS enforcement
void test_max_threads_enforcement() {
    printf("Testing MAX_THREADS enforcement...\n");
    
    // Create pool with MAX_THREADS capacity
    ThreadPool* pool = thread_pool_create(MAX_THREADS, false);
    assert(pool != NULL);
    assert(pool->capacity == MAX_THREADS);
    
    // Should be able to reserve up to MAX_THREADS
    assert(thread_pool_can_spawn(pool, MAX_THREADS) == true);
    assert(thread_pool_can_spawn(pool, MAX_THREADS + 1) == false);
    
    thread_pool_destroy(pool);
    
    printf("✓ MAX_THREADS enforcement passed\n");
}

// Test peak thread count tracking
void test_peak_thread_count() {
    printf("Testing peak thread count tracking...\n");
    
    ThreadPool* pool = thread_pool_create(100, false);
    assert(pool != NULL);
    
    // Register 50 threads
    pthread_t tids[50];
    for (int i = 0; i < 50; i++) {
        tids[i] = (pthread_t)(uintptr_t)(i + 1);
        thread_pool_register(pool, tids[i], i, 0, 0);
    }
    
    ThreadPoolStats stats;
    thread_pool_get_stats(pool, &stats);
    assert(stats.peak_thread_count == 50);
    
    // Unregister 25 threads
    for (int i = 0; i < 25; i++) {
        thread_pool_unregister(pool, tids[i]);
    }
    
    // Peak should still be 50
    thread_pool_get_stats(pool, &stats);
    assert(stats.peak_thread_count == 50);
    
    // Register 40 more threads (total 65)
    for (int i = 0; i < 40; i++) {
        tids[i] = (pthread_t)(uintptr_t)(i + 100);
        thread_pool_register(pool, tids[i], i, 0, 0);
    }
    
    // Peak should now be 65
    thread_pool_get_stats(pool, &stats);
    assert(stats.peak_thread_count == 65);
    
    thread_pool_destroy(pool);
    
    printf("✓ peak thread count tracking passed\n");
}

int main() {
    printf("=== Thread Pool Tests ===\n\n");
    
    test_thread_pool_create();
    test_thread_pool_create_with_reuse();
    test_thread_registration();
    test_multiple_registrations();
    test_pool_capacity_limit();
    test_can_spawn();
    test_reserve_release();
    test_thread_find();
    test_statistics();
    test_spawn_request_tracking();
    test_thread_reuse();
    test_available_slots();
    test_validation();
    test_max_threads_enforcement();
    test_peak_thread_count();
    
    printf("\n=== All Thread Pool Tests Passed! ===\n");
    return 0;
}