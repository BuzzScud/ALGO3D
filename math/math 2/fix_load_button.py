#!/usr/bin/env python3

with open('app/ui/tabs/tab_models.c', 'r') as f:
    content = f.read()

# Fix the function call
content = content.replace(
    'model_manager_list_models(&models, &model_count);',
    'models = model_manager_list(&model_count);'
)

with open('app/ui/tabs/tab_models.c', 'w') as f:
    f.write(content)

print("✓ Fixed model_manager_list_models call")

