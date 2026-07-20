#include <algorithm>
#include <array>
#include <string>

#include "ui/os/theme.hpp"
#include "ui/widgets/design.hpp"

// The Mint-cultured theme — the working development skin. Bottom
// panel instead of bar-and-dock, window controls on the right, a
// flat geometric wallpaper. Its own palette, no one's logo.

namespace izan::os {

namespace {

    constexpr ImU32 kAccent = IM_COL32(140, 190, 90, 255);

    void soft_shadow(ImDrawList* draw, ImVec2 min, ImVec2 max, float rounding,
        float unit, int strength)
    {
        for (int layer = 7; layer >= 1; --layer) {
            const float spread = static_cast<float>(layer) * 0.13f * unit;
            const int alpha = std::max(1, strength / (layer * 2));
            draw->AddRectFilled({ min.x - spread, min.y - spread * 0.25f },
                { max.x + spread, max.y + spread }, IM_COL32(0, 0, 0, alpha),
                rounding + spread);
        }
    }

    class MintTheme final : public Theme {
    public:
        float title_height(float em) const override
        {
            return em * 1.55f;
        }

        OsRect control_rect(WindowControl control, ImVec2 rmin, ImVec2 rmax,
            float em) const override
        {
            // Right-hand controls, close at the edge: … [–] [□] [×]
            static constexpr int kSlot[3] = { 0, 2, 1 }; // Close/Min/Max
            const int slot = kSlot[static_cast<int>(control)];
            const float title_h = title_height(em);
            const ImVec2 center { rmax.x
                    - em * (0.85f + static_cast<float>(slot) * 1.45f),
                rmin.y + title_h * 0.5f };
            const float r = em * 0.55f;
            return { { center.x - r, center.y - r },
                { center.x + r, center.y + r } };
        }

        void paint_window(ImDrawList* draw, const WindowLook& look, ImVec2 rmin,
            ImVec2 rmax, float em) const override
        {
            const bool dark = ui::kit_is_dark();
            const float title_h = title_height(em);
            const float rounding = em * 0.28f;
            const ImVec2 rsize { rmax.x - rmin.x, rmax.y - rmin.y };

            soft_shadow(draw, rmin, rmax, rounding, em, look.focused ? 90 : 50);

            const ImVec4 bg = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
            ImVec4 body = bg;
            body.w = 1.0f;
            draw->AddRectFilled(rmin, rmax, ImGui::GetColorU32(body), rounding);
            ImVec4 strip = dark ? ui::kit_blend(bg, ImVec4(1, 1, 1, 1), 0.045f)
                                : ui::kit_blend(bg, ImVec4(1, 1, 1, 1), 0.42f);
            strip.w = 1.0f;
            draw->AddRectFilled(rmin, { rmax.x, rmin.y + title_h },
                ImGui::GetColorU32(strip), rounding,
                ImDrawFlags_RoundCornersTop);
            draw->AddLine({ rmin.x, rmin.y + title_h },
                { rmax.x, rmin.y + title_h }, IM_COL32(0, 0, 0, dark ? 90 : 40),
                1.0f);
            if (look.focused)
                draw->AddLine({ rmin.x + rounding, rmin.y + 1.0f },
                    { rmax.x - rounding, rmin.y + 1.0f }, kAccent, 2.0f);
            draw->AddRect(rmin, rmax,
                IM_COL32(255, 255, 255, dark ? (look.focused ? 44 : 26) : 105),
                rounding, 0, 1.0f);

            // Controls: always-on glyphs, a wash behind the hovered
            // one, the close button turning warning-red.
            const ImVec2 mouse = ImGui::GetIO().MousePos;
            const ImU32 glyph_col = dark ? IM_COL32(225, 227, 228, 235)
                                         : IM_COL32(60, 64, 68, 235);
            for (int c = 0; c < 3; ++c) {
                const OsRect r = control_rect(
                    static_cast<WindowControl>(c), rmin, rmax, em);
                const ImVec2 center { (r.min.x + r.max.x) * 0.5f,
                    (r.min.y + r.max.y) * 0.5f };
                const bool hot = r.contains(mouse);
                const bool is_close
                    = c == static_cast<int>(WindowControl::Close);
                if (hot)
                    draw->AddCircleFilled(center, em * 0.52f,
                        is_close ? IM_COL32(204, 66, 66, 255)
                                 : IM_COL32(255, 255, 255, dark ? 30 : 60));
                const ImU32 ink = hot && is_close ? IM_COL32_WHITE : glyph_col;
                const float g = em * 0.24f;
                if (is_close) {
                    draw->AddLine({ center.x - g, center.y - g },
                        { center.x + g, center.y + g }, ink, 1.5f);
                    draw->AddLine({ center.x + g, center.y - g },
                        { center.x - g, center.y + g }, ink, 1.5f);
                } else if (c == static_cast<int>(WindowControl::Minimize)) {
                    draw->AddLine({ center.x - g, center.y + g * 0.9f },
                        { center.x + g, center.y + g * 0.9f }, ink, 1.5f);
                } else if (look.maximized) {
                    const float o = g * 0.45f;
                    draw->AddRect({ center.x - g, center.y - g + o },
                        { center.x + g - o, center.y + g }, ink, 2.0f, 0, 1.4f);
                    draw->AddRect({ center.x - g + o, center.y - g },
                        { center.x + g, center.y + g - o }, ink, 2.0f, 0, 1.4f);
                } else {
                    draw->AddRect({ center.x - g, center.y - g },
                        { center.x + g, center.y + g }, ink, 2.0f, 0, 1.4f);
                }
            }

            // Title: mark and name, centered in the strip.
            const std::string title
                = std::string(look.app->mark()) + " " + look.app->name();
            ImGui::PushFont(nullptr, em * 0.9f);
            const ImVec2 ts = ImGui::CalcTextSize(title.c_str());
            const ImVec4 tcol = ImGui::GetStyleColorVec4(
                look.focused ? ImGuiCol_Text : ImGuiCol_TextDisabled);
            draw->AddText({ rmin.x + (rsize.x - ts.x) * 0.5f,
                              rmin.y + (title_h - ts.y) * 0.5f },
                ImGui::GetColorU32(tcol), title.c_str());
            ImGui::PopFont();

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

        void paint_wallpaper(
            ImDrawList* draw, ImVec2 min, ImVec2 max, float) const override
        {
            const ImVec2 view { max.x - min.x, max.y - min.y };
            constexpr int bands = 50;
            for (int band = 0; band < bands; ++band) {
                const float t = static_cast<float>(band) / bands;
                const float t1 = static_cast<float>(band + 1) / bands;
                const auto shade = [](float f) {
                    return IM_COL32(static_cast<int>(20 + f * 13),
                        static_cast<int>(68 + f * 43),
                        static_cast<int>(66 + f * 24), 255);
                };
                draw->AddRectFilledMultiColor({ min.x, min.y + view.y * t },
                    { max.x, min.y + view.y * t1 + 1.0f }, shade(t), shade(t),
                    shade(t1), shade(t1));
            }
            const float basis = std::min(view.x, view.y);
            draw->AddCircleFilled(
                { min.x + view.x * 0.77f, min.y + view.y * 0.27f },
                basis * 0.31f, IM_COL32(62, 145, 102, 55), 96);
            draw->AddCircleFilled(
                { min.x + view.x * 0.63f, min.y + view.y * 0.70f },
                basis * 0.39f, IM_COL32(13, 77, 72, 65), 96);
            draw->AddTriangleFilled({ min.x + view.x * 0.58f, max.y },
                { max.x, min.y + view.y * 0.34f }, max,
                IM_COL32(26, 96, 84, 70));
            draw->AddTriangleFilled({ min.x, min.y + view.y * 0.56f },
                { min.x + view.x * 0.35f, max.y }, { min.x, max.y },
                IM_COL32(9, 56, 61, 55));
        }
    };

}

const Theme& mint_theme()
{
    static const MintTheme theme;
    return theme;
}

}
