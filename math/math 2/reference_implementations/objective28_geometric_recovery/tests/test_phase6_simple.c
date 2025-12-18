#include <stdio.h>
#include <stdlib.h>
#include "multi_scalar_analysis.h"

int main() {
    printf("\n========================================\n");
    printf("PHASE 6: MULTI-SCALAR ANALYSIS - SIMPLE TEST\n");
    printf("========================================\n\n");
    
    // Create ECDLP instance with k=42
    BIGNUM* k = BN_new();
    BN_set_word(k, 42);
    
    ECDLPInstance* instance = ecdlp_create_instance_with_k(NID_secp192k1, k);
    if (!instance) {
        printf("❌ Failed to create ECDLP instance\n");
        BN_free(k);
        return 1;
    }
    printf("✅ ECDLP instance created (k=42)\n");
    
    // Get Q embedding
    ECPointCoords* coords = ec_get_point_coords(instance->group, instance->Q);
    if (!coords) {
        printf("❌ Failed to get point coordinates\n");
        ecdlp_free_instance(instance);
        BN_free(k);
        return 1;
    }
    
    ECLatticeEmbedding* Q_embedding = malloc(sizeof(ECLatticeEmbedding));
    if (!lattice_embed_ec_point(coords, Q_embedding)) {
        printf("❌ Failed to embed Q\n");
        free(coords);
        free(Q_embedding);
        ecdlp_free_instance(instance);
        BN_free(k);
        return 1;
    }
    free(coords);
    printf("✅ Q embedded to 15D lattice\n");
    
    // Create scalars
    double scalars[] = {1.0};
    uint32_t num_scalars = 1;
    
    // Create multi-scalar analysis
    MultiScalarAnalysis* analysis = create_multi_scalar_analysis(
        instance, Q_embedding, scalars, num_scalars
    );
    if (!analysis) {
        printf("❌ Failed to create multi-scalar analysis\n");
        free(Q_embedding);
        ecdlp_free_instance(instance);
        BN_free(k);
        return 1;
    }
    printf("✅ Multi-scalar analysis created\n");
    
    // Test at 1.0x scalar
    printf("\nTesting at 1.0x scalar...\n");
    if (!test_at_scalar(analysis, 0)) {
        printf("❌ Test failed\n");
        free_multi_scalar_analysis(analysis);
        free(Q_embedding);
        ecdlp_free_instance(instance);
        BN_free(k);
        return 1;
    }
    
    const ScalarTestResult* result = get_scalar_result(analysis, 0);
    if (!result) {
        printf("❌ Failed to get result\n");
        free_multi_scalar_analysis(analysis);
        free(Q_embedding);
        ecdlp_free_instance(instance);
        BN_free(k);
        return 1;
    }
    
    printf("\nResult:\n");
    printf("  Success: %s\n", result->success ? "YES" : "NO");
    printf("  Score: %.6f\n", result->final_score);
    printf("  Iterations: %u\n", result->iterations);
    printf("  Time: %.3f seconds\n", result->elapsed_time);
    
    if (result->success && result->final_score > 0.95) {
        printf("\n✅ PHASE 6 TEST PASSED\n");
    } else {
        printf("\n❌ PHASE 6 TEST FAILED\n");
    }
    
    // Cleanup
    free_multi_scalar_analysis(analysis);
    free(Q_embedding);
    ecdlp_free_instance(instance);
    BN_free(k);
    
    printf("\n========================================\n");
    return 0;
}
