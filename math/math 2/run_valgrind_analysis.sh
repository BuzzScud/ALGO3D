#!/bin/bash

# Valgrind Memory Analysis Script
# Runs training under valgrind to detect memory issues

export LD_LIBRARY_PATH=/workspace:/workspace/algorithms:$LD_LIBRARY_PATH

echo "=== Valgrind Memory Analysis ==="
echo ""
echo "Running short training session under valgrind..."
echo "This will take several minutes..."
echo ""

# Run 3 epochs of training under valgrind
valgrind \
  --leak-check=full \
  --show-leak-kinds=all \
  --track-origins=yes \
  --verbose \
  --log-file=valgrind_output.log \
  ./tools/cllm-simple train \
    -d data/all_training \
    -v 500 \
    -e 128 \
    -l 4 \
    -H 8 \
    -E 3 \
    -b 4 \
    -s 32 \
    -r 0.0001 \
    -t 1 \
    -c models/valgrind_test

echo ""
echo "✅ Valgrind analysis complete"
echo ""
echo "=== Summary ==="
grep -A 5 "LEAK SUMMARY" valgrind_output.log
echo ""
echo "=== Error Summary ==="
grep -A 10 "ERROR SUMMARY" valgrind_output.log
echo ""
echo "Full report: valgrind_output.log"