#include "ui/os/menu_bar.hpp"

#include <array>
#include <cstdio>
#include <ctime>
#include <string>

#include <imgui_internal.h>

#include "ui/widgets/design.hpp"

namespace izan::os {

float MenuBar::frame(
    const Wm& wm, const char* shell_mark, ImVec2 view_min, ImVec2 view_max)
{
    const float em = ImGui::GetFontSize();
    const bool dark = ui::kit_is_dark();
    const float bar_h = em * 1.45f;
    const ImVec2 bar_min = view_min;
    const ImVec2 bar_max { view_max.x, view_min.y + bar_h };

    ImGui::SetNextWindowPos(bar_min);
    ImGui::SetNextWindowSize({ bar_max.x - bar_min.x, bar_h });
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(0, 0, 0, 0));
    ImGui::Begin("###os-menubar", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse
            | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar
            | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoDocking
            | ImGuiWindowFlags_NoFocusOnAppearing
            | ImGuiWindowFlags_NoBringToFrontOnFocus);
    ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
    ImDrawList* draw = ImGui::GetWindowDrawList();

    draw->AddRectFilled(bar_min, bar_max,
        dark ? IM_COL32(14, 16, 24, 215) : IM_COL32(238, 241, 246, 205));
    draw->AddLine({ bar_min.x, bar_max.y }, { bar_max.x, bar_max.y },
        IM_COL32(255, 255, 255, dark ? 26 : 90), 1.0f);

    const ImU32 fg
        = dark ? IM_COL32(240, 241, 244, 246) : IM_COL32(38, 40, 46, 246);

    // Left: the shell's mark, then the focused app in a stronger hand.
    float x = bar_min.x + em * 0.8f;
    {
        ImGui::PushFont(nullptr, em * 0.95f);
        const ImVec2 ms = ImGui::CalcTextSize(shell_mark);
        draw->AddText(
            { x, bar_min.y + (bar_h - ms.y) * 0.5f }, IM_COL32_WHITE,
            shell_mark);
        x += ms.x + em * 0.75f;
        ImGui::PopFont();
    }
    {
        const App* app = wm.focused();
        const char* name = app ? app->name() : "桌面";
        ImGui::PushFont(nullptr, em * 0.9f);
        const ImVec2 ns = ImGui::CalcTextSize(name);
        draw->AddText({ x, bar_min.y + (bar_h - ns.y) * 0.5f }, fg, name);
        ImGui::PopFont();
    }

    // Right: the live clock.
    {
        std::time_t now = std::time(nullptr);
        std::tm local {};
#if defined(_WIN32)
        localtime_s(&local, &now);
#else
        localtime_r(&now, &local);
#endif
        static constexpr std::array<const char*, 7> weekdays
            = { "周日", "周一", "周二", "周三", "周四", "周五", "周六" };
        char clock[64] {};
        std::snprintf(clock, sizeof(clock), "%s %d月%d日  %02d:%02d",
            weekdays[static_cast<std::size_t>(local.tm_wday)],
            local.tm_mon + 1, local.tm_mday, local.tm_hour, local.tm_min);
        ImGui::PushFont(nullptr, em * 0.85f);
        const ImVec2 cs = ImGui::CalcTextSize(clock);
        draw->AddText({ bar_max.x - cs.x - em * 0.8f,
                          bar_min.y + (bar_h - cs.y) * 0.5f },
            fg, clock);
        ImGui::PopFont();
    }

    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
    return bar_h;
}

}
