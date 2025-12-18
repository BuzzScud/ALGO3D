#!/usr/bin/env python3
"""
THESIS.md Deduplication and Reorganization Script

This script analyzes THESIS.md, identifies duplicate sections,
and creates a clean, reorganized version.
"""

import re
from collections import defaultdict
from typing import List, Dict, Tuple

def read_thesis(filepath: str) -> List[str]:
    """Read the thesis file into a list of lines."""
    with open(filepath, 'r', encoding='utf-8') as f:
        return f.readlines()

def identify_sections(lines: List[str]) -> List[Tuple[int, str, int]]:
    """
    Identify all major sections in the thesis.
    Returns list of (line_number, section_title, level)
    """
    sections = []
    for i, line in enumerate(lines):
        # Match # headers
        match = re.match(r'^(#{1,3})\s+(.+)$', line)
        if match:
            level = len(match.group(1))
            title = match.group(2).strip()
            sections.append((i, title, level))
    return sections

def find_duplicates(sections: List[Tuple[int, str, int]]) -> Dict[str, List[int]]:
    """
    Find duplicate section titles.
    Returns dict mapping title to list of line numbers.
    """
    title_map = defaultdict(list)
    for line_num, title, level in sections:
        # Normalize title for comparison
        normalized = title.upper().strip()
        # Skip very generic titles
        if normalized not in ['PART I', 'PART II', 'PART III', 'CONCLUSIONS', 'INTRODUCTION']:
            title_map[normalized].append(line_num)
    
    # Return only duplicates
    duplicates = {title: lines for title, lines in title_map.items() if len(lines) > 1}
    return duplicates

def get_section_content(lines: List[str], start_line: int, sections: List[Tuple[int, str, int]]) -> Tuple[int, List[str]]:
    """
    Extract content of a section from start_line until next section of same or higher level.
    Returns (end_line, content_lines)
    """
    start_level = None
    for line_num, title, level in sections:
        if line_num == start_line:
            start_level = level
            break
    
    if start_level is None:
        return start_line, []
    
    # Find end of section
    end_line = len(lines)
    for line_num, title, level in sections:
        if line_num > start_line and level <= start_level:
            end_line = line_num
            break
    
    return end_line, lines[start_line:end_line]

def analyze_thesis(filepath: str):
    """Analyze the thesis and print statistics."""
    print("=" * 80)
    print("THESIS.MD ANALYSIS")
    print("=" * 80)
    
    lines = read_thesis(filepath)
    print(f"\nTotal lines: {len(lines):,}")
    
    sections = identify_sections(lines)
    print(f"Total sections: {len(sections)}")
    
    # Count by level
    level_counts = defaultdict(int)
    for _, _, level in sections:
        level_counts[level] += 1
    
    print("\nSections by level:")
    for level in sorted(level_counts.keys()):
        print(f"  Level {level} ('#' * {level}): {level_counts[level]}")
    
    # Find duplicates
    duplicates = find_duplicates(sections)
    print(f"\nDuplicate sections found: {len(duplicates)}")
    
    if duplicates:
        print("\nTop duplicates:")
        sorted_dupes = sorted(duplicates.items(), key=lambda x: len(x[1]), reverse=True)
        for title, line_nums in sorted_dupes[:20]:
            print(f"  '{title}' appears {len(line_nums)} times at lines: {line_nums}")
            
            # Calculate total lines for this duplicate
            total_dupe_lines = 0
            for line_num in line_nums:
                end_line, content = get_section_content(lines, line_num, sections)
                total_dupe_lines += len(content)
            
            avg_lines = total_dupe_lines // len(line_nums)
            wasted_lines = total_dupe_lines - avg_lines
            print(f"    Average section size: {avg_lines} lines")
            print(f"    Wasted lines: {wasted_lines} lines")
    
    # Estimate total duplication
    total_duplicate_lines = 0
    for title, line_nums in duplicates.items():
        section_sizes = []
        for line_num in line_nums:
            end_line, content = get_section_content(lines, line_num, sections)
            section_sizes.append(len(content))
        
        if section_sizes:
            # Keep the largest version, count others as duplicates
            total_duplicate_lines += sum(section_sizes) - max(section_sizes)
    
    print(f"\nEstimated duplicate lines: {total_duplicate_lines:,}")
    print(f"Estimated clean size: {len(lines) - total_duplicate_lines:,} lines")
    print(f"Reduction: {(total_duplicate_lines / len(lines) * 100):.1f}%")
    
    return lines, sections, duplicates

def create_section_map(lines: List[str], sections: List[Tuple[int, str, int]]) -> Dict[str, Tuple[int, int, List[str]]]:
    """
    Create a map of section titles to their content.
    Returns dict mapping normalized title to (start_line, end_line, content)
    """
    section_map = {}
    
    for i, (line_num, title, level) in enumerate(sections):
        # Get end line
        if i + 1 < len(sections):
            # Find next section of same or higher level
            end_line = len(lines)
            for j in range(i + 1, len(sections)):
                if sections[j][2] <= level:
                    end_line = sections[j][0]
                    break
        else:
            end_line = len(lines)
        
        content = lines[line_num:end_line]
        normalized_title = title.upper().strip()
        
        # If duplicate, keep the longest version
        if normalized_title in section_map:
            existing_content = section_map[normalized_title][2]
            if len(content) > len(existing_content):
                section_map[normalized_title] = (line_num, end_line, content)
        else:
            section_map[normalized_title] = (line_num, end_line, content)
    
    return section_map

if __name__ == "__main__":
    import sys
    
    filepath = "thesis/THESIS.md"
    if len(sys.argv) > 1:
        filepath = sys.argv[1]
    
    lines, sections, duplicates = analyze_thesis(filepath)
    
    print("\n" + "=" * 80)
    print("Analysis complete. Run with --deduplicate to create clean version.")
    print("=" * 80)