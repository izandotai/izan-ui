#pragma once

#include <imgui.h>

#include "paint.hpp"
#include "state.hpp"

namespace macos {

// The Dock: frosted shelf, hover magnification, name tooltips,
// running dots. Clicking Finder's tile re-opens the window.
void draw_dock(ImDrawList* draw, const Layout& layout, DesktopState& state);

}
