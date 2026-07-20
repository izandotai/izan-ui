#include "finder.hpp"

#include <algorithm>
#include <array>
#include <string>

namespace macos {

void draw_finder(ImDrawList* draw, const Layout& layout, DesktopState& state)
{
    const float s = layout.scale;
    const bool dark = state.control.dark_mode;
    FinderState& finder = state.finder;
    const ImVec2 min = layout.point(finder.position.x, finder.position.y);
    const ImVec2 size = layout.span(finder.size.x, finder.size.y);
    const ImVec2 max = add(min, size);
    const float rounding = 13.0f * s;

    soft_shadow(draw, min, max, rounding, s, 95);
    draw->AddRectFilled(min, max,
        dark ? rgba(39, 40, 44, 251) : rgba(247, 247, 249, 251), rounding);
    draw->AddRect(
        min, max, rgba(255, 255, 255, dark ? 30 : 100), rounding, 0, 1.0f * s);

    const float sidebar_width = 234.0f;
    draw->AddRectFilled(min, { min.x + sidebar_width * s, max.y },
        dark ? rgba(45, 45, 49, 248) : rgba(223, 227, 232, 246), rounding,
        ImDrawFlags_RoundCornersLeft);
    draw->AddLine({ min.x + sidebar_width * s, min.y },
        { min.x + sidebar_width * s, max.y },
        dark ? rgba(255, 255, 255, 20) : rgba(100, 105, 112, 32), s);
    draw->AddLine({ min.x, min.y + 69.0f * s }, { max.x, min.y + 69.0f * s },
        dark ? rgba(255, 255, 255, 24) : rgba(95, 99, 106, 36), s);

    const std::array<ImU32, 3> traffic
        = { rgba(255, 95, 87), rgba(255, 189, 46), rgba(40, 201, 64) };
    const bool traffic_hovered = contains(ImGui::GetIO().MousePos,
        { min.x + 10.0f * s, min.y + 8.0f * s },
        { min.x + 76.0f * s, min.y + 37.0f * s });
    for (int i = 0; i < 3; ++i) {
        const ImVec2 center { min.x + (21.0f + i * 22.0f) * s,
            min.y + 22.0f * s };
        draw->AddCircleFilled(center, 7.0f * s, traffic[i]);
        draw->AddCircle(center, 7.0f * s, rgba(0, 0, 0, 45), 20, s);
        if (traffic_hovered) {
            if (i == 0) {
                draw->AddLine(add(center, { -2.3f * s, -2.3f * s }),
                    add(center, { 2.3f * s, 2.3f * s }), rgba(75, 35, 31, 180),
                    s);
                draw->AddLine(add(center, { 2.3f * s, -2.3f * s }),
                    add(center, { -2.3f * s, 2.3f * s }), rgba(75, 35, 31, 180),
                    s);
            } else if (i == 1) {
                draw->AddLine(add(center, { -3.0f * s, 0 }),
                    add(center, { 3.0f * s, 0 }), rgba(85, 60, 22, 180), s);
            } else {
                draw->AddTriangleFilled(add(center, { -3.0f * s, 2.0f * s }),
                    add(center, { 2.0f * s, 2.0f * s }),
                    add(center, { 2.0f * s, -3.0f * s }),
                    rgba(20, 82, 31, 180));
                draw->AddTriangleFilled(add(center, { 3.0f * s, -2.0f * s }),
                    add(center, { -2.0f * s, -2.0f * s }),
                    add(center, { -2.0f * s, 3.0f * s }),
                    rgba(20, 82, 31, 180));
            }
        }
        if (hit_rect(("##traffic-" + std::to_string(i)).c_str(),
                add(center, { -9.0f * s, -9.0f * s }),
                add(center, { 9.0f * s, 9.0f * s }))) {
            if (i == 0) {
                finder.visible = false;
            } else if (i == 1) {
                finder.minimized = true;
            } else if (!finder.maximized) {
                finder.restore_position = finder.position;
                finder.restore_size = finder.size;
                finder.position = { 24.0f, 40.0f };
                finder.size = { 1392.0f, 744.0f };
                finder.maximized = true;
            } else {
                finder.position = finder.restore_position;
                finder.size = finder.restore_size;
                finder.maximized = false;
            }
        }
    }

    // A wide empty section of the toolbar behaves as the native
    // title-bar drag target.
    const ImVec2 drag_min { min.x + 360.0f * s, min.y };
    const ImVec2 drag_max { max.x - 240.0f * s, min.y + 44.0f * s };
    ImGui::SetCursorScreenPos(drag_min);
    ImGui::InvisibleButton("##finder-drag", sub(drag_max, drag_min));
    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)
        && !finder.maximized) {
        finder.position.x += ImGui::GetIO().MouseDelta.x / s;
        finder.position.y += ImGui::GetIO().MouseDelta.y / s;
        finder.position.x = std::clamp(
            finder.position.x, -finder.size.x + 180.0f, kCanvasWidth - 180.0f);
        finder.position.y
            = std::clamp(finder.position.y, 31.0f, kCanvasHeight - 120.0f);
    }

    // Back/forward navigation controls.
    const ImU32 toolbar_color = dark ? rgba(227, 228, 231) : rgba(52, 54, 60);
    const ImVec2 back_center { min.x + (sidebar_width + 31.0f) * s,
        min.y + 35.0f * s };
    const ImVec2 forward_center { min.x + (sidebar_width + 68.0f) * s,
        min.y + 35.0f * s };
    draw->AddLine(add(back_center, { 3.0f * s, -5.0f * s }),
        add(back_center, { -2.0f * s, 0 }), toolbar_color, 1.7f * s);
    draw->AddLine(add(back_center, { -2.0f * s, 0 }),
        add(back_center, { 3.0f * s, 5.0f * s }), toolbar_color, 1.7f * s);
    draw->AddLine(add(forward_center, { -3.0f * s, -5.0f * s }),
        add(forward_center, { 2.0f * s, 0 }), rgba(125, 126, 131), 1.7f * s);
    draw->AddLine(add(forward_center, { 2.0f * s, 0 }),
        add(forward_center, { -3.0f * s, 5.0f * s }), rgba(125, 126, 131),
        1.7f * s);

    const char* title_label = "最近使用";
    switch (finder.location) {
    case Location::AirDrop:
        title_label = "隔空投送";
        break;
    case Location::Applications:
        title_label = "应用程序";
        break;
    case Location::Desktop:
        title_label = "桌面";
        break;
    case Location::Documents:
        title_label = "文稿";
        break;
    case Location::Downloads:
        title_label = "下载";
        break;
    case Location::ICloud:
        title_label = "iCloud 云盘";
        break;
    case Location::Recent:
        break;
    }
    text(draw, { min.x + (sidebar_width + 103.0f) * s, min.y + 22.0f * s },
        toolbar_color, title_label, 16.2f * s);

    // Toolbar view/share controls.
    const float tools_x = max.x - 338.0f * s;
    for (int col = 0; col < 3; ++col) {
        for (int row = 0; row < 3; ++row) {
            draw->AddRectFilled(
                { tools_x + col * 5.0f * s, min.y + (27.0f + row * 5.0f) * s },
                { tools_x + (col * 5.0f + 2.5f) * s,
                    min.y + (29.5f + row * 5.0f) * s },
                toolbar_color, 0.7f * s);
        }
    }
    draw->AddCircle({ max.x - 291.0f * s, min.y + 35.0f * s }, 8.0f * s,
        toolbar_color, 20, 1.2f * s);
    draw->AddLine({ max.x - 291.0f * s, min.y + 38.0f * s },
        { max.x - 291.0f * s, min.y + 24.0f * s }, toolbar_color, 1.2f * s);
    draw->AddTriangleFilled({ max.x - 295.0f * s, min.y + 27.0f * s },
        { max.x - 287.0f * s, min.y + 27.0f * s },
        { max.x - 291.0f * s, min.y + 21.5f * s }, toolbar_color);

    const ImVec2 search_min { max.x - 224.0f * s, min.y + 17.0f * s };
    const ImVec2 search_max { max.x - 16.0f * s, min.y + 53.0f * s };
    draw->AddRectFilled(search_min, search_max,
        dark ? rgba(91, 92, 97, 150) : rgba(255, 255, 255, 185), 8.0f * s);
    draw->AddRect(search_min, search_max,
        dark ? rgba(255, 255, 255, 28) : rgba(91, 94, 102, 42), 8.0f * s, 0, s);
    draw_magnifier(draw, { search_min.x + 17.0f * s, search_min.y + 17.0f * s },
        s, rgba(114, 116, 122));

    ImGui::SetCursorScreenPos(
        { search_min.x + 31.0f * s, search_min.y + 6.0f * s });
    ImGui::SetNextItemWidth(search_max.x - search_min.x - 38.0f * s);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 1.0f * s, 2.0f * s });
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, rgba(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_Text, toolbar_color);
    ImGui::PushStyleColor(ImGuiCol_TextDisabled, rgba(115, 116, 121));
    ImGui::InputTextWithHint(
        "##finder-search", "搜索", finder.search.data(), finder.search.size());
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);

    draw_finder_sidebar(draw, layout, state, { min.x, min.y + 70.0f * s },
        { sidebar_width * s, size.y - 70.0f * s });
    draw_finder_content(draw, layout, state,
        { min.x + sidebar_width * s, min.y + 70.0f * s },
        { size.x - sidebar_width * s, size.y - 70.0f * s });
}

}
