#include "ui/widgets/search_field.hpp"

#include <imgui.h>

#include "ui/widgets/design.hpp"
#include "ui/widgets/text_field.hpp"

namespace izan::ui {

void kit_search_field(
    const char* id, const char* hint, char* buf, std::size_t size)
{
    const float em = ImGui::GetFontSize();
    const float w = ImGui::CalcItemWidth();
    // The composite wears the same well metrics as every plain field —
    // ambient style would leave it a squat stranger among tall wells.
    const float h = em * (1.0f + 2.0f * design().field_pad_y);
    const float d = em * 1.1f;

    ImGui::PushID(id);

    // The frame spans the full row; the input stops before the glyph
    // zone so text and caret never run under the cross.
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    kit_field_frame(pos, ImVec2(w, h));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,
        ImVec2(em * design().field_pad_x, em * design().field_pad_y));
    ImGui::SetNextItemWidth(w - d - em * 0.55f);
    ImGui::InputTextWithHint("##text", hint, buf, size);
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();

    if (buf[0] != '\0') {
        const ImVec2 keep = ImGui::GetCursorScreenPos();
        ImGui::SetCursorScreenPos(
            ImVec2(pos.x + w - d - em * 0.55f, pos.y + (h - d) * 0.5f));
        ImGui::InvisibleButton("##clear", ImVec2(d, d));
        const bool hovered = ImGui::IsItemHovered();
        if (hovered)
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        if (ImGui::IsItemClicked())
            buf[0] = '\0';
        const ImVec2 gpos = ImGui::GetItemRectMin();
        const ImVec2 c(gpos.x + d * 0.5f, gpos.y + d * 0.5f);
        const float a = d * 0.22f;
        const float t = d * 0.085f;
        const ImU32 tone = ImGui::GetColorU32(
            hovered ? ImGuiCol_Text : ImGuiCol_TextDisabled);
        ImDrawList* draw = ImGui::GetWindowDrawList();
        draw->AddLine(
            ImVec2(c.x - a, c.y - a), ImVec2(c.x + a, c.y + a), tone, t);
        draw->AddLine(
            ImVec2(c.x - a, c.y + a), ImVec2(c.x + a, c.y - a), tone, t);
        kit_cursor_restore(keep);
    }

    ImGui::PopID();
}

}
