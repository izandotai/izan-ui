#include "ui/os/shell.hpp"

#include <algorithm>

#include "ui/widgets/scrollbar.hpp"

namespace izan::os {

void Shell::attach(App* app)
{
    if (std::find(apps_.begin(), apps_.end(), app) != apps_.end())
        return;
    apps_.push_back(app);
    wm_.attach(app);
}

void Shell::detach(App* app)
{
    wm_.close(app);
    const auto it = std::find(apps_.begin(), apps_.end(), app);
    if (it != apps_.end())
        apps_.erase(it);
}

void Shell::frame(ImVec2 pos, ImVec2 size)
{
    const float em = ImGui::GetFontSize();
    const ImVec2 view_max { pos.x + size.x, pos.y + size.y };

    // Stock scrollbars stay invisible under every theme; the skin
    // pass at the end of this frame repaints them Mint-mannered.
    ui::kit_scrollbars_stock_hide();

    // The desktop canvas: wallpaper only, pinned to the bottom of
    // the stack.
    ImGui::SetNextWindowPos(pos);
    ImGui::SetNextWindowSize(size);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(0, 0, 0, 0));
    ImGui::Begin("###os-desktop", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse
            | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar
            | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoDocking
            | ImGuiWindowFlags_NoBringToFrontOnFocus
            | ImGuiWindowFlags_NoFocusOnAppearing);
    ImDrawList* draw = ImGui::GetWindowDrawList();
    draw->PushClipRect(pos, view_max, true);
    if (wallpaper_ != 0)
        draw->AddImage(
            wallpaper_, pos, view_max, wallpaper_uv0_, wallpaper_uv1_);
    else
        wm_.theme().paint_wallpaper(draw, pos, view_max, em);
    // Furniture rides on either backdrop, painted or blitted.
    wm_.theme().paint_desktop_icons(draw, pos, view_max, em);
    draw->PopClipRect();
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);

    // The panel carves the workspace from below; windows live above
    // it and its click territory (strip plus open menu) is blocked
    // from window input.
    const ImVec2 ws_max { view_max.x, view_max.y - panel_.height(em) };
    wm_.frame(pos, ws_max, panel_.blocked());
    panel_.frame(wm_, apps_, pos, view_max);
    // The frame's last word on display order: popups above everything,
    // panel included — the one ordering imgui's own click-to-focus
    // agrees with (see front_popups).
    wm_.front_popups();

    // Every window has drawn; dress every live scrollbar in the
    // theme's own ink.
    ui::kit_skin_scrollbars(wm_.theme().scrollbar_ink());
}

}
