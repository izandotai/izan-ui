// The built-in cursor set, drawn with signed distance fields the same
// way the widgets are. Every shape lives in a 100-unit art box and is
// evaluated per pixel: dark ink body, white rim, coverage from the
// distance itself — crisp at any size the system asks for. The
// geometry is original; the ink-and-rim scheme is generic cursor
// culture, owned by no one.
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

    float sd_capsule(V p, V a, V b, float r)
    {
        const V pa = p - a;
        const V ba = b - a;
        const float h = clampf(dot(pa, ba) / dot(ba, ba), 0.0f, 1.0f);
        const V q { pa.x - ba.x * h, pa.y - ba.y * h };
        return std::sqrt(dot(q, q)) - r;
    }

    float sd_circle(V p, V c, float r)
    {
        const V d = p - c;
        return std::sqrt(dot(d, d)) - r;
    }

    float sd_round_box(V p, V c, V half, float r)
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

    // A shape is the min over its parts, each part one primitive
    // evaluated in art space. rot spins the whole shape around center.
    struct Shape {
        std::vector<std::vector<V>> polys;

        struct Cap {
            V a, b;
            float r;
        };

        std::vector<Cap> caps;

        struct Ring {
            V c;
            float radius, hw;
        };

        std::vector<Ring> rings;

        struct Box {
            V c, half;
            float r;
        };

        std::vector<Box> boxes;
        float rot = 0.0f;
        float round = 0.0f; // dilate: rounds convex joins
        V hot { 0.0f, 0.0f };

        float eval(V p) const
        {
            if (rot != 0.0f)
                p = rotated(p, { 50.0f, 50.0f }, -rot);
            float d = 1e9f;
            for (const auto& poly : polys)
                d = std::min(d, sd_polygon(poly, p));
            for (const Cap& c : caps)
                d = std::min(d, sd_capsule(p, c.a, c.b, c.r));
            for (const Ring& r : rings)
                d = std::min(d, std::abs(sd_circle(p, r.c, r.radius)) - r.hw);
            for (const Box& b : boxes)
                d = std::min(d, sd_round_box(p, b.c, b.half, b.r));
            return d - round;
        }
    };

    Shape arrow_shape()
    {
        Shape s;
        s.polys.push_back({ { 14.0f, 6.0f }, { 14.0f, 70.0f }, { 28.5f, 56.5f },
            { 38.0f, 78.0f }, { 47.5f, 74.0f }, { 38.0f, 52.5f },
            { 58.0f, 52.5f } });
        s.round = 1.5f;
        s.hot = { 14.0f, 6.0f };
        return s;
    }

    Shape text_shape()
    {
        Shape s;
        s.caps.push_back({ { 50.0f, 16.0f }, { 50.0f, 84.0f }, 3.5f });
        s.caps.push_back({ { 41.0f, 13.0f }, { 59.0f, 13.0f }, 3.5f });
        s.caps.push_back({ { 41.0f, 87.0f }, { 59.0f, 87.0f }, 3.5f });
        s.hot = { 50.0f, 50.0f };
        return s;
    }

    Shape ns_shape()
    {
        Shape s;
        s.caps.push_back({ { 50.0f, 24.0f }, { 50.0f, 76.0f }, 4.5f });
        s.polys.push_back(
            { { 50.0f, 6.0f }, { 36.0f, 26.0f }, { 64.0f, 26.0f } });
        s.polys.push_back(
            { { 50.0f, 94.0f }, { 64.0f, 74.0f }, { 36.0f, 74.0f } });
        s.round = 1.2f;
        s.hot = { 50.0f, 50.0f };
        return s;
    }

    Shape move_shape()
    {
        Shape s;
        s.caps.push_back({ { 50.0f, 24.0f }, { 50.0f, 76.0f }, 4.0f });
        s.caps.push_back({ { 24.0f, 50.0f }, { 76.0f, 50.0f }, 4.0f });
        s.polys.push_back(
            { { 50.0f, 8.0f }, { 38.0f, 25.0f }, { 62.0f, 25.0f } });
        s.polys.push_back(
            { { 50.0f, 92.0f }, { 62.0f, 75.0f }, { 38.0f, 75.0f } });
        s.polys.push_back(
            { { 8.0f, 50.0f }, { 25.0f, 62.0f }, { 25.0f, 38.0f } });
        s.polys.push_back(
            { { 92.0f, 50.0f }, { 75.0f, 38.0f }, { 75.0f, 62.0f } });
        s.round = 1.2f;
        s.hot = { 50.0f, 50.0f };
        return s;
    }

    Shape hand_shape()
    {
        Shape s;
        // Palm, index up, three folded knuckles, thumb, wrist — the
        // classic link hand reduced to capsules and a rounded box.
        s.boxes.push_back({ { 55.0f, 66.0f }, { 19.0f, 17.0f }, 11.0f });
        s.caps.push_back({ { 43.0f, 22.0f }, { 43.0f, 58.0f }, 6.5f });
        s.caps.push_back({ { 56.0f, 42.0f }, { 56.0f, 52.0f }, 6.0f });
        s.caps.push_back({ { 67.0f, 45.0f }, { 67.0f, 55.0f }, 5.5f });
        s.caps.push_back({ { 36.0f, 62.0f }, { 29.0f, 73.0f }, 6.0f });
        s.boxes.push_back({ { 55.0f, 84.0f }, { 14.0f, 7.0f }, 6.0f });
        s.hot = { 43.0f, 16.0f };
        return s;
    }

    Shape no_shape()
    {
        Shape s;
        s.rings.push_back({ { 50.0f, 50.0f }, 34.0f, 6.0f });
        s.caps.push_back({ { 28.0f, 28.0f }, { 72.0f, 72.0f }, 6.0f });
        s.hot = { 50.0f, 50.0f };
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
    constexpr float kRim = 5.5f; // art units; ~1.8px on a 32px cursor
    constexpr float kAA = 1.0f;  // device pixels
    const std::uint32_t ink_r = 0x1d, ink_g = 0x1d, ink_b = 0x21;

    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            const V p { (static_cast<float>(x) + 0.5f) * to_art,
                (static_cast<float>(y) + 0.5f) * to_art };
            const float d_px = shape.eval(p) * to_px;
            const float body = clampf(0.5f - d_px / kAA, 0.0f, 1.0f);
            const float rim
                = clampf(0.5f - (d_px - kRim * to_px) / kAA, 0.0f, 1.0f);
            // Ink over a white underlay, straight alpha out — the
            // same convention the .cur loader and shadow baker speak.
            const float oa = body + rim * (1.0f - body);
            if (oa <= 0.0f)
                continue;
            const auto ch = [&](std::uint32_t ink) {
                const float c
                    = (ink * body + 255.0f * rim * (1.0f - body)) / oa;
                return static_cast<std::uint32_t>(
                    clampf(c + 0.5f, 0.0f, 255.0f));
            };
            art.px[static_cast<std::size_t>(y) * size + x]
                = (static_cast<std::uint32_t>(oa * 255.0f + 0.5f) << 24)
                | (ch(ink_r) << 16) | (ch(ink_g) << 8) | ch(ink_b);
        }
    }

    art.hot_x = static_cast<int>(shape.hot.x * to_px + 0.5f);
    art.hot_y = static_cast<int>(shape.hot.y * to_px + 0.5f);
    return art;
}

}
