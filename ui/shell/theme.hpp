#pragma once

#include "ui/shell/chrome_state.hpp"

#include <imgui.h>

#include <array>

namespace izan::ui {

// Fifteen UI themes plus the semantic colors (accent/ok/info) and the
// chrome color derivations the frame widgets draw with. Pure style
// functions only; whatever binds them to application state stays in
// the application.

constexpr std::array<const char*, 15> kThemeNames = {
    "Izan",
    "Dracula",
    "Claude",
    "Nord",
    "Light",
    "One Dark",
    "Monokai",
    "Gruvbox Dark",
    "Gruvbox Light",
    "Solarized Dark",
    "Solarized Light",
    "Tokyo Night",
    "Catppuccin Mocha",
    "GitHub Dark",
    "GitHub Light",
};

void apply_style();
void set_theme_rounding(ImGuiStyle& style, float radius);
void set_theme_spacing(ImGuiStyle& style, float window_padding,
    float frame_padding, float item_spacing);
void apply_nord_style();
void apply_claude_style();
void apply_light_style();
void apply_custom_style(const ImVec4& text, const ImVec4& text_disabled,
    const ImVec4& window, const ImVec4& child, const ImVec4& popup,
    const ImVec4& frame, const ImVec4& frame_hovered,
    const ImVec4& frame_active, const ImVec4& accent, const ImVec4& title,
    bool light);
void apply_indexed_theme_style(int theme_index);
void make_popup_background_opaque();
void apply_theme_style_only(int theme_index);

ImVec4 theme_accent_color(int theme_index);
ImVec4 theme_ok_color(int theme_index);
ImVec4 theme_info_color(int theme_index);

ImU32 color_u32_from_vec4(const ImVec4& color);
bool is_light_theme_index(int theme_index);
ImU32 theme_frame_background_color(const ChromeState& app);
ImU32 theme_chrome_background_color(const ChromeState& app);
ImU32 theme_chrome_line_color(const ChromeState& app, int alpha);
ImU32 theme_transparent_chrome_window_color(const ChromeState& app);
ImVec4 theme_panel_window_color(const ChromeState& app);
ImVec4 theme_panel_child_color(const ChromeState& app);
void draw_themed_panel_content_background(const ChromeState& app);
ImU32 theme_snap_preview_border_color(const ChromeState& app, bool hovered);
ImU32 theme_snap_preview_fill_color(const ChromeState& app, bool hovered);
ImU32 theme_snap_preview_background_color(const ChromeState& app);
ImU32 theme_popup_shadow_color(const ChromeState& app, float alpha);
ImVec4 theme_clear_color(const ChromeState& app);

}
