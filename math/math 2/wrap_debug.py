#!/usr/bin/env python3
"""
Wrap all [DEBUG] printf statements in #ifdef CLLM_DEBUG blocks
"""

import re
import sys

def wrap_debug_statements(content):
    """Wrap debug printf statements in ifdef blocks"""
    
    # Pattern to match printf statements with [DEBUG]
    # This matches the printf line and any continuation lines
    pattern = r'(    printf\("[DEBUG][^"]*"[^;]*;)'
    
    def replace_func(match):
        statement = match.group(1)
        # Check if already wrapped
        if '#ifdef CLLM_DEBUG' in statement:
            return statement
        return f'#ifdef CLLM_DEBUG\n{statement}\n#endif'
    
    result = re.sub(pattern, replace_func, content, flags=re.MULTILINE)
    return result

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("Usage: wrap_debug.py <file>")
        sys.exit(1)
    
    filename = sys.argv[1]
    
    with open(filename, 'r') as f:
        content = f.read()
    
    wrapped = wrap_debug_statements(content)
    
    with open(filename, 'w') as f:
        f.write(wrapped)
    
    print(f"Wrapped debug statements in {filename}")
</file>