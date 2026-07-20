#include "desktop.hpp"

#include "apple_menu.hpp"
#include "control_center.hpp"
#include "desktop_icons.hpp"
#include "dock.hpp"
#include "finder.hpp"
#include "menu_bar.hpp"
#include "paint.hpp"
#include "wallpaper.hpp"

namespace macos {

void Desktop::render(ImVec2 pos, ImVec2 size)
{
    const Layout layout = Layout::fit(pos, size);

    ImGui::SetNextWindowPos(pos);
    ImGui::SetNextWindowSize(size);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(0, 0, 0, 0));
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration
        | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings
        | ImGuiWindowFlags_NoBringToFrontOnFocus;
    ImGui::Begin("##macos-desktop", nullptr, flags);
    ImDrawList* draw = ImGui::GetWindowDrawList();
    draw->PushClipRect(pos, add(pos, size), true);

    draw_wallpaper(draw, layout, state_);
    draw_desktop_icons(draw, layout, state_);
    if (state_.finder.visible && !state_.finder.minimized) {
        draw_finder(draw, layout, state_);
    }
    draw_menu_bar(draw, layout, state_);
    draw_dock(draw, layout, state_);
    if (state_.menu.apple_menu_open) {
        draw_apple_menu(draw, layout, state_);
    }
    if (state_.menu.control_center_open) {
        draw_control_center(draw, layout, state_);
    }

    draw->PopClipRect();
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}

}
