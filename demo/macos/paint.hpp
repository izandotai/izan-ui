#pragma once

#include <imgui.h>

#include <string_view>

// The demo's shared painting vocabulary: the design-canvas layout,
// color and text helpers, and the recurring glyphs several
// components draw (folders, documents, dock icons, status icons).

namespace macos {

inline constexpr float kCanvasWidth = 1440.0f;
inline constexpr float kCanvasHeight = 900.0f;

// The 1440x900 design canvas mapped into whatever rectangle the
// host hands over: uniform scale, centered origin.
struct Layout {
    float scale = 1.0f;
    ImVec2 origin {};
    ImVec2 view_min {};
    ImVec2 view_max {};

    static Layout fit(ImVec2 pos, ImVec2 size);

    ImVec2 point(float x, float y) const;
    ImVec2 span(float x, float y) const;
    float px(float value) const;
};

ImU32 rgba(int red, int green, int blue, int alpha = 255);
ImU32 mix_color(ImU32 a, ImU32 b, float amount);
ImVec2 add(ImVec2 a, ImVec2 b);
ImVec2 sub(ImVec2 a, ImVec2 b);
bool contains(ImVec2 point, ImVec2 min, ImVec2 max);

void text(ImDrawList* draw, ImVec2 position, ImU32 color,
    std::string_view value, float size);
void centered_text(ImDrawList* draw, ImVec2 center, ImU32 color,
    std::string_view value, float size);
void shadowed_text(
    ImDrawList* draw, ImVec2 position, std::string_view value, float size);
void soft_shadow(ImDrawList* draw, ImVec2 min, ImVec2 max, float rounding,
    float scale, int strength = 70);

// An invisible button over a rectangle; true on left click.
bool hit_rect(const char* id, ImVec2 min, ImVec2 max);

void draw_wifi(ImDrawList* draw, ImVec2 center, float scale, ImU32 color);
void draw_control_glyph(
    ImDrawList* draw, ImVec2 center, float scale, ImU32 color);
void draw_battery(ImDrawList* draw, ImVec2 min, float scale, ImU32 color);
void draw_apple(ImDrawList* draw, ImVec2 center, float scale, ImU32 color);
void draw_magnifier(ImDrawList* draw, ImVec2 center, float scale, ImU32 color);
void draw_folder(
    ImDrawList* draw, ImVec2 center, float scale, bool selected = false);
void draw_document(ImDrawList* draw, ImVec2 center, float scale, ImU32 tint);
void draw_dock_icon(ImDrawList* draw, int index, ImVec2 center, float size);

}
