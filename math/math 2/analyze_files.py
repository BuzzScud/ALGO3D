#!/usr/bin/env python3
import os
import subprocess
import re

def get_files_in_directory(directory, extension):
    """Get all files with given extension in directory"""
    files = []
    if os.path.exists(directory):
        for root, dirs, filenames in os.walk(directory):
            for filename in filenames:
                if filename.endswith(extension):
                    files.append(os.path.join(root, filename))
    return sorted(files)

def check_in_makefile(filename):
    """Check if file is referenced in Makefile"""
    try:
        with open('Makefile', 'r') as f:
            makefile_content = f.read()
            basename = os.path.basename(filename)
            # Check for explicit mention or wildcard pattern
            if basename in makefile_content:
                return True
            # Check if directory is in wildcard
            dirname = os.path.dirname(filename)
            if f"$(wildcard {dirname}/*.c)" in makefile_content:
                return True
    except:
        pass
    return False

def find_function_definitions(filename):
    """Extract function definitions from C file"""
    functions = []
    try:
        with open(filename, 'r') as f:
            content = f.read()
            # Simple regex to find function definitions
            pattern = r'^[a-zA-Z_][a-zA-Z0-9_\s\*]+\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*\([^)]*\)\s*\{'
            for match in re.finditer(pattern, content, re.MULTILINE):
                functions.append(match.group(1))
    except:
        pass
    return functions

def check_function_usage(function_name, all_files):
    """Check if function is called in any file"""
    for filepath in all_files:
        try:
            with open(filepath, 'r') as f:
                content = f.read()
                # Look for function calls
                if re.search(rf'\b{function_name}\s*\(', content):
                    return True
        except:
            pass
    return False

# Get all C and H files
c_files = []
for directory in ['src/core', 'src/transcendental', 'src/geometry', 'src/ai', 'src/ai/infrastructure', 
                  'src/crawler', 'src/document_processing', 'src/utils', 'tools', 'app']:
    c_files.extend(get_files_in_directory(directory, '.c'))

h_files = []
for directory in ['include', 'algorithms/include']:
    h_files.extend(get_files_in_directory(directory, '.h'))

print(f"Total C files found: {len(c_files)}")
print(f"Total H files found: {len(h_files)}")
print()

# Analyze each C file
unused_files = []
for c_file in c_files:
    in_makefile = check_in_makefile(c_file)
    if not in_makefile:
        unused_files.append(c_file)

print(f"Files NOT in Makefile: {len(unused_files)}")
if unused_files:
    print("\nFiles not compiled:")
    for f in unused_files[:20]:  # Show first 20
        print(f"  - {f}")
    if len(unused_files) > 20:
        print(f"  ... and {len(unused_files) - 20} more")

