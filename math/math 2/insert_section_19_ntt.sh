#!/bin/bash

# Script to insert Section 19 NTT attention into the thesis

THESIS_FILE="thesis/THESIS.md"
SECTION_FILE="THESIS_SECTION_19_NTT_ATTENTION.md"
BACKUP_FILE="thesis/THESIS_BACKUP_BEFORE_SECTION_19_NTT.md"
OUTPUT_FILE="thesis/THESIS_UPDATED.md"

# Create backup
echo "Creating backup..."
cp "$THESIS_FILE" "$BACKUP_FILE"

# Find the line number where question 2 starts (after question 1)
QUESTION_2_LINE=$(grep -n "^### 2. How can clock lattice enable efficient model compression" "$THESIS_FILE" | head -1 | cut -d: -f1)

if [ -z "$QUESTION_2_LINE" ]; then
    echo "Error: Could not find Question 2"
    exit 1
fi

echo "Found Question 2 at line $QUESTION_2_LINE"

# Extract everything before Question 2
head -n $((QUESTION_2_LINE - 1)) "$THESIS_FILE" > "$OUTPUT_FILE"

# Add the new NTT attention content
cat "$SECTION_FILE" >> "$OUTPUT_FILE"

# Add a blank line
echo "" >> "$OUTPUT_FILE"

# Add everything from Question 2 onwards
tail -n +$QUESTION_2_LINE "$THESIS_FILE" >> "$OUTPUT_FILE"

# Replace original with updated
mv "$OUTPUT_FILE" "$THESIS_FILE"

echo "Section 19 NTT attention successfully inserted!"
echo "Backup saved to: $BACKUP_FILE"
echo "Updated thesis: $THESIS_FILE"

# Show statistics
echo ""
echo "Statistics:"
echo "Original thesis lines: $(wc -l < "$BACKUP_FILE")"
echo "New section lines: $(wc -l < "$SECTION_FILE")"
echo "Updated thesis lines: $(wc -l < "$THESIS_FILE")"