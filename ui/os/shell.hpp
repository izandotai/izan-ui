#pragma once

#include <imgui.h>

#include <vector>

#include "ui/os/app.hpp"
#include "ui/os/dock.hpp"
#include "ui/os/menu_bar.hpp"
#include "ui/os/theme.hpp"
#include "ui/os/wm.hpp"

namespace izan::os {

// The composition root: wallpaper below, windows in the middle,
// menu bar and dock above — one frame() inside whatever rectangle
// the host chrome hands over. Apps stay owned by the host.
class Shell {
public:
    void attach(App* app);

    void set_theme(const Theme* theme)
    {
        wm_.set_theme(theme);
    }

    // The emoji the menu bar wears on its left edge.
    void set_mark(const char* mark)
    {
        mark_ = mark;
    }

    void frame(ImVec2 pos, ImVec2 size);

    Wm& wm()
    {
        return wm_;
    }

private:
    std::vector<App*> apps_;
    Wm wm_;
    Dock dock_;
    MenuBar menu_;
    const char* mark_ = "⛩️";
};

}
