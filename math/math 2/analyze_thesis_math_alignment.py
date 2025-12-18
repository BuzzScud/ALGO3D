#!/usr/bin/env python3
"""
Deep analysis of thesis vs math library alignment.
Identifies inconsistencies, redundancies, and integration opportunities.
"""

import os
import re
from collections import defaultdict

def extract_thesis_principles():
    """Extract core mathematical principles from thesis."""
    print("=" * 80)
    print("EXTRACTING CORE PRINCIPLES FROM THESIS")
    print("=" * 80)
    print()
    
    with open('thesis/THESIS.md', 'r') as f:
        content = f.read()
    
    principles = {
        'clock_lattice': [],
        'babylonian_arithmetic': [],
        'geometric_operations': [],
        'abacus': [],
        'prime_generation': [],
        'platonic_solids': [],
        'ntt': [],
        'compact_vectors': [],
        'threading': [],
        'unified_design': []
    }
    
    # Extract mentions of each principle
    sections = content.split('##')
    
    for section in sections:
        lower = section.lower()
        
        if 'clock lattice' in lower or '12-fold' in lower:
            principles['clock_lattice'].append(section[:200])
        
        if 'babylonian' in lower and ('arithmetic' in lower or 'operation' in lower):
            principles['babylonian_arithmetic'].append(section[:200])
        
        if 'geometric' in lower and ('operation' in lower or 'arithmetic' in lower):
            principles['geometric_operations'].append(section[:200])
        
        if 'abacus' in lower or 'crystalline abacus' in lower:
            principles['abacus'].append(section[:200])
        
        if 'prime' in lower and ('generation' in lower or 'o(1)' in lower):
            principles['prime_generation'].append(section[:200])
        
        if 'platonic' in lower or 'polytope' in lower:
            principles['platonic_solids'].append(section[:200])
        
        if 'ntt' in lower or 'number theoretic transform' in lower:
            principles['ntt'].append(section[:200])
        
        if 'compact vector' in lower or 'memory hopping' in lower:
            principles['compact_vectors'].append(section[:200])
        
        if 'thread' in lower or 'parallel' in lower or 'concurrent' in lower:
            principles['threading'].append(section[:200])
        
        if 'unified' in lower or 'single' in lower or 'integrated' in lower:
            principles['unified_design'].append(section[:200])
    
    print("THESIS PRINCIPLES FOUND:")
    print()
    for key, items in principles.items():
        print(f"{key.upper()}: {len(items)} mentions")
    
    return principles

def analyze_math_library_structure():
    """Analyze math library file structure."""
    print()
    print("=" * 80)
    print("ANALYZING MATH LIBRARY STRUCTURE")
    print("=" * 80)
    print()
    
    structure = {
        'headers': [],
        'implementations': [],
        'tests': [],
        'categories': defaultdict(list)
    }
    
    for root, dirs, files in os.walk('math'):
        for file in files:
            if file.endswith('.h'):
                structure['headers'].append(os.path.join(root, file))
            elif file.endswith('.c'):
                path = os.path.join(root, file)
                if 'test' in path:
                    structure['tests'].append(path)
                else:
                    structure['implementations'].append(path)
                
                # Categorize by directory
                if 'bigint' in path or 'abacus' in path:
                    structure['categories']['abacus'].append(path)
                elif 'compact' in path:
                    structure['categories']['compact'].append(path)
                elif 'core' in path:
                    structure['categories']['core'].append(path)
                elif 'geometry' in path:
                    structure['categories']['geometry'].append(path)
                elif 'ntt' in path:
                    structure['categories']['ntt'].append(path)
                elif 'platonic' in path:
                    structure['categories']['platonic'].append(path)
                elif 'prime' in path:
                    structure['categories']['prime'].append(path)
                elif 'visualization' in path:
                    structure['categories']['visualization'].append(path)
    
    print("LIBRARY STRUCTURE:")
    print(f"  Headers: {len(structure['headers'])}")
    print(f"  Implementations: {len(structure['implementations'])}")
    print(f"  Tests: {len(structure['tests'])}")
    print()
    print("CATEGORIES:")
    for cat, files in sorted(structure['categories'].items()):
        print(f"  {cat}: {len(files)} files")
    
    return structure

def identify_redundancies(structure):
    """Identify potential redundancies in the library."""
    print()
    print("=" * 80)
    print("IDENTIFYING REDUNDANCIES")
    print("=" * 80)
    print()
    
    redundancies = []
    
    # Check for multiple arithmetic implementations
    arithmetic_files = [f for f in structure['implementations'] if 'arithmetic' in f.lower()]
    if len(arithmetic_files) > 1:
        redundancies.append({
            'type': 'Multiple Arithmetic Implementations',
            'files': arithmetic_files,
            'concern': 'Should have single unified arithmetic system'
        })
    
    # Check for multiple abacus implementations
    abacus_files = structure['categories']['abacus']
    if len(abacus_files) > 3:
        redundancies.append({
            'type': 'Multiple Abacus Implementations',
            'files': abacus_files,
            'concern': 'Should integrate into single abacus system'
        })
    
    # Check for multiple platonic generators
    platonic_files = structure['categories']['platonic']
    generator_files = [f for f in platonic_files if 'generator' in f]
    if len(generator_files) > 5:
        redundancies.append({
            'type': 'Multiple Platonic Generators',
            'files': generator_files,
            'concern': 'Should have single unified generator with parameters'
        })
    
    # Check for threading implementations
    threading_files = []
    for impl in structure['implementations']:
        with open(impl, 'r') as f:
            content = f.read()
            if 'pthread' in content or 'thread' in content.lower():
                threading_files.append(impl)
    
    if len(threading_files) > 1:
        redundancies.append({
            'type': 'Multiple Threading Implementations',
            'files': threading_files,
            'concern': 'Should have single unified threading model'
        })
    
    print(f"FOUND {len(redundancies)} POTENTIAL REDUNDANCIES:")
    print()
    for i, red in enumerate(redundancies, 1):
        print(f"{i}. {red['type']}")
        print(f"   Files: {len(red['files'])}")
        for f in red['files'][:5]:
            print(f"     - {f}")
        if len(red['files']) > 5:
            print(f"     ... and {len(red['files']) - 5} more")
        print(f"   Concern: {red['concern']}")
        print()
    
    return redundancies

def check_thesis_alignment(principles, structure):
    """Check if library aligns with thesis principles."""
    print("=" * 80)
    print("CHECKING THESIS-LIBRARY ALIGNMENT")
    print("=" * 80)
    print()
    
    alignment_issues = []
    
    # Check if clock lattice is central
    clock_files = [f for f in structure['implementations'] if 'clock' in f.lower()]
    if len(clock_files) < 3:
        alignment_issues.append({
            'principle': 'Clock Lattice as Foundation',
            'issue': f'Only {len(clock_files)} clock-related files',
            'recommendation': 'Clock lattice should be central to all operations'
        })
    
    # Check if geometric operations are primary
    geometric_files = [f for f in structure['implementations'] if 'geometric' in f.lower() or 'angular' in f.lower()]
    arithmetic_files = [f for f in structure['implementations'] if 'arithmetic' in f.lower()]
    
    if len(arithmetic_files) > len(geometric_files):
        alignment_issues.append({
            'principle': 'Geometric Operations Primary',
            'issue': f'{len(arithmetic_files)} arithmetic files vs {len(geometric_files)} geometric files',
            'recommendation': 'Arithmetic should be implemented geometrically'
        })
    
    # Check for unified design
    if len(structure['categories']['abacus']) > 0 and len(structure['categories']['compact']) > 0:
        alignment_issues.append({
            'principle': 'Unified Design',
            'issue': 'Separate abacus and compact vector implementations',
            'recommendation': 'Should integrate into single unified system'
        })
    
    print(f"FOUND {len(alignment_issues)} ALIGNMENT ISSUES:")
    print()
    for i, issue in enumerate(alignment_issues, 1):
        print(f"{i}. Principle: {issue['principle']}")
        print(f"   Issue: {issue['issue']}")
        print(f"   Recommendation: {issue['recommendation']}")
        print()
    
    return alignment_issues

def main():
    """Main analysis function."""
    print()
    print("╔" + "=" * 78 + "╗")
    print("║" + " " * 15 + "THESIS vs MATH LIBRARY DEEP ANALYSIS" + " " * 27 + "║")
    print("╚" + "=" * 78 + "╝")
    print()
    
    # Phase 1: Extract thesis principles
    principles = extract_thesis_principles()
    
    # Phase 2: Analyze library structure
    structure = analyze_math_library_structure()
    
    # Phase 3: Identify redundancies
    redundancies = identify_redundancies(structure)
    
    # Phase 4: Check alignment
    alignment_issues = check_thesis_alignment(principles, structure)
    
    # Summary
    print()
    print("=" * 80)
    print("SUMMARY")
    print("=" * 80)
    print()
    print(f"Total Files Analyzed: {len(structure['headers']) + len(structure['implementations']) + len(structure['tests'])}")
    print(f"Redundancies Found: {len(redundancies)}")
    print(f"Alignment Issues: {len(alignment_issues)}")
    print()
    print("RECOMMENDATION: Perform deep refactoring to:")
    print("  1. Unify arithmetic implementations (geometric + abacus)")
    print("  2. Consolidate platonic generators into single parameterized system")
    print("  3. Integrate threading into single model")
    print("  4. Make clock lattice central to all operations")
    print("  5. Eliminate redundant implementations")
    print()

if __name__ == '__main__':
    main()
