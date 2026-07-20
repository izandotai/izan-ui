#pragma once

#include <imgui.h>

#include <string_view>

#include "ui/shell/fonts.hpp"

// The Mint skin's shared brush box: logo, icons, window-control
// glyphs, shadow and text helpers, plus the logical font sizes the
// whole skin speaks. Everything scales by mint_scale() — monitor DPI
// times the accessibility text scale, recovered from the em the
// shell already baked — so one machine's pixels match another's.

namespace izan::os::mint {

// Logical pixel sizes at 96 DPI; multiply by mint_scale() at draw.
inline constexpr float kFontBody = 19.0f;
inline constexpr float kFontBodyCompact = 18.0f;
inline constexpr float kFontSecondary = 17.0f;
inline constexpr float kFontWindowTitle = 20.0f;
inline constexpr float kFontSectionTitle = 24.0f;
inline constexpr float kFontSymbol = 16.0f;
inline constexpr float kFontSymbolSmall = 14.0f;

inline constexpr float kPanelHeight = 46.0f;
inline constexpr float kFrameHeight = 42.0f;

// The em the shell bakes is (design size / design scale) x ui_scale;
// dividing the design base back out leaves ui_scale itself.
float mint_scale();

void mint_logo(ImDrawList* draw, ImVec2 center, float radius);
void folder_icon(ImDrawList* draw, ImVec2 center, float scale,
    ImU32 color = IM_COL32(116, 184, 83, 255));
void document_icon(ImDrawList* draw, ImVec2 center, float scale, ImU32 accent);
void wifi_icon(ImDrawList* draw, ImVec2 center, float scale, ImU32 color);
// control: 0 minimize, 1 maximize (restore variant when restore), 2 close.
void control_icon(ImDrawList* draw, ImVec2 center, int control, bool restore,
    ImU32 color, float scale);
void window_shadow(
    ImDrawList* draw, ImVec2 min, ImVec2 max, float rounding, float scale);

void text_at(ImDrawList* draw, ImVec2 pos, ImU32 color, std::string_view value,
    float size);
void text_centered(ImDrawList* draw, ImVec2 center, ImU32 color,
    std::string_view value, float size);
void text_vcentered(ImDrawList* draw, float x, float center_y, ImU32 color,
    std::string_view value, float size);

}
