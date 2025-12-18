#!/usr/bin/env python3
"""
Wrap remaining [DEBUG] printf statements in #ifdef CLLM_DEBUG blocks
"""

import re

def wrap_debug_statements(filename):
    with open(filename, 'r') as f:
        lines = f.readlines()
    
    result = []
    i = 0
    while i < len(lines):
        line = lines[i]
        
        # Check if this is a debug printf that's not already wrapped
        if '[DEBUG]' in line and 'printf' in line:
            # Check if previous line is already #ifdef
            if i > 0 and '#ifdef CLLM_DEBUG' in lines[i-1]:
                result.append(line)
                i += 1
                continue
            
            # This is an unwrapped debug statement
            # Find the complete statement (might span multiple lines)
            statement_lines = [line]
            j = i + 1
            
            # Keep adding lines until we find the semicolon
            while j < len(lines) and ';' not in lines[j-1]:
                statement_lines.append(lines[j])
                j += 1
            
            # Add the line with semicolon if we haven't already
            if ';' not in statement_lines[-1] and j < len(lines):
                statement_lines.append(lines[j])
                j += 1
            
            # Check if next line is fflush
            if j < len(lines) and 'fflush(stdout)' in lines[j]:
                statement_lines.append(lines[j])
                j += 1
            
            # Wrap the statement
            result.append('#ifdef CLLM_DEBUG\n')
            result.extend(statement_lines)
            result.append('#endif\n')
            
            i = j
        else:
            result.append(line)
            i += 1
    
    with open(filename, 'w') as f:
        f.writelines(result)
    
    print(f"Wrapped debug statements in {filename}")

if __name__ == '__main__':
    wrap_debug_statements('src/ai/cllm_training_threaded.c')