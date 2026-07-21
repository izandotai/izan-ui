#pragma once

#include <imgui.h>

// Mint-mannered scrollbars for the whole surface: the stock imgui
// bars go invisible (their gutter and input logic stay), and a
// frame-end pass repaints every live scrollbar as an analytic pill —
// slim at rest, widening under the cursor, a faint track only while
// touched.
namespace izan::ui {

// Zero the four stock scrollbar colors on the current style. Cheap
// and idempotent — a per-frame host may call it after any theme push.
void kit_scrollbars_stock_hide();

// Repaint every window's visible scrollbars, children included, in
// the given ink (state alpha applied on top). Call once at the end of
// the frame, after all windows have drawn. The ink comes from the
// host's theme — never from style Text, which may sit at the opposite
// polarity of the glass actually painted (the white-on-white lesson).
void kit_skin_scrollbars(ImVec4 ink);

}
