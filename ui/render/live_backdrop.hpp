#pragma once

#include <imgui.h>

namespace izan::render {

// The animated wallpaper: an aurora drifting in a fragment shader,
// drawn as one quad through the same callback machinery as sdf_rect.
// GPU cost is a fraction of a millisecond; there are no assets and no
// codecs — the picture is arithmetic. Returns false when the shader
// path is unavailable (the caller keeps its static wallpaper).
bool live_backdrop(ImDrawList* draw, ImVec2 min, ImVec2 max, float time_s);

}
