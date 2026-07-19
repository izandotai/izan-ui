#pragma once

#include <cstddef>

namespace izan::ui {

// The amount row: a framed field wearing the kit's field dress, digits
// in larger type on the left, and an embedded badge button on the
// right edge — a minted color swatch plus a short label; badge_hint
// serves the full story (symbol and chain) as a tooltip. Width comes
// from SetNextItemWidth. Returns true on Enter; *badge_clicked reports
// a press on the badge.
bool kit_amount_field(const char* id, char* buf, std::size_t size,
    const char* badge, bool* badge_clicked = nullptr,
    const char* badge_hint = nullptr);

}
