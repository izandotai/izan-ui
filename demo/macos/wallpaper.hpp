#pragma once

#include <imgui.h>

#include "paint.hpp"
#include "state.hpp"

namespace macos {

// The mesh-gradient wallpaper: banded vertical ramp, seven glow
// pools, a dusting of grain. Fills the layout's whole view.
void draw_wallpaper(
    ImDrawList* draw, const Layout& layout, const DesktopState& state);

}
