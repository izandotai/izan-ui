#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include "ui/render/svg_raster.hpp"

namespace izan::render {

// A photo decoded to the same RGBA bitmap the SVG road produces —
// jpg/png/bmp via stb_image. Empty on failure.
SvgBitmap load_image_rgba(const char* path);

// The sandbox/resource road: decode jpg/png/bmp bytes already read by a
// trusted host. Inputs are rejected before allocation when dimensions exceed
// max_edge or max_pixels. This keeps a hostile package from turning a tiny
// compressed file into an unbounded texture allocation.
SvgBitmap load_image_rgba_memory(std::span<const std::uint8_t> bytes,
    int max_edge = 4096, std::size_t max_pixels = 16u * 1024u * 1024u);

}
