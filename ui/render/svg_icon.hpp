#pragma once

#include <filesystem>

#include <imgui.h>

namespace izan::render {

// An SVG file rasterized at the given pixel size and uploaded once —
// the texture door of the SVG road (the CPU door is svg_raster). The
// cache key is path+size; repeat calls are a map lookup. Returns 0
// when the file is missing or unparsable, so callers can keep a
// fallback glyph.
ImTextureID svg_icon(const std::filesystem::path& file, int px);

}
