#pragma once

#include <imgui.h>

namespace izan::ui {

// Capsule chip: tinted background, full-strength text. For badges,
// states, recognition results.
void kit_pill(const char* text, ImVec4 tint);

}
