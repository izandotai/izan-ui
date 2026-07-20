#pragma once

#include <cstdint>
#include <vector>

namespace izan::ui {

// One procedurally painted cursor: straight-alpha ARGB rows, top-down,
// hotspot in pixels. Empty px means the slot has no built-in design.
struct CursorArt {
    std::vector<std::uint32_t> px;
    int w = 0;
    int h = 0;
    int hot_x = 0;
    int hot_y = 0;
};

// The built-in cursor set, painted analytically at the given pixel
// size (square). Original geometry in the family's ink-and-rim style —
// no cursor pack's artwork is traced, so the set ships inside the exe
// with nothing to license. The slot is an ImGuiMouseCursor value.
CursorArt paint_cursor(int imgui_slot, int size);

}
