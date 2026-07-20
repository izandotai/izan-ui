#pragma once

#include "ui/render/svg_raster.hpp"

namespace izan::render {

// A photo decoded to the same RGBA bitmap the SVG road produces —
// jpg/png/bmp via stb_image. Empty on failure.
SvgBitmap load_image_rgba(const char* path);

}
