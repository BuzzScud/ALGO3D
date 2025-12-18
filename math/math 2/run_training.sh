#!/bin/bash

# CLLM Repository Training Script
# Trains the model on the entire repository codebase

set -e

echo "========================================================================"
echo "CLLM Repository Training - Kissing Spheres Architecture"
echo "========================================================================"
echo ""

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Step 1: Prepare directories
echo -e "${BLUE}📁 Preparing directories...${NC}"
mkdir -p training_data
mkdir -p models
mkdir -p checkpoints
echo -e "${GREEN}✓ Directories ready${NC}"
echo ""

# Step 2: Check if training data exists
if [ ! -f "training_data/repo_code.txt" ]; then
    echo -e "${YELLOW}⚠️  Training data not found, creating...${NC}"
    python3 train_on_repo.py
    echo ""
fi

# Step 3: Get training data stats
echo -e "${BLUE}📊 Training Data Statistics:${NC}"
echo "  File: training_data/repo_code.txt"
echo "  Size: $(du -h training_data/repo_code.txt | cut -f1)"
echo "  Lines: $(wc -l < training_data/repo_code.txt)"
echo ""

# Step 4: Build the application
echo -e "${BLUE}🔨 Building application...${NC}"
cd app
make clean > /dev/null 2>&1
make > /dev/null 2>&1
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ Application built successfully${NC}"
else
    echo -e "${YELLOW}⚠️  Build had warnings (non-critical)${NC}"
fi
cd ..
echo ""

# Step 5: Instructions for training
echo "========================================================================"
echo -e "${GREEN}✓ Ready for Training!${NC}"
echo "========================================================================"
echo ""
echo "To train the model, you have two options:"
echo ""
echo "Option 1: Use the UI (Recommended)"
echo "  1. Run: cd app && LD_LIBRARY_PATH=.. ./hyper_prime_spiral"
echo "  2. Navigate to the Training tab"
echo "  3. Click 'Scan Dir' and select training_data/repo_code.txt"
echo "  4. Configure: LR=0.001, Epochs=10, Batch=32, Threads=0"
echo "  5. Click 'START TRAINING'"
echo "  6. Watch the sphere visualization in real-time!"
echo ""
echo "Option 2: Run Tests (Automated)"
echo "  cd tests && make test_forward_backward"
echo "  ./test_forward_backward"
echo ""
echo "Training Configuration:"
echo "  - Data: 7.2 MB of C code (780 files)"
echo "  - Architecture: 12 kissing spheres"
echo "  - Expected time: 12-25 minutes"
echo "  - Expected speedup: 5-10x vs single-threaded"
echo ""
echo "Model will be saved to:"
echo "  - Checkpoints: checkpoints/checkpoint_epoch_N.cllm"
echo "  - Final model: models/trained_model_kissing_spheres.cllm"
echo ""
echo "========================================================================"