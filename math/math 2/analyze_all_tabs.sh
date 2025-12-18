#!/bin/bash

echo "=== COMPREHENSIVE TAB ANALYSIS ==="
echo ""

tabs=(
    "tab_training"
    "tab_llm"
    "tab_models"
    "tab_crawler"
    "tab_research"
    "tab_url_manager"
    "tab_downloaded_files"
    "tab_video"
    "tab_benchmark"
)

for tab in "${tabs[@]}"; do
    echo "=== $tab ==="
    
    # Current line count
    current_lines=$(wc -l "app/ui/tabs/${tab}.c" 2>/dev/null | awk '{print $1}')
    
    if [ -z "$current_lines" ]; then
        echo "  ❌ File not found"
        echo ""
        continue
    fi
    
    echo "  Current: $current_lines lines"
    
    # Find most recent commit that modified this file
    last_commit=$(git log --all --oneline -- "app/ui/tabs/${tab}.c" | head -1 | awk '{print $1}')
    
    if [ -n "$last_commit" ]; then
        echo "  Last modified: $last_commit"
        
        # Get line count from that commit
        old_lines=$(git show "$last_commit:app/ui/tabs/${tab}.c" 2>/dev/null | wc -l)
        echo "  Previous: $old_lines lines"
        
        # Calculate difference
        diff=$((current_lines - old_lines))
        if [ $diff -gt 0 ]; then
            echo "  Change: +$diff lines (EXPANDED)"
        elif [ $diff -lt 0 ]; then
            echo "  Change: $diff lines (REDUCED)"
        else
            echo "  Change: No change"
        fi
    fi
    
    # Check for key features
    echo "  Features:"
    grep -q "CrawlerURLManager" "app/ui/tabs/${tab}.c" && echo "    ✓ URL Manager integration"
    grep -q "model_selector\|ModelSelector" "app/ui/tabs/${tab}.c" && echo "    ✓ Model selector"
    grep -q "CrystallineButton" "app/ui/tabs/${tab}.c" && echo "    ✓ Crystalline UI"
    grep -q "prime_config\|PrimeConfig" "app/ui/tabs/${tab}.c" && echo "    ✓ Prime configuration"
    grep -q "SQLite\|sqlite" "app/ui/tabs/${tab}.c" && echo "    ✓ SQLite database"
    
    echo ""
done