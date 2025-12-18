#!/usr/bin/env python3
"""
Clean up duplicate includes added by the fix script
"""

import re
import sys
from pathlib import Path

def cleanup_file(filepath):
    """Remove duplicate includes from a file"""
    with open(filepath, 'r') as f:
        lines = f.readlines()
    
    # Track seen includes
    seen_includes = set()
    cleaned_lines = []
    
    for line in lines:
        # Check if it's an include line
        include_match = re.match(r'^\s*#include\s+[<"]([^>"]+)[>"]', line)
        
        if include_match:
            include_path = include_match.group(1)
            if include_path not in seen_includes:
                seen_includes.add(include_path)
                cleaned_lines.append(line)
            # else: skip duplicate
        else:
            cleaned_lines.append(line)
    
    # Write back
    with open(filepath, 'w') as f:
        f.writelines(cleaned_lines)
    
    return len(lines) - len(cleaned_lines)

def main():
    # Find all C files in src/ai and src/ai/platonic
    files = list(Path('src/ai').glob('*.c')) + list(Path('src/ai/platonic').glob('*.c'))
    
    total_removed = 0
    for filepath in files:
        removed = cleanup_file(filepath)
        if removed > 0:
            print(f"✓ {filepath}: removed {removed} duplicate includes")
            total_removed += removed
    
    print(f"\nTotal duplicates removed: {total_removed}")

if __name__ == '__main__':
    main()