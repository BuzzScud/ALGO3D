#!/bin/bash

# Script to insert Section 15 into the thesis

THESIS_FILE="thesis/THESIS.md"
SECTION_FILE="THESIS_SECTION_15_COMPLETE.md"
BACKUP_FILE="thesis/THESIS_BACKUP_BEFORE_SECTION_15.md"
OUTPUT_FILE="thesis/THESIS_UPDATED.md"

# Create backup
echo "Creating backup..."
cp "$THESIS_FILE" "$BACKUP_FILE"

# Find the line number where Section 15 header is
SECTION_15_LINE=$(grep -n "^## 15. INFINITE PLATONIC SOLID GENERATOR$" "$THESIS_FILE" | head -1 | cut -d: -f1)

if [ -z "$SECTION_15_LINE" ]; then
    echo "Error: Could not find Section 15 header"
    exit 1
fi

echo "Found Section 15 at line $SECTION_15_LINE"

# Find the next section (Section 16)
SECTION_16_LINE=$(grep -n "^## 16." "$THESIS_FILE" | head -1 | cut -d: -f1)

if [ -z "$SECTION_16_LINE" ]; then
    echo "Error: Could not find Section 16"
    exit 1
fi

echo "Found Section 16 at line $SECTION_16_LINE"

# Extract everything before Section 15
head -n $((SECTION_15_LINE - 1)) "$THESIS_FILE" > "$OUTPUT_FILE"

# Add the new Section 15 content
cat "$SECTION_FILE" >> "$OUTPUT_FILE"

# Add a blank line
echo "" >> "$OUTPUT_FILE"

# Add everything from Section 16 onwards
tail -n +$SECTION_16_LINE "$THESIS_FILE" >> "$OUTPUT_FILE"

# Replace original with updated
mv "$OUTPUT_FILE" "$THESIS_FILE"

echo "Section 15 successfully inserted!"
echo "Backup saved to: $BACKUP_FILE"
echo "Updated thesis: $THESIS_FILE"

# Show statistics
echo ""
echo "Statistics:"
echo "Original thesis lines: $(wc -l < "$BACKUP_FILE")"
echo "New section lines: $(wc -l < "$SECTION_FILE")"
echo "Updated thesis lines: $(wc -l < "$THESIS_FILE")"