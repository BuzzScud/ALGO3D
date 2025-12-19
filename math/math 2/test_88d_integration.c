#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cllm.h"
#include "ai/cllm_88d_integration.h"
#include "../../algorithms/include/hierarchical_threading.h"

int main() {
    printf("Testing 88D Threading Integration\n");
    printf("==================================\n\n");
    
    // Create a CLLM configuration
    CLLMConfig config;
    memset(&config, 0, sizeof(CLLMConfig));
    config.solid_type = PLATONIC_CUBE;
    config.vocab_size = 512;
    config.max_seq_len = 512;
    config.embedding_dim = 0;  // Auto-calculate
    config.hidden_dim = 0;     // Auto-calculate
    config.num_layers = 0;     // Auto-calculate
    config.num_heads = 0;      // Auto-calculate (always 12)
    
    // Create a CLLM model
    CLLMModel* model = cllm_create_model(&config);
    
    if (!model) {
        fprintf(stderr, "Failed to create CLLM model\n");
        return 1;
    }
    
    printf("✓ CLLM model created successfully\n");
    printf("  Embedding dim: %u\n", model->embedding_dim);
    printf("  Hidden dim: %u\n", model->hidden_dim);
    printf("  Num layers: %u\n", model->num_layers);
    printf("  Num heads: %u\n", model->num_heads);
    
    // Note: Threading is now initialized automatically in cllm_create_model()
    // This test verifies that threading was created successfully
    if (!model->threads) {
        fprintf(stderr, "FATAL: Threading was not initialized (threading is mandatory)\n");
        cllm_free_model(model);
        return 1;
    }
    
    printf("\n✓ Threading initialized successfully (mandatory)\n");
    
    // Print threading statistics
    printf("\nThreading Statistics:\n");
    cllm_print_threading_stats(model);
    
    // Get threading stats
    HierarchicalThreadPoolStats stats;
    cllm_get_threading_stats(model, &stats);
    printf("\nDetailed Stats:\n");
    printf("  Number of threads: %u\n", stats.num_threads);
    printf("  Number of levels: %u\n", stats.num_levels);
    printf("  Symmetry fold: %u\n", stats.symmetry_fold);
    printf("  Total work items: %lu\n", (unsigned long)stats.total_work_items);
    printf("  Total messages: %lu\n", (unsigned long)stats.total_messages);
    printf("  Avg CPU utilization: %.2f%%\n", stats.avg_cpu_utilization * 100.0);
    printf("  Load balance factor: %.2f\n", stats.load_balance_factor);
    
    // Test thread mapping
    printf("\nTesting thread mapping:\n");
    for (int i = 0; i < 8; i++) {
        int thread_id = cllm_get_thread_for_vertex(model, i);
        printf("  Vertex %d -> Thread %d\n", i, thread_id);
    }
    
    // Cleanup (threading is cleaned up in cllm_free_model)
    printf("\n✓ Threading will be cleaned up in cllm_free_model()\n");
    
    cllm_free_model(model);
    printf("✓ CLLM model freed successfully\n");
    
    printf("\n==================================\n");
    printf("All tests passed!\n");
    
    return 0;
}