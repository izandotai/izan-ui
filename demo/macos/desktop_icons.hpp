#pragma once

#include <imgui.h>

#include "paint.hpp"
#include "state.hpp"

namespace macos {

// The right-edge desktop column: Macintosh HD and two folders,
// selectable, labels pilled when selected. Shares the finder's
// selection register (desktop icons live at 100+).
void draw_desktop_icons(
    ImDrawList* draw, const Layout& layout, DesktopState& state);

}
