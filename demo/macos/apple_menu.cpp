#include "apple_menu.hpp"

#include <array>
#include <string>

namespace macos {

void draw_apple_menu(ImDrawList* draw, const Layout& layout, DesktopState& state)
{
    const float s = layout.scale;
    const bool dark = state.control.dark_mode;
    const ImVec2 min = layout.point(8.0f, 35.0f);
    const ImVec2 max = layout.point(286.0f, 394.0f);
    soft_shadow(draw, min, max, 12.0f * s, s, 85);
    draw->AddRectFilled(min, max,
        dark ? rgba(46, 47, 52, 242) : rgba(239, 242, 247, 239), 12.0f * s);
    draw->AddRect(min, max, rgba(255, 255, 255, 80), 12.0f * s, 0, s);

    struct AppleEntry {
        const char* label;
        const char* shortcut_key;
        float y;
        bool separator_after;
    };

    const std::array<AppleEntry, 10> entries = { {
        { "关于本机", "", 13.0f, true },
        { "系统设置…", "", 55.0f, false },
        { "App Store…", "", 84.0f, true },
        { "最近使用的项目", "›", 126.0f, true },
        { "强制退出…", "⌥⌘⎋", 168.0f, true },
        { "睡眠", "", 210.0f, false },
        { "重新启动…", "", 239.0f, false },
        { "关机…", "", 268.0f, true },
        { "锁定屏幕", "⌃⌘Q", 310.0f, false },
        { "退出登录 Izan…", "⇧⌘Q", 339.0f, false },
    } };

    const ImU32 foreground = dark ? rgba(245, 245, 246) : rgba(28, 29, 32);
    for (int i = 0; i < static_cast<int>(entries.size()); ++i) {
        const AppleEntry& entry = entries[i];
        const ImVec2 row_min { min.x + 7.0f * s, min.y + entry.y * s };
        const ImVec2 row_max { max.x - 7.0f * s, row_min.y + 27.0f * s };
        const bool hovered
            = contains(ImGui::GetIO().MousePos, row_min, row_max);
        if (hovered) {
            draw->AddRectFilled(row_min, row_max, rgba(35, 122, 229), 5.0f * s);
        }
        if (hit_rect(("##apple-entry-" + std::to_string(i)).c_str(), row_min,
                row_max)) {
            if (i == 1) {
                state.dock.active_item = 8;
            }
            state.menu.apple_menu_open = false;
        }
        text(draw, { row_min.x + 9.0f * s, row_min.y + 3.0f * s },
            hovered ? rgba(255, 255, 255) : foreground, entry.label, 14.0f * s);
        if (entry.shortcut_key[0] != '\0') {
            const ImVec2 dimensions = ImGui::GetFont()->CalcTextSizeA(
                13.0f * s, 1000.0f, 0.0f, entry.shortcut_key);
            text(draw,
                { row_max.x - dimensions.x - 9.0f * s, row_min.y + 3.8f * s },
                hovered ? rgba(255, 255, 255) : rgba(105, 106, 112),
                entry.shortcut_key, 13.0f * s);
        }
        if (entry.separator_after) {
            draw->AddLine({ min.x + 12.0f * s, row_max.y + 5.5f * s },
                { max.x - 12.0f * s, row_max.y + 5.5f * s },
                dark ? rgba(255, 255, 255, 28) : rgba(68, 70, 76, 28), s);
        }
    }

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)
        && !contains(ImGui::GetIO().MousePos, min, max)
        && ImGui::GetIO().MousePos.y > layout.point(0, 31.0f).y) {
        state.menu.apple_menu_open = false;
    }
}

}
