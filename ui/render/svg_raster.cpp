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

SvgBitmap raster_svg(const char* svg_text, int px)
{
    if (svg_text == nullptr || px <= 0)
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
    out.w = px;
    out.h = px;
    out.rgba.assign(static_cast<std::size_t>(px) * px * 4, 0);
    NSVGrasterizer* rast = nsvgCreateRasterizer();
    if (rast != nullptr) {
        const float scale
            = static_cast<float>(px) / std::max(image->width, image->height);
        const float tx = (px - image->width * scale) * 0.5f;
        const float ty = (px - image->height * scale) * 0.5f;
        nsvgRasterize(
            rast, image, tx, ty, scale, out.rgba.data(), px, px, px * 4);
        nsvgDeleteRasterizer(rast);
    } else {
        out = {};
    }
    nsvgDelete(image);
    return out;
}

}
