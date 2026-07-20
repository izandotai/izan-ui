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

#include <backends/imgui_impl_opengl3.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "ui/render/svg_raster.hpp"

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

        // Every interior surface is a faint tint over the theme's one
        // sheet of glass — square-edged: the silhouette (and its
        // rounded corners) belongs to the theme alone, and a tint
        // spilling a few pixels past the arc is invisible where a
        // second opaque arc would cut a crescent of shadow.
        const ImVec2 toolbar_max { max.x, min.y + 56.0f * s };
        draw->AddRectFilled(min, toolbar_max, IM_COL32(0, 0, 0, 6));
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

        // The shared input component wears the white suit here; the
        // magnifier is the caller's garnish, painted over the well.
        IzanFieldStyle search_style;
        search_style.inset_x = 29.0f;
        izan_input("##nemo-search", "Search files", search_.data(),
            search_.size(), search_min, search_max, search_style);
        const ImVec2 magnifier { search_min.x + 16.0f * s,
            (search_min.y + search_max.y) * 0.5f };
        draw->AddCircle(
            magnifier, 5.0f * s, IM_COL32(113, 117, 114, 255), 18, 1.3f * s);
        draw->AddLine({ magnifier.x + 3.7f * s, magnifier.y + 3.7f * s },
            { magnifier.x + 7.2f * s, magnifier.y + 7.2f * s },
            IM_COL32(113, 117, 114, 255), 1.3f * s);

        // The sidebar of places.
        const ImVec2 content_min { min.x, toolbar_max.y };
        const float status_top = max.y - 28.0f * s;
        const float sidebar_width = 224.0f * s;
        draw->AddRectFilled(content_min,
            { content_min.x + sidebar_width, status_top },
            IM_COL32(0, 0, 0, 12));
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
        // The file grid sits directly on the glass; no tint needed.
        (void)files_min;
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

        // The status bar is a square tint too — an app arc over the
        // theme's arc never lands on the same pixels, and the sliver
        // between them shows the shadow as a black notch.
        (void)rounding;
        draw->AddRectFilled({ min.x, status_top }, max, IM_COL32(0, 0, 0, 8));
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
        static char buffer[256] = {};
        const float s = mint_scale();
        const ImVec2 origin { ImGui::GetWindowPos().x + 14.0f * s,
            ImGui::GetWindowPos().y + 14.0f * s };
        ImGui::SetCursorScreenPos(origin);
        ImGui::TextDisabled("窗口里是真组件——外框怎么变，字号不变。");
        IzanFieldStyle st;
        const ImVec2 wmin { origin.x, origin.y + 44.0f * s };
        const ImVec2 wmax { origin.x
                + std::min(ImGui::GetWindowSize().x - 28.0f * s, 460.0f * s),
            wmin.y + 40.0f * s };
        izan_input(
            "##notes", "写点东西…", buffer, sizeof buffer, wmin, wmax, st);
        probe_well_min = wmin;
        probe_well_max = wmax;
        probe_active = ImGui::IsItemActive();
    }

    // The caret probe reads where the well landed and whether the
    // input actually holds ActiveId.
    ImVec2 probe_well_min {};
    ImVec2 probe_well_max {};
    bool probe_active = false;
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
        {
            const float s = mint_scale();
            const ImVec2 wmin = ImGui::GetCursorScreenPos();
            const ImVec2 wmax { wmin.x + 300.0f * s, wmin.y + 40.0f * s };
            IzanFieldStyle st;
            izan_input("##g-field", "输入点什么…", field, sizeof field, wmin,
                wmax, st);
            ImGui::SetCursorScreenPos({ wmin.x, wmax.y + 10.0f * s });
        }
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

    // The frame clock rides the bar, always: a performance argument
    // is settled by numbers instead of feel.
    {
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

// ---- the SVG wallpaper: assets/wallpaper/mint-waves.svg (or the
// IZAN_OS_WALLPAPER file) rastered at desktop size through the SVG
// road, uploaded once per size; IZAN_OS_WALLPAPER=paint keeps the
// theme's painted backdrop ----

typedef unsigned int GLuint_t;

class SvgWallpaper {
public:
    // Baked ONCE at the primary monitor's full size; every later call
    // is pure UV math — a centered cover-crop of the bake for the
    // current desk. Resize, maximize, restore: no rebake, no stretch
    // lag, the crop just breathes. 0 while unavailable (no file /
    // parse failure / painted mode by request).
    ImTextureID texture(ImVec2 size, ImVec2& uv0, ImVec2& uv1)
    {
        if (broken_ || size.x <= 0.0f || size.y <= 0.0f)
            return 0;
        if (tex_ == 0)
            bake();
        if (tex_ == 0)
            return 0;
        const float want = size.x / size.y;
        const float have = static_cast<float>(w_) / static_cast<float>(h_);
        float du = 1.0f, dv = 1.0f;
        if (want >= have)
            dv = have / want; // wider desk: crop top and bottom
        else
            du = want / have; // taller desk: crop left and right
        uv0 = { (1.0f - du) * 0.5f, (1.0f - dv) * 0.5f };
        uv1 = { (1.0f + du) * 0.5f, (1.0f + dv) * 0.5f };
        return static_cast<ImTextureID>(tex_);
    }

private:
    void bake()
    {
        if (broken_)
            return;
        const char* pick = std::getenv("IZAN_OS_WALLPAPER");
        if (pick && std::string_view(pick) == "paint") {
            broken_ = true; // painted mode by request
            return;
        }
        const std::filesystem::path path = pick && *pick
            ? std::filesystem::path(pick)
            : ui::executable_dir() / "assets" / "wallpaper" / "mint-waves.svg";
        std::ifstream in(path, std::ios::binary);
        if (!in) {
            broken_ = true;
            return;
        }
        std::stringstream text;
        text << in.rdbuf();
        const std::string svg = text.str();
        int w = 1920, h = 1200;
        if (const GLFWvidmode* mode
            = glfwGetVideoMode(glfwGetPrimaryMonitor())) {
            w = mode->width;
            h = mode->height;
        }
        const izan::render::SvgBitmap art
            = izan::render::raster_svg_cover(svg.c_str(), w, h);
        if (art.empty()) {
            broken_ = true;
            return;
        }
        glGenTextures(1, &tex_);
        glBindTexture(GL_TEXTURE_2D, tex_);
        glTexImage2D(GL_TEXTURE_2D, 0, 0x8058 /* GL_RGBA8 */, w, h, 0, GL_RGBA,
            GL_UNSIGNED_BYTE, art.rgba.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        w_ = w;
        h_ = h;
    }

    GLuint_t tex_ = 0;
    int w_ = 0;
    int h_ = 0;
    bool broken_ = false;
};

// ---- the wallpaper cache: the theme's painter runs once per size
// into a texture; every frame after that is a single blit ----

class WallpaperCache {
public:
    ImTextureID texture(const os::Theme& theme, ImVec2 size, float em)
    {
        const int w = static_cast<int>(size.x);
        const int h = static_cast<int>(size.y);
        // The font atlas texture only truly exists after the first
        // full frame has rendered; a bake before that samples black.
        if (w <= 0 || h <= 0 || attempts_ > 8 || ImGui::GetFrameCount() < 2)
            return 0;
        if (baked_ && w == w_ && h == h_)
            return static_cast<ImTextureID>(tex_);
        ++attempts_;
        bake(theme, w, h, em);
        // A failed bake falls back to live painting and tries again
        // next frame — a black wallpaper is never an option.
        return baked_ ? static_cast<ImTextureID>(tex_) : 0;
    }

private:
    void bake(const os::Theme& theme, int w, int h, float em)
    {
        typedef void(APIENTRY * GenFramebuffers)(int, GLuint_t*);
        typedef void(APIENTRY * BindFramebuffer)(unsigned, GLuint_t);
        typedef void(APIENTRY * FramebufferTexture2D)(
            unsigned, unsigned, unsigned, GLuint_t, int);
        typedef unsigned(APIENTRY * CheckFramebufferStatus)(unsigned);
        static const auto gen_fbo = reinterpret_cast<GenFramebuffers>(
            glfwGetProcAddress("glGenFramebuffers"));
        static const auto bind_fbo = reinterpret_cast<BindFramebuffer>(
            glfwGetProcAddress("glBindFramebuffer"));
        static const auto fbo_tex2d = reinterpret_cast<FramebufferTexture2D>(
            glfwGetProcAddress("glFramebufferTexture2D"));
        static const auto fbo_status = reinterpret_cast<CheckFramebufferStatus>(
            glfwGetProcAddress("glCheckFramebufferStatus"));
        if (!gen_fbo || !bind_fbo || !fbo_tex2d || !fbo_status)
            return; // no FBO support: the shell keeps painting live

        constexpr unsigned kFramebuffer = 0x8D40; // GL_FRAMEBUFFER
        constexpr unsigned kColor0 = 0x8CE0;      // GL_COLOR_ATTACHMENT0
        constexpr unsigned kComplete = 0x8CD5;    // _COMPLETE
        constexpr unsigned kRgba8 = 0x8058;       // GL_RGBA8
        if (tex_ == 0)
            glGenTextures(1, &tex_);
        glBindTexture(GL_TEXTURE_2D, tex_);
        glTexImage2D(GL_TEXTURE_2D, 0, kRgba8, w, h, 0, GL_RGBA,
            GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        if (fbo_ == 0)
            gen_fbo(1, &fbo_);
        bind_fbo(kFramebuffer, fbo_);
        fbo_tex2d(kFramebuffer, kColor0, GL_TEXTURE_2D, tex_, 0);
        int bound = -1;
        glGetIntegerv(0x8CA6 /* GL_FRAMEBUFFER_BINDING */, &bound);
        if (static_cast<GLuint_t>(bound) != fbo_
            || fbo_status(kFramebuffer) != kComplete) {
            bind_fbo(kFramebuffer, 0);
            return;
        }
        glDisable(GL_SCISSOR_TEST);
        glViewport(0, 0, w, h);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // One throwaway draw list, rendered by the regular backend
        // into the bound framebuffer.
        ImDrawList list(ImGui::GetDrawListSharedData());
        list._ResetForNewFrame();
        list.PushClipRect({ 0.0f, 0.0f },
            { static_cast<float>(w), static_cast<float>(h) }, false);
        list.PushTexture(ImGui::GetIO().Fonts->TexRef);
        theme.paint_wallpaper(&list, { 0.0f, 0.0f },
            { static_cast<float>(w), static_cast<float>(h) }, em);
        list.PopTexture();
        list.PopClipRect();

        ImDrawData data;
        data.Valid = true;
        // The backend creates and updates atlas textures off this
        // list; without it a first-frame bake samples a texture that
        // does not exist yet and comes out black.
        data.Textures = &ImGui::GetPlatformIO().Textures;
        data.CmdLists.push_back(&list);
        data.CmdListsCount = 1;
        data.TotalVtxCount = list.VtxBuffer.Size;
        data.TotalIdxCount = list.IdxBuffer.Size;
        data.DisplayPos = { 0.0f, 0.0f };
        data.DisplaySize = { static_cast<float>(w), static_cast<float>(h) };
        data.FramebufferScale = { 1.0f, 1.0f };
        ImGui_ImplOpenGL3_RenderDrawData(&data);
        data.CmdLists.clear(); // the list is stack-owned, not the data's

        bind_fbo(kFramebuffer, 0);
        w_ = w;
        h_ = h;
        baked_ = true;
    }

    GLuint_t tex_ = 0;
    GLuint_t fbo_ = 0;
    int w_ = 0;
    int h_ = 0;
    int attempts_ = 0;
    bool baked_ = false;
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
    // IZAN_OS_LAZY=1: try the event-driven idle mode.
    opts.lazy_redraw = std::getenv("IZAN_OS_LAZY") != nullptr;
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
    // IZAN_CARET_PROBE=1: open the notes window alone, inject a
    // synthetic click into its input through imgui's event queue (no
    // OS-level input, which this session cannot fake reliably), then
    // depose ActiveId and the window roster to a file.
    const bool caret_probe = std::getenv("IZAN_CARET_PROBE") != nullptr;
    // IZAN_OS_APP=files|notes|gallery picks the first window for
    // screenshot verification runs.
    const char* first_id = std::getenv("IZAN_OS_APP");
    os::App* first = &files;
    if (caret_probe || (first_id && std::string_view(first_id) == "notes"))
        first = &notes;
    else if (first_id && std::string_view(first_id) == "gallery")
        first = &gallery;
    shell.wm().launch(first);
    WallpaperCache wallpaper;
    SvgWallpaper svg_wallpaper;

    int rendered_frames = 0;
    app.set_render_callback([&] {
        app.begin_frame();

        if (ImGui::IsKeyPressed(ImGuiKey_Escape))
            glfwSetWindowShouldClose(app.window(), GLFW_TRUE);

        const ImGuiViewport* vp = ImGui::GetMainViewport();
        const float bar_h = kFrameHeight * mint_scale();
        const ImVec2 desk_size(vp->Size.x, vp->Size.y - bar_h);
        // SVG backdrop first choice; the FBO-baked painter is the
        // fallback when no file renders. Uploaded textures are
        // top-down, so the UVs go straight (the FBO bake is
        // bottom-up and keeps the flipped default).
        ImVec2 wp_uv0, wp_uv1;
        if (ImTextureID svg_tex
            = svg_wallpaper.texture(desk_size, wp_uv0, wp_uv1))
            shell.set_wallpaper(svg_tex, wp_uv0, wp_uv1);
        else
            shell.set_wallpaper(wallpaper.texture(
                os::mint_theme(), desk_size, ImGui::GetFontSize()));
        shell.frame(ImVec2(vp->Pos.x, vp->Pos.y + bar_h), desk_size);
        draw_mint_host_frame(app.window());
        // Window in hand → the cursor rides in the frame, pixel-locked
        // to what it drags; hands off → hardware cursor, zero latency.
        ui::draw_inframe_cursor(shell.wm().interacting());

        if (caret_probe) {
            ImGuiIO& io = ImGui::GetIO();
            const int f = ImGui::GetFrameCount();
            const ImVec2 c { (notes.probe_well_min.x + notes.probe_well_max.x)
                    * 0.5f,
                (notes.probe_well_min.y + notes.probe_well_max.y) * 0.5f };
            if (f == 30)
                io.AddMousePosEvent(c.x, c.y);
            if (f == 33)
                io.AddMouseButtonEvent(ImGuiMouseButton_Left, true);
            if (f == 35)
                io.AddMouseButtonEvent(ImGuiMouseButton_Left, false);
            if (f == 60) {
                ImGuiContext& g = *ImGui::GetCurrentContext();
                if (std::FILE* fp = std::fopen("caret-probe.txt", "w")) {
                    std::fprintf(fp,
                        "active_id=%u item_active=%d hovered=%s nav=%s "
                        "active_window=%s well=(%.0f,%.0f)-(%.0f,%.0f)\n",
                        g.ActiveId, notes.probe_active ? 1 : 0,
                        g.HoveredWindow ? g.HoveredWindow->Name : "-",
                        g.NavWindow ? g.NavWindow->Name : "-",
                        g.ActiveIdWindow ? g.ActiveIdWindow->Name : "-",
                        notes.probe_well_min.x, notes.probe_well_min.y,
                        notes.probe_well_max.x, notes.probe_well_max.y);
                    std::fclose(fp);
                }
            }
            if (f == 65) {
                int dw = 0, dh = 0;
                glfwGetFramebufferSize(app.window(), &dw, &dh);
                capture_front_buffer("caret-probe.bmp", dw, dh);
            }
            if (f == 70)
                glfwSetWindowShouldClose(app.window(), GLFW_TRUE);
        }

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
