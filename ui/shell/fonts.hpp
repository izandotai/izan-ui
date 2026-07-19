#pragma once

#include <imgui.h>

#include <filesystem>

namespace izan::ui {

// LXGW WenKai GB Lite (OFL) as the primary face, Windows seguiemj
// merged in for color emoji at runtime — the system font is loaded
// from the user's machine, never redistributed.
inline constexpr float kDefaultFontSize = 28.0f;
inline constexpr const char* kDefaultFontRelativePath
    = "assets/fonts/LXGWWenKaiGBLite-Regular.ttf";
inline constexpr const char* kDefaultFontFaceName = "LXGW WenKai GB Lite";
inline constexpr const char* kEmojiFontPath = "C:/Windows/Fonts/seguiemj.ttf";

std::filesystem::path executable_dir();
void load_default_font(ImGuiIO& io);

}
