#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ai/cllm.h"

int main() {
    printf("Test starting...\n");
    fflush(stdout);
    
    CLLMConfig config = cllm_default_config(PLATONIC_CUBE, 100);
    
    printf("Creating model...\n");
    fflush(stdout);
    
    CLLMModel* model = cllm_create_model(&config);
    
    if (model) {
        printf("Model created successfully!\n");
        printf("Sleeping 1 second before free...\n");
        fflush(stdout);
        sleep(1);
        
        printf("About to call cllm_free_model...\n");
        fflush(stdout);
        
        fprintf(stderr, "DEBUG: Calling cllm_free_model(%p)\n", (void*)model);
        
        cllm_free_model(model);
        
        fprintf(stderr, "DEBUG: cllm_free_model returned\n");
        
        printf("cllm_free_model returned!\n");
        fflush(stdout);
    }
    
    printf("Test completed!\n");
    return 0;
}
