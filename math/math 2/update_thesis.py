#!/usr/bin/env python3
"""
Properly update THESIS.md with all expansion content
Following MASTER_PLAN rules
"""

def read_file(filename):
    with open(filename, 'r', encoding='utf-8') as f:
        return f.read()

def write_file(filename, content):
    with open(filename, 'w', encoding='utf-8') as f:
        f.write(content)

print("=== UPDATING THESIS.MD ===\n")

# Read the fully integrated version (which has all content)
print("Reading THESIS_FULLY_INTEGRATED.md...")
integrated = read_file('THESIS_FULLY_INTEGRATED.md')
print(f"  Size: {len(integrated)} characters")

# Read the Q&A appendix
print("Reading QA_APPENDIX_PART1.md...")
qa = read_file('QA_APPENDIX_PART1.md')
print(f"  Size: {len(qa)} characters")

# Combine everything
print("\nCombining content...")
final_thesis = integrated + "\n\n---\n\n" + qa

print(f"Final size: {len(final_thesis)} characters")

# Write to THESIS.md
print("\nWriting to THESIS.md...")
write_file('THESIS.md', final_thesis)

# Also write to thesis directory
print("Writing to thesis/THESIS.md...")
write_file('thesis/THESIS.md', final_thesis)

print("\n✓ THESIS.md updated successfully!")

# Print statistics
lines = final_thesis.count('\n')
print(f"\nStatistics:")
print(f"  Total lines: {lines:,}")
print(f"  Total characters: {len(final_thesis):,}")
print(f"  Sections: {final_thesis.count('## ')}")
print(f"  Subsections: {final_thesis.count('### ')}")

