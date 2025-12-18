/**
 * @file test_shared_memory_week2.c
 * @brief Week 2 Tests: Shared Memory + Rainbow Table Integration
 */

#include "../include/shared_memory.h"
#include "../include/shared_memory_rainbow.h"
#include "../include/shared_memory_enhanced.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

// Test counter
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
// TEST: Basic Shared Memory (Existing Functionality)
// ============================================================================

bool test_read_only_mode() {
    SharedMemoryRegion* region = shared_memory_create(1024, SHARED_READ_ONLY);
    assert(region != NULL);
    
    // Read access should work
    const void* data = shared_memory_read(region);
    assert(data != NULL);
    printf("Read-only region created: %p\n", data);
    
    shared_memory_release_read(region);
    
    // Write access should fail
    void* write_ptr = shared_memory_write(region);
    assert(write_ptr == NULL);
    printf("Write to read-only correctly rejected\n");
    
    shared_memory_free(region);
    return true;
}

bool test_copy_on_write_mode() {
    SharedMemoryRegion* region = shared_memory_create(1024, SHARED_COPY_ON_WRITE);
    assert(region != NULL);
    
    // Write some data
    void* write_ptr = shared_memory_write(region);
    assert(write_ptr != NULL);
    memset(write_ptr, 0xAB, 1024);
    shared_memory_release_write(region);
    printf("COW region written\n");
    
    // Read back
    const void* read_ptr = shared_memory_read(region);
    assert(read_ptr != NULL);
    assert(((unsigned char*)read_ptr)[0] == 0xAB);
    shared_memory_release_read(region);
    printf("COW region read back correctly\n");
    
    // Check statistics
    uint64_t reads, writes, copies;
    shared_memory_get_stats(region, &reads, &writes, &copies);
    printf("Stats: reads=%lu, writes=%lu, copies=%lu\n", reads, writes, copies);
    
    shared_memory_free(region);
    return true;
}

bool test_locked_write_mode() {
    SharedMemoryRegion* region = shared_memory_create(1024, SHARED_LOCKED_WRITE);
    assert(region != NULL);
    
    // Write some data
    void* write_ptr = shared_memory_write(region);
    assert(write_ptr != NULL);
    memset(write_ptr, 0xCD, 1024);
    shared_memory_release_write(region);
    printf("Locked write region written\n");
    
    // Read back
    const void* read_ptr = shared_memory_read(region);
    assert(read_ptr != NULL);
    assert(((unsigned char*)read_ptr)[0] == 0xCD);
    shared_memory_release_read(region);
    printf("Locked write region read back correctly\n");
    
    shared_memory_free(region);
    return true;
}

bool test_version_tracking() {
    SharedMemoryRegion* region = shared_memory_create(1024, SHARED_LOCKED_WRITE);
    assert(region != NULL);
    
    uint64_t v1 = shared_memory_get_version(region);
    printf("Initial version: %lu\n", v1);
    
    // Write should increment version
    void* write_ptr = shared_memory_write(region);
    assert(write_ptr != NULL);
    memset(write_ptr, 0xEF, 1024);
    shared_memory_release_write(region);
    
    uint64_t v2 = shared_memory_get_version(region);
    printf("After write version: %lu\n", v2);
    assert(v2 > v1);
    
    // Check if modified
    assert(shared_memory_is_modified(region, v1) == true);
    assert(shared_memory_is_modified(region, v2) == false);
    printf("Version tracking working correctly\n");
    
    shared_memory_free(region);
    return true;
}

bool test_resize() {
    SharedMemoryRegion* region = shared_memory_create(1024, SHARED_LOCKED_WRITE);
    assert(region != NULL);
    
    printf("Initial size: %zu\n", region->size);
    
    // Resize to 2048
    int result = shared_memory_resize(region, 2048);
    assert(result == 0);
    assert(region->size == 2048);
    printf("Resized to: %zu\n", region->size);
    
    shared_memory_free(region);
    return true;
}

// ============================================================================
// TEST: Rainbow Table Integration
// ============================================================================

bool test_rainbow_table_basic() {
    SharedMemoryRainbowTable* table = shared_memory_rainbow_create(16, 12);
    assert(table != NULL);
    printf("Created rainbow table with capacity 16\n");
    
    // Create some regions
    SharedMemoryRegion* r1 = shared_memory_create(1024, SHARED_READ_ONLY);
    SharedMemoryRegion* r2 = shared_memory_create(2048, SHARED_COPY_ON_WRITE);
    SharedMemoryRegion* r3 = shared_memory_create(4096, SHARED_LOCKED_WRITE);
    
    // Register regions
    assert(shared_memory_rainbow_register(table, r1, 1) == 0);
    assert(shared_memory_rainbow_register(table, r2, 2) == 0);
    assert(shared_memory_rainbow_register(table, r3, 3) == 0);
    printf("Registered 3 regions\n");
    
    // Lookup regions
    SharedMemoryRegion* found1 = shared_memory_rainbow_lookup(table, 1);
    assert(found1 == r1);
    printf("Lookup region 1: OK\n");
    
    SharedMemoryRegion* found2 = shared_memory_rainbow_lookup(table, 2);
    assert(found2 == r2);
    printf("Lookup region 2: OK\n");
    
    // Check existence
    assert(shared_memory_rainbow_exists(table, 1) == true);
    assert(shared_memory_rainbow_exists(table, 99) == false);
    printf("Existence checks: OK\n");
    
    // Get stats
    size_t num_regions, capacity;
    double load_factor;
    shared_memory_rainbow_get_stats(table, &num_regions, &capacity, &load_factor);
    printf("Stats: %zu regions, capacity %zu, load %.2f%%\n",
           num_regions, capacity, load_factor * 100.0);
    
    // Cleanup
    shared_memory_free(r1);
    shared_memory_free(r2);
    shared_memory_free(r3);
    shared_memory_rainbow_free(table);
    
    return true;
}

bool test_rainbow_table_many_regions() {
    SharedMemoryRainbowTable* table = shared_memory_rainbow_create(32, 12);
    assert(table != NULL);
    
    // Create and register 20 regions
    SharedMemoryRegion* regions[20];
    for (int i = 0; i < 20; i++) {
        regions[i] = shared_memory_create(1024, SHARED_LOCKED_WRITE);
        assert(regions[i] != NULL);
        assert(shared_memory_rainbow_register(table, regions[i], i) == 0);
    }
    printf("Registered 20 regions\n");
    
    // Lookup all regions
    for (int i = 0; i < 20; i++) {
        SharedMemoryRegion* found = shared_memory_rainbow_lookup(table, i);
        assert(found == regions[i]);
    }
    printf("All 20 regions found via O(1) lookup\n");
    
    // Get all IDs
    uint64_t ids[20];
    size_t count = shared_memory_rainbow_get_all_ids(table, ids, 20);
    printf("Retrieved %zu region IDs\n", count);
    
    // Cleanup
    for (int i = 0; i < 20; i++) {
        shared_memory_free(regions[i]);
    }
    shared_memory_rainbow_free(table);
    
    return true;
}

// ============================================================================
// TEST: Enhanced Features
// ============================================================================

bool test_enhanced_basic() {
    SharedMemoryEnhanced* region = shared_memory_enhanced_create(1024, SHARED_LOCKED_WRITE, 42);
    assert(region != NULL);
    
    printf("Created enhanced region with ID: %lu\n", 
           shared_memory_enhanced_get_id(region));
    assert(shared_memory_enhanced_get_id(region) == 42);
    
    shared_memory_enhanced_free(region);
    return true;
}

bool test_version_history() {
    SharedMemoryEnhanced* region = shared_memory_enhanced_create(1024, SHARED_LOCKED_WRITE, 1);
    assert(region != NULL);
    
    // Enable version history
    assert(shared_memory_enhanced_enable_history(region, 10) == 0);
    printf("Enabled version history (capacity: 10)\n");
    
    // Make some writes to create version history
    for (int i = 0; i < 5; i++) {
        void* ptr = shared_memory_write(&region->base);
        assert(ptr != NULL);
        memset(ptr, i, 1024);
        
        uint64_t old_version = atomic_load(&region->base.version);
        shared_memory_release_write(&region->base);
        uint64_t new_version = atomic_load(&region->base.version);
        
        // Trigger invalidation manually
        shared_memory_enhanced_trigger_invalidation(region, old_version, new_version);
    }
    printf("Made 5 writes\n");
    
    // Get version history
    SharedMemoryVersionEntry history[10];
    size_t count = shared_memory_enhanced_get_history(region, history, 10);
    printf("Version history entries: %zu\n", count);
    
    for (size_t i = 0; i < count; i++) {
        printf("  Version %lu: size=%zu, timestamp=%lu\n",
               history[i].version, history[i].size, history[i].timestamp);
    }
    
    shared_memory_enhanced_free(region);
    return true;
}

// Callback for testing
static int callback_count = 0;
static void test_callback(void* user_data, uint64_t region_id, 
                         uint64_t old_version, uint64_t new_version) {
    callback_count++;
    printf("  Callback: region=%lu, %lu -> %lu\n", region_id, old_version, new_version);
}

bool test_invalidation_callback() {
    SharedMemoryEnhanced* region = shared_memory_enhanced_create(1024, SHARED_LOCKED_WRITE, 5);
    assert(region != NULL);
    
    // Set callback
    callback_count = 0;
    shared_memory_enhanced_set_callback(region, test_callback, NULL);
    printf("Set invalidation callback\n");
    
    // Make some writes
    for (int i = 0; i < 3; i++) {
        void* ptr = shared_memory_write(&region->base);
        assert(ptr != NULL);
        
        uint64_t old_version = atomic_load(&region->base.version);
        shared_memory_release_write(&region->base);
        uint64_t new_version = atomic_load(&region->base.version);
        
        shared_memory_enhanced_trigger_invalidation(region, old_version, new_version);
    }
    
    printf("Callback invoked %d times\n", callback_count);
    assert(callback_count == 3);
    
    shared_memory_enhanced_free(region);
    return true;
}

bool test_enhanced_statistics() {
    SharedMemoryEnhanced* region = shared_memory_enhanced_create(1024, SHARED_LOCKED_WRITE, 10);
    assert(region != NULL);
    
    // Enable history
    shared_memory_enhanced_enable_history(region, 5);
    
    // Make some operations
    for (int i = 0; i < 3; i++) {
        void* ptr = shared_memory_write(&region->base);
        uint64_t old_v = atomic_load(&region->base.version);
        shared_memory_release_write(&region->base);
        uint64_t new_v = atomic_load(&region->base.version);
        shared_memory_enhanced_trigger_invalidation(region, old_v, new_v);
    }
    
    // Get stats
    uint64_t reads, writes, copies, invalidations;
    size_t history_size;
    shared_memory_enhanced_get_stats(region, &reads, &writes, &copies, 
                                    &invalidations, &history_size);
    
    printf("Enhanced stats:\n");
    printf("  Reads: %lu\n", reads);
    printf("  Writes: %lu\n", writes);
    printf("  Copies: %lu\n", copies);
    printf("  Invalidations: %lu\n", invalidations);
    printf("  History size: %zu\n", history_size);
    
    assert(writes == 3);
    assert(invalidations == 3);
    
    shared_memory_enhanced_free(region);
    return true;
}

// ============================================================================
// TEST: Integration Tests
// ============================================================================

bool test_rainbow_with_enhanced() {
    SharedMemoryRainbowTable* table = shared_memory_rainbow_create(16, 12);
    assert(table != NULL);
    
    // Create enhanced regions
    SharedMemoryEnhanced* r1 = shared_memory_enhanced_create(1024, SHARED_READ_ONLY, 1);
    SharedMemoryEnhanced* r2 = shared_memory_enhanced_create(2048, SHARED_COPY_ON_WRITE, 2);
    SharedMemoryEnhanced* r3 = shared_memory_enhanced_create(4096, SHARED_LOCKED_WRITE, 3);
    
    // Register in rainbow table
    assert(shared_memory_rainbow_register(table, &r1->base, 1) == 0);
    assert(shared_memory_rainbow_register(table, &r2->base, 2) == 0);
    assert(shared_memory_rainbow_register(table, &r3->base, 3) == 0);
    printf("Registered 3 enhanced regions in rainbow table\n");
    
    // Lookup and verify
    SharedMemoryRegion* found = shared_memory_rainbow_lookup(table, 1);
    assert(found == &r1->base);
    printf("O(1) lookup of enhanced region: OK\n");
    
    // Print info
    shared_memory_rainbow_print_info(table);
    
    // Cleanup
    shared_memory_enhanced_free(r1);
    shared_memory_enhanced_free(r2);
    shared_memory_enhanced_free(r3);
    shared_memory_rainbow_free(table);
    
    return true;
}

bool test_cache_coherency() {
    SharedMemoryEnhanced* region = shared_memory_enhanced_create(1024, SHARED_LOCKED_WRITE, 1);
    assert(region != NULL);
    
    // Enable version history
    shared_memory_enhanced_enable_history(region, 10);
    
    // Simulate cache coherency protocol
    uint64_t cached_version = shared_memory_get_version(&region->base);
    printf("Cached version: %lu\n", cached_version);
    
    // Another thread modifies the region
    void* ptr = shared_memory_write(&region->base);
    memset(ptr, 0xFF, 1024);
    uint64_t old_v = atomic_load(&region->base.version);
    shared_memory_release_write(&region->base);
    uint64_t new_v = atomic_load(&region->base.version);
    shared_memory_enhanced_trigger_invalidation(region, old_v, new_v);
    
    // Check if cache is stale
    bool is_stale = shared_memory_is_modified(&region->base, cached_version);
    assert(is_stale == true);
    printf("Cache staleness detected: OK\n");
    
    // Update cached version
    cached_version = shared_memory_get_version(&region->base);
    printf("Updated cached version: %lu\n", cached_version);
    
    // Verify not stale anymore
    assert(shared_memory_is_modified(&region->base, cached_version) == false);
    printf("Cache coherency verified\n");
    
    shared_memory_enhanced_free(region);
    return true;
}

bool test_concurrent_access() {
    SharedMemoryRegion* region = shared_memory_create(1024, SHARED_LOCKED_WRITE);
    assert(region != NULL);
    
    // Simulate multiple readers
    const void* r1 = shared_memory_read(region);
    const void* r2 = shared_memory_read(region);
    const void* r3 = shared_memory_read(region);
    
    assert(r1 != NULL && r2 != NULL && r3 != NULL);
    printf("3 concurrent readers: OK\n");
    
    shared_memory_release_read(region);
    shared_memory_release_read(region);
    shared_memory_release_read(region);
    
    // Verify reader count
    assert(atomic_load(&region->num_readers) == 0);
    printf("All readers released: OK\n");
    
    shared_memory_free(region);
    return true;
}

bool test_validation() {
    SharedMemoryRegion* region = shared_memory_create(1024, SHARED_LOCKED_WRITE);
    assert(region != NULL);
    
    bool valid = shared_memory_validate(region);
    assert(valid == true);
    printf("Region validation: VALID\n");
    
    shared_memory_free(region);
    return true;
}

// ============================================================================
// TEST: Stress Tests
// ============================================================================

bool test_stress_rainbow() {
    SharedMemoryRainbowTable* table = shared_memory_rainbow_create(128, 12);
    assert(table != NULL);
    
    printf("Stress testing with 100 regions...\n");
    
    // Create and register 100 regions
    SharedMemoryEnhanced* regions[100];
    for (int i = 0; i < 100; i++) {
        regions[i] = shared_memory_enhanced_create(1024, SHARED_LOCKED_WRITE, i);
        assert(regions[i] != NULL);
        assert(shared_memory_rainbow_register(table, &regions[i]->base, i) == 0);
    }
    printf("Registered 100 regions\n");
    
    // Lookup all regions
    for (int i = 0; i < 100; i++) {
        SharedMemoryRegion* found = shared_memory_rainbow_lookup(table, i);
        assert(found == &regions[i]->base);
    }
    printf("All 100 regions found via O(1) lookup\n");
    
    // Get stats
    size_t num_regions, capacity;
    double load_factor;
    shared_memory_rainbow_get_stats(table, &num_regions, &capacity, &load_factor);
    printf("Final stats: %zu regions, capacity %zu, load %.2f%%\n",
           num_regions, capacity, load_factor * 100.0);
    
    // Cleanup
    for (int i = 0; i < 100; i++) {
        shared_memory_enhanced_free(regions[i]);
    }
    shared_memory_rainbow_free(table);
    
    return true;
}

bool test_stress_version_history() {
    SharedMemoryEnhanced* region = shared_memory_enhanced_create(1024, SHARED_LOCKED_WRITE, 1);
    assert(region != NULL);
    
    // Enable version history
    shared_memory_enhanced_enable_history(region, 50);
    
    printf("Making 100 writes with version tracking...\n");
    
    // Make 100 writes
    for (int i = 0; i < 100; i++) {
        void* ptr = shared_memory_write(&region->base);
        assert(ptr != NULL);
        memset(ptr, i % 256, 1024);
        
        uint64_t old_v = atomic_load(&region->base.version);
        shared_memory_release_write(&region->base);
        uint64_t new_v = atomic_load(&region->base.version);
        
        shared_memory_enhanced_trigger_invalidation(region, old_v, new_v);
    }
    
    // Get stats
    uint64_t reads, writes, copies, invalidations;
    size_t history_size;
    shared_memory_enhanced_get_stats(region, &reads, &writes, &copies,
                                    &invalidations, &history_size);
    
    printf("Stats after 100 writes:\n");
    printf("  Writes: %lu\n", writes);
    printf("  Invalidations: %lu\n", invalidations);
    printf("  History size: %zu (capacity: 50)\n", history_size);
    
    assert(writes == 100);
    assert(invalidations == 100);
    assert(history_size == 50); // Circular buffer, max 50
    
    shared_memory_enhanced_free(region);
    return true;
}

// ============================================================================
// MAIN
// ============================================================================

int main() {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║   WEEK 2: SHARED MEMORY + RAINBOW TABLE INTEGRATION        ║\n");
    printf("║   Comprehensive Test Suite                                ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");

    printf("\n--- Basic Shared Memory Tests ---\n");
    TEST(read_only_mode);
    TEST(copy_on_write_mode);
    TEST(locked_write_mode);
    TEST(version_tracking);
    TEST(resize);
    TEST(concurrent_access);
    TEST(validation);

    printf("\n--- Rainbow Table Integration Tests ---\n");
    TEST(rainbow_table_basic);
    TEST(rainbow_table_many_regions);

    printf("\n--- Enhanced Features Tests ---\n");
    TEST(enhanced_basic);
    TEST(version_history);
    TEST(invalidation_callback);
    TEST(enhanced_statistics);

    printf("\n--- Integration Tests ---\n");
    TEST(rainbow_with_enhanced);

    printf("\n--- Stress Tests ---\n");
    TEST(stress_rainbow);
    TEST(stress_version_history);

    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                      TEST SUMMARY                          ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║  Total Tests:   %2d                                         ║\n", tests_passed + tests_failed);
    printf("║  Passed:        %2d  ✓                                      ║\n", tests_passed);
    printf("║  Failed:        %2d  ✗                                      ║\n", tests_failed);
    printf("║  Success Rate: %3d%%                                        ║\n", 
           (tests_passed * 100) / (tests_passed + tests_failed));
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");

    return tests_failed > 0 ? 1 : 0;
}