// The built-in cursor set, drawn with signed distance fields the same
// way the widgets are. The design language is studied from the folded-
// paper school of cursor art: solid white faceted bodies, a thin dark
// outline, dark crease strokes inside the shape, one red accent on the
// forbidden sign — all edges straight, all corners sharp. The geometry
// here is original; only the vocabulary is shared.
//
// Every shape lives in a 100-unit art box and is evaluated per pixel:
// coverage comes straight from the distance, so the set is crisp at
// whatever size the system asks for. Ink spans ~60 of the 100 units,
// the proportion the school sizes its cursors at.
#include "ui/shell/cursor_paint.hpp"

#include <imgui.h>

#include <algorithm>
#include <cmath>

namespace izan::ui {

namespace {

    struct V {
        float x, y;
    };

    V operator-(V a, V b)
    {
        return { a.x - b.x, a.y - b.y };
    }

    float dot(V a, V b)
    {
        return a.x * b.x + a.y * b.y;
    }

    float clampf(float v, float lo, float hi)
    {
        return std::min(hi, std::max(lo, v));
    }

    // iq's signed polygon distance: negative inside, winding by parity.
    float sd_polygon(const std::vector<V>& v, V p)
    {
        float d = dot(p - v[0], p - v[0]);
        float s = 1.0f;
        for (std::size_t i = 0, j = v.size() - 1; i < v.size(); j = i++) {
            const V e = v[j] - v[i];
            const V w = p - v[i];
            const float t = clampf(dot(w, e) / dot(e, e), 0.0f, 1.0f);
            const V b { w.x - e.x * t, w.y - e.y * t };
            d = std::min(d, dot(b, b));
            const bool c0 = p.y >= v[i].y;
            const bool c1 = p.y < v[j].y;
            const bool c2 = e.x * w.y > e.y * w.x;
            if ((c0 && c1 && c2) || (!c0 && !c1 && !c2))
                s = -s;
        }
        return s * std::sqrt(d);
    }

    float sd_segment(V p, V a, V b)
    {
        const V pa = p - a;
        const V ba = b - a;
        const float h = clampf(dot(pa, ba) / dot(ba, ba), 0.0f, 1.0f);
        const V q { pa.x - ba.x * h, pa.y - ba.y * h };
        return std::sqrt(dot(q, q));
    }

    float sd_box(V p, V c, V half, float r)
    {
        const V q { std::abs(p.x - c.x) - half.x + r,
            std::abs(p.y - c.y) - half.y + r };
        const V qp { std::max(q.x, 0.0f), std::max(q.y, 0.0f) };
        return std::sqrt(dot(qp, qp)) + std::min(std::max(q.x, q.y), 0.0f) - r;
    }

    V rotated(V p, V c, float deg)
    {
        const float rad = deg * 3.14159265f / 180.0f;
        const float cs = std::cos(rad), sn = std::sin(rad);
        const V d = p - c;
        return { c.x + d.x * cs - d.y * sn, c.y + d.x * sn + d.y * cs };
    }

    struct Stroke {
        V a, b;
        float hw; // half-width
    };

    struct Box {
        V c, half;
        float r;
    };

    // A cursor: white body (polygons + boxes), dark creases over it,
    // an optional red accent patch with dark marks over that. rot
    // spins everything around the box center.
    struct Shape {
        std::vector<std::vector<V>> polys;
        std::vector<Box> boxes;
        std::vector<Stroke> creases;
        std::vector<Box> accents;
        std::vector<Stroke> marks;
        float rot = 0.0f;
        V hot { 0.0f, 0.0f };

        float body(V p) const
        {
            float d = 1e9f;
            for (const auto& poly : polys)
                d = std::min(d, sd_polygon(poly, p));
            for (const Box& b : boxes)
                d = std::min(d, sd_box(p, b.c, b.half, b.r));
            return d;
        }

        float crease(V p) const
        {
            float d = 1e9f;
            for (const Stroke& s : creases)
                d = std::min(d, sd_segment(p, s.a, s.b) - s.hw);
            return d;
        }

        float accent(V p) const
        {
            float d = 1e9f;
            for (const Box& b : accents)
                d = std::min(d, sd_box(p, b.c, b.half, b.r));
            return d;
        }

        float mark(V p) const
        {
            float d = 1e9f;
            for (const Stroke& s : marks)
                d = std::min(d, sd_segment(p, s.a, s.b) - s.hw);
            return d;
        }
    };

    // The sail: vertical left edge, straight hypotenuse, flat base —
    // with a fold-echo of itself creased inside.
    Shape arrow_shape()
    {
        Shape s;
        s.polys.push_back(
            { { 25.0f, 12.0f }, { 25.0f, 70.0f }, { 60.0f, 70.0f } });
        s.creases.push_back({ { 36.0f, 41.0f }, { 36.0f, 61.0f }, 2.0f });
        s.creases.push_back({ { 36.0f, 61.0f }, { 48.0f, 61.0f }, 2.0f });
        s.creases.push_back({ { 36.0f, 41.0f }, { 48.0f, 61.0f }, 2.0f });
        s.hot = { 25.0f, 12.0f };
        return s;
    }

    // A slender bar with small squared flares.
    Shape text_shape()
    {
        Shape s;
        s.boxes.push_back({ { 50.0f, 50.0f }, { 2.6f, 30.0f }, 0.5f });
        s.boxes.push_back({ { 50.0f, 22.5f }, { 5.0f, 2.5f }, 0.5f });
        s.boxes.push_back({ { 50.0f, 77.5f }, { 5.0f, 2.5f }, 0.5f });
        s.hot = { 50.0f, 50.0f };
        return s;
    }

    // The kite: an elongated diamond, creased across its waist.
    Shape ns_shape()
    {
        Shape s;
        s.polys.push_back({ { 50.0f, 16.0f }, { 72.0f, 50.0f },
            { 50.0f, 84.0f }, { 28.0f, 50.0f } });
        s.creases.push_back({ { 41.0f, 50.0f }, { 59.0f, 50.0f }, 2.0f });
        s.hot = { 50.0f, 50.0f };
        return s;
    }

    // The compass: a square diamond, creased into four quadrants with
    // the center left unfolded.
    Shape move_shape()
    {
        Shape s;
        s.polys.push_back({ { 50.0f, 10.0f }, { 90.0f, 50.0f },
            { 50.0f, 90.0f }, { 10.0f, 50.0f } });
        s.creases.push_back({ { 50.0f, 26.0f }, { 50.0f, 43.0f }, 2.0f });
        s.creases.push_back({ { 50.0f, 57.0f }, { 50.0f, 74.0f }, 2.0f });
        s.creases.push_back({ { 26.0f, 50.0f }, { 43.0f, 50.0f }, 2.0f });
        s.creases.push_back({ { 57.0f, 50.0f }, { 74.0f, 50.0f }, 2.0f });
        s.hot = { 50.0f, 50.0f };
        return s;
    }

    // The pointing hand, cut angular: squared index finger, faceted
    // palm with a flat base, two knuckle creases.
    Shape hand_shape()
    {
        Shape s;
        s.boxes.push_back({ { 44.0f, 37.0f }, { 5.0f, 21.0f }, 1.5f });
        s.polys.push_back({ { 35.0f, 52.0f }, { 49.0f, 44.0f },
            { 66.0f, 46.0f }, { 69.0f, 60.0f }, { 64.0f, 76.0f },
            { 38.0f, 76.0f }, { 35.0f, 66.0f } });
        s.creases.push_back({ { 55.0f, 50.0f }, { 55.0f, 59.0f }, 1.8f });
        s.creases.push_back({ { 62.0f, 52.0f }, { 62.0f, 61.0f }, 1.8f });
        s.hot = { 44.0f, 16.0f };
        return s;
    }

    // The sail again, wearing the red refusal badge.
    Shape no_shape()
    {
        Shape s = arrow_shape();
        s.accents.push_back({ { 30.0f, 72.0f }, { 17.0f, 15.0f }, 4.0f });
        s.marks.push_back({ { 23.0f, 65.0f }, { 37.0f, 79.0f }, 2.2f });
        s.marks.push_back({ { 37.0f, 65.0f }, { 23.0f, 79.0f }, 2.2f });
        s.hot = { 25.0f, 12.0f };
        return s;
    }

    Shape shape_for(int slot, bool& ok)
    {
        ok = true;
        if (slot == ImGuiMouseCursor_Arrow)
            return arrow_shape();
        if (slot == ImGuiMouseCursor_TextInput)
            return text_shape();
        if (slot == ImGuiMouseCursor_ResizeAll)
            return move_shape();
        if (slot == ImGuiMouseCursor_ResizeNS)
            return ns_shape();
        if (slot == ImGuiMouseCursor_ResizeEW) {
            Shape s = ns_shape();
            s.rot = 90.0f;
            return s;
        }
        if (slot == ImGuiMouseCursor_ResizeNWSE) {
            Shape s = ns_shape();
            s.rot = -45.0f;
            return s;
        }
        if (slot == ImGuiMouseCursor_ResizeNESW) {
            Shape s = ns_shape();
            s.rot = 45.0f;
            return s;
        }
        if (slot == ImGuiMouseCursor_Hand)
            return hand_shape();
        if (slot == ImGuiMouseCursor_NotAllowed)
            return no_shape();
        ok = false;
        return {};
    }

    struct Layer {
        float r, g, b;
    };

    constexpr Layer kInk { 53.0f, 53.0f, 53.0f }; // outline + creases
    constexpr Layer kPaper { 255.0f, 255.0f, 255.0f };
    constexpr Layer kBadge { 240.0f, 122.0f, 112.0f };

}

CursorArt paint_cursor(int imgui_slot, int size)
{
    bool ok = false;
    const Shape shape = shape_for(imgui_slot, ok);
    if (!ok || size <= 0)
        return {};

    CursorArt art;
    art.w = size;
    art.h = size;
    art.px.assign(static_cast<std::size_t>(size) * size, 0u);
    const float to_art = 100.0f / static_cast<float>(size);
    const float to_px = 1.0f / to_art;
    constexpr float kRim = 4.2f; // art units: 2px on the school's 48px
    constexpr float kAA = 1.0f;  // device pixels

    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            V p { (static_cast<float>(x) + 0.5f) * to_art,
                (static_cast<float>(y) + 0.5f) * to_art };
            if (shape.rot != 0.0f)
                p = rotated(p, { 50.0f, 50.0f }, -shape.rot);

            const float body = shape.body(p) * to_px;
            const float accent = shape.accent(p) * to_px;
            // The outline hugs body and badge alike.
            const float rim = std::min(body, accent) - kRim * to_px;
            const float crease = shape.crease(p) * to_px;
            const float mark = shape.mark(p) * to_px;

            // Bottom to top: outline, paper, badge, crease strokes.
            float out_r = 0.0f, out_g = 0.0f, out_b = 0.0f, out_a = 0.0f;
            const auto over = [&](const Layer& c, float d_px) {
                const float cov = clampf(0.5f - d_px / kAA, 0.0f, 1.0f);
                if (cov <= 0.0f)
                    return;
                const float a = cov + out_a * (1.0f - cov);
                out_r = (c.r * cov + out_r * out_a * (1.0f - cov)) / a;
                out_g = (c.g * cov + out_g * out_a * (1.0f - cov)) / a;
                out_b = (c.b * cov + out_b * out_a * (1.0f - cov)) / a;
                out_a = a;
            };
            over(kInk, rim);
            over(kPaper, body);
            over(kBadge, accent);
            over(kInk, std::min(crease, mark));

            if (out_a <= 0.0f)
                continue;
            art.px[static_cast<std::size_t>(y) * size + x]
                = (static_cast<std::uint32_t>(out_a * 255.0f + 0.5f) << 24)
                | (static_cast<std::uint32_t>(out_r + 0.5f) << 16)
                | (static_cast<std::uint32_t>(out_g + 0.5f) << 8)
                | static_cast<std::uint32_t>(out_b + 0.5f);
        }
    }

    art.hot_x = static_cast<int>(shape.hot.x * to_px + 0.5f);
    art.hot_y = static_cast<int>(shape.hot.y * to_px + 0.5f);
    return art;
}

}
