#include "wallpaper.hpp"

#include <array>
#include <cmath>

namespace macos {

void draw_wallpaper(
    ImDrawList* draw, const Layout& layout, const DesktopState& state)
{
    const bool dark = state.control.dark_mode;
    const ImVec2 min = layout.view_min;
    const ImVec2 max = layout.view_max;
    const ImVec2 view { max.x - min.x, max.y - min.y };
    const ImU32 top = dark ? rgba(10, 15, 28) : rgba(25, 44, 82);
    const ImU32 middle = dark ? rgba(18, 31, 55) : rgba(29, 75, 113);
    const ImU32 bottom = dark ? rgba(29, 21, 49) : rgba(73, 52, 112);
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
        const float y0 = min.y + view.y * t0;
        const float y1 = min.y + view.y * t1 + 1.0f;
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
        const ImVec2 center = layout.point(glow.x, glow.y);
        for (int ring = 10; ring >= 1; --ring) {
            const float ratio = static_cast<float>(ring) / 10.0f;
            const int alpha
                = static_cast<int>(((glow.color >> IM_COL32_A_SHIFT) & 0xff)
                    * (1.0f - ratio * 0.55f));
            const ImU32 color = (glow.color & ~(0xffu << IM_COL32_A_SHIFT))
                | (static_cast<ImU32>(alpha) << IM_COL32_A_SHIFT);
            draw->AddCircleFilled(
                center, layout.px(glow.radius * ratio), color, 96);
        }
    }

    // Fine grain makes the generated mesh-gradient feel less synthetic.
    for (int i = 0; i < 180; ++i) {
        const float x = std::fmod(static_cast<float>(i * 193), kCanvasWidth);
        const float y = std::fmod(static_cast<float>(i * 317), kCanvasHeight);
        draw->AddCircleFilled(
            layout.point(x, y), layout.px(0.65f), rgba(255, 255, 255, 12));
    }
}

}
