/**
 * @file shared_memory_rainbow.h
 * @brief Rainbow Table Integration for Shared Memory
 * 
 * Provides O(1) lookup of shared memory regions using rainbow table indexing.
 * Enables fast region lookup by ID and hierarchical organization.
 */

#ifndef SHARED_MEMORY_RAINBOW_H
#define SHARED_MEMORY_RAINBOW_H

#include "shared_memory.h"
#include "math/rainbow.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Shared Memory Rainbow Table
 * 
 * Maps region IDs to SharedMemoryRegion pointers for O(1) lookup.
 */
typedef struct SharedMemoryRainbowTable {
    RainbowTable* rainbow;           // Underlying rainbow table
    SharedMemoryRegion** regions;    // Array of region pointers
    size_t num_regions;              // Number of registered regions
    size_t capacity;                 // Capacity of regions array
    pthread_rwlock_t lock;           // Lock for table modifications
} SharedMemoryRainbowTable;

/**
 * Create shared memory rainbow table
 * 
 * @param initial_capacity Initial capacity for regions
 * @param base Number base for rainbow table
 * @return Allocated table, or NULL on error
 */
SharedMemoryRainbowTable* shared_memory_rainbow_create(size_t initial_capacity, uint32_t base);

/**
 * Free shared memory rainbow table
 * 
 * Note: Does not free the regions themselves, only the table.
 * 
 * @param table Table to free
 */
void shared_memory_rainbow_free(SharedMemoryRainbowTable* table);

/**
 * Register shared memory region in rainbow table
 * 
 * @param table Rainbow table
 * @param region Region to register
 * @param region_id Unique region ID
 * @return 0 on success, -1 on error
 */
int shared_memory_rainbow_register(SharedMemoryRainbowTable* table,
                                   SharedMemoryRegion* region,
                                   uint64_t region_id);

/**
 * Unregister shared memory region from rainbow table
 * 
 * @param table Rainbow table
 * @param region_id Region ID to unregister
 * @return 0 on success, -1 on error
 */
int shared_memory_rainbow_unregister(SharedMemoryRainbowTable* table,
                                     uint64_t region_id);

/**
 * Lookup shared memory region by ID (O(1))
 * 
 * @param table Rainbow table
 * @param region_id Region ID to lookup
 * @return Region pointer, or NULL if not found
 */
SharedMemoryRegion* shared_memory_rainbow_lookup(const SharedMemoryRainbowTable* table,
                                                 uint64_t region_id);

/**
 * Check if region exists in table
 * 
 * @param table Rainbow table
 * @param region_id Region ID to check
 * @return true if exists, false otherwise
 */
bool shared_memory_rainbow_exists(const SharedMemoryRainbowTable* table,
                                  uint64_t region_id);

/**
 * Get all registered region IDs
 * 
 * @param table Rainbow table
 * @param out_ids Output buffer for region IDs
 * @param max_ids Maximum number of IDs to return
 * @return Number of IDs returned
 */
size_t shared_memory_rainbow_get_all_ids(const SharedMemoryRainbowTable* table,
                                        uint64_t* out_ids,
                                        size_t max_ids);

/**
 * Get statistics for rainbow table
 * 
 * @param table Rainbow table
 * @param out_num_regions Output: number of registered regions
 * @param out_capacity Output: capacity of table
 * @param out_load_factor Output: load factor (num_regions / capacity)
 */
void shared_memory_rainbow_get_stats(const SharedMemoryRainbowTable* table,
                                    size_t* out_num_regions,
                                    size_t* out_capacity,
                                    double* out_load_factor);

/**
 * Print rainbow table info (for debugging)
 * 
 * @param table Rainbow table
 */
void shared_memory_rainbow_print_info(const SharedMemoryRainbowTable* table);

#ifdef __cplusplus
}
#endif

#endif /* SHARED_MEMORY_RAINBOW_H */