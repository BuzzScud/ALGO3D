#!/usr/bin/env python3
"""
Generate comprehensive table of contents for THESIS_FULLY_INTEGRATED.md
"""

def read_file(filename):
    with open(filename, 'r', encoding='utf-8') as f:
        return f.readlines()

def generate_toc(lines):
    """Generate table of contents from markdown headers."""
    toc = []
    toc.append("# COMPREHENSIVE TABLE OF CONTENTS\n\n")
    
    current_part = None
    section_num = 0
    subsection_num = 0
    
    for i, line in enumerate(lines, 1):
        if line.startswith('# ') and 'PART' in line:
            current_part = line.strip('# \n')
            toc.append(f"\n## {current_part}\n\n")
            section_num = 0
        elif line.startswith('## '):
            section_num += 1
            subsection_num = 0
            title = line.strip('# \n')
            toc.append(f"  {section_num}. {title} (Line {i})\n")
        elif line.startswith('### '):
            subsection_num += 1
            title = line.strip('# \n')
            toc.append(f"    {section_num}.{subsection_num} {title} (Line {i})\n")
    
    return toc

# Read the integrated thesis
lines = read_file('THESIS_FULLY_INTEGRATED.md')
print(f"Processing {len(lines)} lines...")

# Generate TOC
toc = generate_toc(lines)
print(f"Generated TOC with {len(toc)} entries")

# Write TOC
with open('TABLE_OF_CONTENTS.md', 'w', encoding='utf-8') as f:
    f.writelines(toc)

print("Table of contents written to TABLE_OF_CONTENTS.md")

# Also print summary
print("\n=== DOCUMENT STRUCTURE ===")
parts = [line for line in lines if line.startswith('# PART')]
print(f"Parts: {len(parts)}")
sections = [line for line in lines if line.startswith('## ')]
print(f"Sections: {len(sections)}")
subsections = [line for line in lines if line.startswith('### ')]
print(f"Subsections: {len(subsections)}")
