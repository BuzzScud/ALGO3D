#!/bin/bash

# Backup thesis
cp thesis/THESIS.md thesis/THESIS_BACKUP_BEFORE_SECTION_14_15.md

# Find the line number for Section 14
LINE=$(grep -n "^## 14\. KISSING SPHERES AND OPTIMAL PACKING" thesis/THESIS.md | head -1 | cut -d: -f1)

echo "Found Section 14 at line: $LINE"

# Delete the empty Section 14 heading and insert new content
# We'll delete lines from Section 14 heading to just before Section 15 heading
SECTION_15_LINE=$(grep -n "^## 15\. INFINITE PLATONIC SOLID GENERATOR" thesis/THESIS.md | head -1 | cut -d: -f1)

echo "Found Section 15 at line: $SECTION_15_LINE"

# Calculate lines to delete (from Section 14 to just before Section 15)
START_DELETE=$LINE
END_DELETE=$((SECTION_15_LINE - 1))

echo "Deleting lines $START_DELETE to $END_DELETE"

# Create temporary file with new content
{
    # Keep everything before Section 14
    sed -n "1,$((START_DELETE - 1))p" thesis/THESIS.md
    
    # Insert new Section 14-15 content
    cat THESIS_SECTION_14_15_PLATONIC_DUALITY.md
    
    # Add separator
    echo ""
    echo "---"
    echo ""
    
    # Keep everything from Section 15 onwards
    sed -n "${SECTION_15_LINE},\$p" thesis/THESIS.md
    
} > thesis/THESIS_NEW.md

# Replace original with new version
mv thesis/THESIS_NEW.md thesis/THESIS.md

echo "Section 14-15 inserted successfully!"
echo "Backup saved as: thesis/THESIS_BACKUP_BEFORE_SECTION_14_15.md"

# Show line count change
OLD_LINES=$(wc -l < thesis/THESIS_BACKUP_BEFORE_SECTION_14_15.md)
NEW_LINES=$(wc -l < thesis/THESIS.md)
ADDED_LINES=$((NEW_LINES - OLD_LINES))

echo "Old thesis: $OLD_LINES lines"
echo "New thesis: $NEW_LINES lines"
echo "Added: $ADDED_LINES lines"