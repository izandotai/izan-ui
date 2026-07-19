#pragma once

namespace izan::ui {

// A pick-one row for grouped cards: selection circle, label, a muted
// trailing caption (an address, a detail). The whole row is the hit
// target. Returns true when clicked.
bool kit_choice_row(const char* id, const char* label,
    const char* trailing_caption, bool selected);

// The bare selection circle (filled accent with a check when selected,
// a quiet ring otherwise) as its own small hit target — for rows whose
// body is built from other widgets. Returns true when clicked.
bool kit_selection_mark(const char* id, bool selected);

}
