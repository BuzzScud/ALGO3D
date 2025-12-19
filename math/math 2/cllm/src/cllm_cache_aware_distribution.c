/**
 * Cache-Aware Work Distribution Implementation
 * 
 * Phase 4, Day 14
 */

#include "ai/cllm_cache_aware_distribution.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
// Migrated to NEW math library - no prime_float_math needed
#include <stdatomic.h>

// ============================================================================
// TOPOLOGY DETECTION
// ============================================================================

CpuTopology detect_cpu_topology(void) {
    CpuTopology topology;
    memset(&topology, 0, sizeof(CpuTopology));
    
    // Get number of CPUs
    topology.num_cpus = (uint32_t)sysconf(_SC_NPROCESSORS_ONLN);
    
    // Check NUMA availability (simplified - assume single node if not available)
    topology.numa_available = false;
    topology.num_numa_nodes = 1;
    
    // Initialize single NUMA node
    topology.numa_nodes[0].node_id = 0;
    topology.numa_nodes[0].num_cpus = topology.num_cpus;
    topology.numa_nodes[0].is_available = true;
    
    // Detect cache sizes (using sysconf)
    // Note: macOS doesn't support _SC_LEVEL*_CACHE_SIZE, use defaults
    #ifdef __linux__
    // L1 cache
    long l1_size = sysconf(_SC_LEVEL1_DCACHE_SIZE);
    if (l1_size > 0) {
        topology.cache_levels[0].level = 1;
        topology.cache_levels[0].size = (uint64_t)l1_size;
        topology.cache_levels[0].line_size = CACHE_LINE_SIZE;
        topology.cache_levels[0].is_shared = false;
    }
    
    // L2 cache
    long l2_size = sysconf(_SC_LEVEL2_CACHE_SIZE);
    if (l2_size > 0) {
        topology.cache_levels[1].level = 2;
        topology.cache_levels[1].size = (uint64_t)l2_size;
        topology.cache_levels[1].line_size = CACHE_LINE_SIZE;
        topology.cache_levels[1].is_shared = false;
    }
    
    // L3 cache
    long l3_size = sysconf(_SC_LEVEL3_CACHE_SIZE);
    if (l3_size > 0) {
        topology.cache_levels[2].level = 3;
        topology.cache_levels[2].size = (uint64_t)l3_size;
        topology.cache_levels[2].line_size = CACHE_LINE_SIZE;
        topology.cache_levels[2].is_shared = true;
    }
    #else
    // macOS: Use reasonable defaults
    topology.cache_levels[0].level = 1;
    topology.cache_levels[0].size = 32 * 1024;  // 32KB L1 (typical)
    topology.cache_levels[0].line_size = CACHE_LINE_SIZE;
    topology.cache_levels[0].is_shared = false;
    
    topology.cache_levels[1].level = 2;
    topology.cache_levels[1].size = 256 * 1024;  // 256KB L2 (typical)
    topology.cache_levels[1].line_size = CACHE_LINE_SIZE;
    topology.cache_levels[1].is_shared = false;
    
    topology.cache_levels[2].level = 3;
    topology.cache_levels[2].size = 8 * 1024 * 1024;  // 8MB L3 (typical)
    topology.cache_levels[2].line_size = CACHE_LINE_SIZE;
    topology.cache_levels[2].is_shared = true;
    #endif
    
    return topology;
}

uint32_t cache_aware_get_num_numa_nodes(void) {
    CpuTopology topology = detect_cpu_topology();
    return topology.num_numa_nodes;
}

uint32_t cache_aware_get_numa_node_cpu_count(uint32_t node_id) {
    CpuTopology topology = detect_cpu_topology();
    
    if (node_id >= topology.num_numa_nodes) {
        return 0;
    }
    
    return topology.numa_nodes[node_id].num_cpus;
}

uint64_t cache_aware_get_cache_size(uint32_t level) {
    if (level == 0 || level > 3) return 0;
    
    CpuTopology topology = detect_cpu_topology();
    return topology.cache_levels[level - 1].size;
}

bool cache_aware_is_numa_available(void) {
    CpuTopology topology = detect_cpu_topology();
    return topology.numa_available;
}

// ============================================================================
// CACHE-AWARE DISTRIBUTION
// ============================================================================

CacheAwareDistribution create_cache_aware_distribution(
    size_t num_workers,
    const uint64_t* work_sizes,
    const CpuTopology* topology
) {
    CacheAwareDistribution dist;
    memset(&dist, 0, sizeof(CacheAwareDistribution));
    
    if (num_workers == 0 || !work_sizes || !topology) {
        return dist;
    }
    
    dist.num_placements = num_workers;
    dist.placements = calloc(num_workers, sizeof(WorkPlacement));
    
    if (!dist.placements) {
        fprintf(stderr, "ERROR: Failed to allocate placements\n");
        return dist;
    }
    
    // Calculate total work size
    for (size_t i = 0; i < num_workers; i++) {
        dist.total_work_size += work_sizes[i];
    }
    
    // Assign work to CPUs
    for (size_t i = 0; i < num_workers; i++) {
        dist.placements[i].cpu_id = i % topology->num_cpus;
        dist.placements[i].numa_node = 0;  // Simplified: single NUMA node
        dist.placements[i].work_size = work_sizes[i];
        dist.placements[i].pin_to_cpu = true;
    }
    
    dist.is_valid = true;
    
    return dist;
}

bool optimize_for_cache_locality(
    CacheAwareDistribution* distribution,
    const CpuTopology* topology
) {
    if (!distribution || !topology) return false;
    if (!distribution->is_valid) return false;
    
    // Sort placements by work size (larger work on separate CPUs)
    // This is a simplified optimization
    
    // For each placement, check if work fits in L3 cache
    uint64_t l3_size = topology->cache_levels[2].size;
    
    for (size_t i = 0; i < distribution->num_placements; i++) {
        WorkPlacement* placement = &distribution->placements[i];
        
        // If work fits in L3, prefer to keep on same CPU
        if (placement->work_size <= l3_size) {
            placement->pin_to_cpu = true;
        } else {
            // Large work - allow migration
            placement->pin_to_cpu = false;
        }
    }
    
    return true;
}

bool optimize_for_numa(
    CacheAwareDistribution* distribution,
    const CpuTopology* topology
) {
    if (!distribution || !topology) return false;
    if (!distribution->is_valid) return false;
    
    if (!topology->numa_available) {
        // NUMA not available, nothing to optimize
        return true;
    }
    
    // Distribute work across NUMA nodes evenly
    for (size_t i = 0; i < distribution->num_placements; i++) {
        uint32_t numa_node = i % topology->num_numa_nodes;
        distribution->placements[i].numa_node = numa_node;
    }
    
    return true;
}

bool balance_distribution(CacheAwareDistribution* distribution) {
    if (!distribution) return false;
    if (!distribution->is_valid) return false;
    
    // Calculate average work size
    uint64_t avg_work = distribution->total_work_size / distribution->num_placements;
    
    // Check if distribution is already balanced
    bool is_balanced = true;
    for (size_t i = 0; i < distribution->num_placements; i++) {
        uint64_t diff = (distribution->placements[i].work_size > avg_work) ?
                        (distribution->placements[i].work_size - avg_work) :
                        (avg_work - distribution->placements[i].work_size);
        
        // Allow 10% deviation
        if (diff > avg_work / 10) {
            is_balanced = false;
            break;
        }
    }
    
    return is_balanced;
}

// ============================================================================
// WORK STEALING
// ============================================================================

WorkStealingQueue* work_stealing_queue_create(size_t capacity) {
    if (capacity == 0) {
        capacity = 1024;  // Default capacity
    }
    
    WorkStealingQueue* queue = calloc(1, sizeof(WorkStealingQueue));
    if (!queue) {
        fprintf(stderr, "ERROR: Failed to allocate work stealing queue\n");
        return NULL;
    }
    
    queue->items = calloc(capacity, sizeof(void*));
    if (!queue->items) {
        fprintf(stderr, "ERROR: Failed to allocate queue items\n");
        free(queue);
        return NULL;
    }
    
    queue->capacity = capacity;
    atomic_init(&queue->head, 0);
    atomic_init(&queue->tail, 0);
    atomic_init(&queue->size, 0);
    
    return queue;
}

void work_stealing_queue_destroy(WorkStealingQueue* queue) {
    if (!queue) return;
    
    free(queue->items);
    free(queue);
}

bool work_stealing_queue_push(WorkStealingQueue* queue, void* item) {
    if (!queue || !item) return false;
    
    size_t current_size = atomic_load(&queue->size);
    if (current_size >= queue->capacity) {
        return false;  // Queue full
    }
    
    size_t tail = atomic_load(&queue->tail);
    queue->items[tail % queue->capacity] = item;
    
    atomic_store(&queue->tail, tail + 1);
    atomic_fetch_add(&queue->size, 1);
    
    return true;
}

void* work_stealing_queue_pop(WorkStealingQueue* queue) {
    if (!queue) return NULL;
    
    size_t current_size = atomic_load(&queue->size);
    if (current_size == 0) {
        return NULL;  // Queue empty
    }
    
    size_t tail = atomic_load(&queue->tail);
    if (tail == 0) return NULL;
    
    tail--;
    void* item = queue->items[tail % queue->capacity];
    
    atomic_store(&queue->tail, tail);
    atomic_fetch_sub(&queue->size, 1);
    
    return item;
}

void* work_stealing_queue_steal(WorkStealingQueue* queue) {
    if (!queue) return NULL;
    
    size_t current_size = atomic_load(&queue->size);
    if (current_size == 0) {
        return NULL;  // Queue empty
    }
    
    size_t head = atomic_load(&queue->head);
    void* item = queue->items[head % queue->capacity];
    
    atomic_store(&queue->head, head + 1);
    atomic_fetch_sub(&queue->size, 1);
    
    return item;
}

size_t work_stealing_queue_size(const WorkStealingQueue* queue) {
    if (!queue) return 0;
    return atomic_load(&queue->size);
}

bool work_stealing_queue_is_empty(const WorkStealingQueue* queue) {
    return work_stealing_queue_size(queue) == 0;
}

// ============================================================================
// STATISTICS
// ============================================================================

void cache_aware_stats_init(CacheAwareStats* stats) {
    if (!stats) return;
    memset(stats, 0, sizeof(CacheAwareStats));
}

void cache_aware_stats_update(
    CacheAwareStats* stats,
    const CacheAwareDistribution* distribution
) {
    if (!stats || !distribution) return;
    
    stats->total_distributions++;
    
    if (distribution->is_valid) {
        // Check if NUMA-aware
        bool numa_aware = false;
        for (size_t i = 0; i < distribution->num_placements; i++) {
            if (distribution->placements[i].numa_node > 0) {
                numa_aware = true;
                break;
            }
        }
        if (numa_aware) {
            stats->numa_aware_distributions++;
        }
        
        // Check if cache-optimized
        bool cache_optimized = false;
        for (size_t i = 0; i < distribution->num_placements; i++) {
            if (distribution->placements[i].pin_to_cpu) {
                cache_optimized = true;
                break;
            }
        }
        if (cache_optimized) {
            stats->cache_optimized_distributions++;
        }
        
        // Update average work size
        double n = (double)stats->total_distributions;
        double avg_work = (double)distribution->total_work_size / (double)distribution->num_placements;
        stats->avg_work_size = (stats->avg_work_size * (n - 1.0) + avg_work) / n;
    }
}

void cache_aware_stats_record_steal(
    CacheAwareStats* stats,
    bool success
) {
    if (!stats) return;
    
    if (success) {
        stats->work_steals++;
    } else {
        stats->failed_steals++;
    }
}

void cache_aware_stats_print(const CacheAwareStats* stats) {
    if (!stats) return;
    
    printf("Cache-Aware Distribution Statistics:\n");
    printf("  Total Distributions:        %llu\n", 
           (unsigned long long)stats->total_distributions);
    printf("  NUMA-Aware Distributions:   %llu\n", 
           (unsigned long long)stats->numa_aware_distributions);
    printf("  Cache-Optimized:            %llu\n", 
           (unsigned long long)stats->cache_optimized_distributions);
    printf("  Work Steals:                %llu\n", 
           (unsigned long long)stats->work_steals);
    printf("  Failed Steals:              %llu\n", 
           (unsigned long long)stats->failed_steals);
    
    if (stats->total_distributions > 0) {
        printf("  Avg Work Size:              %.2f bytes\n", stats->avg_work_size);
    }
    
    if (stats->work_steals + stats->failed_steals > 0) {
        double steal_success_rate = (double)stats->work_steals / 
                                    (double)(stats->work_steals + stats->failed_steals);
        printf("  Steal Success Rate:         %.2f%%\n", steal_success_rate * 100.0);
    }
}

void cache_aware_stats_reset(CacheAwareStats* stats) {
    if (!stats) return;
    cache_aware_stats_init(stats);
}

// ============================================================================
// UTILITIES
// ============================================================================

void cache_aware_distribution_free(CacheAwareDistribution* distribution) {
    if (!distribution) return;
    
    free(distribution->placements);
    memset(distribution, 0, sizeof(CacheAwareDistribution));
}

bool cache_aware_distribution_validate(const CacheAwareDistribution* distribution) {
    if (!distribution) {
        fprintf(stderr, "ERROR: NULL distribution\n");
        return false;
    }
    
    if (!distribution->is_valid) {
        fprintf(stderr, "ERROR: Distribution marked invalid\n");
        return false;
    }
    
    if (!distribution->placements) {
        fprintf(stderr, "ERROR: NULL placements\n");
        return false;
    }
    
    if (distribution->num_placements == 0) {
        fprintf(stderr, "ERROR: Zero placements\n");
        return false;
    }
    
    // Validate total work size
    uint64_t total = 0;
    for (size_t i = 0; i < distribution->num_placements; i++) {
        total += distribution->placements[i].work_size;
    }
    
    if (total != distribution->total_work_size) {
        fprintf(stderr, "ERROR: Work size mismatch\n");
        return false;
    }
    
    return true;
}

void cache_aware_distribution_print(const CacheAwareDistribution* distribution) {
    if (!distribution) return;
    
    printf("Cache-Aware Distribution:\n");
    printf("  Num Placements:  %zu\n", distribution->num_placements);
    printf("  Total Work Size: %llu bytes\n", 
           (unsigned long long)distribution->total_work_size);
    printf("  Valid:           %s\n", distribution->is_valid ? "Yes" : "No");
    
    for (size_t i = 0; i < distribution->num_placements; i++) {
        const WorkPlacement* p = &distribution->placements[i];
        printf("  [%zu] CPU: %u, NUMA: %u, Size: %llu, Pin: %s\n",
               i, p->cpu_id, p->numa_node,
               (unsigned long long)p->work_size,
               p->pin_to_cpu ? "Yes" : "No");
    }
}

void cpu_topology_print(const CpuTopology* topology) {
    if (!topology) return;
    
    printf("CPU Topology:\n");
    printf("  Num CPUs:        %u\n", topology->num_cpus);
    printf("  Num NUMA Nodes:  %u\n", topology->num_numa_nodes);
    printf("  NUMA Available:  %s\n", topology->numa_available ? "Yes" : "No");
    
    printf("\n  NUMA Nodes:\n");
    for (uint32_t i = 0; i < topology->num_numa_nodes; i++) {
        const NumaNodeInfo* node = &topology->numa_nodes[i];
        if (node->is_available) {
            printf("    Node %u: %u CPUs\n", node->node_id, node->num_cpus);
        }
    }
    
    printf("\n  Cache Levels:\n");
    for (uint32_t i = 0; i < MAX_CACHE_LEVELS; i++) {
        const CacheLevelInfo* cache = &topology->cache_levels[i];
        if (cache->size > 0) {
            printf("    L%u: %llu bytes, Line: %u bytes, Shared: %s\n",
                   cache->level,
                   (unsigned long long)cache->size,
                   cache->line_size,
                   cache->is_shared ? "Yes" : "No");
        }
    }
}

uint32_t get_optimal_cpu(uint64_t work_size, const CpuTopology* topology) {
    if (!topology) return 0;
    
    // Simple strategy: round-robin assignment
    // In a real implementation, this would consider:
    // - Current CPU load
    // - Cache affinity
    // - NUMA node locality
    
    static uint32_t next_cpu = 0;
    uint32_t cpu = next_cpu % topology->num_cpus;
    next_cpu++;
    
    (void)work_size;  // Unused in simplified version
    
    return cpu;
}

double calculate_cache_affinity(uint64_t work_size, uint64_t cache_size) {
    if (cache_size == 0) return 0.0;
    
    if (work_size <= cache_size) {
        // Work fits entirely in cache
        return 1.0;
    } else if (work_size <= cache_size * 2) {
        // Work partially fits
        return 0.5;
    } else {
        // Work doesn't fit
        return 0.1;
    }
}