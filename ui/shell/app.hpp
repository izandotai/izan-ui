#pragma once

#include <imgui.h>

#include <functional>

#include "ui/shell/fonts.hpp"

struct GLFWwindow;

namespace izan::ui {

// GLFW + ImGui (docking, freetype) application lifecycle:
// init / begin_frame / end_frame / shutdown. imgui's own ini file is
// disabled; layout persistence is the host's business.

struct AppOptions {
    const char* title = "izan";
    int width = 1280;
    int height = 800;
    bool attach_parent_console = true;
    // Hardware multisampling; 0 leans on imgui's own geometry AA,
    // which is cheaper and just as clean for 2D chrome.
    int msaa_samples = 0;
    // Event-driven redraw: idle at a 4fps heartbeat, wake to full
    // rate on any input - the battery-friendly mode for a desktop
    // that sits open all day.
    bool lazy_redraw = false;
    // Present with vsync off, paced by DwmFlush: the frame lands in
    // the very next composition instead of queueing 2-3 deep behind
    // the swap chain — the difference between an inner window trailing
    // the cursor and sticking to it. IZAN_CLASSIC_PRESENT=1 falls back
    // to plain vsync at runtime; non-Windows always uses vsync.
    bool low_latency_present = true;
    // The host's typeface; the defaults reproduce the izan look.
    FontOptions fonts {};
};

class GlfwApp {
public:
    GlfwApp() = default;
    ~GlfwApp();

    GlfwApp(const GlfwApp&) = delete;
    GlfwApp& operator=(const GlfwApp&) = delete;

    // The window is created hidden and shown after the first rendered
    // frame — no white flash on startup.
    bool init(const AppOptions& options);
    void show();
    void shutdown();

    GLFWwindow* window() const
    {
        return window_;
    }

    bool should_close() const;
    void poll_events() const;

    void begin_frame() const;
    void end_frame(const ImVec4& clear_color) const;

    // Registering the whole-frame render function also hooks it up as
    // the window refresh callback: Windows traps glfwPollEvents inside
    // the modal size loop while a border is dragged, so every repaint
    // during the drag comes from that callback — skip it and resizing
    // smears the frame. run() = first frame → show() → poll+render
    // until should_close.
    void set_render_callback(std::function<void()> render);
    void run();

private:
    GLFWwindow* window_ = nullptr;
    bool initialized_ = false;
    bool lazy_ = false;
    // mutable: end_frame is const but must drop to vsync if DwmFlush
    // ever fails (composition off — remote sessions, exotic setups).
    mutable bool low_latency_ = false;
    std::function<void()> render_;
};

}
