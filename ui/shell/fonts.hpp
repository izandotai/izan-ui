#pragma once

#include <imgui.h>

#include <filesystem>

namespace izan::ui {

// LXGW WenKai GB Lite (OFL) as the primary face, Windows seguiemj
// merged in for color emoji at runtime — the system font is loaded
// from the user's machine, never redistributed.
//
// kDefaultFontSize was approved on a 150% Windows scale monitor;
// that approval is the design reference. The shell converts to the
// running monitor: pixel size = size * content_scale / design_scale
// — identical pixels at 150%, the same physical look everywhere
// else, native-rastered at every scale (no DPI-virtualized blur).
inline constexpr float kDefaultFontSize = 28.0f;
inline constexpr float kFontDesignScale = 1.5f;
inline constexpr const char* kDefaultFontRelativePath
    = "assets/fonts/LXGWWenKaiGBLite-Regular.ttf";
inline constexpr const char* kDefaultFontFaceName = "LXGW WenKai GB Lite";
inline constexpr const char* kEmojiFontPath = "C:/Windows/Fonts/seguiemj.ttf";

// The typeface is the host's voice, not the shell's: every path and
// size here is the host's to override. The defaults reproduce the
// izan look; the glyph waterfall behind them (invisibles → emoji →
// primary-face fallback → symbol face) is shell machinery and stays.
struct FontOptions {
    // Primary face. A relative path resolves against the executable
    // directory, then the working directory. All strings must outlive
    // the app (literals, in practice).
    const char* primary_path = kDefaultFontRelativePath;
    float size = kDefaultFontSize;
    // The face name as Windows knows it — the IME composition window
    // is styled through GDI and can't read a ttf path.
    const char* face_name = kDefaultFontFaceName;
    // Color-emoji merge; nullptr skips it. The IZAN_EMOJI_FONT
    // environment variable still outranks this for experiments.
    const char* emoji_path = kEmojiFontPath;
    // Glyph raster density. Layout metrics do not change — only how
    // finely the atlas rasterizes. <= 0 means "read the window's
    // content scale", which keeps text crisp on high-DPI monitors.
    float rasterizer_density = 0.0f;
};

std::filesystem::path executable_dir();
// The Windows accessibility text scale (Settings → Accessibility →
// Text size), 1.0 elsewhere or when unset. Multiplies the monitor
// content scale so the shell honors both knobs a user can turn.
float system_text_scale();
void load_font(ImGuiIO& io, const FontOptions& options);
void load_default_font(ImGuiIO& io);
// Whatever load_font last applied — the IME composition window and
// any other GDI-side styling read the host's choice from here
// instead of hardcoding the shell's default.
const FontOptions& active_font();

}
