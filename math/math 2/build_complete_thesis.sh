#!/bin/bash
# Build complete thesis from individual chapters

echo "Building complete thesis..."

OUTPUT="thesis/THESIS_COMPLETE.md"

# Start with main document
cat thesis/THESIS_MAIN.md > "$OUTPUT"

echo "" >> "$OUTPUT"
echo "---" >> "$OUTPUT"
echo "" >> "$OUTPUT"

# Add each part
for part in thesis/part_*/; do
    echo "Adding $(basename $part)..."
    
    # Add part heading
    if [ -f "$part/README.md" ]; then
        head -1 "$part/README.md" >> "$OUTPUT"
        echo "" >> "$OUTPUT"
    fi
    
    # Add each chapter in the part
    for chapter in "$part"/chapter_*/content.md; do
        if [ -f "$chapter" ]; then
            echo "  Adding $(basename $(dirname $chapter))..."
            cat "$chapter" >> "$OUTPUT"
            echo "" >> "$OUTPUT"
            echo "---" >> "$OUTPUT"
            echo "" >> "$OUTPUT"
        fi
    done
done

echo "Complete thesis built: $OUTPUT"
wc -l "$OUTPUT"
