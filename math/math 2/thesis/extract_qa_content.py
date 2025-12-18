#!/usr/bin/env python3
"""
Q&A Content Extraction Script

This script extracts Q&A content from THESIS.md and organizes it by section
for integration into the main body.
"""

import re
import os
from typing import Dict, List, Tuple

def read_thesis(filepath: str = "thesis/THESIS.md") -> List[str]:
    """Read thesis file."""
    with open(filepath, 'r', encoding='utf-8') as f:
        return f.readlines()

def find_qa_sections(lines: List[str]) -> Dict[str, Tuple[int, int]]:
    """
    Find all Q&A sections and their line ranges.
    Returns dict mapping section name to (start_line, end_line).
    """
    qa_sections = {}
    
    # Define Q&A section patterns
    patterns = {
        'clock_lattice': r'# CLOCK LATTICE QUESTIONS',
        'crystalline_abacus': r'# CRYSTALLINE ABACUS QUESTIONS',
        'novel_hashing': r'# NOVEL HASHING QUESTIONS',
        'bitcoin': r'# BITCOIN.*BLOCKCHAIN QUESTIONS',
        'ai_applications': r'# AI APPLICATIONS QUESTIONS',
        'additional_topics': r'# ADDITIONAL TOPICS QUESTIONS',
        'foundational_1': r'# FOUNDATIONAL QUESTIONS - COMPLETE ANSWERS',
        'foundational_2': r'# FOUNDATIONAL QUESTIONS - PART 2',
        'geometric_arithmetic': r'# GEOMETRIC ARITHMETIC QUESTIONS'
    }
    
    # Find each section
    for name, pattern in patterns.items():
        for i, line in enumerate(lines):
            if re.search(pattern, line):
                # Find end of section (next major heading or end of file)
                end = len(lines)
                for j in range(i + 1, len(lines)):
                    if re.match(r'^# [A-Z]', lines[j]) and not re.search(pattern, lines[j]):
                        end = j
                        break
                
                if name not in qa_sections:
                    qa_sections[name] = (i, end)
                    print(f"Found {name}: lines {i} to {end} ({end-i} lines)")
    
    return qa_sections

def extract_questions(lines: List[str], start: int, end: int) -> Dict[int, List[str]]:
    """
    Extract individual questions from a Q&A section.
    Returns dict mapping question number to content lines.
    """
    questions = {}
    current_q = None
    current_content = []
    
    for i in range(start, end):
        line = lines[i]
        
        # Check for question header
        match = re.match(r'^##\s+QUESTION\s+(\d+):', line, re.IGNORECASE)
        if match:
            # Save previous question
            if current_q is not None:
                questions[current_q] = current_content
            
            # Start new question
            current_q = int(match.group(1))
            current_content = [line]
        elif current_q is not None:
            current_content.append(line)
    
    # Save last question
    if current_q is not None:
        questions[current_q] = current_content
    
    return questions

def save_extracted_content(qa_sections: Dict[str, Tuple[int, int]], 
                          lines: List[str],
                          output_dir: str = "thesis/extracted_qa"):
    """
    Save extracted Q&A content to separate files.
    """
    os.makedirs(output_dir, exist_ok=True)
    
    for section_name, (start, end) in qa_sections.items():
        # Extract questions
        questions = extract_questions(lines, start, end)
        
        # Save to file
        output_file = os.path.join(output_dir, f"{section_name}.md")
        with open(output_file, 'w', encoding='utf-8') as f:
            f.write(f"# {section_name.upper().replace('_', ' ')} - EXTRACTED Q&A\n\n")
            f.write(f"Total Questions: {len(questions)}\n")
            f.write(f"Total Lines: {end - start}\n\n")
            f.write("---\n\n")
            
            for q_num in sorted(questions.keys()):
                f.writelines(questions[q_num])
                f.write("\n---\n\n")
        
        print(f"Saved {section_name}: {len(questions)} questions to {output_file}")

def create_integration_templates(qa_sections: Dict[str, Tuple[int, int]],
                                lines: List[str],
                                output_dir: str = "thesis/integration_templates"):
    """
    Create integration templates for each section.
    """
    os.makedirs(output_dir, exist_ok=True)
    
    # Mapping of Q&A sections to target thesis sections
    integration_map = {
        'clock_lattice': {
            'target_section': '5. CLOCK LATTICE STRUCTURE',
            'current_lines': 600,
            'target_lines': 8000,
            'description': 'Deep dive into 12-fold symmetry, geometric optimality, and clock lattice properties'
        },
        'crystalline_abacus': {
            'target_section': '7. CRYSTALLINE ABACUS',
            'current_lines': 66,
            'target_lines': 6000,
            'description': 'Complete computational model with all 15 questions integrated'
        },
        'geometric_arithmetic': {
            'target_section': '4. GEOMETRIC ARITHMETIC',
            'current_lines': 1138,
            'target_lines': 5000,
            'description': 'All 25 operations with detailed examples and proofs'
        },
        'novel_hashing': {
            'target_section': '17. NOVEL HASHING ALGORITHMS',
            'current_lines': 100,
            'target_lines': 5500,
            'description': 'Complete hashing algorithms with security analysis'
        },
        'bitcoin': {
            'target_section': '18. BITCOIN AND BLOCKCHAIN',
            'current_lines': 100,
            'target_lines': 4000,
            'description': 'Blockchain applications and optimizations'
        },
        'ai_applications': {
            'target_section': '19. AI ARCHITECTURE',
            'current_lines': 100,
            'target_lines': 2500,
            'description': 'AI/ML applications and optimizations'
        }
    }
    
    for section_name, (start, end) in qa_sections.items():
        if section_name not in integration_map:
            continue
        
        info = integration_map[section_name]
        questions = extract_questions(lines, start, end)
        
        # Create template
        template_file = os.path.join(output_dir, f"{section_name}_template.md")
        with open(template_file, 'w', encoding='utf-8') as f:
            f.write(f"# INTEGRATION TEMPLATE: {section_name.upper()}\n\n")
            f.write(f"## Target Section: {info['target_section']}\n\n")
            f.write(f"**Current Size**: {info['current_lines']} lines\n")
            f.write(f"**Target Size**: {info['target_lines']} lines\n")
            f.write(f"**Growth**: {info['target_lines'] - info['current_lines']} lines ")
            f.write(f"({(info['target_lines'] - info['current_lines']) / info['current_lines'] * 100:.0f}%)\n\n")
            f.write(f"**Description**: {info['description']}\n\n")
            f.write(f"**Questions to Integrate**: {len(questions)}\n\n")
            f.write("---\n\n")
            f.write("## Integration Structure\n\n")
            
            # Create subsection structure based on questions
            for q_num in sorted(questions.keys()):
                q_content = questions[q_num]
                # Extract question title
                for line in q_content:
                    if line.startswith('## QUESTION'):
                        title = line.split(':', 1)[1].strip() if ':' in line else ''
                        f.write(f"### Subsection {q_num}: {title}\n")
                        f.write(f"- Source: Question {q_num}\n")
                        f.write(f"- Lines: ~{len(q_content)}\n")
                        f.write(f"- Integration: Convert Q&A to narrative format\n\n")
                        break
            
            f.write("\n---\n\n")
            f.write("## Extracted Questions\n\n")
            
            for q_num in sorted(questions.keys()):
                f.writelines(questions[q_num])
                f.write("\n---\n\n")
        
        print(f"Created template for {section_name}: {template_file}")

def main():
    """Main extraction process."""
    print("=" * 80)
    print("Q&A CONTENT EXTRACTION")
    print("=" * 80)
    print()
    
    # Read thesis
    print("Reading THESIS.md...")
    lines = read_thesis()
    print(f"Total lines: {len(lines):,}")
    print()
    
    # Find Q&A sections
    print("Finding Q&A sections...")
    qa_sections = find_qa_sections(lines)
    print(f"Found {len(qa_sections)} Q&A sections")
    print()
    
    # Save extracted content
    print("Extracting and saving Q&A content...")
    save_extracted_content(qa_sections, lines)
    print()
    
    # Create integration templates
    print("Creating integration templates...")
    create_integration_templates(qa_sections, lines)
    print()
    
    print("=" * 80)
    print("EXTRACTION COMPLETE")
    print("=" * 80)
    print()
    print("Output directories:")
    print("  - thesis/extracted_qa/ - Extracted Q&A content")
    print("  - thesis/integration_templates/ - Integration templates")
    print()
    print("Next step: Use templates to integrate content into main sections")

if __name__ == "__main__":
    main()