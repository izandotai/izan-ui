// Photos join the wallpaper roads: stb_image decodes, the caller
// uploads. Kept apart from svg_raster so neither implementation macro
// leaks into the other.
#include "ui/render/image_load.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_ONLY_BMP
#include <stb_image.h>

namespace izan::render {

SvgBitmap load_image_rgba(const char* path)
{
    int w = 0, h = 0, comp = 0;
    unsigned char* px = stbi_load(path, &w, &h, &comp, 4);
    if (px == nullptr || w <= 0 || h <= 0) {
        if (px != nullptr)
            stbi_image_free(px);
        return {};
    }
    SvgBitmap out;
    out.w = w;
    out.h = h;
    out.rgba.assign(px, px + static_cast<std::size_t>(w) * h * 4);
    stbi_image_free(px);
    return out;
}

}
