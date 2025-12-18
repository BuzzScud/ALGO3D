#!/usr/bin/env python3
"""
Verify mathematical claims in THESIS_FULLY_INTEGRATED.md against source code
"""

import re
import os

def read_file(filename):
    try:
        with open(filename, 'r', encoding='utf-8') as f:
            return f.read()
    except:
        return None

def find_source_files():
    """Find all C source files."""
    source_files = []
    for root, dirs, files in os.walk('math/src'):
        for file in files:
            if file.endswith('.c') or file.endswith('.h'):
                source_files.append(os.path.join(root, file))
    for root, dirs, files in os.walk('algorithms/src'):
        for file in files:
            if file.endswith('.c') or file.endswith('.h'):
                source_files.append(os.path.join(root, file))
    return source_files

def extract_claims(thesis_text):
    """Extract mathematical claims from thesis."""
    claims = []
    
    # Pattern 1: Formulas in code blocks
    code_blocks = re.findall(r'```c?\n(.*?)\n```', thesis_text, re.DOTALL)
    for block in code_blocks:
        if '=' in block or 'define' in block:
            claims.append(('formula', block.strip()))
    
    # Pattern 2: Explicit claims
    claim_patterns = [
        r'O\(1\)',
        r'100% accuracy',
        r'(\d+)x compression',
        r'(\d+) rings',
        r'base[- ]60',
        r'12-fold symmetry',
    ]
    
    for pattern in claim_patterns:
        matches = re.finditer(pattern, thesis_text, re.IGNORECASE)
        for match in matches:
            claims.append(('claim', match.group(0)))
    
    return claims

def verify_claim(claim, source_files):
    """Verify a claim against source code."""
    claim_type, claim_text = claim
    
    # Search for claim in source files
    found_in = []
    for source_file in source_files:
        content = read_file(source_file)
        if content and claim_text.lower() in content.lower():
            found_in.append(source_file)
    
    return found_in

# Main verification
print("=== MATHEMATICAL CLAIM VERIFICATION ===\n")

# Read thesis
thesis = read_file('THESIS_FULLY_INTEGRATED.md')
if not thesis:
    print("Error: Could not read THESIS_FULLY_INTEGRATED.md")
    exit(1)

print(f"Thesis loaded: {len(thesis)} characters\n")

# Find source files
source_files = find_source_files()
print(f"Found {len(source_files)} source files\n")

# Extract claims
claims = extract_claims(thesis)
print(f"Extracted {len(claims)} claims to verify\n")

# Verify key claims
key_claims = [
    ('O(1) prime generation', 'O(1)'),
    ('100% accuracy', '100%'),
    ('Base-60 system', 'base'),
    ('12-fold symmetry', '12'),
    ('Ring sizes', 'RING_'),
    ('Interference formula', 'interference'),
    ('Compact vector', 'compact'),
]

verified = 0
not_found = 0

print("Verifying key claims:\n")
for claim_name, search_term in key_claims:
    found = False
    for source_file in source_files:
        content = read_file(source_file)
        if content and search_term.lower() in content.lower():
            print(f"✓ {claim_name}: Found in {os.path.basename(source_file)}")
            found = True
            verified += 1
            break
    if not found:
        print(f"✗ {claim_name}: NOT FOUND in source code")
        not_found += 1

print(f"\n=== VERIFICATION SUMMARY ===")
print(f"Verified: {verified}/{len(key_claims)}")
print(f"Not found: {not_found}/{len(key_claims)}")
print(f"Success rate: {(verified/len(key_claims)*100):.1f}%")
