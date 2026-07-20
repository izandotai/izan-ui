#pragma once

#include <imgui.h>

#include "ui/os/wm.hpp"

namespace izan::os {

// The top bar: the shell's mark, the focused app's name, a live
// clock on the right. Translucent, always above the windows.
class MenuBar {
public:
    // Returns the bar height so the shell can carve the workspace.
    float frame(const Wm& wm, const char* shell_mark, ImVec2 view_min,
        ImVec2 view_max);
};

}
