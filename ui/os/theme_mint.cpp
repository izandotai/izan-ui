#include <algorithm>
#include <array>
#include <string>

#include "ui/os/mint_paint.hpp"
#include "ui/os/theme.hpp"
#include "ui/render/sdf_rect.hpp"

// The Mint-cultured theme — the working development skin, drawn to
// match the Cinnamon desktop line for line: a light Nemo-style
// window, full-height title-bar controls on the right, the teal
// geometric wallpaper with desktop icons living on it.

namespace izan::os {

namespace {

    using namespace mint;

    constexpr float kTitleLogical = 46.0f;
    constexpr float kControlLogical = 44.0f;

    class MintTheme final : public Theme {
    public:
        float title_height(float em) const override
        {
            return em * kTitleLogical
                / (ui::kDefaultFontSize / ui::kFontDesignScale);
        }

        OsRect control_rect(WindowControl control, ImVec2 rmin, ImVec2 rmax,
            float em) const override
        {
            // Full-height buttons flush to the top-right corner,
            // close at the edge: [–] [□] [×]
            static constexpr int kSlot[3] = { 0, 2, 1 }; // Close/Min/Max
            const int slot = kSlot[static_cast<int>(control)];
            const float s = mint_scale();
            const float w = kControlLogical * s;
            const float x1 = rmax.x - static_cast<float>(slot) * w;
            return { { x1 - w, rmin.y }, { x1, rmin.y + title_height(em) } };
        }

        void paint_window(ImDrawList* draw, const WindowLook& look, ImVec2 rmin,
            ImVec2 rmax, float em) const override
        {
            const float s = mint_scale();
            const float title_h = title_height(em);
            const float rounding = 8.0f * s;

            window_shadow(draw, rmin, rmax, rounding, s);
            // One sheet of glass owns the window's opacity: the body
            // alone carries alpha 245 (0.96). Every surface an app
            // paints on top is a faint tint, not a second sheet —
            // stacked sheets square the transmission away and read as
            // plain darkening, never as glass.
            {
                render::SdfRect body;
                body.min = rmin;
                body.max = rmax;
                body.radius[0] = body.radius[1] = body.radius[2]
                    = body.radius[3] = rounding;
                body.fill = IM_COL32(247, 248, 247, 245);
                render::sdf_rect(draw, body);
            }

            // The title strip is a tint over the glass; same rect and
            // radius as the body, so its arc lies exactly on the
            // body's arc — a corner has one silhouette or it shows a
            // crescent of whatever lies beneath.
            const ImVec2 title_max { rmax.x, rmin.y + title_h };
            draw->AddRectFilled(rmin, title_max, IM_COL32(0, 0, 0, 15),
                rounding, ImDrawFlags_RoundCornersTop);
            draw->AddLine({ rmin.x, title_max.y }, { rmax.x, title_max.y },
                IM_COL32(69, 73, 70, 45), 1.0f);

            // Identity on the left: mark and name in one string, one
            // draw — two separate draws each center their own glyph
            // box and the emoji drifts off the text's baseline.
            {
                const std::string title
                    = std::string(look.app->mark()) + "  " + look.app->name();
                text_vcentered(draw, rmin.x + 14.0f * s,
                    (rmin.y + title_max.y) * 0.5f,
                    look.focused ? IM_COL32(47, 51, 48, 255)
                                 : IM_COL32(47, 51, 48, 150),
                    title, kFontWindowTitle * s);
            }

            // Controls: bare glyphs, a wash behind the hovered one,
            // close turning warning-red and keeping the corner round.
            const ImVec2 mouse = ImGui::GetIO().MousePos;
            for (int c = 0; c < 3; ++c) {
                const OsRect r = control_rect(
                    static_cast<WindowControl>(c), rmin, rmax, em);
                const bool hot = r.contains(mouse);
                const bool is_close
                    = c == static_cast<int>(WindowControl::Close);
                if (hot) {
                    render::SdfRect wash;
                    wash.min = r.min;
                    wash.max = r.max;
                    if (is_close)
                        wash.radius[1] = rounding; // top-right only
                    wash.fill = is_close ? IM_COL32(205, 65, 65, 255)
                                         : IM_COL32(72, 78, 74, 25);
                    render::sdf_rect(draw, wash);
                }
                const ImVec2 center { (r.min.x + r.max.x) * 0.5f,
                    (r.min.y + r.max.y) * 0.5f };
                static constexpr int kGlyph[3] = { 2, 0, 1 }; // Close/Min/Max
                control_icon(draw, center, kGlyph[c], look.maximized,
                    hot && is_close ? IM_COL32_WHITE
                                    : IM_COL32(68, 72, 69, 255),
                    s);
            }

            // The outline lands last, over the hovered controls, so
            // the rounded outer edge never breaks at a corner —
            // analytic, so the arc is computed, not approximated.
            {
                render::SdfRect rim;
                rim.min = rmin;
                rim.max = rmax;
                rim.radius[0] = rim.radius[1] = rim.radius[2] = rim.radius[3]
                    = rounding;
                rim.fill = IM_COL32(0, 0, 0, 0);
                rim.border = IM_COL32(25, 29, 28, 82);
                rim.border_px = 2.0f;
                render::sdf_rect(draw, rim);
            }
        }

        void paint_wallpaper(
            ImDrawList* draw, ImVec2 min, ImVec2 max, float) const override
        {
            const float s = mint_scale();
            const ImVec2 view { max.x - min.x, max.y - min.y };
            constexpr int bands = 50;
            for (int band = 0; band < bands; ++band) {
                const float t = static_cast<float>(band) / bands;
                draw->AddRectFilled({ min.x, min.y + view.y * t },
                    { max.x,
                        min.y + view.y * (static_cast<float>(band + 1) / bands)
                            + 1.0f },
                    IM_COL32(static_cast<int>(20 + t * 13),
                        static_cast<int>(68 + t * 43),
                        static_cast<int>(66 + t * 24), 255));
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
            mint_logo(draw, { min.x + view.x * 0.72f, min.y + view.y * 0.51f },
                std::clamp(basis * 0.095f, 72.0f, 118.0f));

            // The desktop icons live on the wallpaper.
            struct Item {
                const char* name;
                float y;
                int type;
            };

            static constexpr std::array<Item, 4> kItems = { {
                { "Home", 82.0f, 0 },
                { "Computer", 178.0f, 1 },
                { "Network", 274.0f, 2 },
                { "Trash", 370.0f, 3 },
            } };
            for (const Item& item : kItems) {
                const ImVec2 center { min.x + 65.0f * s, min.y + item.y * s };
                if (item.type == 0) {
                    folder_icon(draw, center, 1.15f * s);
                    draw->AddTriangleFilled(
                        { center.x - 26.0f * s, center.y - 8.0f * s },
                        { center.x, center.y - 29.0f * s },
                        { center.x + 26.0f * s, center.y - 8.0f * s },
                        IM_COL32(87, 158, 58, 255));
                } else if (item.type == 3) {
                    draw->AddRectFilled(
                        { center.x - 18.0f * s, center.y - 22.0f * s },
                        { center.x + 18.0f * s, center.y + 23.0f * s },
                        IM_COL32(219, 224, 220, 255), 4.0f * s);
                    draw->AddRectFilled(
                        { center.x - 22.0f * s, center.y - 27.0f * s },
                        { center.x + 22.0f * s, center.y - 20.0f * s },
                        IM_COL32(241, 243, 241, 255), 2.0f * s);
                } else {
                    draw->AddRectFilled(
                        { center.x - 24.0f * s, center.y - 18.0f * s },
                        { center.x + 24.0f * s, center.y + 15.0f * s },
                        item.type == 1 ? IM_COL32(191, 198, 196, 255)
                                       : IM_COL32(92, 164, 179, 255),
                        5.0f * s);
                    draw->AddRectFilled(
                        { center.x - 17.0f * s, center.y + 15.0f * s },
                        { center.x + 17.0f * s, center.y + 20.0f * s },
                        IM_COL32(225, 229, 228, 255), 2.0f * s);
                }
                text_centered(draw, { center.x, min.y + (item.y + 42.0f) * s },
                    IM_COL32(255, 255, 255, 255), item.name, kFontBody * s);
            }
        }
    };

}

const Theme& mint_theme()
{
    static const MintTheme theme;
    return theme;
}

}
