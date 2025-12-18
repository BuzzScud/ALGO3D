# Find the line "// Training loop" and insert the optimized code before the for loop
/\/\/ Training loop/,/for (int epoch = 0/ {
    /for (int epoch = 0/ {
        i\    // PERFORMANCE FIX: Create threaded system ONCE before epoch loop\
    CLLMBatchIterator* batch_iterator = cllm_batch_iterator_create(\
        training->tokens, training->num_tokens,\
        config->batch_size, config->sequence_length, 0, 0);\
    if (!batch_iterator) { fprintf(stderr, "Failed to create batch iterator\\n"); goto cleanup; }\
    \
    ThreadedTrainingSystem* threaded_system = threaded_training_create(\
        training, batch_iterator, num_threads);\
    if (!threaded_system) {\
        cllm_batch_iterator_free(batch_iterator);\
        fprintf(stderr, "Failed to create threaded system\\n");\
        goto cleanup;\
    }\
    printf("✓ Created threaded system with %d workers (reusable for all epochs)\\n\\n", num_threads);\
    
    }
}
