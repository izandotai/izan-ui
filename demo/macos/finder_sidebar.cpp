#include "finder.hpp"

#include <array>
#include <string>
#include <utility>

namespace macos {

void draw_finder_sidebar(ImDrawList* draw, const Layout& layout,
    DesktopState& state, ImVec2 pos, ImVec2 size)
{
    const float s = layout.scale;
    const bool dark = state.control.dark_mode;
    FinderState& finder = state.finder;
    const ImU32 primary = dark ? rgba(232, 233, 235) : rgba(50, 52, 57);
    const ImU32 secondary = dark ? rgba(168, 169, 174) : rgba(104, 106, 112);
    draw->PushClipRect(pos, add(pos, size), true);

    struct SidebarItem {
        const char* label;
        Location location;
        int icon;
    };

    const std::array<SidebarItem, 7> entries = { {
        { "隔空投送", Location::AirDrop, 0 },
        { "最近使用", Location::Recent, 1 },
        { "应用程序", Location::Applications, 2 },
        { "桌面", Location::Desktop, 3 },
        { "文稿", Location::Documents, 4 },
        { "下载", Location::Downloads, 5 },
        { "iCloud 云盘", Location::ICloud, 6 },
    } };

    text(draw, { pos.x + 18.0f * s, pos.y + 14.0f * s }, secondary, "个人收藏",
        12.8f * s);
    for (int i = 0; i < 6; ++i) {
        const SidebarItem& entry = entries[i];
        const float y = pos.y + (38.0f + i * 34.0f) * s;
        const ImVec2 row_min { pos.x + 9.0f * s, y - 3.0f * s };
        const ImVec2 row_max { pos.x + size.x - 9.0f * s, y + 28.0f * s };
        if (hit_rect(
                ("##sidebar-" + std::to_string(i)).c_str(), row_min, row_max)) {
            finder.location = entry.location;
            finder.selected_item = -1;
        }
        if (finder.location == entry.location) {
            draw->AddRectFilled(row_min, row_max,
                dark ? rgba(255, 255, 255, 34) : rgba(105, 110, 119, 38),
                6.0f * s);
        }
        const ImVec2 icon { pos.x + 28.0f * s, y + 12.0f * s };
        const ImU32 blue = rgba(31, 132, 235);
        switch (entry.icon) {
        case 0:
            draw->AddCircle(icon, 8.0f * s, blue, 24, 1.6f * s);
            draw->AddCircle(icon, 4.0f * s, blue, 20, 1.5f * s);
            draw->AddCircleFilled(icon, 1.7f * s, blue);
            break;
        case 1:
            draw->AddCircle(icon, 8.0f * s, blue, 24, 1.6f * s);
            draw->AddLine(icon, add(icon, { 0, -5.0f * s }), blue, 1.3f * s);
            draw->AddLine(
                icon, add(icon, { -4.0f * s, -2.0f * s }), blue, 1.3f * s);
            break;
        case 2:
            draw->AddLine(add(icon, { -6.0f * s, 7.0f * s }),
                add(icon, { 0, -7.0f * s }), blue, 2.3f * s);
            draw->AddLine(add(icon, { 6.0f * s, 7.0f * s }),
                add(icon, { 0, -7.0f * s }), blue, 2.3f * s);
            draw->AddLine(add(icon, { -4.0f * s, 3.0f * s }),
                add(icon, { 4.0f * s, 3.0f * s }), blue, 2.0f * s);
            break;
        case 3:
            draw->AddRectFilled(add(icon, { -8.0f * s, -6.0f * s }),
                add(icon, { 8.0f * s, 7.0f * s }), blue, 2.0f * s);
            draw->AddRectFilled(add(icon, { -6.0f * s, -9.0f * s }),
                add(icon, { -0.5f * s, -5.0f * s }), blue, 1.0f * s);
            break;
        case 4:
            draw->AddRect(icon, add(icon, { 8.0f * s, 10.0f * s }), blue,
                1.5f * s, 0, 1.4f * s);
            draw->AddLine(add(icon, { 2.0f * s, 3.0f * s }),
                add(icon, { 6.0f * s, 3.0f * s }), blue, s);
            break;
        default:
            draw->AddLine(add(icon, { 0, -8.0f * s }),
                add(icon, { 0, 5.0f * s }), blue, 1.8f * s);
            draw->AddLine(add(icon, { -4.0f * s, 1.0f * s }),
                add(icon, { 0, 5.0f * s }), blue, 1.8f * s);
            draw->AddLine(add(icon, { 4.0f * s, 1.0f * s }),
                add(icon, { 0, 5.0f * s }), blue, 1.8f * s);
            break;
        }
        text(draw, { pos.x + 47.0f * s, y + 2.2f * s }, primary, entry.label,
            14.3f * s);
    }

    const float cloud_heading_y = pos.y + 259.0f * s;
    text(draw, { pos.x + 18.0f * s, cloud_heading_y }, secondary, "iCloud",
        12.8f * s);
    const SidebarItem& cloud = entries[6];
    const ImVec2 row_min { pos.x + 9.0f * s, cloud_heading_y + 22.0f * s };
    const ImVec2 row_max { pos.x + size.x - 9.0f * s,
        cloud_heading_y + 53.0f * s };
    if (hit_rect("##sidebar-cloud", row_min, row_max)) {
        finder.location = cloud.location;
        finder.selected_item = -1;
    }
    if (finder.location == Location::ICloud) {
        draw->AddRectFilled(row_min, row_max,
            dark ? rgba(255, 255, 255, 34) : rgba(105, 110, 119, 38),
            6.0f * s);
    }
    const ImVec2 cloud_icon { pos.x + 28.0f * s, cloud_heading_y + 37.0f * s };
    draw->AddCircleFilled(
        add(cloud_icon, { -4.5f * s, 1.0f * s }), 5.0f * s, rgba(31, 132, 235));
    draw->AddCircleFilled(
        add(cloud_icon, { 1.0f * s, -2.0f * s }), 6.0f * s, rgba(31, 132, 235));
    draw->AddCircleFilled(
        add(cloud_icon, { 6.0f * s, 2.0f * s }), 4.5f * s, rgba(31, 132, 235));
    draw->AddRectFilled(add(cloud_icon, { -8.0f * s, 1.0f * s }),
        add(cloud_icon, { 9.0f * s, 6.0f * s }), rgba(31, 132, 235), 2.0f * s);
    text(draw, { pos.x + 47.0f * s, cloud_heading_y + 27.0f * s }, primary,
        cloud.label, 14.3f * s);

    const float tags_y = cloud_heading_y + 79.0f * s;
    text(draw, { pos.x + 18.0f * s, tags_y }, secondary, "标签", 12.8f * s);
    const std::array<std::pair<const char*, ImU32>, 3> tags = { {
        { "工作", rgba(246, 73, 73) },
        { "个人", rgba(248, 167, 44) },
        { "重要", rgba(72, 180, 95) },
    } };
    for (int i = 0; i < static_cast<int>(tags.size()); ++i) {
        const float y = tags_y + (29.0f + i * 30.0f) * s;
        draw->AddCircleFilled(
            { pos.x + 28.0f * s, y + 7.0f * s }, 6.0f * s, tags[i].second);
        text(draw, { pos.x + 47.0f * s, y - 2.0f * s }, primary, tags[i].first,
            14.0f * s);
    }
    draw->PopClipRect();
}

}
