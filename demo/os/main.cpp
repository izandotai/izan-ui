// The izan OS acceptance shell: the family chrome outside, the
// windowing kernel inside, three real apps on the dock — every
// window full of live kit widgets, never a scaled picture.
// --screenshot <file.bmp> renders a few frames and exits.

#include <GLFW/glfw3.h>

#if !defined(__APPLE__)
#include <GL/gl.h>
#endif

#include <imgui.h>

#include <array>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string_view>
#include <vector>

#include "ui/os/shell.hpp"
#include "ui/shell/app.hpp"
#include "ui/shell/chrome_state.hpp"
#include "ui/shell/chrome_widgets.hpp"
#include "ui/shell/constants.hpp"
#include "ui/shell/theme.hpp"
#include "ui/shell/win_chrome.hpp"
#include "ui/widgets/kit.hpp"

namespace {

namespace ui = izan::ui;
namespace os = izan::os;

// ---- the demo fleet ----

class GalleryApp final : public os::App {
public:
    const char* id() const override
    {
        return "gallery";
    }

    const char* name() const override
    {
        return "组件画廊";
    }

    const char* mark() const override
    {
        return "🧩";
    }

    void draw() override
    {
        static char field[64] = {};
        static char amount[24] = "1.25";
        static bool toggled = true;
        ui::kit_text_field("##g-field", "输入点什么…", field, sizeof field);
        ui::kit_vspace(0.3f);
        ImGui::SetNextItemWidth(ImGui::GetFontSize() * 11.0f);
        ui::kit_amount_field(
            "##g-amount", amount, sizeof amount, "SOL", nullptr, "Solana");
        ui::kit_vspace(0.3f);
        ui::kit_toggle("##g-toggle", &toggled);
        ImGui::SameLine();
        ImGui::TextDisabled(toggled ? "打开" : "关闭");
        ImGui::SameLine(0.0f, ImGui::GetFontSize());
        ui::kit_pill("live", ui::kit_accent());
        ui::kit_vspace(0.4f);
        if (ui::kit_primary_button("主按钮", ImGui::GetFontSize() * 8.0f))
            counter_++;
        ImGui::SameLine();
        if (ui::kit_subtle_button("次按钮", ImGui::GetFontSize() * 8.0f))
            counter_--;
        ImGui::SameLine();
        ImGui::TextDisabled("count=%d", counter_);
        ui::kit_vspace(0.4f);
        if (ui::kit_table_begin("##g-table", 3)) {
            static const char* kHeads[] = { "资产", "余额", "状态" };
            ui::kit_table_headers(kHeads, 3);
            static constexpr std::array<std::array<const char*, 3>, 3> kRows
                = { { { "SOL", "12.4", "在线" }, { "ETH", "0.88", "在线" },
                      { "BTC", "0.02", "观察" } } };
            for (const auto& row : kRows) {
                ImGui::TableNextRow();
                for (int c = 0; c < 3; ++c) {
                    ImGui::TableSetColumnIndex(c);
                    ImGui::TextUnformatted(row[static_cast<std::size_t>(c)]);
                }
            }
            ui::kit_table_end();
        }
    }

private:
    int counter_ = 0;
};

class NotesApp final : public os::App {
public:
    const char* id() const override
    {
        return "notes";
    }

    const char* name() const override
    {
        return "备忘";
    }

    const char* mark() const override
    {
        return "📝";
    }

    ImVec2 initial_size_em() const override
    {
        return { 24.0f, 18.0f };
    }

    void draw() override
    {
        static char buffer[2048] = {};
        static bool focus = false;
        ImGui::TextDisabled("窗口里是真组件——外框怎么变，字号不变。");
        ui::kit_vspace(0.3f);
        ui::kit_paste_box(
            "##notes", "写点东西…", buffer, sizeof buffer, 8.0f, focus);
    }
};

class AboutApp final : public os::App {
public:
    const char* id() const override
    {
        return "about";
    }

    const char* name() const override
    {
        return "关于 izan OS";
    }

    const char* mark() const override
    {
        return "⛩️";
    }

    ImVec2 initial_size_em() const override
    {
        return { 22.0f, 13.0f };
    }

    void draw() override
    {
        const float em = ImGui::GetFontSize();
        ImGui::PushFont(nullptr, em * 2.2f);
        ImGui::TextUnformatted("⛩️");
        ImGui::PopFont();
        ImGui::PushFont(nullptr, em * 1.15f);
        ImGui::TextUnformatted("izan OS");
        ImGui::PopFont();
        ImGui::TextDisabled("窗口内核 · 一套主题接口 · 家族所有产品的基石");
        ui::kit_vspace(0.3f);
        ImGui::TextDisabled("机制归内核，像素归主题：");
        ImGui::TextDisabled("macOS 风只是第一个主题实现。");
    }
};

// ---- headless capture ----

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
    opts.title = "izan-os";
    opts.width = 1500;
    opts.height = 980;
    if (!app.init(opts))
        return 1;
    ui::set_window_icon_resource(app.window(), 1);

    ui::ChromeState chrome;
    ui::apply_theme_style_only(chrome.theme_index);
    glfwSetWindowOpacity(app.window(), chrome.window_opacity);

    GalleryApp gallery;
    NotesApp notes;
    AboutApp about;
    os::Shell shell;
    shell.attach(&gallery);
    shell.attach(&notes);
    shell.attach(&about);
    shell.wm().launch(&gallery);
    shell.wm().launch(&about);

    int rendered_frames = 0;
    app.set_render_callback([&] {
        app.begin_frame();

        if (ImGui::IsKeyPressed(ImGuiKey_Escape))
            glfwSetWindowShouldClose(app.window(), GLFW_TRUE);

        ui::draw_main_window_frame(chrome);
        ui::draw_custom_title_bar(
            app.window(), chrome, "izan OS", "windowing kernel");

        const ImGuiViewport* vp = ImGui::GetMainViewport();
        const float top = ui::kWindowFrameMargin + ui::kTitleBarHeight;
        shell.frame(
            ImVec2(vp->Pos.x + ui::kWindowFrameMargin, vp->Pos.y + top),
            ImVec2(vp->Size.x - ui::kWindowFrameMargin * 2.0f,
                vp->Size.y - top - ui::kWindowFrameMargin));

        ui::draw_snap_layout_popup(app.window(), chrome);
        ui::draw_menu_popup_shadows(chrome);
        if (chrome.request_exit)
            glfwSetWindowShouldClose(app.window(), GLFW_TRUE);
        app.end_frame(ui::theme_clear_color(chrome));

        if (!screenshot_path.empty() && ++rendered_frames >= 4) {
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
