#!/bin/bash

# Backup
cp thesis/THESIS.md thesis/THESIS_BACKUP_ABSTRACT_ONLY.md

echo "Updating Abstract only..."

# Find Abstract boundaries
ABSTRACT_START=$(grep -n "^## ABSTRACT" thesis/THESIS.md | head -1 | cut -d: -f1)
TOC_START=$(grep -n "^## TABLE OF CONTENTS" thesis/THESIS.md | head -1 | cut -d: -f1)

echo "Abstract: lines $ABSTRACT_START to $((TOC_START - 1))"

# Extract new abstract (lines 5-95 from update file, which is just the abstract content)
sed -n '7,67p' THESIS_UPDATED_INTRODUCTION_ABSTRACT.md > /tmp/new_abstract_content.txt

# Create new thesis
{
    # Keep everything before Abstract
    sed -n "1,$((ABSTRACT_START - 1))p" thesis/THESIS.md
    
    # Insert Abstract heading
    echo "## ABSTRACT"
    echo ""
    
    # Insert new abstract content
    cat /tmp/new_abstract_content.txt
    
    # Keep everything from TOC onwards
    sed -n "${TOC_START},\$p" thesis/THESIS.md
    
} > thesis/THESIS_NEW.md

mv thesis/THESIS_NEW.md thesis/THESIS.md
rm /tmp/new_abstract_content.txt

echo "Abstract updated!"

OLD_LINES=$(wc -l < thesis/THESIS_BACKUP_ABSTRACT_ONLY.md)
NEW_LINES=$(wc -l < thesis/THESIS.md)
echo "Old: $OLD_LINES lines"
echo "New: $NEW_LINES lines"
echo "Difference: $((NEW_LINES - OLD_LINES)) lines"