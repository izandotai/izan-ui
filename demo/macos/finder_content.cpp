#include "finder.hpp"

#include <algorithm>
#include <array>
#include <cstdio>
#include <string>

namespace macos {

void draw_finder_content(ImDrawList* draw, const Layout& layout,
    DesktopState& state, ImVec2 pos, ImVec2 size)
{
    const float s = layout.scale;
    const bool dark = state.control.dark_mode;
    FinderState& finder = state.finder;
    const ImU32 primary = dark ? rgba(239, 239, 241) : rgba(41, 42, 46);
    const ImU32 secondary = dark ? rgba(160, 162, 167) : rgba(112, 114, 120);
    draw->PushClipRect(pos, add(pos, size), true);

    const char* heading = "最近使用";
    if (finder.location == Location::Applications)
        heading = "应用程序";
    else if (finder.location == Location::Desktop)
        heading = "桌面上的项目";
    else if (finder.location == Location::Documents)
        heading = "文稿";
    else if (finder.location == Location::Downloads)
        heading = "下载项目";
    else if (finder.location == Location::AirDrop)
        heading = "隔空投送";
    else if (finder.location == Location::ICloud)
        heading = "iCloud 云盘";

    text(draw, { pos.x + 30.0f * s, pos.y + 23.0f * s }, primary, heading,
        20.0f * s);
    text(draw, { pos.x + 30.0f * s, pos.y + 51.0f * s }, secondary,
        finder.location == Location::AirDrop ? "附近的设备会显示在这里"
                                             : "按名称排列",
        12.5f * s);

    struct FileItem {
        const char* name;
        int kind;
        ImU32 tint;
    };

    const std::array<FileItem, 10> recent = { {
        { "项目资料", 0, 0 },
        { "设计素材", 0, 0 },
        { "macOS ImGui", 0, 0 },
        { "界面预览.png", 2, rgba(115, 92, 205) },
        { "README.md", 1, rgba(72, 148, 229) },
        { "CMakeLists.txt", 1, rgba(238, 107, 65) },
        { "演示视频.mov", 2, rgba(62, 174, 200) },
        { "会议记录.pdf", 1, rgba(237, 74, 71) },
        { "归档.zip", 3, rgba(174, 145, 84) },
        { "备忘清单.txt", 1, rgba(128, 132, 142) },
    } };

    const std::array<FileItem, 10> applications = { {
        { "App Store", 4, rgba(45, 138, 239) },
        { "Safari", 4, rgba(54, 168, 234) },
        { "邮件", 4, rgba(54, 136, 235) },
        { "信息", 4, rgba(56, 200, 78) },
        { "照片", 4, rgba(231, 89, 134) },
        { "日历", 4, rgba(239, 73, 73) },
        { "备忘录", 4, rgba(245, 202, 47) },
        { "系统设置", 4, rgba(134, 139, 147) },
        { "终端", 4, rgba(53, 56, 61) },
        { "字体册", 4, rgba(55, 119, 215) },
    } };

    const auto& files
        = finder.location == Location::Applications ? applications : recent;
    const std::string query(finder.search.data());
    int visible_index = 0;
    for (int index = 0; index < static_cast<int>(files.size()); ++index) {
        const FileItem& file = files[index];
        if (!query.empty()
            && std::string(file.name).find(query) == std::string::npos) {
            continue;
        }
        if (finder.location == Location::AirDrop && index > 2) {
            continue;
        }
        const int column = visible_index % 5;
        const int row = visible_index / 5;
        const float cell_width = size.x / (5.0f * s);
        const ImVec2 cell_center { pos.x
                + (cell_width * (static_cast<float>(column) + 0.5f)) * s,
            pos.y + (126.0f + row * 151.0f) * s };
        const ImVec2 hit_min { cell_center.x - (cell_width * 0.43f) * s,
            cell_center.y - 55.0f * s };
        const ImVec2 hit_max { cell_center.x + (cell_width * 0.43f) * s,
            cell_center.y + 69.0f * s };
        if (hit_rect(("##file-" + std::to_string(index)).c_str(), hit_min,
                hit_max)) {
            finder.selected_item = index;
        }
        const bool selected = finder.selected_item == index;
        if (selected) {
            draw->AddRectFilled(hit_min, hit_max,
                dark ? rgba(45, 111, 205, 120) : rgba(56, 132, 229, 95),
                8.0f * s);
        } else if (contains(ImGui::GetIO().MousePos, hit_min, hit_max)) {
            draw->AddRectFilled(hit_min, hit_max,
                dark ? rgba(255, 255, 255, 15) : rgba(40, 45, 52, 12),
                8.0f * s);
        }

        const ImVec2 icon_center { cell_center.x, cell_center.y - 14.0f * s };
        if (file.kind == 0) {
            draw_folder(draw, icon_center, 1.38f * s, selected);
        } else if (file.kind == 1) {
            draw_document(draw, icon_center, 1.25f * s, file.tint);
        } else if (file.kind == 2) {
            draw->AddRectFilled(add(icon_center, { -29.0f * s, -23.0f * s }),
                add(icon_center, { 29.0f * s, 23.0f * s }), rgba(225, 231, 238),
                5.0f * s);
            draw->AddRectFilledMultiColor(
                add(icon_center, { -26.0f * s, -20.0f * s }),
                add(icon_center, { 26.0f * s, 20.0f * s }), rgba(55, 143, 212),
                rgba(151, 101, 198), rgba(36, 80, 130), rgba(221, 105, 154));
            draw->AddCircleFilled(add(icon_center, { 12.0f * s, -8.0f * s }),
                5.0f * s, rgba(255, 224, 117));
            draw->AddTriangleFilled(add(icon_center, { -25.0f * s, 19.0f * s }),
                add(icon_center, { -6.0f * s, -2.0f * s }),
                add(icon_center, { 8.0f * s, 19.0f * s }), rgba(63, 158, 124));
            draw->AddTriangleFilled(add(icon_center, { -2.0f * s, 19.0f * s }),
                add(icon_center, { 14.0f * s, 1.0f * s }),
                add(icon_center, { 26.0f * s, 19.0f * s }), rgba(48, 124, 108));
        } else if (file.kind == 3) {
            draw->AddRectFilled(add(icon_center, { -22.0f * s, -25.0f * s }),
                add(icon_center, { 22.0f * s, 25.0f * s }), rgba(207, 180, 105),
                4.0f * s);
            for (int zipper = -4; zipper <= 4; ++zipper) {
                draw->AddRectFilled(
                    add(icon_center,
                        { -2.0f * s, zipper * 5.0f * s - 2.0f * s }),
                    add(icon_center,
                        { 2.0f * s, zipper * 5.0f * s + 1.0f * s }),
                    rgba(91, 78, 53), 0.5f * s);
            }
        } else {
            const int dock_index = std::clamp(index + 2, 2, 8);
            draw_dock_icon(draw, dock_index, icon_center, 56.0f * s);
        }

        const float label_size = 13.0f * s;
        const ImVec2 label_dimensions = ImGui::GetFont()->CalcTextSizeA(
            label_size, 1000.0f, 0.0f, file.name);
        ImVec2 label_pos { cell_center.x - label_dimensions.x * 0.5f,
            cell_center.y + 30.0f * s };
        if (label_dimensions.x > cell_width * 0.85f * s) {
            label_pos.x = hit_min.x + 5.0f * s;
        }
        text(draw, label_pos, selected ? rgba(255, 255, 255) : primary,
            file.name, label_size);
        ++visible_index;
    }

    if (visible_index == 0) {
        centered_text(draw, { pos.x + size.x * 0.5f, pos.y + size.y * 0.45f },
            secondary, "没有符合条件的项目", 16.0f * s);
    }

    const float status_y = pos.y + size.y - 27.0f * s;
    draw->AddLine({ pos.x, status_y }, { pos.x + size.x, status_y },
        dark ? rgba(255, 255, 255, 22) : rgba(93, 96, 104, 30), s);
    char status[48] {};
    std::snprintf(
        status, sizeof(status), "%d 个项目，128.42 GB 可用", visible_index);
    centered_text(draw, { pos.x + size.x * 0.5f, status_y + 13.0f * s },
        secondary, status, 11.7f * s);
    draw->PopClipRect();
}

}
