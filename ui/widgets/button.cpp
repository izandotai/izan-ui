#include "ui/widgets/button.hpp"

#include <imgui_internal.h>

#include "ui/render/sdf_rect.hpp"
#include "ui/widgets/design.hpp"

namespace izan::ui {

namespace {

    // Auto-width buttons get a floor: a capsule around two characters
    // reads as a squat blob, not a control. Explicit widths are the
    // caller's own business.
    float resolved_width(const char* label, float width)
    {
        if (width != 0.0f)
            return width;
        // Capsule ends eat into the text visually; pill mode grants
        // the label more air than the theme's frame padding does.
        const float pad_x = design().button_pill
            ? ImGui::GetFontSize() * design().button_pad_x_em
            : ImGui::GetStyle().FramePadding.x;
        const float natural = ImGui::CalcTextSize(label).x + pad_x * 2.0f;
        const float floor_w = ImGui::GetFontSize() * design().button_min_em;
        return natural < floor_w ? floor_w : natural;
    }

    // The shape token: pill mode swaps the theme rounding for full
    // capsule ends. Returns the rounding in force for the finish pass.
    float push_button_shape()
    {
        if (!design().button_pill)
            return ImGui::GetStyle().FrameRounding;
        const float r = ImGui::GetFrameHeight() * 0.5f;
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, r);
        return r;
    }

    void pop_button_shape()
    {
        if (design().button_pill)
            ImGui::PopStyleVar();
    }

    // The finish token, tuned against macOS: a soft light falling from
    // the crown (layered caps stand in for a gradient, corners kept
    // round), a shallow floor shade, a hairline crown highlight and a
    // quiet darker rim to seat the button in the surface.
    void paint_gloss(float rounding, bool with_rim = true)
    {
        const float g = design().button_gloss;
        if (g <= 0.0f)
            return;
        const ImVec2 min = ImGui::GetItemRectMin();
        const ImVec2 max = ImGui::GetItemRectMax();
        const float h = max.y - min.y;
        ImDrawList* draw = ImGui::GetWindowDrawList();

        static constexpr float kDepth[] = { 0.18f, 0.34f, 0.50f, 0.66f };
        static constexpr float kAlpha[] = { 16.0f, 10.0f, 7.0f, 5.0f };
        for (int i = 0; i < 4; ++i)
            draw->AddRectFilled(ImVec2(min.x + 1.0f, min.y + 1.0f),
                ImVec2(max.x - 1.0f, min.y + h * kDepth[i]),
                IM_COL32(255, 255, 255, int(kAlpha[i] * g)), rounding - 1.0f,
                ImDrawFlags_RoundCornersTop);

        draw->AddRectFilled(ImVec2(min.x + 1.0f, max.y - h * 0.20f),
            ImVec2(max.x - 1.0f, max.y - 1.0f),
            IM_COL32(0, 0, 0, int(16.0f * g)), rounding - 1.0f,
            ImDrawFlags_RoundCornersBottom);

        draw->AddLine(ImVec2(min.x + rounding * 0.9f, min.y + 1.0f),
            ImVec2(max.x - rounding * 0.9f, min.y + 1.0f),
            IM_COL32(255, 255, 255, int(56.0f * g)));

        if (with_rim)
            kit_round_border(
                draw, min, max, rounding, ImVec4(0, 0, 0, 48.0f / 255.0f * g));
    }

    // Every kit button rides the analytic base: an SDF capsule for
    // the fill and the seating rim (the largest radii in the kit,
    // where polygon arcs read worst), the gloss layered inside it,
    // the label drawn by hand. GetColorU32 keeps disabled dimming.
    bool sdf_button(const char* label, float width, const ImVec4* fill_color)
    {
        const float w = resolved_width(label, width);
        const float h = ImGui::GetFrameHeight();
        const ImVec2 pos = ImGui::GetCursorScreenPos();
        const bool clicked = ImGui::InvisibleButton(label, ImVec2(w, h));
        const bool hovered = ImGui::IsItemHovered();
        const bool held = hovered && ImGui::IsItemActive();

        ImVec4 base = fill_color != nullptr
            ? *fill_color
            : ImGui::GetStyleColorVec4(held ? ImGuiCol_ButtonActive
                      : hovered             ? ImGuiCol_ButtonHovered
                                            : ImGuiCol_Button);
        if (fill_color != nullptr && held)
            base = kit_blend(base, ImVec4(0, 0, 0, base.w), 0.12f);
        else if (fill_color != nullptr && hovered)
            base = kit_blend(base, ImVec4(1, 1, 1, base.w), 0.12f);

        const float rounding
            = design().button_pill ? h * 0.5f : ImGui::GetStyle().FrameRounding;
        const float g = design().button_gloss;
        ImDrawList* draw = ImGui::GetWindowDrawList();
        render::SdfRect body;
        body.min = pos;
        body.max = { pos.x + w, pos.y + h };
        body.radius[0] = body.radius[1] = body.radius[2] = body.radius[3]
            = rounding;
        body.fill = ImGui::GetColorU32(base);
        if (g > 0.0f) {
            // The seating rim at the kit gauge (2px decree), eased by
            // the language's border alpha.
            body.border = ImGui::GetColorU32(
                ImVec4(0, 0, 0, 48.0f / 255.0f * g * design().border_alpha));
            body.border_px = design().border_px;
        }
        render::sdf_rect(draw, body);
        paint_gloss(rounding, false);

        const ImVec4 ink = fill_color != nullptr
            ? ImVec4(1, 1, 1, 0.96f)
            : ImGui::GetStyleColorVec4(ImGuiCol_Text);
        const char* text_end = ImGui::FindRenderedTextEnd(label);
        const ImVec2 ts = ImGui::CalcTextSize(label, text_end);
        draw->AddText({ pos.x + (w - ts.x) * 0.5f, pos.y + (h - ts.y) * 0.5f },
            ImGui::GetColorU32(ink), label, text_end);
        return clicked;
    }

}

bool kit_primary_button(const char* label, float width)
{
    ImVec4 accent = kit_accent();
    accent.w = 1.0f;
    return sdf_button(label, width, &accent);
}

bool kit_danger_button(const char* label, float width)
{
    const ImVec4 danger = kit_danger();
    return sdf_button(label, width, &danger);
}

bool kit_subtle_button(const char* label, float width)
{
    return sdf_button(label, width, nullptr);
}

float kit_button_width(const char* label, float width)
{
    return resolved_width(label, width);
}

bool kit_add_button(const char* id, float side)
{
    if (side <= 0.0f)
        side = ImGui::GetFrameHeight();
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const bool clicked = ImGui::InvisibleButton(id, ImVec2(side, side));
    const bool hovered = ImGui::IsItemHovered();
    const bool held = ImGui::IsItemActive();
    if (hovered)
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

    ImVec4 accent = kit_accent();
    accent.w = 1.0f;
    const ImVec4 fill = held ? kit_blend(accent, ImVec4(0, 0, 0, 1), 0.12f)
        : hovered            ? kit_blend(accent, ImVec4(1, 1, 1, 1), 0.12f)
                             : accent;
    // Always a rounded square, even in pill mode — a full circle at
    // this size reads as a bead, not a button, and the square sits
    // flush with the field beside it.
    const float rounding = ImGui::GetStyle().FrameRounding;
    ImDrawList* draw = ImGui::GetWindowDrawList();
    draw->AddRectFilled(pos, ImVec2(pos.x + side, pos.y + side),
        ImGui::GetColorU32(fill), rounding);
    paint_gloss(rounding);

    // The cross: two snapped strokes through the true center, arms
    // sized to the square — the glyph the label font never gets right.
    const float cx = kit_snap(pos.x + side * 0.5f);
    const float cy = kit_snap(pos.y + side * 0.5f);
    const float arm = side * 0.21f;
    float t = side * 0.085f;
    if (t < 1.6f)
        t = 1.6f;
    const ImU32 ink = IM_COL32(255, 255, 255, 245);
    draw->AddLine(ImVec2(cx - arm, cy), ImVec2(cx + arm, cy), ink, t);
    draw->AddLine(ImVec2(cx, cy - arm), ImVec2(cx, cy + arm), ink, t);
    return clicked;
}

bool kit_link_button(const char* label)
{
    ImGui::PushStyleColor(ImGuiCol_Text, kit_accent());
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(
        ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,
        ImGui::GetStyleColorVec4(ImGuiCol_FrameBgActive));
    push_button_shape();
    // Link buttons keep the width discipline too — a hover pill around
    // two characters is the same squat blob in a different coat.
    const bool clicked
        = ImGui::Button(label, ImVec2(resolved_width(label, 0.0f), 0.0f));
    pop_button_shape();
    ImGui::PopStyleColor(4);
    return clicked;
}

}
