#pragma once

#include <imgui.h>

#include <string>
#include <unordered_map>
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

// A close button is an intent, not permission to destroy an App while the
// window manager is still painting it. The owning host drains these requests
// after the frame and applies its application policy there.
struct CloseRequest {
    App* app = nullptr;
};

// Window geometry outlives an App instance. JS apps are destroyed on close,
// but a new instance with the same stable id should reopen where the user left
// it instead of consuming another cascade slot.
struct WindowPlacement {
    ImVec2 pos {};
    ImVec2 size {};
    ImVec2 restore_pos {};
    ImVec2 restore_size {};
    bool maximized = false;
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

    // Taskbar-button semantics: closed opens, minimized restores,
    // focused minimizes, anything else comes to the front.
    void toggle(App* app);

    // Shut an app's window from outside — the uninstall path: same
    // effect as its close button, plus any launch still waiting for
    // the first frame is withdrawn. Programmatic close is silent: it
    // does not enqueue a user CloseRequest.
    void close(App* app);

    // Close visually and enqueue one user intent. Repeated asks before the
    // owner drains the queue are coalesced.
    void request_close(App* app);

    // Transfer every close intent to the owning host. The host must finish
    // the UI frame, submit its render data, detach the app, and only then
    // destroy GPU-backed state.
    std::vector<CloseRequest> take_close_requests();

    // Forget an app completely. Unlike close(), this erases its WindowState
    // and repairs every index that referred into the compacted table. A call
    // during frame painting clears the pointer immediately and defers table
    // compaction until the frame's index snapshot is no longer in use.
    void detach(App* app);

    void set_theme(const Theme* theme)
    {
        theme_ = theme;
    }

    const Theme& theme() const
    {
        return theme_ ? *theme_ : mint_theme();
    }

    // One frame: input pre-pass, then paint. `blocked` are rectangles
    // (dock shelf, panel, open popups) whose clicks belong to someone
    // else.
    void frame(
        ImVec2 ws_min, ImVec2 ws_max, const std::vector<OsRect>& blocked);

    // Re-front the open popup chain, nesting order, innermost last.
    // The shell calls this after every self-fronting layer (panel
    // included) has spoken: the popup layer outranks everything, and
    // that verdict must be the frame's last word on display order —
    // imgui's own click-to-focus fronts a clicked modal at EndFrame,
    // and if any layer still sat above the popups, each click would
    // hoist the modal past it for one rendered frame and the layer
    // would flicker between lit and dimmed.
    void front_popups();

    App* focused() const;
    bool attached(const App* app) const;
    bool running(const App* app) const;

    // A drag or resize in flight — the host draws the cursor in-frame
    // during these so window and cursor can never slip apart.
    bool interacting() const
    {
        return drag_ >= 0 || resize_ >= 0;
    }

    bool minimized(const App* app) const;

private:
    int index_of(const App* app) const;
    void close_window(int index);
    void remember_window(int index);
    void erase_window(int index);
    void compact_detached();
    void raise(int index);
    void paint_window(int index);

    std::vector<WindowState> windows_;
    std::vector<int> z_; // bottom → top, open windows only
    // launch() before the first frame has no font metrics and no
    // workspace to place a window in; those launches wait here and
    // land at the top of the next frame.
    std::vector<App*> pending_;
    std::vector<CloseRequest> close_requests_;
    std::unordered_map<std::string, WindowPlacement> placements_;
    const Theme* theme_ = nullptr;
    bool framed_ = false;
    bool in_frame_ = false;
    int drag_ = -1;
    int resize_ = -1;
    // Absolute grab anchors: position follows the cursor exactly
    // (pos = mouse - grab), never an accumulation of deltas — deltas
    // rubber-band the moment a clamp eats part of one.
    ImVec2 grab_offset_ {};
    ImVec2 grab_size_ {};
    ImVec2 grab_mouse_ {};
    int hover_ = -1; // sole window whose hover feedback is honest
    int spawn_count_ = 0;
    ImVec2 ws_min_ {};
    ImVec2 ws_max_ {};
};

}
