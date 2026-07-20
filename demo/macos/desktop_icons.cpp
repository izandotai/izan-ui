#include "desktop_icons.hpp"

#include <array>
#include <string>

namespace macos {

void draw_desktop_icons(
    ImDrawList* draw, const Layout& layout, DesktopState& state)
{
    const float s = layout.scale;
    const float x = 1366.0f;

    struct DesktopItem {
        const char* label;
        float y;
        int type;
    };

    const std::array<DesktopItem, 3> items = { {
        { "Macintosh HD", 84.0f, 0 },
        { "项目资料", 178.0f, 1 },
        { "桌面图片", 272.0f, 1 },
    } };

    for (int i = 0; i < static_cast<int>(items.size()); ++i) {
        const DesktopItem& item = items[i];
        const ImVec2 center = layout.point(x, item.y);
        const ImVec2 hit_min = layout.point(x - 55.0f, item.y - 34.0f);
        const ImVec2 hit_max = layout.point(x + 55.0f, item.y + 48.0f);
        if (hit_rect(
                ("##desktop-" + std::to_string(i)).c_str(), hit_min, hit_max)) {
            state.finder.selected_item = 100 + i;
        }
        const bool selected = state.finder.selected_item == 100 + i;
        if (selected) {
            draw->AddRectFilled(layout.point(x - 49.0f, item.y - 31.0f),
                layout.point(x + 49.0f, item.y + 48.0f),
                rgba(64, 126, 225, 125), 6.0f * s);
        }
        if (item.type == 0) {
            draw->AddRectFilled(layout.point(x - 22.0f, item.y - 23.0f),
                layout.point(x + 22.0f, item.y + 15.0f), rgba(202, 207, 211),
                5.0f * s);
            draw->AddRectFilled(layout.point(x - 20.0f, item.y - 20.0f),
                layout.point(x + 20.0f, item.y - 10.0f), rgba(231, 234, 236),
                4.0f * s);
            draw->AddCircleFilled(layout.point(x + 13.0f, item.y + 8.0f),
                2.0f * s, rgba(70, 187, 104));
        } else {
            draw_folder(draw, center, s * 1.08f, selected);
        }
        const float label_size = 13.7f * s;
        const ImVec2 label_center = layout.point(x, item.y + 35.0f);
        const ImVec2 dimensions = ImGui::GetFont()->CalcTextSizeA(
            label_size, 1000.0f, 0.0f, item.label);
        const ImVec2 position { label_center.x - dimensions.x * 0.5f,
            label_center.y - dimensions.y * 0.5f };
        if (selected) {
            draw->AddRectFilled(
                { position.x - 4.0f * s, position.y - 1.0f * s },
                { position.x + dimensions.x + 4.0f * s,
                    position.y + dimensions.y + 1.0f * s },
                rgba(43, 105, 213), 3.0f * s);
        }
        shadowed_text(draw, position, item.label, label_size);
    }
}

}
