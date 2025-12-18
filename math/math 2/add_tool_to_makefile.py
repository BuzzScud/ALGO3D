#!/usr/bin/env python3

# Read Makefile
with open('Makefile', 'r') as f:
    lines = f.readlines()

# Find the tools: line and add fix_html_entities
modified = False
for i, line in enumerate(lines):
    if 'tools/visualize_angular_positions' in line and not modified:
        # Add fix_html_entities to this line
        lines[i] = line.rstrip() + ' tools/fix_html_entities\n'
        modified = True
        break

if modified:
    # Now add the build rule after diagnose_inference
    for i, line in enumerate(lines):
        if 'Inference diagnostic tool built: tools/diagnose_inference' in line:
            # Insert after this line
            insert_pos = i + 1
            new_rule = '''
tools/fix_html_entities:
\t@echo "Building HTML entity fixer..."
\t@mkdir -p tools
\t$(CC) $(CFLAGS) -o tools/fix_html_entities tools/fix_html_entities.c
\t@echo "✓ HTML entity fixer built: tools/fix_html_entities"

'''
            lines.insert(insert_pos, new_rule)
            break
    
    # Write back
    with open('Makefile', 'w') as f:
        f.writelines(lines)
    print("✓ Added fix_html_entities to Makefile")
else:
    print("✗ Could not find insertion point")

