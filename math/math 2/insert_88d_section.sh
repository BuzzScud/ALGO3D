#!/bin/bash

# Insert 88D architecture section into thesis
# Location: After Section 13 (line 45116), before Section 14

THESIS_FILE="thesis/THESIS.md"
SECTION_FILE="THESIS_SECTION_88D_ARCHITECTURE.md"
BACKUP_FILE="thesis/THESIS_BACKUP_BEFORE_88D_INSERT.md"
OUTPUT_FILE="thesis/THESIS_NEW.md"

echo "Creating backup..."
cp "$THESIS_FILE" "$BACKUP_FILE"

echo "Splitting thesis at line 45116..."
head -n 45115 "$THESIS_FILE" > thesis_part1.tmp
tail -n +45116 "$THESIS_FILE" > thesis_part2.tmp

echo "Assembling new thesis..."
cat thesis_part1.tmp > "$OUTPUT_FILE"
echo "" >> "$OUTPUT_FILE"
echo "---" >> "$OUTPUT_FILE"
echo "" >> "$OUTPUT_FILE"
cat "$SECTION_FILE" >> "$OUTPUT_FILE"
echo "" >> "$OUTPUT_FILE"
echo "---" >> "$OUTPUT_FILE"
echo "" >> "$OUTPUT_FILE"
cat thesis_part2.tmp >> "$OUTPUT_FILE"

echo "Replacing original thesis..."
mv "$OUTPUT_FILE" "$THESIS_FILE"

echo "Cleaning up..."
rm thesis_part1.tmp thesis_part2.tmp

echo "Done! 88D section inserted."
echo "Backup saved to: $BACKUP_FILE"