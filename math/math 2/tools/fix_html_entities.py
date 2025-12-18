#!/usr/bin/env python3
"""
HTML Entity Fixer Tool

This tool fixes HTML entity encoding issues that sometimes occur when
creating files through the AI assistant interface.

Common entities fixed:
- &amp; -> &
- &lt; -> <
- &gt; -> >
- &quot; -> "
- &#39; -> '

Usage:
    python3 tools/fix_html_entities.py <file>
    python3 tools/fix_html_entities.py src/ai/cllm_threads.c
"""

import sys
import os

def decode_html_entities(content):
    """Decode HTML entities in content."""
    replacements = {
        '&amp;': '&',
        '&lt;': '<',
        '&gt;': '>',
        '&quot;': '"',
        '&#39;': "'",
    }
    
    for entity, char in replacements.items():
        content = content.replace(entity, char)
    
    return content

def fix_file(filename):
    """Fix HTML entities in a file."""
    if not os.path.exists(filename):
        print(f"Error: File '{filename}' not found")
        return 1
    
    # Read file
    try:
        with open(filename, 'r', encoding='utf-8') as f:
            content = f.read()
    except Exception as e:
        print(f"Error reading file: {e}")
        return 1
    
    # Check if file has HTML entities
    has_entities = any(entity in content for entity in ['&amp;', '&lt;', '&gt;', '&quot;', '&#39;'])
    
    if not has_entities:
        print(f"No HTML entities found in {filename}")
        return 0
    
    # Decode entities
    decoded = decode_html_entities(content)
    
    # Write back
    try:
        with open(filename, 'w', encoding='utf-8') as f:
            f.write(decoded)
        print(f"✓ Fixed HTML entities in {filename}")
        return 0
    except Exception as e:
        print(f"Error writing file: {e}")
        return 1

def main():
    if len(sys.argv) != 2:
        print("Usage: python3 tools/fix_html_entities.py <file>")
        print("\nExample:")
        print("  python3 tools/fix_html_entities.py src/ai/cllm_threads.c")
        return 1
    
    filename = sys.argv[1]
    return fix_file(filename)

if __name__ == '__main__':
    sys.exit(main())