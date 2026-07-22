#pragma once

#include <imgui.h>

#include <array>
#include <string_view>
#include <utility>
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

    // Roster mode: the menu's application rows list the ATTACHED apps
    // (name, mark, id — every row really launches) instead of the
    // Mint reference's showcase rows. Off by default: the acceptance
    // replica stays word for word.
    void set_menu_roster(bool on)
    {
        menu_roster_ = on;
    }

    // Installed applications are independent from live App instances. An
    // empty catalog preserves the original attached-app roster behavior.
    void set_catalog(std::vector<AppDescriptor> catalog)
    {
        catalog_ = std::move(catalog);
    }

    void request_launch(std::string_view id);
    std::vector<LaunchRequest> take_launch_requests();

    // Open the launcher menu from outside — a keyboard shortcut's or
    // a headless probe's door; clicking the Menu button stays the
    // usual road.
    void open_menu()
    {
        menu_open_ = true;
    }

    float menu_scroll() const
    {
        return menu_scroll_;
    }

    // Nudge the roster scroll from outside (probes, future keys).
    // Physical pixels; the frame clamps it to the list's span.
    void scroll_menu(float px)
    {
        menu_scroll_ += px;
    }

private:
    void draw_menu(Wm& wm, const std::vector<App*>& apps, ImVec2 panel_min,
        ImVec2 panel_max);

    std::vector<OsRect> blocked_;
    bool menu_open_ = false;
    bool menu_roster_ = false;
    float menu_scroll_ = 0.0f;
    int menu_category_ = 0;
    int menu_app_ = -1;
    std::array<char, 96> menu_search_ {};
    std::vector<AppDescriptor> catalog_;
    std::vector<LaunchRequest> launch_requests_;
};

}
