#!/usr/bin/env python3
"""
Thesis Content Extraction Script - Phase 2
Extracts content from the monolithic THESIS.md into organized chapter files
"""

import re
import os
from pathlib import Path
from typing import List, Tuple, Dict

class ThesisExtractor:
    def __init__(self, thesis_file: str):
        self.thesis_file = thesis_file
        self.content = self._read_thesis()
        self.lines = self.content.split('\n')
        self.sections = self._parse_sections()
        
    def _read_thesis(self) -> str:
        """Read the thesis file"""
        with open(self.thesis_file, 'r', encoding='utf-8') as f:
            return f.read()
    
    def _parse_sections(self) -> List[Dict]:
        """Parse all sections with their line numbers"""
        sections = []
        for i, line in enumerate(self.lines):
            # Match section headings (##, ###, etc.)
            match = re.match(r'^(#{1,6})\s+(.+)$', line)
            if match:
                level = len(match.group(1))
                title = match.group(2).strip()
                sections.append({
                    'line': i,
                    'level': level,
                    'title': title,
                    'content_start': i + 1
                })
        
        # Set content_end for each section
        for i in range(len(sections) - 1):
            sections[i]['content_end'] = sections[i + 1]['line']
        if sections:
            sections[-1]['content_end'] = len(self.lines)
        
        return sections
    
    def extract_section(self, start_line: int, end_line: int) -> str:
        """Extract content between line numbers"""
        return '\n'.join(self.lines[start_line:end_line])
    
    def find_section_by_title(self, title_pattern: str) -> List[Dict]:
        """Find sections matching a title pattern"""
        pattern = re.compile(title_pattern, re.IGNORECASE)
        return [s for s in self.sections if pattern.search(s['title'])]
    
    def extract_chapter(self, chapter_num: int, chapter_name: str) -> str:
        """Extract a complete chapter"""
        # Find the chapter heading - format is "## 1. TITLE"
        pattern = f"^{chapter_num}\\."
        matches = self.find_section_by_title(pattern)
        
        if not matches:
            print(f"Warning: Chapter {chapter_num} not found")
            return ""
        
        # Get the first match (in case of duplicates)
        chapter_section = matches[0]
        
        # Find the next chapter (chapter_num + 1)
        next_chapter_pattern = f"^{chapter_num + 1}\\."
        next_matches = self.find_section_by_title(next_chapter_pattern)
        
        if next_matches:
            end_line = next_matches[0]['line']
        else:
            # This is the last chapter, find a reasonable end point
            end_line = chapter_section['content_end']
        
        return self.extract_section(chapter_section['line'], end_line)
    
    def extract_abstract(self) -> str:
        """Extract the abstract"""
        matches = self.find_section_by_title(r'^## ABSTRACT$')
        if matches:
            section = matches[0]
            # Find the next ## level section
            next_section = None
            for s in self.sections:
                if s['line'] > section['line'] and s['level'] == 2:
                    next_section = s
                    break
            
            if next_section:
                return self.extract_section(section['line'], next_section['line'])
        return ""
    
    def extract_toc(self) -> str:
        """Extract table of contents"""
        matches = self.find_section_by_title(r'^## TABLE OF CONTENTS$')
        if matches:
            section = matches[0]
            # Find PART I marker
            part1_matches = self.find_section_by_title(r'^# PART I:')
            if part1_matches:
                return self.extract_section(section['line'], part1_matches[0]['line'])
        return ""
    
    def save_to_file(self, content: str, filepath: str):
        """Save content to a file"""
        os.makedirs(os.path.dirname(filepath), exist_ok=True)
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(content)
        print(f"✓ Created: {filepath}")

def main():
    print("=" * 60)
    print("THESIS CONTENT EXTRACTION - PHASE 2")
    print("=" * 60)
    print()
    
    # Initialize extractor
    extractor = ThesisExtractor('thesis/THESIS.md')
    
    print(f"Loaded thesis: {len(extractor.lines)} lines")
    print(f"Found {len(extractor.sections)} sections")
    print()
    
    # Extract front matter
    print("Extracting front matter...")
    abstract = extractor.extract_abstract()
    if abstract:
        extractor.save_to_file(abstract, 'thesis/front_matter/abstract.md')
    
    toc = extractor.extract_toc()
    if toc:
        extractor.save_to_file(toc, 'thesis/THESIS_MAIN.md')
    
    print()
    
    # Extract chapters
    print("Extracting chapters...")
    
    chapters = [
        (1, "Introduction", "part_01_theoretical_foundations/chapter_01_introduction"),
        (2, "Babylonian Mathematics", "part_01_theoretical_foundations/chapter_02_babylonian_mathematics"),
        (3, "Ancient Proverb", "part_01_theoretical_foundations/chapter_03_ancient_proverb"),
        (4, "Geometric Arithmetic", "part_01_theoretical_foundations/chapter_04_geometric_arithmetic"),
        (5, "Clock Lattice", "part_01_theoretical_foundations/chapter_05_clock_lattice"),
        (6, "Crystalline Abacus", "part_01_theoretical_foundations/chapter_06_crystalline_abacus"),
        (7, "Triangulation", "part_02_core_principles/chapter_07_triangulation"),
        (8, "Self-Similarity", "part_02_core_principles/chapter_08_self_similarity"),
        (9, "Prime Generation", "part_02_core_principles/chapter_09_prime_generation"),
        (10, "Clock Triangle", "part_02_core_principles/chapter_10_clock_triangle"),
        (11, "Babylonian Operations", "part_02_core_principles/chapter_11_babylonian_operations"),
        (12, "Blind Recovery", "part_03_advanced_concepts/chapter_12_blind_recovery"),
        (13, "NTT Algorithms", "part_03_advanced_concepts/chapter_13_ntt_algorithms"),
        (14, "Kissing Spheres", "part_03_advanced_concepts/chapter_14_kissing_spheres"),
        (15, "Platonic Solids", "part_03_advanced_concepts/chapter_15_platonic_solids"),
        (16, "Geometric Recovery", "part_03_advanced_concepts/chapter_16_geometric_recovery"),
        (17, "Novel Hashing", "part_04_applications/chapter_17_novel_hashing"),
        (18, "Blockchain", "part_04_applications/chapter_18_blockchain"),
        (19, "AI Architecture", "part_04_applications/chapter_19_ai_architecture"),
        (20, "Cryptography", "part_04_applications/chapter_20_cryptography"),
        (21, "Quantum Computing", "part_04_applications/chapter_21_quantum_computing"),
        (22, "Web of Concepts", "part_05_synthesis/chapter_22_web_of_concepts"),
        (23, "Unified Framework", "part_05_synthesis/chapter_23_unified_framework"),
        (24, "Philosophical Implications", "part_05_synthesis/chapter_24_philosophical_implications"),
        (25, "Future Directions", "part_05_synthesis/chapter_25_future_directions"),
        (26, "Mathematical Framework", "part_06_implementation/chapter_26_mathematical_framework"),
        (27, "Implementation Details", "part_06_implementation/chapter_27_implementation_details"),
        (28, "Performance Analysis", "part_06_implementation/chapter_28_performance_analysis"),
        (29, "Validation Results", "part_06_implementation/chapter_29_validation_results"),
        (30, "Summary", "part_07_conclusions/chapter_30_summary"),
        (31, "Impact", "part_07_conclusions/chapter_31_impact"),
        (32, "Future Work", "part_07_conclusions/chapter_32_future_work"),
    ]
    
    for chapter_num, chapter_name, chapter_path in chapters:
        print(f"Extracting Chapter {chapter_num}: {chapter_name}...")
        content = extractor.extract_chapter(chapter_num, chapter_name)
        if content:
            filepath = f"thesis/{chapter_path}/content.md"
            extractor.save_to_file(content, filepath)
        else:
            print(f"  Warning: No content found for Chapter {chapter_num}")
    
    print()
    print("=" * 60)
    print("PHASE 2 COMPLETE")
    print("=" * 60)
    print()
    print("Next: Review extracted chapters and run Phase 3")

if __name__ == "__main__":
    main()