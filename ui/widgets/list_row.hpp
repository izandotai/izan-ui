#pragma once

namespace izan::ui {

// A source-list row: identity avatar, title over a muted subtitle, a
// trailing state dot, rounded selection highlight. The whole row is
// the hit target and also anchors a context menu. Returns true when
// clicked.
bool kit_list_row(const char* id, const char* title, const char* subtitle,
    bool selected, bool active_dot);

}
