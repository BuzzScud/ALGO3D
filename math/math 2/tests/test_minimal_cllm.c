#include <stdio.h>
#include <stdlib.h>
#include "ai/cllm.h"

int main() {
    printf("Test starting...\n");
    fflush(stdout);
    
    printf("Creating model config...\n");
    fflush(stdout);
    
    CLLMConfig config = cllm_default_config(PLATONIC_CUBE, 100);
    
    printf("Config created. Calling cllm_create_model...\n");
    fflush(stdout);
    
    CLLMModel* model = cllm_create_model(&config);
    
    printf("Model created: %p\n", (void*)model);
    fflush(stdout);
    
    if (model) {
        printf("Model has pool_88d: %p\n", (void*)model->pool_88d);
        fflush(stdout);
        
        printf("Freeing model...\n");
        fflush(stdout);
        cllm_free_model(model);
        printf("Model freed!\n");
    }
    
    printf("Test completed!\n");
    return 0;
}
