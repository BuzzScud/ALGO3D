#!/usr/bin/env python3
"""
Deep Integration Script - Integrate Q&A content into main thesis sections

This script extracts content from Q&A sections and integrates it into
the appropriate main body sections of THESIS.md.
"""

import re
from typing import List, Dict, Tuple

def read_file(filepath: str) -> List[str]:
    """Read file into list of lines."""
    with open(filepath, 'r', encoding='utf-8') as f:
        return f.readlines()

def write_file(filepath: str, lines: List[str]):
    """Write lines to file."""
    with open(filepath, 'w', encoding='utf-8') as f:
        f.writelines(lines)

def find_section_boundaries(lines: List[str], section_title: str) -> Tuple[int, int]:
    """
    Find start and end line numbers for a section.
    Returns (start_line, end_line)
    """
    start = -1
    end = len(lines)
    
    # Find start
    for i, line in enumerate(lines):
        if section_title.lower() in line.lower() and line.startswith('#'):
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

def extract_qa_content(lines: List[str], start: int, end: int) -> Dict[str, List[str]]:
    """
    Extract Q&A content organized by question.
    Returns dict mapping question number to content lines.
    """
    qa_content = {}
    current_question = None
    current_content = []
    
    for i in range(start, end):
        line = lines[i]
        
        # Check if this is a question header
        match = re.match(r'^##\s+QUESTION\s+(\d+):', line, re.IGNORECASE)
        if match:
            # Save previous question
            if current_question:
                qa_content[current_question] = current_content
            
            # Start new question
            current_question = int(match.group(1))
            current_content = [line]
        elif current_question:
            current_content.append(line)
    
    # Save last question
    if current_question:
        qa_content[current_question] = current_content
    
    return qa_content

def create_integration_plan():
    """
    Create a detailed plan for integrating Q&A content.
    """
    plan = {
        'clock_lattice': {
            'qa_section': 'CLOCK LATTICE QUESTIONS',
            'target_section': '## 5. CLOCK LATTICE STRUCTURE',
            'questions': list(range(1, 21)),  # 20 questions
            'integration_strategy': 'expand_subsections'
        },
        'crystalline_abacus': {
            'qa_section': 'CRYSTALLINE ABACUS QUESTIONS',
            'target_section': '## 6. CRYSTALLINE ABACUS',
            'questions': list(range(1, 16)),  # 15 questions
            'integration_strategy': 'expand_subsections'
        },
        'geometric_arithmetic': {
            'qa_section': 'GEOMETRIC ARITHMETIC QUESTIONS',
            'target_section': '## 4. GEOMETRIC ARITHMETIC',
            'questions': list(range(1, 26)),  # 25 questions
            'integration_strategy': 'expand_subsections'
        },
        'novel_hashing': {
            'qa_section': 'NOVEL HASHING QUESTIONS',
            'target_section': '## 17. NOVEL HASHING',
            'questions': list(range(1, 16)),  # 15 questions
            'integration_strategy': 'expand_subsections'
        },
        'bitcoin': {
            'qa_section': 'BITCOIN.*BLOCKCHAIN QUESTIONS',
            'target_section': '## 18. BITCOIN',
            'questions': list(range(1, 11)),  # 10 questions
            'integration_strategy': 'expand_subsections'
        },
        'ai': {
            'qa_section': 'AI APPLICATIONS QUESTIONS',
            'target_section': '## 19. AI',
            'questions': list(range(1, 7)),  # 6 questions
            'integration_strategy': 'expand_subsections'
        }
    }
    
    return plan

def analyze_integration_needs(filepath: str):
    """
    Analyze the thesis and print integration recommendations.
    """
    print("=" * 80)
    print("DEEP INTEGRATION ANALYSIS")
    print("=" * 80)
    
    lines = read_file(filepath)
    plan = create_integration_plan()
    
    print(f"\nTotal lines in THESIS.md: {len(lines):,}")
    print(f"\nIntegration targets identified: {len(plan)}")
    
    for topic, config in plan.items():
        print(f"\n{topic.upper()}:")
        print(f"  Q&A Section: {config['qa_section']}")
        print(f"  Target Section: {config['target_section']}")
        print(f"  Questions to integrate: {len(config['questions'])}")
        
        # Find Q&A section
        qa_start, qa_end = find_section_boundaries(lines, config['qa_section'])
        if qa_start != -1:
            qa_lines = qa_end - qa_start
            print(f"  Q&A content: {qa_lines:,} lines")
        else:
            print(f"  Q&A content: NOT FOUND")
        
        # Find target section
        target_start, target_end = find_section_boundaries(lines, config['target_section'])
        if target_start != -1:
            target_lines = target_end - target_start
            print(f"  Current target size: {target_lines:,} lines")
            if qa_start != -1:
                print(f"  Potential integrated size: ~{target_lines + qa_lines:,} lines")
        else:
            print(f"  Target section: NOT FOUND")
    
    print("\n" + "=" * 80)
    print("RECOMMENDATION:")
    print("=" * 80)
    print("""
This is a massive integration task. The Q&A sections contain ~30,000+ lines
of detailed content that should be integrated into ~6 main sections.

Recommended approach:
1. Start with one section as pilot (Clock Lattice recommended)
2. Manually integrate to establish quality standard
3. Use this script to extract and organize content
4. Apply same approach to remaining sections
5. Remove Q&A sections after integration complete

Estimated time: 15-20 hours of focused work
Expected result: ~60,000-70,000 line comprehensive treatise
""")

if __name__ == "__main__":
    import sys
    
    filepath = "thesis/THESIS.md"
    if len(sys.argv) > 1:
        filepath = sys.argv[1]
    
    analyze_integration_needs(filepath)