#!/bin/bash

# Set library path
export LD_LIBRARY_PATH=/workspace:$LD_LIBRARY_PATH

echo "=== GDB Threading Analysis ==="
echo "Running training under GDB with automatic interruption..."
echo ""

# Create GDB command file for automated analysis
cat > gdb_auto_commands.txt << 'EOF'
set pagination off
set logging file gdb_thread_analysis.log
set logging enabled on

# Run the program
run

# After timeout, GDB will be interrupted and we can analyze
EOF

# Run with timeout - will be killed after 20 seconds
timeout --signal=INT 20s gdb -batch -x gdb_auto_commands.txt --args ./tools/cllm train -d data/all_training -v 500 -e 128 -l 4 -H 8 -E 3 2>&1 | tee gdb_run_output.log

echo ""
echo "=== Analysis Complete ==="
echo "Output saved to gdb_run_output.log"
echo ""

# Now let's do a manual interrupt analysis
echo "=== Running Interactive GDB Session ==="
cat > gdb_interrupt_commands.txt << 'EOF'
set pagination off
handle SIGINT stop print
run
EOF

# This will run and we'll manually interrupt it
echo "Starting program... will auto-interrupt after 20 seconds"
(
  sleep 20
  pkill -INT -f "gdb.*cllm"
) &

gdb -x gdb_interrupt_commands.txt --args ./tools/cllm train -d data/all_training -v 500 -e 128 -l 4 -H 8 -E 3 << 'GDBEOF' 2>&1 | tee gdb_interactive_output.log
# After interrupt, analyze threads
info threads
thread apply all bt
print *system->work_queue
quit
GDBEOF

echo ""
echo "=== Interactive analysis complete ==="
echo "Results in gdb_interactive_output.log"