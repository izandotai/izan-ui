#pragma once

namespace izan::ui {

// Data tables in the kit's voice: caption-weight headers, hairline row
// separators, no chrome. Wraps imgui tables — columns and rows work as
// usual between begin and end.
bool kit_table_begin(const char* id, int columns);
void kit_table_headers(const char* const* names, int count);
void kit_table_end();

}
