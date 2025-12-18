#!/bin/bash
# Batch migration script for Phase 1 Week 2

# Files to process
files=(
    "algorithms/src/backprop.c"
    "algorithms/src/batch_processing.c"
    "algorithms/src/lattice_embeddings.c"
    "algorithms/src/lattice_sphere_positions.c"
    "algorithms/src/angular_attention.c"
    "algorithms/src/threading.c"
    "algorithms/src/shared_memory.c"
    "algorithms/src/lock_free_queue.c"
    "algorithms/src/memory_management.c"
    "algorithms/src/sphere_packing.c"
    "algorithms/src/sphere_threading.c"
)

echo "Scanning files for OLD library references..."
echo ""

for file in "${files[@]}"; do
    if [ -f "$file" ]; then
        echo "=== $file ==="
        echo "Lines: $(wc -l < "$file")"
        echo "OLD references:"
        grep -o "prime_[a-z_]*" "$file" 2>/dev/null | sort | uniq -c | sort -rn || echo "  None found"
        echo ""
    fi
done