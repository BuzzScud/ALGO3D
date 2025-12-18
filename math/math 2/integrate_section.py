"""
Integration helper script for merging expansion documents into THESIS_INTEGRATED.md
"""

def read_file(filename):
    """Read a file and return its lines."""
    with open(filename, 'r', encoding='utf-8') as f:
        return f.readlines()

def write_file(filename, lines):
    """Write lines to a file."""
    with open(filename, 'w', encoding='utf-8') as f:
        f.writelines(lines)

def extract_section(lines, start_marker, end_marker=None):
    """Extract a section from lines between markers."""
    section = []
    in_section = False
    
    for line in lines:
        if start_marker in line:
            in_section = True
            section.append(line)
        elif end_marker and end_marker in line:
            break
        elif in_section:
            section.append(line)
    
    return section

def merge_sections(original, expansion, section_name):
    """Merge expansion content into original section."""
    print(f"Merging {section_name}...")
    print(f"Original lines: {len(original)}")
    print(f"Expansion lines: {len(expansion)}")
    
    # For now, just append expansion after original
    # In production, would do intelligent merging
    merged = original + ["\n", f"### EXPANDED CONTENT FROM {section_name}\n", "\n"] + expansion
    
    print(f"Merged lines: {len(merged)}")
    return merged

if __name__ == '__main__':
    print("Integration helper loaded. Use functions to merge content.")
