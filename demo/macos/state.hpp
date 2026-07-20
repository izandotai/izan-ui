#pragma once

#include <imgui.h>

#include <array>

// Each component owns a small state struct; the desktop aggregates
// them. Cross-component behavior (the dock re-opening Finder, the
// apple menu jumping to Settings) is expressed as one component
// touching another's struct — visible in the signatures, not hidden
// in a god object.

namespace macos {

enum class Location {
    AirDrop,
    Recent,
    Applications,
    Desktop,
    Documents,
    Downloads,
    ICloud
};

struct FinderState {
    bool visible = true;
    bool minimized = false;
    bool maximized = false;
    ImVec2 position { 160.0f, 78.0f };
    ImVec2 size { 1120.0f, 704.0f };
    ImVec2 restore_position { 160.0f, 78.0f };
    ImVec2 restore_size { 1120.0f, 704.0f };
    Location location = Location::Recent;
    // Shared selection: file-grid cells own 0..99, desktop icons
    // live at 100+.
    int selected_item = -1;
    std::array<char, 96> search {};
};

struct DockState {
    int active_item = 0;
};

struct MenuBarState {
    bool apple_menu_open = false;
    bool control_center_open = false;
};

struct ControlState {
    bool wifi_enabled = true;
    bool bluetooth_enabled = true;
    bool dark_mode = false;
    float brightness = 0.82f;
    float volume = 0.64f;
};

struct DesktopState {
    MenuBarState menu;
    FinderState finder;
    DockState dock;
    ControlState control;
};

}
