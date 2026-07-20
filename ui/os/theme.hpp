#pragma once

#include <imgui.h>

#include "ui/os/app.hpp"

// The mechanism/presentation split that makes the shell portable
// across desktop cultures. The kernel (Wm) owns state and input and
// never paints; a Theme owns every pixel and every metric. Another
// look is a new implementation of this interface, the kernel
// untouched: window buttons may move, metrics may change, and the
// kernel only ever asks "where are the controls" and "paint this".

namespace izan::os {

struct OsRect {
    ImVec2 min {};
    ImVec2 max {};

    bool contains(ImVec2 p) const
    {
        return p.x >= min.x && p.y >= min.y && p.x <= max.x && p.y <= max.y;
    }
};

enum class WindowControl { Close = 0, Minimize = 1, Maximize = 2 };

// What a theme needs to know to paint one window.
struct WindowLook {
    const App* app = nullptr;
    bool focused = false;
    bool maximized = false;
    // True only for the one window directly under the cursor while
    // no drag or resize is in flight - hover feedback anywhere else
    // is a lie.
    bool hover_ok = false;
};

class Theme {
public:
    virtual ~Theme() = default;

    virtual float title_height(float em) const = 0;

    // Where a control lives; the kernel hit-tests these and excludes
    // them from the drag zone. A theme without some control returns
    // an empty rect.
    virtual OsRect control_rect(
        WindowControl control, ImVec2 rmin, ImVec2 rmax, float em) const
        = 0;

    // The whole chrome: shadow, body, title strip, controls, title
    // text, resize grip. Content is painted by the app afterwards.
    virtual void paint_window(ImDrawList* draw, const WindowLook& look,
        ImVec2 rmin, ImVec2 rmax, float em) const
        = 0;

    virtual void paint_wallpaper(
        ImDrawList* draw, ImVec2 min, ImVec2 max, float em) const
        = 0;
};

// The working development skin: Mint-cultured — bottom panel,
// right-hand window controls, geometric wallpaper.
const Theme& mint_theme();

}
