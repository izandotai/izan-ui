#pragma once

#include "ui/shell/chrome_state.hpp"
#include "ui/shell/win_chrome.hpp"

#include <imgui.h>

#include <functional>

struct GLFWwindow;

namespace izan::ui {

// The self-drawn window frame: main frame fill, title bar (window
// buttons, opacity slider, snap-layout popup), menu bar shell, status
// bar, layered soft popup shadows and a lightweight tooltip. Business
// text (title, subtitle, status, menu contents) comes from the caller.

void draw_menu_popup_shadows(const ChromeState& app);
void draw_main_window_frame(const ChromeState& app);
bool draw_window_control_button(
    const char* id, const ImVec2& size, WindowControlIcon icon);
void draw_simple_tooltip(const ChromeState& app, const char* id,
    const char* text, const ImVec2& anchor);
void queue_simple_tooltip(
    ChromeState& app, const char* text, const ImVec2& anchor);
void draw_snap_layout_popup(GLFWwindow* window, ChromeState& app);
void draw_custom_title_bar(GLFWwindow* window, ChromeState& app,
    const char* title_text, const char* subtitle_text);
void draw_custom_menu_bar(
    ChromeState& app, const std::function<void()>& draw_items);
void draw_status_bar(const ChromeState& app, const char* status_text);

}
