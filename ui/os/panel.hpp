#pragma once

#include <imgui.h>

#include <vector>

#include "ui/os/app.hpp"
#include "ui/os/wm.hpp"

namespace izan::os {

// The bottom panel: launcher menu, one taskbar button per running
// app, a clock on the right — the whole furniture set of the
// panel-cultured desktops in a single strip. Its rectangles (strip
// and open menu) are handed to the window manager so window input
// never fires underneath them.
class Panel {
public:
    float height(float em) const
    {
        return em * 2.0f;
    }

    void frame(Wm& wm, const std::vector<App*>& apps, const char* shell_mark,
        ImVec2 view_min, ImVec2 view_max);

    // The click territory the panel claimed last frame.
    const std::vector<OsRect>& blocked() const
    {
        return blocked_;
    }

private:
    std::vector<OsRect> blocked_;
    bool menu_open_ = false;
};

}
