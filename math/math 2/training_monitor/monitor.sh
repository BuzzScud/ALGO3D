#!/bin/bash

echo "=== CLLM 64-Core Training Monitor ==="
echo "Started: $(date)"
echo ""

while true; do
    clear
    echo "=== CLLM 64-Core Training Monitor ==="
    echo "Time: $(date)"
    echo ""
    
    # Check if process is running
    if ! ps -p 957 > /dev/null 2>&1; then
        echo "Training process completed or stopped!"
        break
    fi
    
    # Thread count
    THREAD_COUNT=$(ps -eLf | grep -c "train_model")
    echo "Active Threads: $THREAD_COUNT"
    
    # CPU usage
    CPU_USAGE=$(ps -p 957 -o %cpu | tail -1)
    echo "CPU Usage: ${CPU_USAGE}%"
    
    # Memory usage
    MEM_USAGE=$(ps -p 957 -o rss | tail -1)
    MEM_MB=$((MEM_USAGE / 1024))
    echo "Memory: ${MEM_MB} MB"
    
    # Runtime
    RUNTIME=$(ps -p 957 -o etime | tail -1)
    echo "Runtime: $RUNTIME"
    
    # Thread states
    echo ""
    echo "Thread States:"
    ps -eLf -p 957 | awk 'NR>1 {print $2}' | sort | uniq -c
    
    # Check log file
    echo ""
    echo "Log file size: $(ls -lh training_monitor/64core_training.log | awk '{print $5}')"
    echo "Last log line: $(tail -1 training_monitor/64core_training.log 2>/dev/null || echo 'No output yet')"
    
    sleep 10
done

echo ""
echo "=== Training Complete ==="
echo "Final log:"
tail -50 training_monitor/64core_training.log