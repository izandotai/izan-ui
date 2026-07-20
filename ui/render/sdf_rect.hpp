#pragma once

#include <imgui.h>

// Analytic rounded rectangles: a fragment shader measures each pixel's
// distance to the ideal rounded-rect outline and converts it to
// coverage — the curve is computed, not approximated, so corners
// stay smooth at any zoom, with four vertices instead of a polygon
// fan. Falls back to the polygon path if the shader cannot compile,
// so rendering never breaks.

namespace izan::render {

struct SdfRect {
    ImVec2 min {};
    ImVec2 max {};
    // Corner radii in pixels: top-left, top-right, bottom-right,
    // bottom-left.
    float radius[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    ImU32 fill = 0;
    ImU32 border = 0; // drawn inside the silhouette edge
    float border_px = 0.0f;
    // > 0 turns the fill into a soft shadow: solid inside the
    // silhouette, a gaussian tail of this sigma outside — one draw
    // replaces the layered polygon rings that used to fringe every
    // corner. Border is ignored in this mode.
    float soft_px = 0.0f;
};

// Queue one rectangle into the draw list at the current position in
// the paint order. Safe to call from any imgui window's painter.
void sdf_rect(ImDrawList* draw, const SdfRect& rect);

}
