#!/bin/bash
echo "=== Training Monitor ==="
echo ""
echo "Process Status:"
ps aux | grep "train_model" | grep -v grep | grep -v monitor | awk '{print "  PID: " $2 "  CPU Time: " $10 "  Memory: " $6/1024 "MB  Status: " $8}'
echo ""
echo "Checkpoints:"
ls -lh checkpoints/*.cllm 2>/dev/null | awk '{print "  " $9 " (" $5 ")"}'
if [ $? -ne 0 ]; then
    echo "  No model checkpoints yet"
fi
echo ""
echo "Latest log entries:"
tail -10 full_training.log | grep -v "^$"
echo ""
echo "Files in checkpoints:"
ls -lh checkpoints/ 2>/dev/null | tail -5