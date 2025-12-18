#!/bin/bash

echo "Fixing thesis structure..."

# Backup
cp thesis/THESIS.md thesis/THESIS_BACKUP_STRUCTURE_FIX.md

# The issue: Introduction section (starting at line 70) got interrupted by TOC at line 98
# We need to move the TOC to come AFTER the complete Abstract but BEFORE Introduction

# Strategy:
# 1. Keep lines 1-69 (title, header, Abstract)
# 2. Insert TOC (currently at lines 98-507)
# 3. Insert Introduction (lines 70-97 + continuation after TOC ends)
# 4. Keep rest

# Find key line numbers
INTRO_START=70
TOC_START=98
TOC_END=$(grep -n "^## PART I: THEORETICAL FOUNDATIONS" thesis/THESIS.md | head -1 | cut -d: -f1)
TOC_END=$((TOC_END - 1))

echo "Introduction starts at: $INTRO_START"
echo "TOC starts at: $TOC_START"  
echo "TOC ends at: $TOC_END"

# Find where Section 2 starts (after Introduction)
SECTION_2=$(grep -n "^## 2\. BABYLONIAN MATHEMATICS" thesis/THESIS.md | head -1 | cut -d: -f1)
echo "Section 2 starts at: $SECTION_2"

# Create properly ordered thesis
{
    # 1. Keep title, header, Abstract (lines 1-69)
    sed -n "1,69p" thesis/THESIS.md
    
    echo ""
    echo "---"
    echo ""
    
    # 2. Insert TOC (lines 98 to TOC_END)
    sed -n "${TOC_START},${TOC_END}p" thesis/THESIS.md
    
    echo ""
    echo "---"
    echo ""
    
    # 3. Insert complete Introduction from the update file
    sed -n '99,$p' THESIS_UPDATED_INTRODUCTION_ABSTRACT.md
    
    echo ""
    
    # 4. Keep rest from Section 2 onwards
    sed -n "${SECTION_2},\$p" thesis/THESIS.md
    
} > thesis/THESIS_FIXED.md

mv thesis/THESIS_FIXED.md thesis/THESIS.md

echo "Structure fixed!"

# Show line count
OLD_LINES=$(wc -l < thesis/THESIS_BACKUP_STRUCTURE_FIX.md)
NEW_LINES=$(wc -l < thesis/THESIS.md)
echo "Old: $OLD_LINES lines"
echo "New: $NEW_LINES lines"
echo "Difference: $((NEW_LINES - OLD_LINES)) lines"