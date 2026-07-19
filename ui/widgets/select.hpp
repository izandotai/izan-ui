#pragma once

namespace izan::ui {

// A dropdown in the kit's voice. Usage mirrors imgui combos:
//   if (kit_select_begin("##chain", current_label)) {
//       if (kit_select_item(label, is_current)) { ... }
//       kit_select_end();
//   }
bool kit_select_begin(const char* id, const char* preview, float width = 0.0f);
bool kit_select_item(const char* label, bool selected);
void kit_select_end();

}
