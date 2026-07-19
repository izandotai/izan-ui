#pragma once

#include <cstddef>
#include <functional>

namespace izan::ui {

// The address field: a text entry that knows it holds a crypto
// address. Empty, it wears a one-click paste glyph; filled, a
// one-click clear; right-click serves paste / copy / clear. Labels
// arrive from the caller — the kit speaks no language of its own.
//
// The caller may hand over a validator; a paste that fails it is
// refused outright (the field flashes its border in the danger color)
// — garbage on the clipboard must never come to rest in an address
// slot. Typing is not policed here: live keystrokes are the page's
// business. Returns true on Enter.
bool kit_address_field(const char* id, const char* hint, char* buf,
    std::size_t size, const char* paste_label, const char* copy_label,
    const char* clear_label,
    const std::function<bool(const char*)>& validate = nullptr);

}
