#include "ui/shell/app.hpp"

#include "ui/shell/fonts.hpp"
#include "ui/shell/theme.hpp"
#include "ui/shell/ui_layout.hpp"
#include "ui/shell/win_chrome.hpp"
#include "ui/shell/win_console.hpp"

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <GLFW/glfw3.h>

#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#endif

namespace izan::ui {

namespace {
    constexpr const char* kGlslVersion = "#version 130";
}

GlfwApp::~GlfwApp()
{
    shutdown();
}

bool GlfwApp::init(const AppOptions& options)
{
    if (options.attach_parent_console)
        try_attach_parent_console();
    lazy_ = options.lazy_redraw;

    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return false;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    // No MSAA by default: imgui anti-aliases its own geometry, and a
    // multisampled framebuffer only taxes fill rate and swap latency.
    // A host that wants hardware samples asks via options.
    glfwWindowHint(GLFW_SAMPLES, options.msaa_samples);

    window_ = glfwCreateWindow(
        options.width, options.height, options.title, nullptr, nullptr);
    if (window_ == nullptr) {
        glfwTerminate();
        return false;
    }

    // GLFW_SCALE_TO_MONITOR multiplies the requested size by the DPI
    // scale (at 150%, 1600×900 becomes 2400×1350) which can overflow
    // the work area; clamp to it after creation, then center.
    {
        int w = 0, h = 0;
        glfwGetWindowSize(window_, &w, &h);
        const WorkArea area = current_window_work_area(window_);
        const int max_w = static_cast<int>(area.width * 0.96);
        const int max_h = static_cast<int>(area.height * 0.94);
        if (w > max_w || h > max_h)
            glfwSetWindowSize(window_, std::min(w, max_w), std::min(h, max_h));
    }

    glfwMakeContextCurrent(window_);
    // Plain vsync: adaptive (-1) was tried and TEARS by design when a
    // frame is late - a borderless window on independent flip shows
    // it. DWM-synced swap is how native windows stay tear-free.
    glfwSwapInterval(1);
#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE 0x809D
#endif
    if (options.msaa_samples > 0)
        glEnable(GL_MULTISAMPLE); // some drivers ship with MSAA off
    install_custom_window_chrome(window_);
    // Centering must follow the chrome install: before it the
    // non-client area has not collapsed yet and the math lands the
    // window too high.
    center_window_on_work_area(window_);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    // No imgui ini file, ever: left at its default the library writes
    // imgui.ini into whatever cwd launched the app. A host that wants
    // layout persistence saves SaveIniSettingsToMemory into its own
    // settings file (izan does); a host that doesn't gets no stray
    // files.
    io.IniFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // The host speaks design sizes (authored at kFontDesignScale);
    // the shell converts to this machine's pixels — monitor DPI scale
    // times the Windows accessibility text scale — so the typeface
    // keeps one physical look across user setups.
    FontOptions fonts = options.fonts;
    float content_scale_x = 1.0f;
    float content_scale_y = 1.0f;
    glfwGetWindowContentScale(window_, &content_scale_x, &content_scale_y);
    const float ui_scale = content_scale_x * system_text_scale();
    fonts.size = fonts.size * ui_scale / kFontDesignScale;
    if (fonts.rasterizer_density <= 0.0f)
        fonts.rasterizer_density = ui_scale > 1.0f ? ui_scale : 1.0f;
    load_font(io, fonts);
    apply_style();
    ImGuiStyle& aa = ImGui::GetStyle();
    aa.AntiAliasedLines = true;
    aa.AntiAliasedLinesUseTex = true;
    aa.AntiAliasedFill = true;
    // Finer arc tessellation: rounded corners live or die by this —
    // at the default 0.30 a 12px radius shows its polygon.
    aa.CircleTessellationMaxError = 0.10f;
    // Custom cursors from assets/cursors beside the exe; absent files
    // silently keep the system cursors.
    install_custom_cursors(executable_dir() / "assets" / "cursors");

    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init(kGlslVersion);

    initialized_ = true;
    return true;
}

void GlfwApp::show()
{
    if (window_ != nullptr)
        glfwShowWindow(window_);
}

void GlfwApp::set_render_callback(std::function<void()> render)
{
    render_ = std::move(render);
    if (window_ == nullptr)
        return;
    glfwSetWindowUserPointer(window_, this);
    glfwSetWindowRefreshCallback(window_, [](GLFWwindow* w) {
        auto* self = static_cast<GlfwApp*>(glfwGetWindowUserPointer(w));
        if (self != nullptr && self->render_)
            self->render_();
    });
}

void GlfwApp::run()
{
    if (!render_)
        return;
    render_();
    show();
    while (!should_close()) {
        poll_events();
        render_();
    }
}

bool GlfwApp::should_close() const
{
    return window_ == nullptr || glfwWindowShouldClose(window_) != 0;
}

void GlfwApp::poll_events() const
{
    // Lazy mode parks on the event queue with a heartbeat timeout;
    // input wakes the loop instantly, idling costs almost nothing.
    if (lazy_)
        glfwWaitEventsTimeout(0.25);
    else
        glfwPollEvents();
}

void GlfwApp::begin_frame() const
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GlfwApp::end_frame(const ImVec4& clear_color) const
{
    ImGui::Render();

    int display_w = 0;
    int display_h = 0;
    glfwGetFramebufferSize(window_, &display_w, &display_h);
    if (display_w <= 0 || display_h <= 0)
        return;

    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Per-frame cursor fallback: WM_SETCURSOR does not fire when the
    // shape changes while the mouse sits still.
    apply_custom_cursor();

    glfwSwapBuffers(window_);
}

void GlfwApp::shutdown()
{
    if (!initialized_)
        return;
    initialized_ = false;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window_);
    window_ = nullptr;
    glfwTerminate();
}

}
