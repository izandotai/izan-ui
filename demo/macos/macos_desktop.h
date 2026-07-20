#pragma once

#include "imgui.h"

#include <array>
#include <string>

class MacOSDesktop {
public:
    void render();

private:
    enum class Location {
        AirDrop,
        Recent,
        Applications,
        Desktop,
        Documents,
        Downloads,
        ICloud
    };

    struct Layout {
        float scale = 1.0f;
        ImVec2 origin {};
        ImVec2 display {};

        [[nodiscard]] ImVec2 point(float x, float y) const;
        [[nodiscard]] ImVec2 size(float x, float y) const;
        [[nodiscard]] float px(float value) const;
    };

    Layout layout_ {};
    ImVec2 finder_position_ { 160.0f, 78.0f };
    ImVec2 finder_restore_position_ { 160.0f, 78.0f };
    ImVec2 finder_restore_size_ { 1120.0f, 704.0f };
    ImVec2 finder_size_ { 1120.0f, 704.0f };
    Location location_ = Location::Recent;
    int selected_item_ = -1;
    int active_dock_item_ = 0;
    bool finder_visible_ = true;
    bool finder_minimized_ = false;
    bool finder_maximized_ = false;
    bool apple_menu_open_ = false;
    bool control_center_open_ = false;
    bool wifi_enabled_ = true;
    bool bluetooth_enabled_ = true;
    bool dark_mode_ = false;
    float brightness_ = 0.82f;
    float volume_ = 0.64f;
    std::array<char, 96> search_ {};

    void update_layout();
    void draw_wallpaper(ImDrawList* draw) const;
    void draw_menu_bar(ImDrawList* draw);
    void draw_desktop_icons(ImDrawList* draw);
    void draw_finder(ImDrawList* draw);
    void draw_finder_sidebar(ImDrawList* draw, ImVec2 pos, ImVec2 size);
    void draw_finder_content(ImDrawList* draw, ImVec2 pos, ImVec2 size);
    void draw_dock(ImDrawList* draw);
    void draw_apple_menu(ImDrawList* draw);
    void draw_control_center(ImDrawList* draw);

    bool hit_rect(const char* id, ImVec2 min, ImVec2 max);
};
