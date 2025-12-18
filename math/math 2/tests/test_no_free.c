#include <stdio.h>
#include <stdlib.h>
#include "ai/cllm.h"

int main() {
    printf("Creating model...\n");
    fflush(stdout);
    
    CLLMConfig config = cllm_default_config(PLATONIC_CUBE, 100);
    CLLMModel* model = cllm_create_model(&config);
    
    if (model) {
        printf("Model created: %p\n", (void*)model);
        printf("Pool: %p\n", (void*)model->pool_88d);
        printf("NOT calling free - just exiting\n");
        fflush(stdout);
    }
    
    printf("Exiting main...\n");
    fflush(stdout);
    
    return 0;
}
