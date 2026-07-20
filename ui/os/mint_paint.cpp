#include "ui/os/mint_paint.hpp"

#include <algorithm>

#include "ui/shell/fonts.hpp"

namespace izan::os::mint {

float mint_scale()
{
    return ImGui::GetFontSize() / (ui::kDefaultFontSize / ui::kFontDesignScale);
}

void mint_logo(ImDrawList* draw, ImVec2 center, float radius)
{
    draw->AddCircleFilled(center, radius, IM_COL32(109, 190, 69, 255));
    draw->AddCircle(center, radius, IM_COL32(255, 255, 255, 70), 32,
        std::max(1.0f, radius * 0.045f));
    const float stroke = std::max(1.2f, radius * 0.11f);
    const ImU32 white = IM_COL32(255, 255, 255, 255);
    draw->AddLine({ center.x - radius * 0.44f, center.y - radius * 0.45f },
        { center.x - radius * 0.44f, center.y + radius * 0.38f }, white,
        stroke);
    draw->AddLine({ center.x - radius * 0.44f, center.y + radius * 0.38f },
        { center.x + radius * 0.42f, center.y + radius * 0.38f }, white,
        stroke);
    draw->AddLine({ center.x - radius * 0.05f, center.y - radius * 0.02f },
        { center.x - radius * 0.05f, center.y + radius * 0.38f }, white,
        stroke);
    draw->AddLine({ center.x + radius * 0.18f, center.y - radius * 0.02f },
        { center.x + radius * 0.18f, center.y + radius * 0.38f }, white,
        stroke);
    draw->AddCircle({ center.x + radius * 0.065f, center.y - radius * 0.02f },
        radius * 0.115f, white, 16, stroke * 0.65f);
}

void folder_icon(ImDrawList* draw, ImVec2 center, float scale, ImU32 color)
{
    draw->AddRectFilled({ center.x - 23.0f * scale, center.y - 12.0f * scale },
        { center.x + 21.0f * scale, center.y + 17.0f * scale }, color,
        4.0f * scale);
    draw->AddRectFilled({ center.x - 20.0f * scale, center.y - 18.0f * scale },
        { center.x - 3.0f * scale, center.y - 7.0f * scale }, color,
        3.0f * scale);
    draw->AddRectFilled({ center.x - 24.0f * scale, center.y - 8.0f * scale },
        { center.x + 24.0f * scale, center.y + 18.0f * scale },
        IM_COL32(135, 201, 99, 255), 4.0f * scale);
    draw->AddLine({ center.x - 19.0f * scale, center.y - 5.0f * scale },
        { center.x + 19.0f * scale, center.y - 5.0f * scale },
        IM_COL32(190, 229, 169, 255), scale);
}

void document_icon(ImDrawList* draw, ImVec2 center, float scale, ImU32 accent)
{
    const ImVec2 min { center.x - 17.0f * scale, center.y - 22.0f * scale };
    const ImVec2 max { center.x + 17.0f * scale, center.y + 22.0f * scale };
    draw->AddRectFilled(min, max, IM_COL32(250, 250, 250, 255), 3.0f * scale);
    draw->AddRect(
        min, max, IM_COL32(145, 148, 151, 255), 3.0f * scale, 0, 0.8f * scale);
    draw->AddRectFilled(min, { max.x, min.y + 7.0f * scale }, accent,
        3.0f * scale, ImDrawFlags_RoundCornersTop);
    for (int line = 0; line < 3; ++line) {
        const float y = center.y - 4.0f * scale + line * 7.0f * scale;
        draw->AddLine({ min.x + 5.0f * scale, y }, { max.x - 5.0f * scale, y },
            IM_COL32(168, 171, 174, 255), scale);
    }
}

void wifi_icon(ImDrawList* draw, ImVec2 center, float scale, ImU32 color)
{
    for (int ring = 0; ring < 3; ++ring) {
        draw->PathArcTo(center, (4.0f + ring * 4.0f) * scale, 3.85f, 5.57f, 16);
        draw->PathStroke(color, 0, 1.5f * scale);
    }
    draw->AddCircleFilled(
        { center.x, center.y + 1.0f * scale }, 1.7f * scale, color);
}

void control_icon(ImDrawList* draw, ImVec2 center, int control, bool restore,
    ImU32 color, float scale)
{
    const float stroke = std::max(1.0f, 1.15f * scale);
    if (control == 0) {
        draw->AddLine({ center.x - 5.0f * scale, center.y },
            { center.x + 5.0f * scale, center.y }, color, stroke);
    } else if (control == 1 && restore) {
        draw->AddRect({ center.x - 5.0f * scale, center.y - 2.0f * scale },
            { center.x + 3.0f * scale, center.y + 5.0f * scale }, color, 0.0f,
            0, stroke);
        draw->AddRect({ center.x - 2.0f * scale, center.y - 5.0f * scale },
            { center.x + 6.0f * scale, center.y + 2.0f * scale }, color, 0.0f,
            0, stroke);
    } else if (control == 1) {
        draw->AddRect({ center.x - 5.0f * scale, center.y - 4.0f * scale },
            { center.x + 5.0f * scale, center.y + 4.0f * scale }, color, 0.0f,
            0, stroke);
    } else {
        draw->AddLine({ center.x - 4.0f * scale, center.y - 4.0f * scale },
            { center.x + 4.0f * scale, center.y + 4.0f * scale }, color,
            stroke);
        draw->AddLine({ center.x + 4.0f * scale, center.y - 4.0f * scale },
            { center.x - 4.0f * scale, center.y + 4.0f * scale }, color,
            stroke);
    }
}

void window_shadow(
    ImDrawList* draw, ImVec2 min, ImVec2 max, float rounding, float scale)
{
    // The kit menu's depth profile: a dozen faint rings fading fast —
    // small, blurred, light. A heavy halo reads as smear the moment
    // the window moves.
    static constexpr float kAlpha[12]
        = { 0.020f, 0.017f, 0.014f, 0.011f, 0.0085f, 0.0065f, 0.0050f, 0.0038f,
              0.0028f, 0.0020f, 0.0014f, 0.0010f };
    const float drop = 1.5f * scale;
    for (int index = 11; index >= 0; --index) {
        const float spread
            = (2.0f + static_cast<float>(index + 1) * 1.75f) * scale;
        draw->AddRectFilled({ min.x - spread, min.y - spread + drop },
            { max.x + spread, max.y + spread + drop },
            IM_COL32(0, 0, 0, static_cast<int>(kAlpha[index] * 255.0f + 0.5f)),
            rounding + spread);
    }
}

void text_at(ImDrawList* draw, ImVec2 pos, ImU32 color, std::string_view value,
    float size)
{
    draw->AddText(ImGui::GetFont(), size, pos, color, value.data(),
        value.data() + value.size());
}

void text_centered(ImDrawList* draw, ImVec2 center, ImU32 color,
    std::string_view value, float size)
{
    const ImVec2 extent = ImGui::GetFont()->CalcTextSizeA(
        size, 10000.0f, 0.0f, value.data(), value.data() + value.size());
    text_at(draw, { center.x - extent.x * 0.5f, center.y - extent.y * 0.5f },
        color, value, size);
}

void text_vcentered(ImDrawList* draw, float x, float center_y, ImU32 color,
    std::string_view value, float size)
{
    const ImVec2 extent = ImGui::GetFont()->CalcTextSizeA(
        size, 10000.0f, 0.0f, value.data(), value.data() + value.size());
    text_at(draw, { x, center_y - extent.y * 0.5f }, color, value, size);
}

}
