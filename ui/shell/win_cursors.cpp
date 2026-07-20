// Custom mouse cursors. LoadImageW loads .cur/.ani (the system plays
// .ani animation on its own), ImGui hands over shape control via
// NoMouseCursorChange, and WM_SETCURSOR plus a per-frame fallback do
// the actual SetCursor. The cursor files live beside the exe in
// assets/cursors/; if the directory is absent the system cursors stay.
// The registry and system settings are never touched.
#include "ui/shell/win_chrome.hpp"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#if !defined(__APPLE__) && defined(_WIN32)
#include <GL/gl.h>
#endif

#include <imgui.h>

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <vector>

namespace izan::ui {

namespace {
#ifdef _WIN32
    HCURSOR g_custom[16] {}; // by ImGuiMouseCursor slot, with headroom

    // Bakes a drop shadow into a cursor. System cursor shadows are a
    // renderer feature outside program control, so baking one into the
    // bitmap keeps the look consistent wherever the files travel.
    // Pipeline: GetIconInfo pulls 32bpp ARGB → canvas grows toward
    // bottom-right → shadow = alpha silhouette, two box-blur passes
    // (≈ gaussian), offset (2,2), black at 38% → source over shadow →
    // CreateIconIndirect with the original hotspot. Every failure path
    // silently returns the original (animated .ani frames included).
    HCURSOR bake_shadow(HCURSOR src)
    {
        ICONINFO ii {};
        if (!GetIconInfo(src, &ii))
            return src;

        struct BmpGuard {
            HBITMAP a, b;

            ~BmpGuard()
            {
                if (a)
                    DeleteObject(a);
                if (b)
                    DeleteObject(b);
            }
        } guard { ii.hbmColor, ii.hbmMask };

        if (!ii.hbmColor)
            return src; // monochrome cursor, leave it be
        BITMAP bm {};
        if (!GetObject(ii.hbmColor, sizeof bm, &bm) || bm.bmWidth <= 0
            || bm.bmHeight <= 0)
            return src;
        const int w = bm.bmWidth, h = bm.bmHeight;
        constexpr int kOff = 2, kBlur = 2;
        const int pad = kOff + kBlur * 2; // shadow bleeds bottom-right only
        const int W = w + pad, H = h + pad;

        HDC dc = CreateCompatibleDC(nullptr);
        BITMAPINFO bi {};
        bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bi.bmiHeader.biWidth = w;
        bi.bmiHeader.biHeight = -h; // top-down
        bi.bmiHeader.biPlanes = 1;
        bi.bmiHeader.biBitCount = 32;
        bi.bmiHeader.biCompression = BI_RGB;
        std::vector<uint32_t> srcPx(static_cast<size_t>(w) * h);
        if (GetDIBits(dc, ii.hbmColor, 0, h, srcPx.data(), &bi, DIB_RGB_COLORS)
            != h) {
            DeleteDC(dc);
            return src;
        }
        bool has_alpha = false;
        for (uint32_t p : srcPx) {
            if (p >> 24) {
                has_alpha = true;
                break;
            }
        }
        if (!has_alpha) {
            DeleteDC(dc);
            return src; // legacy mask cursor without an alpha channel
        }

        std::vector<float> acc(static_cast<size_t>(W) * H, 0.0f);
        std::vector<float> tmp(static_cast<size_t>(W) * H, 0.0f);
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x)
                acc[static_cast<size_t>(y + kOff) * W + (x + kOff)]
                    = float(srcPx[static_cast<size_t>(y) * w + x] >> 24);
        const auto box
            = [&](std::vector<float>& in, std::vector<float>& out, bool horiz) {
                  for (int y = 0; y < H; ++y)
                      for (int x = 0; x < W; ++x) {
                          float s = 0;
                          int n = 0;
                          for (int k = -kBlur; k <= kBlur; ++k) {
                              const int xx = horiz ? x + k : x;
                              const int yy = horiz ? y : y + k;
                              if (xx < 0 || xx >= W || yy < 0 || yy >= H)
                                  continue;
                              s += in[static_cast<size_t>(yy) * W + xx];
                              ++n;
                          }
                          out[static_cast<size_t>(y) * W + x] = s / float(n);
                      }
              };
        box(acc, tmp, true);
        box(tmp, acc, false);
        box(acc, tmp, true);
        box(tmp, acc, false);

        // Composite with straight alpha: out = source over black shadow.
        std::vector<uint32_t> outPx(static_cast<size_t>(W) * H, 0);
        for (int y = 0; y < H; ++y)
            for (int x = 0; x < W; ++x) {
                const float sa
                    = std::min(
                          255.0f, acc[static_cast<size_t>(y) * W + x] * 0.38f)
                    / 255.0f;
                const uint32_t s = (x < w && y < h)
                    ? srcPx[static_cast<size_t>(y) * w + x]
                    : 0u;
                const float fa = float(s >> 24) / 255.0f;
                const float oa = fa + sa * (1.0f - fa);
                if (oa <= 0.0f)
                    continue;
                const auto ch = [&](int shift) {
                    const float c = float((s >> shift) & 0xff) * fa;
                    return uint32_t(std::min(255.0f, c / oa + 0.5f));
                };
                outPx[static_cast<size_t>(y) * W + x]
                    = (uint32_t(oa * 255.0f + 0.5f) << 24) | (ch(16) << 16)
                    | (ch(8) << 8) | ch(0);
            }

        BITMAPINFO bo {};
        bo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bo.bmiHeader.biWidth = W;
        bo.bmiHeader.biHeight = -H;
        bo.bmiHeader.biPlanes = 1;
        bo.bmiHeader.biBitCount = 32;
        bo.bmiHeader.biCompression = BI_RGB;
        void* bits = nullptr;
        HBITMAP color
            = CreateDIBSection(dc, &bo, DIB_RGB_COLORS, &bits, nullptr, 0);
        if (!color || !bits) {
            if (color)
                DeleteObject(color);
            DeleteDC(dc);
            return src;
        }
        std::memcpy(bits, outPx.data(), outPx.size() * 4);
        HBITMAP mask = CreateBitmap(W, H, 1, 1, nullptr);
        ICONINFO ni {};
        ni.fIcon = FALSE;
        ni.xHotspot = ii.xHotspot;
        ni.yHotspot = ii.yHotspot;
        ni.hbmColor = color;
        ni.hbmMask = mask;
        HCURSOR out = static_cast<HCURSOR>(CreateIconIndirect(&ni));
        DeleteObject(color);
        DeleteObject(mask);
        DeleteDC(dc);
        if (!out)
            return src;
        DestroyCursor(src); // LoadImage handles are unshared; retire it
        return out;
    }
#endif
    bool g_active = false;
    bool g_inframe = false; // drag in flight: hardware cursor stands down
}

bool custom_cursors_active()
{
    return g_active;
}

bool install_custom_cursors(const std::filesystem::path& dir)
{
#ifdef _WIN32
    struct Entry {
        int slot;
        const wchar_t* file;
    };

    const Entry entries[] = {
        { ImGuiMouseCursor_Arrow, L"Arrow.cur" },
        { ImGuiMouseCursor_TextInput, L"Text.cur" },
        { ImGuiMouseCursor_ResizeAll, L"Move.cur" },
        { ImGuiMouseCursor_ResizeNS, L"VResize.cur" },
        { ImGuiMouseCursor_ResizeEW, L"HResize.cur" },
        { ImGuiMouseCursor_ResizeNESW, L"D2Resize.cur" },
        { ImGuiMouseCursor_ResizeNWSE, L"D1Resize.cur" },
        { ImGuiMouseCursor_Hand, L"Link.cur" },
        { ImGuiMouseCursor_NotAllowed, L"Unavailable.cur" },
    };
    for (const Entry& e : entries) {
        const std::filesystem::path p = dir / e.file;
        std::error_code ec;
        if (!std::filesystem::exists(p, ec))
            continue;
        if (HANDLE h = LoadImageW(nullptr, p.wstring().c_str(), IMAGE_CURSOR, 0,
                0, LR_LOADFROMFILE | LR_DEFAULTSIZE))
            g_custom[e.slot] = bake_shadow(static_cast<HCURSOR>(h));
    }
    if (g_custom[ImGuiMouseCursor_Arrow] == nullptr)
        return false; // no arrow, no takeover
    g_active = true;
    // ImGui/GLFW stand down: cursor shape is this module's job now.
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    return true;
#else
    (void)dir;
    return false;
#endif
}

namespace {

#ifdef _WIN32
    // The arrow rendered to a texture, hotspot and all — the in-frame
    // cursor's pixels. Extracted once from whichever arrow is live:
    // the loaded pack's (shadow already baked) or the system's.
    struct CursorTex {
        ImTextureID tex = 0;
        ImVec2 hot {};
        ImVec2 size {};
        bool tried = false;
    };

    CursorTex g_arrow_tex;

    const CursorTex& arrow_texture()
    {
        CursorTex& ct = g_arrow_tex;
        if (ct.tried)
            return ct;
        ct.tried = true;
        HCURSOR src = g_custom[ImGuiMouseCursor_Arrow];
        if (src == nullptr)
            src = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512) /*IDC_ARROW*/);
        if (src == nullptr)
            return ct;
        ICONINFO ii {};
        if (!GetIconInfo(src, &ii))
            return ct;

        struct BmpGuard {
            HBITMAP a, b;

            ~BmpGuard()
            {
                if (a)
                    DeleteObject(a);
                if (b)
                    DeleteObject(b);
            }
        } guard { ii.hbmColor, ii.hbmMask };

        if (!ii.hbmColor)
            return ct; // monochrome cursor: no alpha to lift
        BITMAP bm {};
        if (!GetObject(ii.hbmColor, sizeof bm, &bm) || bm.bmWidth <= 0
            || bm.bmHeight <= 0)
            return ct;
        const int w = bm.bmWidth, h = bm.bmHeight;
        HDC dc = CreateCompatibleDC(nullptr);
        BITMAPINFO bi {};
        bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bi.bmiHeader.biWidth = w;
        bi.bmiHeader.biHeight = -h; // top-down
        bi.bmiHeader.biPlanes = 1;
        bi.bmiHeader.biBitCount = 32;
        bi.bmiHeader.biCompression = BI_RGB;
        std::vector<uint32_t> px(static_cast<size_t>(w) * h);
        const int got
            = GetDIBits(dc, ii.hbmColor, 0, h, px.data(), &bi, DIB_RGB_COLORS);
        DeleteDC(dc);
        if (got != h)
            return ct;
        // BGRA → RGBA for GL, straight alpha as imgui blends it.
        for (uint32_t& p : px)
            p = (p & 0xff00ff00u) | ((p & 0xffu) << 16) | ((p >> 16) & 0xffu);
        unsigned tex = 0;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, 0x8058 /* GL_RGBA8 */, w, h, 0, GL_RGBA,
            GL_UNSIGNED_BYTE, px.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        ct.tex = static_cast<ImTextureID>(tex);
        ct.hot = { static_cast<float>(ii.xHotspot),
            static_cast<float>(ii.yHotspot) };
        ct.size = { static_cast<float>(w), static_cast<float>(h) };
        return ct;
    }
#endif

}

void draw_inframe_cursor(bool active)
{
#ifdef _WIN32
    if (!active) {
        g_inframe = false;
        return;
    }
    const CursorTex& ct = arrow_texture();
    if (ct.tex == 0) {
        g_inframe = false; // no pixels to show: keep the hardware cursor
        return;
    }
    g_inframe = true;
    if (!g_active) {
        // No custom pack: the imgui/glfw backend owns the shape —
        // asking for None makes it hide the hardware cursor properly.
        ImGui::SetMouseCursor(ImGuiMouseCursor_None);
    }
    const ImVec2 mouse = ImGui::GetIO().MousePos;
    const ImVec2 p0 { mouse.x - ct.hot.x, mouse.y - ct.hot.y };
    const ImVec2 p1 { p0.x + ct.size.x, p0.y + ct.size.y };
    ImGui::GetForegroundDrawList()->AddImage(ct.tex, p0, p1);
#else
    (void)active;
#endif
}

void apply_custom_cursor_slot(int imgui_cursor)
{
#ifdef _WIN32
    if (!g_active)
        return;
    if (imgui_cursor == ImGuiMouseCursor_None) {
        ::SetCursor(nullptr);
        return;
    }
    HCURSOR h = nullptr;
    if (imgui_cursor >= 0 && imgui_cursor < 16)
        h = g_custom[imgui_cursor];
    if (h == nullptr)
        h = g_custom[ImGuiMouseCursor_Arrow];
    ::SetCursor(h);
#else
    (void)imgui_cursor;
#endif
}

void apply_custom_cursor()
{
    if (!g_active || ImGui::GetCurrentContext() == nullptr)
        return;
    // A drag in flight: the in-frame cursor owns the pixels, the
    // hardware cursor stays down (this path also serves WM_SETCURSOR).
    if (g_inframe) {
        apply_custom_cursor_slot(ImGuiMouseCursor_None);
        return;
    }
#ifdef _WIN32
    // Only override while the mouse is truly in the client area. The
    // borders and caption belong to WM_SETCURSOR — otherwise the render
    // frame keeps stomping the resize cursor with a stale client-area
    // arrow, flickering once per frame.
    HWND hwnd = GetActiveWindow();
    if (hwnd == nullptr)
        return;
    POINT pt {};
    GetCursorPos(&pt);
    const LRESULT hit
        = SendMessageW(hwnd, WM_NCHITTEST, 0, MAKELPARAM(pt.x, pt.y));
    if (hit != HTCLIENT)
        return;
#endif
    apply_custom_cursor_slot(ImGui::GetMouseCursor());
}

}
