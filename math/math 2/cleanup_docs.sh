#!/bin/bash

# COMPREHENSIVE DOCUMENTATION CLEANUP SCRIPT
# Removes ephemeral status reports, session summaries, and duplicate documents
# Keeps only essential architectural and mathematical documents

echo "Starting comprehensive documentation cleanup..."

# Create archive directory for historical reference
mkdir -p docs/archive

# Files to KEEP (essential documents)
KEEP_FILES=(
    "THESIS.md"
    "MASTER_PLAN.md"
    "SECONDARY_OBJECTIVES.md"
    "TERTIARY_OBJECTIVES.md"
    "README.md"
    "LICENSE"
    "AUDIT.md"
    "todo.md"
)

# Move all root MD files to archive first
echo "Archiving all root MD files..."
for file in *.md; do
    # Skip if it's in the keep list
    skip=0
    for keep in "${KEEP_FILES[@]}"; do
        if [ "$file" = "$keep" ]; then
            skip=1
            break
        fi
    done
    
    if [ $skip -eq 0 ]; then
        mv "$file" docs/archive/ 2>/dev/null
    fi
done

echo "Cleanup complete!"
echo "Kept files:"
ls -1 *.md
echo ""
echo "Archived files: $(ls -1 docs/archive/*.md | wc -l)"