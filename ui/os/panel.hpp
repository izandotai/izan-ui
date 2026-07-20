#pragma once

#include <imgui.h>

#include <array>
#include <vector>

#include "ui/os/app.hpp"
#include "ui/os/wm.hpp"

namespace izan::os {

// The Cinnamon-style bottom panel: launcher menu, quick launchers,
// one taskbar button per running app, tray and clock — the whole
// furniture set in a single strip, drawn to match the Mint desktop.
// Its rectangles (strip and open menu) are handed to the window
// manager so window input never fires underneath them.
class Panel {
public:
    float height(float em) const;

    void frame(Wm& wm, const std::vector<App*>& apps, ImVec2 view_min,
        ImVec2 view_max);

    // The click territory the panel claimed last frame.
    const std::vector<OsRect>& blocked() const
    {
        return blocked_;
    }

private:
    void draw_menu(Wm& wm, const std::vector<App*>& apps, ImVec2 panel_min,
        ImVec2 panel_max);

    std::vector<OsRect> blocked_;
    bool menu_open_ = false;
    int menu_category_ = 0;
    int menu_app_ = -1;
    std::array<char, 96> menu_search_ {};
};

}
