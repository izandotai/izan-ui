#include "ui/os/dock.hpp"

#include <algorithm>
#include <cmath>
#include <string>

#include <imgui_internal.h>

#include "ui/widgets/design.hpp"

namespace izan::os {

void Dock::frame(
    Wm& wm, const std::vector<App*>& apps, ImVec2 ws_min, ImVec2 ws_max)
{
    if (apps.empty())
        return;
    const float em = ImGui::GetFontSize();
    const bool dark = ui::kit_is_dark();
    const float base = em * 2.4f;
    const float gap = em * 0.5f;
    const float pad = em * 0.55f;
    const float magnify_span = em * 1.0f;
    const int count = static_cast<int>(apps.size());
    const float content = count * base + (count - 1) * gap;

    const float shelf_h = base + pad * 2.0f;
    const ImVec2 shelf_center { (ws_min.x + ws_max.x) * 0.5f,
        ws_max.y - em * 0.5f - shelf_h * 0.5f };
    shelf_min_ = { shelf_center.x - content * 0.5f - pad * 1.6f,
        shelf_center.y - shelf_h * 0.5f };
    shelf_max_ = { shelf_center.x + content * 0.5f + pad * 1.6f,
        shelf_center.y + shelf_h * 0.5f };

    // The dock rides its own window so its buttons capture input
    // cleanly over whatever floats beneath; the window is forced to
    // the display front every frame.
    const float headroom = magnify_span + em * 1.9f; // tooltips too
    ImGui::SetNextWindowPos({ shelf_min_.x - em, shelf_min_.y - headroom });
    ImGui::SetNextWindowSize({ shelf_max_.x - shelf_min_.x + em * 2.0f,
        shelf_max_.y - shelf_min_.y + headroom + em * 0.6f });
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(0, 0, 0, 0));
    ImGui::Begin("###os-dock", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse
            | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar
            | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoDocking
            | ImGuiWindowFlags_NoFocusOnAppearing
            | ImGuiWindowFlags_NoBringToFrontOnFocus);
    ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
    ImDrawList* draw = ImGui::GetWindowDrawList();

    // The shelf: frosted, rimmed, softly grounded.
    const float rounding = em * 0.8f;
    for (int layer = 6; layer >= 1; --layer) {
        const float spread = static_cast<float>(layer) * 0.12f * em;
        draw->AddRectFilled(
            { shelf_min_.x - spread, shelf_min_.y - spread * 0.2f },
            { shelf_max_.x + spread, shelf_max_.y + spread },
            IM_COL32(0, 0, 0, std::max(1, 46 / (layer * 2))),
            rounding + spread);
    }
    draw->AddRectFilled(shelf_min_, shelf_max_,
        dark ? IM_COL32(46, 47, 52, 196) : IM_COL32(240, 243, 248, 186),
        rounding);
    draw->AddRect(shelf_min_, shelf_max_,
        IM_COL32(255, 255, 255, dark ? 60 : 120), rounding, 0, 1.0f);

    const ImVec2 mouse = ImGui::GetIO().MousePos;
    const float baseline = shelf_max_.y - pad;
    for (int i = 0; i < count; ++i) {
        App* app = apps[static_cast<std::size_t>(i)];
        const float cx = shelf_min_.x + pad * 1.6f + base * 0.5f
            + static_cast<float>(i) * (base + gap);

        float magnification = 0.0f;
        if (mouse.y > shelf_min_.y - em * 2.5f
            && mouse.y < shelf_max_.y + em * 0.5f) {
            const float distance = std::abs(mouse.x - cx);
            magnification = std::max(0.0f, 1.0f - distance / (em * 3.4f));
        }
        const float size
            = base + magnify_span * magnification * magnification;
        const ImVec2 center { cx, baseline - size * 0.5f };

        // The tile: a minted rounded square carrying the app's mark.
        const ImVec2 tmin { center.x - size * 0.5f, center.y - size * 0.5f };
        const ImVec2 tmax { center.x + size * 0.5f, center.y + size * 0.5f };
        const float tile_round = size * 0.24f;
        draw->AddRectFilled(tmin, tmax,
            dark ? IM_COL32(72, 74, 82, 235) : IM_COL32(255, 255, 255, 240),
            tile_round);
        draw->AddRect(tmin, tmax, IM_COL32(255, 255, 255, dark ? 46 : 160),
            tile_round, 0, 1.0f);
        ImGui::PushFont(nullptr, size * 0.58f);
        const ImVec2 ms = ImGui::CalcTextSize(app->mark());
        draw->AddText({ center.x - ms.x * 0.5f, center.y - ms.y * 0.5f },
            IM_COL32_WHITE, app->mark());
        ImGui::PopFont();

        if (wm.running(app)) {
            draw->AddCircleFilled({ cx, shelf_max_.y - em * 0.18f },
                em * 0.09f,
                dark ? IM_COL32(235, 236, 240, 255)
                     : IM_COL32(52, 55, 62, 255));
        }

        const bool hovered = ImGui::IsMouseHoveringRect(
            { cx - (base + gap) * 0.5f, shelf_min_.y - em * 2.0f },
            { cx + (base + gap) * 0.5f, shelf_max_.y });
        if (hovered) {
            ImGui::PushFont(nullptr, em * 0.85f);
            const ImVec2 ls = ImGui::CalcTextSize(app->name());
            const ImVec2 tip_min { center.x - ls.x * 0.5f - em * 0.45f,
                tmin.y - em * 1.75f };
            const ImVec2 tip_max { center.x + ls.x * 0.5f + em * 0.45f,
                tip_min.y + em * 1.3f };
            draw->AddRectFilled(tip_min, tip_max,
                dark ? IM_COL32(56, 57, 62, 240) : IM_COL32(246, 248, 251, 240),
                em * 0.3f);
            draw->AddText({ tip_min.x + em * 0.45f,
                              tip_min.y + (em * 1.3f - ls.y) * 0.5f },
                dark ? IM_COL32(244, 244, 246, 255) : IM_COL32(40, 42, 46, 255),
                app->name());
            ImGui::PopFont();
        }

        ImGui::SetCursorScreenPos(
            { cx - (base + gap) * 0.5f, shelf_min_.y - em * 1.2f });
        const std::string bid = std::string("##dock-") + app->id();
        if (ImGui::InvisibleButton(
                bid.c_str(), { base + gap, shelf_h + em * 1.2f }))
            wm.launch(app);
    }

    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}

}
