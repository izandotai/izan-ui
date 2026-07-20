#pragma once

#include <imgui.h>

#include "paint.hpp"
#include "state.hpp"

namespace macos {

// The Apple menu dropdown: hover-highlighted rows, shortcut column,
// closes on any click that lands outside.
void draw_apple_menu(
    ImDrawList* draw, const Layout& layout, DesktopState& state);

}
