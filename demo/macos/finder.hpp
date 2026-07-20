#pragma once

#include <imgui.h>

#include "paint.hpp"
#include "state.hpp"

namespace macos {

// The Finder window: chrome (traffic lights, drag, toolbar, search)
// in finder.cpp, with the sidebar and the content grid as their own
// components underneath.
void draw_finder(ImDrawList* draw, const Layout& layout, DesktopState& state);

void draw_finder_sidebar(ImDrawList* draw, const Layout& layout,
    DesktopState& state, ImVec2 pos, ImVec2 size);
void draw_finder_content(ImDrawList* draw, const Layout& layout,
    DesktopState& state, ImVec2 pos, ImVec2 size);

}
