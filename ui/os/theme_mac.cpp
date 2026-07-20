#include <algorithm>
#include <array>
#include <cmath>
#include <string>

#include "ui/os/theme.hpp"
#include "ui/widgets/design.hpp"

// The macOS-cultured theme: traffic lights on the left, a soft
// drop-shadowed rounded body, a mesh-gradient wallpaper — all in the
// active izan color theme, not Apple's palette.

namespace izan::os {

namespace {

    void soft_shadow(ImDrawList* draw, ImVec2 min, ImVec2 max, float rounding,
        float unit, int strength)
    {
        for (int layer = 8; layer >= 1; --layer) {
            const float spread = static_cast<float>(layer) * 0.14f * unit;
            const int alpha = std::max(1, strength / (layer * 2));
            draw->AddRectFilled({ min.x - spread, min.y - spread * 0.25f },
                { max.x + spread, max.y + spread },
                IM_COL32(0, 0, 0, alpha), rounding + spread);
        }
    }

    ImU32 mix(ImU32 a, ImU32 b, float t)
    {
        t = std::clamp(t, 0.0f, 1.0f);
        const auto ch = [t](ImU32 x, ImU32 y, int shift) {
            const float first = static_cast<float>((x >> shift) & 0xff);
            const float second = static_cast<float>((y >> shift) & 0xff);
            return static_cast<ImU32>(first + (second - first) * t) << shift;
        };
        return ch(a, b, IM_COL32_R_SHIFT) | ch(a, b, IM_COL32_G_SHIFT)
            | ch(a, b, IM_COL32_B_SHIFT) | ch(a, b, IM_COL32_A_SHIFT);
    }

    class MacTheme final : public Theme {
    public:
        float title_height(float em) const override
        {
            return em * 1.7f;
        }

        OsRect control_rect(WindowControl control, ImVec2 rmin, ImVec2,
            float em) const override
        {
            const int i = static_cast<int>(control);
            const float title_h = title_height(em);
            const ImVec2 center { rmin.x + em * (0.9f + i * 0.95f),
                rmin.y + title_h * 0.5f };
            const float r = em * 0.30f + 3.0f;
            return { { center.x - r, center.y - r },
                { center.x + r, center.y + r } };
        }

        void paint_window(ImDrawList* draw, const WindowLook& look,
            ImVec2 rmin, ImVec2 rmax, float em) const override
        {
            const bool dark = ui::kit_is_dark();
            const float title_h = title_height(em);
            const float rounding = em * 0.55f;
            const ImVec2 rsize { rmax.x - rmin.x, rmax.y - rmin.y };

            soft_shadow(
                draw, rmin, rmax, rounding, em, look.focused ? 95 : 55);

            const ImVec4 bg = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
            ImVec4 body = bg;
            body.w = 0.99f;
            draw->AddRectFilled(
                rmin, rmax, ImGui::GetColorU32(body), rounding);
            ImVec4 strip = dark
                ? ui::kit_blend(bg, ImVec4(1, 1, 1, 1), 0.055f)
                : ui::kit_blend(bg, ImVec4(1, 1, 1, 1), 0.5f);
            strip.w = 1.0f;
            draw->AddRectFilled(rmin, { rmax.x, rmin.y + title_h },
                ImGui::GetColorU32(strip), rounding,
                ImDrawFlags_RoundCornersTop);
            draw->AddLine({ rmin.x, rmin.y + title_h },
                { rmax.x, rmin.y + title_h },
                IM_COL32(dark ? 255 : 40, dark ? 255 : 44, dark ? 255 : 52,
                    dark ? 22 : 36),
                1.0f);
            draw->AddRect(rmin, rmax,
                IM_COL32(
                    255, 255, 255, dark ? (look.focused ? 40 : 24) : 110),
                rounding, 0, 1.0f);

            // Traffic lights; glyphs surface on hover over the strip.
            static constexpr ImU32 kLights[3] = { IM_COL32(255, 95, 87, 255),
                IM_COL32(255, 189, 46, 255), IM_COL32(40, 201, 64, 255) };
            const ImVec2 mouse = ImGui::GetIO().MousePos;
            const bool hot = mouse.x >= rmin.x && mouse.x <= rmin.x + em * 3.6f
                && mouse.y >= rmin.y && mouse.y <= rmin.y + title_h;
            for (int i = 0; i < 3; ++i) {
                const ImVec2 center { rmin.x + em * (0.9f + i * 0.95f),
                    rmin.y + title_h * 0.5f };
                const float radius = em * 0.30f;
                ImU32 color = kLights[i];
                if (!look.focused && !hot)
                    color = dark ? IM_COL32(120, 122, 128, 160)
                                 : IM_COL32(180, 183, 189, 200);
                draw->AddCircleFilled(center, radius, color);
                draw->AddCircle(
                    center, radius, IM_COL32(0, 0, 0, 45), 0, 1.0f);
                if (hot) {
                    const ImU32 glyph = IM_COL32(30, 30, 34, 190);
                    const float g = radius * 0.5f;
                    if (i == 0) {
                        draw->AddLine({ center.x - g, center.y - g },
                            { center.x + g, center.y + g }, glyph, 1.4f);
                        draw->AddLine({ center.x + g, center.y - g },
                            { center.x - g, center.y + g }, glyph, 1.4f);
                    } else if (i == 1) {
                        draw->AddLine({ center.x - g, center.y },
                            { center.x + g, center.y }, glyph, 1.4f);
                    } else {
                        draw->AddTriangleFilled({ center.x - g, center.y + g },
                            { center.x + g, center.y + g },
                            { center.x + g, center.y - g }, glyph);
                    }
                }
            }

            // Title: mark and name, centered.
            const std::string title
                = std::string(look.app->mark()) + " " + look.app->name();
            ImGui::PushFont(nullptr, em * 0.92f);
            const ImVec2 ts = ImGui::CalcTextSize(title.c_str());
            const ImVec4 tcol = ImGui::GetStyleColorVec4(
                look.focused ? ImGuiCol_Text : ImGuiCol_TextDisabled);
            draw->AddText({ rmin.x + (rsize.x - ts.x) * 0.5f,
                              rmin.y + (title_h - ts.y) * 0.5f },
                ImGui::GetColorU32(tcol), title.c_str());
            ImGui::PopFont();

            // Resize grip whisper.
            if (!look.maximized) {
                const ImU32 grip = IM_COL32(dark ? 255 : 60, dark ? 255 : 64,
                    dark ? 255 : 70, dark ? 40 : 70);
                for (int i = 1; i <= 3; ++i) {
                    const float o = em * 0.28f * static_cast<float>(i);
                    draw->AddLine({ rmax.x - o, rmax.y - em * 0.2f },
                        { rmax.x - em * 0.2f, rmax.y - o }, grip, 1.2f);
                }
            }
        }

        void paint_wallpaper(ImDrawList* draw, ImVec2 min, ImVec2 max,
            float em) const override
        {
            const bool dark = ui::kit_is_dark();
            const ImVec2 view { max.x - min.x, max.y - min.y };
            const ImU32 top
                = dark ? IM_COL32(10, 15, 28, 255) : IM_COL32(25, 44, 82, 255);
            const ImU32 middle = dark ? IM_COL32(18, 31, 55, 255)
                                      : IM_COL32(29, 75, 113, 255);
            const ImU32 bottom = dark ? IM_COL32(29, 21, 49, 255)
                                      : IM_COL32(73, 52, 112, 255);
            constexpr int bands = 48;
            for (int band = 0; band < bands; ++band) {
                const float t0 = static_cast<float>(band) / bands;
                const float t1 = static_cast<float>(band + 1) / bands;
                const ImU32 c0 = t0 < 0.58f
                    ? mix(top, middle, t0 / 0.58f)
                    : mix(middle, bottom, (t0 - 0.58f) / 0.42f);
                const ImU32 c1 = t1 < 0.58f
                    ? mix(top, middle, t1 / 0.58f)
                    : mix(middle, bottom, (t1 - 0.58f) / 0.42f);
                draw->AddRectFilledMultiColor(
                    { min.x, min.y + view.y * t0 },
                    { max.x, min.y + view.y * t1 + 1.0f }, c0, c0, c1, c1);
            }

            struct Glow {
                float fx;
                float fy;
                float radius_em;
                ImU32 color;
            };
            const std::array<Glow, 7> glows = { {
                { 0.07f, 0.82f, 16.0f, IM_COL32(57, 204, 202, 16) },
                { 0.19f, 0.84f, 14.0f, IM_COL32(59, 176, 235, 18) },
                { 0.83f, 0.87f, 17.0f, IM_COL32(230, 91, 167, 17) },
                { 0.96f, 0.29f, 15.0f, IM_COL32(84, 151, 245, 15) },
                { 0.54f, 0.51f, 19.0f, IM_COL32(41, 108, 171, 11) },
                { 0.30f, 0.11f, 12.0f, IM_COL32(77, 170, 221, 10) },
                { 0.62f, 1.05f, 17.0f, IM_COL32(130, 71, 183, 13) },
            } };
            for (const Glow& glow : glows) {
                const ImVec2 center { min.x + view.x * glow.fx,
                    min.y + view.y * glow.fy };
                for (int ring = 10; ring >= 1; --ring) {
                    const float ratio = static_cast<float>(ring) / 10.0f;
                    const int alpha = static_cast<int>(
                        ((glow.color >> IM_COL32_A_SHIFT) & 0xff)
                        * (1.0f - ratio * 0.55f));
                    const ImU32 color
                        = (glow.color & ~(0xffu << IM_COL32_A_SHIFT))
                        | (static_cast<ImU32>(alpha) << IM_COL32_A_SHIFT);
                    draw->AddCircleFilled(center,
                        em * glow.radius_em * ratio, color, 72);
                }
            }

            // Fine grain keeps the gradient from feeling synthetic.
            for (int i = 0; i < 160; ++i) {
                const float fx
                    = std::fmod(static_cast<float>(i) * 0.61803f, 1.0f);
                const float fy
                    = std::fmod(static_cast<float>(i) * 0.38197f, 1.0f);
                draw->AddCircleFilled(
                    { min.x + view.x * fx, min.y + view.y * fy }, em * 0.025f,
                    IM_COL32(255, 255, 255, 12));
            }
        }
    };

}

const Theme& mac_theme()
{
    static const MacTheme theme;
    return theme;
}

}
