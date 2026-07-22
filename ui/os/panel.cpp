#include "ui/os/panel.hpp"

#include "ui/render/svg_icon.hpp"
#include "ui/shell/fonts.hpp"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <string>
#include <string_view>

#include <imgui_internal.h>

#include "ui/os/mint_paint.hpp"

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <chrono>
#include <format>
#endif

namespace izan::os {

namespace {

    using namespace mint;

    void local_clock_line(char* out, std::size_t size)
    {
#if defined(_WIN32)
        SYSTEMTIME t {};
        GetLocalTime(&t);
        std::snprintf(out, size, "%02u:%02u  %02u/%02u/%04u", t.wHour,
            t.wMinute, t.wDay, t.wMonth, t.wYear);
#else
        const auto now = std::chrono::floor<std::chrono::seconds>(
            std::chrono::system_clock::now());
        const std::string s = std::format("{:%H:%M  %d/%m/%Y}", now);
        std::snprintf(out, size, "%s", s.c_str());
#endif
    }

    // A borderless, transparent, display-front window the furniture
    // paints into; its buttons capture input over whatever floats
    // beneath.
    void begin_furniture_window(const char* id, ImVec2 min, ImVec2 max)
    {
        ImGui::SetNextWindowPos(min);
        ImGui::SetNextWindowSize({ max.x - min.x, max.y - min.y });
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(0, 0, 0, 0));
        ImGui::Begin(id, nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
                | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse
                | ImGuiWindowFlags_NoSavedSettings
                | ImGuiWindowFlags_NoScrollbar
                | ImGuiWindowFlags_NoScrollWithMouse
                | ImGuiWindowFlags_NoDocking
                | ImGuiWindowFlags_NoFocusOnAppearing
                | ImGuiWindowFlags_NoBringToFrontOnFocus);
        ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
    }

    void end_furniture_window()
    {
        ImGui::End();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar(2);
    }

    bool invisible_hit(const char* id, ImVec2 min, ImVec2 max)
    {
        if (max.x <= min.x || max.y <= min.y)
            return false;
        ImGui::SetCursorScreenPos(min);
        return ImGui::InvisibleButton(id, { max.x - min.x, max.y - min.y });
    }

}

float Panel::height(float em) const
{
    return em * kPanelHeight / (ui::kDefaultFontSize / ui::kFontDesignScale);
}

void Panel::request_launch(std::string_view id)
{
    if (id.empty())
        return;
    const bool pending
        = std::any_of(launch_requests_.begin(), launch_requests_.end(),
            [&](const LaunchRequest& request) { return request.id == id; });
    if (!pending)
        launch_requests_.push_back({ std::string(id) });
}

std::vector<LaunchRequest> Panel::take_launch_requests()
{
    std::vector<LaunchRequest> out;
    out.swap(launch_requests_);
    return out;
}

void Panel::frame(
    Wm& wm, const std::vector<App*>& apps, ImVec2 view_min, ImVec2 view_max)
{
    const float em = ImGui::GetFontSize();
    const float s = mint_scale();
    const float bar_h = height(em);
    const ImVec2 bar_min { view_min.x, view_max.y - bar_h };
    const ImVec2 bar_max = view_max;
    const float center_y = (bar_min.y + bar_max.y) * 0.5f;
    const ImVec2 mouse = ImGui::GetIO().MousePos;

    blocked_.clear();
    blocked_.push_back({ bar_min, bar_max });

    begin_furniture_window("###os-panel", bar_min, bar_max);
    ImDrawList* draw = ImGui::GetWindowDrawList();

    draw->AddRectFilled(bar_min, bar_max, IM_COL32(30, 33, 33, 248));
    draw->AddLine(
        bar_min, { bar_max.x, bar_min.y }, IM_COL32(255, 255, 255, 35), 1.0f);

    // ---- left: the Menu button ----
    const ImVec2 menu_min { bar_min.x + 7.0f * s, bar_min.y + 5.0f * s };
    const ImVec2 menu_max { bar_min.x + 119.0f * s, bar_max.y - 5.0f * s };
    const bool menu_hot = ImGui::IsMouseHoveringRect(menu_min, menu_max, false);
    if (menu_open_ || menu_hot)
        draw->AddRectFilled(
            menu_min, menu_max, IM_COL32(255, 255, 255, 25), 4.0f * s);
    mint_logo(draw,
        { menu_min.x + 18.0f * s, (menu_min.y + menu_max.y) * 0.5f },
        12.0f * s);
    text_vcentered(draw, menu_min.x + 38.0f * s, center_y,
        IM_COL32(241, 242, 242, 255), "Menu", kFontWindowTitle * s);
    if (invisible_hit("##panel-menu", menu_min, menu_max))
        menu_open_ = !menu_open_;

    // ---- quick launchers: one dot per attached app ----
    static constexpr std::array<ImU32, 4> kLauncherColors = {
        IM_COL32(89, 155, 213, 255),
        IM_COL32(234, 156, 54, 255),
        IM_COL32(63, 164, 113, 255),
        IM_COL32(114, 190, 77, 255),
    };
    static constexpr std::array<const char*, 4> kLauncherGlyphs
        = { "⌂", "●", ">_", "F" };
    const int shelf_size
        = static_cast<int>(catalog_.empty() ? apps.size() : catalog_.size());
    const int launchers = std::min(4, shelf_size);
    for (int i = 0; i < launchers; ++i) {
        const ImVec2 center { bar_min.x + (148.0f + i * 42.0f) * s, center_y };
        draw->AddCircleFilled(
            center, 14.0f * s, kLauncherColors[static_cast<std::size_t>(i)]);
        const AppDescriptor descriptor = catalog_.empty()
            ? AppDescriptor { apps[std::size_t(i)]->id(),
                  apps[std::size_t(i)]->name(), apps[std::size_t(i)]->mark() }
            : catalog_[std::size_t(i)];
        const char* glyph = descriptor.mark.empty()
            ? kLauncherGlyphs[static_cast<std::size_t>(i)]
            : descriptor.mark.c_str();
        text_centered(
            draw, center, IM_COL32(255, 255, 255, 255), glyph, kFontSymbol * s);
        const std::string hit_id = "##launcher-" + descriptor.id;
        if (invisible_hit(hit_id.c_str(),
                { center.x - 14.0f * s, center.y - 14.0f * s },
                { center.x + 14.0f * s, center.y + 14.0f * s })) {
            App* live = nullptr;
            for (App* app : apps)
                if (descriptor.id == app->id()) {
                    live = app;
                    break;
                }
            if (live)
                wm.launch(live);
            else
                request_launch(descriptor.id);
        }
    }

    draw->AddLine({ bar_min.x + 326.0f * s, bar_min.y + 8.0f * s },
        { bar_min.x + 326.0f * s, bar_max.y - 8.0f * s },
        IM_COL32(255, 255, 255, 20), 1.0f);

    // ---- middle: one task button per running app ----
    float x = bar_min.x + 338.0f * s;
    for (App* app : apps) {
        if (!wm.running(app))
            continue;
        const bool min = wm.minimized(app);
        const ImVec2 bmin { x, bar_min.y + 5.0f * s };
        const float mark_font = kFontBodyCompact * s;
        ImGui::PushFont(nullptr, mark_font);
        const ImVec2 ms = ImGui::CalcTextSize(app->mark());
        const ImVec2 ns = ImGui::CalcTextSize(app->name());
        ImGui::PopFont();
        const ImVec2 bmax { x + 40.0f * s + ns.x + 16.0f * s,
            bar_max.y - 5.0f * s };
        draw->AddRectFilled(
            bmin, bmax, IM_COL32(255, 255, 255, min ? 15 : 28), 4.0f * s);
        draw->AddText(ImGui::GetFont(), mark_font,
            { bmin.x + 20.0f * s - ms.x * 0.5f, center_y - ms.y * 0.5f },
            IM_COL32_WHITE, app->mark());
        text_vcentered(draw, bmin.x + 40.0f * s, center_y,
            IM_COL32(237, 239, 238, 255), app->name(), mark_font);
        const std::string bid = std::string("##task-") + app->id();
        if (invisible_hit(bid.c_str(), bmin, bmax))
            wm.toggle(app);
        x = bmax.x + 10.0f * s;
    }

    // ---- right: clock, then the tray marching left ----
    char clock[48] {};
    local_clock_line(clock, sizeof clock);
    const float clock_font = kFontBodyCompact * s;
    ImGui::PushFont(nullptr, clock_font);
    const ImVec2 cs = ImGui::CalcTextSize(clock);
    ImGui::PopFont();
    const float clock_x = bar_max.x - 16.0f * s - cs.x;
    text_vcentered(draw, clock_x, center_y, IM_COL32(241, 242, 242, 255), clock,
        clock_font);
    const float tray_right = clock_x - 20.0f * s;
    const ImU32 tray_ink = IM_COL32(235, 237, 236, 255);
    // This is a wallet OS, not a desktop: the tray speaks device
    // links, not host plumbing. USB/plug = the transport; lock = the
    // vault on the other end. Noto SVG textures first (hand-drawn
    // primitives have no AA channel and jag — 2026-07-20 production
    // verdict); the primitives below stay only as the no-assets
    // fallback. Still mute furniture until the device layer lands.
    const int icon_px = static_cast<int>(22.0f * s + 0.5f);
    const auto noto = [&](const char* file) {
        return izan::render::svg_icon(
            ui::executable_dir() / "assets" / "noto" / file, icon_px);
    };
    const ImTextureID usb_tex = noto("emoji_u1f50c.svg");
    const ImTextureID lock_tex = noto("emoji_u1f512.svg");
    if (usb_tex != 0 && lock_tex != 0) {
        const float half = icon_px * 0.5f;
        const ImVec2 u { tray_right - 60.0f * s, center_y };
        draw->AddImage(
            usb_tex, { u.x - half, u.y - half }, { u.x + half, u.y + half });
        const ImVec2 l { tray_right - 24.0f * s, center_y };
        draw->AddImage(
            lock_tex, { l.x - half, l.y - half }, { l.x + half, l.y + half });
    } else {
        const ImVec2 c { tray_right - 60.0f * s, center_y };
        draw->AddLine({ c.x, c.y + 8.0f * s }, { c.x, c.y - 6.0f * s },
            tray_ink, 1.5f * s);
        draw->AddTriangleFilled({ c.x, c.y - 9.5f * s },
            { c.x - 2.6f * s, c.y - 4.5f * s },
            { c.x + 2.6f * s, c.y - 4.5f * s }, tray_ink);
        draw->AddCircleFilled({ c.x, c.y + 8.0f * s }, 2.0f * s, tray_ink, 12);
        draw->AddLine({ c.x, c.y + 4.0f * s },
            { c.x - 5.5f * s, c.y - 1.0f * s }, tray_ink, 1.5f * s);
        draw->AddRectFilled({ c.x - 7.3f * s, c.y - 4.6f * s },
            { c.x - 3.7f * s, c.y - 1.0f * s }, tray_ink);
        draw->AddLine({ c.x, c.y + 2.0f * s },
            { c.x + 5.5f * s, c.y - 2.5f * s }, tray_ink, 1.5f * s);
        draw->AddCircleFilled(
            { c.x + 5.5f * s, c.y - 4.0f * s }, 1.9f * s, tray_ink, 12);
        const ImVec2 k { tray_right - 24.0f * s, center_y };
        draw->AddRect({ k.x - 5.5f * s, k.y - 4.5f * s },
            { k.x + 5.5f * s, k.y + 4.5f * s }, tray_ink, 1.5f * s, 0,
            1.5f * s);
        draw->AddRectFilled({ k.x - 2.0f * s, k.y - 1.5f * s },
            { k.x + 2.0f * s, k.y + 1.5f * s }, tray_ink);
        for (int i = -1; i <= 1; ++i) {
            const float y = k.y + static_cast<float>(i) * 3.0f * s;
            draw->AddLine({ k.x - 8.0f * s, y }, { k.x - 5.5f * s, y },
                tray_ink, 1.3f * s);
            draw->AddLine({ k.x + 5.5f * s, y }, { k.x + 8.0f * s, y },
                tray_ink, 1.3f * s);
        }
    }

    end_furniture_window();

    if (menu_open_)
        draw_menu(wm, apps, bar_min, bar_max);
}

void Panel::draw_menu(
    Wm& wm, const std::vector<App*>& apps, ImVec2 panel_min, ImVec2 panel_max)
{
    const float s = mint_scale();
    const ImVec2 min { panel_min.x + 9.0f * s, panel_min.y - 600.0f * s };
    const ImVec2 max { min.x + 720.0f * s, panel_min.y - 7.0f * s };
    blocked_.push_back({ min, max });

    const ImVec2 mouse = ImGui::GetIO().MousePos;
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)
        && !ImGui::IsMouseHoveringRect(min, max, false)
        && mouse.y < panel_min.y) {
        menu_open_ = false;
        return;
    }

    begin_furniture_window("###os-panel-popup",
        { min.x - 16.0f * s, min.y - 16.0f * s },
        { max.x + 16.0f * s, panel_min.y });
    ImDrawList* draw = ImGui::GetWindowDrawList();

    window_shadow(draw, min, max, 8.0f * s, s);
    draw->AddRectFilled(min, max, IM_COL32(42, 45, 45, 248), 8.0f * s);
    draw->AddRect(min, max, IM_COL32(255, 255, 255, 38), 8.0f * s, 0, 1.0f);

    // The search field: the shared input component in its dark suit.
    const ImVec2 search_min { min.x + 18.0f * s, min.y + 17.0f * s };
    const ImVec2 search_max { max.x - 18.0f * s, min.y + 59.0f * s };
    IzanFieldStyle search_style;
    search_style.bg = IM_COL32(28, 30, 30, 255);
    search_style.border = IM_COL32(109, 190, 69, 120);
    search_style.text = IM_COL32(230, 232, 231, 255);
    search_style.hint = IM_COL32(151, 155, 152, 255);
    search_style.rounding = 5.0f;
    izan_input("##cinnamon-search", "Type to search applications…",
        menu_search_.data(), menu_search_.size(), search_min, search_max,
        search_style);

    // The category column.
    const float categories_width = 190.0f * s;
    static constexpr std::array<const char*, 8> kCategories
        = { "All Applications", "Accessories", "Graphics", "Internet", "Office",
              "Sound & Video", "System Tools", "Preferences" };
    for (int i = 0; i < 8; ++i) {
        const ImVec2 row_min { min.x + 12.0f * s,
            min.y + (80.0f + i * 50.0f) * s };
        const ImVec2 row_max { min.x + categories_width,
            row_min.y + 44.0f * s };
        const bool hot = ImGui::IsMouseHoveringRect(row_min, row_max, false);
        if (i == menu_category_ || hot)
            draw->AddRectFilled(row_min, row_max,
                i == menu_category_ ? IM_COL32(109, 190, 69, 75)
                                    : IM_COL32(255, 255, 255, 18),
                4.0f * s);
        draw->AddCircleFilled(
            { row_min.x + 17.0f * s, (row_min.y + row_max.y) * 0.5f }, 6.0f * s,
            i == menu_category_ ? IM_COL32(109, 190, 69, 255)
                                : IM_COL32(124, 128, 125, 255));
        text_vcentered(draw, row_min.x + 34.0f * s,
            (row_min.y + row_max.y) * 0.5f, IM_COL32(230, 232, 231, 255),
            kCategories[static_cast<std::size_t>(i)], kFontBodyCompact * s);
        const std::string cid = "##menu-category-" + std::to_string(i);
        if (invisible_hit(cid.c_str(), row_min, row_max))
            menu_category_ = i;
    }
    draw->AddLine({ min.x + categories_width + 8.0f * s, min.y + 75.0f * s },
        { min.x + categories_width + 8.0f * s, max.y - 58.0f * s },
        IM_COL32(255, 255, 255, 25), 1.0f);

    // The application rows; a row whose launch id matches an attached
    // app really opens it.
    struct Row {
        const char* name;
        const char* detail;
        ImU32 color;
        const char* glyph;
        const char* launch_id;
    };

    static constexpr std::array<Row, 8> kRows = { {
        { "Web Browser", "Browse the web", IM_COL32(77, 149, 209, 255), "●",
            nullptr },
        { "Files", "Access and organize files", IM_COL32(109, 190, 69, 255),
            "F", "files" },
        { "Terminal", "Use the command line", IM_COL32(55, 58, 57, 255), ">_",
            nullptr },
        { "Text Editor", "Edit text files", IM_COL32(79, 155, 115, 255), "●",
            "notes" },
        { "Software Manager", "Install new applications",
            IM_COL32(224, 148, 52, 255), "●", nullptr },
        { "System Settings", "Control Center", IM_COL32(113, 121, 117, 255),
            "●", "gallery" },
        { "Calculator", "Perform calculations", IM_COL32(81, 142, 190, 255),
            "●", nullptr },
        { "Media Player", "Play movies and music", IM_COL32(171, 91, 167, 255),
            "●", nullptr },
    } };
    const float apps_x = min.x + categories_width + 28.0f * s;
    const std::string query(menu_search_.data());
    int visible = 0;
    // One row's furniture, shared by both modes: highlight, colored
    // square with a glyph, name over detail, the whole row a launch
    // target when an app rides it.
    const auto app_row = [&](int i, const char* name, const char* detail,
                             ImU32 color, const char* glyph,
                             App* launches) -> bool {
        const float y = min.y + (82.0f + visible * 55.0f) * s;
        if (y + 53.0f * s > max.y - 58.0f * s)
            return false; // the footer's territory: stop listing
        const ImVec2 app_min { apps_x - 8.0f * s, y - 2.0f * s };
        const ImVec2 app_max { max.x - 14.0f * s, y + 53.0f * s };
        const bool hot = ImGui::IsMouseHoveringRect(app_min, app_max, false);
        if (hot || menu_app_ == i)
            draw->AddRectFilled(app_min, app_max,
                menu_app_ == i ? IM_COL32(109, 190, 69, 48)
                               : IM_COL32(255, 255, 255, 15),
                5.0f * s);
        draw->AddRectFilled({ apps_x, y + 7.0f * s },
            { apps_x + 38.0f * s, y + 45.0f * s }, color, 8.0f * s);
        text_centered(draw, { apps_x + 19.0f * s, y + 26.0f * s },
            IM_COL32(255, 255, 255, 255), glyph, kFontSymbol * s);
        text_vcentered(draw, apps_x + 51.0f * s, y + 13.0f * s,
            IM_COL32(239, 241, 240, 255), name, kFontBodyCompact * s);
        text_vcentered(draw, apps_x + 51.0f * s, y + 39.0f * s,
            IM_COL32(147, 151, 148, 255), detail, kFontSecondary * s);
        const std::string rid = "##menu-app-" + std::to_string(i);
        if (invisible_hit(rid.c_str(), app_min, app_max)) {
            menu_app_ = i;
            if (launches != nullptr) {
                wm.launch(launches);
                menu_open_ = false;
            }
        }
        ++visible;
        return true;
    };
    if (menu_roster_) {
        // The real shelf: every attached app, every row launches. The
        // list scrolls under the wheel — a shelf outgrows a fixed
        // page the day a developer gets a dev loop (2026-07-21).
        static constexpr std::array<ImU32, 6> kRosterColors = { {
            IM_COL32(109, 190, 69, 255),
            IM_COL32(77, 149, 209, 255),
            IM_COL32(79, 155, 115, 255),
            IM_COL32(224, 148, 52, 255),
            IM_COL32(113, 121, 117, 255),
            IM_COL32(171, 91, 167, 255),
        } };
        std::vector<AppDescriptor> roster = catalog_;
        if (roster.empty())
            for (App* app : apps)
                roster.push_back({ app->id(), app->name(), app->mark() });
        std::vector<const AppDescriptor*> shown;
        for (const AppDescriptor& app : roster)
            if (query.empty()
                || std::string_view(app.name).find(query)
                    != std::string_view::npos
                || std::string_view(app.id).find(query)
                    != std::string_view::npos)
                shown.push_back(&app);
        const float row_top = min.y + 75.0f * s;
        const float row_bottom = max.y - 58.0f * s;
        const float span
            = static_cast<float>(shown.size()) * 55.0f * s + 12.0f * s;
        const float max_scroll
            = std::max(0.0f, span - (row_bottom - row_top));
        if (ImGui::IsMouseHoveringRect(
                { apps_x - 8.0f * s, row_top }, { max.x, row_bottom }, false))
            menu_scroll_ -= ImGui::GetIO().MouseWheel * 55.0f * s;
        menu_scroll_ = std::clamp(menu_scroll_, 0.0f, max_scroll);
        for (int i = 0; i < static_cast<int>(shown.size()); ++i) {
            const AppDescriptor& app = *shown[static_cast<std::size_t>(i)];
            const float y
                = min.y + (82.0f + i * 55.0f) * s - menu_scroll_;
            if (y + 53.0f * s > row_bottom || y < row_top)
                continue;
            const ImVec2 app_min { apps_x - 8.0f * s, y - 2.0f * s };
            const ImVec2 app_max { max.x - 14.0f * s, y + 53.0f * s };
            const bool hot
                = ImGui::IsMouseHoveringRect(app_min, app_max, false);
            if (hot || menu_app_ == i)
                draw->AddRectFilled(app_min, app_max,
                    menu_app_ == i ? IM_COL32(109, 190, 69, 48)
                                   : IM_COL32(255, 255, 255, 15),
                    5.0f * s);
            draw->AddRectFilled({ apps_x, y + 7.0f * s },
                { apps_x + 38.0f * s, y + 45.0f * s },
                kRosterColors[static_cast<std::size_t>(i)
                    % kRosterColors.size()],
                8.0f * s);
            text_centered(draw, { apps_x + 19.0f * s, y + 26.0f * s },
                IM_COL32(255, 255, 255, 255), app.mark.c_str(),
                kFontSymbol * s);
            text_vcentered(draw, apps_x + 51.0f * s, y + 13.0f * s,
                IM_COL32(239, 241, 240, 255), app.name.c_str(),
                kFontBodyCompact * s);
            text_vcentered(draw, apps_x + 51.0f * s, y + 39.0f * s,
                IM_COL32(147, 151, 148, 255), app.id.c_str(),
                kFontSecondary * s);
            const std::string rid = "##menu-app-" + std::to_string(i);
            if (invisible_hit(rid.c_str(), app_min, app_max)) {
                menu_app_ = i;
                App* live = nullptr;
                for (App* candidate : apps)
                    if (app.id == candidate->id()) {
                        live = candidate;
                        break;
                    }
                if (live)
                    wm.launch(live);
                else
                    request_launch(app.id);
                menu_open_ = false;
            }
        }
    } else {
        for (int i = 0; i < 8; ++i) {
            const Row& row = kRows[static_cast<std::size_t>(i)];
            if (!query.empty()
                && std::string_view(row.name).find(query)
                    == std::string_view::npos
                && std::string_view(row.detail).find(query)
                    == std::string_view::npos)
                continue;
            App* launches = nullptr;
            if (row.launch_id != nullptr)
                for (App* app : apps)
                    if (std::strcmp(app->id(), row.launch_id) == 0) {
                        launches = app;
                        break;
                    }
            if (!app_row(i, row.name, row.detail, row.color, row.glyph,
                    launches))
                break;
        }
    }

    // The session footer.
    draw->AddLine({ min.x + 12.0f * s, max.y - 53.0f * s },
        { max.x - 12.0f * s, max.y - 53.0f * s }, IM_COL32(255, 255, 255, 28),
        1.0f);
    mint_logo(draw, { min.x + 31.0f * s, max.y - 28.0f * s }, 13.0f * s);
    text_at(draw, { min.x + 52.0f * s, max.y - 39.0f * s },
        IM_COL32(237, 239, 238, 255), "dean", kFontBody * s);
    static constexpr std::array<const char*, 3> kSession = { "S", "L", "P" };
    for (int i = 0; i < 3; ++i) {
        const ImVec2 center { max.x - (118.0f - i * 42.0f) * s,
            max.y - 27.0f * s };
        draw->AddCircle(
            center, 12.0f * s, IM_COL32(202, 205, 203, 255), 24, 1.3f * s);
        text_centered(draw, center, IM_COL32(225, 227, 226, 255),
            kSession[static_cast<std::size_t>(i)], kFontSymbolSmall * s);
    }

    end_furniture_window();
}

}
