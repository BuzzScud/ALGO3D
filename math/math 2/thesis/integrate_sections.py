#!/usr/bin/env python3
"""
Section Integration Script

This script performs the actual deep integration of Q&A content into thesis sections.
"""

import re
import os
from typing import List, Dict, Tuple

def read_file(filepath: str) -> List[str]:
    """Read file into list of lines."""
    with open(filepath, 'r', encoding='utf-8') as f:
        return f.readlines()

def write_file(filepath: str, lines: List[str]):
    """Write lines to file."""
    with open(filepath, 'w', encoding='utf-8') as f:
        f.writelines(lines)

def find_section_in_thesis(lines: List[str], section_marker: str) -> Tuple[int, int]:
    """
    Find a section in the thesis by its marker.
    Returns (start_line, end_line).
    """
    start = -1
    end = len(lines)
    
    # Find start
    for i, line in enumerate(lines):
        if section_marker in line and line.startswith('##'):
            start = i
            break
    
    if start == -1:
        return (-1, -1)
    
    # Find end (next section of same or higher level)
    start_level = len(re.match(r'^#+', lines[start]).group())
    for i in range(start + 1, len(lines)):
        match = re.match(r'^#+', lines[i])
        if match and len(match.group()) <= start_level:
            end = i
            break
    
    return (start, end)

def convert_qa_to_narrative(qa_content: List[str], section_name: str) -> List[str]:
    """
    Convert Q&A format to narrative format.
    This is the core integration logic.
    """
    narrative = []
    current_subsection = 1
    
    # Add section header
    narrative.append(f"## {section_name}\n\n")
    
    # Process each question
    in_question = False
    question_num = 0
    question_title = ""
    question_content = []
    
    for line in qa_content:
        # Detect question header
        if re.match(r'^##\s+QUESTION\s+(\d+):', line, re.IGNORECASE):
            # Save previous question if exists
            if question_content:
                narrative.extend(format_question_as_narrative(
                    question_num, question_title, question_content, current_subsection
                ))
                current_subsection += 1
                question_content = []
            
            # Extract question info
            match = re.match(r'^##\s+QUESTION\s+(\d+):\s*(.+)$', line, re.IGNORECASE)
            if match:
                question_num = int(match.group(1))
                question_title = match.group(2).strip()
                in_question = True
        elif in_question:
            question_content.append(line)
    
    # Save last question
    if question_content:
        narrative.extend(format_question_as_narrative(
            question_num, question_title, question_content, current_subsection
        ))
    
    return narrative

def format_question_as_narrative(q_num: int, title: str, content: List[str], 
                                subsection_num: int) -> List[str]:
    """
    Format a single question as narrative text.
    """
    narrative = []
    
    # Create subsection header
    narrative.append(f"### {subsection_num}. {title}\n\n")
    
    # Process content
    for line in content:
        # Skip empty question headers
        if re.match(r'^##\s+QUESTION', line, re.IGNORECASE):
            continue
        
        # Convert ### headers to #### (increase nesting)
        if line.startswith('###'):
            line = '#' + line
        
        # Keep the content
        narrative.append(line)
    
    narrative.append("\n")
    return narrative

def integrate_section(thesis_lines: List[str], 
                     section_marker: str,
                     qa_file: str,
                     section_name: str) -> List[str]:
    """
    Integrate Q&A content into a thesis section.
    """
    print(f"\nIntegrating {section_name}...")
    
    # Find section in thesis
    start, end = find_section_in_thesis(thesis_lines, section_marker)
    if start == -1:
        print(f"  ERROR: Section '{section_marker}' not found in thesis")
        return thesis_lines
    
    print(f"  Found section at lines {start} to {end} ({end-start} lines)")
    
    # Read Q&A content
    if not os.path.exists(qa_file):
        print(f"  ERROR: Q&A file not found: {qa_file}")
        return thesis_lines
    
    qa_content = read_file(qa_file)
    print(f"  Read Q&A content: {len(qa_content)} lines")
    
    # Convert Q&A to narrative
    narrative = convert_qa_to_narrative(qa_content, section_name)
    print(f"  Converted to narrative: {len(narrative)} lines")
    
    # Replace section in thesis
    new_thesis = thesis_lines[:start] + narrative + thesis_lines[end:]
    print(f"  New section size: {len(narrative)} lines (was {end-start} lines)")
    print(f"  Growth: {len(narrative) - (end-start)} lines ({((len(narrative) - (end-start)) / (end-start) * 100):.0f}%)")
    
    return new_thesis

def perform_all_integrations():
    """
    Perform all section integrations.
    """
    print("=" * 80)
    print("DEEP SECTION INTEGRATION")
    print("=" * 80)
    
    # Read thesis
    print("\nReading THESIS.md...")
    thesis_lines = read_file("thesis/THESIS.md")
    print(f"Original size: {len(thesis_lines):,} lines")
    
    # Define integrations
    integrations = [
        {
            'section_marker': '7. CRYSTALLINE ABACUS',
            'qa_file': 'thesis/extracted_qa/crystalline_abacus.md',
            'section_name': '7. CRYSTALLINE ABACUS: THE COMPUTATIONAL MODEL'
        },
        {
            'section_marker': '4. GEOMETRIC ARITHMETIC',
            'qa_file': 'thesis/extracted_qa/geometric_arithmetic.md',
            'section_name': '4. GEOMETRIC ARITHMETIC: THE FOUNDATION'
        },
        {
            'section_marker': '17. NOVEL HASHING',
            'qa_file': 'thesis/extracted_qa/novel_hashing.md',
            'section_name': '17. NOVEL HASHING ALGORITHMS'
        },
        {
            'section_marker': '18. BITCOIN',
            'qa_file': 'thesis/extracted_qa/bitcoin.md',
            'section_name': '18. BITCOIN AND BLOCKCHAIN SOLUTIONS'
        },
        {
            'section_marker': '19. AI',
            'qa_file': 'thesis/extracted_qa/ai_applications.md',
            'section_name': '19. AI ARCHITECTURE AND APPLICATIONS'
        }
    ]
    
    # Perform integrations
    for integration in integrations:
        thesis_lines = integrate_section(
            thesis_lines,
            integration['section_marker'],
            integration['qa_file'],
            integration['section_name']
        )
    
    # Save integrated thesis
    backup_file = "thesis/THESIS_BEFORE_DEEP_INTEGRATION.md"
    print(f"\nCreating backup: {backup_file}")
    write_file(backup_file, read_file("thesis/THESIS.md"))
    
    output_file = "thesis/THESIS_INTEGRATED.md"
    print(f"Writing integrated thesis: {output_file}")
    write_file(output_file, thesis_lines)
    
    print(f"\nFinal size: {len(thesis_lines):,} lines")
    print(f"Growth: {len(thesis_lines) - len(read_file('thesis/THESIS.md')):,} lines")
    print(f"Percentage: {((len(thesis_lines) - len(read_file('thesis/THESIS.md'))) / len(read_file('thesis/THESIS.md')) * 100):.1f}%")
    
    print("\n" + "=" * 80)
    print("INTEGRATION COMPLETE")
    print("=" * 80)
    print(f"\nOriginal: thesis/THESIS.md")
    print(f"Backup: {backup_file}")
    print(f"Integrated: {output_file}")
    print("\nReview the integrated version, then replace THESIS.md if satisfied.")

if __name__ == "__main__":
    perform_all_integrations()