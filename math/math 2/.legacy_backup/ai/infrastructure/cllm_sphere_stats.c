#include "math/transcendental.h"
#include "math/arithmetic.h"
#include "ai/cllm_sphere_stats.h"
#include <stdio.h>
#include <string.h>

// Crystalline constants from the NEW math library design
#define VECTOR_CULMINATION 144000ULL  // 3 × 12³ × (250/9)
#define TWIN_PRIME_LOWER 143999ULL    // Prime below 144000
#define TWIN_PRIME_UPPER 144001ULL    // Prime above 144000

void cllm_sphere_stats_init(SphereStatistics* stats, 
                            int symmetry_group,
                            int hierarchy_level) {
    if (!stats) return;
    
    // Zero out all atomic counters
    atomic_init(&stats->primes_processed, 0);
    atomic_init(&stats->batches_completed, 0);
    atomic_init(&stats->gradients_computed, 0);
    atomic_init(&stats->weights_updated, 0);
    
    // Initialize symmetry group tracking
    stats->symmetry_group = symmetry_group;
    for (int i = 0; i < 12; i++) {
        atomic_init(&stats->primes_per_group[i], 0);
    }
    
    // Initialize boundary tracking
    atomic_init(&stats->boundary_crossings, 0);
    atomic_init(&stats->twin_prime_hits, 0);
    atomic_init(&stats->near_boundary_primes, 0);
    
    // Initialize performance metrics
    atomic_init(&stats->total_time_ns, 0);
    atomic_init(&stats->idle_time_ns, 0);
    atomic_init(&stats->sync_time_ns, 0);
    
    // Initialize work stealing statistics
    atomic_init(&stats->work_stolen_from, 0);
    atomic_init(&stats->work_stolen_to, 0);
    atomic_init(&stats->work_items_stolen, 0);
    
    // Initialize memory statistics
    atomic_init(&stats->cache_hits, 0);
    atomic_init(&stats->cache_misses, 0);
    atomic_init(&stats->memory_allocated, 0);
    
    // Initialize hierarchy statistics
    stats->hierarchy_level = hierarchy_level;
    stats->num_children = 0;
    atomic_init(&stats->messages_sent, 0);
    atomic_init(&stats->messages_received, 0);
    
    // Initialize error tracking
    atomic_init(&stats->errors_encountered, 0);
    atomic_init(&stats->retries_attempted, 0);
    
    // Set timestamps
    clock_gettime(CLOCK_MONOTONIC, &stats->creation_time);
    stats->last_update_time = stats->creation_time;
}

void cllm_sphere_stats_record_prime(SphereStatistics* stats,
                                    uint64_t prime,
                                    uint64_t processing_time_ns) {
    if (!stats) return;
    
    // Increment total primes processed
    atomic_fetch_add(&stats->primes_processed, 1);
    
    // Track by symmetry group
    int group = prime % 12;
    atomic_fetch_add(&stats->primes_per_group[group], 1);
    
    // Check for 144000 boundary
    if (cllm_is_near_144000_boundary(prime)) {
        atomic_fetch_add(&stats->near_boundary_primes, 1);
        
        if (prime == TWIN_PRIME_LOWER || prime == TWIN_PRIME_UPPER) {
            atomic_fetch_add(&stats->twin_prime_hits, 1);
        }
        
        // Check if we crossed the boundary
        static uint64_t last_prime = 0;
        if ((last_prime < VECTOR_CULMINATION && prime > VECTOR_CULMINATION) ||
            (last_prime > VECTOR_CULMINATION && prime < VECTOR_CULMINATION)) {
            atomic_fetch_add(&stats->boundary_crossings, 1);
        }
        last_prime = prime;
    }
    
    // Update processing time
    atomic_fetch_add(&stats->total_time_ns, processing_time_ns);
    
    // Update timestamp
    clock_gettime(CLOCK_MONOTONIC, &stats->last_update_time);
}

void cllm_sphere_stats_record_batch(SphereStatistics* stats,
                                    uint64_t batch_size,
                                    uint64_t batch_time_ns) {
    (void)batch_size; // Reserved for future batch size tracking
    if (!stats) return;
    
    atomic_fetch_add(&stats->batches_completed, 1);
    atomic_fetch_add(&stats->total_time_ns, batch_time_ns);
    
    clock_gettime(CLOCK_MONOTONIC, &stats->last_update_time);
}

void cllm_sphere_stats_record_gradients(SphereStatistics* stats,
                                        uint64_t num_gradients) {
    if (!stats) return;
    
    atomic_fetch_add(&stats->gradients_computed, num_gradients);
    
    clock_gettime(CLOCK_MONOTONIC, &stats->last_update_time);
}

void cllm_sphere_stats_record_weights(SphereStatistics* stats,
                                      uint64_t num_weights) {
    if (!stats) return;
    
    atomic_fetch_add(&stats->weights_updated, num_weights);
    
    clock_gettime(CLOCK_MONOTONIC, &stats->last_update_time);
}

void cllm_sphere_stats_record_boundary_crossing(SphereStatistics* stats,
                                                uint64_t prime) {
    if (!stats) return;
    
    atomic_fetch_add(&stats->boundary_crossings, 1);
    
    if (prime == TWIN_PRIME_LOWER || prime == TWIN_PRIME_UPPER) {
        atomic_fetch_add(&stats->twin_prime_hits, 1);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &stats->last_update_time);
}

void cllm_sphere_stats_record_work_stealing(SphereStatistics* stats,
                                            int stolen_from,
                                            uint64_t num_items) {
    if (!stats) return;
    
    if (stolen_from) {
        atomic_fetch_add(&stats->work_stolen_from, 1);
    } else {
        atomic_fetch_add(&stats->work_stolen_to, 1);
    }
    
    atomic_fetch_add(&stats->work_items_stolen, num_items);
    
    clock_gettime(CLOCK_MONOTONIC, &stats->last_update_time);
}

void cllm_sphere_stats_record_cache_access(SphereStatistics* stats,
                                           int hit) {
    if (!stats) return;
    
    if (hit) {
        atomic_fetch_add(&stats->cache_hits, 1);
    } else {
        atomic_fetch_add(&stats->cache_misses, 1);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &stats->last_update_time);
}

void cllm_sphere_stats_record_message(SphereStatistics* stats,
                                      int sent) {
    if (!stats) return;
    
    if (sent) {
        atomic_fetch_add(&stats->messages_sent, 1);
    } else {
        atomic_fetch_add(&stats->messages_received, 1);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &stats->last_update_time);
}

void cllm_sphere_stats_record_error(SphereStatistics* stats,
                                    int retry) {
    if (!stats) return;
    
    atomic_fetch_add(&stats->errors_encountered, 1);
    
    if (retry) {
        atomic_fetch_add(&stats->retries_attempted, 1);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &stats->last_update_time);
}

void cllm_sphere_stats_record_idle_time(SphereStatistics* stats,
                                        uint64_t idle_time_ns) {
    if (!stats) return;
    
    atomic_fetch_add(&stats->idle_time_ns, idle_time_ns);
    
    clock_gettime(CLOCK_MONOTONIC, &stats->last_update_time);
}

void cllm_sphere_stats_record_sync_time(SphereStatistics* stats,
                                        uint64_t sync_time_ns) {
    if (!stats) return;
    
    atomic_fetch_add(&stats->sync_time_ns, sync_time_ns);
    
    clock_gettime(CLOCK_MONOTONIC, &stats->last_update_time);
}

void cllm_sphere_stats_print(const SphereStatistics* stats,
                             int sphere_id) {
    if (!stats) return;
    
    printf("\n=== Sphere %d Statistics ===\n", sphere_id);
    printf("Symmetry Group: %d\n", stats->symmetry_group);
    printf("Hierarchy Level: %d\n", stats->hierarchy_level);
    printf("Primes Processed: %lu\n", 
           atomic_load(&stats->primes_processed));
    printf("Batches Completed: %lu\n", 
           atomic_load(&stats->batches_completed));
    printf("Gradients Computed: %lu\n", 
           atomic_load(&stats->gradients_computed));
    printf("Weights Updated: %lu\n", 
           atomic_load(&stats->weights_updated));
    
    // Boundary statistics
    uint64_t boundary_crossings = atomic_load(&stats->boundary_crossings);
    uint64_t twin_prime_hits = atomic_load(&stats->twin_prime_hits);
    uint64_t near_boundary = atomic_load(&stats->near_boundary_primes);
    
    if (boundary_crossings > 0 || twin_prime_hits > 0 || near_boundary > 0) {
        printf("\n144000 Boundary Statistics:\n");
        printf("  Boundary Crossings: %lu\n", boundary_crossings);
        printf("  Twin Prime Hits: %lu\n", twin_prime_hits);
        printf("  Near Boundary Primes: %lu\n", near_boundary);
    }
    
    // Performance metrics
    double cache_hit_rate = cllm_sphere_stats_get_cache_hit_rate(stats);
    double avg_prime_time = cllm_sphere_stats_get_avg_prime_time(stats);
    double utilization = cllm_sphere_stats_get_utilization(stats);
    
    printf("\nPerformance Metrics:\n");
    printf("  Cache Hit Rate: %.2f%%\n", cache_hit_rate * 100.0);
    printf("  Avg Prime Time: %.2f ns\n", avg_prime_time);
    printf("  Utilization: %.2f%%\n", utilization * 100.0);
    
    printf("===========================\n\n");
}

void cllm_sphere_stats_print_detailed(const SphereStatistics* stats,
                                      int sphere_id) {
    if (!stats) return;
    
    cllm_sphere_stats_print(stats, sphere_id);
    
    printf("=== Detailed Statistics for Sphere %d ===\n", sphere_id);
    
    // Symmetry group breakdown
    printf("\nPrimes per Symmetry Group:\n");
    for (int i = 0; i < 12; i++) {
        uint64_t count = atomic_load(&stats->primes_per_group[i]);
        if (count > 0) {
            printf("  Group %2d: %lu\n", i, count);
        }
    }
    
    // Work stealing statistics
    uint64_t stolen_from = atomic_load(&stats->work_stolen_from);
    uint64_t stolen_to = atomic_load(&stats->work_stolen_to);
    uint64_t items_stolen = atomic_load(&stats->work_items_stolen);
    
    if (stolen_from > 0 || stolen_to > 0) {
        printf("\nWork Stealing:\n");
        printf("  Times Stolen From: %lu\n", stolen_from);
        printf("  Times Stole Work: %lu\n", stolen_to);
        printf("  Total Items Stolen: %lu\n", items_stolen);
    }
    
    // Message passing
    uint64_t msgs_sent = atomic_load(&stats->messages_sent);
    uint64_t msgs_recv = atomic_load(&stats->messages_received);
    
    if (msgs_sent > 0 || msgs_recv > 0) {
        printf("\nMessage Passing:\n");
        printf("  Messages Sent: %lu\n", msgs_sent);
        printf("  Messages Received: %lu\n", msgs_recv);
    }
    
    // Error tracking
    uint64_t errors = atomic_load(&stats->errors_encountered);
    uint64_t retries = atomic_load(&stats->retries_attempted);
    
    if (errors > 0) {
        printf("\nError Tracking:\n");
        printf("  Errors Encountered: %lu\n", errors);
        printf("  Retries Attempted: %lu\n", retries);
    }
    
    // Time breakdown
    uint64_t total_time = atomic_load(&stats->total_time_ns);
    uint64_t idle_time = atomic_load(&stats->idle_time_ns);
    uint64_t sync_time = atomic_load(&stats->sync_time_ns);
    
    printf("\nTime Breakdown:\n");
    printf("  Total Time: %.3f ms\n", total_time / 1e6);
    printf("  Idle Time: %.3f ms (%.1f%%)\n", 
           idle_time / 1e6, 
           total_time > 0 ? (idle_time * 100.0 / total_time) : 0.0);
    printf("  Sync Time: %.3f ms (%.1f%%)\n", 
           sync_time / 1e6,
           total_time > 0 ? (sync_time * 100.0 / total_time) : 0.0);
    
    printf("=========================================\n\n");
}

double cllm_sphere_stats_get_cache_hit_rate(const SphereStatistics* stats) {
    if (!stats) return 0.0;
    
    uint64_t hits = atomic_load(&stats->cache_hits);
    uint64_t misses = atomic_load(&stats->cache_misses);
    uint64_t total = hits + misses;
    
    if (total == 0) return 0.0;
    
    return (double)hits / (double)total;
}

double cllm_sphere_stats_get_avg_prime_time(const SphereStatistics* stats) {
    if (!stats) return 0.0;
    
    uint64_t total_time = atomic_load(&stats->total_time_ns);
    uint64_t primes = atomic_load(&stats->primes_processed);
    
    if (primes == 0) return 0.0;
    
    return (double)total_time / (double)primes;
}

double cllm_sphere_stats_get_utilization(const SphereStatistics* stats) {
    if (!stats) return 0.0;
    
    uint64_t total_time = atomic_load(&stats->total_time_ns);
    uint64_t idle_time = atomic_load(&stats->idle_time_ns);
    
    if (total_time == 0) return 0.0;
    
    uint64_t active_time = total_time - idle_time;
    return (double)active_time / (double)total_time;
}

void cllm_sphere_stats_reset(SphereStatistics* stats) {
    if (!stats) return;
    
    // Reset counters but preserve configuration
    atomic_store(&stats->primes_processed, 0);
    atomic_store(&stats->batches_completed, 0);
    atomic_store(&stats->gradients_computed, 0);
    atomic_store(&stats->weights_updated, 0);
    
    for (int i = 0; i < 12; i++) {
        atomic_store(&stats->primes_per_group[i], 0);
    }
    
    atomic_store(&stats->boundary_crossings, 0);
    atomic_store(&stats->twin_prime_hits, 0);
    atomic_store(&stats->near_boundary_primes, 0);
    
    atomic_store(&stats->total_time_ns, 0);
    atomic_store(&stats->idle_time_ns, 0);
    atomic_store(&stats->sync_time_ns, 0);
    
    atomic_store(&stats->work_stolen_from, 0);
    atomic_store(&stats->work_stolen_to, 0);
    atomic_store(&stats->work_items_stolen, 0);
    
    atomic_store(&stats->cache_hits, 0);
    atomic_store(&stats->cache_misses, 0);
    
    atomic_store(&stats->messages_sent, 0);
    atomic_store(&stats->messages_received, 0);
    
    atomic_store(&stats->errors_encountered, 0);
    atomic_store(&stats->retries_attempted, 0);
    
    clock_gettime(CLOCK_MONOTONIC, &stats->last_update_time);
}

void cllm_sphere_stats_merge(SphereStatistics* parent,
                             const SphereStatistics* child) {
    if (!parent || !child) return;
    
    // Merge counters
    atomic_fetch_add(&parent->primes_processed, 
                     atomic_load(&child->primes_processed));
    atomic_fetch_add(&parent->batches_completed, 
                     atomic_load(&child->batches_completed));
    atomic_fetch_add(&parent->gradients_computed, 
                     atomic_load(&child->gradients_computed));
    atomic_fetch_add(&parent->weights_updated, 
                     atomic_load(&child->weights_updated));
    
    // Merge symmetry group counts
    for (int i = 0; i < 12; i++) {
        atomic_fetch_add(&parent->primes_per_group[i], 
                        atomic_load(&child->primes_per_group[i]));
    }
    
    // Merge boundary statistics
    atomic_fetch_add(&parent->boundary_crossings, 
                     atomic_load(&child->boundary_crossings));
    atomic_fetch_add(&parent->twin_prime_hits, 
                     atomic_load(&child->twin_prime_hits));
    atomic_fetch_add(&parent->near_boundary_primes, 
                     atomic_load(&child->near_boundary_primes));
    
    // Merge time statistics
    atomic_fetch_add(&parent->total_time_ns, 
                     atomic_load(&child->total_time_ns));
    atomic_fetch_add(&parent->idle_time_ns, 
                     atomic_load(&child->idle_time_ns));
    atomic_fetch_add(&parent->sync_time_ns, 
                     atomic_load(&child->sync_time_ns));
    
    // Merge work stealing statistics
    atomic_fetch_add(&parent->work_stolen_from, 
                     atomic_load(&child->work_stolen_from));
    atomic_fetch_add(&parent->work_stolen_to, 
                     atomic_load(&child->work_stolen_to));
    atomic_fetch_add(&parent->work_items_stolen, 
                     atomic_load(&child->work_items_stolen));
    
    // Merge cache statistics
    atomic_fetch_add(&parent->cache_hits, 
                     atomic_load(&child->cache_hits));
    atomic_fetch_add(&parent->cache_misses, 
                     atomic_load(&child->cache_misses));
    
    // Merge message statistics
    atomic_fetch_add(&parent->messages_sent, 
                     atomic_load(&child->messages_sent));
    atomic_fetch_add(&parent->messages_received, 
                     atomic_load(&child->messages_received));
    
    // Merge error statistics
    atomic_fetch_add(&parent->errors_encountered, 
                     atomic_load(&child->errors_encountered));
    atomic_fetch_add(&parent->retries_attempted, 
                     atomic_load(&child->retries_attempted));
}