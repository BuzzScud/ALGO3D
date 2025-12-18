/**
 * @file cllm_clock_memory.c
 * @brief Clock-based memory mapping for CLLM threading
 * 
 * Maps threads to memory positions using clock lattice structure.
 * Uses NEW math library exclusively.
 */

#include "math/types.h"
#include "math/clock.h"
#include <string.h>

/**
 * @brief Memory position mapped from clock structure
 */
typedef struct {
    uint32_t ring;           /**< Ring number from clock */
    uint32_t position;       /**< Position within ring */
    size_t segment_size;     /**< Size of memory segment */
    size_t memory_offset;    /**< Offset in memory */
} ClockMemoryPosition;

/**
 * @brief Map thread to memory position using clock structure
 * 
 * Uses clock lattice structure to determine memory layout:
 * - Ring 0: 12 positions (hours)
 * - Ring 1: 60 positions (minutes)
 * - Ring 2: 60 positions (seconds)
 * - Ring 3: 100 positions (milliseconds)
 * - Ring 4+: 1000 positions each
 * 
 * @param thread_id Thread ID
 * @param hierarchy_level Level in hierarchy (0 = root)
 * @param total_memory Total memory available (bytes)
 * @return Memory position for this thread
 */
ClockMemoryPosition map_thread_to_memory(
    int thread_id,
    int hierarchy_level,
    size_t total_memory
) {
    ClockMemoryPosition mem_pos;
    memset(&mem_pos, 0, sizeof(ClockMemoryPosition));
    
    // Get clock position for this thread using NEW math library
    ClockPosition clock_pos;
    clock_map_index_to_position((uint64_t)thread_id, &clock_pos);
    
    // Map clock position to memory position
    mem_pos.ring = clock_pos.ring;
    mem_pos.position = clock_pos.position;
    
    // Calculate segment size based on ring
    size_t positions_in_ring;
    switch (mem_pos.ring) {
        case 0: positions_in_ring = 12; break;
        case 1: positions_in_ring = 60; break;
        case 2: positions_in_ring = 60; break;
        case 3: positions_in_ring = 100; break;
        default: positions_in_ring = 1000; break;
    }
    
    mem_pos.segment_size = total_memory / positions_in_ring;
    
    // Calculate memory offset
    mem_pos.memory_offset = mem_pos.position * mem_pos.segment_size;
    
    // Adjust for hierarchy level (each level gets a fraction of the segment)
    if (hierarchy_level > 0) {
        size_t hierarchy_offset = (mem_pos.segment_size / 10) * hierarchy_level;
        mem_pos.memory_offset += hierarchy_offset;
        mem_pos.segment_size /= (hierarchy_level + 1);
    }
    
    return mem_pos;
}