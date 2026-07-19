#pragma once

namespace izan::ui {

// A -mwindows GUI binary started from a terminal reattaches to the
// parent console so prints land somewhere visible.
bool try_attach_parent_console();

}
