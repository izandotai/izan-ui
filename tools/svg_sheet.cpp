// Proof sheet for SVG icon rasterization: each input file at 16, 24,
// 32 and 64 px, over paper and over ink, one BMP. Usage:
//   izan_svg_sheet out.bmp icon1.svg icon2.svg ...
#include "ui/render/svg_raster.hpp"

#include <cstdint>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace {

#pragma pack(push, 1)

struct FileHeader {
    std::uint16_t type = 0x4d42;
    std::uint32_t size = 0;
    std::uint16_t r1 = 0, r2 = 0;
    std::uint32_t pixel_offset = 54;
};

struct InfoHeader {
    std::uint32_t size = 40;
    std::int32_t width = 0, height = 0;
    std::uint16_t planes = 1, bpp = 24;
    std::uint32_t compression = 0, image_size = 0;
    std::int32_t xppm = 3780, yppm = 3780;
    std::uint32_t used = 0, important = 0;
};

#pragma pack(pop)

}

int main(int argc, char** argv)
{
    if (argc < 3) {
        std::fprintf(stderr, "usage: %s out.bmp icon.svg...\n", argv[0]);
        return 2;
    }
    const int sizes[] = { 16, 24, 32, 64 };
    const std::uint32_t grounds[] = { 0xf2f1ec, 0x2e3436 };
    constexpr int cell = 76;
    const int cols = 8; // 4 sizes x 2 grounds
    const int rows = argc - 2;
    const int W = cols * cell, H = rows * cell;

    std::vector<std::uint32_t> canvas(
        static_cast<std::size_t>(W) * H, grounds[0]);
    for (int row = 0; row < rows; ++row) {
        std::ifstream in(argv[row + 2], std::ios::binary);
        std::stringstream text;
        text << in.rdbuf();
        const std::string svg = text.str();
        for (int col = 0; col < cols; ++col) {
            const std::uint32_t ground = grounds[col & 1];
            for (int y = 0; y < cell; ++y)
                for (int x = 0; x < cell; ++x)
                    canvas[static_cast<std::size_t>(row * cell + y) * W
                        + col * cell + x]
                        = ground;
            const izan::render::SvgBitmap art
                = izan::render::raster_svg(svg.c_str(), sizes[col / 2]);
            if (art.empty()) {
                std::fprintf(stderr, "%s: parse failed\n", argv[row + 2]);
                continue;
            }
            const int ox = col * cell + (cell - art.w) / 2;
            const int oy = row * cell + (cell - art.h) / 2;
            for (int y = 0; y < art.h; ++y)
                for (int x = 0; x < art.w; ++x) {
                    const std::uint8_t* s
                        = &art.rgba[(static_cast<std::size_t>(y) * art.w + x)
                            * 4];
                    const float a = s[3] / 255.0f;
                    if (a <= 0.0f)
                        continue;
                    std::uint32_t& dst
                        = canvas[static_cast<std::size_t>(oy + y) * W + ox + x];
                    const auto mix = [&](int channel, int shift) {
                        const float dc = float((dst >> shift) & 0xff);
                        return std::uint32_t(
                                   s[channel] * a + dc * (1.0f - a) + 0.5f)
                            << shift;
                    };
                    dst = mix(0, 16) | mix(1, 8) | mix(2, 0);
                }
        }
    }

    FileHeader fh;
    InfoHeader ih;
    const int row_size = (W * 3 + 3) & ~3;
    ih.width = W;
    ih.height = H;
    ih.image_size = static_cast<std::uint32_t>(row_size) * H;
    fh.size = fh.pixel_offset + ih.image_size;
    std::ofstream f(argv[1], std::ios::binary);
    if (!f)
        return 1;
    f.write(reinterpret_cast<const char*>(&fh), sizeof fh);
    f.write(reinterpret_cast<const char*>(&ih), sizeof ih);
    std::vector<char> line(static_cast<std::size_t>(row_size), 0);
    for (int y = H - 1; y >= 0; --y) { // BMP rows run bottom-up
        for (int x = 0; x < W; ++x) {
            const std::uint32_t c = canvas[static_cast<std::size_t>(y) * W + x];
            line[static_cast<std::size_t>(x) * 3 + 0] = char(c & 0xff);
            line[static_cast<std::size_t>(x) * 3 + 1] = char((c >> 8) & 0xff);
            line[static_cast<std::size_t>(x) * 3 + 2] = char((c >> 16) & 0xff);
        }
        f.write(line.data(), row_size);
    }
    std::printf("wrote %s (%dx%d)\n", argv[1], W, H);
    return f.good() ? 0 : 1;
}
