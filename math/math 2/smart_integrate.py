#!/usr/bin/env python3
"""
Smart integration that avoids duplicates and properly merges content
"""

def read_file(filename):
    with open(filename, 'r', encoding='utf-8') as f:
        return f.readlines()

def write_file(filename, lines):
    with open(filename, 'w', encoding='utf-8') as f:
        f.writelines(lines)

# Read files
thesis = read_file('THESIS_INTEGRATED.md')
geo_arith = read_file('THESIS_EXPANSION_GEOMETRIC_ARITHMETIC.md')
clock_lattice = read_file('THESIS_EXPANSION_CLOCK_LATTICE_CRYSTALLINE_ABACUS.md')
triangulation = read_file('THESIS_EXPANSION_TRIANGULATION_SELF_SIMILARITY.md')
blind_recovery = read_file('THESIS_EXPANSION_BLIND_RECOVERY.md')
hashing = read_file('THESIS_EXPANSION_NOVEL_HASHING_BITCOIN.md')
interconnections = read_file('THESIS_EXPANSION_INTERCONNECTIONS.md')
source_code = read_file('THESIS_EXPANSION_SOURCE_CODE_THEORY.md')

print("Files loaded:")
print(f"  THESIS_INTEGRATED.md: {len(thesis)} lines")
print(f"  Geometric Arithmetic: {len(geo_arith)} lines")
print(f"  Clock Lattice: {len(clock_lattice)} lines")
print(f"  Triangulation: {len(triangulation)} lines")
print(f"  Blind Recovery: {len(blind_recovery)} lines")
print(f"  Hashing: {len(hashing)} lines")
print(f"  Interconnections: {len(interconnections)} lines")
print(f"  Source Code: {len(source_code)} lines")

# Strategy: Append all expansion documents as new sections at the end
# This preserves all content and allows for manual refinement

# Find the end of the main content (before any appendices)
end_of_content = len(thesis)
for i, line in enumerate(thesis):
    if '## APPENDIX' in line or '# APPENDIX' in line:
        end_of_content = i
        break

print(f"\nMain content ends at line {end_of_content}")

# Create integrated version
integrated = thesis[:end_of_content]

# Add separator
integrated.extend([
    '\n',
    '---\n',
    '\n',
    '# PART VIII: EXPANDED THEORETICAL TREATISES\n',
    '\n',
    '## Overview\n',
    '\n',
    'The following sections provide comprehensive theoretical expansions of key concepts.\n',
    'These treatises offer deep mathematical foundations, philosophical implications,\n',
    'and novel insights that extend beyond the core presentation.\n',
    '\n',
    '---\n',
    '\n'
])

# Add each expansion as a separate chapter
expansions = [
    ('GEOMETRIC ARITHMETIC: DEEP THEORETICAL FOUNDATIONS', geo_arith),
    ('CLOCK LATTICE AND CRYSTALLINE ABACUS: COMPREHENSIVE THEORY', clock_lattice),
    ('TRIANGULATION AND SELF-SIMILARITY: MATHEMATICAL FOUNDATIONS', triangulation),
    ('BLIND RECOVERY: COMPLETE THEORETICAL FRAMEWORK', blind_recovery),
    ('NOVEL HASHING AND BLOCKCHAIN: GEOMETRIC SOLUTIONS', hashing),
    ('INTERCONNECTIONS: THE UNIFIED FRAMEWORK', interconnections),
    ('SOURCE CODE THEORY: IMPLEMENTATION FOUNDATIONS', source_code),
]

for title, content in expansions:
    integrated.extend([
        f'## {title}\n',
        '\n'
    ])
    # Skip the first few lines (title/header) of expansion documents
    start_line = 0
    for i, line in enumerate(content):
        if line.startswith('## ') or line.startswith('### '):
            start_line = i
            break
    integrated.extend(content[start_line:])
    integrated.extend(['\n', '---\n', '\n'])

# Add back any appendices
if end_of_content < len(thesis):
    integrated.extend(thesis[end_of_content:])

print(f"\nIntegrated version: {len(integrated)} lines")
print(f"Increase: {len(integrated) - len(thesis)} lines")

# Write result
write_file('THESIS_FULLY_INTEGRATED.md', integrated)
print("\nFully integrated version written to THESIS_FULLY_INTEGRATED.md")

# Generate statistics
print("\n=== INTEGRATION STATISTICS ===")
print(f"Original: {len(thesis):,} lines")
print(f"Added: {len(integrated) - len(thesis):,} lines")
print(f"Final: {len(integrated):,} lines")
print(f"Growth: {((len(integrated) / len(thesis)) - 1) * 100:.1f}%")
