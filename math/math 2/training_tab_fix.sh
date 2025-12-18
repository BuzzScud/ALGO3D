#!/bin/bash
# Precise surgical fixes for Training Tab

echo "Step 1: Adding callback function..."
sed -i '285a\
\
// File checkbox callback\
static void on_file_checkbox_changed(int index, bool checked, void* user_data) {\
    (void)user_data;\
    if (index >= 0 && index < g_training_ui.file_count) {\
        g_training_ui.files[index].selected = checked;\
    }\
}' app/ui/tabs/tab_training.c

echo "Step 2: Enabling checkboxes..."
sed -i '456a\
    crystalline_list_enable_checkboxes(g_training_ui.file_list, true);\
    crystalline_list_set_check_callback(g_training_ui.file_list, on_file_checkbox_changed, state);' app/ui/tabs/tab_training.c

echo "Step 3: Adding checkbox syncing..."
sed -i '137a\
        \
        // Sync checkbox states\
        for (int i = 0; i < g_training_ui.file_count; i++) {\
            crystalline_list_set_item_checked(g_training_ui.file_list, i, g_training_ui.files[i].selected);\
        }' app/ui/tabs/tab_training.c

echo "Step 4: Replacing manual rendering..."
sed -i '608,650d' app/ui/tabs/tab_training.c
sed -i '607a\
    // Render file list using Crystalline UI\
    if (g_training_ui.file_list) {\
        crystalline_list_render(g_training_ui.file_list, renderer);\
    }' app/ui/tabs/tab_training.c

echo "Step 5: Replacing manual click detection..."
sed -i '716,745d' app/ui/tabs/tab_training.c
sed -i '715a\
    // Handle file list clicks\
    if (g_training_ui.file_list) {\
        crystalline_list_handle_mouse(g_training_ui.file_list, &event);\
    }' app/ui/tabs/tab_training.c

echo "✅ Done!"
