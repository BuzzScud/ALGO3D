#!/usr/bin/env python3
"""
Integrate remaining Q&A sections (Triangulation, Self-Similarity, Blind Recovery)
into the main THESIS.md narrative.
"""

import re
import sys

def read_file(filepath):
    """Read file content."""
    with open(filepath, 'r', encoding='utf-8') as f:
        return f.read()

def write_file(filepath, content):
    """Write content to file."""
    with open(filepath, 'w', encoding='utf-8') as f:
        f.write(content)

def extract_questions(content):
    """Extract individual questions from Q&A file."""
    # Split by question headers
    questions = []
    pattern = r'## QUESTION \d+:.*?(?=## QUESTION \d+:|$)'
    matches = re.findall(pattern, content, re.DOTALL)
    return matches

def convert_qa_to_narrative(qa_content):
    """Convert Q&A format to narrative prose."""
    # Remove question headers
    narrative = re.sub(r'## QUESTION \d+: ', '#### ', qa_content)
    
    # Convert code blocks to inline formulas where appropriate
    # Keep mathematical proofs in code blocks
    
    # Remove excessive markdown formatting
    narrative = re.sub(r'\*\*([^*]+)\*\*:', r'\n**\1:**\n', narrative)
    
    # Clean up spacing
    narrative = re.sub(r'\n{3,}', '\n\n', narrative)
    
    return narrative

def integrate_triangulation(thesis_content, qa_content):
    """Integrate triangulation Q&A into thesis."""
    print("Integrating Triangulation questions...")
    
    # Find the triangulation section
    pattern = r'(## 7\. TRIANGULATION: THE UNIVERSAL METHOD.*?)(## 8\. SELF-SIMILAR STRUCTURES)'
    match = re.search(pattern, thesis_content, re.DOTALL)
    
    if not match:
        print("ERROR: Could not find Triangulation section")
        return thesis_content
    
    # Extract questions
    questions = extract_questions(qa_content)
    print(f"Found {len(questions)} triangulation questions")
    
    # Convert to narrative
    narrative_content = "\n\n".join([convert_qa_to_narrative(q) for q in questions[:10]])  # First 10 questions
    
    # Insert into thesis
    section_start = match.group(1)
    section_end = match.group(2)
    
    new_section = section_start + "\n\n### Additional Deep Analysis\n\n" + narrative_content + "\n\n" + section_end
    
    thesis_content = thesis_content.replace(match.group(0), new_section)
    
    return thesis_content

def integrate_self_similarity(thesis_content, qa_content):
    """Integrate self-similarity Q&A into thesis."""
    print("Integrating Self-Similarity questions...")
    
    # Find the self-similarity section
    pattern = r'(## 8\. SELF-SIMILAR STRUCTURES: THE RECURSIVE PRINCIPLE.*?)(## \d+\.)'
    match = re.search(pattern, thesis_content, re.DOTALL)
    
    if not match:
        print("ERROR: Could not find Self-Similarity section")
        return thesis_content
    
    # Extract questions
    questions = extract_questions(qa_content)
    print(f"Found {len(questions)} self-similarity questions")
    
    # Convert to narrative
    narrative_content = "\n\n".join([convert_qa_to_narrative(q) for q in questions[:10]])  # First 10 questions
    
    # Insert into thesis
    section_start = match.group(1)
    section_end = match.group(2)
    
    new_section = section_start + "\n\n### Additional Deep Analysis\n\n" + narrative_content + "\n\n" + section_end
    
    thesis_content = thesis_content.replace(match.group(0), new_section)
    
    return thesis_content

def integrate_blind_recovery(thesis_content, qa_content):
    """Integrate blind recovery Q&A into thesis."""
    print("Integrating Blind Recovery questions...")
    
    # Find the blind recovery section
    pattern = r'(## 12\. BLIND RECOVERY: THE CENTRAL PILLAR.*?)(## 13\.)'
    match = re.search(pattern, thesis_content, re.DOTALL)
    
    if not match:
        print("ERROR: Could not find Blind Recovery section")
        return thesis_content
    
    # Extract questions
    questions = extract_questions(qa_content)
    print(f"Found {len(questions)} blind recovery questions")
    
    # Convert to narrative
    narrative_content = "\n\n".join([convert_qa_to_narrative(q) for q in questions[:10]])  # First 10 questions
    
    # Insert into thesis
    section_start = match.group(1)
    section_end = match.group(2)
    
    new_section = section_start + "\n\n### Additional Deep Analysis\n\n" + narrative_content + "\n\n" + section_end
    
    thesis_content = thesis_content.replace(match.group(0), new_section)
    
    return thesis_content

def main():
    """Main integration function."""
    print("=" * 80)
    print("INTEGRATING REMAINING Q&A SECTIONS")
    print("=" * 80)
    
    # Read thesis
    print("\nReading THESIS.md...")
    thesis_content = read_file('THESIS.md')
    original_lines = len(thesis_content.split('\n'))
    print(f"Original: {original_lines:,} lines")
    
    # Read Q&A files
    print("\nReading Q&A files...")
    triangulation_qa = read_file('TRIANGULATION_QUESTIONS.md')
    self_similarity_qa = read_file('SELF_SIMILARITY_QUESTIONS.md')
    blind_recovery_qa = read_file('BLIND_RECOVERY_QUESTIONS.md')
    
    # Integrate each section
    print("\n" + "=" * 80)
    thesis_content = integrate_triangulation(thesis_content, triangulation_qa)
    
    print("\n" + "=" * 80)
    thesis_content = integrate_self_similarity(thesis_content, self_similarity_qa)
    
    print("\n" + "=" * 80)
    thesis_content = integrate_blind_recovery(thesis_content, blind_recovery_qa)
    
    # Write updated thesis
    print("\n" + "=" * 80)
    print("Writing updated THESIS.md...")
    write_file('THESIS_INTEGRATED_REMAINING.md', thesis_content)
    
    final_lines = len(thesis_content.split('\n'))
    added_lines = final_lines - original_lines
    
    print(f"\nFinal: {final_lines:,} lines")
    print(f"Added: {added_lines:,} lines (+{added_lines/original_lines*100:.1f}%)")
    print("\n" + "=" * 80)
    print("INTEGRATION COMPLETE!")
    print("=" * 80)

if __name__ == '__main__':
    main()