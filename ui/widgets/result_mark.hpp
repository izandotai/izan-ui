#pragma once

namespace izan::ui {

// The verdict glyph: a filled circle on the center axis carrying a
// tick (accent) or a cross (danger). The end of a flow deserves one
// clear mark, not a line of text alone.
void kit_result_mark(bool ok, float size_em = 3.0f);

}
