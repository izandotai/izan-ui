// SVG file → GL texture, cached. Rasterization is nanosvg through
// svg_raster; the upload is plain GL 1.1, which izan_imgui already
// links everywhere this library goes.
#include "ui/render/svg_icon.hpp"

#include <fstream>
#include <map>
#include <sstream>
#include <string>

#if !defined(__APPLE__) && defined(_WIN32)
#include <GL/gl.h>
#endif

#include "ui/render/svg_raster.hpp"

namespace izan::render {

ImTextureID svg_icon(const std::filesystem::path& file, int px)
{
#if !defined(__APPLE__) && defined(_WIN32)
    static std::map<std::pair<std::string, int>, ImTextureID> cache;
    const auto key = std::make_pair(file.string(), px);
    if (const auto it = cache.find(key); it != cache.end())
        return it->second;

    ImTextureID tex = 0;
    std::ifstream in(file, std::ios::binary);
    if (in) {
        std::stringstream text;
        text << in.rdbuf();
        const SvgBitmap art = raster_svg(text.str().c_str(), px);
        if (!art.empty()) {
            unsigned id = 0;
            glGenTextures(1, &id);
            glBindTexture(GL_TEXTURE_2D, id);
            glTexImage2D(GL_TEXTURE_2D, 0, 0x8058 /* GL_RGBA8 */, art.w, art.h,
                0, GL_RGBA, GL_UNSIGNED_BYTE, art.rgba.data());
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            tex = static_cast<ImTextureID>(id);
        }
    }
    cache.emplace(key, tex); // failures cache too: no per-frame retry storm
    return tex;
#else
    (void)file;
    (void)px;
    return 0;
#endif
}

}
