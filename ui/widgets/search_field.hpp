#pragma once

#include <cstddef>

namespace izan::ui {

// A filter box: the field dress, a hint while empty, and an embedded
// clear cross once there is anything to clear — for lists that narrow
// as you type.
void kit_search_field(
    const char* id, const char* hint, char* buf, std::size_t size);

}
