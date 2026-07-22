// Photos join the wallpaper roads: stb_image decodes, the caller
// uploads. Kept apart from svg_raster so neither implementation macro
// leaks into the other.
#include "ui/render/image_load.hpp"

#include <limits>

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

SvgBitmap load_image_rgba_memory(std::span<const std::uint8_t> bytes,
    int max_edge, std::size_t max_pixels)
{
    if (bytes.empty()
        || bytes.size() > static_cast<std::size_t>(std::numeric_limits<int>::max())
        || max_edge <= 0 || max_pixels == 0)
        return {};
    const auto* data = reinterpret_cast<const stbi_uc*>(bytes.data());
    const int size = static_cast<int>(bytes.size());
    int w = 0, h = 0, comp = 0;
    if (!stbi_info_from_memory(data, size, &w, &h, &comp)
        || w <= 0 || h <= 0 || w > max_edge || h > max_edge
        || static_cast<std::size_t>(w) > max_pixels / static_cast<std::size_t>(h))
        return {};
    unsigned char* px = stbi_load_from_memory(data, size, &w, &h, &comp, 4);
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
