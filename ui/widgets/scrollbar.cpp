#include "ui/widgets/scrollbar.hpp"

#include <algorithm>

#include <imgui.h>
#include <imgui_internal.h>

#include "ui/widgets/design.hpp"

namespace izan::ui {

void kit_scrollbars_stock_hide()
{
    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0, 0, 0, 0);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0, 0, 0, 0);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0, 0, 0, 0);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0, 0, 0, 0);
}

namespace {

    void skin_axis(ImGuiWindow* window, ImGuiAxis axis, ImVec4 ink)
    {
        ImGuiContext& g = *ImGui::GetCurrentContext();
        const ImRect bb = ImGui::GetWindowScrollbarRect(window, axis);
        const bool vertical = axis == ImGuiAxis_Y;
        const float gutter = vertical ? bb.GetWidth() : bb.GetHeight();
        float track_len = vertical ? bb.GetHeight() : bb.GetWidth();
        if (gutter <= 2.0f || track_len <= 8.0f)
            return;

        const float view = vertical ? window->InnerRect.GetHeight()
                                    : window->InnerRect.GetWidth();
        const float max_scroll
            = vertical ? window->ScrollMax.y : window->ScrollMax.x;
        if (view <= 0.0f || max_scroll <= 0.0f)
            return;
        const float scroll = vertical ? window->Scroll.y : window->Scroll.x;

        // Pills keep a breath of air off the track's ends.
        const float pad = std::min(3.0f, track_len * 0.05f);
        track_len -= pad * 2.0f;

        float grab_len = std::max(track_len * view / (view + max_scroll),
            ImGui::GetStyle().GrabMinSize);
        grab_len = std::min(grab_len, track_len);
        const float t = std::clamp(scroll / max_scroll, 0.0f, 1.0f);
        const float grab_pos = pad + (track_len - grab_len) * t;

        const ImGuiID id = ImGui::GetWindowScrollbarID(window, axis);
        const bool held = g.ActiveId == id;
        const bool hovered = held || g.HoveredId == id;

        // Mint manners: slim at rest, wider under the cursor, always
        // centered in the gutter the stock bar reserved.
        const float rest_w = std::clamp(gutter * 0.34f, 4.0f, gutter);
        const float wide_w = std::clamp(
            gutter * 0.62f, rest_w, std::max(rest_w, gutter - 2.0f));
        const float w = hovered ? wide_w : rest_w;
        const float inset = (gutter - w) * 0.5f;

        ImRect pill;
        if (vertical)
            pill = ImRect(bb.Min.x + inset, bb.Min.y + grab_pos,
                bb.Max.x - inset, bb.Min.y + grab_pos + grab_len);
        else
            pill = ImRect(bb.Min.x + grab_pos, bb.Min.y + inset,
                bb.Min.x + grab_pos + grab_len, bb.Max.y - inset);

        ink.w = held ? 0.55f : hovered ? 0.42f : 0.26f;

        ImDrawList* draw = window->DrawList;
        draw->PushClipRect(bb.Min, bb.Max, false);
        if (hovered) {
            // The track only whispers while the bar is in hand.
            ImVec4 bed = ink;
            bed.w = 0.06f;
            kit_round_fill(draw, bb.Min, bb.Max, gutter * 0.5f,
                ImGui::ColorConvertFloat4ToU32(bed));
        }
        kit_round_fill(draw, pill.Min, pill.Max, w * 0.5f,
            ImGui::ColorConvertFloat4ToU32(ink));
        draw->PopClipRect();
    }

}

void kit_skin_scrollbars(ImVec4 ink)
{
    ImGuiContext& g = *ImGui::GetCurrentContext();
    for (ImGuiWindow* window : g.Windows) {
        if (!window->Active || window->Hidden)
            continue;
        if (window->ScrollbarY)
            skin_axis(window, ImGuiAxis_Y, ink);
        if (window->ScrollbarX)
            skin_axis(window, ImGuiAxis_X, ink);
    }
}

}
