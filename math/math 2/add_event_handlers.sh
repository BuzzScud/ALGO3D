#!/bin/bash

# Add mouse motion and mouse up handlers to all converted tabs

TABS=("benchmark" "downloaded_files" "research" "url_manager")

for tab in "${TABS[@]}"; do
    echo "Processing tab_${tab}..."
    
    # Check if handlers already exist
    if grep -q "handle_${tab}_tab_mouse_motion" "app/ui/tabs/tab_${tab}.c"; then
        echo "  Handlers already exist, skipping..."
        continue
    fi
    
    echo "  Adding handlers to tab_${tab}.c and tab_${tab}.h"
done

echo "Done!"