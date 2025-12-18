#!/bin/bash
echo "=== FINAL COMPREHENSIVE AUDIT ==="
echo ""
echo "Repository Size:"
du -sh /workspace
echo ""
echo "=== Directory Breakdown ==="
du -sh /workspace/*/ 2>/dev/null | sort -hr | head -20
echo ""
echo "=== Large Files (>1MB) ==="
find /workspace -type f -size +1M ! -path "*/.git/*" ! -path "*/outputs/*" -exec ls -lh {} \; | awk '{print $5, $9}' | sort -hr | head -20
echo ""
echo "=== Remaining Test Files ==="
echo "Total: $(find /workspace/tests -name "*.c" | wc -l)"
echo "In Makefile: $(grep -o "test_[a-zA-Z_0-9]*" tests/Makefile 2>/dev/null | sort | uniq | wc -l)"
echo ""
echo "=== Documentation Files ==="
ls -lh /workspace/*.md 2>/dev/null | awk '{print $5, $9}'
echo ""
echo "=== Build Artifacts ==="
find /workspace -name "*.o" -o -name "*.so" -o -name "*.a" 2>/dev/null | wc -l
