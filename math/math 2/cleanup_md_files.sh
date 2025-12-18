#!/bin/bash

# Essential files to KEEP
KEEP_FILES=(
    "MASTER_PLAN.md"
    "PRIMARY_OBJECTIVES.md"
    "SECONDARY_OBJECTIVES.md"
    "TERTIARY_OBJECTIVES.md"
    "README.md"
    "LICENSE"
    "AUDIT.md"
    "todo.md"
)

# Delete all other MD files in root
echo "Cleaning up extraneous MD files in root directory..."
for file in *.md; do
    if [[ ! " ${KEEP_FILES[@]} " =~ " ${file} " ]]; then
        echo "Deleting: $file"
        rm -f "$file"
    fi
done

# Clean up docs/archive (keep the directory but remove old files)
echo ""
echo "Cleaning up docs/archive..."
if [ -d "docs/archive" ]; then
    rm -rf docs/archive/*
    echo "Cleaned docs/archive/"
fi

# Clean up thesis backup files
echo ""
echo "Cleaning up thesis backup files..."
cd thesis
for file in THESIS_*.md; do
    if [ "$file" != "THESIS.md" ]; then
        echo "Deleting: thesis/$file"
        rm -f "$file"
    fi
done

# Clean up thesis Q&A files (already integrated)
echo ""
echo "Cleaning up integrated Q&A files..."
for file in *_QUESTIONS.md; do
    echo "Deleting: thesis/$file"
    rm -f "$file"
done

# Clean up thesis documentation files
for file in *_COMPLETE.md *_PLAN.md *_REPORT.md *_SUMMARY.md; do
    if [ -f "$file" ]; then
        echo "Deleting: thesis/$file"
        rm -f "$file"
    fi
done

cd ..

echo ""
echo "Cleanup complete!"
echo ""
echo "Remaining files:"
ls -lh *.md 2>/dev/null | wc -l
echo "Disk usage:"
df -h .
