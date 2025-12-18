#!/usr/bin/env python3
"""
Integrate remaining major sections including Clock Lattice and Foundational questions.
"""

import re
import os
from typing import List, Tuple

def read_file(filepath: str) -> List[str]:
    with open(filepath, 'r', encoding='utf-8') as f:
        return f.readlines()

def write_file(filepath: str, lines: List[str]):
    with open(filepath, 'w', encoding='utf-8') as f:
        f.writelines(lines)

def find_section(lines: List[str], marker: str) -> Tuple[int, int]:
    """Find section by marker."""
    start = -1
    for i, line in enumerate(lines):
        if marker in line and line.startswith('##'):
            start = i
            break
    
    if start == -1:
        return (-1, -1)
    
    # Find end
    start_level = len(re.match(r'^#+', lines[start]).group())
    end = len(lines)
    for i in range(start + 1, len(lines)):
        match = re.match(r'^#+', lines[i])
        if match and len(match.group()) <= start_level:
            end = i
            break
    
    return (start, end)

def convert_qa_to_narrative(qa_content: List[str], section_name: str) -> List[str]:
    """Convert Q&A to narrative format."""
    narrative = []
    narrative.append(f"## {section_name}\n\n")
    
    current_subsection = 1
    in_question = False
    question_content = []
    question_title = ""
    
    for line in qa_content:
        if re.match(r'^##\s+QUESTION\s+(\d+):', line, re.IGNORECASE):
            # Save previous
            if question_content:
                narrative.append(f"### {current_subsection}. {question_title}\n\n")
                for content_line in question_content:
                    if not re.match(r'^##\s+QUESTION', content_line, re.IGNORECASE):
                        if content_line.startswith('###'):
                            narrative.append('#' + content_line)
                        else:
                            narrative.append(content_line)
                narrative.append("\n")
                current_subsection += 1
                question_content = []
            
            # Extract new question
            match = re.match(r'^##\s+QUESTION\s+(\d+):\s*(.+)$', line, re.IGNORECASE)
            if match:
                question_title = match.group(2).strip()
                in_question = True
        elif in_question:
            question_content.append(line)
    
    # Save last question
    if question_content:
        narrative.append(f"### {current_subsection}. {question_title}\n\n")
        for content_line in question_content:
            if not re.match(r'^##\s+QUESTION', content_line, re.IGNORECASE):
                if content_line.startswith('###'):
                    narrative.append('#' + content_line)
                else:
                    narrative.append(content_line)
        narrative.append("\n")
    
    return narrative

def integrate_clock_lattice(thesis_lines: List[str]) -> List[str]:
    """
    Integrate Clock Lattice questions into Section 2 (Babylonian Mathematics)
    or create new comprehensive Clock Lattice section.
    """
    print("\nIntegrating Clock Lattice Questions...")
    
    # Read Clock Lattice Q&A
    qa_file = 'thesis/extracted_qa/clock_lattice.md'
    if not os.path.exists(qa_file):
        print(f"  ERROR: {qa_file} not found")
        return thesis_lines
    
    qa_content = read_file(qa_file)
    print(f"  Read Q&A: {len(qa_content)} lines (20 questions)")
    
    # Convert to narrative
    narrative = convert_qa_to_narrative(qa_content, 
        "5. CLOCK LATTICE STRUCTURE: COMPREHENSIVE ANALYSIS")
    print(f"  Converted to narrative: {len(narrative)} lines")
    
    # Find where to insert (after Section 4 or replace existing Section 5)
    start, end = find_section(thesis_lines, '5. O(1) DETERMINISTIC PRIME')
    if start != -1:
        print(f"  Found Section 5 at lines {start} to {end}")
        # Insert before Section 5
        new_thesis = thesis_lines[:start] + narrative + ["\n"] + thesis_lines[start:]
        print(f"  Inserted {len(narrative)} lines before Section 5")
    else:
        print("  ERROR: Could not find insertion point")
        return thesis_lines
    
    return new_thesis

def integrate_foundational(thesis_lines: List[str]) -> List[str]:
    """
    Integrate Foundational questions throughout relevant sections.
    """
    print("\nIntegrating Foundational Questions...")
    
    # Read both parts
    qa_file1 = 'thesis/extracted_qa/foundational_1.md'
    qa_file2 = 'thesis/extracted_qa/foundational_2.md'
    
    if not os.path.exists(qa_file1) or not os.path.exists(qa_file2):
        print("  ERROR: Foundational Q&A files not found")
        return thesis_lines
    
    qa1 = read_file(qa_file1)
    qa2 = read_file(qa_file2)
    print(f"  Read Part 1: {len(qa1)} lines")
    print(f"  Read Part 2: {len(qa2)} lines")
    
    # Combine
    combined = qa1 + ["\n"] + qa2
    
    # Convert to narrative
    narrative = convert_qa_to_narrative(combined,
        "2. BABYLONIAN MATHEMATICS: DEEP FOUNDATIONS")
    print(f"  Converted to narrative: {len(narrative)} lines")
    
    # Find Section 2 and expand it
    start, end = find_section(thesis_lines, '2. BABYLONIAN MATHEMATICS')
    if start != -1:
        print(f"  Found Section 2 at lines {start} to {end} ({end-start} lines)")
        # Replace with expanded version
        new_thesis = thesis_lines[:start] + narrative + thesis_lines[end:]
        print(f"  Replaced with {len(narrative)} lines")
        print(f"  Growth: {len(narrative) - (end-start)} lines")
    else:
        print("  ERROR: Could not find Section 2")
        return thesis_lines
    
    return new_thesis

def main():
    print("=" * 80)
    print("INTEGRATING REMAINING MAJOR SECTIONS")
    print("=" * 80)
    
    # Read current integrated thesis
    print("\nReading THESIS_INTEGRATED.md...")
    thesis_lines = read_file("thesis/THESIS_INTEGRATED.md")
    print(f"Current size: {len(thesis_lines):,} lines")
    
    # Integrate Clock Lattice
    thesis_lines = integrate_clock_lattice(thesis_lines)
    print(f"After Clock Lattice: {len(thesis_lines):,} lines")
    
    # Integrate Foundational
    thesis_lines = integrate_foundational(thesis_lines)
    print(f"After Foundational: {len(thesis_lines):,} lines")
    
    # Save
    output_file = "thesis/THESIS_FULLY_INTEGRATED.md"
    print(f"\nWriting: {output_file}")
    write_file(output_file, thesis_lines)
    
    original_size = len(read_file("thesis/THESIS.md"))
    print(f"\nOriginal: {original_size:,} lines")
    print(f"Final: {len(thesis_lines):,} lines")
    print(f"Growth: {len(thesis_lines) - original_size:,} lines ({((len(thesis_lines) - original_size) / original_size * 100):.1f}%)")
    
    print("\n" + "=" * 80)
    print("INTEGRATION COMPLETE")
    print("=" * 80)

if __name__ == "__main__":
    main()