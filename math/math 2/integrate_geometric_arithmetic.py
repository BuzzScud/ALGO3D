#!/usr/bin/env python3
"""
Intelligent integration of THESIS_EXPANSION_GEOMETRIC_ARITHMETIC.md into Section 4
"""

def read_file(filename):
    with open(filename, 'r', encoding='utf-8') as f:
        return f.readlines()

def find_section_boundaries(lines, section_marker):
    """Find start and end of a section."""
    start = None
    end = None
    
    for i, line in enumerate(lines):
        if section_marker in line and start is None:
            start = i
        elif start is not None and line.startswith('## ') and section_marker not in line:
            end = i
            break
    
    if end is None:
        end = len(lines)
    
    return start, end

def extract_unique_content(expansion_lines, original_lines):
    """Extract content from expansion that's not in original."""
    # Simple approach: find where expansion diverges from original
    unique_content = []
    
    # Skip the header and find where new content starts
    in_unique = False
    for i, line in enumerate(expansion_lines):
        # Look for subsections that don't exist in original
        if '### 1.2 The Nature of Number' in line:
            in_unique = True
        elif '## PART II:' in line or '# PART II:' in line:
            in_unique = True
        
        if in_unique:
            unique_content.append(line)
    
    return unique_content

# Main integration
print("Reading files...")
thesis_lines = read_file('THESIS_INTEGRATED.md')
expansion_lines = read_file('THESIS_EXPANSION_GEOMETRIC_ARITHMETIC.md')

print(f"THESIS_INTEGRATED.md: {len(thesis_lines)} lines")
print(f"THESIS_EXPANSION_GEOMETRIC_ARITHMETIC.md: {len(expansion_lines)} lines")

# Find Section 4 boundaries
start, end = find_section_boundaries(thesis_lines, '## 4. GEOMETRIC ARITHMETIC')
print(f"\nSection 4 found: lines {start} to {end} ({end-start} lines)")

# Extract unique content from expansion
unique = extract_unique_content(expansion_lines, thesis_lines[start:end])
print(f"Unique content to add: {len(unique)} lines")

# Create integrated version
integrated = (
    thesis_lines[:end] +  # Everything up to end of Section 4
    ['\n', '### 4.8 EXPANDED THEORETICAL FOUNDATIONS\n', '\n'] +
    unique +  # Add unique content
    thesis_lines[end:]  # Everything after Section 4
)

print(f"\nIntegrated version: {len(integrated)} lines")
print(f"Increase: {len(integrated) - len(thesis_lines)} lines")

# Write integrated version
with open('THESIS_INTEGRATED_V2.md', 'w', encoding='utf-8') as f:
    f.writelines(integrated)

print("\nIntegrated version written to THESIS_INTEGRATED_V2.md")
