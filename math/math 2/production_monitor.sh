#!/bin/bash

echo "=== Production Training Monitor ==="
echo "Started: $(date)"
echo ""

while true; do
    clear
    echo "╔════════════════════════════════════════════════════════════╗"
    echo "║     CLLM Production Training - Real-time Monitor          ║"
    echo "╚════════════════════════════════════════════════════════════╝"
    echo ""
    echo "Time: $(date '+%Y-%m-%d %H:%M:%S')"
    echo ""
    
    # Check if process is running
    if ps aux | grep "train_model production_training" | grep -v grep | grep -v monitor > /dev/null; then
        PID=$(ps aux | grep "train_model production_training" | grep -v grep | grep -v monitor | awk '{print $2}' | tail -1)
        CPU=$(ps aux | grep "train_model production_training" | grep -v grep | grep -v monitor | awk '{print $3}' | tail -1)
        MEM=$(ps aux | grep "train_model production_training" | grep -v grep | grep -v monitor | awk '{print $4}' | tail -1)
        TIME=$(ps aux | grep "train_model production_training" | grep -v grep | grep -v monitor | awk '{print $10}' | tail -1)
        
        echo "✓ Training Status: RUNNING"
        echo "  PID: $PID"
        echo "  CPU: ${CPU}%"
        echo "  Memory: ${MEM}%"
        echo "  Runtime: $TIME"
        echo ""
        
        # Check for checkpoint files
        echo "Checkpoints:"
        if [ -d "production_checkpoints" ]; then
            checkpoint_count=$(ls -1 production_checkpoints/*.cllm 2>/dev/null | wc -l)
            vocab_exists=$([ -f "production_checkpoints/vocab.txt" ] && echo "✓" || echo "⏳")
            dataset_exists=$([ -f "production_checkpoints/dataset.bin" ] && echo "✓" || echo "⏳")
            
            echo "  Vocabulary: $vocab_exists"
            echo "  Dataset: $dataset_exists"
            echo "  Model checkpoints: $checkpoint_count"
            
            if [ $checkpoint_count -gt 0 ]; then
                echo ""
                echo "  Latest checkpoint:"
                ls -lht production_checkpoints/*.cllm 2>/dev/null | head -1 | awk '{printf "    %s (%s) - %s %s %s\n", $9, $5, $6, $7, $8}'
            fi
        fi
        
        # Show training phase
        echo ""
        echo "Training Phase:"
        if [ ! -f "production_checkpoints/vocab.txt" ]; then
            echo "  ⏳ Building vocabulary..."
        elif [ ! -f "production_checkpoints/dataset.bin" ]; then
            echo "  ⏳ Creating dataset..."
        elif [ $checkpoint_count -eq 0 ]; then
            echo "  ⏳ Initializing model..."
        else
            echo "  🚀 Training in progress (Epoch $checkpoint_count/30)"
        fi
        
    else
        echo "✗ Training Status: NOT RUNNING"
        
        # Check if it completed
        if [ -f "production_checkpoints/checkpoint_step_0.cllm" ]; then
            echo ""
            echo "✓ Training appears to be COMPLETE!"
            model_size=$(ls -lh production_checkpoints/checkpoint_step_0.cllm 2>/dev/null | awk '{print $5}')
            echo "  Model size: $model_size"
        fi
    fi
    
    echo ""
    echo "────────────────────────────────────────────────────────────"
    echo "Press Ctrl+C to stop monitoring | Refreshing in 5 seconds..."
    
    sleep 5
done