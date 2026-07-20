#pragma once

#include <imgui.h>

#include "paint.hpp"
#include "state.hpp"

namespace macos {

// The translucent top bar: apple mark, app menus, battery/wifi/
// control-center status cluster and the live clock. Toggles the
// apple menu and control center panels.
void draw_menu_bar(ImDrawList* draw, const Layout& layout, DesktopState& state);

}
