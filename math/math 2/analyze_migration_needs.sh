#!/bin/bash

# Analysis Script - Determine migration needs for each file

echo "=== MIGRATION NEEDS ANALYSIS ==="
echo ""

# Analyze the 23 critical files using BOTH libraries
echo "Analyzing 23 files using BOTH OLD and NEW libraries..."
echo ""

while IFS= read -r file; do
    if [ -f "$file" ]; then
        echo "========================================="
        echo "FILE: $file"
        echo "========================================="
        
        # Show OLD includes
        echo "OLD includes:"
        grep '#include.*"prime_' "$file" 2>/dev/null || echo "  (none found)"
        
        # Show NEW includes
        echo ""
        echo "NEW includes:"
        grep '#include.*"math/' "$file" 2>/dev/null || echo "  (none found)"
        
        # Count prime_* function calls
        echo ""
        echo "prime_* function calls:"
        prime_calls=$(grep -o "prime_[a-z_]*(" "$file" 2>/dev/null | wc -l)
        if [ "$prime_calls" -gt 0 ]; then
            grep -o "prime_[a-z_]*(" "$file" 2>/dev/null | sort | uniq -c | sort -rn
        else
            echo "  (none found)"
        fi
        
        # Determine action needed
        echo ""
        if [ "$prime_calls" -eq 0 ]; then
            echo "ACTION: Remove OLD includes (no prime_* calls)"
        else
            echo "ACTION: Replace prime_* calls with NEW library equivalents, then remove OLD includes"
        fi
        
        echo ""
    fi
done < files_using_both.txt

echo "========================================="
echo "ANALYSIS COMPLETE"
echo "========================================="