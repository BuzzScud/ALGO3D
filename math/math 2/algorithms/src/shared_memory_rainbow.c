/**
 * @file shared_memory_rainbow.c
 * @brief Rainbow Table Integration for Shared Memory - Implementation
 */

#include "shared_memory_rainbow.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
 * Create shared memory rainbow table
 */
SharedMemoryRainbowTable* shared_memory_rainbow_create(size_t initial_capacity, uint32_t base) {
    if (initial_capacity == 0 || base < 2) {
        fprintf(stderr, "Error: invalid parameters\n");
        return NULL;
    }
    
    SharedMemoryRainbowTable* table = (SharedMemoryRainbowTable*)calloc(1, sizeof(SharedMemoryRainbowTable));
    if (!table) {
        fprintf(stderr, "Error: failed to allocate rainbow table\n");
        return NULL;
    }
    
    // Create underlying rainbow table
    table->rainbow = (RainbowTable*)calloc(1, sizeof(RainbowTable));
    if (!table->rainbow) {
        fprintf(stderr, "Error: failed to allocate rainbow table\n");
        free(table);
        return NULL;
    }
    
    MathError err = rainbow_init(table->rainbow, initial_capacity);
    if (err != MATH_SUCCESS) {
        fprintf(stderr, "Error: failed to initialize rainbow table\n");
        free(table->rainbow);
        free(table);
        return NULL;
    }
    
    // Allocate regions array
    table->regions = (SharedMemoryRegion**)calloc(initial_capacity, sizeof(SharedMemoryRegion*));
    if (!table->regions) {
        fprintf(stderr, "Error: failed to allocate regions array\n");
        rainbow_cleanup(table->rainbow);
        free(table);
        return NULL;
    }
    
    table->num_regions = 0;
    table->capacity = initial_capacity;
    
    // Initialize lock
    if (pthread_rwlock_init(&table->lock, NULL) != 0) {
        fprintf(stderr, "Error: failed to initialize rwlock\n");
        free(table->regions);
        rainbow_cleanup(table->rainbow);
        free(table);
        return NULL;
    }
    
    return table;
}

/**
 * Free shared memory rainbow table
 */
void shared_memory_rainbow_free(SharedMemoryRainbowTable* table) {
    if (!table) {
        return;
    }
    
    pthread_rwlock_destroy(&table->lock);
    
    if (table->regions) {
        free(table->regions);
    }
    
    if (table->rainbow) {
        rainbow_cleanup(table->rainbow);
    }
    
    free(table);
}

/**
 * Register shared memory region
 */
int shared_memory_rainbow_register(SharedMemoryRainbowTable* table,
                                   SharedMemoryRegion* region,
                                   uint64_t region_id) {
    if (!table || !region) {
        return -1;
    }
    
    // Acquire write lock
    pthread_rwlock_wrlock(&table->lock);
    
    // Check if we need to resize
    if (table->num_regions >= table->capacity) {
        size_t new_capacity = table->capacity * 2;
        SharedMemoryRegion** new_regions = (SharedMemoryRegion**)realloc(
            table->regions,
            new_capacity * sizeof(SharedMemoryRegion*)
        );
        
        if (!new_regions) {
            pthread_rwlock_unlock(&table->lock);
            fprintf(stderr, "Error: failed to resize regions array\n");
            return -1;
        }
        
        // Zero new memory
        memset(new_regions + table->capacity, 0, 
               (new_capacity - table->capacity) * sizeof(SharedMemoryRegion*));
        
        table->regions = new_regions;
        table->capacity = new_capacity;
    }
    
    // Store region directly at region_id index (simple hash)
    if (region_id >= table->capacity) {
        pthread_rwlock_unlock(&table->lock);
        fprintf(stderr, "Error: region_id %lu exceeds capacity %zu\n", region_id, table->capacity);
        return -1;
    }
    
    // Check if slot is already occupied
    if (table->regions[region_id] != NULL) {
        pthread_rwlock_unlock(&table->lock);
        fprintf(stderr, "Error: region_id %lu already registered\n", region_id);
        return -1;
    }
    
    // Store region
    table->regions[region_id] = region;
    table->num_regions++;
    
    pthread_rwlock_unlock(&table->lock);
    
    return 0;
}

/**
 * Unregister shared memory region
 */
int shared_memory_rainbow_unregister(SharedMemoryRainbowTable* table,
                                     uint64_t region_id) {
    if (!table) {
        return -1;
    }
    
    // Acquire write lock
    pthread_rwlock_wrlock(&table->lock);
    
    // Find region
    size_t slot = region_id % table->capacity;
    size_t attempts = 0;
    
    while (table->regions[slot] != NULL && attempts < table->capacity) {
        // Check if this is the region we're looking for
        // (We'd need to store region_id in SharedMemoryRegion for this to work properly)
        // For now, just remove from slot
        if (slot == region_id % table->capacity) {
            table->regions[slot] = NULL;
            table->num_regions--;
            pthread_rwlock_unlock(&table->lock);
            return 0;
        }
        
        slot = (slot + 1) % table->capacity;
        attempts++;
    }
    
    pthread_rwlock_unlock(&table->lock);
    
    return -1; // Not found
}

/**
 * Lookup shared memory region by ID (O(1))
 */
SharedMemoryRegion* shared_memory_rainbow_lookup(const SharedMemoryRainbowTable* table,
                                                 uint64_t region_id) {
    if (!table) {
        return NULL;
    }
    
    // Acquire read lock
    pthread_rwlock_rdlock((pthread_rwlock_t*)&table->lock);
    
    // Direct lookup by region_id
    if (region_id >= table->capacity) {
        pthread_rwlock_unlock((pthread_rwlock_t*)&table->lock);
        return NULL;
    }
    
    SharedMemoryRegion* region = table->regions[region_id];
    
    pthread_rwlock_unlock((pthread_rwlock_t*)&table->lock);
    
    return region;
}

/**
 * Check if region exists
 */
bool shared_memory_rainbow_exists(const SharedMemoryRainbowTable* table,
                                  uint64_t region_id) {
    return shared_memory_rainbow_lookup(table, region_id) != NULL;
}

/**
 * Get all registered region IDs
 */
size_t shared_memory_rainbow_get_all_ids(const SharedMemoryRainbowTable* table,
                                        uint64_t* out_ids,
                                        size_t max_ids) {
    if (!table || !out_ids || max_ids == 0) {
        return 0;
    }
    
    // Acquire read lock
    pthread_rwlock_rdlock((pthread_rwlock_t*)&table->lock);
    
    size_t count = 0;
    for (size_t i = 0; i < table->capacity && count < max_ids; i++) {
        if (table->regions[i] != NULL) {
            out_ids[count++] = i; // Use slot as ID for now
        }
    }
    
    pthread_rwlock_unlock((pthread_rwlock_t*)&table->lock);
    
    return count;
}

/**
 * Get statistics
 */
void shared_memory_rainbow_get_stats(const SharedMemoryRainbowTable* table,
                                    size_t* out_num_regions,
                                    size_t* out_capacity,
                                    double* out_load_factor) {
    if (!table) {
        return;
    }
    
    if (out_num_regions) {
        *out_num_regions = table->num_regions;
    }
    
    if (out_capacity) {
        *out_capacity = table->capacity;
    }
    
    if (out_load_factor) {
        *out_load_factor = (double)table->num_regions / (double)table->capacity;
    }
}

/**
 * Print info
 */
void shared_memory_rainbow_print_info(const SharedMemoryRainbowTable* table) {
    if (!table) {
        return;
    }
    
    printf("=== Shared Memory Rainbow Table ===\n");
    printf("Registered regions: %zu\n", table->num_regions);
    printf("Capacity: %zu\n", table->capacity);
    printf("Load factor: %.2f%%\n", 
           (double)table->num_regions / (double)table->capacity * 100.0);
    printf("\n");
}