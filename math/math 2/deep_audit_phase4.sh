#!/bin/bash
echo "=== PHASE 4 DEEP AUDIT ==="
echo ""
echo "Current repository size:"
du -sh /workspace
echo ""
echo "=== Tracked Files Analysis ==="
echo "Total tracked files: $(git ls-files | wc -l)"
echo ""
echo "=== Tracked Files by Extension ==="
git ls-files | sed 's/.*\.//' | sort | uniq -c | sort -rn | head -20
echo ""
echo "=== Remaining Large Tracked Files (>100KB) ==="
git ls-files | while read f; do
  if [ -f "$f" ]; then
    size=$(stat -c %s "$f" 2>/dev/null || stat -f %z "$f" 2>/dev/null)
    if [ "$size" -gt 102400 ]; then
      echo "$size $f"
    fi
  fi
done | sort -rn | awk '{printf "%.2fMB %s\n", $1/1024/1024, $2}' | head -20
echo ""
echo "=== Documentation Files ==="
git ls-files | grep "\.md$" | grep -v "^docs/"
echo ""
echo "=== Text/Data Files ==="
git ls-files | grep -E "\.txt$|\.json$|\.csv$" | head -20
