#include "ui/widgets/pill.hpp"

#include "ui/widgets/label.hpp"

namespace izan::ui {

void kit_pill(const char* text, ImVec4 tint)
{
    ImGui::PushFont(nullptr, kit_caption_size());
    const ImVec2 label = ImGui::CalcTextSize(text);
    const float pad_x = ImGui::GetFontSize() * 0.55f;
    const float pad_y = ImGui::GetFontSize() * 0.18f;
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const ImVec2 size(label.x + pad_x * 2.0f, label.y + pad_y * 2.0f);

    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImVec4 bg = tint;
    bg.w = 0.16f;
    draw->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y),
        ImGui::GetColorU32(bg), size.y * 0.5f);
    draw->AddText(
        ImVec2(pos.x + pad_x, pos.y + pad_y), ImGui::GetColorU32(tint), text);
    ImGui::Dummy(size);
    ImGui::PopFont();
}

}
