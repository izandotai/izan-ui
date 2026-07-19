#include "ui/widgets/card.hpp"

#include "ui/widgets/design.hpp"

namespace izan::ui {

void kit_group_begin(const char* id, float width)
{
    const DesignLanguage& dl = design();
    const ImVec4 base = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
    const ImVec4 text = ImGui::GetStyleColorVec4(ImGuiCol_Text);
    ImGui::PushStyleColor(ImGuiCol_ChildBg,
        kit_blend(base, text,
            kit_is_dark() ? dl.group_elevation_dark
                          : dl.group_elevation_light));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
        ImVec2(ImGui::GetFontSize() * dl.group_pad_x,
            ImGui::GetFontSize() * dl.group_pad_y));
    // The card never scrolls itself — it sizes to its content — so the
    // wheel must pass through to the page, or hovering the card kills
    // scrolling for whatever it covers.
    ImGui::BeginChild(id, ImVec2(width, 0.0f),
        ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysUseWindowPadding,
        ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);
}

void kit_group_end()
{
    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

void kit_hairline()
{
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const float width = ImGui::GetContentRegionAvail().x;
    ImGui::GetWindowDrawList()->AddLine(ImVec2(pos.x, pos.y),
        ImVec2(pos.x + width, pos.y),
        ImGui::GetColorU32(ImGuiCol_Separator, 0.45f));
    ImGui::Dummy(ImVec2(0.0f, ImGui::GetStyle().ItemSpacing.y * 0.5f));
}

}
