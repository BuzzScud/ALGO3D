// This file contains the exact changes needed for Training Tab

// 1. Add this callback function after line 285 (after on_model_selected):

// File checkbox callback
static void on_file_checkbox_changed(int index, bool checked, void* user_data) {
    (void)user_data;
    if (index >= 0 && index < g_training_ui.file_count) {
        g_training_ui.files[index].selected = checked;
    }
}

// 2. Add these two lines after line 456 (after creating file list):
    crystalline_list_enable_checkboxes(g_training_ui.file_list, true);
    crystalline_list_set_check_callback(g_training_ui.file_list, on_file_checkbox_changed, state);

// 3. Add this in update_file_list after line 137 (after crystalline_list_set_items):
        // Sync checkbox states
        for (int i = 0; i < g_training_ui.file_count; i++) {
            crystalline_list_set_item_checked(g_training_ui.file_list, i, g_training_ui.files[i].selected);
        }

// 4. Replace lines 608-650 (manual rendering) with:
    // Render file list using Crystalline UI
    if (g_training_ui.file_list) {
        crystalline_list_render(g_training_ui.file_list, renderer);
    }

// 5. Replace lines 716-745 (manual click detection) with:
    // Handle file list clicks
    if (g_training_ui.file_list) {
        crystalline_list_handle_mouse(g_training_ui.file_list, &event);
    }
