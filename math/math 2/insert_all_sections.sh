#!/bin/bash

# Insert all new sections into thesis
# Order: 5.5 (Duality), 5.6 (Points vs Units), 5.7 (Quadrant Folding), 5.8 (Phase Angles)
# Location: After Section 5 (Clock Lattice), before Section 6

THESIS_FILE="thesis/THESIS.md"
BACKUP_FILE="thesis/THESIS_BACKUP_BEFORE_ALL_INSERTS.md"

echo "Creating backup..."
cp "$THESIS_FILE" "$BACKUP_FILE"

echo "Finding insertion point (end of Section 5)..."
# Find where Section 5 ends (before "## 5. O(1) DETERMINISTIC PRIME GENERATION" or "## 6.")
SECTION5_END=$(grep -n "^## 5\. O(1) DETERMINISTIC PRIME GENERATION\|^## 6\." "$THESIS_FILE" | head -1 | cut -d: -f1)

if [ -z "$SECTION5_END" ]; then
    echo "Error: Could not find end of Section 5"
    exit 1
fi

echo "Section 5 ends at line $SECTION5_END"
echo "Inserting new sections before line $SECTION5_END..."

# Calculate insertion point (one line before)
INSERT_LINE=$((SECTION5_END - 1))

echo "Splitting thesis at line $INSERT_LINE..."
head -n $INSERT_LINE "$THESIS_FILE" > thesis_part1.tmp
tail -n +$SECTION5_END "$THESIS_FILE" > thesis_part2.tmp

echo "Assembling new thesis with all sections..."
cat thesis_part1.tmp > thesis_new.tmp
echo "" >> thesis_new.tmp
echo "---" >> thesis_new.tmp
echo "" >> thesis_new.tmp

# Insert Section 5.5: Duality
echo "Inserting Section 5.5: Duality..."
cat THESIS_SECTION_DUALITY.md >> thesis_new.tmp
echo "" >> thesis_new.tmp
echo "---" >> thesis_new.tmp
echo "" >> thesis_new.tmp

# Insert Section 5.6: Points vs Units
echo "Inserting Section 5.6: Points vs Units..."
cat THESIS_SECTION_POINTS_VS_UNITS.md >> thesis_new.tmp
echo "" >> thesis_new.tmp
echo "---" >> thesis_new.tmp
echo "" >> thesis_new.tmp

# Insert Section 5.7: Quadrant Folding
echo "Inserting Section 5.7: Quadrant Folding..."
cat THESIS_SECTION_QUADRANT_FOLDING.md >> thesis_new.tmp
echo "" >> thesis_new.tmp
echo "---" >> thesis_new.tmp
echo "" >> thesis_new.tmp

# Insert Section 5.8: Phase Angles
echo "Inserting Section 5.8: Phase Angles..."
cat THESIS_SECTION_PHASE_ANGLES.md >> thesis_new.tmp
echo "" >> thesis_new.tmp
echo "---" >> thesis_new.tmp
echo "" >> thesis_new.tmp

# Add rest of thesis
cat thesis_part2.tmp >> thesis_new.tmp

echo "Replacing original thesis..."
mv thesis_new.tmp "$THESIS_FILE"

echo "Cleaning up..."
rm thesis_part1.tmp thesis_part2.tmp

echo ""
echo "Done! All sections inserted."
echo "Backup saved to: $BACKUP_FILE"
echo ""
echo "Checking new thesis size..."
wc -l "$THESIS_FILE"