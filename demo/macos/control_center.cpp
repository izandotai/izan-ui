#include "control_center.hpp"

#include <algorithm>
#include <array>
#include <string>

namespace macos {

void draw_control_center(
    ImDrawList* draw, const Layout& layout, DesktopState& state)
{
    const float s = layout.scale;
    ControlState& control = state.control;
    const bool dark = control.dark_mode;
    const ImVec2 min = layout.point(1063.0f, 38.0f);
    const ImVec2 max = layout.point(1426.0f, 366.0f);
    const ImU32 panel = dark ? rgba(49, 50, 55, 239) : rgba(235, 239, 245, 235);
    const ImU32 card = dark ? rgba(91, 92, 98, 142) : rgba(255, 255, 255, 164);
    const ImU32 primary = dark ? rgba(247, 247, 248) : rgba(30, 31, 34);
    const ImU32 secondary = dark ? rgba(184, 185, 190) : rgba(94, 97, 103);
    soft_shadow(draw, min, max, 16.0f * s, s, 90);
    draw->AddRectFilled(min, max, panel, 16.0f * s);
    draw->AddRect(min, max, rgba(255, 255, 255, 80), 16.0f * s, 0, s);

    const ImVec2 network_min = add(min, { 12.0f * s, 12.0f * s });
    const ImVec2 network_max = add(min, { 185.0f * s, 164.0f * s });
    draw->AddRectFilled(network_min, network_max, card, 13.0f * s);

    struct Toggle {
        const char* label;
        const char* detail;
        bool* value;
        float y;
        int glyph;
    };

    std::array<Toggle, 3> toggles = { {
        { "无线局域网", control.wifi_enabled ? "Home 5G" : "关闭",
            &control.wifi_enabled, 20.0f, 0 },
        { "蓝牙", control.bluetooth_enabled ? "打开" : "关闭",
            &control.bluetooth_enabled, 66.0f, 1 },
        { "隔空投送", "仅限联系人", nullptr, 112.0f, 2 },
    } };
    for (int i = 0; i < static_cast<int>(toggles.size()); ++i) {
        Toggle& toggle = toggles[i];
        const ImVec2 center { network_min.x + 25.0f * s,
            network_min.y + (toggle.y + 11.0f) * s };
        const bool enabled = toggle.value == nullptr || *toggle.value;
        draw->AddCircleFilled(center, 16.0f * s,
            enabled ? rgba(28, 126, 235) : rgba(128, 131, 138, 145));
        if (toggle.glyph == 0) {
            draw_wifi(draw, { center.x, center.y + 5.0f * s }, 0.72f * s,
                rgba(255, 255, 255));
        } else if (toggle.glyph == 1) {
            draw->AddLine(add(center, { 0, -8.0f * s }),
                add(center, { 0, 8.0f * s }), rgba(255, 255, 255), 1.5f * s);
            draw->AddLine(add(center, { 0, -8.0f * s }),
                add(center, { 5.0f * s, -3.0f * s }), rgba(255, 255, 255),
                1.5f * s);
            draw->AddLine(add(center, { 5.0f * s, -3.0f * s }),
                add(center, { -4.0f * s, 5.0f * s }), rgba(255, 255, 255),
                1.5f * s);
            draw->AddLine(add(center, { -4.0f * s, -5.0f * s }),
                add(center, { 5.0f * s, 4.0f * s }), rgba(255, 255, 255),
                1.5f * s);
            draw->AddLine(add(center, { 5.0f * s, 4.0f * s }),
                add(center, { 0, 8.0f * s }), rgba(255, 255, 255), 1.5f * s);
        } else {
            draw->AddCircle(
                center, 8.0f * s, rgba(255, 255, 255), 24, 1.4f * s);
            draw->AddCircleFilled(center, 2.0f * s, rgba(255, 255, 255));
        }
        text(draw,
            { network_min.x + 49.0f * s,
                network_min.y + (toggle.y - 1.0f) * s },
            primary, toggle.label, 13.5f * s);
        text(draw,
            { network_min.x + 49.0f * s,
                network_min.y + (toggle.y + 16.0f) * s },
            secondary, toggle.detail, 11.4f * s);
        if (toggle.value != nullptr
            && hit_rect(("##cc-toggle-" + std::to_string(i)).c_str(),
                add(center, { -18.0f * s, -18.0f * s }),
                add(center, { 18.0f * s, 18.0f * s }))) {
            *toggle.value = !*toggle.value;
        }
    }

    const ImVec2 focus_min = add(min, { 197.0f * s, 12.0f * s });
    const ImVec2 focus_max = add(min, { 351.0f * s, 82.0f * s });
    draw->AddRectFilled(focus_min, focus_max, card, 13.0f * s);
    draw->AddCircleFilled({ focus_min.x + 27.0f * s, focus_min.y + 35.0f * s },
        16.0f * s, rgba(120, 91, 205));
    draw->AddCircleFilled(
        { focus_min.x + 31.0f * s, focus_min.y + 31.0f * s }, 9.0f * s, card);
    text(draw, { focus_min.x + 51.0f * s, focus_min.y + 18.0f * s }, primary,
        "专注模式", 13.5f * s);
    text(draw, { focus_min.x + 51.0f * s, focus_min.y + 37.0f * s }, secondary,
        "关闭", 11.5f * s);

    const ImVec2 theme_min = add(min, { 197.0f * s, 94.0f * s });
    const ImVec2 theme_max = add(min, { 351.0f * s, 164.0f * s });
    draw->AddRectFilled(theme_min, theme_max, card, 13.0f * s);
    const ImVec2 theme_center { theme_min.x + 27.0f * s,
        theme_min.y + 35.0f * s };
    draw->AddCircleFilled(theme_center, 16.0f * s,
        dark ? rgba(75, 81, 103) : rgba(105, 111, 129));
    draw->AddCircleFilled(
        add(theme_center, { 4.0f * s, -4.0f * s }), 9.0f * s, card);
    text(draw, { theme_min.x + 51.0f * s, theme_min.y + 18.0f * s }, primary,
        "深色模式", 13.5f * s);
    text(draw, { theme_min.x + 51.0f * s, theme_min.y + 37.0f * s }, secondary,
        dark ? "打开" : "关闭", 11.5f * s);
    if (hit_rect("##dark-mode", theme_min, theme_max)) {
        control.dark_mode = !control.dark_mode;
    }

    const auto draw_slider = [&](const char* id, const char* label,
                                 float& value, float y, bool volume) {
        const ImVec2 slider_min = add(min, { 12.0f * s, y * s });
        const ImVec2 slider_max = add(min, { 351.0f * s, (y + 65.0f) * s });
        draw->AddRectFilled(slider_min, slider_max, card, 13.0f * s);
        text(draw, { slider_min.x + 12.0f * s, slider_min.y + 8.0f * s },
            primary, label, 13.0f * s);
        const ImVec2 track_min { slider_min.x + 13.0f * s,
            slider_min.y + 36.0f * s };
        const ImVec2 track_max { slider_max.x - 13.0f * s,
            slider_min.y + 55.0f * s };
        draw->AddRectFilled(track_min, track_max,
            dark ? rgba(28, 29, 33, 185) : rgba(194, 198, 205, 190),
            10.0f * s);
        const float knob_x = track_min.x + (track_max.x - track_min.x) * value;
        draw->AddRectFilled(track_min, { knob_x, track_max.y },
            rgba(255, 255, 255, 240), 10.0f * s, ImDrawFlags_RoundCornersLeft);
        draw->AddCircleFilled({ knob_x, (track_min.y + track_max.y) * 0.5f },
            9.5f * s, rgba(255, 255, 255));
        if (volume) {
            draw->AddTriangleFilled(
                { track_min.x + 7.0f * s, track_min.y + 9.0f * s },
                { track_min.x + 13.0f * s, track_min.y + 4.0f * s },
                { track_min.x + 13.0f * s, track_min.y + 15.0f * s },
                rgba(81, 84, 91));
        } else {
            draw->AddCircleFilled(
                { track_min.x + 11.0f * s, track_min.y + 9.5f * s }, 3.3f * s,
                rgba(81, 84, 91));
        }
        ImGui::SetCursorScreenPos(track_min);
        ImGui::InvisibleButton(id, sub(track_max, track_min));
        if (ImGui::IsItemActive()) {
            value = std::clamp((ImGui::GetIO().MousePos.x - track_min.x)
                    / (track_max.x - track_min.x),
                0.0f, 1.0f);
        }
    };
    draw_slider("##brightness", "显示器", control.brightness, 176.0f, false);
    draw_slider("##volume", "声音", control.volume, 253.0f, true);

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)
        && !contains(ImGui::GetIO().MousePos, min, max)
        && ImGui::GetIO().MousePos.y > layout.point(0, 31.0f).y) {
        state.menu.control_center_open = false;
    }
}

}
