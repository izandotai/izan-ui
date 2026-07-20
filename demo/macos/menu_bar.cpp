#include "menu_bar.hpp"

#include <array>
#include <cstdio>
#include <ctime>

namespace macos {

void draw_menu_bar(ImDrawList* draw, const Layout& layout, DesktopState& state)
{
    const float s = layout.scale;
    const bool dark = state.control.dark_mode;
    const ImVec2 bar_min = layout.view_min;
    const ImVec2 bar_max { layout.view_max.x, layout.view_min.y + 31.0f * s };
    draw->AddRectFilled(bar_min, bar_max,
        dark ? rgba(12, 15, 24, 218) : rgba(23, 28, 45, 190));
    draw->AddLine({ bar_min.x, bar_max.y }, { bar_max.x, bar_max.y },
        rgba(255, 255, 255, 28), 1.0f);

    const float left = layout.origin.x;
    const ImU32 foreground = rgba(255, 255, 255, 246);
    const ImVec2 apple_center { left + 24.0f * s, bar_min.y + 15.0f * s };
    if (hit_rect("##apple-menu-button", { left + 7.0f * s, bar_min.y },
            { left + 43.0f * s, bar_max.y })) {
        state.menu.apple_menu_open = !state.menu.apple_menu_open;
        state.menu.control_center_open = false;
    }
    if (state.menu.apple_menu_open) {
        draw->AddRectFilled({ left + 7.0f * s, bar_min.y + 2.0f * s },
            { left + 43.0f * s, bar_max.y - 2.0f * s }, rgba(255, 255, 255, 28),
            5.0f * s);
    }
    draw_apple(draw, apple_center, s, foreground);

    struct MenuItem {
        const char* label;
        float x;
        bool strong;
    };

    const std::array<MenuItem, 7> items = { {
        { "访达", 51.0f, true },
        { "文件", 105.0f, false },
        { "编辑", 158.0f, false },
        { "显示", 211.0f, false },
        { "前往", 264.0f, false },
        { "窗口", 317.0f, false },
        { "帮助", 370.0f, false },
    } };
    for (const MenuItem& item : items) {
        text(draw, { left + item.x * s, bar_min.y + 6.4f * s }, foreground,
            item.label, (item.strong ? 15.3f : 14.8f) * s);
    }

    std::time_t now = std::time(nullptr);
    std::tm local {};
#if defined(_WIN32)
    localtime_s(&local, &now);
#else
    localtime_r(&now, &local);
#endif
    char clock[64] {};
    static constexpr std::array<const char*, 7> weekdays
        = { "周日", "周一", "周二", "周三", "周四", "周五", "周六" };
    std::snprintf(clock, sizeof(clock), "%s  %d月%d日  %02d:%02d",
        weekdays[static_cast<std::size_t>(local.tm_wday)], local.tm_mon + 1,
        local.tm_mday, local.tm_hour, local.tm_min);

    const float right = layout.origin.x + kCanvasWidth * s;
    const ImVec2 control_center { right - 183.0f * s, bar_min.y + 15.0f * s };
    const ImVec2 control_min { right - 200.0f * s, bar_min.y };
    const ImVec2 control_max { right - 164.0f * s, bar_max.y };
    if (hit_rect("##control-center-button", control_min, control_max)) {
        state.menu.control_center_open = !state.menu.control_center_open;
        state.menu.apple_menu_open = false;
    }
    if (state.menu.control_center_open) {
        draw->AddRectFilled(
            { control_min.x + 2.0f * s, control_min.y + 2.0f * s },
            { control_max.x - 2.0f * s, control_max.y - 2.0f * s },
            rgba(255, 255, 255, 28), 5.0f * s);
    }

    draw_battery(
        draw, { right - 279.0f * s, bar_min.y + 10.2f * s }, s, foreground);
    draw_wifi(
        draw, { right - 236.0f * s, bar_min.y + 20.0f * s }, s, foreground);
    draw_control_glyph(draw, control_center, s, foreground);
    text(draw, layout.point(1287.0f, 6.5f), foreground,
        std::string_view(clock), 14.4f * s);
}

}
