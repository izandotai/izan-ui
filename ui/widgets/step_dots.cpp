#include "ui/widgets/step_dots.hpp"

#include <imgui.h>

#include "ui/widgets/design.hpp"

namespace izan::ui {

void kit_step_dots(int current, int total)
{
    const float em = ImGui::GetFontSize();
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    ImDrawList* draw = ImGui::GetWindowDrawList();
    for (int i = 0; i < total; ++i) {
        const ImVec2 c(
            pos.x + em * 0.25f + float(i) * em * 0.75f, pos.y + em * 0.5f);
        if (i == current)
            draw->AddCircleFilled(
                c, em * 0.16f, ImGui::GetColorU32(kit_accent()));
        else
            draw->AddCircle(
                c, em * 0.16f, ImGui::GetColorU32(ImGuiCol_TextDisabled, 0.6f));
    }
    ImGui::Dummy(ImVec2(em * (0.5f + 0.75f * float(total)), em));
}

}
