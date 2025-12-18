#!/bin/bash

# Build complete thesis from individual chapters
# This script concatenates all chapter content files into THESIS_COMPLETE.md

OUTPUT="THESIS_COMPLETE.md"
TEMP="THESIS_TEMP.md"

echo "Building complete thesis..."

# Start with main header
cat THESIS_MAIN.md > $TEMP

# Add separator
echo -e "\n\n---\n\n# COMPLETE THESIS CONTENT\n\n" >> $TEMP

# Process each part in order
for part_dir in part_*/; do
    if [ -d "$part_dir" ]; then
        echo "Processing $part_dir..."
        
        # Add part header
        part_name=$(basename "$part_dir")
        echo -e "\n\n# ${part_name//_/ }\n\n" >> $TEMP
        
        # Process each chapter in the part
        for chapter_dir in "$part_dir"chapter_*/; do
            if [ -d "$chapter_dir" ]; then
                chapter_name=$(basename "$chapter_dir")
                echo "  - $chapter_name"
                
                # Add chapter header
                echo -e "\n\n## ${chapter_name//_/ }\n\n" >> $TEMP
                
                # Add chapter content if it exists
                if [ -f "$chapter_dir/content.md" ]; then
                    cat "$chapter_dir/content.md" >> $TEMP
                    echo -e "\n" >> $TEMP
                fi
            fi
        done
    fi
done

# Move temp to output
mv $TEMP $OUTPUT

# Get statistics
total_lines=$(wc -l < $OUTPUT)
total_size=$(du -h $OUTPUT | cut -f1)

echo ""
echo "✅ Build complete!"
echo "   Output: $OUTPUT"
echo "   Lines: $total_lines"
echo "   Size: $total_size"