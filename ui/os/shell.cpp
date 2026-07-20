#include "ui/os/shell.hpp"

namespace izan::os {

void Shell::attach(App* app)
{
    apps_.push_back(app);
    wm_.attach(app);
}

void Shell::frame(ImVec2 pos, ImVec2 size)
{
    const float em = ImGui::GetFontSize();
    const ImVec2 view_max { pos.x + size.x, pos.y + size.y };

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
    wm_.theme().paint_wallpaper(draw, pos, view_max, em);
    draw->PopClipRect();
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);

    // The bar carves the workspace; windows live below it, the dock
    // floats over them and owns its own clicks.
    const float bar_h = menu_.frame(wm_, mark_, pos, view_max);
    const ImVec2 ws_min { pos.x, pos.y + bar_h };
    wm_.frame(ws_min, view_max, dock_.shelf_min(), dock_.shelf_max());
    dock_.frame(wm_, apps_, ws_min, view_max);
}

}
