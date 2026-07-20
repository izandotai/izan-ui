#pragma once

#include <imgui.h>

#include "paint.hpp"
#include "state.hpp"

namespace macos {

// The control center panel: wifi/bluetooth/airdrop toggles, focus
// and dark-mode cards, brightness and volume sliders.
void draw_control_center(
    ImDrawList* draw, const Layout& layout, DesktopState& state);

}
