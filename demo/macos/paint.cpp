#include "paint.hpp"

#include <algorithm>
#include <cmath>

namespace macos {

Layout Layout::fit(ImVec2 pos, ImVec2 size)
{
    Layout out;
    out.view_min = pos;
    out.view_max = add(pos, size);
    out.scale = std::min(size.x / kCanvasWidth, size.y / kCanvasHeight);
    out.scale = std::max(out.scale, 0.55f);
    out.origin = { pos.x + (size.x - kCanvasWidth * out.scale) * 0.5f,
        pos.y + (size.y - kCanvasHeight * out.scale) * 0.5f };
    return out;
}

ImVec2 Layout::point(float x, float y) const
{
    return { origin.x + x * scale, origin.y + y * scale };
}

ImVec2 Layout::span(float x, float y) const
{
    return { x * scale, y * scale };
}

float Layout::px(float value) const
{
    return value * scale;
}

ImU32 rgba(int red, int green, int blue, int alpha)
{
    return IM_COL32(red, green, blue, alpha);
}

ImU32 mix_color(ImU32 a, ImU32 b, float amount)
{
    amount = std::clamp(amount, 0.0f, 1.0f);
    const auto channel = [amount](ImU32 x, ImU32 y, int shift) {
        const float first = static_cast<float>((x >> shift) & 0xff);
        const float second = static_cast<float>((y >> shift) & 0xff);
        return static_cast<ImU32>(first + (second - first) * amount) << shift;
    };
    return channel(a, b, IM_COL32_R_SHIFT) | channel(a, b, IM_COL32_G_SHIFT)
        | channel(a, b, IM_COL32_B_SHIFT) | channel(a, b, IM_COL32_A_SHIFT);
}

ImVec2 add(ImVec2 a, ImVec2 b)
{
    return { a.x + b.x, a.y + b.y };
}

ImVec2 sub(ImVec2 a, ImVec2 b)
{
    return { a.x - b.x, a.y - b.y };
}

bool contains(ImVec2 point, ImVec2 min, ImVec2 max)
{
    return point.x >= min.x && point.y >= min.y && point.x <= max.x
        && point.y <= max.y;
}

void text(ImDrawList* draw, ImVec2 position, ImU32 color,
    std::string_view value, float size)
{
    draw->AddText(ImGui::GetFont(), size, position, color, value.data(),
        value.data() + value.size());
}

void centered_text(ImDrawList* draw, ImVec2 center, ImU32 color,
    std::string_view value, float size)
{
    const ImVec2 dimensions = ImGui::GetFont()->CalcTextSizeA(
        size, 10000.0f, 0.0f, value.data(), value.data() + value.size());
    text(draw,
        { center.x - dimensions.x * 0.5f, center.y - dimensions.y * 0.5f },
        color, value, size);
}

void shadowed_text(
    ImDrawList* draw, ImVec2 position, std::string_view value, float size)
{
    text(draw, add(position, { 0.0f, 1.2f }), rgba(0, 0, 0, 135), value, size);
    text(draw, position, rgba(255, 255, 255), value, size);
}

void soft_shadow(ImDrawList* draw, ImVec2 min, ImVec2 max, float rounding,
    float scale, int strength)
{
    for (int layer = 8; layer >= 1; --layer) {
        const float spread = static_cast<float>(layer) * 2.0f * scale;
        const int alpha = std::max(1, strength / (layer * 2));
        draw->AddRectFilled({ min.x - spread, min.y - spread * 0.25f },
            { max.x + spread, max.y + spread }, rgba(0, 0, 0, alpha),
            rounding + spread);
    }
}

bool hit_rect(const char* id, ImVec2 min, ImVec2 max)
{
    ImGui::SetCursorScreenPos(min);
    ImGui::InvisibleButton(id, sub(max, min));
    return ImGui::IsItemClicked(ImGuiMouseButton_Left);
}

void draw_wifi(ImDrawList* draw, ImVec2 center, float scale, ImU32 color)
{
    for (int ring = 0; ring < 3; ++ring) {
        const float radius = (4.0f + ring * 4.2f) * scale;
        draw->PathArcTo(center, radius, 3.85f, 5.57f, 18);
        draw->PathStroke(color, 0, 1.45f * scale);
    }
    draw->AddCircleFilled(
        { center.x, center.y + 1.0f * scale }, 1.65f * scale, color);
}

void draw_control_glyph(
    ImDrawList* draw, ImVec2 center, float scale, ImU32 color)
{
    const float width = 15.0f * scale;
    for (int row = -1; row <= 1; row += 2) {
        const float y = center.y + row * 3.7f * scale;
        draw->AddLine({ center.x - width * 0.5f, y },
            { center.x + width * 0.5f, y }, color, 1.5f * scale);
        draw->AddCircleFilled(
            { center.x + row * 3.5f * scale, y }, 2.4f * scale, color);
    }
}

void draw_battery(ImDrawList* draw, ImVec2 min, float scale, ImU32 color)
{
    const ImVec2 max { min.x + 22.0f * scale, min.y + 10.0f * scale };
    draw->AddRect(min, max, color, 2.5f * scale, 0, 1.2f * scale);
    draw->AddRectFilled({ max.x + 1.5f * scale, min.y + 3.0f * scale },
        { max.x + 3.3f * scale, max.y - 3.0f * scale }, color, 1.0f * scale);
    draw->AddRectFilled({ min.x + 2.1f * scale, min.y + 2.1f * scale },
        { min.x + 17.0f * scale, max.y - 2.1f * scale }, color, 1.2f * scale);
}

void draw_apple(ImDrawList* draw, ImVec2 center, float scale, ImU32 color)
{
    draw->AddCircleFilled(
        { center.x - 3.0f * scale, center.y + 1.0f * scale }, 5.1f * scale,
        color);
    draw->AddCircleFilled(
        { center.x + 3.0f * scale, center.y + 1.0f * scale }, 5.1f * scale,
        color);
    draw->AddCircleFilled(
        { center.x, center.y + 5.0f * scale }, 5.0f * scale, color);
    draw->AddCircleFilled({ center.x + 6.1f * scale, center.y - 1.0f * scale },
        2.8f * scale, rgba(25, 30, 49, 220));
    draw->AddEllipseFilled({ center.x + 2.4f * scale, center.y - 7.0f * scale },
        { 2.3f * scale, 4.1f * scale }, color, -0.62f);
}

void draw_magnifier(ImDrawList* draw, ImVec2 center, float scale, ImU32 color)
{
    draw->AddCircle(center, 5.4f * scale, color, 24, 1.5f * scale);
    draw->AddLine({ center.x + 4.0f * scale, center.y + 4.0f * scale },
        { center.x + 9.0f * scale, center.y + 9.0f * scale }, color,
        1.8f * scale);
}

void draw_folder(ImDrawList* draw, ImVec2 center, float scale, bool selected)
{
    const ImU32 back = selected ? rgba(69, 154, 249) : rgba(92, 178, 252);
    const ImU32 front = selected ? rgba(46, 137, 239) : rgba(61, 155, 241);
    draw->AddRectFilled({ center.x - 21.0f * scale, center.y - 12.0f * scale },
        { center.x + 18.0f * scale, center.y + 15.0f * scale }, back,
        4.0f * scale);
    draw->AddRectFilled({ center.x - 18.0f * scale, center.y - 17.0f * scale },
        { center.x - 2.0f * scale, center.y - 8.0f * scale }, back,
        3.0f * scale);
    draw->AddRectFilled({ center.x - 22.0f * scale, center.y - 8.0f * scale },
        { center.x + 22.0f * scale, center.y + 17.0f * scale }, front,
        4.0f * scale);
    draw->AddLine({ center.x - 18.0f * scale, center.y - 5.0f * scale },
        { center.x + 18.0f * scale, center.y - 5.0f * scale },
        rgba(130, 205, 255), 1.0f * scale);
}

void draw_document(ImDrawList* draw, ImVec2 center, float scale, ImU32 tint)
{
    const ImVec2 min { center.x - 17.0f * scale, center.y - 21.0f * scale };
    const ImVec2 max { center.x + 17.0f * scale, center.y + 22.0f * scale };
    draw->AddRectFilled(min, max, rgba(252, 252, 253), 4.0f * scale);
    draw->AddRect(min, max, rgba(190, 194, 202), 4.0f * scale, 0, 0.8f * scale);
    draw->AddRectFilled({ min.x, min.y }, { max.x, min.y + 10.0f * scale },
        tint, 4.0f * scale, ImDrawFlags_RoundCornersTop);
    for (int line = 0; line < 3; ++line) {
        const float y = center.y - 2.0f * scale + line * 7.0f * scale;
        draw->AddLine({ min.x + 6.0f * scale, y }, { max.x - 6.0f * scale, y },
            rgba(188, 192, 199), scale);
    }
}

void draw_dock_icon(ImDrawList* draw, int index, ImVec2 center, float size)
{
    const float half = size * 0.5f;
    const float rounding = size * 0.22f;
    const ImVec2 min { center.x - half, center.y - half };
    const ImVec2 max { center.x + half, center.y + half };

    switch (index) {
    case 0: { // Finder
        draw->AddRectFilled(min, max, rgba(75, 174, 246), rounding);
        draw->AddRectFilled({ center.x, min.y }, max, rgba(225, 242, 255),
            rounding, ImDrawFlags_RoundCornersRight);
        draw->AddLine({ center.x, min.y + size * 0.08f },
            { center.x - size * 0.06f, center.y + size * 0.08f },
            rgba(20, 69, 119), size * 0.025f);
        draw->AddCircleFilled(
            { center.x - size * 0.17f, center.y - size * 0.08f }, size * 0.025f,
            rgba(20, 69, 119));
        draw->AddCircleFilled(
            { center.x + size * 0.16f, center.y - size * 0.08f }, size * 0.025f,
            rgba(20, 69, 119));
        draw->PathLineTo({ center.x - size * 0.22f, center.y + size * 0.16f });
        draw->PathBezierCubicCurveTo(
            { center.x - size * 0.08f, center.y + size * 0.28f },
            { center.x + size * 0.13f, center.y + size * 0.28f },
            { center.x + size * 0.23f, center.y + size * 0.12f });
        draw->PathStroke(rgba(20, 69, 119), 0, size * 0.025f);
        break;
    }
    case 1: { // Launchpad
        draw->AddRectFilledMultiColor(min, max, rgba(97, 78, 235),
            rgba(228, 91, 177), rgba(61, 195, 231), rgba(78, 96, 235));
        for (int y = -1; y <= 1; ++y) {
            for (int x = -1; x <= 1; ++x) {
                draw->AddCircleFilled({ center.x + x * size * 0.19f,
                                          center.y + y * size * 0.19f },
                    size * 0.055f, rgba(255, 255, 255, 235));
            }
        }
        break;
    }
    case 2: { // Safari
        draw->AddRectFilled(min, max, rgba(247, 249, 251), rounding);
        draw->AddCircleFilled(center, size * 0.37f, rgba(52, 173, 239));
        draw->AddCircle(
            center, size * 0.32f, rgba(231, 248, 255), 40, size * 0.018f);
        draw->AddTriangleFilled(
            { center.x + size * 0.08f, center.y - size * 0.25f },
            { center.x - size * 0.04f, center.y + size * 0.07f },
            { center.x - size * 0.11f, center.y + size * 0.24f },
            rgba(244, 62, 81));
        draw->AddTriangleFilled(
            { center.x - size * 0.08f, center.y + size * 0.25f },
            { center.x + size * 0.04f, center.y - size * 0.07f },
            { center.x + size * 0.11f, center.y - size * 0.24f },
            rgba(248, 250, 251));
        break;
    }
    case 3: { // Messages
        draw->AddRectFilled(min, max, rgba(61, 207, 80), rounding);
        draw->AddEllipseFilled({ center.x, center.y - size * 0.03f },
            { size * 0.31f, size * 0.25f }, rgba(255, 255, 255));
        draw->AddTriangleFilled(
            { center.x - size * 0.18f, center.y + size * 0.13f },
            { center.x - size * 0.28f, center.y + size * 0.29f },
            { center.x - size * 0.03f, center.y + size * 0.19f },
            rgba(255, 255, 255));
        break;
    }
    case 4: { // Mail
        draw->AddRectFilled(min, max, rgba(46, 145, 242), rounding);
        const ImVec2 envelope_min { center.x - size * 0.32f,
            center.y - size * 0.21f };
        const ImVec2 envelope_max { center.x + size * 0.32f,
            center.y + size * 0.22f };
        draw->AddRectFilled(
            envelope_min, envelope_max, rgba(248, 252, 255), size * 0.05f);
        draw->AddTriangleFilled(envelope_min,
            { center.x, center.y + size * 0.05f },
            { envelope_max.x, envelope_min.y }, rgba(210, 234, 252));
        break;
    }
    case 5: { // Photos
        draw->AddRectFilled(min, max, rgba(250, 250, 250), rounding);
        static constexpr ImU32 petals[]
            = { IM_COL32(244, 70, 80, 235), IM_COL32(250, 145, 47, 235),
                  IM_COL32(247, 205, 54, 235), IM_COL32(69, 192, 98, 235),
                  IM_COL32(55, 172, 224, 235), IM_COL32(77, 103, 224, 235),
                  IM_COL32(146, 79, 207, 235), IM_COL32(226, 76, 155, 235) };
        for (int petal = 0; petal < 8; ++petal) {
            const float angle = static_cast<float>(petal) * 0.785398f;
            draw->AddCircleFilled(
                { center.x + std::cos(angle) * size * 0.17f,
                    center.y + std::sin(angle) * size * 0.17f },
                size * 0.14f, petals[petal]);
        }
        draw->AddCircleFilled(center, size * 0.08f, rgba(255, 255, 255));
        break;
    }
    case 6: { // Calendar
        draw->AddRectFilled(min, max, rgba(252, 252, 252), rounding);
        draw->AddRectFilled(min, { max.x, min.y + size * 0.26f },
            rgba(245, 73, 73), rounding, ImDrawFlags_RoundCornersTop);
        centered_text(draw, { center.x, center.y + size * 0.10f },
            rgba(35, 35, 38), "20", size * 0.34f);
        break;
    }
    case 7: { // Notes
        draw->AddRectFilled(min, max, rgba(255, 253, 239), rounding);
        draw->AddRectFilled(min, { max.x, min.y + size * 0.22f },
            rgba(255, 211, 52), rounding, ImDrawFlags_RoundCornersTop);
        for (int line = 0; line < 4; ++line) {
            const float y = center.y - size * 0.03f + line * size * 0.13f;
            draw->AddLine({ min.x + size * 0.13f, y },
                { max.x - size * 0.13f, y }, rgba(201, 198, 187),
                size * 0.014f);
        }
        break;
    }
    case 8: { // Settings
        draw->AddRectFilled(min, max, rgba(156, 161, 169), rounding);
        draw->AddCircleFilled(center, size * 0.29f, rgba(235, 238, 241));
        for (int tooth = 0; tooth < 8; ++tooth) {
            const float angle = static_cast<float>(tooth) * 0.785398f;
            const ImVec2 a { center.x + std::cos(angle) * size * 0.30f,
                center.y + std::sin(angle) * size * 0.30f };
            const ImVec2 b { center.x + std::cos(angle) * size * 0.40f,
                center.y + std::sin(angle) * size * 0.40f };
            draw->AddLine(a, b, rgba(235, 238, 241), size * 0.09f);
        }
        draw->AddCircleFilled(center, size * 0.12f, rgba(119, 125, 134));
        break;
    }
    default: { // Trash
        draw->AddRectFilled(
            { center.x - size * 0.25f, center.y - size * 0.27f },
            { center.x + size * 0.25f, center.y + size * 0.35f },
            rgba(224, 238, 242), size * 0.06f);
        draw->AddRectFilled(
            { center.x - size * 0.30f, center.y - size * 0.33f },
            { center.x + size * 0.30f, center.y - size * 0.25f },
            rgba(245, 249, 250), size * 0.03f);
        for (int line = -1; line <= 1; ++line) {
            const float x = center.x + line * size * 0.12f;
            draw->AddLine({ x, center.y - size * 0.16f },
                { x, center.y + size * 0.25f }, rgba(144, 177, 185),
                size * 0.018f);
        }
        break;
    }
    }
    draw->AddRect(
        min, max, rgba(255, 255, 255, 85), rounding, 0, size * 0.015f);
}

}
