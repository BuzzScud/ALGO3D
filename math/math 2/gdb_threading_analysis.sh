#!/bin/bash

# GDB script to analyze threading deadlock

export LD_LIBRARY_PATH=/workspace:/workspace/algorithms:$LD_LIBRARY_PATH

# Create GDB commands file
cat > gdb_commands.txt << 'EOF'
# Set breakpoint after workers are created
break threaded_train_epoch_lockfree
commands
  silent
  printf "=== EPOCH START ===\n"
  continue
end

# Run until we hit the deadlock
run

# When it hangs (after 5 seconds), interrupt
# This will be done manually with Ctrl-C

# Commands to run after interrupt:
# info threads
# thread apply all bt
# print *system->work_queue
# print system->work_queue->head
# print system->work_queue->tail
# print system->work_queue->epoch_done
# thread 2
# print *ctx
# print ctx->sphere_id
# print batches_processed
EOF

echo "Starting GDB analysis..."
echo "The program will run and likely hang."
echo "After 10 seconds, we'll interrupt it and examine thread states."

# Run with timeout
timeout 15 gdb -batch -x gdb_commands.txt --args ./tools/cllm train \
  -d data/all_training \
  -v 500 \
  -e 128 \
  -l 4 \
  -H 8 \
  -E 3 \
  -b 4 \
  -s 32 \
  -r 0.0001 \
  -c models/gdb_test \
  2>&1 | tee gdb_output.log

echo ""
echo "GDB analysis complete. Check gdb_output.log for results."