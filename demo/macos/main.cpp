#include "macos_desktop.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#if defined(__APPLE__)
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif
#include <GLFW/glfw3.h>

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace {

void glfw_error_callback(int error, const char* description)
{
    std::fprintf(stderr, "GLFW error %d: %s\n", error, description);
}

std::filesystem::path executable_directory(const char* argv0)
{
#if defined(_WIN32)
    (void)argv0;
    std::wstring path(32768, L'\0');
    const DWORD length = GetModuleFileNameW(
        nullptr, path.data(), static_cast<DWORD>(path.size()));
    path.resize(length);
    return std::filesystem::path(path).parent_path();
#else
    std::error_code error;
    auto path = std::filesystem::canonical(argv0, error);
    return (error ? std::filesystem::absolute(argv0) : path).parent_path();
#endif
}

ImFont* load_font(ImGuiIO& io, const char* argv0)
{
    // The family face, delivered next to the exe by izan_ui_copy_assets.
    const auto font_path = executable_directory(argv0) / "assets" / "fonts"
        / "LXGWWenKaiGBLite-Regular.ttf";

    if (!std::filesystem::exists(font_path)) {
        std::fprintf(
            stderr, "Font not found: %s\n", font_path.string().c_str());
        return io.Fonts->AddFontDefault();
    }

    ImFontConfig config;
    config.OversampleH = 2;
    config.OversampleV = 2;
    config.PixelSnapH = false;
    config.RasterizerDensity = 1.25f;
    return io.Fonts->AddFontFromFileTTF(font_path.string().c_str(), 17.0f,
        &config, io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
}

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

bool capture_framebuffer(
    const std::filesystem::path& output, int width, int height)
{
    const int row_size = (width * 3 + 3) & ~3;
    std::vector<unsigned char> pixels(
        static_cast<std::size_t>(row_size) * height);
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glReadBuffer(GL_BACK);
    glReadPixels(0, 0, width, height, 0x80E0 /* GL_BGR */, GL_UNSIGNED_BYTE,
        pixels.data());

    BitmapFileHeader file_header;
    BitmapInfoHeader info_header;
    info_header.width = width;
    info_header.height
        = height; // OpenGL and BMP both store the bottom row first.
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

} // namespace

int main(int argc, char** argv)
{
    std::filesystem::path screenshot_path;
    for (int argument = 1; argument + 1 < argc; ++argument) {
        if (std::string_view(argv[argument]) == "--screenshot") {
            screenshot_path = argv[argument + 1];
        }
    }

    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        return 1;
    }

#if defined(__APPLE__)
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#else
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    const int width = mode ? std::min(1440, mode->width - 80) : 1440;
    const int height = mode ? std::min(900, mode->height - 80) : 900;
    GLFWwindow* window = glfwCreateWindow(
        width, height, "macOS · Dear ImGui", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return 1;
    }
    if (mode) {
        glfwSetWindowPos(
            window, (mode->width - width) / 2, (mode->height - height) / 2);
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    io.FontDefault = load_font(io, argc > 0 ? argv[0] : "macos_imgui");

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.AntiAliasedLines = true;
    style.AntiAliasedLinesUseTex = true;
    style.AntiAliasedFill = true;
    style.WindowRounding = 12.0f;
    style.FrameRounding = 8.0f;
    style.PopupRounding = 12.0f;
    style.ScrollbarSize = 8.0f;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    MacOSDesktop desktop;
    int rendered_frames = 0;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
            glfwWaitEventsTimeout(0.1);
            continue;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        desktop.render();

        ImGui::Render();
        int display_width = 0;
        int display_height = 0;
        glfwGetFramebufferSize(window, &display_width, &display_height);
        glViewport(0, 0, display_width, display_height);
        glClearColor(0.025f, 0.04f, 0.09f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        if (!screenshot_path.empty() && ++rendered_frames >= 3) {
            if (!capture_framebuffer(
                    screenshot_path, display_width, display_height)) {
                std::fprintf(stderr, "Unable to save screenshot: %s\n",
                    screenshot_path.string().c_str());
            }
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
