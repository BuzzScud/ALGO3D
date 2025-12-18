#!/usr/bin/env python3
"""
Deep file-by-file analysis of math library.
Examines each file for alignment with thesis principles.
"""

import os
import re

def analyze_file_content(filepath):
    """Analyze a single file's content."""
    with open(filepath, 'r') as f:
        content = f.read()
    
    analysis = {
        'file': filepath,
        'lines': len(content.split('\n')),
        'functions': len(re.findall(r'\n\w+\s+\w+\([^)]*\)\s*{', content)),
        'uses_clock_lattice': 'clock' in content.lower() or 'lattice' in content.lower(),
        'uses_abacus': 'abacus' in content.lower(),
        'uses_geometric': 'geometric' in content.lower() or 'angular' in content.lower(),
        'uses_threading': 'pthread' in content or 'thread' in content.lower(),
        'has_redundancy': False,
        'alignment_score': 0
    }
    
    # Calculate alignment score
    if analysis['uses_clock_lattice']:
        analysis['alignment_score'] += 2
    if analysis['uses_geometric']:
        analysis['alignment_score'] += 2
    if analysis['uses_abacus']:
        analysis['alignment_score'] += 1
    
    return analysis

def analyze_all_files():
    """Analyze all implementation files."""
    print("=" * 80)
    print("DEEP FILE-BY-FILE ANALYSIS")
    print("=" * 80)
    print()
    
    implementations = []
    for root, dirs, files in os.walk('math/src'):
        for file in files:
            if file.endswith('.c'):
                filepath = os.path.join(root, file)
                analysis = analyze_file_content(filepath)
                implementations.append(analysis)
    
    # Sort by alignment score
    implementations.sort(key=lambda x: x['alignment_score'], reverse=True)
    
    print("FILES RANKED BY ALIGNMENT WITH THESIS PRINCIPLES:")
    print()
    print(f"{'File':<50} {'Lines':<8} {'Funcs':<7} {'Clock':<7} {'Geom':<7} {'Abacus':<8} {'Score':<6}")
    print("-" * 100)
    
    for impl in implementations:
        filename = impl['file'].replace('math/src/', '')
        clock = '✓' if impl['uses_clock_lattice'] else '✗'
        geom = '✓' if impl['uses_geometric'] else '✗'
        abacus = '✓' if impl['uses_abacus'] else '✗'
        
        print(f"{filename:<50} {impl['lines']:<8} {impl['functions']:<7} {clock:<7} {geom:<7} {abacus:<8} {impl['alignment_score']:<6}")
    
    return implementations

def identify_specific_issues(implementations):
    """Identify specific issues in each file."""
    print()
    print("=" * 80)
    print("SPECIFIC ISSUES BY FILE")
    print("=" * 80)
    print()
    
    issues = []
    
    for impl in implementations:
        file_issues = []
        
        # Check if file should use clock lattice but doesn't
        if 'arithmetic' in impl['file'] and not impl['uses_clock_lattice']:
            file_issues.append("Arithmetic should use clock lattice")
        
        # Check if file should be geometric but isn't
        if 'arithmetic' in impl['file'] and not impl['uses_geometric']:
            file_issues.append("Arithmetic should be geometric")
        
        # Check for low alignment score
        if impl['alignment_score'] < 2 and 'test' not in impl['file']:
            file_issues.append(f"Low alignment score ({impl['alignment_score']})")
        
        if file_issues:
            issues.append({
                'file': impl['file'],
                'issues': file_issues
            })
    
    if issues:
        print(f"FOUND {len(issues)} FILES WITH ISSUES:")
        print()
        for i, issue in enumerate(issues, 1):
            print(f"{i}. {issue['file'].replace('math/src/', '')}")
            for iss in issue['issues']:
                print(f"   - {iss}")
            print()
    
    return issues

def analyze_redundant_implementations():
    """Analyze redundant implementations in detail."""
    print("=" * 80)
    print("DETAILED REDUNDANCY ANALYSIS")
    print("=" * 80)
    print()
    
    redundancies = []
    
    # Arithmetic redundancy
    print("1. ARITHMETIC IMPLEMENTATIONS:")
    print()
    arith_files = [
        'math/src/core/arithmetic.c',
        'math/src/compact/compact_arithmetic.c'
    ]
    for f in arith_files:
        if os.path.exists(f):
            with open(f, 'r') as file:
                content = file.read()
                funcs = re.findall(r'\n(\w+\s+\w+)\([^)]*\)\s*{', content)
                print(f"   {f}:")
                print(f"     Functions: {len(funcs)}")
                print(f"     Lines: {len(content.split(chr(10)))}")
                print(f"     Uses clock: {'clock' in content.lower()}")
                print(f"     Uses geometric: {'geometric' in content.lower()}")
                print()
    
    redundancies.append({
        'type': 'Arithmetic',
        'files': arith_files,
        'recommendation': 'Merge into single geometric arithmetic using clock lattice'
    })
    
    # Platonic generator redundancy
    print("2. PLATONIC GENERATOR IMPLEMENTATIONS:")
    print()
    platonic_gens = [
        'math/src/platonic/simplex_generator.c',
        'math/src/platonic/hypercube_generator.c',
        'math/src/platonic/cross_polytope_generator.c',
        'math/src/platonic/dodecahedron_generator.c',
        'math/src/platonic/icosahedron_generator.c'
    ]
    for f in platonic_gens:
        if os.path.exists(f):
            with open(f, 'r') as file:
                content = file.read()
                funcs = re.findall(r'\n(\w+\s+\w+)\([^)]*\)\s*{', content)
                print(f"   {f.replace('math/src/platonic/', '')}:")
                print(f"     Functions: {len(funcs)}")
                print(f"     Lines: {len(content.split(chr(10)))}")
                print()
    
    redundancies.append({
        'type': 'Platonic Generators',
        'files': platonic_gens,
        'recommendation': 'Create single unified generator with Schläfli symbol parameter'
    })
    
    # Abacus implementations
    print("3. ABACUS IMPLEMENTATIONS:")
    print()
    abacus_files = [
        'math/src/bigint/abacus.c',
        'math/src/bigint/abacus_gcd.c',
        'math/src/bigint/abacus_modular.c'
    ]
    for f in abacus_files:
        if os.path.exists(f):
            with open(f, 'r') as file:
                content = file.read()
                funcs = re.findall(r'\n(\w+\s+\w+)\([^)]*\)\s*{', content)
                print(f"   {f.replace('math/src/bigint/', '')}:")
                print(f"     Functions: {len(funcs)}")
                print(f"     Lines: {len(content.split(chr(10)))}")
                print()
    
    redundancies.append({
        'type': 'Abacus Operations',
        'files': abacus_files,
        'recommendation': 'These are fine - different operations on same structure'
    })
    
    return redundancies

def main():
    """Main analysis."""
    print()
    print("╔" + "=" * 78 + "╗")
    print("║" + " " * 20 + "DEEP FILE-BY-FILE ANALYSIS" + " " * 32 + "║")
    print("╚" + "=" * 78 + "╝")
    print()
    
    # Analyze all files
    implementations = analyze_all_files()
    
    # Identify specific issues
    issues = identify_specific_issues(implementations)
    
    # Analyze redundancies
    redundancies = analyze_redundant_implementations()
    
    # Final recommendations
    print()
    print("=" * 80)
    print("FINAL RECOMMENDATIONS")
    print("=" * 80)
    print()
    print("CRITICAL REFACTORING NEEDED:")
    print()
    print("1. UNIFY ARITHMETIC:")
    print("   - Merge core/arithmetic.c and compact/compact_arithmetic.c")
    print("   - Implement ALL arithmetic geometrically using clock lattice")
    print("   - Make abacus an optional high-precision backend")
    print()
    print("2. CONSOLIDATE PLATONIC GENERATORS:")
    print("   - Create single generator_core.c with Schläfli parameter")
    print("   - Remove individual generator files")
    print("   - Use unified algorithm for all regular polytopes")
    print()
    print("3. INTEGRATE THREADING:")
    print("   - Identify all threading implementations")
    print("   - Create single threading model")
    print("   - Apply consistently across library")
    print()
    print("4. MAKE CLOCK LATTICE CENTRAL:")
    print("   - Every operation should use clock lattice")
    print("   - Geometric operations should be primary")
    print("   - Traditional arithmetic should be secondary/compatibility")
    print()
    print("5. SIMPLIFY STRUCTURE:")
    print("   - Reduce number of files")
    print("   - Increase cohesion")
    print("   - Follow single design philosophy")
    print()

if __name__ == '__main__':
    main()
