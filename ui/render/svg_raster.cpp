// SVG in, pixels out — nanosvg does the geometry, this wrapper only
// owns the memory and the fit-to-box math. The source of record for
// icon artwork is the Noto emoji SVG set (OFL, license shipped in
// ui/assets/noto); anything it lacks is drawn by hand as before.
#include "ui/render/svg_raster.hpp"

#define NANOSVG_IMPLEMENTATION
#define NANOSVGRAST_IMPLEMENTATION
#include <nanosvg.h>
#include <nanosvgrast.h>

#include <algorithm>
#include <cstring>

namespace izan::render {

namespace {

    SvgBitmap raster_impl(const char* svg_text, int w, int h, bool cover)
    {
        if (svg_text == nullptr || w <= 0 || h <= 0)
            return {};
        // nsvgParse writes into the buffer it is given; feed it a copy.
        std::vector<char> text(svg_text, svg_text + std::strlen(svg_text) + 1);
        NSVGimage* image = nsvgParse(text.data(), "px", 96.0f);
        if (image == nullptr)
            return {};
        if (image->width <= 0.0f || image->height <= 0.0f) {
            nsvgDelete(image);
            return {};
        }

        SvgBitmap out;
        out.w = w;
        out.h = h;
        out.rgba.assign(static_cast<std::size_t>(w) * h * 4, 0);
        NSVGrasterizer* rast = nsvgCreateRasterizer();
        if (rast != nullptr) {
            const float sx = static_cast<float>(w) / image->width;
            const float sy = static_cast<float>(h) / image->height;
            // contain picks the smaller scale (whole art visible);
            // cover the larger (whole box filled, overflow cropped)
            const float scale = cover ? std::max(sx, sy) : std::min(sx, sy);
            const float tx = (w - image->width * scale) * 0.5f;
            const float ty = (h - image->height * scale) * 0.5f;
            nsvgRasterize(
                rast, image, tx, ty, scale, out.rgba.data(), w, h, w * 4);
            nsvgDeleteRasterizer(rast);
        } else {
            out = {};
        }
        nsvgDelete(image);
        return out;
    }

}

SvgBitmap raster_svg(const char* svg_text, int px)
{
    return raster_impl(svg_text, px, px, false);
}

SvgBitmap raster_svg_cover(const char* svg_text, int w, int h)
{
    return raster_impl(svg_text, w, h, true);
}

}
