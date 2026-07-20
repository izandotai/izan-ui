#include "macos_desktop.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <string_view>

namespace {

constexpr float kCanvasWidth = 1440.0f;
constexpr float kCanvasHeight = 900.0f;

ImU32 rgba(int red, int green, int blue, int alpha = 255)
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
    float scale, int strength = 70)
{
    for (int layer = 8; layer >= 1; --layer) {
        const float spread = static_cast<float>(layer) * 2.0f * scale;
        const int alpha = std::max(1, strength / (layer * 2));
        draw->AddRectFilled({ min.x - spread, min.y - spread * 0.25f },
            { max.x + spread, max.y + spread }, rgba(0, 0, 0, alpha),
            rounding + spread);
    }
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
    draw->AddCircleFilled({ center.x - 3.0f * scale, center.y + 1.0f * scale },
        5.1f * scale, color);
    draw->AddCircleFilled({ center.x + 3.0f * scale, center.y + 1.0f * scale },
        5.1f * scale, color);
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

void draw_folder(
    ImDrawList* draw, ImVec2 center, float scale, bool selected = false)
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

} // namespace

ImVec2 MacOSDesktop::Layout::point(float x, float y) const
{
    return { origin.x + x * scale, origin.y + y * scale };
}

ImVec2 MacOSDesktop::Layout::size(float x, float y) const
{
    return { x * scale, y * scale };
}

float MacOSDesktop::Layout::px(float value) const
{
    return value * scale;
}

void MacOSDesktop::update_layout()
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    layout_.display = viewport->Size;
    layout_.scale = std::min(
        viewport->Size.x / kCanvasWidth, viewport->Size.y / kCanvasHeight);
    layout_.scale = std::max(layout_.scale, 0.55f);
    layout_.origin = { viewport->Pos.x
            + (viewport->Size.x - kCanvasWidth * layout_.scale) * 0.5f,
        viewport->Pos.y
            + (viewport->Size.y - kCanvasHeight * layout_.scale) * 0.5f };
}

bool MacOSDesktop::hit_rect(const char* id, ImVec2 min, ImVec2 max)
{
    ImGui::SetCursorScreenPos(min);
    ImGui::InvisibleButton(id, sub(max, min));
    return ImGui::IsItemClicked(ImGuiMouseButton_Left);
}

void MacOSDesktop::render()
{
    update_layout();
    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(0, 0, 0, 0));
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration
        | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings
        | ImGuiWindowFlags_NoBringToFrontOnFocus;
    ImGui::Begin("##macos-desktop", nullptr, flags);
    ImDrawList* draw = ImGui::GetWindowDrawList();

    draw_wallpaper(draw);
    draw_desktop_icons(draw);
    if (finder_visible_ && !finder_minimized_) {
        draw_finder(draw);
    }
    draw_menu_bar(draw);
    draw_dock(draw);
    if (apple_menu_open_) {
        draw_apple_menu(draw);
    }
    if (control_center_open_) {
        draw_control_center(draw);
    }

    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}

void MacOSDesktop::draw_wallpaper(ImDrawList* draw) const
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImVec2 min = viewport->Pos;
    const ImVec2 max = add(viewport->Pos, viewport->Size);
    const ImU32 top = dark_mode_ ? rgba(10, 15, 28) : rgba(25, 44, 82);
    const ImU32 middle = dark_mode_ ? rgba(18, 31, 55) : rgba(29, 75, 113);
    const ImU32 bottom = dark_mode_ ? rgba(29, 21, 49) : rgba(73, 52, 112);
    constexpr int bands = 48;
    for (int band = 0; band < bands; ++band) {
        const float t0 = static_cast<float>(band) / bands;
        const float t1 = static_cast<float>(band + 1) / bands;
        const ImU32 c0 = t0 < 0.58f
            ? mix_color(top, middle, t0 / 0.58f)
            : mix_color(middle, bottom, (t0 - 0.58f) / 0.42f);
        const ImU32 c1 = t1 < 0.58f
            ? mix_color(top, middle, t1 / 0.58f)
            : mix_color(middle, bottom, (t1 - 0.58f) / 0.42f);
        const float y0 = min.y + viewport->Size.y * t0;
        const float y1 = min.y + viewport->Size.y * t1 + 1.0f;
        draw->AddRectFilledMultiColor(
            { min.x, y0 }, { max.x, y1 }, c0, c0, c1, c1);
    }

    struct Glow {
        float x;
        float y;
        float radius;
        ImU32 color;
    };

    const std::array<Glow, 7> glows = { {
        { 100.0f, 740.0f, 460.0f, rgba(57, 204, 202, 16) },
        { 280.0f, 760.0f, 390.0f, rgba(59, 176, 235, 18) },
        { 1200.0f, 780.0f, 470.0f, rgba(230, 91, 167, 17) },
        { 1380.0f, 260.0f, 410.0f, rgba(84, 151, 245, 15) },
        { 780.0f, 460.0f, 520.0f, rgba(41, 108, 171, 11) },
        { 430.0f, 100.0f, 330.0f, rgba(77, 170, 221, 10) },
        { 890.0f, 950.0f, 480.0f, rgba(130, 71, 183, 13) },
    } };
    for (const Glow& glow : glows) {
        const ImVec2 center = layout_.point(glow.x, glow.y);
        for (int ring = 10; ring >= 1; --ring) {
            const float ratio = static_cast<float>(ring) / 10.0f;
            const int alpha
                = static_cast<int>(((glow.color >> IM_COL32_A_SHIFT) & 0xff)
                    * (1.0f - ratio * 0.55f));
            const ImU32 color = (glow.color & ~(0xffu << IM_COL32_A_SHIFT))
                | (static_cast<ImU32>(alpha) << IM_COL32_A_SHIFT);
            draw->AddCircleFilled(
                center, layout_.px(glow.radius * ratio), color, 96);
        }
    }

    // Fine grain makes the generated mesh-gradient feel less synthetic.
    for (int i = 0; i < 180; ++i) {
        const float x = std::fmod(static_cast<float>(i * 193), kCanvasWidth);
        const float y = std::fmod(static_cast<float>(i * 317), kCanvasHeight);
        draw->AddCircleFilled(
            layout_.point(x, y), layout_.px(0.65f), rgba(255, 255, 255, 12));
    }
}

void MacOSDesktop::draw_menu_bar(ImDrawList* draw)
{
    const float s = layout_.scale;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImVec2 bar_min = viewport->Pos;
    const ImVec2 bar_max { viewport->Pos.x + viewport->Size.x,
        viewport->Pos.y + 31.0f * s };
    draw->AddRectFilled(bar_min, bar_max,
        dark_mode_ ? rgba(12, 15, 24, 218) : rgba(23, 28, 45, 190));
    draw->AddLine({ bar_min.x, bar_max.y }, { bar_max.x, bar_max.y },
        rgba(255, 255, 255, 28), 1.0f);

    const float left = layout_.origin.x;
    const ImU32 foreground = rgba(255, 255, 255, 246);
    const ImVec2 apple_center { left + 24.0f * s, viewport->Pos.y + 15.0f * s };
    if (hit_rect("##apple-menu-button", { left + 7.0f * s, bar_min.y },
            { left + 43.0f * s, bar_max.y })) {
        apple_menu_open_ = !apple_menu_open_;
        control_center_open_ = false;
    }
    if (apple_menu_open_) {
        draw->AddRectFilled({ left + 7.0f * s, bar_min.y + 2.0f * s },
            { left + 43.0f * s, bar_max.y - 2.0f * s }, rgba(255, 255, 255, 28),
            5.0f * s);
    }
    draw_apple(draw, apple_center, s, foreground);

    struct MenuItem {
        const char* label;
        float x;
        bool strong;
    };

    const std::array<MenuItem, 7> items = { {
        { "访达", 51.0f, true },
        { "文件", 105.0f, false },
        { "编辑", 158.0f, false },
        { "显示", 211.0f, false },
        { "前往", 264.0f, false },
        { "窗口", 317.0f, false },
        { "帮助", 370.0f, false },
    } };
    for (const MenuItem& item : items) {
        text(draw, { left + item.x * s, bar_min.y + 6.4f * s }, foreground,
            item.label, (item.strong ? 15.3f : 14.8f) * s);
    }

    std::time_t now = std::time(nullptr);
    std::tm local {};
#if defined(_WIN32)
    localtime_s(&local, &now);
#else
    localtime_r(&now, &local);
#endif
    char clock[64] {};
    static constexpr std::array<const char*, 7> weekdays
        = { "周日", "周一", "周二", "周三", "周四", "周五", "周六" };
    std::snprintf(clock, sizeof(clock), "%s  %d月%d日  %02d:%02d",
        weekdays[static_cast<std::size_t>(local.tm_wday)], local.tm_mon + 1,
        local.tm_mday, local.tm_hour, local.tm_min);

    const float right = layout_.origin.x + kCanvasWidth * s;
    const ImVec2 control_center { right - 183.0f * s, bar_min.y + 15.0f * s };
    const ImVec2 control_min { right - 200.0f * s, bar_min.y };
    const ImVec2 control_max { right - 164.0f * s, bar_max.y };
    if (hit_rect("##control-center-button", control_min, control_max)) {
        control_center_open_ = !control_center_open_;
        apple_menu_open_ = false;
    }
    if (control_center_open_) {
        draw->AddRectFilled(
            { control_min.x + 2.0f * s, control_min.y + 2.0f * s },
            { control_max.x - 2.0f * s, control_max.y - 2.0f * s },
            rgba(255, 255, 255, 28), 5.0f * s);
    }

    draw_battery(
        draw, { right - 279.0f * s, bar_min.y + 10.2f * s }, s, foreground);
    draw_wifi(
        draw, { right - 236.0f * s, bar_min.y + 20.0f * s }, s, foreground);
    draw_control_glyph(draw, control_center, s, foreground);
    text(draw, layout_.point(1287.0f, 6.5f), foreground,
        std::string_view(clock), 14.4f * s);
}

void MacOSDesktop::draw_desktop_icons(ImDrawList* draw)
{
    const float s = layout_.scale;
    const float x = 1366.0f;

    struct DesktopItem {
        const char* label;
        float y;
        int type;
    };

    const std::array<DesktopItem, 3> items = { {
        { "Macintosh HD", 84.0f, 0 },
        { "项目资料", 178.0f, 1 },
        { "桌面图片", 272.0f, 1 },
    } };

    for (int i = 0; i < static_cast<int>(items.size()); ++i) {
        const DesktopItem& item = items[i];
        const ImVec2 center = layout_.point(x, item.y);
        const ImVec2 hit_min = layout_.point(x - 55.0f, item.y - 34.0f);
        const ImVec2 hit_max = layout_.point(x + 55.0f, item.y + 48.0f);
        if (hit_rect(
                ("##desktop-" + std::to_string(i)).c_str(), hit_min, hit_max)) {
            selected_item_ = 100 + i;
        }
        const bool selected = selected_item_ == 100 + i;
        if (selected) {
            draw->AddRectFilled(layout_.point(x - 49.0f, item.y - 31.0f),
                layout_.point(x + 49.0f, item.y + 48.0f),
                rgba(64, 126, 225, 125), 6.0f * s);
        }
        if (item.type == 0) {
            draw->AddRectFilled(layout_.point(x - 22.0f, item.y - 23.0f),
                layout_.point(x + 22.0f, item.y + 15.0f), rgba(202, 207, 211),
                5.0f * s);
            draw->AddRectFilled(layout_.point(x - 20.0f, item.y - 20.0f),
                layout_.point(x + 20.0f, item.y - 10.0f), rgba(231, 234, 236),
                4.0f * s);
            draw->AddCircleFilled(layout_.point(x + 13.0f, item.y + 8.0f),
                2.0f * s, rgba(70, 187, 104));
        } else {
            draw_folder(draw, center, s * 1.08f, selected);
        }
        const float label_size = 13.7f * s;
        const ImVec2 label_center = layout_.point(x, item.y + 35.0f);
        const ImVec2 dimensions = ImGui::GetFont()->CalcTextSizeA(
            label_size, 1000.0f, 0.0f, item.label);
        const ImVec2 position { label_center.x - dimensions.x * 0.5f,
            label_center.y - dimensions.y * 0.5f };
        if (selected) {
            draw->AddRectFilled(
                { position.x - 4.0f * s, position.y - 1.0f * s },
                { position.x + dimensions.x + 4.0f * s,
                    position.y + dimensions.y + 1.0f * s },
                rgba(43, 105, 213), 3.0f * s);
        }
        shadowed_text(draw, position, item.label, label_size);
    }
}

void MacOSDesktop::draw_finder(ImDrawList* draw)
{
    const float s = layout_.scale;
    const ImVec2 min = layout_.point(finder_position_.x, finder_position_.y);
    const ImVec2 size = layout_.size(finder_size_.x, finder_size_.y);
    const ImVec2 max = add(min, size);
    const float rounding = 13.0f * s;

    soft_shadow(draw, min, max, rounding, s, 95);
    draw->AddRectFilled(min, max,
        dark_mode_ ? rgba(39, 40, 44, 251) : rgba(247, 247, 249, 251),
        rounding);
    draw->AddRect(min, max, rgba(255, 255, 255, dark_mode_ ? 30 : 100),
        rounding, 0, 1.0f * s);

    const float sidebar_width = 234.0f;
    draw->AddRectFilled(min, { min.x + sidebar_width * s, max.y },
        dark_mode_ ? rgba(45, 45, 49, 248) : rgba(223, 227, 232, 246), rounding,
        ImDrawFlags_RoundCornersLeft);
    draw->AddLine({ min.x + sidebar_width * s, min.y },
        { min.x + sidebar_width * s, max.y },
        dark_mode_ ? rgba(255, 255, 255, 20) : rgba(100, 105, 112, 32), s);
    draw->AddLine({ min.x, min.y + 69.0f * s }, { max.x, min.y + 69.0f * s },
        dark_mode_ ? rgba(255, 255, 255, 24) : rgba(95, 99, 106, 36), s);

    const std::array<ImU32, 3> traffic
        = { rgba(255, 95, 87), rgba(255, 189, 46), rgba(40, 201, 64) };
    const bool traffic_hovered = contains(ImGui::GetIO().MousePos,
        { min.x + 10.0f * s, min.y + 8.0f * s },
        { min.x + 76.0f * s, min.y + 37.0f * s });
    for (int i = 0; i < 3; ++i) {
        const ImVec2 center { min.x + (21.0f + i * 22.0f) * s,
            min.y + 22.0f * s };
        draw->AddCircleFilled(center, 7.0f * s, traffic[i]);
        draw->AddCircle(center, 7.0f * s, rgba(0, 0, 0, 45), 20, s);
        if (traffic_hovered) {
            if (i == 0) {
                draw->AddLine(add(center, { -2.3f * s, -2.3f * s }),
                    add(center, { 2.3f * s, 2.3f * s }), rgba(75, 35, 31, 180),
                    s);
                draw->AddLine(add(center, { 2.3f * s, -2.3f * s }),
                    add(center, { -2.3f * s, 2.3f * s }), rgba(75, 35, 31, 180),
                    s);
            } else if (i == 1) {
                draw->AddLine(add(center, { -3.0f * s, 0 }),
                    add(center, { 3.0f * s, 0 }), rgba(85, 60, 22, 180), s);
            } else {
                draw->AddTriangleFilled(add(center, { -3.0f * s, 2.0f * s }),
                    add(center, { 2.0f * s, 2.0f * s }),
                    add(center, { 2.0f * s, -3.0f * s }),
                    rgba(20, 82, 31, 180));
                draw->AddTriangleFilled(add(center, { 3.0f * s, -2.0f * s }),
                    add(center, { -2.0f * s, -2.0f * s }),
                    add(center, { -2.0f * s, 3.0f * s }),
                    rgba(20, 82, 31, 180));
            }
        }
        if (hit_rect(("##traffic-" + std::to_string(i)).c_str(),
                add(center, { -9.0f * s, -9.0f * s }),
                add(center, { 9.0f * s, 9.0f * s }))) {
            if (i == 0) {
                finder_visible_ = false;
            } else if (i == 1) {
                finder_minimized_ = true;
            } else if (!finder_maximized_) {
                finder_restore_position_ = finder_position_;
                finder_restore_size_ = finder_size_;
                finder_position_ = { 24.0f, 40.0f };
                finder_size_ = { 1392.0f, 744.0f };
                finder_maximized_ = true;
            } else {
                finder_position_ = finder_restore_position_;
                finder_size_ = finder_restore_size_;
                finder_maximized_ = false;
            }
        }
    }

    // A wide empty section of the toolbar behaves as the native title-bar drag
    // target.
    const ImVec2 drag_min { min.x + 360.0f * s, min.y };
    const ImVec2 drag_max { max.x - 240.0f * s, min.y + 44.0f * s };
    ImGui::SetCursorScreenPos(drag_min);
    ImGui::InvisibleButton("##finder-drag", sub(drag_max, drag_min));
    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)
        && !finder_maximized_) {
        finder_position_.x += ImGui::GetIO().MouseDelta.x / s;
        finder_position_.y += ImGui::GetIO().MouseDelta.y / s;
        finder_position_.x = std::clamp(finder_position_.x,
            -finder_size_.x + 180.0f, kCanvasWidth - 180.0f);
        finder_position_.y
            = std::clamp(finder_position_.y, 31.0f, kCanvasHeight - 120.0f);
    }

    // Back/forward navigation controls.
    const ImU32 toolbar_color
        = dark_mode_ ? rgba(227, 228, 231) : rgba(52, 54, 60);
    const ImVec2 back_center { min.x + (sidebar_width + 31.0f) * s,
        min.y + 35.0f * s };
    const ImVec2 forward_center { min.x + (sidebar_width + 68.0f) * s,
        min.y + 35.0f * s };
    draw->AddLine(add(back_center, { 3.0f * s, -5.0f * s }),
        add(back_center, { -2.0f * s, 0 }), toolbar_color, 1.7f * s);
    draw->AddLine(add(back_center, { -2.0f * s, 0 }),
        add(back_center, { 3.0f * s, 5.0f * s }), toolbar_color, 1.7f * s);
    draw->AddLine(add(forward_center, { -3.0f * s, -5.0f * s }),
        add(forward_center, { 2.0f * s, 0 }), rgba(125, 126, 131), 1.7f * s);
    draw->AddLine(add(forward_center, { 2.0f * s, 0 }),
        add(forward_center, { -3.0f * s, 5.0f * s }), rgba(125, 126, 131),
        1.7f * s);

    const char* title_label = "最近使用";
    switch (location_) {
    case Location::AirDrop:
        title_label = "隔空投送";
        break;
    case Location::Applications:
        title_label = "应用程序";
        break;
    case Location::Desktop:
        title_label = "桌面";
        break;
    case Location::Documents:
        title_label = "文稿";
        break;
    case Location::Downloads:
        title_label = "下载";
        break;
    case Location::ICloud:
        title_label = "iCloud 云盘";
        break;
    case Location::Recent:
        break;
    }
    text(draw, { min.x + (sidebar_width + 103.0f) * s, min.y + 22.0f * s },
        toolbar_color, title_label, 16.2f * s);

    // Toolbar view/share controls.
    const float tools_x = max.x - 338.0f * s;
    for (int col = 0; col < 3; ++col) {
        for (int row = 0; row < 3; ++row) {
            draw->AddRectFilled(
                { tools_x + col * 5.0f * s, min.y + (27.0f + row * 5.0f) * s },
                { tools_x + (col * 5.0f + 2.5f) * s,
                    min.y + (29.5f + row * 5.0f) * s },
                toolbar_color, 0.7f * s);
        }
    }
    draw->AddCircle({ max.x - 291.0f * s, min.y + 35.0f * s }, 8.0f * s,
        toolbar_color, 20, 1.2f * s);
    draw->AddLine({ max.x - 291.0f * s, min.y + 38.0f * s },
        { max.x - 291.0f * s, min.y + 24.0f * s }, toolbar_color, 1.2f * s);
    draw->AddTriangleFilled({ max.x - 295.0f * s, min.y + 27.0f * s },
        { max.x - 287.0f * s, min.y + 27.0f * s },
        { max.x - 291.0f * s, min.y + 21.5f * s }, toolbar_color);

    const ImVec2 search_min { max.x - 224.0f * s, min.y + 17.0f * s };
    const ImVec2 search_max { max.x - 16.0f * s, min.y + 53.0f * s };
    draw->AddRectFilled(search_min, search_max,
        dark_mode_ ? rgba(91, 92, 97, 150) : rgba(255, 255, 255, 185),
        8.0f * s);
    draw->AddRect(search_min, search_max,
        dark_mode_ ? rgba(255, 255, 255, 28) : rgba(91, 94, 102, 42), 8.0f * s,
        0, s);
    draw_magnifier(draw, { search_min.x + 17.0f * s, search_min.y + 17.0f * s },
        s, rgba(114, 116, 122));

    ImGui::SetCursorScreenPos(
        { search_min.x + 31.0f * s, search_min.y + 6.0f * s });
    ImGui::SetNextItemWidth(search_max.x - search_min.x - 38.0f * s);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 1.0f * s, 2.0f * s });
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, rgba(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_Text, toolbar_color);
    ImGui::PushStyleColor(ImGuiCol_TextDisabled, rgba(115, 116, 121));
    ImGui::InputTextWithHint(
        "##finder-search", "搜索", search_.data(), search_.size());
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);

    draw_finder_sidebar(draw, { min.x, min.y + 70.0f * s },
        { sidebar_width * s, size.y - 70.0f * s });
    draw_finder_content(draw, { min.x + sidebar_width * s, min.y + 70.0f * s },
        { size.x - sidebar_width * s, size.y - 70.0f * s });
}

void MacOSDesktop::draw_finder_sidebar(
    ImDrawList* draw, ImVec2 pos, ImVec2 size)
{
    const float s = layout_.scale;
    const ImU32 primary = dark_mode_ ? rgba(232, 233, 235) : rgba(50, 52, 57);
    const ImU32 secondary
        = dark_mode_ ? rgba(168, 169, 174) : rgba(104, 106, 112);
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
            location_ = entry.location;
            selected_item_ = -1;
        }
        if (location_ == entry.location) {
            draw->AddRectFilled(row_min, row_max,
                dark_mode_ ? rgba(255, 255, 255, 34) : rgba(105, 110, 119, 38),
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
        location_ = cloud.location;
        selected_item_ = -1;
    }
    if (location_ == Location::ICloud) {
        draw->AddRectFilled(row_min, row_max,
            dark_mode_ ? rgba(255, 255, 255, 34) : rgba(105, 110, 119, 38),
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

void MacOSDesktop::draw_finder_content(
    ImDrawList* draw, ImVec2 pos, ImVec2 size)
{
    const float s = layout_.scale;
    const ImU32 primary = dark_mode_ ? rgba(239, 239, 241) : rgba(41, 42, 46);
    const ImU32 secondary
        = dark_mode_ ? rgba(160, 162, 167) : rgba(112, 114, 120);
    draw->PushClipRect(pos, add(pos, size), true);

    const char* heading = "最近使用";
    if (location_ == Location::Applications)
        heading = "应用程序";
    else if (location_ == Location::Desktop)
        heading = "桌面上的项目";
    else if (location_ == Location::Documents)
        heading = "文稿";
    else if (location_ == Location::Downloads)
        heading = "下载项目";
    else if (location_ == Location::AirDrop)
        heading = "隔空投送";
    else if (location_ == Location::ICloud)
        heading = "iCloud 云盘";

    text(draw, { pos.x + 30.0f * s, pos.y + 23.0f * s }, primary, heading,
        20.0f * s);
    text(draw, { pos.x + 30.0f * s, pos.y + 51.0f * s }, secondary,
        location_ == Location::AirDrop ? "附近的设备会显示在这里"
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
        = location_ == Location::Applications ? applications : recent;
    const std::string query(search_.data());
    int visible_index = 0;
    for (int index = 0; index < static_cast<int>(files.size()); ++index) {
        const FileItem& file = files[index];
        if (!query.empty()
            && std::string(file.name).find(query) == std::string::npos) {
            continue;
        }
        if (location_ == Location::AirDrop && index > 2) {
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
            selected_item_ = index;
        }
        const bool selected = selected_item_ == index;
        if (selected) {
            draw->AddRectFilled(hit_min, hit_max,
                dark_mode_ ? rgba(45, 111, 205, 120) : rgba(56, 132, 229, 95),
                8.0f * s);
        } else if (contains(ImGui::GetIO().MousePos, hit_min, hit_max)) {
            draw->AddRectFilled(hit_min, hit_max,
                dark_mode_ ? rgba(255, 255, 255, 15) : rgba(40, 45, 52, 12),
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
        dark_mode_ ? rgba(255, 255, 255, 22) : rgba(93, 96, 104, 30), s);
    char status[48] {};
    std::snprintf(
        status, sizeof(status), "%d 个项目，128.42 GB 可用", visible_index);
    centered_text(draw, { pos.x + size.x * 0.5f, status_y + 13.0f * s },
        secondary, status, 11.7f * s);
    draw->PopClipRect();
}

void MacOSDesktop::draw_dock(ImDrawList* draw)
{
    const float s = layout_.scale;
    constexpr int icon_count = 10;
    constexpr float base_size = 53.0f;
    constexpr float spacing = 11.0f;
    const float content_width
        = icon_count * base_size + (icon_count - 1) * spacing + 24.0f;
    const float dock_x = (kCanvasWidth - content_width) * 0.5f;
    const float dock_y = 810.0f;
    const ImVec2 dock_min = layout_.point(dock_x - 15.0f, dock_y - 9.0f);
    const ImVec2 dock_max
        = layout_.point(dock_x + content_width + 15.0f, dock_y + 69.0f);

    soft_shadow(draw, dock_min, dock_max, 22.0f * s, s, 70);
    draw->AddRectFilled(dock_min, dock_max,
        dark_mode_ ? rgba(46, 47, 52, 190) : rgba(237, 241, 246, 173),
        22.0f * s);
    draw->AddRect(
        dock_min, dock_max, rgba(255, 255, 255, 102), 22.0f * s, 0, s);
    draw->AddLine(layout_.point(dock_x + content_width - base_size - 19.0f,
                      dock_y + 2.0f),
        layout_.point(
            dock_x + content_width - base_size - 19.0f, dock_y + 58.0f),
        rgba(55, 59, 67, 72), s);

    const ImVec2 mouse = ImGui::GetIO().MousePos;
    static constexpr std::array<const char*, icon_count> labels
        = { "访达", "启动台", "Safari 浏览器", "信息", "邮件", "照片", "日历",
              "备忘录", "系统设置", "废纸篓" };

    for (int index = 0; index < icon_count; ++index) {
        const float extra_gap = index == icon_count - 1 ? 13.0f : 0.0f;
        const float reference_x = dock_x + 12.0f + base_size * 0.5f
            + index * (base_size + spacing) + extra_gap;
        const ImVec2 base_center = layout_.point(reference_x, dock_y + 28.0f);
        const float distance = std::abs(mouse.x - base_center.x) / s;
        float magnification = 0.0f;
        if (mouse.y > dock_min.y - 70.0f * s
            && mouse.y < dock_max.y + 6.0f * s) {
            magnification = std::max(0.0f, 1.0f - distance / 92.0f);
        }
        const float icon_size
            = (base_size + 22.0f * magnification * magnification) * s;
        const ImVec2 center { base_center.x,
            layout_.point(0, dock_y + 54.0f).y - icon_size * 0.5f };
        const ImVec2 hit_min { base_center.x - (base_size + spacing) * 0.5f * s,
            dock_min.y - 50.0f * s };
        const ImVec2 hit_max { base_center.x + (base_size + spacing) * 0.5f * s,
            dock_max.y };
        const bool hovered = contains(mouse, hit_min, hit_max);

        if (hit_rect(("##dock-" + std::to_string(index)).c_str(), hit_min,
                hit_max)) {
            active_dock_item_ = index;
            if (index == 0) {
                finder_visible_ = true;
                finder_minimized_ = false;
            }
        }

        draw_dock_icon(draw, index, center, icon_size);
        if (index == active_dock_item_ && index != 9) {
            draw->AddCircleFilled({ base_center.x, dock_max.y - 4.0f * s },
                2.1f * s, dark_mode_ ? rgba(237, 238, 240) : rgba(45, 48, 54));
        }
        if (hovered) {
            const float label_size = 13.0f * s;
            const ImVec2 dimensions = ImGui::GetFont()->CalcTextSizeA(
                label_size, 1000.0f, 0.0f, labels[index]);
            const ImVec2 tooltip_min { center.x - dimensions.x * 0.5f
                    - 9.0f * s,
                center.y - icon_size * 0.5f - 36.0f * s };
            const ImVec2 tooltip_max { center.x + dimensions.x * 0.5f
                    + 9.0f * s,
                tooltip_min.y + 27.0f * s };
            soft_shadow(draw, tooltip_min, tooltip_max, 6.0f * s, s, 42);
            draw->AddRectFilled(tooltip_min, tooltip_max,
                dark_mode_ ? rgba(56, 57, 62, 235) : rgba(244, 246, 249, 235),
                6.0f * s);
            centered_text(draw,
                { (tooltip_min.x + tooltip_max.x) * 0.5f,
                    (tooltip_min.y + tooltip_max.y) * 0.5f },
                dark_mode_ ? rgba(245, 245, 246) : rgba(39, 41, 45),
                labels[index], label_size);
        }
    }
}

void MacOSDesktop::draw_apple_menu(ImDrawList* draw)
{
    const float s = layout_.scale;
    const ImVec2 min = layout_.point(8.0f, 35.0f);
    const ImVec2 max = layout_.point(286.0f, 394.0f);
    soft_shadow(draw, min, max, 12.0f * s, s, 85);
    draw->AddRectFilled(min, max,
        dark_mode_ ? rgba(46, 47, 52, 242) : rgba(239, 242, 247, 239),
        12.0f * s);
    draw->AddRect(min, max, rgba(255, 255, 255, 80), 12.0f * s, 0, s);

    struct AppleEntry {
        const char* label;
        const char* shortcut_key;
        float y;
        bool separator_after;
    };

    const std::array<AppleEntry, 12> entries = { {
        { "关于本机", "", 13.0f, true },
        { "系统设置…", "", 55.0f, false },
        { "App Store…", "", 84.0f, true },
        { "最近使用的项目", "›", 126.0f, true },
        { "强制退出…", "⌥⌘⎋", 168.0f, true },
        { "睡眠", "", 210.0f, false },
        { "重新启动…", "", 239.0f, false },
        { "关机…", "", 268.0f, true },
        { "锁定屏幕", "⌃⌘Q", 310.0f, false },
        { "退出登录 Izan…", "⇧⌘Q", 339.0f, false },
        { "", "", 0.0f, false },
        { "", "", 0.0f, false },
    } };

    const ImU32 foreground
        = dark_mode_ ? rgba(245, 245, 246) : rgba(28, 29, 32);
    for (int i = 0; i < 10; ++i) {
        const AppleEntry& entry = entries[i];
        const ImVec2 row_min { min.x + 7.0f * s, min.y + entry.y * s };
        const ImVec2 row_max { max.x - 7.0f * s, row_min.y + 27.0f * s };
        const bool hovered
            = contains(ImGui::GetIO().MousePos, row_min, row_max);
        if (hovered) {
            draw->AddRectFilled(row_min, row_max, rgba(35, 122, 229), 5.0f * s);
        }
        if (hit_rect(("##apple-entry-" + std::to_string(i)).c_str(), row_min,
                row_max)) {
            if (i == 1) {
                active_dock_item_ = 8;
            }
            apple_menu_open_ = false;
        }
        text(draw, { row_min.x + 9.0f * s, row_min.y + 3.0f * s },
            hovered ? rgba(255, 255, 255) : foreground, entry.label, 14.0f * s);
        if (entry.shortcut_key[0] != '\0') {
            const ImVec2 dimensions = ImGui::GetFont()->CalcTextSizeA(
                13.0f * s, 1000.0f, 0.0f, entry.shortcut_key);
            text(draw,
                { row_max.x - dimensions.x - 9.0f * s, row_min.y + 3.8f * s },
                hovered ? rgba(255, 255, 255) : rgba(105, 106, 112),
                entry.shortcut_key, 13.0f * s);
        }
        if (entry.separator_after) {
            draw->AddLine({ min.x + 12.0f * s, row_max.y + 5.5f * s },
                { max.x - 12.0f * s, row_max.y + 5.5f * s },
                dark_mode_ ? rgba(255, 255, 255, 28) : rgba(68, 70, 76, 28), s);
        }
    }

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)
        && !contains(ImGui::GetIO().MousePos, min, max)
        && ImGui::GetIO().MousePos.y > layout_.point(0, 31.0f).y) {
        apple_menu_open_ = false;
    }
}

void MacOSDesktop::draw_control_center(ImDrawList* draw)
{
    const float s = layout_.scale;
    const ImVec2 min = layout_.point(1063.0f, 38.0f);
    const ImVec2 max = layout_.point(1426.0f, 366.0f);
    const ImU32 panel
        = dark_mode_ ? rgba(49, 50, 55, 239) : rgba(235, 239, 245, 235);
    const ImU32 card
        = dark_mode_ ? rgba(91, 92, 98, 142) : rgba(255, 255, 255, 164);
    const ImU32 primary = dark_mode_ ? rgba(247, 247, 248) : rgba(30, 31, 34);
    const ImU32 secondary
        = dark_mode_ ? rgba(184, 185, 190) : rgba(94, 97, 103);
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
        { "无线局域网", wifi_enabled_ ? "Home 5G" : "关闭", &wifi_enabled_,
            20.0f, 0 },
        { "蓝牙", bluetooth_enabled_ ? "打开" : "关闭", &bluetooth_enabled_,
            66.0f, 1 },
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
        dark_mode_ ? rgba(75, 81, 103) : rgba(105, 111, 129));
    draw->AddCircleFilled(
        add(theme_center, { 4.0f * s, -4.0f * s }), 9.0f * s, card);
    text(draw, { theme_min.x + 51.0f * s, theme_min.y + 18.0f * s }, primary,
        "深色模式", 13.5f * s);
    text(draw, { theme_min.x + 51.0f * s, theme_min.y + 37.0f * s }, secondary,
        dark_mode_ ? "打开" : "关闭", 11.5f * s);
    if (hit_rect("##dark-mode", theme_min, theme_max)) {
        dark_mode_ = !dark_mode_;
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
            dark_mode_ ? rgba(28, 29, 33, 185) : rgba(194, 198, 205, 190),
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
    draw_slider("##brightness", "显示器", brightness_, 176.0f, false);
    draw_slider("##volume", "声音", volume_, 253.0f, true);

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)
        && !contains(ImGui::GetIO().MousePos, min, max)
        && ImGui::GetIO().MousePos.y > layout_.point(0, 31.0f).y) {
        control_center_open_ = false;
    }
}
