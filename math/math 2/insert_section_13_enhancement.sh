#!/bin/bash

# Script to insert Section 13.5 enhancement into the thesis

THESIS_FILE="thesis/THESIS.md"
SECTION_FILE="THESIS_SECTION_13_ENHANCEMENT.md"
BACKUP_FILE="thesis/THESIS_BACKUP_BEFORE_SECTION_13_ENHANCEMENT.md"
OUTPUT_FILE="thesis/THESIS_UPDATED.md"

# Create backup
echo "Creating backup..."
cp "$THESIS_FILE" "$BACKUP_FILE"

# Find the line number where Section 14 starts
SECTION_14_LINE=$(grep -n "^## 14. KISSING SPHERES AND OPTIMAL PACKING$" "$THESIS_FILE" | head -1 | cut -d: -f1)

if [ -z "$SECTION_14_LINE" ]; then
    echo "Error: Could not find Section 14 header"
    exit 1
fi

echo "Found Section 14 at line $SECTION_14_LINE"

# Extract everything before Section 14
head -n $((SECTION_14_LINE - 1)) "$THESIS_FILE" > "$OUTPUT_FILE"

# Add the new Section 13.5 content
cat "$SECTION_FILE" >> "$OUTPUT_FILE"

# Add a blank line
echo "" >> "$OUTPUT_FILE"

# Add everything from Section 14 onwards
tail -n +$SECTION_14_LINE "$THESIS_FILE" >> "$OUTPUT_FILE"

# Replace original with updated
mv "$OUTPUT_FILE" "$THESIS_FILE"

echo "Section 13.5 successfully inserted!"
echo "Backup saved to: $BACKUP_FILE"
echo "Updated thesis: $THESIS_FILE"

# Show statistics
echo ""
echo "Statistics:"
echo "Original thesis lines: $(wc -l < "$BACKUP_FILE")"
echo "New section lines: $(wc -l < "$SECTION_FILE")"
echo "Updated thesis lines: $(wc -l < "$THESIS_FILE")"