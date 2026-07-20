#include "ui/widgets/list_row.hpp"

#include <string>

#include <imgui.h>

#include "ui/widgets/avatar.hpp"
#include "ui/widgets/design.hpp"
#include "ui/widgets/label.hpp"

namespace izan::ui {

bool kit_list_row(const char* id, const char* title, const char* subtitle,
    bool selected, bool active_dot)
{
    const DesignLanguage& dl = design();
    const float em = ImGui::GetFontSize();
    const float row_h = em * dl.list_row_height;
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    // The host's window padding is the breath on every side — the
    // scrollbar sits outside the padded region, so rows and bar never
    // touch without the row second-guessing the host.
    const float row_w = ImGui::GetContentRegionAvail().x;
    const bool clicked = ImGui::InvisibleButton(id, ImVec2(row_w, row_h));
    const bool hovered = ImGui::IsItemHovered();

    ImDrawList* draw = ImGui::GetWindowDrawList();
    if (selected || hovered)
        kit_round_fill(draw, pos, ImVec2(pos.x + row_w, pos.y + row_h),
            em * dl.selection_radius,
            ImGui::GetColorU32(
                selected ? ImGuiCol_Header : ImGuiCol_HeaderHovered,
                selected ? 1.0f : 0.55f));

    const float avatar = em * dl.list_avatar;
    kit_avatar_at(ImVec2(pos.x + em * 0.35f, pos.y + (row_h - avatar) * 0.5f),
        title, avatar);

    const bool has_subtitle = subtitle && *subtitle;
    const float text_x = kit_snap(pos.x + em * 0.35f + avatar + em * 0.45f);
    const float name_y = kit_snap(
        has_subtitle ? pos.y + em * 0.28f : pos.y + (row_h - em) * 0.5f);
    // Text ends before the state dot, however long the name runs.
    const float text_budget = pos.x + row_w - em * 1.2f - text_x;
    const std::string shown_title = kit_elide_end(title, text_budget);
    draw->AddText(ImVec2(text_x, name_y), ImGui::GetColorU32(ImGuiCol_Text),
        shown_title.c_str());
    if (has_subtitle) {
        const std::string shown_sub
            = kit_elide_end(subtitle, text_budget, kit_caption_size());
        draw->AddText(ImGui::GetFont(), kit_caption_size(),
            ImVec2(text_x, kit_snap(name_y + em * 1.05f)),
            ImGui::GetColorU32(ImGuiCol_TextDisabled), shown_sub.c_str());
    }

    const ImVec2 dot(pos.x + row_w - em * 0.7f, pos.y + row_h * 0.5f);
    if (active_dot)
        draw->AddCircleFilled(
            dot, em * 0.16f, ImGui::GetColorU32(kit_accent()));
    else
        draw->AddCircle(
            dot, em * 0.16f, ImGui::GetColorU32(ImGuiCol_TextDisabled, 0.7f));
    return clicked;
}

}
