#!/bin/bash

# Backup thesis
cp thesis/THESIS.md thesis/THESIS_BACKUP_BEFORE_ABSTRACT_INTRO_UPDATE.md

echo "Creating updated thesis with new Abstract and Introduction..."

# Extract the new Abstract (lines 5-95 of the update file)
sed -n '5,95p' THESIS_UPDATED_INTRODUCTION_ABSTRACT.md > /tmp/new_abstract.txt

# Extract the new Introduction (lines 99-end of the update file)
sed -n '99,$p' THESIS_UPDATED_INTRODUCTION_ABSTRACT.md > /tmp/new_introduction.txt

# Find line numbers
ABSTRACT_START=$(grep -n "^## ABSTRACT" thesis/THESIS.md | head -1 | cut -d: -f1)
ABSTRACT_END=$(grep -n "^## TABLE OF CONTENTS" thesis/THESIS.md | head -1 | cut -d: -f1)
INTRO_START=$(grep -n "^## 1\. INTRODUCTION" thesis/THESIS.md | head -1 | cut -d: -f1)
INTRO_END=$(grep -n "^## 2\. BABYLONIAN MATHEMATICS" thesis/THESIS.md | head -1 | cut -d: -f1)

echo "Abstract: lines $ABSTRACT_START to $((ABSTRACT_END - 1))"
echo "Introduction: lines $INTRO_START to $((INTRO_END - 1))"

# Create new thesis
{
    # Keep title and header (lines 1 to ABSTRACT_START-1)
    sed -n "1,$((ABSTRACT_START - 1))p" thesis/THESIS.md
    
    # Insert new Abstract
    cat /tmp/new_abstract.txt
    
    echo ""
    
    # Keep Table of Contents (lines ABSTRACT_END to INTRO_START-1)
    sed -n "$ABSTRACT_END,$((INTRO_START - 1))p" thesis/THESIS.md
    
    # Insert new Introduction
    cat /tmp/new_introduction.txt
    
    echo ""
    
    # Keep rest of thesis (from INTRO_END onwards)
    sed -n "${INTRO_END},\$p" thesis/THESIS.md
    
} > thesis/THESIS_NEW.md

# Replace original
mv thesis/THESIS_NEW.md thesis/THESIS.md

# Cleanup
rm /tmp/new_abstract.txt /tmp/new_introduction.txt

echo "Abstract and Introduction updated successfully!"

# Show line count change
OLD_LINES=$(wc -l < thesis/THESIS_BACKUP_BEFORE_ABSTRACT_INTRO_UPDATE.md)
NEW_LINES=$(wc -l < thesis/THESIS.md)
DIFF_LINES=$((NEW_LINES - OLD_LINES))

echo "Old thesis: $OLD_LINES lines"
echo "New thesis: $NEW_LINES lines"
if [ $DIFF_LINES -gt 0 ]; then
    echo "Added: $DIFF_LINES lines"
else
    echo "Removed: $((-DIFF_LINES)) lines"
fi