#!/usr/bin/env python3

with open('app/ui/tabs/tab_models.c', 'r') as f:
    content = f.read()

# Fix format truncation warnings by limiting model name length
replacements = [
    ('"Failed to prepare model \'%s\'"', '"Failed to prepare model \'%.200s\'"'),
    ('"Failed to load model \'%s\'"', '"Failed to load model \'%.200s\'"'),
    ('"Model \'%s\' loaded successfully"', '"Model \'%.200s\' loaded successfully"'),
]

for old, new in replacements:
    content = content.replace(old, new)

with open('app/ui/tabs/tab_models.c', 'w') as f:
    f.write(content)

print("✓ Fixed format truncation warnings")

