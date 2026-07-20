#pragma once

#include <imgui.h>

#include <vector>

#include "ui/os/app.hpp"
#include "ui/os/theme.hpp"

namespace izan::os {

struct WindowState {
    App* app = nullptr;
    ImVec2 pos {};
    ImVec2 size {};
    ImVec2 restore_pos {};
    ImVec2 restore_size {};
    bool open = false;
    bool minimized = false;
    bool maximized = false;
    bool spawn_focus = false;
};

// The window manager — mechanism only, no pixels. Input runs as a
// pre-pass at the top of the frame against the current rectangles,
// so a drag lands in the same frame it happens: the inner windows
// track the cursor with the same latency as the native frame. All
// painting is delegated to the active Theme.
class Wm {
public:
    void attach(App* app);

    // Open (or restore) and raise an app's window.
    void launch(App* app);

    void set_theme(const Theme* theme)
    {
        theme_ = theme;
    }

    const Theme& theme() const
    {
        return theme_ ? *theme_ : mac_theme();
    }

    // One frame: input pre-pass, then paint. `blocked_min/max` is a
    // rectangle (the dock shelf) whose clicks belong to someone else.
    void frame(ImVec2 ws_min, ImVec2 ws_max, ImVec2 blocked_min,
        ImVec2 blocked_max);

    App* focused() const;
    bool running(const App* app) const;
    bool minimized(const App* app) const;

private:
    int index_of(const App* app) const;
    void raise(int index);
    void paint_window(int index);

    std::vector<WindowState> windows_;
    std::vector<int> z_; // bottom → top, open windows only
    // launch() before the first frame has no font metrics and no
    // workspace to place a window in; those launches wait here and
    // land at the top of the next frame.
    std::vector<App*> pending_;
    const Theme* theme_ = nullptr;
    bool framed_ = false;
    int drag_ = -1;
    int resize_ = -1;
    int spawn_count_ = 0;
    ImVec2 ws_min_ {};
    ImVec2 ws_max_ {};
};

}
