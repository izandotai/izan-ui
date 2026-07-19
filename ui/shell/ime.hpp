#pragma once

#include <imgui.h>

struct GLFWwindow;

namespace izan::ui {

// Keeps the Windows IME composition and candidate windows glued to the
// text caret. Callers that know the caret position pass override_pos;
// nullptr falls back to ImGui's PlatformImeData.
void update_ime_position(GLFWwindow* window, const ImVec2* override_pos);

// Whole-window IME switch. Passphrase fields must physically detach
// the IME (composition strings get cached by the system and surface in
// candidate lists — unacceptable for secrets); enabled=true reattaches
// the thread's default input context.
void set_ime_enabled(GLFWwindow* window, bool enabled);

}
