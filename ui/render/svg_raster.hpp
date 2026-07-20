#pragma once

#include <cstdint>
#include <vector>

namespace izan::render {

// One rasterized SVG: straight-alpha RGBA bytes (r,g,b,a per pixel,
// row-major top-down) — the layout OpenGL uploads as GL_RGBA and the
// proof tooling composites directly.
struct SvgBitmap {
    std::vector<std::uint8_t> rgba;
    int w = 0;
    int h = 0;

    bool empty() const
    {
        return rgba.empty();
    }
};

// Parses SVG text and rasterizes it to fit a px-square box, aspect
// preserved and centered. Returns an empty bitmap on parse failure.
SvgBitmap raster_svg(const char* svg_text, int px);

// Cover semantics for backdrops: fills the whole w×h box, aspect
// preserved, overflow cropped equally — a wallpaper never letterboxes.
SvgBitmap raster_svg_cover(const char* svg_text, int w, int h);

}
