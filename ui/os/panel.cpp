#include "ui/os/panel.hpp"

#include <algorithm>
#include <cstdio>
#include <string>

#include <imgui_internal.h>

#include "ui/widgets/design.hpp"

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <chrono>
#include <format>
#endif

namespace izan::os {

namespace {

    void local_clock_line(char* out, std::size_t size)
    {
#if defined(_WIN32)
        SYSTEMTIME t {};
        GetLocalTime(&t);
        std::snprintf(out, size, "%02u:%02u  %02u/%02u/%04u", t.wHour,
            t.wMinute, t.wDay, t.wMonth, t.wYear);
#else
        const auto now = std::chrono::floor<std::chrono::seconds>(
            std::chrono::system_clock::now());
        const std::string s = std::format("{:%H:%M  %d/%m/%Y}", now);
        std::snprintf(out, size, "%s", s.c_str());
#endif
    }

    // A borderless, transparent, display-front strip window the
    // furniture paints into; its buttons capture input over whatever
    // floats beneath.
    void begin_furniture_window(const char* id, ImVec2 min, ImVec2 max)
    {
        ImGui::SetNextWindowPos(min);
        ImGui::SetNextWindowSize({ max.x - min.x, max.y - min.y });
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(0, 0, 0, 0));
        ImGui::Begin(id, nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
                | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse
                | ImGuiWindowFlags_NoSavedSettings
                | ImGuiWindowFlags_NoScrollbar
                | ImGuiWindowFlags_NoScrollWithMouse
                | ImGuiWindowFlags_NoDocking
                | ImGuiWindowFlags_NoFocusOnAppearing
                | ImGuiWindowFlags_NoBringToFrontOnFocus);
        ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
    }

    void end_furniture_window()
    {
        ImGui::End();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar(2);
    }

}

void Panel::frame(Wm& wm, const std::vector<App*>& apps, const char* shell_mark,
    ImVec2 view_min, ImVec2 view_max)
{
    const float em = ImGui::GetFontSize();
    const float bar_h = height(em);
    const ImVec2 bar_min { view_min.x, view_max.y - bar_h };
    const ImVec2 bar_max = view_max;
    const float center_y = (bar_min.y + bar_max.y) * 0.5f;
    const ImVec2 mouse = ImGui::GetIO().MousePos;

    blocked_.clear();
    blocked_.push_back({ bar_min, bar_max });

    begin_furniture_window("###os-panel", bar_min, bar_max);
    ImDrawList* draw = ImGui::GetWindowDrawList();

    draw->AddRectFilled(bar_min, bar_max, IM_COL32(30, 33, 33, 248));
    draw->AddLine(
        bar_min, { bar_max.x, bar_min.y }, IM_COL32(255, 255, 255, 35), 1.0f);

    const ImU32 fg = IM_COL32(241, 242, 242, 255);
    const ImU32 fg_dim = IM_COL32(190, 193, 193, 235);

    // ---- left: the launcher button ----
    const char* menu_label = "Menu";
    ImGui::PushFont(nullptr, em * 0.9f);
    const ImVec2 label_size = ImGui::CalcTextSize(menu_label);
    ImGui::PopFont();
    const float mark_w = em * 1.05f;
    const ImVec2 menu_min { bar_min.x + em * 0.3f, bar_min.y + em * 0.22f };
    const ImVec2 menu_max { menu_min.x + em * 0.9f + mark_w + label_size.x,
        bar_max.y - em * 0.22f };
    const bool menu_hot = ImGui::IsMouseHoveringRect(menu_min, menu_max, false);
    if (menu_open_ || menu_hot)
        draw->AddRectFilled(menu_min, menu_max,
            IM_COL32(255, 255, 255, menu_open_ ? 34 : 24), em * 0.25f);
    ImGui::PushFont(nullptr, em * 0.95f);
    draw->AddText({ menu_min.x + em * 0.3f,
                      center_y - ImGui::CalcTextSize(shell_mark).y * 0.5f },
        IM_COL32_WHITE, shell_mark);
    ImGui::PopFont();
    ImGui::PushFont(nullptr, em * 0.9f);
    draw->AddText(
        { menu_min.x + em * 0.3f + mark_w, center_y - label_size.y * 0.5f }, fg,
        menu_label);
    ImGui::PopFont();
    ImGui::SetCursorScreenPos(menu_min);
    if (ImGui::InvisibleButton("##panel-menu",
            { menu_max.x - menu_min.x, menu_max.y - menu_min.y }))
        menu_open_ = !menu_open_;

    // ---- middle: one task button per running app ----
    float x = menu_max.x + em * 0.6f;
    draw->AddLine({ x, bar_min.y + em * 0.4f }, { x, bar_max.y - em * 0.4f },
        IM_COL32(255, 255, 255, 20), 1.0f);
    x += em * 0.6f;
    ImGui::PushFont(nullptr, em * 0.88f);
    for (App* app : apps) {
        if (!wm.running(app))
            continue;
        const bool min = wm.minimized(app);
        const bool front = wm.focused() == app;
        const std::string label = std::string(app->mark()) + " " + app->name();
        const ImVec2 ts = ImGui::CalcTextSize(label.c_str());
        const ImVec2 bmin { x, bar_min.y + em * 0.22f };
        const ImVec2 bmax { x + ts.x + em * 1.0f, bar_max.y - em * 0.22f };
        const bool hot = ImGui::IsMouseHoveringRect(bmin, bmax, false);
        const int wash = front ? 36 : (min ? 12 : 20);
        draw->AddRectFilled(bmin, bmax,
            IM_COL32(255, 255, 255, hot ? wash + 10 : wash), em * 0.25f);
        if (front)
            draw->AddLine({ bmin.x + em * 0.25f, bmax.y - 2.0f },
                { bmax.x - em * 0.25f, bmax.y - 2.0f },
                IM_COL32(140, 190, 90, 220), 2.0f);
        draw->AddText({ bmin.x + em * 0.5f, center_y - ts.y * 0.5f },
            min ? fg_dim : fg, label.c_str());
        ImGui::SetCursorScreenPos(bmin);
        const std::string bid = std::string("##task-") + app->id();
        if (ImGui::InvisibleButton(
                bid.c_str(), { bmax.x - bmin.x, bmax.y - bmin.y }))
            wm.toggle(app);
        x = bmax.x + em * 0.35f;
    }
    ImGui::PopFont();

    // ---- right: the clock ----
    {
        char clock[48] {};
        local_clock_line(clock, sizeof clock);
        ImGui::PushFont(nullptr, em * 0.85f);
        const ImVec2 cs = ImGui::CalcTextSize(clock);
        draw->AddText({ bar_max.x - cs.x - em * 0.6f, center_y - cs.y * 0.5f },
            fg, clock);
        ImGui::PopFont();
    }

    end_furniture_window();

    // ---- the launcher menu, floating above the button ----
    if (!menu_open_)
        return;
    const float row_h = em * 1.9f;
    const float pop_w = em * 13.0f;
    const float pop_h
        = static_cast<float>(std::max<std::size_t>(apps.size(), 1)) * row_h
        + em * 0.6f;
    const ImVec2 pop_min { bar_min.x + em * 0.3f,
        bar_min.y - pop_h - em * 0.35f };
    const ImVec2 pop_max { pop_min.x + pop_w, bar_min.y - em * 0.35f };
    blocked_.push_back({ pop_min, pop_max });

    // A click that lands nowhere on the panel's territory closes the
    // menu before it can start a drag underneath it.
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)
        && !ImGui::IsMouseHoveringRect(pop_min, pop_max, false)
        && !ImGui::IsMouseHoveringRect(menu_min, menu_max, false)) {
        menu_open_ = false;
        return;
    }

    begin_furniture_window("###os-panel-popup",
        { pop_min.x - em, pop_min.y - em }, { pop_max.x + em, bar_min.y });
    ImDrawList* pop = ImGui::GetWindowDrawList();

    for (int layer = 5; layer >= 1; --layer) {
        const float spread = static_cast<float>(layer) * 0.14f * em;
        pop->AddRectFilled({ pop_min.x - spread, pop_min.y - spread * 0.3f },
            { pop_max.x + spread, pop_max.y + spread },
            IM_COL32(0, 0, 0, std::max(1, 40 / (layer * 2))),
            em * 0.35f + spread);
    }
    pop->AddRectFilled(pop_min, pop_max, IM_COL32(38, 41, 41, 250), em * 0.35f);
    pop->AddRect(
        pop_min, pop_max, IM_COL32(255, 255, 255, 40), em * 0.35f, 0, 1.0f);

    float y = pop_min.y + em * 0.3f;
    for (App* app : apps) {
        const ImVec2 rmin { pop_min.x + em * 0.3f, y };
        const ImVec2 rmax { pop_max.x - em * 0.3f, y + row_h };
        const bool hot = ImGui::IsMouseHoveringRect(rmin, rmax, false);
        if (hot)
            pop->AddRectFilled(
                rmin, rmax, IM_COL32(255, 255, 255, 26), em * 0.25f);
        ImGui::PushFont(nullptr, em * 1.05f);
        const ImVec2 ms = ImGui::CalcTextSize(app->mark());
        pop->AddText(
            { rmin.x + em * 0.35f, (rmin.y + rmax.y) * 0.5f - ms.y * 0.5f },
            IM_COL32_WHITE, app->mark());
        ImGui::PopFont();
        ImGui::PushFont(nullptr, em * 0.92f);
        const ImVec2 ns = ImGui::CalcTextSize(app->name());
        pop->AddText(
            { rmin.x + em * 1.9f, (rmin.y + rmax.y) * 0.5f - ns.y * 0.5f }, fg,
            app->name());
        ImGui::PopFont();
        ImGui::SetCursorScreenPos(rmin);
        const std::string bid = std::string("##launch-") + app->id();
        if (ImGui::InvisibleButton(
                bid.c_str(), { rmax.x - rmin.x, rmax.y - rmin.y })) {
            wm.launch(app);
            menu_open_ = false;
        }
        y += row_h;
    }

    end_furniture_window();
}

}
