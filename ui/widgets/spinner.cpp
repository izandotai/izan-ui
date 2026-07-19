#include "ui/widgets/spinner.hpp"

#include <cmath>

#include <imgui.h>

#include "ui/widgets/design.hpp"

namespace izan::ui {

void kit_spinner(float radius_em)
{
    const float em = ImGui::GetFontSize();
    const float radius = em * radius_em;
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const ImVec2 center(pos.x + radius, pos.y + radius);

    const float turn = float(ImGui::GetTime()) * 6.28318f * 0.9f;
    ImDrawList* draw = ImGui::GetWindowDrawList();
    draw->PathArcTo(center, radius, turn, turn + 4.4f, 24);
    draw->PathStroke(ImGui::GetColorU32(kit_accent()), 0, radius * 0.28f);
    ImGui::Dummy(ImVec2(radius * 2.0f, radius * 2.0f));
}

}
