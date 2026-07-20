// The izan OS acceptance shell: the Mint-cultured working skin drawn
// line for line — host frame, wallpaper, panel, Cinnamon menu and a
// Nemo-style file manager — riding the izan windowing kernel and the
// family's native chrome (no startup flash, native drag and snap).
// --screenshot <file.bmp> renders a few frames and exits.

#include <GLFW/glfw3.h>

#if !defined(__APPLE__)
#include <GL/gl.h>
#endif

#include <imgui.h>
#include <imgui_internal.h>

#include <array>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

#include "ui/os/mint_paint.hpp"
#include "ui/os/shell.hpp"
#include "ui/shell/app.hpp"
#include "ui/shell/chrome_state.hpp"
#include "ui/shell/theme.hpp"
#include "ui/shell/ui_layout.hpp"
#include "ui/shell/win_chrome.hpp"
#include "ui/widgets/kit.hpp"

namespace {

namespace ui = izan::ui;
namespace os = izan::os;
using namespace izan::os::mint;

// ---- the file manager: the Nemo interior, drawn inside the kernel's
// content region ----

class FilesApp final : public os::App {
public:
    const char* id() const override
    {
        return "files";
    }

    const char* name() const override
    {
        return "Home — File Manager";
    }

    const char* mark() const override
    {
        return "📁";
    }

    ImVec2 initial_size_em() const override
    {
        // The reference Nemo opens at 1120x704 logical pixels.
        return { 60.0f, 37.7f };
    }

    void draw() override
    {
        const float s = mint_scale();
        const ImVec2 min = ImGui::GetWindowPos();
        const ImVec2 wsize = ImGui::GetWindowSize();
        const ImVec2 max { min.x + wsize.x, min.y + wsize.y };
        ImDrawList* draw = ImGui::GetWindowDrawList();
        constexpr float rounding = 8.0f;

        // The toolbar: history arrows, refresh, path bar, search.
        const ImVec2 toolbar_max { max.x, min.y + 56.0f * s };
        draw->AddRectFilled(min, toolbar_max, IM_COL32(244, 245, 244, 204));
        draw->AddLine({ min.x, toolbar_max.y }, { max.x, toolbar_max.y },
            IM_COL32(70, 73, 71, 35), 1.0f);
        const float toolbar_center_y = (min.y + toolbar_max.y) * 0.5f;
        const ImU32 toolbar_icon = IM_COL32(73, 78, 74, 255);
        const ImVec2 back { min.x + 24.0f * s, toolbar_center_y };
        const ImVec2 forward { min.x + 55.0f * s, toolbar_center_y };
        draw->AddLine({ back.x + 4.0f * s, back.y - 6.0f * s },
            { back.x - 2.0f * s, back.y }, toolbar_icon, 1.7f * s);
        draw->AddLine({ back.x - 2.0f * s, back.y },
            { back.x + 4.0f * s, back.y + 6.0f * s }, toolbar_icon, 1.7f * s);
        draw->AddLine({ forward.x - 4.0f * s, forward.y - 6.0f * s },
            { forward.x + 2.0f * s, forward.y }, IM_COL32(132, 136, 133, 255),
            1.7f * s);
        draw->AddLine({ forward.x + 2.0f * s, forward.y },
            { forward.x - 4.0f * s, forward.y + 6.0f * s },
            IM_COL32(132, 136, 133, 255), 1.7f * s);
        const ImVec2 refresh { min.x + 91.0f * s, toolbar_center_y };
        draw->PathArcTo(refresh, 8.0f * s, 0.4f, 5.6f, 22);
        draw->PathStroke(toolbar_icon, 0, 1.5f * s);
        draw->AddTriangleFilled({ refresh.x + 5.0f * s, refresh.y - 7.0f * s },
            { refresh.x + 10.0f * s, refresh.y - 8.0f * s },
            { refresh.x + 8.0f * s, refresh.y - 3.0f * s }, toolbar_icon);

        const ImVec2 search_min { max.x - 228.0f * s, min.y + 10.0f * s };
        const ImVec2 search_max { max.x - 14.0f * s,
            toolbar_max.y - 10.0f * s };
        const ImVec2 path_min { min.x + 120.0f * s, search_min.y };
        const ImVec2 path_max { search_min.x - 12.0f * s, search_max.y };
        draw->AddRectFilled(
            path_min, path_max, IM_COL32(255, 255, 255, 255), 6.0f * s);
        draw->AddRect(
            path_min, path_max, IM_COL32(72, 76, 73, 54), 6.0f * s, 0, 1.0f);
        mint_logo(draw,
            { path_min.x + 18.0f * s, (path_min.y + path_max.y) * 0.5f },
            8.0f * s);
        text_vcentered(draw, path_min.x + 34.0f * s,
            (path_min.y + path_max.y) * 0.5f, IM_COL32(55, 59, 56, 255), "Home",
            kFontBody * s);

        draw->AddRectFilled(
            search_min, search_max, IM_COL32(255, 255, 255, 255), 6.0f * s);
        draw->AddRect(search_min, search_max, IM_COL32(72, 76, 73, 54),
            6.0f * s, 0, 1.0f);
        const ImVec2 magnifier { search_min.x + 16.0f * s,
            (search_min.y + search_max.y) * 0.5f };
        draw->AddCircle(
            magnifier, 5.0f * s, IM_COL32(113, 117, 114, 255), 18, 1.3f * s);
        draw->AddLine({ magnifier.x + 3.7f * s, magnifier.y + 3.7f * s },
            { magnifier.x + 7.2f * s, magnifier.y + 7.2f * s },
            IM_COL32(113, 117, 114, 255), 1.3f * s);
        ImGui::SetCursorScreenPos(
            { search_min.x + 29.0f * s, search_min.y + 3.0f * s });
        ImGui::SetNextItemWidth(search_max.x - search_min.x - 35.0f * s);
        ImGui::PushFont(nullptr, kFontBody * s);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 3.0f * s, 3.0f * s });
        ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(52, 55, 53, 255));
        ImGui::PushStyleColor(
            ImGuiCol_TextDisabled, IM_COL32(138, 142, 139, 255));
        ImGui::PushStyleColor(ImGuiCol_NavCursor, IM_COL32(0, 0, 0, 0));
        ImGui::InputTextWithHint(
            "##nemo-search", "Search files", search_.data(), search_.size());
        ImGui::PopStyleColor(4);
        ImGui::PopStyleVar();
        ImGui::PopFont();

        // The sidebar of places.
        const ImVec2 content_min { min.x, toolbar_max.y };
        const float status_top = max.y - 28.0f * s;
        const float sidebar_width = 224.0f * s;
        draw->AddRectFilled(content_min,
            { content_min.x + sidebar_width, status_top },
            IM_COL32(235, 237, 235, 204));
        draw->AddLine({ content_min.x + sidebar_width, content_min.y },
            { content_min.x + sidebar_width, status_top },
            IM_COL32(65, 69, 66, 38), 1.0f);
        static constexpr std::array<const char*, 7> kPlaces
            = { "Home", "Desktop", "Documents", "Downloads", "Pictures",
                  "Videos", "Trash" };
        for (int i = 0; i < static_cast<int>(kPlaces.size()); ++i) {
            const float row_y = content_min.y + (14.0f + i * 40.0f) * s;
            const ImVec2 row_min { content_min.x + 8.0f * s, row_y };
            const ImVec2 row_max { content_min.x + sidebar_width - 8.0f * s,
                row_y + 36.0f * s };
            if (row_max.y > status_top)
                break;
            ImGui::SetCursorScreenPos(row_min);
            const std::string rid = "##place-" + std::to_string(i);
            if (ImGui::InvisibleButton(rid.c_str(),
                    { row_max.x - row_min.x, row_max.y - row_min.y })) {
                sidebar_item_ = i;
                selected_file_ = -1;
            }
            if (sidebar_item_ == i)
                draw->AddRectFilled(
                    row_min, row_max, IM_COL32(109, 190, 69, 72), 5.0f * s);
            const ImVec2 icon_center { row_min.x + 18.0f * s,
                (row_min.y + row_max.y) * 0.5f };
            if (i == 6)
                draw->AddCircleFilled(
                    icon_center, 7.0f * s, IM_COL32(118, 124, 120, 255));
            else
                folder_icon(draw, icon_center, 0.32f * s);
            text_vcentered(draw, row_min.x + 38.0f * s,
                (row_min.y + row_max.y) * 0.5f, IM_COL32(48, 52, 49, 255),
                kPlaces[static_cast<std::size_t>(i)], kFontBodyCompact * s);
        }

        // The file grid.
        const ImVec2 files_min { content_min.x + sidebar_width, content_min.y };
        const ImVec2 files_max { max.x, status_top };
        draw->AddRectFilled(files_min, files_max, IM_COL32(248, 249, 248, 204));
        text_vcentered(draw, files_min.x + 28.0f * s, files_min.y + 31.0f * s,
            IM_COL32(42, 46, 43, 255),
            kPlaces[static_cast<std::size_t>(sidebar_item_)],
            kFontSectionTitle * s);

        struct File {
            const char* name;
            bool folder;
            ImU32 color;
        };

        static constexpr std::array<File, 10> kFiles = { {
            { "Desktop", true, 0 },
            { "Documents", true, 0 },
            { "Downloads", true, 0 },
            { "Music", true, 0 },
            { "Pictures", true, 0 },
            { "Videos", true, 0 },
            { "Projects", true, 0 },
            { "README.txt", false, IM_COL32(92, 150, 201, 255) },
            { "mint-notes.md", false, IM_COL32(109, 190, 69, 255) },
            { "wallpaper.png", false, IM_COL32(66, 151, 119, 255) },
        } };
        const std::string query(search_.data());
        const float files_width = files_max.x - files_min.x;
        constexpr float cell_width = 168.0f;
        const int columns = std::max(
            1, static_cast<int>((files_width - 32.0f * s) / (cell_width * s)));
        const float grid_width = columns * cell_width * s;
        const float grid_left = files_min.x
            + std::max(16.0f * s, (files_width - grid_width) * 0.5f);
        int visible = 0;
        for (int i = 0; i < static_cast<int>(kFiles.size()); ++i) {
            const File& file = kFiles[static_cast<std::size_t>(i)];
            if (!query.empty()
                && std::string_view(file.name).find(query)
                    == std::string_view::npos)
                continue;
            const int col = visible % columns;
            const int row = visible / columns;
            const ImVec2 center { grid_left + (col + 0.5f) * cell_width * s,
                files_min.y + (108.0f + row * 132.0f) * s };
            const ImVec2 hit_min { center.x - 74.0f * s, center.y - 53.0f * s };
            const ImVec2 hit_max { center.x + 74.0f * s, center.y + 57.0f * s };
            if (hit_max.y < files_max.y) {
                ImGui::SetCursorScreenPos(hit_min);
                const std::string fid = "##file-" + std::to_string(i);
                if (ImGui::InvisibleButton(fid.c_str(),
                        { hit_max.x - hit_min.x, hit_max.y - hit_min.y }))
                    selected_file_ = i;
                if (selected_file_ == i) {
                    draw->AddRectFilled(
                        hit_min, hit_max, IM_COL32(109, 190, 69, 54), 6.0f * s);
                    draw->AddRect(hit_min, hit_max, IM_COL32(109, 190, 69, 105),
                        6.0f * s, 0, 1.0f);
                }
                if (file.folder)
                    folder_icon(
                        draw, { center.x, center.y - 12.0f * s }, 1.12f * s);
                else
                    document_icon(draw, { center.x, center.y - 12.0f * s },
                        1.04f * s, file.color);
                text_centered(draw, { center.x, center.y + 30.0f * s },
                    IM_COL32(43, 47, 44, 255), file.name, kFontBodyCompact * s);
            }
            ++visible;
        }

        // The status bar closes the window with the body's rounding.
        draw->AddRectFilled({ min.x, status_top }, max,
            IM_COL32(243, 244, 243, 204), rounding * s,
            ImDrawFlags_RoundCornersBottom);
        draw->AddLine({ min.x, status_top }, { max.x, status_top },
            IM_COL32(62, 66, 63, 34), 1.0f);
        char status[64] {};
        std::snprintf(status, sizeof(status), "%d item%s, 312.8 GB free",
            visible, visible == 1 ? "" : "s");
        text_centered(draw,
            { (min.x + max.x) * 0.5f, (status_top + max.y) * 0.5f },
            IM_COL32(91, 96, 92, 255), status, kFontSecondary * s);
    }

private:
    std::array<char, 96> search_ {};
    int sidebar_item_ = 0;
    int selected_file_ = -1;
};

class NotesApp final : public os::App {
public:
    const char* id() const override
    {
        return "notes";
    }

    const char* name() const override
    {
        return "Text Editor";
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
        ImGui::SetCursorScreenPos({ ImGui::GetWindowPos().x + 14.0f,
            ImGui::GetWindowPos().y + 14.0f });
        ImGui::BeginGroup();
        ImGui::TextDisabled("窗口里是真组件——外框怎么变，字号不变。");
        ui::kit_vspace(0.3f);
        ImGui::SetNextItemWidth(ImGui::GetWindowSize().x - 28.0f);
        ui::kit_paste_box(
            "##notes", "写点东西…", buffer, sizeof buffer, 8.0f, focus);
        ImGui::EndGroup();
    }
};

class GalleryApp final : public os::App {
public:
    const char* id() const override
    {
        return "gallery";
    }

    const char* name() const override
    {
        return "System Settings";
    }

    const char* mark() const override
    {
        return "🧩";
    }

    void draw() override
    {
        ImGui::SetCursorScreenPos({ ImGui::GetWindowPos().x + 14.0f,
            ImGui::GetWindowPos().y + 14.0f });
        ImGui::BeginGroup();
        static char field[64] = {};
        static char amount[24] = "1.25";
        static bool toggled = true;
        ImGui::SetNextItemWidth(ImGui::GetFontSize() * 12.0f);
        ui::kit_text_field("##g-field", "输入点什么…", field, sizeof field);
        ui::kit_vspace(0.3f);
        ImGui::SetNextItemWidth(ImGui::GetFontSize() * 11.0f);
        ui::kit_amount_field(
            "##g-amount", amount, sizeof amount, "SOL", nullptr, "Solana");
        ui::kit_vspace(0.3f);
        ui::kit_toggle("##g-toggle", &toggled);
        ImGui::SameLine();
        ImGui::TextDisabled(toggled ? "打开" : "关闭");
        ui::kit_vspace(0.4f);
        if (ui::kit_primary_button("主按钮", ImGui::GetFontSize() * 8.0f))
            counter_++;
        ImGui::SameLine();
        ImGui::TextDisabled("count=%d", counter_);
        ImGui::EndGroup();
    }

private:
    int counter_ = 0;
};

// ---- the Mint host frame on the family's native chrome ----

void draw_mint_host_frame(GLFWwindow* window)
{
    const float s = mint_scale();
    const ImGuiViewport* vp = ImGui::GetMainViewport();
    const ImVec2 min = vp->Pos;
    const ImVec2 max { vp->Pos.x + vp->Size.x, vp->Pos.y + kFrameHeight * s };
    const ImVec2 view_max { vp->Pos.x + vp->Size.x, vp->Pos.y + vp->Size.y };
    const bool maximized
        = glfwGetWindowAttrib(window, GLFW_MAXIMIZED) == GLFW_TRUE;
    const float controls_width = 138.0f * s;

    // The whole strip drags natively; the controls corner is carved
    // out so its clicks stay with the buttons below.
    ui::update_title_bar_hit_regions(window, min, max, { 0.0f, 0.0f },
        { 0.0f, 0.0f }, { max.x - controls_width, min.y }, { max.x, max.y });

    ImGui::SetNextWindowPos(min);
    ImGui::SetNextWindowSize({ max.x - min.x, max.y - min.y });
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(0, 0, 0, 0));
    ImGui::Begin("###mint-host-frame", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse
            | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar
            | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoDocking
            | ImGuiWindowFlags_NoFocusOnAppearing
            | ImGuiWindowFlags_NoBringToFrontOnFocus);
    ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
    ImDrawList* draw = ImGui::GetWindowDrawList();

    draw->AddRectFilledMultiColor(min, max, IM_COL32(49, 52, 54, 255),
        IM_COL32(49, 52, 54, 255), IM_COL32(39, 42, 44, 255),
        IM_COL32(39, 42, 44, 255));
    draw->AddLine({ min.x, max.y - 1.0f }, { max.x, max.y - 1.0f },
        IM_COL32(0, 0, 0, 130), 1.0f);
    mint_logo(draw, { min.x + 22.0f * s, (min.y + max.y) * 0.5f }, 11.5f * s);
    text_vcentered(draw, min.x + 42.0f * s, (min.y + max.y) * 0.5f,
        IM_COL32(231, 233, 234, 255), "izan OS · Cinnamon 测试皮肤",
        kFontWindowTitle * s);

    // IZAN_OS_STATS=1: the frame clock on the bar, so a performance
    // argument is settled by numbers instead of feel.
    if (std::getenv("IZAN_OS_STATS") != nullptr) {
        char stats[64] {};
        const ImGuiIO& io = ImGui::GetIO();
        std::snprintf(stats, sizeof stats, "%.1f fps · %.2f ms", io.Framerate,
            1000.0f / (io.Framerate > 1.0f ? io.Framerate : 1.0f));
        text_vcentered(draw, min.x + 340.0f * s, (min.y + max.y) * 0.5f,
            IM_COL32(160, 200, 130, 255), stats, kFontBodyCompact * s);
    }

    const float button_width = 46.0f * s;
    for (int i = 0; i < 3; ++i) {
        const ImVec2 bmin { max.x - (3 - i) * button_width, min.y };
        const ImVec2 bmax { bmin.x + button_width, max.y };
        const bool hovered = ImGui::IsMouseHoveringRect(bmin, bmax, false);
        if (hovered)
            draw->AddRectFilled(bmin, bmax,
                i == 2 ? IM_COL32(204, 66, 66, 255)
                       : IM_COL32(255, 255, 255, 30));
        control_icon(draw, { (bmin.x + bmax.x) * 0.5f, (min.y + max.y) * 0.5f },
            i, maximized, IM_COL32(238, 239, 240, 255), s);
        ImGui::SetCursorScreenPos(bmin);
        const std::string bid = "##host-control-" + std::to_string(i);
        if (ImGui::InvisibleButton(
                bid.c_str(), { bmax.x - bmin.x, bmax.y - bmin.y })) {
            if (i == 0)
                glfwIconifyWindow(window);
            else if (i == 1)
                ui::toggle_window_maximized(window);
            else
                glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }

    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);

    // The outer edge rides above everything, keeping the rounded
    // outline whole even under hovered corner buttons.
    ImGui::GetForegroundDrawList()->AddRect(min, view_max,
        IM_COL32(255, 255, 255, 38), maximized ? 0.0f : 9.0f * s, 0, 1.0f);
}

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

    // The host window stays fully opaque: translucency belongs to
    // the components inside (panel, menus), never the whole frame.
    ui::ChromeState chrome;
    ui::apply_theme_style_only(chrome.theme_index);

    FilesApp files;
    NotesApp notes;
    GalleryApp gallery;
    os::Shell shell;
    shell.attach(&files);
    shell.attach(&notes);
    shell.attach(&gallery);
    shell.wm().launch(&files);

    int rendered_frames = 0;
    app.set_render_callback([&] {
        app.begin_frame();

        if (ImGui::IsKeyPressed(ImGuiKey_Escape))
            glfwSetWindowShouldClose(app.window(), GLFW_TRUE);

        const ImGuiViewport* vp = ImGui::GetMainViewport();
        const float bar_h = kFrameHeight * mint_scale();
        shell.frame(ImVec2(vp->Pos.x, vp->Pos.y + bar_h),
            ImVec2(vp->Size.x, vp->Size.y - bar_h));
        draw_mint_host_frame(app.window());

        app.end_frame(ImVec4(0.08f, 0.16f, 0.15f, 1.0f));

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
