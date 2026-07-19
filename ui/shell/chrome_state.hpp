#pragma once

#include <imgui.h>

#include <string>

namespace izan::ui {

// Frame-to-frame state of the custom window chrome: theme, opacity,
// the snap-layout popup's hover machinery and the deferred tooltip.
// Application state lives in the application; only the chrome's own
// moving parts belong here.
struct ChromeState {
    int theme_index = 0;
    float window_opacity = 0.96f;
    bool snap_layout_open = false;
    double snap_layout_hover_started = -1.0;
    double snap_layout_last_hovered = -1.0;
    ImVec2 snap_layout_anchor {};
    ImVec2 snap_layout_button_min {};
    ImVec2 snap_layout_button_max {};
    bool pending_tooltip_visible = false;
    std::string pending_tooltip_text;
    ImVec2 pending_tooltip_anchor {};
    bool window_opacity_needs_apply = false;
    bool request_exit = false;
};

}
