#!/bin/bash

# This script consolidates the inference implementation
# Moving thread-centric code from cllm_inference_threaded.c to cllm_inference.c
# and removing _threaded suffixes

echo "Step 1: Copy helper functions from threaded file to main file"

# Extract helper functions (signal_all_threads, wait_for_completion, sample_top_k, sample_top_p)
# These are at the beginning of cllm_inference_threaded.c

echo "Step 2: Replace cllm_generate with thread-centric version"

# The new cllm_generate will call cllm_generate_token in a loop

echo "Step 3: Add cllm_generate_token function"

# This will be the main token generation function

echo "Step 4: Remove _threaded suffixes from function names"

# Update header file
sed -i 's/cllm_generate_token_threaded/cllm_generate_token/g' cllm/include/ai/cllm_inference.h
sed -i 's/cllm_generate_threaded/cllm_generate/g' cllm/include/ai/cllm_inference.h

# Update implementation file
sed -i 's/cllm_generate_token_threaded/cllm_generate_token/g' cllm/src/cllm_inference_threaded.c
sed -i 's/cllm_generate_threaded/cllm_generate/g' cllm/src/cllm_inference_threaded.c

echo "Step 5: Update comments to remove references to 'threaded'"

sed -i 's/thread-local computation/thread-centric computation/g' cllm/include/ai/cllm_inference.h
sed -i 's/88D thread pool/thread pool/g' cllm/include/ai/cllm_inference.h

echo "Done!"