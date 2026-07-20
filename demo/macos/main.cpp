// The macOS replica inside the izan shell: family window frame and
// title bar around the ImDrawList desktop, family typeface at the
// family size. --screenshot <file.bmp> renders a few frames and
// exits with a headless capture.

#include "desktop.hpp"

#include <GLFW/glfw3.h>

#if !defined(__APPLE__)
#include <GL/gl.h>
#endif

#include <imgui.h>

#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string_view>
#include <vector>

#include "ui/shell/app.hpp"
#include "ui/shell/chrome_state.hpp"
#include "ui/shell/chrome_widgets.hpp"
#include "ui/shell/constants.hpp"
#include "ui/shell/theme.hpp"
#include "ui/shell/win_chrome.hpp"

namespace {

namespace ui = izan::ui;

#pragma pack(push, 1)

struct BitmapFileHeader {
    std::uint16_t type = 0x4d42;
    std::uint32_t size = 0;
    std::uint16_t reserved1 = 0;
    std::uint16_t reserved2 = 0;
    std::uint32_t pixel_offset = 54;
};

struct BitmapInfoHeader {
    std::uint32_t size = 40;
    std::int32_t width = 0;
    std::int32_t height = 0;
    std::uint16_t planes = 1;
    std::uint16_t bits_per_pixel = 24;
    std::uint32_t compression = 0;
    std::uint32_t image_size = 0;
    std::int32_t x_pixels_per_meter = 3780;
    std::int32_t y_pixels_per_meter = 3780;
    std::uint32_t colors_used = 0;
    std::uint32_t colors_important = 0;
};

#pragma pack(pop)

// Reads the front buffer — called right after a swap, so the frame
// just presented is what lands in the file.
bool capture_front_buffer(
    const std::filesystem::path& output, int width, int height)
{
    const int row_size = (width * 3 + 3) & ~3;
    std::vector<unsigned char> pixels(
        static_cast<std::size_t>(row_size) * height);
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glReadBuffer(GL_FRONT);
    glReadPixels(0, 0, width, height, 0x80E0 /* GL_BGR */, GL_UNSIGNED_BYTE,
        pixels.data());

    BitmapFileHeader file_header;
    BitmapInfoHeader info_header;
    info_header.width = width;
    info_header.height = height;
    info_header.image_size = static_cast<std::uint32_t>(pixels.size());
    file_header.size = file_header.pixel_offset + info_header.image_size;

    std::ofstream stream(output, std::ios::binary);
    if (!stream) {
        return false;
    }
    stream.write(
        reinterpret_cast<const char*>(&file_header), sizeof(file_header));
    stream.write(
        reinterpret_cast<const char*>(&info_header), sizeof(info_header));
    stream.write(reinterpret_cast<const char*>(pixels.data()),
        static_cast<std::streamsize>(pixels.size()));
    return stream.good();
}

}

int main(int argc, char** argv)
{
    std::filesystem::path screenshot_path;
    for (int argument = 1; argument + 1 < argc; ++argument) {
        if (std::string_view(argv[argument]) == "--screenshot") {
            screenshot_path = argv[argument + 1];
        }
    }

    ui::GlfwApp app;
    ui::AppOptions opts;
    opts.title = "macos";
    opts.width = 1476;
    opts.height = 1000;
    if (!app.init(opts))
        return 1;
    ui::set_window_icon_resource(app.window(), 1);

    ui::ChromeState chrome;
    chrome.title_mark = "🖥️";
    ui::apply_theme_style_only(chrome.theme_index);
    glfwSetWindowOpacity(app.window(), chrome.window_opacity);

    macos::Desktop desktop;
    int rendered_frames = 0;
    app.set_render_callback([&] {
        app.begin_frame();

        if (ImGui::IsKeyPressed(ImGuiKey_Escape))
            glfwSetWindowShouldClose(app.window(), GLFW_TRUE);

        ui::draw_main_window_frame(chrome);
        ui::draw_custom_title_bar(
            app.window(), chrome, "macOS", "ImDrawList replica");

        const ImGuiViewport* vp = ImGui::GetMainViewport();
        const float top = ui::kWindowFrameMargin + ui::kTitleBarHeight;
        desktop.render(
            ImVec2(vp->Pos.x + ui::kWindowFrameMargin, vp->Pos.y + top),
            ImVec2(vp->Size.x - ui::kWindowFrameMargin * 2.0f,
                vp->Size.y - top - ui::kWindowFrameMargin));

        ui::draw_snap_layout_popup(app.window(), chrome);
        ui::draw_menu_popup_shadows(chrome);
        if (chrome.request_exit)
            glfwSetWindowShouldClose(app.window(), GLFW_TRUE);
        app.end_frame(ui::theme_clear_color(chrome));

        if (!screenshot_path.empty() && ++rendered_frames >= 3) {
            int display_width = 0;
            int display_height = 0;
            glfwGetFramebufferSize(
                app.window(), &display_width, &display_height);
            if (!capture_front_buffer(
                    screenshot_path, display_width, display_height)) {
                std::fprintf(stderr, "Unable to save screenshot: %s\n",
                    screenshot_path.string().c_str());
            }
            glfwSetWindowShouldClose(app.window(), GLFW_TRUE);
        }
    });
    app.run();
    app.shutdown();
    return 0;
}
