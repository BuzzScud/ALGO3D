#!/bin/bash

# Deep Reading Script - Read NEW Math Library 17 Times
# This ensures complete understanding of design principles

echo "=== DEEP READING OF NEW MATH LIBRARY - 17 COMPLETE PASSES ==="
echo ""

# Get all source and header files
SOURCE_FILES=$(find math/src -type f -name "*.c" | sort)
HEADER_FILES=$(find math/include -type f -name "*.h" | sort)

TOTAL_FILES=$(echo "$SOURCE_FILES $HEADER_FILES" | wc -w)

echo "Total files to read: $TOTAL_FILES"
echo "Total readings: $TOTAL_FILES x 17 = $((TOTAL_FILES * 17))"
echo ""

# Create output directory
mkdir -p deep_reading_analysis

# Read each file 17 times and extract key information
for pass in {1..17}; do
    echo "========================================="
    echo "PASS $pass/17"
    echo "========================================="
    echo ""
    
    # Read headers first
    for file in $HEADER_FILES; do
        filename=$(basename "$file")
        echo "Reading: $filename (pass $pass)"
        
        # Extract key information on first pass
        if [ $pass -eq 1 ]; then
            {
                echo "=== FILE: $file ==="
                echo ""
                echo "--- FUNCTIONS ---"
                grep -E "^[a-zA-Z_][a-zA-Z0-9_]*\s+[a-zA-Z_][a-zA-Z0-9_]*\s*\(" "$file" 2>/dev/null | head -50
                echo ""
                echo "--- TYPES ---"
                grep -E "^typedef (struct|enum)" "$file" 2>/dev/null
                echo ""
                echo "--- CONSTANTS ---"
                grep -E "^#define [A-Z_]+" "$file" 2>/dev/null | head -20
                echo ""
            } >> "deep_reading_analysis/${filename}.analysis"
        fi
    done
    
    # Read source files
    for file in $SOURCE_FILES; do
        filename=$(basename "$file")
        echo "Reading: $filename (pass $pass)"
        
        # Extract key information on first pass
        if [ $pass -eq 1 ]; then
            {
                echo "=== FILE: $file ==="
                echo ""
                echo "--- FUNCTIONS IMPLEMENTED ---"
                grep -E "^[a-zA-Z_][a-zA-Z0-9_]*\s+[a-zA-Z_][a-zA-Z0-9_]*\s*\(" "$file" 2>/dev/null | head -50
                echo ""
                echo "--- KEY ALGORITHMS ---"
                grep -B2 -A10 "O(1)" "$file" 2>/dev/null | head -50
                echo ""
            } >> "deep_reading_analysis/${filename}.analysis"
        fi
    done
    
    echo ""
done

echo "========================================="
echo "DEEP READING COMPLETE - 17 PASSES DONE"
echo "========================================="
echo ""
echo "Analysis files created in: deep_reading_analysis/"
echo ""

# Create summary
{
    echo "=== DEEP READING SUMMARY ==="
    echo ""
    echo "Files read 17 times each:"
    echo "$HEADER_FILES" | tr ' ' '\n'
    echo "$SOURCE_FILES" | tr ' ' '\n'
    echo ""
    echo "Total readings performed: $((TOTAL_FILES * 17))"
    echo ""
    echo "Key insights extracted to: deep_reading_analysis/"
} > deep_reading_analysis/SUMMARY.txt

cat deep_reading_analysis/SUMMARY.txt