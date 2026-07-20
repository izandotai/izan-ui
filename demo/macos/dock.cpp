#include "dock.hpp"

#include <array>
#include <cmath>
#include <string>

namespace macos {

void draw_dock(ImDrawList* draw, const Layout& layout, DesktopState& state)
{
    const float s = layout.scale;
    const bool dark = state.control.dark_mode;
    constexpr int icon_count = 10;
    constexpr float base_size = 53.0f;
    constexpr float spacing = 11.0f;
    const float content_width
        = icon_count * base_size + (icon_count - 1) * spacing + 24.0f;
    const float dock_x = (kCanvasWidth - content_width) * 0.5f;
    const float dock_y = 810.0f;
    const ImVec2 dock_min = layout.point(dock_x - 15.0f, dock_y - 9.0f);
    const ImVec2 dock_max
        = layout.point(dock_x + content_width + 15.0f, dock_y + 69.0f);

    soft_shadow(draw, dock_min, dock_max, 22.0f * s, s, 70);
    draw->AddRectFilled(dock_min, dock_max,
        dark ? rgba(46, 47, 52, 190) : rgba(237, 241, 246, 173), 22.0f * s);
    draw->AddRect(
        dock_min, dock_max, rgba(255, 255, 255, 102), 22.0f * s, 0, s);
    draw->AddLine(
        layout.point(dock_x + content_width - base_size - 19.0f, dock_y + 2.0f),
        layout.point(
            dock_x + content_width - base_size - 19.0f, dock_y + 58.0f),
        rgba(55, 59, 67, 72), s);

    const ImVec2 mouse = ImGui::GetIO().MousePos;
    static constexpr std::array<const char*, icon_count> labels
        = { "访达", "启动台", "Safari 浏览器", "信息", "邮件", "照片", "日历",
              "备忘录", "系统设置", "废纸篓" };

    for (int index = 0; index < icon_count; ++index) {
        const float extra_gap = index == icon_count - 1 ? 13.0f : 0.0f;
        const float reference_x = dock_x + 12.0f + base_size * 0.5f
            + index * (base_size + spacing) + extra_gap;
        const ImVec2 base_center = layout.point(reference_x, dock_y + 28.0f);
        const float distance = std::abs(mouse.x - base_center.x) / s;
        float magnification = 0.0f;
        if (mouse.y > dock_min.y - 70.0f * s
            && mouse.y < dock_max.y + 6.0f * s) {
            magnification = std::max(0.0f, 1.0f - distance / 92.0f);
        }
        const float icon_size
            = (base_size + 22.0f * magnification * magnification) * s;
        const ImVec2 center { base_center.x,
            layout.point(0, dock_y + 54.0f).y - icon_size * 0.5f };
        const ImVec2 hit_min { base_center.x - (base_size + spacing) * 0.5f * s,
            dock_min.y - 50.0f * s };
        const ImVec2 hit_max { base_center.x + (base_size + spacing) * 0.5f * s,
            dock_max.y };
        const bool hovered = contains(mouse, hit_min, hit_max);

        if (hit_rect(("##dock-" + std::to_string(index)).c_str(), hit_min,
                hit_max)) {
            state.dock.active_item = index;
            if (index == 0) {
                state.finder.visible = true;
                state.finder.minimized = false;
            }
        }

        draw_dock_icon(draw, index, center, icon_size);
        if (index == state.dock.active_item && index != 9) {
            draw->AddCircleFilled({ base_center.x, dock_max.y - 4.0f * s },
                2.1f * s, dark ? rgba(237, 238, 240) : rgba(45, 48, 54));
        }
        if (hovered) {
            const float label_size = 13.0f * s;
            const ImVec2 dimensions = ImGui::GetFont()->CalcTextSizeA(
                label_size, 1000.0f, 0.0f, labels[index]);
            const ImVec2 tooltip_min { center.x - dimensions.x * 0.5f
                    - 9.0f * s,
                center.y - icon_size * 0.5f - 36.0f * s };
            const ImVec2 tooltip_max { center.x + dimensions.x * 0.5f
                    + 9.0f * s,
                tooltip_min.y + 27.0f * s };
            soft_shadow(draw, tooltip_min, tooltip_max, 6.0f * s, s, 42);
            draw->AddRectFilled(tooltip_min, tooltip_max,
                dark ? rgba(56, 57, 62, 235) : rgba(244, 246, 249, 235),
                6.0f * s);
            centered_text(draw,
                { (tooltip_min.x + tooltip_max.x) * 0.5f,
                    (tooltip_min.y + tooltip_max.y) * 0.5f },
                dark ? rgba(245, 245, 246) : rgba(39, 41, 45), labels[index],
                label_size);
        }
    }
}

}
